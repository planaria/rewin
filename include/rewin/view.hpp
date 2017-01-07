#pragma once
#include "view_hooks.hpp"
#include "mouse_button.hpp"
#include "mouse_action.hpp"
#include "unsubscriber.hpp"

namespace rewin
{

	class window;

	class view
		: public std::enable_shared_from_this<view>
		, protected unsubscriber
		, detail::view_view_hook
		//, detail::view_window_hook
	{
	protected:

		view()
		{
		}

		virtual void initialize()
		{
			auto parent_enable = parent().as_reactive()
				.map([](const std::weak_ptr<view>& parent) -> reactive<bool>
			{
				auto p = parent.lock();
				if (!p)
					return true;

				return p->enable_actual().as_reactive();
			}).switch_on_next();

			enable_actual_ =
				combine(
					parent_enable,
					enable().as_reactive())
				.map([](const auto& t)
			{
				return std::get<0>(t) && std::get<1>(t);
			});
		}

	public:

		virtual ~view()
		{
		}

		virtual reactive<bool> is_hover_target()
		{
			return false;
		}

		virtual reactive<bool> is_focus_target()
		{
			return false;
		}

		reactive<bool> is_hovered()
		{
			return owner_window().as_reactive()
				.map([](const auto& owner_window) -> reactive<std::weak_ptr<view>>
			{
				auto w = owner_window.lock();
				if (!w)
					return std::weak_ptr<view>();

				return w->hovered_view().as_reactive();
			})
				.switch_on_next()
				.map([&](const std::weak_ptr<view>& view)
			{
				return view.lock().get() == this;
			});
		}

		reactive<bool> is_focused()
		{
			auto window_is_focused = owner_window().as_reactive()
				.map([](const auto& owner_window) -> reactive<bool>
			{
				auto w = owner_window.lock();
				if (!w)
					return false;

				return w->focused().as_reactive();
			}).switch_on_next();

			auto this_is_focused = owner_window().as_reactive()
				.map([](const auto& owner_window) -> reactive<std::weak_ptr<view>>
			{
				auto w = owner_window.lock();
				if (!w)
					return std::weak_ptr<view>();

				return w->focused_view().as_reactive();
			})
				.switch_on_next()
				.map([&](const std::weak_ptr<view>& view)
			{
				return view.lock().get() == this;
			});

			return combine(window_is_focused, this_is_focused)
				.map([](const auto& t)
			{
				return std::get<0>(t) && std::get<1>(t);
			});
		}

		virtual void on_paint(WTL::CDCHandle dc)
		{
		}

		virtual bool on_key_down(UINT nChar, UINT nRepCnt, UINT nFlags)
		{
			return false;
		}

		virtual bool on_key_up(UINT nChar, UINT nRepCnt, UINT nFlags)
		{
			return false;
		}

		virtual bool on_char(UINT nChar, UINT nRepCnt, UINT nFlags)
		{
			return false;
		}

		virtual bool on_ime_start_composition()
		{
			return false;
		}

		virtual bool on_ime_composition(const std::wstring& result_str)
		{
			return false;
		}

		virtual bool on_query_char_position(WTL::CRect& result)
		{
			return false;
		}

		virtual bool on_mouse_move(UINT nFlags, WTL::CPoint point)
		{
			return false;
		}

		virtual bool on_mouse_wheel(UINT nFlags, short zDelta, WTL::CPoint pt)
		{
			return false;
		}

		virtual bool on_mouse_button(mouse_button button, mouse_action action, UINT nFlags, WTL::CPoint point)
		{
			return false;
		}

		const reactive_property<std::weak_ptr<window>>& owner_window()
		{
			return owner_window_;
		}

		const reactive_property<std::weak_ptr<view>>& parent()
		{
			return parent_;
		}

		reactive_property<bool>& enable()
		{
			return enable_;
		}

		const reactive_property<bool>& enable_actual()
		{
			return enable_actual_;
		}

		reactive_property<int>& minimized_width()
		{
			return minimized_width_;
		}

		reactive_property<int>& minimized_height()
		{
			return minimized_height_;
		}

		reactive_property<WTL::CRect>& rect_arranged_horizontal()
		{
			return rect_arranged_horizontal_;
		}

		reactive_property<WTL::CRect>& rect_arranged_vertical()
		{
			return rect_arranged_vertical_;
		}

		std::shared_ptr<view> first_child()
		{
			if (children_.empty())
				return nullptr;

			return static_cast<view&>(children_.front()).shared_from_this();
		}

		std::shared_ptr<view> last_child()
		{
			if (children_.empty())
				return nullptr;

			return static_cast<view&>(children_.back()).shared_from_this();
		}

		std::shared_ptr<view> next_sibling()
		{
			auto p = parent().value().lock();
			if (!p)
				return nullptr;

			auto it = std::next(p->children_.iterator_to(*this));
			if (it == p->children_.end())
				return nullptr;

			return static_cast<view&>(*it).shared_from_this();
		}

		std::shared_ptr<view> prev_sibling()
		{
			auto p = parent().value().lock();
			if (!p)
				return nullptr;

			auto it = p->children_.iterator_to(*this);
			if (it == p->children_.begin())
				return nullptr;

			return static_cast<view&>(*std::prev(it)).shared_from_this();
		}

	protected:

		void clear_children()
		{
			for (auto& child : children_)
			{
				auto& v = static_cast<view&>(child);
				v.parent_ = std::weak_ptr<view>();
				v.owner_window_ = std::weak_ptr<window>();
			}

			children_.clear();
		}

		void add_child(view& child)
		{
			BOOST_ASSERT(!child.is_linked());

			child.parent_ = this->shared_from_this();
			child.owner_window_ = owner_window().as_reactive();
			children_.push_back(child);
		}

	private:

		friend class window;

		reactive_property<std::weak_ptr<window>> owner_window_;
		reactive_property<std::weak_ptr<view>> parent_;

		reactive_property<bool> enable_ = true;
		reactive_property<bool> enable_actual_;

		reactive_property<int> minimized_width_;
		reactive_property<int> minimized_height_;
		reactive_property<WTL::CRect> rect_arranged_horizontal_;
		reactive_property<WTL::CRect> rect_arranged_vertical_;

		boost::intrusive::list<
			detail::view_view_hook,
			boost::intrusive::base_hook<detail::view_view_hook>,
			boost::intrusive::constant_time_size<false>
		> children_;

	};

}

#pragma once
#include "view.hpp"
#include "view_impl.hpp"
#include "scroll_info.hpp"
#include "orientation.hpp"

namespace rewin
{

	class scroll_bar_view : public detail::view_impl<scroll_bar_view, view>
	{
	protected:

		scroll_bar_view()
		{
		}

		virtual void initialize() override
		{
			view::initialize();

			*this +=
				combine(
					rect_arranged_vertical().as_reactive(),
					thumb_position_size_.as_reactive(),
					is_hovered(),
					is_focused(),
					background_brush().as_reactive())
				.subscribe([&](const auto&)
			{
				auto window = owner_window().value().lock();
				if (window)
					window->Invalidate();
			});

			minimized_width() = orientation().as_reactive()
				.map([&](rewin::orientation ori)
			{
				if (ori == rewin::orientation::horizontal)
					return minimum_thumb_size().as_reactive();
				else
					return minimum_thumb_width().as_reactive();
			}).switch_on_next();

			minimized_height() = orientation().as_reactive()
				.map([&](rewin::orientation ori)
			{
				if (ori == rewin::orientation::horizontal)
					return minimum_thumb_width().as_reactive();
				else
					return minimum_thumb_size().as_reactive();
			}).switch_on_next();

			view_size_ = orientation().as_reactive()
				.map([&](rewin::orientation ori)
			{
				if (ori == rewin::orientation::horizontal)
				{
					return rect_arranged_horizontal().as_reactive()
						.map([](const WTL::CRect& rect)
					{
						return rect.Width();
					});
				}
				else
				{
					return rect_arranged_vertical().as_reactive()
						.map([](const WTL::CRect& rect)
					{
						return rect.Height();
					});
				}
			}).switch_on_next();

			thumb_position_size_ =
				combine(
					scroll_info().as_reactive(),
					value().as_reactive(),
					view_size_.as_reactive(),
					minimum_thumb_size().as_reactive())
				.map([](const auto& t)
			{
				const rewin::scroll_info& info = std::get<0>(t);
				int value = std::get<1>(t);
				int view_size = std::get<2>(t);
				auto minimum_thumb_size = std::get<3>(t);

				if (info.check())
				{
					int scroll_size = info.maximum - info.minimum - info.page_size;
					int thumb_size = std::max(minimum_thumb_size, info.page_size * view_size / (info.maximum - info.minimum));
					int thumb_position = (value - info.minimum) * (view_size - thumb_size) / scroll_size;

					return std::make_pair(thumb_position, thumb_size);
				}
				else
				{
					return std::make_pair(0, view_size);
				}
			});

			thumb_excursion_size_ =
				combine(
					view_size_.as_reactive(),
					thumb_position_size_.as_reactive())
				.map([](const auto& t)
			{
				return std::get<0>(t) - std::get<1>(t).second;
			});

			enable() = scroll_info().as_reactive()
				.map([](const rewin::scroll_info& info)
			{
				return info.check();
			});
		}

	public:

		virtual reactive<bool> is_hover_target() override
		{
			return enable_actual().as_reactive();
		}

		virtual reactive<bool> is_focus_target() override
		{
			return enable_actual().as_reactive();
		}

		virtual void on_paint(WTL::CDCHandle dc) override
		{
			WTL::CRect rect = rect_arranged_vertical().value();

			background_brush().value()->fill(dc, &rect);

			if (enable_actual().value())
			{
				auto tps = thumb_position_size_.value();

				WTL::CRect thumb_rect = rect;

				if (orientation().value() == rewin::orientation::horizontal)
				{
					thumb_rect.left += tps.first;
					thumb_rect.right = thumb_rect.left + tps.second;
				}
				else
				{
					thumb_rect.top += tps.first;
					thumb_rect.bottom = thumb_rect.top + tps.second;
				}

				REWIN_CHECK_WIN32(dc.DrawEdge(&thumb_rect, EDGE_RAISED, BF_ADJUST | BF_MIDDLE | BF_RECT));

				if (is_focused().value())
					dc.DrawFocusRect(&thumb_rect);
			}
		}

		virtual bool on_key_down(UINT nChar, UINT nRepCnt, UINT nFlags) override
		{
			if (!enable_actual().value())
				return false;

			const auto& info = scroll_info().value();

			switch (nChar)
			{
			case VK_LEFT:
				if (orientation().value() == rewin::orientation::horizontal)
					value().invoke(info.limit_value(value().value() - info.change_step));
				return true;
			case VK_RIGHT:
				if (orientation().value() == rewin::orientation::horizontal)
					value().invoke(info.limit_value(value().value() + info.change_step));
				return true;
			case VK_UP:
				if (orientation().value() == rewin::orientation::vertical)
					value().invoke(info.limit_value(value().value() - info.change_step));
				return true;
			case VK_DOWN:
				if (orientation().value() == rewin::orientation::vertical)
					value().invoke(info.limit_value(value().value() + info.change_step));
				return true;
			}

			return false;
		}

		virtual bool on_mouse_move(UINT nFlags, WTL::CPoint point) override
		{
			if (!enable_actual().value())
				return false;

			const auto& rect = rect_arranged_vertical().value();

			int p;

			if (orientation().value() == rewin::orientation::horizontal)
				p = point.x - rect.left;
			else
				p = point.y - rect.top;

			if (drag_thumb_ && thumb_excursion_size_.value() > 0)
			{
				int delta = p - drag_thumb_start_position_;
				const auto& info = scroll_info().value();

				int scroll_size = info.maximum - info.minimum - info.page_size;
				int value = drag_thumb_start_value_ + delta * scroll_size / thumb_excursion_size_.value();
				value_.invoke(info.limit_value(value));
			}

			if (drag_outer_)
				scroll_to(p);

			last_position_ = p;

			return false;
		}

		virtual bool on_mouse_button(mouse_button button, mouse_action action, UINT nFlags, WTL::CPoint point) override
		{
			if (!enable_actual().value())
				return false;

			auto w = owner_window().value().lock();
			if (!w)
				return false;

			if (button == mouse_button::left)
			{
				switch (action)
				{
				case mouse_action::down:
				case mouse_action::double_click:
					w->capture_view() = this->shared_from_this();
					w->focused_view() = this->shared_from_this();
					if (last_position_)
					{
						if (is_thumb_hovered())
						{
							drag_thumb_ = true;
							drag_thumb_start_position_ = *last_position_;
							drag_thumb_start_value_ = value().value();
						}
						else
						{
							drag_outer_ = true;
							scroll_to(*last_position_);
						}
					}
					break;
				case mouse_action::up:
					if (drag_thumb_ || drag_outer_)
					{
						w->capture_view() = std::weak_ptr<view>();
						drag_thumb_ = false;
						drag_outer_ = false;
					}
					break;
				}

				return true;
			}

			return false;
		}

		reactive_property<rewin::scroll_info>& scroll_info()
		{
			return scroll_info_;
		}

		reactive_property<int>& value()
		{
			return value_;
		}

		reactive_property<rewin::orientation>& orientation()
		{
			return orientation_;
		}

		reactive_property<int>& minimum_thumb_size()
		{
			return minimum_thumb_size_;
		}

		reactive_property<int>& minimum_thumb_width()
		{
			return minimum_thumb_width_;
		}

		reactive_property<std::shared_ptr<brush>>& background_brush()
		{
			return background_brush_;
		}

	private:

		bool is_thumb_hovered()
		{
			if (drag_thumb_ || drag_outer_)
				return true;

			if (!is_hovered().value())
				return false;

			if (!last_position_)
				return false;

			auto tps = thumb_position_size_.value();

			return
				tps.first < *last_position_ &&
				*last_position_ < tps.first + tps.second;
		}

		void scroll_to(int position)
		{
			int view_size = view_size_.value();
			if (view_size <= 0)
				return;

			auto thumb_size = thumb_position_size_.value().second;
			const auto& info = scroll_info().value();

			int value = info.minimum + (info.maximum - info.minimum) * (position - thumb_size / 2) / view_size;
			value_.invoke(info.limit_value(value));
		}

		reactive_property<rewin::scroll_info> scroll_info_;
		reactive_property<int> value_;

		reactive_property<rewin::orientation> orientation_ = rewin::orientation::horizontal;
		reactive_property<int> minimum_thumb_size_ = 16;
		reactive_property<int> minimum_thumb_width_ = 16;

		reactive_property<std::shared_ptr<brush>> background_brush_ = std::make_shared<solid_brush>(0xcc, 0xcc, 0xcc);

		reactive_property<int> view_size_;
		reactive_property<std::pair<int, int>> thumb_position_size_;
		reactive_property<int> thumb_excursion_size_;

		bool drag_thumb_ = false;
		int drag_thumb_start_position_ = 0;
		int drag_thumb_start_value_ = 0;

		bool drag_outer_ = false;

		boost::optional<int> last_position_;

	};

}

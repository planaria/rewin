#pragma once
#include "view.hpp"
#include "view_impl.hpp"
#include "dock_view.hpp"
#include "clip_view.hpp"
#include "scroll_bar_view.hpp"

namespace rewin
{

	class scroll_view : public detail::view_impl<scroll_view, view>
	{
	protected:

		scroll_view()
		{
		}

		virtual void initialize() override
		{
			view::initialize();

			direction() = rewin::direction::right;
			dock_view_->contents() = { scroll_bar_view_, clip_view_ };

			minimized_width() = dock_view_->minimized_width().as_reactive();
			minimized_height() = dock_view_->minimized_height().as_reactive();

			dock_view_->rect_arranged_horizontal() = rect_arranged_horizontal().as_reactive();
			dock_view_->rect_arranged_vertical() = rect_arranged_vertical().as_reactive();

			clip_view_->clip_horizontal() = direction().as_reactive()
				.map([](rewin::direction dir)
			{
				return dir == rewin::direction::top || dir == rewin::direction::bottom;
			});

			clip_view_->clip_vertical() = direction().as_reactive()
				.map([](rewin::direction dir)
			{
				return dir == rewin::direction::left || dir == rewin::direction::right;
			});

			clip_view_->offset_x() = direction().as_reactive()
				.map([&](rewin::direction dir) -> reactive<int>
			{
				if (dir == rewin::direction::left || dir == rewin::direction::right)
					return 0;

				return scroll_bar_view_->value().as_reactive();
			}).switch_on_next();

			clip_view_->offset_y() = direction().as_reactive()
				.map([&](rewin::direction dir) -> reactive<int>
			{
				if (dir == rewin::direction::top || dir == rewin::direction::bottom)
					return 0;

				return scroll_bar_view_->value().as_reactive();
			}).switch_on_next();

			scroll_bar_view_->orientation() = direction().as_reactive()
				.map([](rewin::direction dir)
			{
				if (dir == rewin::direction::left || dir == rewin::direction::right)
					return rewin::orientation::vertical;
				else
					return rewin::orientation::horizontal;
			});

			scroll_bar_view_->scroll_info() =
				combine(
					direction().as_reactive(),
					clip_view_->rect_arranged_vertical().as_reactive())
				.map([&](const auto& t)
			{
				rewin::direction dir = std::get<0>(t);
				const WTL::CRect& rect = std::get<1>(t);

				switch (dir)
				{
				case rewin::direction::left:
				case rewin::direction::right:
					return clip_view_->contents_maximum_height()
						.map([=](int height)
					{
						return scroll_info(0, height, rect.Height(), 20);
					});
					break;
				case rewin::direction::top:
				case rewin::direction::bottom:
				default:
					return clip_view_->contents_maximum_width()
						.map([=](int width)
					{
						return scroll_info(0, width, rect.Width(), 1);
					});
					break;
				}
			}).switch_on_next();

			*this +=
				combine(
					scroll_bar_view_->scroll_info().as_reactive(),
					scroll_bar_view_->value().as_reactive())
				.subscribe([&](const auto& t)
			{
				const scroll_info& info = std::get<0>(t);
				int val = std::get<1>(t);

				if (info.check())
				{
					if (val < info.minimum)
						scroll_bar_view_->value().invoke(info.minimum);
					else if (val > info.maximum - info.page_size)
						scroll_bar_view_->value().invoke(info.maximum - info.page_size);
				}
				else
				{
					if (val != info.minimum)
						scroll_bar_view_->value().invoke(info.minimum);
				}
			});

			add_child(*dock_view_);
		}

	public:

		virtual reactive<bool> is_hover_target()
		{
			return enable_actual().as_reactive();
		}

		virtual void on_paint(WTL::CDCHandle dc) override
		{
			dock_view_->on_paint(dc);
		}

		virtual bool on_mouse_wheel(UINT nFlags, short zDelta, WTL::CPoint pt) override
		{
			if (!enable_actual().value())
				return false;

			rewin::direction dir = direction().value();
			if (dir == rewin::direction::top || dir == rewin::direction::bottom)
				return false;

			const scroll_info& info = scroll_bar_view_->scroll_info().value();
			if (!info.check())
				return false;

			int value = scroll_bar_view_->value().value();

			if (zDelta < 0)
			{
				if (value == info.maximum)
					return false;
			}
			else
			{
				if (value == info.minimum)
					return false;
			}

			value -= zDelta;
			value = info.limit_value(value);
			scroll_bar_view_->value().invoke(value);

			return true;
		}

		reactive_property<rewin::direction>& direction()
		{
			return dock_view_->direction();
		}

		reactive_property<std::vector<std::shared_ptr<view>>>& contents()
		{
			return clip_view_->contents();
		}

	private:

		std::shared_ptr<dock_view> dock_view_ = dock_view::create();
		std::shared_ptr<clip_view> clip_view_ = clip_view::create();
		std::shared_ptr<scroll_bar_view> scroll_bar_view_ = scroll_bar_view::create();

	};

}

#pragma once
#include "contents_view.hpp"
#include "view_impl.hpp"

namespace rewin
{

	class clip_view : public detail::view_impl<clip_view, detail::contents_view>
	{
	protected:

		clip_view()
		{
		}

		virtual void initialize() override
		{
			detail::contents_view::initialize();

			minimized_width() = clip_horizontal().as_reactive()
				.map([&](bool clip) -> reactive<int>
			{
				if (clip)
					return 0;
				else
					return contents_maximum_width();
			}).switch_on_next();

			minimized_height() = clip_vertical().as_reactive()
				.map([&](bool clip) -> reactive<int>
			{
				if (clip)
					return 0;
				else
					return contents_maximum_height();
			}).switch_on_next();

			*this += contents().as_reactive()
				.subscribe([&](const auto& contents)
			{
				auto rect_h = combine(
					rect_arranged_horizontal().as_reactive(),
					clip_horizontal().as_reactive(),
					minimized_width().as_reactive(),
					offset_x().as_reactive())
					.map([](const auto& t)
				{
					const WTL::CRect& rect = std::get<0>(t);
					bool clip_horizontal = std::get<1>(t);
					int width = std::get<2>(t);
					int offset_x = std::get<3>(t);

					WTL::CRect r = rect;

					if (clip_horizontal)
					{
						r.left -= offset_x;
						r.right = r.left + std::max(width, r.Width());
					}

					return r;
				});

				auto rect_v = combine(
					rect_arranged_vertical().as_reactive(),
					clip_horizontal().as_reactive(),
					clip_vertical().as_reactive(),
					minimized_width().as_reactive(),
					minimized_height().as_reactive(),
					offset_x().as_reactive(),
					offset_y().as_reactive())
					.map([](const auto& t)
				{
					const WTL::CRect& rect = std::get<0>(t);
					bool clip_horizontal = std::get<1>(t);
					bool clip_vertical = std::get<2>(t);
					int width = std::get<3>(t);
					int height = std::get<4>(t);
					int offset_x = std::get<5>(t);
					int offset_y = std::get<6>(t);

					WTL::CRect r = rect;

					if (clip_horizontal)
					{
						r.left -= offset_x;
						r.right = r.left + std::max(width, r.Width());
					}

					if (clip_vertical)
					{
						r.top -= offset_y;
						r.bottom = r.top + std::max(height, r.Height());
					}

					return r;
				});
				
				for (const auto& content : contents)
				{
					content->rect_arranged_horizontal() = rect_h;
					content->rect_arranged_vertical() = rect_v;
				}
			});
		}

	public:

		virtual void on_paint(WTL::CDCHandle dc) override
		{
			WTL::CRgn region;
			REWIN_CHECK_WIN32(dc.GetClipRgn(region) != -1);

			const WTL::CRect& rect = rect_arranged_vertical().value();
			REWIN_CHECK_WIN32(dc.IntersectClipRect(rect) != ERROR);

			detail::contents_view::on_paint(dc);

			REWIN_CHECK_WIN32(dc.SelectClipRgn(region) != ERROR);
		}

		reactive_property<bool>& clip_horizontal()
		{
			return clip_horizontal_;
		}

		reactive_property<bool>& clip_vertical()
		{
			return clip_vertical_;
		}

		reactive_property<int>& offset_x()
		{
			return offset_x_;
		}

		reactive_property<int>& offset_y()
		{
			return offset_y_;
		}

		reactive<int> contents_maximum_width()
		{
			return detail::contents_view::contents_maximum_width();
		}

		reactive<int> contents_maximum_height()
		{
			return detail::contents_view::contents_maximum_height();
		}

	private:

		reactive_property<bool> clip_horizontal_;
		reactive_property<bool> clip_vertical_;

		reactive_property<int> offset_x_;
		reactive_property<int> offset_y_;

	};

}

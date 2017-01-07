#pragma once
#include "contents_view.hpp"
#include "view_impl.hpp"

namespace rewin
{

	class fill_view : public detail::view_impl<fill_view, detail::contents_view>
	{
	protected:

		fill_view()
		{
		}

		virtual void initialize() override
		{
			detail::contents_view::initialize();

			minimized_width() = contents_maximum_width();
			minimized_height() = contents_maximum_height();

			*this += contents().as_reactive()
				.subscribe([&](const auto& contents)
			{
				auto rect_h = rect_arranged_horizontal().as_reactive();
				auto rect_v = rect_arranged_vertical().as_reactive();

				for (const auto& content : contents)
				{
					content->rect_arranged_horizontal() = rect_h;
					content->rect_arranged_vertical() = rect_v;
				}
			});
		}

	};

}

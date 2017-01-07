#pragma once
#include "contents_view.hpp"
#include "view_impl.hpp"
#include "margin.hpp"

namespace rewin
{

	class margin_view : public detail::view_impl<margin_view, detail::contents_view>
	{
	protected:

		margin_view()
		{
		}

		virtual void initialize() override
		{
			detail::contents_view::initialize();

			minimized_width() =
				combine(
					contents_maximum_width(),
					margin().as_reactive())
				.map([](const auto& t)
			{
				int width = std::get<0>(t);
				rewin::margin m = std::get<1>(t);

				return width + m.left + m.right;
			});

			minimized_height() =
				combine(
					contents_maximum_height(),
					margin().as_reactive())
				.map([](const auto& t)
			{
				int height = std::get<0>(t);
				rewin::margin m = std::get<1>(t);

				return height + m.top + m.bottom;
			});

			*this +=
				combine(
					contents().as_reactive(),
					margin().as_reactive())
				.subscribe([&](const auto& t)
			{
				const auto& contents = std::get<0>(t);
				rewin::margin m = std::get<1>(t);

				auto rect_h = rect_arranged_horizontal().as_reactive()
					.map([=](const WTL::CRect& rect)
				{
					WTL::CRect r = rect;
					r.left += m.left;
					r.right -= m.left + m.right;
					return r;
				});

				auto rect_v = rect_arranged_vertical().as_reactive()
					.map([=](const WTL::CRect& rect)
				{
					WTL::CRect r = rect;
					r.left += m.left;
					r.top += m.top;
					r.right -= m.left + m.right;
					r.bottom -= m.top + m.bottom;
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

		reactive_property<rewin::margin>& margin()
		{
			return margin_;
		}

	private:

		reactive_property<rewin::margin> margin_;

	};

}

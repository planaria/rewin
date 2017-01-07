#pragma once
#include "contents_view.hpp"
#include "view_impl.hpp"

namespace rewin
{

	class limit_view : public detail::view_impl<limit_view, detail::contents_view>
	{
	protected:

		limit_view()
		{
		}

		virtual void initialize() override
		{
			detail::contents_view::initialize();

			minimized_width() =
				combine(
					contents_maximum_width(),
					minimum_size().as_reactive())
				.map([](const auto& t)
			{
				int value = std::get<0>(t);
				int minimum = std::get<1>(t).cx;
				return std::max(minimum, value);
			});

			minimized_height() =
				combine(
					contents_maximum_height(),
					minimum_size().as_reactive())
				.map([](const auto& t)
			{
				int value = std::get<0>(t);
				int minimum = std::get<1>(t).cy;
				return std::max(minimum, value);
			});

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

	public:

		reactive_property<WTL::CSize>& minimum_size()
		{
			return minimum_size_;
		}

	private:

		reactive_property<WTL::CSize> minimum_size_ = WTL::CSize(0, 0);

	};

}

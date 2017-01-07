#pragma once
#include "stack_view_base.hpp"
#include "view_impl.hpp"

namespace rewin
{

	class stack_view : public detail::view_impl<stack_view, detail::stack_view_base>
	{
	protected:

		stack_view()
		{
		}

		virtual void initialize() override
		{
			detail::stack_view_base::initialize();

			*this +=
				combine(
					contents().as_reactive(),
					direction().as_reactive())
				.subscribe([&](const auto& t1)
			{
				const auto& contents = std::get<0>(t1);
				rewin::direction direction = std::get<1>(t1);

				switch (direction)
				{
				case rewin::direction::left:
				{
					reactive<int> sum = 0;

					for (const auto& content : contents)
					{
						content->rect_arranged_horizontal() =
							combine(
								rect_arranged_horizontal().as_reactive(),
								content->minimized_width().as_reactive(),
								sum)
							.map([](const auto& t2)
						{
							const WTL::CRect& rect = std::get<0>(t2);
							int minimized = std::get<1>(t2);
							int sum = std::get<2>(t2);

							WTL::CRect r = rect;
							r.left += sum;
							r.right = r.left + minimized;
							return r;
						});

						sum = combine(sum, content->minimized_width().as_reactive())
							.map([](const auto& t2)
						{
							return std::get<0>(t2) + std::get<1>(t2);
						});
					}

					break;
				}
				case rewin::direction::right:
				{
					reactive<int> sum = 0;

					for (const auto& content : contents)
					{
						content->rect_arranged_horizontal() =
							combine(
								rect_arranged_horizontal().as_reactive(),
								content->minimized_width().as_reactive(),
								sum)
							.map([](const auto& t2)
						{
							const WTL::CRect& rect = std::get<0>(t2);
							int minimized = std::get<1>(t2);
							int sum = std::get<2>(t2);

							WTL::CRect r = rect;
							r.right -= sum;
							r.left = r.right - minimized;
							return r;
						});

						sum = combine(sum, content->minimized_width().as_reactive())
							.map([](const auto& t2)
						{
							return std::get<0>(t2) + std::get<1>(t2);
						});
					}

					break;
				}
				case rewin::direction::top:
				{
					reactive<int> sum = 0;

					for (const auto& content : contents)
					{
						content->rect_arranged_vertical() =
							combine(
								rect_arranged_vertical().as_reactive(),
								content->minimized_height().as_reactive(),
								sum)
							.map([](const auto& t2)
						{
							const WTL::CRect& rect = std::get<0>(t2);
							int minimized = std::get<1>(t2);
							int sum = std::get<2>(t2);

							WTL::CRect r = rect;
							r.top += sum;
							r.bottom = r.top + minimized;
							return r;
						});

						sum = combine(sum, content->minimized_height().as_reactive())
							.map([](const auto& t2)
						{
							return std::get<0>(t2) + std::get<1>(t2);
						});
					}

					break;
				}
				case rewin::direction::bottom:
				{
					reactive<int> sum = 0;

					for (const auto& content : contents)
					{
						content->rect_arranged_vertical() =
							combine(
								rect_arranged_vertical().as_reactive(),
								content->minimized_height().as_reactive(),
								sum)
							.map([](const auto& t2)
						{
							const WTL::CRect& rect = std::get<0>(t2);
							int minimized = std::get<1>(t2);
							int sum = std::get<2>(t2);

							WTL::CRect r = rect;
							r.bottom -= sum;
							r.top = r.bottom - minimized;
							return r;
						});

						sum = combine(sum, content->minimized_height().as_reactive())
							.map([](const auto& t2)
						{
							return std::get<0>(t2) + std::get<1>(t2);
						});
					}

					break;
				}
				}
			});
		}

	};

}

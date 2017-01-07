#pragma once
#include "contents_view.hpp"
#include "direction.hpp"

namespace rewin
{
	namespace detail
	{

		class stack_view_base : public contents_view
		{
		protected:

			stack_view_base()
			{
			}

			virtual void initialize() override
			{
				contents_view::initialize();

				minimized_width() =
					combine(
						contents().as_reactive(),
						direction().as_reactive())
					.map([&](const auto& t1)
				{
					const auto& contents = std::get<0>(t1);
					rewin::direction direction = std::get<1>(t1);

					reactive<int> width = 0;

					switch (direction)
					{
					case rewin::direction::left:
					case rewin::direction::right:
						for (const auto& content : contents)
						{
							width = combine(width, content->minimized_width().as_reactive())
								.map([](const auto& t2)
							{
								return std::get<0>(t2) + std::get<1>(t2);
							});
						}
						break;
					case rewin::direction::top:
					case rewin::direction::bottom:
						for (const auto& content : contents)
						{
							width = combine(width, content->minimized_width().as_reactive())
								.map([](const auto& t2)
							{
								return std::max(std::get<0>(t2), std::get<1>(t2));
							});
						}
						break;
					}

					return width;
				}).switch_on_next();

				minimized_height() =
					combine(
						contents().as_reactive(),
						direction().as_reactive())
					.map([&](const auto& t1)
				{
					const auto& contents = std::get<0>(t1);
					rewin::direction direction = std::get<1>(t1);

					reactive<int> height = 0;

					switch (direction)
					{
					case rewin::direction::left:
					case rewin::direction::right:
						for (const auto& content : contents)
						{
							height = combine(height, content->minimized_height().as_reactive())
								.map([](const auto& t2)
							{
								return std::max(std::get<0>(t2), std::get<1>(t2));
							});
						}
						break;
					case rewin::direction::top:
					case rewin::direction::bottom:
						for (const auto& content : contents)
						{
							height = combine(height, content->minimized_height().as_reactive())
								.map([](const auto& t2)
							{
								return std::get<0>(t2) + std::get<1>(t2);
							});
						}
						break;
					}

					return height;
				}).switch_on_next();

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
						for (const auto& content : contents)
						{
							content->rect_arranged_vertical() =
								combine(
									rect_arranged_vertical().as_reactive(),
									content->rect_arranged_horizontal().as_reactive())
								.map([](const auto& t2)
							{
								const WTL::CRect& r1 = std::get<0>(t2);
								const WTL::CRect& r2 = std::get<1>(t2);
								return WTL::CRect(r2.left, r1.top, r2.right, r1.bottom);
							});
						}

						break;
					}
					case rewin::direction::right:
					{
						for (const auto& content : contents)
						{
							content->rect_arranged_vertical() =
								combine(
									rect_arranged_vertical().as_reactive(),
									content->rect_arranged_horizontal().as_reactive())
								.map([](const auto& t2)
							{
								const WTL::CRect& r1 = std::get<0>(t2);
								const WTL::CRect& r2 = std::get<1>(t2);
								return WTL::CRect(r2.left, r1.top, r2.right, r1.bottom);
							});
						}

						break;
					}
					case rewin::direction::top:
					{
						for (const auto& content : contents)
							content->rect_arranged_horizontal() = rect_arranged_horizontal().as_reactive();

						break;
					}
					case rewin::direction::bottom:
					{
						for (const auto& content : contents)
							content->rect_arranged_horizontal() = rect_arranged_horizontal().as_reactive();

						break;
					}
					}
				});
			}

		public:

			reactive_property<rewin::direction>& direction()
			{
				return direction_;
			}

		private:

			reactive_property<rewin::direction> direction_ = rewin::direction::top;

		};

	}
}

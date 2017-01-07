#pragma once
#include "view.hpp"

namespace rewin
{
	namespace detail
	{

		class contents_view : public view
		{
		protected:

			contents_view()
			{
			}

			virtual void initialize() override
			{
				view::initialize();

				*this += contents().as_reactive()
					.subscribe([&](const auto& contents)
				{
					clear_children();

					for (const auto& content : contents)
						add_child(*content);
				});
			}

		public:

			virtual void on_paint(WTL::CDCHandle dc) override
			{
				for (const auto& content : contents().value())
					content->on_paint(dc);
			}

			reactive_property<std::vector<std::shared_ptr<view>>>& contents()
			{
				return contents_;
			}

		protected:

			reactive<int> contents_maximum_width()
			{
				return
					contents().as_reactive()
					.map([](const auto& contents)
				{
					reactive<int> width = 0;

					for (const auto& content : contents)
					{
						width = combine(width, content->minimized_width().as_reactive())
							.map([](const auto& t2)
						{
							return std::max(std::get<0>(t2), std::get<1>(t2));
						});
					}

					return width;
				}).switch_on_next();
			}

			reactive<int> contents_maximum_height()
			{
				return
					contents().as_reactive()
					.map([](const auto& contents)
				{
					reactive<int> height = 0;

					for (const auto& content : contents)
					{
						height = combine(height, content->minimized_height().as_reactive())
							.map([](const auto& t2)
						{
							return std::max(std::get<0>(t2), std::get<1>(t2));
						});
					}

					return height;
				}).switch_on_next();
			}

		private:

			reactive_property<std::vector<std::shared_ptr<view>>> contents_;

		};

	}
}

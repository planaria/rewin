#pragma once
#include "view.hpp"
#include "view_impl.hpp"
#include "brush.hpp"

namespace rewin
{

	class rectangle_view : public detail::view_impl<rectangle_view, view>
	{
	protected:

		rectangle_view()
		{
		}

		virtual void initialize() override
		{
			view::initialize();

			*this +=
				combine(
					rect_arranged_vertical().as_reactive(),
					brush().as_reactive())
				.subscribe([&](const auto&)
			{
				auto window = owner_window().value().lock();
				if (window)
					window->Invalidate();
			});
		}

	public:

		virtual void on_paint(WTL::CDCHandle dc) override
		{
			if (brush_.value())
				brush_.value()->fill(dc, &rect_arranged_vertical().value());
		}

		reactive_property<std::shared_ptr<rewin::brush>>& brush()
		{
			return brush_;
		}

	private:

		reactive_property<std::shared_ptr<rewin::brush>> brush_;

	};

}

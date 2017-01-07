#pragma once
#include "observation_base.hpp"

namespace rewin
{

	template <class Tag>
	class observable : public detail::observable_base
	{
	};

	template <class Tag>
	class observer : public detail::observer_base
	{
	protected:

		typedef Tag tag_type;

		virtual void on_notify() override
		{
			on_notify(tag_type());
		}

		virtual void on_notify(tag_type) = 0;

	};

	template <class Observable, class Observer>
	class observation : public detail::observation_base
	{
	public:

		typedef Observable observable_type;
		typedef Observer observer_type;

		observation()
		{
		}

		observation(observable_type& observable, observer_type& observer)
			: detail::observation_base(observable, observer)
		{
		}

		observable_type& observable()
		{
			return static_cast<observable_type&>(detail::observation_base::observable());
		}

		observer_type& observer()
		{
			return static_cast<observer_type&>(detail::observation_base::observer());
		}

		void link(observable_type& observable, observer_type& observer)
		{
			detail::observation_base::link(observable, observer);
		}

	};

}

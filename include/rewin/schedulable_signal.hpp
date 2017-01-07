#pragma once
#include "schedulable.hpp"

namespace rewin
{

	class schedulable_signal : public schedulable
	{
	public:

		virtual void on_invoke() override
		{
			signal();
		}

		boost::signals2::signal<void()> signal;

	};

}

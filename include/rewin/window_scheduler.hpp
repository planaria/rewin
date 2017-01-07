#pragma once
#include "window_scheduler_impl.hpp"

namespace rewin
{

	class window_scheduler : boost::noncopyable
	{
	public:

		typedef std::chrono::high_resolution_clock clock_type;
		typedef clock_type::time_point time_point;

		window_scheduler()
		{
			REWIN_CHECK_WIN32(impl_.Create(NULL));
		}

		time_point now() const
		{
			return clock_type::now();
		}

		void schedule(const std::weak_ptr<schedulable>& s)
		{
			impl_.schedule(s);
		}

		void schedule(const std::weak_ptr<schedulable>& s, time_point when)
		{
			impl_.schedule(s, when);
		}

		static window_scheduler& singleton()
		{
			thread_local window_scheduler instance;
			return instance;
		}

	private:

		detail::window_scheduler_impl impl_;

	};

}

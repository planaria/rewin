#pragma once
#include "reactive_node_combine.hpp"
#include "reactive_node_timer.hpp"

namespace rewin
{

	template <class... Args>
	reactive<std::tuple<Args...>> combine(const reactive<Args>&... args)
	{
		return reactive<std::tuple<Args...>>(std::make_shared<detail::reactive_node_combine<Args...>>(args.root()...));
	}

	template <class Scheduler>
	reactive<std::size_t> timer(Scheduler& scheduler, typename Scheduler::clock_type::duration duration)
	{
		return reactive<std::size_t>(std::make_shared<detail::reactive_node_timer<Scheduler>>(scheduler, duration));
	}

	inline reactive<std::size_t> timer(window_scheduler::clock_type::duration duration)
	{
		return reactive<std::size_t>(std::make_shared<detail::reactive_node_timer<window_scheduler>>(window_scheduler::singleton(), duration));
	}

}

#pragma once
#include "reactive_node.hpp"
#include "schedulable.hpp"

namespace rewin
{
	namespace detail
	{

		template <class Scheduler>
		class reactive_node_timer
			: public reactive_node<std::size_t>
		{
		public:

			typedef Scheduler scheduler_type;
			typedef typename scheduler_type::clock_type clock_type;
			typedef typename clock_type::duration duration;

			reactive_node_timer(scheduler_type& scheduler, duration duration)
				: scheduler_(scheduler)
				, duration_(duration)
			{
				signal_->signal.connect(std::bind(&reactive_node_timer::on_invoke, this));
				schedule();
			}

		protected:

			virtual void on_update() override
			{
			}

		private:

			void on_invoke()
			{
				++value_raw();
				invalidate();

				schedule();
			}

			void schedule()
			{
				scheduler_.schedule(signal_, scheduler_.now() + duration_);
			}

			scheduler_type& scheduler_;
			duration duration_;

			std::shared_ptr<schedulable_signal> signal_ = std::make_shared<schedulable_signal>();

		};

	}
}

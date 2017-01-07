#pragma once

namespace rewin
{
	namespace detail
	{

		typedef boost::intrusive::list_base_hook<
			boost::intrusive::tag<struct observable_hook_tag>,
			boost::intrusive::link_mode<boost::intrusive::auto_unlink>
		> observable_hook;

		typedef boost::intrusive::list_base_hook<
			boost::intrusive::tag<struct observer_hook_tag>,
			boost::intrusive::link_mode<boost::intrusive::auto_unlink>
		> observer_hook;

		typedef boost::intrusive::list_base_hook<
			boost::intrusive::tag<struct notification_hook_tag>,
			boost::intrusive::link_mode<boost::intrusive::auto_unlink>
		> notification_hook;

		class observation_base;

		class observable_base
		{
		protected:

			observable_base();

			~observable_base();

			void notify();

		private:

			friend class observation_base;

			boost::intrusive::list<
				observation_base,
				boost::intrusive::base_hook<observable_hook>,
				boost::intrusive::constant_time_size<false>
			> observations_;

		};

		class observer_base : notification_hook
		{
		protected:

			observer_base();

			~observer_base();

			virtual void on_notify() = 0;

		private:

			friend class observation_base;
			friend class observable_base;

			boost::intrusive::list<
				observation_base,
				boost::intrusive::base_hook<observer_hook>,
				boost::intrusive::constant_time_size<false>
			> observations_;

		};

		class observation_base
			: public observable_hook
			, public observer_hook
			, boost::noncopyable
		{
		public:

			observation_base()
			{
			}

			observation_base(observable_base& observable, observer_base& observer)
			{
				link(observable, observer);
			}

			observation_base(observation_base&& other)
			{
				if (other.is_linked())
				{
					link(other.observable(), other.observer());
					other.unlink();
				}
			}

			observable_base& observable() const
			{
				BOOST_ASSERT(is_linked());
				return *observable_;
			}

			observer_base& observer() const
			{
				BOOST_ASSERT(is_linked());
				return *observer_;
			}

			bool is_linked() const
			{
				BOOST_ASSERT(!!observable_ == !!observer_);
				return !!observable_;
			}

			void link(observable_base& observable, observer_base& observer)
			{
				unlink();

				observable.observations_.push_back(*this);
				observer.observations_.push_back(*this);
				observable_ = &observable;
				observer_ = &observer;
			}

			void unlink()
			{
				if (!is_linked())
					return;

				static_cast<observable_hook&>(*this).unlink();
				static_cast<observer_hook&>(*this).unlink();
				observable_ = nullptr;
				observer_ = nullptr;
			}

			observation_base& operator =(observation_base&& other)
			{
				if (other.is_linked())
				{
					link(other.observable(), other.observer());
					other.unlink();
				}
				else
				{
					unlink();
				}
			}

		private:

			observable_base* observable_ = nullptr;
			observer_base* observer_ = nullptr;

		};

		inline observable_base::observable_base()
		{
		}

		inline observable_base::~observable_base()
		{
			while (!observations_.empty())
				observations_.front().unlink();
		}

		inline void observable_base::notify()
		{
			boost::intrusive::list<
				notification_hook,
				boost::intrusive::base_hook<notification_hook>,
				boost::intrusive::constant_time_size<false>
			> notifications;

			for (auto& o : observations_)
			{
				static_cast<notification_hook&>(o.observer()).unlink();
				notifications.push_back(o.observer());
			}

			while (!notifications.empty())
			{
				auto& front = notifications.front();
				notifications.erase(notifications.iterator_to(front));
				static_cast<observer_base&>(front).on_notify();
			}
		}

		inline observer_base::observer_base()
		{
		}

		inline observer_base::~observer_base()
		{
			while (!observations_.empty())
				observations_.front().unlink();
		}

	}
}

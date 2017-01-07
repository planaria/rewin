#pragma once
#include "reactive_subscription.hpp"

namespace rewin
{

	class unsubscriber : boost::noncopyable
	{
	public:

		unsubscriber& operator +=(std::unique_ptr<reactive_subscription_base> subscription)
		{
			subscriptions_.push_back(std::move(subscription));
			return *this;
		}

	private:

		std::vector<std::unique_ptr<reactive_subscription_base>> subscriptions_;

	};

}

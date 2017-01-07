#pragma once
#include "reactive_node.hpp"

namespace rewin
{

	class reactive_subscription_base : boost::noncopyable
	{
	public:

		virtual ~reactive_subscription_base()
		{
		}

	protected:

		reactive_subscription_base()
		{
		}

	};

	struct reactive_subscription_tag {};

	template <class T>
	class reactive_subscription
		: public reactive_subscription_base
		, observer<reactive_subscription_tag>
	{
	public:

		typedef T value_type;
		typedef detail::reactive_node<value_type> node_type;

		reactive_subscription(const std::shared_ptr<node_type>& node, const std::function<void(const value_type&)>& function)
			: node_(node)
			, function_(function)
			, observation_(*node, static_cast<observer<reactive_subscription_tag>&>(*this))
		{
			invoke();
		}

	protected:

		virtual void on_notify(reactive_subscription_tag) override
		{
			invoke();
		}

	private:

		void invoke()
		{
			function_(node_->value());
		}

		std::shared_ptr<node_type> node_;
		std::function<void(const value_type&)> function_;
		detail::observation_base observation_;

	};

}

#pragma once
#include "reactive.hpp"
#include "reactive_node_variable.hpp"

namespace rewin
{

	template <class T>
	class reactive_property : boost::noncopyable
	{
	public:

		typedef T value_type;

	private:

		typedef detail::reactive_node_variable<value_type> node_type;

	public:

		reactive_property()
			: node_(std::make_shared<node_type>())
		{
		}

		reactive_property(const value_type& value)
			: node_(std::make_shared<node_type>(value))
		{
		}

		reactive_property(value_type&& value)
			: node_(std::make_shared<node_type>(std::move(value)))
		{
		}

		reactive_property(const reactive<value_type>& value)
			: node_(std::make_shared<node_type>())
		{
			subscribe(value);
		}

		reactive<value_type> as_reactive() const
		{
			return reactive<value_type>(node_);
		}

		const value_type& value() const
		{
			return node_->value();
		}

		void invoke(const value_type& value)
		{
			node_->set_value(value);
		}

		void invoke(value_type&& value)
		{
			node_->set_value(std::move(value));
		}

		reactive_property& operator =(const value_type& value)
		{
			subscription_.reset();
			node_->set_value(value);
			return *this;
		}

		reactive_property& operator =(value_type&& value)
		{
			subscription_.reset();
			node_->set_value(std::move(value));
			return *this;
		}

		reactive_property& operator =(const reactive<value_type>& value)
		{
			subscribe(value);
			return *this;
		}

	private:

		void subscribe(const reactive<value_type>& value)
		{
			subscription_ = value
				.subscribe([&](const value_type& value)
			{
				node_->set_value(value);
			});
		}

		std::shared_ptr<node_type> node_;
		std::unique_ptr<reactive_subscription_base> subscription_;

	};

}

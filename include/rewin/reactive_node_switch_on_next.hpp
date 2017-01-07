#pragma once
#include "reactive_node.hpp"

namespace rewin
{

	template <class T>
	class reactive;
	
	namespace detail
	{

		struct reactive_node_switch_on_next_tag1 {};
		struct reactive_node_switch_on_next_tag2 {};

		template <class T>
		class reactive_node_switch_on_next
			: public reactive_node<T>
			, observer<reactive_node_switch_on_next_tag1>
			, observer<reactive_node_switch_on_next_tag2>
		{
		public:

			typedef T value_type;
			typedef reactive_node<reactive<value_type>> node_type;

			reactive_node_switch_on_next(const std::shared_ptr<node_type>& node)
				: node_(node)
			{
				observation_parent_.link(*node, static_cast<observer<reactive_node_switch_on_next_tag1>&>(*this));
				on_parent_updated();
			}

		protected:

			virtual void on_update() override
			{
				value_raw() = node_->value().value();
			}

			virtual void on_notify(reactive_node_switch_on_next_tag1) override
			{
				on_parent_updated();
				invalidate();
			}

			virtual void on_notify(reactive_node_switch_on_next_tag2) override
			{
				invalidate();
			}

		private:

			void on_parent_updated()
			{
				observation_child_.link(*node_->value().root(), static_cast<observer<reactive_node_switch_on_next_tag2>&>(*this));
			}

			std::shared_ptr<node_type> node_;
			observation_base observation_parent_;
			observation_base observation_child_;

		};

	}
}

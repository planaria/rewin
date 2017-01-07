#pragma once
#include "reactive_node.hpp"

namespace rewin
{
	namespace detail
	{

		template <class T>
		class reactive_node_variable : public reactive_node<T>
		{
		public:

			typedef reactive_node<T> base_type;
			using base_type::value_type;

			reactive_node_variable()
			{
			}

			reactive_node_variable(const value_type& value)
				: base_type(value)
			{
			}

			reactive_node_variable(value_type&& value)
				: base_type(std::move(value))
			{
			}

			void set_value(const value_type& value)
			{
				value_raw() = value;
				notify();
			}

			void set_value(value_type&& value)
			{
				value_raw() = std::move(value);
				notify();
			}

		protected:

			virtual void on_update() override
			{
			}

		};

	}
}

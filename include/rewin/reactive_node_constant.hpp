#pragma once
#include "reactive_node.hpp"

namespace rewin
{
	namespace detail
	{

		template <class T>
		class reactive_node_constant : public reactive_node<T>
		{
		public:

			typedef reactive_node<T> base_type;
			using base_type::value_type;

			reactive_node_constant()
			{
			}

			reactive_node_constant(const value_type& value)
				: base_type(value)
			{
			}

			reactive_node_constant(value_type&& value)
				: base_type(std::move(value))
			{
			}

		protected:

			virtual void on_update() override
			{
			}

		};

	}
}

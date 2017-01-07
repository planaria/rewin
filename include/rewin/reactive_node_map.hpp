#pragma once
#include "reactive_node.hpp"

namespace rewin
{
	namespace detail
	{

		struct reactive_node_map_tag {};

		template <class Source, class Destination>
		class reactive_node_map
			: public reactive_node<Destination>
			, observer<reactive_node_map_tag>
		{
		public:

			typedef Source source_type;
			typedef Destination destination_type;

			typedef std::function<destination_type(const source_type&)> function_type;

			reactive_node_map(const std::shared_ptr<reactive_node<source_type>>& src, const function_type& function)
				: src_(src)
				, function_(function)
				, observation_(*src, *this)
			{
			}

		protected:

			virtual void on_update() override
			{
				value_raw() = function_(src_->value());
			}

			virtual void on_notify(reactive_node_map_tag) override
			{
				invalidate();
			}

		private:

			std::shared_ptr<reactive_node<source_type>> src_;
			function_type function_;
			observation_base observation_;

		};

	}
}

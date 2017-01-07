#pragma once
#include "reactive_node.hpp"

namespace rewin
{
	namespace detail
	{

		struct reactive_node_distinct_tag {};

		template <class T>
		class reactive_node_distinct
			: public reactive_node<T>
			, observer<reactive_node_distinct_tag>
		{
		public:

			typedef T value_type;
			typedef reactive_node<value_type> base_type;

			typedef std::function<bool(const value_type&, const value_type&)> comparer_type;

			reactive_node_distinct(
				const std::shared_ptr<reactive_node<value_type>>& src,
				const comparer_type& comparer)
				: base_type(src->value())
				, src_(src)
				, comparer_(comparer)
				, observation_(*src, *this)
			{
			}

		protected:

			virtual void on_update() override
			{
				value_raw() = src_->value();
			}

			virtual void on_notify(reactive_node_distinct_tag) override
			{
				if (!comparer_(value_raw(), src_->value()))
					invalidate();
			}

		private:

			std::shared_ptr<reactive_node<value_type>> src_;
			comparer_type comparer_;
			observation_base observation_;

		};

	}
}

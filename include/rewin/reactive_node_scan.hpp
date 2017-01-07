#pragma once
#include "reactive_node.hpp"

namespace rewin
{
	namespace detail
	{

		struct reactive_node_scan_tag {};

		template <class Value, class Result>
		class reactive_node_scan
			: public reactive_node<Result>
			, observer<reactive_node_scan_tag>
		{
		public:

			typedef Value value_type;
			typedef Result result_type;
			typedef reactive_node<result_type> base_type;

			typedef std::function<result_type(result_type&, const value_type&)> function_type;

			template <class T>
			reactive_node_scan(
				const std::shared_ptr<reactive_node<value_type>>& src,
				T&& value,
				const function_type& function)
				: base_type(std::forward<T>(value))
				, src_(src)
				, function_(function)
				, observation_(*src, *this)
			{
			}

		protected:

			virtual void on_update() override
			{
				value_raw() = function_(value_raw(), src_->value());
			}

			virtual void on_notify(reactive_node_scan_tag) override
			{
				invalidate();
			}

		private:

			std::shared_ptr<reactive_node<value_type>> src_;
			function_type function_;
			observation_base observation_;

		};

	}
}

#pragma once
#include "reactive_node.hpp"
#include "tuple_util.hpp"

namespace rewin
{
	namespace detail
	{

		struct reactive_node_combine_tag {};

		template <class... Args>
		class reactive_node_combine
			: public reactive_node<std::tuple<Args...>>
			, observer<reactive_node_combine_tag>
		{
		public:

			typedef reactive_node<std::tuple<Args...>> base_type;
			using base_type::value_type;

			explicit reactive_node_combine(const std::shared_ptr<reactive_node<Args>>&... args)
				: args_(args...)
				, observations_{ detail::observation_base(*args, *this)... }
			{
			}

		protected:

			virtual void on_update() override
			{
				value_raw() = transform(args_,
					[](const auto& arg)
				{
					return arg->value();
				});
			}

			virtual void on_notify(reactive_node_combine_tag) override
			{
				invalidate();
			}

		private:

			std::tuple<std::shared_ptr<reactive_node<Args>>...> args_;
			std::array<observation_base, sizeof...(Args)> observations_;

		};

	}
}

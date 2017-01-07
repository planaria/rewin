#pragma once
#include "reactive_node.hpp"
#include "reactive_node_constant.hpp"
#include "reactive_node_map.hpp"
#include "reactive_node_scan.hpp"
#include "reactive_node_distinct.hpp"
#include "reactive_node_switch_on_next.hpp"
#include "reactive_subscription.hpp"

namespace rewin
{

	template <class T>
	class reactive;

	template <class T>
	struct is_reactive : std::false_type
	{
	};

	template <class T>
	struct is_reactive<reactive<T>> : std::true_type
	{
	};

	template <class T>
	struct is_nested_reactive : std::false_type
	{
	};

	template <class T>
	struct is_nested_reactive<reactive<reactive<T>>> : std::true_type
	{
	};

	template <class T>
	class reactive
	{
	public:

		typedef T value_type;

	private:

		typedef detail::reactive_node<value_type> node_type;

	public:

		reactive()
			: root_(std::make_shared<detail::reactive_node_constant<value_type>>())
		{
		}

		reactive(const value_type& value)
			: root_(std::make_shared<detail::reactive_node_constant<value_type>>(value))
		{
		}

		reactive(value_type&& value)
			: root_(std::make_shared<detail::reactive_node_constant<value_type>>(std::move(value)))
		{
		}

		explicit reactive(const std::shared_ptr<node_type>& root)
			: root_(root)
		{
		}

		const value_type& value() const
		{
			return root_->value();
		}

		const std::shared_ptr<node_type>& root() const
		{
			return root_;
		}

		template <class F>
		auto map(F f) const
		{
			typedef typename std::remove_cv<typename std::result_of<F(const value_type&)>::type>::type result_type;
			return reactive<result_type>(std::make_shared<detail::reactive_node_map<value_type, result_type>>(root_, f));
		}

		template <class U, class F>
		auto scan(U&& u, F f) const
		{
			typedef typename std::remove_cv<U>::type result_type;
			typedef typename std::remove_cv<typename std::result_of<F(result_type&, const value_type&)>::type>::type function_result_type;
			static_assert(std::is_same<result_type, function_result_type>::value, "result type mismatch");

			return reactive<result_type>(std::make_shared<detail::reactive_node_scan<value_type, result_type>>(root_, std::forward<U>(u), f));
		}

		template <class Comparer = std::equal_to<value_type>>
		auto distinct(Comparer comparer = Comparer()) const
		{
			typedef typename std::remove_cv<typename std::result_of<Comparer(const value_type&, const value_type&)>::type>::type result_type;
			static_assert(std::is_same<result_type, bool>::value, "result type of comparer must be bool");

			return reactive<value_type>(std::make_shared<detail::reactive_node_distinct<value_type>>(root_, comparer));
		}

		auto switch_on_next() const
		{
			static_assert(is_nested_reactive<reactive>::value, "this must be reactive<reactive<T>>");
			typedef typename value_type::value_type result_type;
			return reactive<result_type>(std::make_shared<detail::reactive_node_switch_on_next<result_type>>(root_));
		}

		auto subscribe(const std::function<void(const value_type&)>& function) const
		{
			return std::make_unique<reactive_subscription<value_type>>(root_, function);
		}

	private:

		std::shared_ptr<node_type> root_;

	};

}

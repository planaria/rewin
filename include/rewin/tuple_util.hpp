#pragma once

namespace rewin
{
	namespace detail
	{

		template <class... Args, class F, std::size_t... Indices>
		auto transform_impl(const std::tuple<Args...>& t, F f, std::index_sequence<Indices...>)
		{
			return std::make_tuple(f(std::get<Indices>(t))...);
		}

	}

	template <class... Args, class F>
	auto transform(const std::tuple<Args...>& t, F f)
	{
		return detail::transform_impl(t, f, std::index_sequence_for<Args...>());
	}

}

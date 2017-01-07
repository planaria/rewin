#include <catch.hpp>
#include <rewin/rewin.hpp>

TEST_CASE("reactive_combine")
{
	rewin::reactive_property<int> x = 123;
	rewin::reactive_property<int> y = 456;

	auto z = combine(x.as_reactive(), y.as_reactive());

	CHECK(z.value() == std::make_tuple(123, 456));

	x = 789;

	CHECK(z.value() == std::make_tuple(789, 456));
}

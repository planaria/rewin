#include <catch.hpp>
#include <rewin/rewin.hpp>

TEST_CASE("reactive_map")
{
	rewin::reactive_property<int> x = 123;
	auto y = x.as_reactive().map([](int val) { return val * val; });

	CHECK(y.value() == 123 * 123);
}

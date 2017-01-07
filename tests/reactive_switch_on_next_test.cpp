#include <catch.hpp>
#include <rewin/rewin.hpp>

TEST_CASE("reactive_switch_on_next")
{
	rewin::reactive_property<int> value1 = 1;
	rewin::reactive_property<int> value2 = 2;

	rewin::reactive_property<rewin::reactive<int>> values = value1.as_reactive();

	auto result = values.as_reactive().switch_on_next();

	CHECK(result.value() == 1);

	value1 = 3;

	CHECK(result.value() == 3);

	values = value2.as_reactive();

	CHECK(result.value() == 2);

	value1 = 4;

	CHECK(result.value() == 2);

	value2 = 5;

	CHECK(result.value() == 5);

	values = value1.as_reactive();

	CHECK(result.value() == 4);
}

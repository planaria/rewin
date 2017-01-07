#include <catch.hpp>
#include <rewin/rewin.hpp>

TEST_CASE("reactive_scan")
{
	rewin::reactive_property<int> x = 123;

	auto values = x.as_reactive().scan(
		std::vector<int>(),
		[](std::vector<int>& values, int value)
	{
		values.push_back(value);
		return std::move(values);
	});

	CHECK(values.value() == (std::vector<int>{ 123 }));

	x = 456;

	CHECK(values.value() == (std::vector<int>{ 123, 456 }));

	x = 789;

	CHECK(values.value() == (std::vector<int>{ 123, 456, 789 }));
}

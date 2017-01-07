#include <catch.hpp>
#include <rewin/rewin.hpp>

TEST_CASE("reactive_subscribe")
{
	rewin::reactive_property<int> x = 123;

	std::vector<int> values;

	auto s = x.as_reactive().subscribe(
		[&](int v)
	{
		values.push_back(v);
	});

	CHECK(values == std::vector<int>{ 123 });

	x = 456;

	CHECK(values == (std::vector<int>{ 123, 456 }));

	s.reset();

	x = 789;

	CHECK(values == (std::vector<int>{ 123, 456 }));
}

#include <catch.hpp>
#include <rewin/rewin.hpp>

TEST_CASE("reactive_distinct")
{
	rewin::reactive_property<int> x = 123;

	std::size_t count = 0;

	rewin::unsubscriber us;

	us += x.as_reactive().distinct()
		.subscribe([&](int value)
	{
		++count;
	});

	CHECK(count == 1);

	x = 456;

	CHECK(count == 2);

	x = 123;

	CHECK(count == 3);

	x = 123;

	CHECK(count == 3);

	x = 789;

	CHECK(count == 4);

	x = 789;

	CHECK(count == 4);

	x = 123;

	CHECK(count == 5);
}

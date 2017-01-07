#pragma once

namespace rewin
{

	struct margin
	{
	public:

		margin()
		{
		}

		explicit margin(int value)
			: left(value)
			, top(value)
			, right(value)
			, bottom(value)
		{
		}

		margin(int horizontal, int vertical)
			: left(horizontal)
			, top(vertical)
			, right(horizontal)
			, bottom(vertical)
		{
		}

		margin(int left_, int top_, int right_, int bottom_)
			: left(left_)
			, top(top_)
			, right(right_)
			, bottom(bottom_)
		{
		}

		int left = 0;
		int top = 0;
		int right = 0;
		int bottom = 0;

	};

}

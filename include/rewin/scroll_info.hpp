#pragma once

namespace rewin
{

	struct scroll_info
		: boost::equality_comparable<scroll_info>
	{

		scroll_info()
		{
		}

		scroll_info(
			int minimum_,
			int maximum_,
			int page_size_,
			int change_step_)
			: minimum(minimum_)
			, maximum(maximum_)
			, page_size(page_size_)
			, change_step(change_step_)
		{
		}

		bool check() const
		{
			if (maximum <= minimum)
				return false;

			if (page_size <= 0)
				return false;

			if (maximum - minimum - page_size <= 0)
				return false;

			return true;
		}

		int limit_value(int value) const
		{
			return std::max(minimum, std::min(maximum - page_size, value));
		}

		int minimum = 0;
		int maximum = 0;
		int page_size = 0;
		int change_step = 0;

	};

	inline bool operator ==(const scroll_info& lhs, const scroll_info& rhs)
	{
		return
			lhs.minimum == rhs.minimum &&
			lhs.maximum == rhs.maximum &&
			lhs.page_size == rhs.page_size &&
			lhs.change_step == rhs.change_step;
	}

}

#pragma once

namespace rewin
{

	struct text_info
		: boost::equality_comparable<text_info>
	{

		text_info()
		{
		}

		text_info(std::wstring text_, std::size_t caret_begin_ = 0, std::size_t caret_end_ = 0)
			: text(std::move(text_))
			, caret_begin(caret_begin_)
			, caret_end(caret_end_)
		{
		}

		std::wstring text;
		std::size_t caret_begin = 0;
		std::size_t caret_end = 0;

	};

	inline bool operator ==(const text_info& lhs, const text_info& rhs)
	{
		return
			lhs.text == rhs.text &&
			lhs.caret_begin == rhs.caret_begin &&
			lhs.caret_end == rhs.caret_end;
	}

}

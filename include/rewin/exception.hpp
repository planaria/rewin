#pragma once

namespace rewin
{

	class exception
		: public std::exception
		, public boost::exception
	{
	public:

		virtual std::wstring message() const = 0;

	};

}

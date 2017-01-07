#pragma once
#include "win32_exception.hpp"

namespace rewin
{

	class ime_context : boost::noncopyable
	{
	public:

		explicit ime_context(HWND window)
			: window_(window)
			, handle_(ImmGetContext(window))
		{
			REWIN_CHECK_WIN32(handle_);
		}

		~ime_context()
		{
			ImmReleaseContext(window_, handle_);
		}

		HIMC handle() const
		{
			return handle_;
		}

	private:

		HWND window_;
		HIMC handle_;

	};

}

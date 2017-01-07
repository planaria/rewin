#pragma once
#include "win32_exception.hpp"

namespace rewin
{

	class message_loop : boost::noncopyable
	{
	public:

		WPARAM run()
		{
			while (true)
			{
				MSG msg;
				BOOL ret = GetMessage(&msg, NULL, 0, 0);
				REWIN_CHECK_WIN32(ret != -1);

				if (!ret)
					return msg.wParam;

				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

	};

}

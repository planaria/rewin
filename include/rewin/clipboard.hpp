#pragma once
#include "win32_exception.hpp"

namespace rewin
{

	class clipboard : boost::noncopyable
	{
	public:

		clipboard()
		{
			REWIN_CHECK_WIN32(OpenClipboard(NULL));
		}

		~clipboard()
		{
			CloseClipboard();
		}

		void set_text(const std::wstring& str)
		{
			HANDLE handle = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, (str.size() + 1) * sizeof(wchar_t));
			REWIN_CHECK_WIN32(handle);

			try
			{
				LPVOID ptr = GlobalLock(handle);
				REWIN_CHECK_WIN32(ptr);

				auto p = static_cast<wchar_t*>(ptr);
				std::copy(str.begin(), str.end(), p);
				p[str.size()] = '\0';

				GlobalUnlock(handle);

				REWIN_CHECK_WIN32(EmptyClipboard());
				REWIN_CHECK_WIN32(SetClipboardData(CF_UNICODETEXT, handle));
			}
			catch (...)
			{
				GlobalFree(handle);
				throw;
			}
		}

		boost::optional<std::wstring> get_text()
		{
			HANDLE handle = GetClipboardData(CF_UNICODETEXT);
			if (!handle)
				return boost::none;

			LPVOID ptr = GlobalLock(handle);
			REWIN_CHECK_WIN32(ptr);

			try
			{
				std::size_t size = GlobalSize(handle);
				REWIN_CHECK_WIN32(size);

				std::wstring str;

				auto p = static_cast<wchar_t*>(ptr);
				str.assign(p, p + size / 2 - 1);

				auto end = str.find(L'\0');
				if (end != std::wstring::npos)
					str.resize(end);

				GlobalUnlock(handle);

				return str;
			}
			catch (...)
			{
				GlobalUnlock(handle);
				throw;
			}
		}

	private:

	};

}

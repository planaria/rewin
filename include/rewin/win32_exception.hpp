#pragma once
#include "exception.hpp"

namespace rewin
{

	class win32_exception : public exception
	{
	public:

		explicit win32_exception(HRESULT hresult)
			: hresult_(hresult)
		{
		}

		virtual const char* what() const override
		{
			return "win32_exception";
		}

		virtual std::wstring message() const override
		{
			LPWSTR p;

			std::wstring msg;
			msg += L"win32_exception";

			if (!FormatMessageW(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				hresult_,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				reinterpret_cast<LPWSTR>(&p),
				0,
				nullptr))
				return msg;

			try
			{
				msg += L"\n";
				msg += p;
			}
			catch (...)
			{
				LocalFree(p);
				throw;
			}

			LocalFree(p);

			return msg;
		}

	private:

		HRESULT hresult_;

	};

}

#define REWIN_THROW_WIN32(x) BOOST_THROW_EXCEPTION(rewin::win32_exception(x))
#define REWIN_CHECK_WIN32(x) do { if (!(x)) REWIN_THROW_WIN32(GetLastError()); } while(false)
#define REWIN_CHECK_HRESULT(x) do { HRESULT hr___ = (x); if (FAILED(hr___)) REWIN_THROW_WIN32(hr___); } while(false)

#pragma once
#include "win32_exception.hpp"

namespace rewin
{

	class brush : boost::noncopyable
	{
	public:

		virtual ~brush()
		{
		}

		virtual void fill(WTL::CDCHandle dc, const WTL::CRect& rect) const = 0;

	private:

	};

	class solid_brush : public brush
	{
	public:

		explicit solid_brush(COLORREF color)
		{
			REWIN_CHECK_WIN32(brush_.CreateSolidBrush(color));
		}

		solid_brush(BYTE r, BYTE g, BYTE b)
		{
			REWIN_CHECK_WIN32(brush_.CreateSolidBrush(RGB(r, g, b)));
		}

		virtual void fill(WTL::CDCHandle dc, const WTL::CRect& rect) const override
		{
			REWIN_CHECK_WIN32(dc.FillRect(&rect, brush_));
		}

	private:

		WTL::CBrush brush_;

	};

}

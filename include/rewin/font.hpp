#pragma once

namespace rewin
{

	class font : boost::noncopyable
	{
	public:

		font(const std::wstring& face_name, LONG height)
		{
			ZeroMemory(&log_font_, sizeof(log_font_));

			log_font_.lfHeight = height;
			wcscpy_s(log_font_.lfFaceName, face_name.c_str());

			REWIN_CHECK_WIN32(font_.CreateFontIndirectW(&log_font_));
		}

		LOGFONTW& log_font()
		{
			return log_font_;
		}

		HFONT handle()
		{
			return font_;
		}

	private:

		LOGFONTW log_font_;
		WTL::CFont font_;

	};

}

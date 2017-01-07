#pragma once
#include "view.hpp"
#include "view_impl.hpp"
#include "font.hpp"
#include "ssa_deleter.hpp"

namespace rewin
{

	class text_view : public detail::view_impl<text_view, view>
	{
	protected:

		text_view()
		{
		}

		virtual void initialize() override
		{
			view::initialize();

			*this +=
				combine(
					rect_arranged_vertical().as_reactive(),
					text().as_reactive(),
					rtl().as_reactive(),
					font().as_reactive(),
					text_color().as_reactive(),
					enable_actual().as_reactive())
				.subscribe([&](const auto&)
			{
				auto window = owner_window().value().lock();
				if (window)
					window->Invalidate();
			});

			auto minimum_size =
				combine(
					text().as_reactive(),
					rtl().as_reactive(),
					font().as_reactive(),
					owner_window().as_reactive())
				.map([](const auto& t)
			{
				const std::wstring& text = std::get<0>(t);
				bool rtl = std::get<1>(t);
				auto font = std::get<2>(t);
				auto window = std::get<3>(t).lock();

				if (!window)
					return WTL::CSize();

				WTL::CClientDC dc(*window);
				REWIN_CHECK_WIN32(dc);

				dc.SelectFont(font->handle());

				WTL::CSize size;

				split_lines(static_cast<HDC>(dc), text, rtl,
					[&](SCRIPT_STRING_ANALYSIS ssa, const WTL::CSize& s)
				{
					size.cx = std::max(size.cx, s.cx);
					size.cy += s.cy;
				});

				return size;
			});

			minimized_width() = minimum_size
				.map([](const auto& size)
			{
				return static_cast<int>(size.cx);
			});

			minimized_height() = minimum_size
				.map([](const auto& size)
			{
				return static_cast<int>(size.cy);
			});
		}

	public:

		virtual void on_paint(WTL::CDCHandle dc) override
		{
			COLORREF color = enable_actual().value() ? text_color().value() : text_disabled_color().value();

			dc.SetTextColor(color);
			dc.SetBkMode(TRANSPARENT);

			dc.SelectFont(font().value()->handle());

			const WTL::CRect& rect = rect_arranged_vertical().value();

			int y = 0;

			split_lines(dc, text().value(), rtl().value(),
				[&](SCRIPT_STRING_ANALYSIS ssa, const WTL::CSize& s)
			{
				if (ssa)
				{
					auto psize = ScriptString_pSize(ssa);
					REWIN_CHECK_WIN32(psize);

					int x = rtl().value() ? (rect.right - psize->cx) : rect.left;
					REWIN_CHECK_HRESULT(ScriptStringOut(ssa, x, rect.top + y, 0, nullptr, 0, 0, FALSE));
				}

				y += s.cy;
			});
		}

		reactive_property<std::wstring>& text()
		{
			return text_;
		}

		reactive_property<bool>& rtl()
		{
			return rtl_;
		}

		reactive_property<std::shared_ptr<rewin::font>>& font()
		{
			return font_;
		}

		reactive_property<COLORREF>& text_color()
		{
			return text_color_;
		}

		reactive_property<COLORREF>& text_disabled_color()
		{
			return text_disabled_color_;
		}

	private:

		template <class Callback>
		static void split_lines(WTL::CDCHandle dc, const std::wstring& str, bool rtl, Callback callback)
		{
			std::size_t begin_index = 0;

			while (true)
			{
				std::size_t end_index = str.find_first_of(L"\r\n", begin_index);
				bool end = end_index == std::wstring::npos;
				if (end)
					end_index = str.size();

				int length = boost::numeric_cast<int>(end_index - begin_index);

				const wchar_t* begin;

				if (length == 0)
				{
					begin = L" ";
					length = 1;
				}
				else
				{
					begin = &str[begin_index];
				}

				DWORD flags = SSA_TAB | SSA_FALLBACK | SSA_GLYPHS;
				if (rtl)
					flags |= SSA_RTL;

				SCRIPT_STRING_ANALYSIS ssa = nullptr;
				REWIN_CHECK_HRESULT(ScriptStringAnalyse(
					dc,
					begin,
					length,
					length * 3 / 2 + 1,
					-1,
					flags,
					0,
					nullptr,
					nullptr,
					nullptr,
					nullptr,
					nullptr,
					&ssa));

				auto ssa_ptr = std::unique_ptr<void, ssa_deleter>(ssa);

				auto psize = ScriptString_pSize(ssa);
				REWIN_CHECK_WIN32(psize);

				callback(ssa, *psize);

				if (end)
					break;

				begin_index = end_index + 2;
			}
		}

		reactive_property<std::wstring> text_;

		reactive_property<bool> rtl_ = false;

		reactive_property<std::shared_ptr<rewin::font>> font_ = std::make_shared<rewin::font>(L"Meiryo", 23);

		reactive_property<COLORREF> text_color_ = GetSysColor(COLOR_BTNTEXT);
		reactive_property<COLORREF> text_disabled_color_ = GetSysColor(COLOR_GRAYTEXT);

	};

}

#pragma once
#include "view.hpp"
#include "view_impl.hpp"
#include "brush.hpp"
#include "font.hpp"
#include "text_info.hpp"
#include "clipboard.hpp"
#include "ime_context.hpp"

namespace rewin
{

	class text_box_view : public detail::view_impl<text_box_view, view>
	{
	protected:

		text_box_view()
		{
		}

		virtual void initialize() override
		{
			view::initialize();

			*this += is_focused()
				.distinct()
				.subscribe([&](bool is_focused)
			{
				if (is_focused)
					start_blink();
				else
					stop_blink();
			});

			*this +=
				combine(
					rect_arranged_vertical().as_reactive(),
					text_info().as_reactive(),
					rtl().as_reactive(),
					font().as_reactive(),
					text_color().as_reactive(),
					caret_brush().as_reactive(),
					enable_actual().as_reactive(),
					caret_blink_.as_reactive())
				.subscribe([&](const auto&)
			{
				auto window = owner_window().value().lock();
				if (window)
					window->Invalidate();
			});

			auto text = text_info().as_reactive()
				.map([](const rewin::text_info& text_info)
			{
				return text_info.text;
			}).distinct();

			analyze_result_ =
				combine(
					text,
					rtl().as_reactive(),
					font().as_reactive(),
					owner_window().as_reactive())
				.map([&](const auto& t)
			{
				const std::wstring& text = std::get<0>(t);
				bool rtl = std::get<1>(t);
				auto font = std::get<2>(t);
				auto window = std::get<3>(t).lock();

				analyze_result result;

				if (!window)
					return result;

				WTL::CClientDC dc(*window);
				REWIN_CHECK_WIN32(dc);

				dc.SelectFont(font->handle());

				result.char_info.resize(text.size());

				int y = 0;

				split_lines(static_cast<HDC>(dc), rewin::text_info(text), rtl,
					[&](SCRIPT_STRING_ANALYSIS ssa, std::size_t begin_index, std::size_t end_index)
				{
					auto psize = ScriptString_pSize(ssa);
					REWIN_CHECK_WIN32(psize);

					result.whole_size.cx = std::max(result.whole_size.cx, psize->cx);
					result.whole_size.cy += psize->cy;

					line_info info;
					info.begin_index = begin_index;
					info.end_index = end_index;
					info.top = y;
					info.size = *psize;

					result.line_info.push_back(info);

					std::size_t length = end_index - begin_index;

					auto attrs = ScriptString_pLogAttr(ssa);

					for (std::size_t i = 0; i < length; ++i)
					{
						char_info& ci = result.char_info[begin_index + i];
						ci.attr = attrs[i];
					}

					bool last = text.size() < end_index + 2;

					if (!last)
					{
						result.char_info[end_index].attr.fCharStop = 1;
						result.char_info[end_index].attr.fWordStop = 1;
						result.char_info[end_index + 1].attr.fCharStop = 0;
						result.char_info[end_index + 1].attr.fWordStop = 0;
					}

					if (!last || length != 0)
					{
						result.char_info[begin_index].attr.fCharStop = 1;
						result.char_info[begin_index].attr.fWordStop = 1;
					}

					y += psize->cy;
				});

				return result;
			});

			minimized_width() = analyze_result_.as_reactive()
				.map([](const analyze_result& ar)
			{
				return static_cast<int>(ar.whole_size.cx);
			});

			minimized_height() = analyze_result_.as_reactive()
				.map([](const analyze_result& ar)
			{
				return static_cast<int>(ar.whole_size.cy);
			});
		}

	public:

		virtual reactive<bool> is_hover_target() override
		{
			return enable_actual().as_reactive();
		}

		virtual reactive<bool> is_focus_target() override
		{
			return enable_actual().as_reactive();
		}

		virtual void on_paint(WTL::CDCHandle dc) override
		{
			dc.SetTextColor(text_color().value());
			dc.SetBkMode(TRANSPARENT);

			dc.SelectFont(font().value()->handle());

			const WTL::CRect& rect = rect_arranged_vertical().value();

			if (enable_actual().value())
				background_brush().value()->fill(dc, rect);
			else
				background_disabled_brush().value()->fill(dc, rect);

			int y = 0;

			split_lines(dc, text_info().value(), rtl().value(),
				[&](SCRIPT_STRING_ANALYSIS ssa, std::size_t begin_index, std::size_t end_index)
			{
				auto psize = ScriptString_pSize(ssa);
				REWIN_CHECK_WIN32(psize);

				std::size_t selection_begin = 0;
				std::size_t selection_end = 0;

				const auto& info = text_info().value();

				std::size_t length = end_index - begin_index + 1;

				if (info.caret_begin > begin_index)
					selection_begin = std::min<std::size_t>(length, info.caret_begin - begin_index);

				if (info.caret_end > begin_index)
					selection_end = std::min<std::size_t>(length, info.caret_end - begin_index);

				if (selection_end < selection_begin)
					std::swap(selection_begin, selection_end);

				boost::optional<std::size_t> caret_index;
				if (begin_index <= info.caret_end && info.caret_end <= end_index)
					caret_index = info.caret_end - begin_index;

				int x = rtl().value() ? (rect.right - psize->cx) : rect.left;

				REWIN_CHECK_HRESULT(ScriptStringOut(
					ssa,
					x,
					rect.top + y,
					0,
					nullptr,
					boost::numeric_cast<int>(selection_begin),
					boost::numeric_cast<int>(selection_end),
					FALSE));

				if (caret_index && caret_blink_.value())
				{
					int caret_x;
					REWIN_CHECK_HRESULT(ScriptStringCPtoX(ssa, boost::numeric_cast<int>(*caret_index), FALSE, &caret_x));

					WTL::CRect r;
					r.left = x + caret_x;
					r.top = rect.top + y;
					r.right = r.left + 2;
					r.bottom = r.top + psize->cy;

					if (rtl().value())
					{
						r.left -= 2;
						r.right -= 2;
					}

					caret_brush().value()->fill(dc, r);
				}

				y += psize->cy;
			});
		}

		virtual bool on_key_down(UINT nChar, UINT nRepCnt, UINT nFlags) override
		{
			if (!enable_actual().value())
				return false;

			bool shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
			bool control = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

			switch (nChar)
			{
			case VK_LEFT:
				if (rtl().value())
					move_caret_forward();
				else
					move_caret_backward();
				return true;
			case VK_RIGHT:
				if (rtl().value())
					move_caret_backward();
				else
					move_caret_forward();
				return true;
			case VK_UP:
				move_caret_up();
				return true;
			case VK_DOWN:
				move_caret_down();
				return true;
			case VK_HOME:
				move_caret_home();
				return true;
			case VK_END:
				move_caret_end();
				return true;
			case VK_RETURN:
				if (allow_enter().value())
				{
					input(L"\r\n");
					return true;
				}
				break;
			case VK_TAB:
				if (allow_tab().value())
				{
					input(L"\t");
					return true;
				}
				break;
			case VK_BACK:
				input_backspace();
				return true;
			case VK_DELETE:
				input_delete();
				return true;
			case L'A':
				if (control)
				{
					select_all();
					return true;
				}
				break;
			case L'X':
				if (control)
				{
					cut();
					return true;
				}
				break;
			case L'C':
				if (control)
				{
					copy();
					return true;
				}
				break;
			case L'V':
				if (control)
				{
					paste();
					return true;
				}
				break;
			}

			return false;
		}

		virtual bool on_char(UINT nChar, UINT nRepCnt, UINT nFlags) override
		{
			if (!enable_actual().value())
				return false;

			if (!std::iswprint(nChar))
				return false;

			input(std::wstring(1, nChar));

			return true;
		}

		virtual bool on_ime_start_composition() override
		{
			if (!enable_actual().value())
				return false;

			auto window = owner_window().value().lock();
			if (!window)
				return false;

			ime_context context(*window);
			
			REWIN_CHECK_WIN32(ImmSetCompositionFontW(context.handle(), &font().value()->log_font()));

			const auto& info = text_info().value();

			WTL::CPoint p = index_to_point(info.caret_end);

			COMPOSITIONFORM cf = { CFS_POINT, p };
			REWIN_CHECK_WIN32(ImmSetCompositionWindow(context.handle(), &cf));

			return true;
		}

		virtual bool on_ime_composition(const std::wstring& result_str) override
		{
			if (!enable_actual().value())
				return false;

			input(result_str);
			return true;
		}

		virtual bool on_query_char_position(WTL::CRect& result) override
		{
			if (!enable_actual().value())
				return false;

			const auto& info = text_info().value();

			WTL::CPoint p = index_to_point(info.caret_end);

			result.left = p.x;
			result.top = p.y;
			result.right = result.left;
			result.bottom = result.top + font().value()->log_font().lfHeight;

			return true;
		}

		virtual bool on_mouse_move(UINT nFlags, WTL::CPoint point) override
		{
			if (!enable_actual().value())
				return false;

			if (drag_)
			{
				auto info = text_info().value();
				info.caret_end = point_to_index(point);

				bool control = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

				if (control)
				{
					if (info.caret_end < info.caret_begin)
					{
						if (info.caret_end != 0)
							info.caret_end = prev_word(info.caret_end);
					}
					else
					{
						if (info.caret_end != info.text.size())
							info.caret_end = next_word(info.caret_end);
					}
				}

				text_info().invoke(std::move(info));
				start_blink();

				return true;
			}

			return false;
		}

		virtual bool on_mouse_button(mouse_button button, mouse_action action, UINT nFlags, WTL::CPoint point) override
		{
			if (!enable_actual().value())
				return false;

			auto window = owner_window().value().lock();
			if (!window)
				return false;

			bool shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
			bool control = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

			if (button == mouse_button::left)
			{
				if ((action == mouse_action::down && control) ||
					action == mouse_action::double_click)
				{
					window->capture_view() = shared_from_this();
					window->focused_view() = shared_from_this();

					auto info = text_info().value();

					info.caret_end = point_to_index(point);

					if (shift)
					{
						if (info.caret_end < info.caret_begin)
						{
							if (info.caret_end != 0)
								info.caret_end = prev_word(info.caret_end);
						}
						else
						{
							if (info.caret_end != info.text.size())
								info.caret_end = next_word(info.caret_end);
						}
					}
					else
					{
						info.caret_begin = info.caret_end;
						if (info.caret_begin != 0)
							info.caret_begin = prev_word(info.caret_begin);

						if (info.caret_end != info.text.size())
							info.caret_end = next_word(info.caret_end);
					}

					text_info().invoke(std::move(info));
					start_blink();
					drag_ = true;
				}
				else if(action == mouse_action::down)
				{
					window->capture_view() = shared_from_this();
					window->focused_view() = shared_from_this();

					auto info = text_info().value();

					info.caret_end = point_to_index(point);

					if (!shift)
						info.caret_begin = info.caret_end;

					text_info().invoke(std::move(info));
					start_blink();
					drag_ = true;
				}
				else if (action == mouse_action::up)
				{
					if (drag_)
					{
						window->capture_view() = std::weak_ptr<view>();
						drag_ = false;
					}
				}

				return true;
			}

			return false;
		}

		reactive_property<rewin::text_info>& text_info()
		{
			return text_info_;
		}

		reactive_property<bool>& rtl()
		{
			return rtl_;
		}

		reactive_property<std::shared_ptr<rewin::font>>& font()
		{
			return font_;
		}

		reactive_property<int>& text_color()
		{
			return text_color_;
		}

		reactive_property<std::shared_ptr<rewin::brush>>& caret_brush()
		{
			return caret_brush_;
		}

		reactive_property<std::shared_ptr<rewin::brush>>& background_brush()
		{
			return background_brush_;
		}

		reactive_property<std::shared_ptr<rewin::brush>>& background_disabled_brush()
		{
			return background_disabled_brush_;
		}

		reactive_property<bool>& allow_enter()
		{
			return allow_enter_;
		}
		
		reactive_property<bool>& allow_tab()
		{
			return allow_tab_;
		}

	private:

		struct line_info
		{
			std::size_t begin_index = 0;
			std::size_t end_index = 0;
			int top = 0;
			WTL::CSize size;
		};

		struct char_info
		{
			SCRIPT_LOGATTR attr = {};
		};

		struct analyze_result
		{
			WTL::CSize whole_size;
			std::vector<line_info> line_info;
			std::vector<char_info> char_info;
		};

		template <class Callback>
		static void split_lines(WTL::CDCHandle dc, const rewin::text_info& text_info, bool rtl, Callback callback)
		{
			std::size_t begin_index = 0;
			std::wstring line;

			while (true)
			{
				std::size_t end_index = text_info.text.find_first_of(L"\r\n", begin_index);
				bool end = end_index == std::wstring::npos;
				if (end)
					end_index = text_info.text.size();

				line = text_info.text.substr(begin_index, end_index - begin_index) + L" ";
				int length = boost::numeric_cast<int>(line.size());

				DWORD flags = SSA_TAB | SSA_BREAK | SSA_FALLBACK | SSA_GLYPHS;
				if (rtl)
					flags |= SSA_RTL;

				SCRIPT_STRING_ANALYSIS ssa = nullptr;
				REWIN_CHECK_HRESULT(ScriptStringAnalyse(
					dc,
					line.c_str(),
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

				callback(ssa, begin_index, end_index);

				if (end)
					break;

				begin_index = end_index + 2;
			}
		}

		void input(const std::wstring& str)
		{
			auto info = text_info().value();

			std::size_t index_min = std::min(info.caret_begin, info.caret_end);
			std::size_t index_max = std::max(info.caret_begin, info.caret_end);

			info.text.replace(info.text.begin() + index_min, info.text.begin() + index_max, str);
			info.caret_begin = info.caret_end = index_min + str.size();

			text_info().invoke(std::move(info));
			start_blink();
		}

		void input_backspace()
		{
			auto info = text_info().value();

			if (info.caret_begin == info.caret_end)
			{
				if (info.caret_begin != 0)
				{
					bool control = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

					std::size_t index_begin = control
						? prev_word(info.caret_begin)
						: prev_char(info.caret_begin);

					info.text.erase(index_begin, info.caret_begin - index_begin);
					info.caret_begin = info.caret_end = index_begin;
				}
			}
			else
			{
				std::size_t index_min = std::min(info.caret_begin, info.caret_end);
				std::size_t index_max = std::max(info.caret_begin, info.caret_end);
				info.text.erase(index_min, index_max - index_min);
				info.caret_begin = info.caret_end = index_min;
			}

			text_info().invoke(std::move(info));
			start_blink();
		}

		void input_delete()
		{
			auto info = text_info().value();

			if (info.caret_begin == info.caret_end)
			{
				if (info.caret_begin != info.text.size())
				{
					bool control = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

					std::size_t end_index = control
						? next_word(info.caret_begin)
						: next_char(info.caret_begin);

					info.text.erase(info.caret_begin, end_index - info.caret_begin);
				}
			}
			else
			{
				std::size_t index_min = std::min(info.caret_begin, info.caret_end);
				std::size_t index_max = std::max(info.caret_begin, info.caret_end);
				info.text.erase(index_min, index_max - index_min);
				info.caret_begin = info.caret_end = index_min;
			}

			text_info().invoke(std::move(info));
			start_blink();
		}

		void move_caret_backward()
		{
			auto info = text_info().value();

			bool shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
			bool control = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

			if (!shift && !control && info.caret_begin != info.caret_end)
			{
				info.caret_begin = info.caret_end = std::min(info.caret_begin, info.caret_end);
			}
			else
			{
				if (info.caret_end != 0)
				{
					info.caret_end = control
						? prev_word(info.caret_end)
						: prev_char(info.caret_end);
				}

				if (!shift)
					info.caret_begin = info.caret_end;
			}

			text_info().invoke(std::move(info));
			start_blink();
		}

		void move_caret_forward()
		{
			auto info = text_info().value();

			bool shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
			bool control = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

			if (!shift && !control && info.caret_begin != info.caret_end)
			{
				info.caret_begin = info.caret_end = std::max(info.caret_begin, info.caret_end);
			}
			else
			{
				if (info.caret_end != info.text.size())
				{
					info.caret_end = control
						? next_word(info.caret_end)
						: next_char(info.caret_end);
				}

				if (!shift)
					info.caret_begin = info.caret_end;
			}

			text_info().invoke(std::move(info));
			start_blink();
		}

		void move_caret_up()
		{
			auto info = text_info().value();
			const auto& ar = analyze_result_.value();

			bool shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
			bool control = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

			std::size_t head = get_head(info.caret_end);

			int x = index_to_x(info.caret_end);

			if (head != 0)
				info.caret_end = x_to_index(prev_char(head), x);
			else
				info.caret_end = 0;

			if (!shift)
				info.caret_begin = info.caret_end;

			text_info().invoke(std::move(info));
			start_blink();
		}

		void move_caret_down()
		{
			auto info = text_info().value();
			const auto& ar = analyze_result_.value();

			bool shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
			bool control = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

			std::size_t tail = get_tail(info.caret_end);

			int x = index_to_x(info.caret_end);

			if (tail != info.text.size())
				info.caret_end = x_to_index(next_char(tail), x);
			else
				info.caret_end = info.text.size();

			if (!shift)
				info.caret_begin = info.caret_end;

			text_info().invoke(std::move(info));
			start_blink();
		}

		void move_caret_home()
		{
			auto info = text_info().value();

			bool shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
			bool control = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

			if (control)
				info.caret_end = 0;
			else
				info.caret_end = get_head(info.caret_end);

			if (!shift)
				info.caret_begin = info.caret_end;

			text_info().invoke(std::move(info));
			start_blink();
		}

		void move_caret_end()
		{
			auto info = text_info().value();

			bool shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
			bool control = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

			if (control)
				info.caret_end = info.text.size();
			else
				info.caret_end = get_tail(info.caret_end);

			if (!shift)
				info.caret_begin = info.caret_end;

			text_info().invoke(std::move(info));
			start_blink();
		}

		void select_all()
		{
			auto info = text_info().value();

			info.caret_begin = 0;
			info.caret_end = info.text.size();

			text_info().invoke(std::move(info));
		}

		void cut()
		{
			auto info = text_info().value();
			if (info.caret_begin == info.caret_end)
				return;

			std::size_t index_min = std::min(info.caret_begin, info.caret_end);
			std::size_t index_max = std::max(info.caret_begin, info.caret_end);
			std::size_t length = index_max - index_min;

			clipboard clip;
			clip.set_text(info.text.substr(index_min, length));

			info.text.erase(index_min, length);
			info.caret_begin = info.caret_end = index_min;

			text_info().invoke(std::move(info));
		}

		void copy()
		{
			auto info = text_info().value();
			if (info.caret_begin == info.caret_end)
				return;

			std::size_t index_min = std::min(info.caret_begin, info.caret_end);
			std::size_t index_max = std::max(info.caret_begin, info.caret_end);
			std::size_t length = index_max - index_min;

			clipboard clip;
			clip.set_text(info.text.substr(index_min, length));

			text_info().invoke(std::move(info));
		}

		void paste()
		{
			clipboard clip;

			auto str = clip.get_text();
			if (str)
				input(*str);
		}

		std::size_t get_head(std::size_t index)
		{
			const auto& info = text_info().value();

			std::size_t head = index;

			while (head != 0)
			{
				std::size_t prev_index = prev_char(head);
				if (info.text[prev_index] == L'\r')
					break;

				head = prev_index;
			}

			return head;
		}

		std::size_t get_tail(std::size_t index)
		{
			const auto& info = text_info().value();

			std::size_t tail = index;

			while (tail != info.text.size())
			{
				tail = next_char(tail);

				if (info.text[tail] == L'\r')
					break;
			}

			return tail;
		}

		std::size_t next_char(std::size_t index)
		{
			const auto& info = text_info().value();
			const auto& ar = analyze_result_.value();

			for (++index; index < info.text.size(); ++index)
			{
				if (ar.char_info[index].attr.fCharStop)
					break;
			}

			return index;
		}

		std::size_t prev_char(std::size_t index)
		{
			const auto& ar = analyze_result_.value();

			for (--index; index != 0; --index)
			{
				if (ar.char_info[index].attr.fCharStop)
					break;
			}

			return index;
		}

		std::size_t next_word(std::size_t index)
		{
			const auto& info = text_info().value();
			const auto& ar = analyze_result_.value();

			for (++index; index < info.text.size(); ++index)
			{
				if (ar.char_info[index].attr.fWordStop)
					break;
			}

			return index;
		}

		std::size_t prev_word(std::size_t index)
		{
			const auto& ar = analyze_result_.value();

			for (--index; index != 0; --index)
			{
				if (ar.char_info[index].attr.fWordStop)
					break;
			}

			return index;
		}

		int index_to_x(std::size_t index)
		{
			auto window = owner_window().value().lock();
			if (!window)
				return 0;

			WTL::CClientDC dc(*window);
			REWIN_CHECK_WIN32(dc);

			dc.SelectFont(font().value()->handle());

			const auto& ar = analyze_result_.value();

			auto it = std::find_if(ar.line_info.begin(), ar.line_info.end(),
				[=](const line_info& info)
			{
				return index <= info.end_index;
			});

			int x = 0;

			if (it != ar.line_info.end() && it->begin_index != it->end_index)
			{
				int ilength = boost::numeric_cast<int>(it->end_index - it->begin_index);
				int iindex = boost::numeric_cast<int>(index - it->begin_index);

				DWORD flags = SSA_TAB | SSA_BREAK | SSA_FALLBACK | SSA_GLYPHS;
				if (rtl().value())
					flags |= SSA_RTL;

				const auto& info = text_info().value();

				SCRIPT_STRING_ANALYSIS ssa = nullptr;
				REWIN_CHECK_HRESULT(ScriptStringAnalyse(
					dc,
					info.text.c_str() + it->begin_index,
					ilength,
					ilength * 3 / 2 + 1,
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

				if (index == it->end_index)
					REWIN_CHECK_HRESULT(ScriptStringCPtoX(ssa, ilength - 1, TRUE, &x));
				else
					REWIN_CHECK_HRESULT(ScriptStringCPtoX(ssa, iindex, FALSE, &x));
			}

			const WTL::CRect& rect = rect_arranged_vertical().value();

			if (rtl().value())
				x += rect.right - it->size.cx;
			else
				x += rect.left;

			return x;
		}

		std::size_t x_to_index(std::size_t base_index, int x)
		{
			auto window = owner_window().value().lock();
			if (!window)
				return 0;

			WTL::CClientDC dc(*window);
			REWIN_CHECK_WIN32(dc);

			dc.SelectFont(font().value()->handle());

			const auto& ar = analyze_result_.value();

			auto it = std::find_if(ar.line_info.begin(), ar.line_info.end(),
				[=](const line_info& info)
			{
				return base_index <= info.end_index;
			});

			if (it == ar.line_info.end())
				return 0;

			if (it->begin_index == it->end_index)
				return it->begin_index;

			int ilength = boost::numeric_cast<int>(it->end_index - it->begin_index);

			DWORD flags = SSA_TAB | SSA_BREAK | SSA_FALLBACK | SSA_GLYPHS;
			if (rtl().value())
				flags |= SSA_RTL;

			const auto& info = text_info().value();

			SCRIPT_STRING_ANALYSIS ssa = nullptr;
			REWIN_CHECK_HRESULT(ScriptStringAnalyse(
				dc,
				info.text.c_str() + it->begin_index,
				ilength,
				ilength * 3 / 2 + 1,
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

			const WTL::CRect& rect = rect_arranged_vertical().value();

			if (rtl().value())
				x -= rect.right - it->size.cx;
			else
				x -= rect.left;

			int index;
			int trailing;

			REWIN_CHECK_HRESULT(ScriptStringXtoCP(ssa, x, &index, &trailing));
			if (!rtl().value() && trailing)
				++index;

			index = std::max(0, index);

			return it->begin_index + index;
		}

		std::size_t point_to_index(const WTL::CPoint& p)
		{
			const auto& info = text_info().value();
			const auto& ar = analyze_result_.value();

			const WTL::CRect& rect = rect_arranged_vertical().value();

			auto it = std::find_if(ar.line_info.begin(), ar.line_info.end(),
				[=](const line_info& info)
			{
				return p.y <= rect.top + info.top + info.size.cy;
			});

			if (it == ar.line_info.end())
			{
				if (ar.line_info.empty())
					return 0;

				--it;
			}

			return x_to_index(it->begin_index, p.x);
		}

		WTL::CPoint index_to_point(std::size_t index)
		{
			const auto& info = text_info().value();
			const auto& ar = analyze_result_.value();

			auto it = std::find_if(ar.line_info.begin(), ar.line_info.end(),
				[=](const line_info& info)
			{
				return index <= info.end_index;
			});

			if (it == ar.line_info.end())
				return WTL::CPoint();

			const WTL::CRect& rect = rect_arranged_vertical().value();

			WTL::CPoint p;
			p.x = index_to_x(index);
			p.y = rect.top + it->top;

			return p;
		}

		void start_blink()
		{
			caret_blink_ =
				timer(std::chrono::milliseconds(GetCaretBlinkTime()))
				.map([](std::size_t count)
			{
				return count % 2 == 0;
			});
		}

		void stop_blink()
		{
			caret_blink_ = false;
		}

		reactive_property<rewin::text_info> text_info_;

		reactive_property<bool> rtl_ = false;

		reactive_property<std::shared_ptr<rewin::font>> font_ = std::make_shared<rewin::font>(L"Meiryo", 23);
		reactive_property<int> text_color_;
		reactive_property<std::shared_ptr<rewin::brush>> caret_brush_ = std::make_shared<solid_brush>(0x00, 0x00, 0x00);
		reactive_property<std::shared_ptr<rewin::brush>> background_brush_ = std::make_shared<solid_brush>(0xff, 0xff, 0xff);
		reactive_property<std::shared_ptr<rewin::brush>> background_disabled_brush_ = std::make_shared<solid_brush>(0xcc, 0xcc, 0xcc);

		reactive_property<bool> allow_enter_;
		reactive_property<bool> allow_tab_;

		reactive_property<analyze_result> analyze_result_;

		reactive_property<bool> caret_blink_;

		bool drag_ = false;

	};

}

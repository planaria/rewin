#pragma once
#include "view.hpp"
#include "fill_view.hpp"
#include "brush.hpp"
#include "unsubscriber.hpp"
#include "wtl_util.hpp"
#include "ime_context.hpp"
#include "win32_exception.hpp"
#include "view_iterator.hpp"

namespace rewin
{

	typedef ATL::CWinTraits<
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		WS_EX_APPWINDOW | WS_EX_WINDOWEDGE
	> window_traits;

	class window
		: public ATL::CWindowImpl<window, ATL::CWindow, window_traits>
		, public std::enable_shared_from_this<window>
		, unsubscriber
	{
	public:

		DECLARE_WND_CLASS(_T("rewin window"));

		BEGIN_MSG_MAP(window)
			MSG_WM_SHOWWINDOW(OnShowWindow)
			MSG_WM_CLOSE(OnClose)
			MSG_WM_SIZE(OnSize)
			MSG_WM_PAINT(OnPaint)
			MSG_WM_ERASEBKGND(OnEraseBkgnd)
			MSG_WM_KEYDOWN(OnKeyDown)
			MSG_WM_KEYUP(OnKeyUp)
			MSG_WM_CHAR(OnChar)
			MESSAGE_HANDLER(WM_IME_NOTIFY, OnImeNotify)
			MESSAGE_HANDLER(WM_IME_REQUEST, OnImeRequest)
			MESSAGE_HANDLER(WM_IME_STARTCOMPOSITION, OnImeStartComposition)
			MESSAGE_HANDLER(WM_IME_COMPOSITION, OnImeComposition)
			MESSAGE_HANDLER(WM_IME_CHAR, OnImeChar)
			MSG_WM_SETFOCUS(OnSetFocus)
			MSG_WM_KILLFOCUS(OnKillFocus)
			MSG_WM_MOUSEMOVE(OnMouseMove)
			MSG_WM_MOUSEWHEEL(OnMouseWheel)
			MSG_WM_MOUSELEAVE(OnMouseLeave)
			MSG_WM_LBUTTONDOWN(OnLButtonDown)
			MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
			MSG_WM_LBUTTONUP(OnLButtonUp)
		END_MSG_MAP()

	protected:

		window()
		{
		}

		void initialize()
		{
			REWIN_CHECK_WIN32(Create(NULL));

			SetFocus();

			*this += visible().as_reactive().subscribe(
				[&](bool visible)
			{
				ShowWindow(visible ? SW_NORMAL : SW_HIDE);
			});

			*this += size().as_reactive().subscribe(
				[&](const WTL::CSize& size)
			{
				REWIN_CHECK_WIN32(ResizeClient(size.cx, size.cy));
			});

			*this += title().as_reactive().subscribe(
				[&](const std::wstring& title)
			{
				REWIN_CHECK_WIN32(SetWindowTextW(title.c_str()));
			});

			contents_view_->owner_window_ = this->shared_from_this();

			contents_view_->rect_arranged_horizontal() =
				combine(
					contents_view_->minimized_width().as_reactive(),
					size().as_reactive())
				.map([](const auto& t)
			{
				int width = std::get<0>(t);
				const WTL::CSize& size = std::get<1>(t);
				return WTL::CRect(0, 0, std::max<int>(width, size.cx), 0);
			});

			contents_view_->rect_arranged_vertical() =
				combine(
					contents_view_->rect_arranged_horizontal().as_reactive(),
					contents_view_->minimized_height().as_reactive(),
					size().as_reactive())
				.map([](const auto& t)
			{
				const WTL::CRect& rect = std::get<0>(t);
				int height = std::get<1>(t);
				const WTL::CSize& size = std::get<2>(t);

				return WTL::CRect(0, 0, rect.Width(), std::max<int>(height, size.cy));
			});

			*this += focused_view().as_reactive()
				.map([](const std::weak_ptr<view>& view) -> reactive<bool>
			{
				auto v = view.lock();
				if (!v)
					return true;

				return v->is_focus_target();
			})
				.switch_on_next()
				.subscribe([&](bool target)
			{
				if (!target)
					update_focus();
			});

			*this += hovered_view().as_reactive()
				.map([](const std::weak_ptr<view>& view) -> reactive<bool>
			{
				auto v = view.lock();
				if (!v)
					return true;

				return v->is_hover_target();
			})
				.switch_on_next()
				.subscribe([&](bool target)
			{
				if (!target)
					update_hover();
			});

			*this += capture_view().as_reactive()
				.subscribe([&](const std::weak_ptr<view>& view)
			{
				if (view.lock())
					SetCapture();
				else
					REWIN_CHECK_WIN32(ReleaseCapture());
			});
		}

	public:

		~window()
		{
			if (IsWindow())
				DestroyWindow();
		}

		static std::shared_ptr<window> create()
		{
			struct create_helper : public window
			{
			};

			auto p = std::make_shared<create_helper>();
			p->initialize();
			return p;
		}

		reactive_property<bool>& visible()
		{
			return visible_;
		}

		reactive_property<WTL::CSize>& size()
		{
			return size_;
		}

		reactive_property<std::wstring>& title()
		{
			return title_;
		}

		reactive_property<std::shared_ptr<rewin::brush>>& background_brush()
		{
			return background_brush_;
		}

		reactive_property<std::vector<std::shared_ptr<view>>>& contents()
		{
			return contents_view_->contents();
		}

		reactive_property<std::weak_ptr<view>>& focused_view()
		{
			return focused_view_;
		}
		
		reactive_property<std::weak_ptr<view>>& hovered_view()
		{
			return hovered_view_;
		}

		reactive_property<std::weak_ptr<view>>& capture_view()
		{
			return capture_view_;
		}

		const reactive_property<bool>& focused()
		{
			return focused_;
		}

		const reactive_property<boost::optional<WTL::CPoint>>& mouse_position()
		{
			return mouse_position_;
		}

	private:

		void OnShowWindow(BOOL bShow, UINT nStatus)
		{
			visible().invoke(!!bShow);
		}

		void OnClose()
		{
			visible().invoke(false);
		}

		void OnSize(UINT nType, WTL::CSize s)
		{
			size().invoke(s);
		}

		void OnPaint(WTL::CDCHandle dc)
		{
			if (dc)
			{
				WTL::CRect rect;
				REWIN_CHECK_WIN32(GetClientRect(&rect));

				WTL::CMemoryDC memory_dc(dc, rect);
				REWIN_CHECK_WIN32(memory_dc);

				on_paint(static_cast<HDC>(memory_dc));
			}
			else
			{
				WTL::CPaintDC paint_dc(*this);
				REWIN_CHECK_WIN32(paint_dc);

				WTL::CMemoryDC memory_dc(paint_dc, paint_dc.m_ps.rcPaint);
				REWIN_CHECK_WIN32(memory_dc);

				on_paint(static_cast<HDC>(memory_dc));
			}
		}

		void on_paint(WTL::CDCHandle dc)
		{
			if (background_brush_.value())
			{
				WTL::CRect rect;
				REWIN_CHECK_WIN32(GetClientRect(&rect));

				background_brush_.value()->fill(dc, rect);
			}

			contents_view_->on_paint(dc);
		}

		BOOL OnEraseBkgnd(WTL::CDCHandle dc)
		{
			return TRUE;
		}

		void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
		{
			bool handled = raise_key_event([&](view& v)
			{
				return v.on_key_down(nChar, nRepCnt, nFlags);
			});

			if (!handled)
			{
				switch (nChar)
				{
				case VK_TAB:
				{
					detail::view_iterator begin(contents_view_, contents_view_);
					detail::view_iterator current(contents_view_, focused_view().value().lock());
					detail::view_iterator end(contents_view_);

					bool reverse = (GetKeyState(VK_SHIFT) & 0x8000) != 0;

					auto pred = [](view& v)
					{
						return v.is_focus_target().value();
					};

					if (reverse)
					{
						auto reversed_begin = std::make_reverse_iterator(begin);
						auto reversed_current = std::make_reverse_iterator(current);
						auto reversed_end = std::make_reverse_iterator(end);

						auto it = std::find_if(reversed_current, reversed_begin, pred);
						if (it == reversed_begin && reversed_current != reversed_end)
							it = std::find_if(reversed_end, std::prev(reversed_current), pred);
						if (it != reversed_begin)
							focused_view() = it->shared_from_this();
					}
					else
					{
						auto it = end;
						if (current != end)
							it = std::find_if(std::next(current), end, pred);
						if (it == end)
							it = std::find_if(begin, current, pred);
						if (it != end)
							focused_view() = it->shared_from_this();
					}

					break;
				}
				}
			}
		}

		void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
		{
			raise_key_event([&](view& v)
			{
				return v.on_key_up(nChar, nRepCnt, nFlags);
			});
		}

		void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
		{
			raise_key_event([&](view& v)
			{
				return v.on_char(nChar, nRepCnt, nFlags);
			});
		}

		LRESULT OnImeNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			bHandled = FALSE;

			switch (wParam)
			{
			case IMN_SETOPENSTATUS:
			{
				ime_context context(*this);

				COMPOSITIONFORM cf = { CFS_DEFAULT };
				REWIN_CHECK_WIN32(ImmSetCompositionWindow(context.handle(), &cf));

				bHandled = TRUE;
				break;
			}
			}

			return 0;
		}

		LRESULT OnImeRequest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			bHandled = FALSE;

			switch (wParam)
			{
			case IMR_QUERYCHARPOSITION:
			{
				IMECHARPOSITION* pos = reinterpret_cast<IMECHARPOSITION*>(lParam);
				if (!pos)
					break;

				if (pos->dwSize < sizeof(IMECHARPOSITION))
					break;

				WTL::CRect rect;
				if (!raise_key_event([&](view& v) { return v.on_query_char_position(rect); }))
					break;

				REWIN_CHECK_WIN32(ClientToScreen(&rect));

				pos->pt = rect.TopLeft();
				pos->cLineHeight = rect.Height();

				bHandled = TRUE;
				return 1;
			}
			}

			return 0;
		}

		LRESULT OnImeStartComposition(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			bHandled = FALSE;

			if (raise_key_event([](view& v) { return v.on_ime_start_composition(); }))
				return 0;

			ime_context context(*this);
			REWIN_CHECK_WIN32(ImmNotifyIME(context.handle(), NI_COMPOSITIONSTR, CPS_CANCEL, 0));

			bHandled = TRUE;
			return 0;
		}

		LRESULT OnImeComposition(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			bHandled = FALSE;

			std::wstring result_str;

			if (lParam & GCS_RESULTSTR)
			{
				ime_context context(*this);

				LONG length = ImmGetCompositionString(context.handle(), GCS_RESULTSTR, NULL, 0);
				REWIN_CHECK_WIN32(length >= 0);

				if (length == 0 || length % 2 != 0)
					return 0;

				std::vector<wchar_t> buffer(length / 2);
				REWIN_CHECK_WIN32(ImmGetCompositionString(context.handle(), GCS_RESULTSTR, buffer.data(), length) == length);

				result_str.assign(buffer.begin(), buffer.end());
			}

			if (result_str.empty())
				return 0;

			if(!raise_key_event([&](view& v) { return v.on_ime_composition(result_str); }))
				return 0;

			bHandled = TRUE;
			return 0;
		}

		LRESULT OnImeChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			return 0;
		}

		void OnSetFocus(ATL::CWindow wndOld)
		{
			focused_ = true;
		}

		void OnKillFocus(ATL::CWindow wndFocus)
		{
			focused_ = false;
		}

		void OnMouseMove(UINT nFlags, WTL::CPoint point)
		{
			mouse_position_ = point;
			update_hover();

			raise_mouse_event([&](view& v)
			{
				return v.on_mouse_move(nFlags, point);
			});
		}

		BOOL OnMouseWheel(UINT nFlags, short zDelta, WTL::CPoint pt)
		{
			return raise_mouse_event([&](view& v)
			{
				return v.on_mouse_wheel(nFlags, zDelta, pt);
			}) ? TRUE : FALSE;
		}

		void OnMouseLeave()
		{
			mouse_position_ = boost::none;
			update_hover();
		}

		void OnLButtonDown(UINT nFlags, WTL::CPoint point)
		{
			raise_mouse_event([&](view& v)
			{
				return v.on_mouse_button(mouse_button::left, mouse_action::down, nFlags, point);
			});
		}

		void OnLButtonDblClk(UINT nFlags, WTL::CPoint point)
		{
			raise_mouse_event([&](view& v)
			{
				return v.on_mouse_button(mouse_button::left, mouse_action::double_click, nFlags, point);
			});
		}

		void OnLButtonUp(UINT nFlags, WTL::CPoint point)
		{
			raise_mouse_event([&](view& v)
			{
				return v.on_mouse_button(mouse_button::left, mouse_action::up, nFlags, point);
			});
		}

		template <class Callback>
		bool raise_key_event(Callback callback)
		{
			auto v = focused_view().value().lock();

			while (v)
			{
				if (callback(*v))
					return true;

				v = v->parent().value().lock();
			}

			return false;
		}

		template <class Callback>
		bool raise_mouse_event(Callback callback)
		{
			auto v = hovered_view().value().lock();

			while (v)
			{
				if (callback(*v))
					return true;

				v = v->parent().value().lock();
			}

			return false;
		}

		void update_focus()
		{
			auto v = focused_view().value().lock();
			if (v && !v->is_focus_target().value())
				focused_view() = std::weak_ptr<view>();
		}

		void update_hover()
		{
			std::shared_ptr<view> hover_new;

			if (auto capture = capture_view().value().lock())
			{
				if (capture->is_hover_target().value())
					hover_new = capture;
			}
			else
			{
				if (mouse_position().value())
					hover_new = find_hover(contents_view_, *mouse_position().value());
			}

			hovered_view() = hover_new;
		}

		std::shared_ptr<view> find_hover(const std::shared_ptr<view>& view, const WTL::CPoint& p)
		{
			if (!contains(view->rect_arranged_vertical().value(), p))
				return nullptr;

			auto child = view->last_child();

			while (child)
			{
				auto h = find_hover(child, p);
				if (h)
					return h;

				child = child->prev_sibling();
			}

			if (!view->is_hover_target().value())
				return nullptr;

			return view;
		}

		reactive_property<bool> visible_;

		reactive_property<WTL::CSize> size_ = WTL::CSize(800, 600);

		reactive_property<std::wstring> title_;

		reactive_property<std::shared_ptr<rewin::brush>> background_brush_ = std::make_shared<solid_brush>(0xff, 0xff, 0xff);

		std::shared_ptr<fill_view> contents_view_ = fill_view::create();

		reactive_property<std::weak_ptr<view>> focused_view_;
		reactive_property<std::weak_ptr<view>> hovered_view_;
		reactive_property<std::weak_ptr<view>> capture_view_;

		reactive_property<bool> focused_;

		reactive_property<boost::optional<WTL::CPoint>> mouse_position_;

	};

}

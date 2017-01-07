#pragma once
#include "view.hpp"
#include "view_impl.hpp"
#include "command.hpp"
#include "margin_view.hpp"
#include "view_messages.hpp"
#include "win32_exception.hpp"

namespace rewin
{

	class button_view : public detail::view_impl<button_view, view>
	{
	protected:

		button_view()
		{
		}

		virtual void initialize() override
		{
			view::initialize();

			*this +=
				combine(
					rect_arranged_vertical().as_reactive(),
					is_hovered(),
					is_focused(),
					pressing().as_reactive())
				.subscribe([&](const auto&)
			{
				auto window = owner_window().value().lock();
				if (window)
					window->Invalidate();
			});

			int margin_x = GetSystemMetrics(SM_CXEDGE) + GetSystemMetrics(SM_CXFOCUSBORDER);
			int margin_y = GetSystemMetrics(SM_CYEDGE) + GetSystemMetrics(SM_CYFOCUSBORDER);

			margin_view_->margin() = margin(margin_x, margin_y);

			minimized_width() = margin_view_->minimized_width().as_reactive();
			minimized_height() = margin_view_->minimized_height().as_reactive();

			margin_view_->rect_arranged_horizontal() = rect_arranged_horizontal().as_reactive();
			margin_view_->rect_arranged_vertical() = rect_arranged_vertical().as_reactive();

			add_child(*margin_view_);

			enable() = click_command().as_reactive()
				.map([&](const auto& command) -> reactive<bool>
			{
				if (!command)
					return false;

				return command->can_execute(view_message(this->shared_from_this()));
			}).switch_on_next();
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
			WTL::CRect rect = rect_arranged_vertical().value();

			UINT edge = pressing().value() ? EDGE_SUNKEN : EDGE_RAISED;
			REWIN_CHECK_WIN32(dc.DrawEdge(&rect, edge, BF_ADJUST | BF_MIDDLE | BF_RECT));

			if (is_focused().value())
				dc.DrawFocusRect(&rect);

			margin_view_->on_paint(dc);
		}

		virtual bool on_key_down(UINT nChar, UINT nRepCnt, UINT nFlags) override
		{
			if (!enable_actual().value())
				return false;

			switch (nChar)
			{
			case VK_SPACE:
				if (nRepCnt == 1 && !pressing().value())
					on_press();
				return true;
			case VK_RETURN:
				on_click();
				return true;
			case VK_ESCAPE:
				if (pressing().value())
					on_release();
				return true;
			}

			return false;
		}

		virtual bool on_key_up(UINT nChar, UINT nRepCnt, UINT nFlags) override
		{
			if (!enable_actual().value())
				return false;

			switch (nChar)
			{
			case VK_SPACE:
				if (pressing().value())
				{
					on_click();
					on_release();
				}
				return true;
			}

			return false;
		}

		virtual bool on_mouse_button(mouse_button button, mouse_action action, UINT nFlags, WTL::CPoint point) override
		{
			if (!enable_actual().value())
				return false;

			if (button == mouse_button::left)
			{
				switch (action)
				{
				case mouse_action::down:
				case mouse_action::double_click:
					on_press();
					break;
				case mouse_action::up:
					if (pressing().value())
					{
						if (contains(rect_arranged_vertical().value(), point))
							on_click();

						on_release();
					}
					break;
				}
			}

			return false;
		}

		reactive_property<std::vector<std::shared_ptr<view>>>& contents()
		{
			return margin_view_->contents();
		}

		const reactive_property<bool>& pressing()
		{
			return pressing_;
		}

		reactive_property<std::shared_ptr<command<view_message>>>& click_command()
		{
			return click_command_;
		}

	private:

		void on_press()
		{
			auto w = owner_window().value().lock();
			if (!w)
				return;

			w->capture_view() = this->shared_from_this();
			w->focused_view() = this->shared_from_this();
			pressing_ = true;
		}

		void on_release()
		{
			auto w = owner_window().value().lock();
			if (!w)
				return;

			w->capture_view() = std::weak_ptr<view>();
			pressing_ = false;
		}

		void on_click()
		{
			auto cmd = click_command_.value();
			if (cmd)
				cmd->execute(view_message(this->shared_from_this()));
		}

		std::shared_ptr<margin_view> margin_view_ = margin_view::create();

		reactive_property<bool> pressing_;

		reactive_property<std::shared_ptr<command<view_message>>> click_command_;

	};

}

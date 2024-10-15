#include "stdafx.h"
#include <rewin/rewin.hpp>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	try
	{
		auto window = rewin::window::create();
		window->visible() = true;

		auto text_box = rewin::text_box_view::create();
		text_box->allow_enter() = true;

		auto scroll_view = rewin::scroll_view::create();
		scroll_view->direction() = rewin::direction::bottom;
		scroll_view->contents() = { text_box };

		window->contents() = { scroll_view };

		window->title() = text_box->rect_arranged_vertical().as_reactive().map(
			[](const auto& t)
		{
			return boost::lexical_cast<std::wstring>(t.left);
		});

		rewin::unsubscriber us;

		us += window->visible().as_reactive()
			.subscribe([](bool value)
		{
			if (!value)
				PostQuitMessage(0);
		});

		rewin::message_loop loop;
		return static_cast<int>(loop.run());
	}
	catch (rewin::exception& e)
	{
		MessageBox(NULL, e.message().c_str(), L"Error", MB_ICONERROR | MB_OK);
		return E_FAIL;
	}
	catch (std::exception& e)
	{
		MessageBoxA(NULL, e.what(), "Error", MB_ICONERROR | MB_OK);
		return E_FAIL;
	}
}

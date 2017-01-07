#include "stdafx.h"
#include <rewin/rewin.hpp>

#include <boost/format.hpp>

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	try
	{
		auto window = rewin::window::create();
		window->visible() = true;

		auto text_box = rewin::text_box_view::create();
		text_box->allow_enter() = true;

		window->contents() = { text_box };

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

#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <array>
#include <tuple>
#include <string>
#include <queue>
#include <memory>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cwctype>

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _WTL_NO_AUTOMATIC_NAMESPACE

#include <atlbase.h>
#include <atlstr.h>
#include <atlapp.h>

#define _WTL_NO_CSTRING
#include <atlmisc.h>

#include <atlwin.h>
#include <atlframe.h>
#include <atlcrack.h>

#include <imm.h>
#pragma comment(lib, "imm32.lib")

#include <usp10.h>
#pragma comment(lib, "usp10.lib")

#include <boost/cast.hpp>
#include <boost/operators.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/intrusive/list.hpp>
#include <boost/intrusive/list_hook.hpp>
#include <boost/signals2.hpp>
#include <boost/exception/all.hpp>
#include <boost/optional.hpp>
#include <boost/utility.hpp>

#include "window_scheduler.hpp"

#include "schedulable_signal.hpp"

#include "observation.hpp"

#include "reactive.hpp"
#include "reactive_property.hpp"
#include "reactive_functions.hpp"

#include "function_command.hpp"

#include "window.hpp"
#include "message_loop.hpp"

#include "stack_view.hpp"
#include "dock_view.hpp"
#include "margin_view.hpp"
#include "limit_view.hpp"
#include "clip_view.hpp"
#include "rectangle_view.hpp"
#include "text_view.hpp"
#include "text_box_view.hpp"
#include "button_view.hpp"
#include "scroll_bar_view.hpp"
#include "scroll_view.hpp"

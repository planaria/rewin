#pragma once

namespace rewin
{
	namespace detail
	{

		typedef boost::intrusive::list_base_hook<
			boost::intrusive::tag<struct view_view_hook_tag>,
			boost::intrusive::link_mode<boost::intrusive::auto_unlink>
		> view_view_hook;

		//typedef boost::intrusive::list_base_hook<
		//	boost::intrusive::tag<struct view_window_hook_tag>,
		//	boost::intrusive::link_mode<boost::intrusive::auto_unlink>
		//> view_window_hook;

	}
}

#pragma once

namespace rewin
{

	struct ssa_deleter
	{

		void operator ()(SCRIPT_STRING_ANALYSIS ssa) const
		{
			ScriptStringFree(&ssa);
		}

	};

}

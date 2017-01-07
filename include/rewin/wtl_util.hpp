#pragma once

namespace rewin
{

	inline bool contains(const WTL::CRect& rect, const WTL::CPoint& p)
	{
		return
			rect.left <= p.x &&
			p.x < rect.right &&
			rect.top <= p.y &&
			p.y < rect.bottom;
	}

}
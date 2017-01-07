#pragma once

namespace rewin
{

	class view_message
	{
	public:

		explicit view_message(const std::shared_ptr<view>& sender_)
			: sender(sender_)
		{
		}

		std::shared_ptr<view> sender;

	};

}

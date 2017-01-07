#pragma once

namespace rewin
{

	template <class T>
	class command : boost::noncopyable
	{
	public:

		typedef T message_type;

		virtual ~command()
		{
		}

		virtual void execute(const message_type& message) = 0;

		virtual reactive<bool> can_execute(const message_type& message) = 0;

	};

}

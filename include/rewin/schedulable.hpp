#pragma once

namespace rewin
{

	class schedulable : boost::noncopyable
	{
	protected:

		schedulable()
		{
		}

		~schedulable()
		{
		}

	public:

		virtual void on_invoke() = 0;

	};

}

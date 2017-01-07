#pragma once
#include "command.hpp"

namespace rewin
{

	template <class T>
	class function_command : public command<T>
	{
	public:

		typedef T message_type;

		typedef std::function<void(const message_type&)> execute_type;
		typedef std::function<reactive<bool>(const message_type&)> can_execute_type;

		explicit function_command(const execute_type& execute)
			: execute_(execute)
		{
		}

		function_command(const execute_type& execute, const can_execute_type& can_execute)
			: execute_(execute)
			, can_execute_(can_execute)
		{
		}

		virtual void execute(const message_type& message) override
		{
			execute_(message);
		}

		virtual reactive<bool> can_execute(const message_type& message) override
		{
			if (!can_execute_)
				return true;

			return can_execute_(message);
		}

	private:

		execute_type execute_;
		can_execute_type can_execute_;

	};

	template <class Message>
	std::shared_ptr<function_command<Message>> make_command(
		const std::function<void(const Message&)>& execute)
	{
		return std::make_shared<function_command<Message>>(execute);
	}

	template <class Message>
	std::shared_ptr<function_command<Message>> make_command(
		const std::function<void(const Message&)>& execute,
		const std::function<reactive<bool>(const Message&)>& can_execute)
	{
		return std::make_shared<function_command<Message>>(execute, can_execute);
	}

}

#pragma once

namespace rewin
{
	namespace detail
	{

		struct reactive_node_tag {};

		template <class T>
		class reactive_node
			: public observable<reactive_node_tag>
			, public std::enable_shared_from_this<reactive_node<T>>
			, boost::noncopyable
		{
		public:

			typedef T value_type;

			reactive_node()
				: value_()
			{
			}

			reactive_node(const value_type& value)
				: value_(value)
			{
			}

			reactive_node(value_type&& value)
				: value_(std::move(value))
			{
			}

			virtual ~reactive_node()
			{
			}

			const value_type& value() const
			{
				const_cast<reactive_node&>(*this).update();
				return value_;
			}

		protected:

			value_type& value_raw()
			{
				return value_;
			}

			void invalidate()
			{
				if (dirty_)
					return;

				dirty_ = true;
				notify();
			}

			virtual void on_update() = 0;

		private:

			void update()
			{
				if (dirty_)
				{
					on_update();
					dirty_ = false;
				}
			}

			value_type value_;

			bool dirty_ = true;

		};

	}
}

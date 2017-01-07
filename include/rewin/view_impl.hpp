#pragma once
#include "view.hpp"

namespace rewin
{
	namespace detail
	{

		template <class Derived, class Base>
		class view_impl : public Base
		{
		protected:

			view_impl()
			{
			}

			~view_impl()
			{
			}

		public:

			template <class... Args>
			static std::shared_ptr<Derived> create(Args&&... args)
			{
				struct create_helper : public Derived
				{
				public:

					void initialize_impl()
					{
						Derived::initialize();
					}

				};

				auto p = std::make_shared<create_helper>(std::forward<Args>(args)...);
				p->initialize_impl();
				return p;
			}

		};

	}
}

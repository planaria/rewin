#pragma once
#include "view.hpp"

namespace rewin
{
	namespace detail
	{

		class view_iterator
			: public boost::iterator_facade<view_iterator, view, boost::bidirectional_traversal_tag>
		{
		public:

			view_iterator()
			{
			}

			explicit view_iterator(
				const std::shared_ptr<view>& root_view,
				const std::shared_ptr<view>& view = nullptr)
				: root_view_(root_view)
				, view_(view)
			{
				BOOST_ASSERT(root_view);
			}

		private:

			friend class boost::iterator_core_access;

			void increment()
			{
				BOOST_ASSERT(root_view_);
				BOOST_ASSERT(view_);

				auto child = view_->first_child();
				if (child)
				{
					view_ = child;
					return;
				}

				while (true)
				{
					auto sibling = view_->next_sibling();
					if (sibling)
					{
						view_ = sibling;
						break;
					}

					if (view_ == root_view_)
					{
						view_ = nullptr;
						break;
					}

					view_ = view_->parent().value().lock();
					BOOST_ASSERT(view_);
				}
			}

			void decrement()
			{
				BOOST_ASSERT(root_view_);
				BOOST_ASSERT(view_ != root_view_);

				auto sibling = view_ ? view_->prev_sibling() : root_view_;
				if (sibling)
				{
					view_ = sibling;

					while (true)
					{
						auto child = view_->last_child();
						if (!child)
							break;

						view_ = child;
					}

					return;
				}

				view_ = view_->parent().value().lock();
				BOOST_ASSERT(view_);
			}

			bool equal(const view_iterator& other) const
			{
				BOOST_ASSERT(root_view_);
				BOOST_ASSERT(root_view_ == other.root_view_);
				return view_ == other.view_;
			}

			view& dereference() const
			{
				BOOST_ASSERT(root_view_);
				BOOST_ASSERT(view_);
				return *view_;
			}

			std::shared_ptr<view> root_view_;
			std::shared_ptr<view> view_;

		};

	}
}

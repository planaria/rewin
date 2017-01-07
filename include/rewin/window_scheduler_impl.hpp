#pragma once
#include "schedulable.hpp"
#include "win32_exception.hpp"

#define REWIN_WM_INVOKE (WM_USER + 1)

namespace rewin
{
	namespace detail
	{

		class window_scheduler_impl
			: public ATL::CWindowImpl<window_scheduler_impl, ATL::CWindow, ATL::CWinTraits<WS_OVERLAPPEDWINDOW, 0>>
			, boost::noncopyable
		{
		public:

			typedef std::chrono::high_resolution_clock clock_type;
			typedef clock_type::time_point time_point;

			DECLARE_WND_CLASS(_T("rewin window_scheduler"));

			BEGIN_MSG_MAP(window_scheduler_impl)
				MESSAGE_HANDLER(REWIN_WM_INVOKE, OnInvoke)
			END_MSG_MAP()

			window_scheduler_impl()
			{
				thread_ = std::thread(std::bind(&window_scheduler_impl::run, this));
			}

			~window_scheduler_impl()
			{
				try
				{
					{
						std::lock_guard<std::mutex> lock(mutex_);
						stop_ = true;
					}

					cv_.notify_all();
					thread_.join();

					if (IsWindow())
						REWIN_CHECK_WIN32(DestroyWindow());
				}
				catch (...)
				{
				}
			}

			void schedule(const std::weak_ptr<schedulable>& s)
			{
				{
					std::lock_guard<std::mutex> lock(mutex_);
					timeout_queue_.push(s);
				}

				REWIN_CHECK_WIN32(PostMessageW(REWIN_WM_INVOKE));
			}

			void schedule(const std::weak_ptr<schedulable>& s, time_point when)
			{
				{
					std::lock_guard<std::mutex> lock(mutex_);
					waiting_queue_.insert(std::make_pair(when, s));
				}

				cv_.notify_one();
			}

		private:

			LRESULT OnInvoke(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
			{
				bHandled = true;

				while (true)
				{
					std::shared_ptr<schedulable> s;

					{
						std::lock_guard<std::mutex> lock(mutex_);

						do
						{
							if (timeout_queue_.empty())
								break;

							s = timeout_queue_.front().lock();
							timeout_queue_.pop();
						} while (!s);
					}

					if (!s)
						break;

					s->on_invoke();
				}

				return TRUE;
			}

			void run()
			{
				while (true)
				{
					{
						std::unique_lock<std::mutex> lock(mutex_);

						if (waiting_queue_.empty())
						{
							cv_.wait(lock, [&]()
							{
								return stop_ || !waiting_queue_.empty();
							});
						}

						if (stop_)
							break;

						auto top = *waiting_queue_.begin();
						auto s = top.second.lock();

						if (!s)
						{
							waiting_queue_.erase(waiting_queue_.begin());
							continue;
						}

						if (clock_type::now() < top.first)
						{
							cv_.wait_until(lock, top.first);
							continue;
						}

						waiting_queue_.erase(waiting_queue_.begin());

						lock.unlock();

						schedule(s);
					}
				}
			}

			std::queue<std::weak_ptr<schedulable>> timeout_queue_;
			std::multimap<time_point, std::weak_ptr<schedulable>> waiting_queue_;

			std::thread thread_;
			std::mutex mutex_;
			std::condition_variable cv_;
			bool stop_ = false;

		};

	}
}

#pragma once

#include <future>
#include <queue>
#include <thread>
#include <map>
#include <mutex>
#include <atomic>
#include <iostream>

namespace utils
{
	class QueueCommand
	{
	public:
		QueueCommand()
		{

		}

		~QueueCommand()
		{
			
		}

		void SetCallback(std::function<void(const std::string&)> callback)
		{
			std::lock_guard<std::mutex> locker(mutex_callback_);
			callback_ = callback;
		}

		void StartScopeCommand()
		{
			scope_command_ = true;
		}

		void EndScopeCommand()
		{
			if (scope_command_)
			{
				condition_.notify_one();
			}
			scope_command_ = false;
		}

		QueueCommand(const QueueCommand&) = delete;
		QueueCommand(QueueCommand&&) = delete;

		void Reinit()
		{
			exception_ = false;
		}

		template<typename F, typename ... Args>
		void AddAsync(F f, Args ...args)
		{
			std::lock_guard<std::mutex> guard(mutex_next_queue_);

			auto newCommand = [=, this]()
			{
				try
				{
					if (exception_)
					{
						return;
					}
					else
					{
						f(args...);
					}
				}
				catch (const std::exception& e)
				{
					exception_ = true;
					std::lock_guard<std::mutex> locker(mutex_callback_);
					if (callback_)
					{
						callback_(e.what());
					}
				}
				
			};
			next_queue_.push(std::async(std::launch::deferred, newCommand));

			if (!scope_command_)
			{
				condition_.notify_one();
			}
		}

		template<typename T, typename F, typename ... Args>
		std::future<T> AddSync(const F& f, Args ...args)
		{
			std::lock_guard<std::mutex> guard(mutex_next_queue_);
			if (scope_command_)
			{
				scope_command_ = false;
			}

			std::promise<T> newPromise;
			std::future<T> result = newPromise.get_future();

			auto newCommand = [&](std::promise<T>&& promise)
			{
				try
				{
					if (exception_)
					{
						promise.set_value(T());
					}
					else
					{
						promise.set_value(f(args...));
					}
				}
				catch (...)
				{
					exception_ = true;
					try
					{
						promise.set_exception(std::current_exception());
					}
					catch (...)
					{
						// set_exception() It can also throw an exception.
						// In this case, we are not doing anything, as we have already tried an earlier exeption.
					}
				}
			};
			next_queue_.push(std::async(std::launch::deferred, newCommand, std::move(newPromise)));
			
			condition_.notify_one();
			return result;
		}
		
		void SwapQueue()
		{
			std::scoped_lock lock(mutex_next_queue_, mutex_current_queue_);
			current_queue_.swap(next_queue_);
		}

		bool RunCommand()
		{
			std::lock_guard<std::mutex> guard(mutex_current_queue_);            

			if (current_queue_.empty())
			{
				return false;
			}

			runQueue();

			return true;
		}

		template<typename T, typename F, typename ... Args>
		T AddSyncForce(F f, Args ...args)
		{
			std::lock_guard<std::mutex> lock(mutex_sync_force_);
			return f(args...);
		}

		void Wait(std::unique_lock<std::mutex>& lock)
		{
			condition_.wait(
				lock,
				[this]() {
					return !next_queue_.empty() || force_stop_;
				}
			);

			if (force_stop_)
			{
				force_stop_ = false;
			}
		}

		void ForceStop()
		{
			force_stop_ = true;
			condition_.notify_one();
		}

		std::mutex mutex_sync_force_;

	private:

		void runQueue()
		{
			size_t n = current_queue_.size();
			for (size_t i = 0; i < n; i++)
			{
				current_queue_.front().get();
				current_queue_.pop();
			}
		}

		std::mutex mutex_next_queue_;
		std::mutex mutex_current_queue_;

		std::queue<std::future<void>> current_queue_;
		std::queue<std::future<void>> next_queue_;

		std::atomic_bool exception_ = false;

		std::mutex mutex_callback_;
		std::function<void(const std::string&)> callback_;

		std::condition_variable condition_;

		// To combine several teams into a group.
		// There can be only one synchronous team in a group. 
		// Calling the synchronous command will end recording to the group.
		std::atomic<bool> scope_command_ = false;

		std::atomic<bool> force_stop_ = false;
	};
}

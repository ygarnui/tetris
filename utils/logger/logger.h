#pragma once

#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <fstream>
#include <stdio.h>

#include <queue_command.h>

#define DLL __declspec(dllexport)
#define LOG LoggerInstance::GetInstance()->GetLogger()->Log
#define LOGEXC(exc, ...) LoggerInstance::GetInstance()->GetLogger()->Log<exc>(__VA_ARGS__)

enum class Loglvl
{
	info,
	warning,
	debug,
	error,
	exception
};

struct LogCreateInfo
{
	std::wstring log_path;
	bool         log_to_cmd = true;
	bool         log_to_file = true;
};

class Logger
{
public:
	DLL Logger(LogCreateInfo logInit);

	DLL ~Logger();

	/*
	* \brief explicitly call before exiting main
	* 
	* If you do not do this, you may lose the last few records.
	*/
	DLL void Stop();

	template <typename F, typename ... T>
	DLL void Log(Loglvl logLvl, F first, T... t)
	{
		auto now = std::chrono::system_clock::now();
		std::stringstream threadIdStringStream;

		threadIdStringStream << "[" << std::this_thread::get_id() << "]";

		std::string threadIdString = threadIdStringStream.str();

		queue_command_->AddAsync([=, this]()
			{
				const auto current_time = std::chrono::system_clock::to_time_t(now);

				tm local_tm;
				localtime_s(&local_tm, &current_time);

				std::stringstream time;

				time << std::put_time(&local_tm, "[%Y:%m:%d %H:%M:%S:") << std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000 << "]";

				std::string LoglvlString;

				switch (logLvl)
				{
				case Loglvl::info:
					LoglvlString = "[INFO]";
					break;
				case Loglvl::warning:
					LoglvlString = "[WARNING]";
					break;
				case Loglvl::debug:
					LoglvlString = "[DEBUG]";
					break;
				case Loglvl::error:
					LoglvlString = "[ERROR]";
					break;
				case Loglvl::exception:
					LoglvlString = "[EXCEPTION]";
					break;
				}

				std::string time_string(time.str());

				if (log_to_cmd_)
				{
					logToCmd(time_string, threadIdString, LoglvlString, first, t...);
				}

				if (log_to_file_)
				{
					logToFile(time_string, threadIdString, LoglvlString, first, t...);
				}
			});
	}

	template <typename EXC, typename ... T>
	DLL void Log(T ... t)
	{
		Log(Loglvl::exception, t...);

		if (std::is_base_of_v<std::exception, EXC>)
		{
			std::stringstream ss;

			([&]
			{
				ss << t;

			}(), ...);

			for (auto callback : callbacks_)
			{
				callback(ss.str());
			}

			throw EXC(ss.str());
		}
	}

	DLL size_t AddCallback(const std::function<void(const std::string&)>& callback)
	{
		callbacks_.emplace_back(callback);
		return callbacks_.size() - 1;
	}

private:
	DLL void logToCmd();

	template <typename F, typename ... T>
	void logToCmd(F first, T ...t)
	{
		std::cout << first << " ";
		this->logToCmd(t...);
	}

	DLL void logToFile();

	template <typename F, typename ... T>
	DLL void logToFile(F first, T ...t)
	{
		file_ << first << " ";
		this->logToFile(t...);
	}

	DLL void run();

	std::wstring log_file_path_;
	std::ofstream file_;
	bool log_to_cmd_;
	bool log_to_file_;
	std::shared_ptr<utils::QueueCommand> queue_command_;
	std::future<void> future_run_;
	std::atomic<bool> running_ = true;
	std::vector<std::function<void(const std::string&)>> callbacks_;

	std::mutex queue_mutex_;
};

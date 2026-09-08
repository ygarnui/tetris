#include "logger.h"

Logger::Logger(LogCreateInfo logInit)
{
	log_file_path_ = logInit.log_path;
	log_to_cmd_ = logInit.log_to_cmd;
	log_to_file_ = logInit.log_to_file;

	if (log_to_file_)
	{
		file_.open(log_file_path_,std::ios::app);

		if (!file_.is_open())
		{
			throw std::runtime_error("Unable to open file");
		}
	}
	queue_command_ = std::make_shared<utils::QueueCommand>();

	future_run_ = std::async(std::launch::async, &Logger::run, this);
}

Logger::~Logger()
{
	Stop();
	file_.close();
}

void Logger::Stop()
{
	if (!running_.exchange(false))
	{
		return;
	}

	queue_command_->ForceStop();
	future_run_.get();
}

void Logger::logToCmd()
{
	std::cout << std::endl;
}

void Logger::logToFile()
{
	file_ << std::endl;
}

void Logger::run()
{
	while (running_)
	{
		std::unique_lock<std::mutex> lock(queue_mutex_);
		queue_command_->Wait(lock);

		std::lock_guard guard(queue_command_->mutex_sync_force_);
		queue_command_->SwapQueue();
		queue_command_->RunCommand();
	}
}

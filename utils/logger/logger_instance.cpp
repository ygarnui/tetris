#include <logger_instance.h>

LoggerInstance::~LoggerInstance()
{
	logger_.reset();
}

std::shared_ptr<LoggerInstance> LoggerInstance::GetInstance()
{
	static std::shared_ptr<LoggerInstance> instance_;

	if (instance_ == nullptr)
	{
		instance_ = std::shared_ptr<LoggerInstance>(new LoggerInstance());
	}

	return instance_;
}

std::shared_ptr<Logger> LoggerInstance::GetLogger() const
{
	return logger_;
}

LoggerInstance::LoggerInstance()
{
#ifdef _DEBUG 
	//LogCreateInfo log_init{ L"",true,false };
	LogCreateInfo log_init{ L"log.txt",true,true };
#else
	LogCreateInfo log_init{ L"log.txt",false,true };
#endif

	logger_ = std::make_shared<Logger>(log_init);

	logger_->Log(Loglvl::info, "log init successfully");
}

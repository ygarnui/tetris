#pragma once

#include <memory>
#include "logger.h"

class LoggerInstance
{
public:
	__declspec(dllexport) ~LoggerInstance();

	[[nodiscard]] __declspec(dllexport) static std::shared_ptr<LoggerInstance> GetInstance();

	LoggerInstance(const LoggerInstance&) = delete;

	__declspec(dllexport) std::shared_ptr<Logger> GetLogger() const;

private:

	std::shared_ptr<Logger> logger_;

	LoggerInstance();
};

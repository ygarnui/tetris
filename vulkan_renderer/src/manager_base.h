#pragma once

#include <logger_instance.h>

#include <memory>

namespace render
{
	class ManagerBase
	{

	public:
		ManagerBase()
		{
			logger_instance_ = LoggerInstance::GetInstance();
		}

		virtual ~ManagerBase() {};

		void ReinitManager()
		{
			need_reinit_ = true;
		}

		bool NeedReinit() const noexcept
		{
			return need_reinit_;
		}

	private:
		bool need_reinit_ = false;

		std::shared_ptr<LoggerInstance> logger_instance_;
	};
}

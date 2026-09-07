#include "guard_time.h"
#include "manager_time.h"

namespace render
{
	GuardTime::GuardTime(size_t numId) : num_id_(numId)
	{

	}

	//GuardTime::GuardTime(GuardTime&& val)
	//{
	//	num_id_ = val.num_id_;
	//}

	GuardTime::~GuardTime()
	{
		ManagerTime::Get().EndTimer(num_id_);
	}
}

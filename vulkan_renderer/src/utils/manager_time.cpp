#include "manager_time.h"
#include "guard_time.h"

#include <logger_instance.h>

namespace render
{

    ManagerTime& ManagerTime::Get()
    {
        static ManagerTime instanse_Manager_timer_;
        return instanse_Manager_timer_;
    }

    size_t ManagerTime::CreateTimer()
    {
        auto newId = cur_pos_++;
        auto newPos = newId + 1;

        current_time_.push_back(0);
        total_time_.push_back(0);
        old_time_.resize(newPos);
        cur_num_.push_back(0);
        max_time_.push_back(0);
        min_time_.push_back(std::numeric_limits<uint64_t>::max());

        return newId;
    }

    GuardTime ManagerTime::StartTimer(size_t numId)
    {
        cur_num_[numId]++;
        old_time_[numId] = std::chrono::steady_clock::now();
        return GuardTime(numId);
    }

    void ManagerTime::EndTimer(size_t numId)
    {
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        current_time_[numId] = std::chrono::duration_cast<std::chrono::nanoseconds>(end - old_time_[numId]).count();
        total_time_[numId] += current_time_[numId];
        max_time_[numId] = std::max(max_time_[numId], current_time_[numId]);
        min_time_[numId] = std::min(min_time_[numId], current_time_[numId]);
    }

    void ManagerTime::PrintInfo(size_t numId)
    {
        LOG(Loglvl::info, "id: " , numId);
        LOG(Loglvl::info, "total time: " , total_time_[numId] / divider_);
        LOG(Loglvl::info, "num: " , cur_num_[numId]);
        LOG(Loglvl::info, "average execution time: " , total_time_[numId] / cur_num_[numId] / divider_);
    }

    void ManagerTime::PrintMoreInfo(size_t numId)
    {
        LOG(Loglvl::info, "id: ", numId);
        LOG(Loglvl::info, "total time: ", total_time_[numId] / divider_);
        LOG(Loglvl::info, "max time: " , max_time_[numId] / divider_);
        LOG(Loglvl::info, "min time: " , min_time_[numId] / divider_);
        LOG(Loglvl::info, "num: " , cur_num_[numId]);
        LOG(Loglvl::info, "average execution time: " , total_time_[numId] / cur_num_[numId] / divider_);
    }

    void ManagerTime::PrintAllInfo(size_t numId)
    {
        auto curEnd = std::chrono::steady_clock::now();

        LOG(Loglvl::info, "id: ", numId);
        LOG(Loglvl::info, "total time: ", total_time_[numId] / divider_);
        LOG(Loglvl::info, "max time: ", max_time_[numId] / divider_);
        LOG(Loglvl::info, "min time: ", min_time_[numId] / divider_);
        LOG(Loglvl::info, "num: ", cur_num_[numId]);

        if (cur_num_[numId] != 0)
        {
            LOG(Loglvl::info, "average execution time: ", total_time_[numId] / cur_num_[numId] / divider_);
        }

        auto all_time = std::chrono::duration_cast<std::chrono::nanoseconds>(curEnd - start_).count();
        LOG(Loglvl::info, "program all time: " , all_time / divider_);
        if (cur_num_[numId] != 0)
        {
            LOG(Loglvl::info, "FPS: " , 1e9 / (total_time_[numId] / cur_num_[numId]));
        }
    }

    ManagerTime::ManagerTime()
    {
        Start();
    }

    void ManagerTime::Start()
    {
        start_ = std::chrono::steady_clock::now();
    }
}

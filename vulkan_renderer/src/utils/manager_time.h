#pragma once

#include <chrono>
#include <atomic>
#include <iostream>
#include <vector>

namespace render
{
    class GuardTime;

    class ManagerTime
    {
    public:
        static ManagerTime& Get();

        ManagerTime(const ManagerTime&) = delete;
        ManagerTime(ManagerTime&&) = delete;

        ManagerTime& operator= (const ManagerTime&) = delete;
        ManagerTime& operator= (ManagerTime&&) = delete;

        [[nodiscard]] size_t CreateTimer();

        [[nodiscard]] GuardTime StartTimer(size_t numId);

        void EndTimer(size_t numId);

        void PrintInfo(size_t numId);

        void PrintMoreInfo(size_t numId);

        void PrintAllInfo(size_t numId);

    private:

        ManagerTime();

        void Start();

        std::atomic<size_t> cur_pos_;
        std::vector<std::chrono::steady_clock::time_point> old_time_;
        std::vector<uint64_t> current_time_;
        std::vector<uint64_t> max_time_;
        std::vector<uint64_t> min_time_;
        std::vector<uint64_t> total_time_;
        std::vector<uint64_t> cur_num_;
        std::chrono::steady_clock::time_point start_;

        uint64_t divider_ = uint64_t(1e3);
    };
}
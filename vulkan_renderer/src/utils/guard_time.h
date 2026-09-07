#pragma once

#include <cfloat>
namespace render
{
    class GuardTime
    {
    public:
        GuardTime(size_t numId);

        /*GuardTime(const GuardTime&) = delete;
        GuardTime(GuardTime&&);

        GuardTime& operator= (const GuardTime&) = delete;
        GuardTime& operator= (GuardTime&&) = delete;*/

        ~GuardTime();

    private:

        size_t num_id_;
    };
}

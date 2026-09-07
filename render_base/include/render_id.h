#pragma once

#include <stdint.h>

namespace render
{
    class GeneratorId;
    class RenderId
    {
    public:

        RenderId()
        {
            id_ = -1;
        };

        RenderId(const RenderId&) = default;
        RenderId(RenderId&&) noexcept = default;
        virtual ~RenderId() = default;

        RenderId& operator= (const RenderId&) = default;
        RenderId& operator= (RenderId&&) noexcept = default;

        bool operator == (const RenderId& id) const noexcept
        {
            return id_ == id.id_;
        }

        bool operator != (const RenderId& id) const noexcept
        {
            return id_ != id.id_;
        }

        friend bool operator< (const RenderId& t1, const RenderId& t2)
        {
            return t1.id_ < t2.id_;
        }

        bool IsValid() const noexcept
        {
            return id_ != -1;

        }

        [[nodiscard]] inline size_t GetId() const
        {
            return static_cast<size_t>(id_);
        }

        [[nodiscard]] operator size_t() const
        {
            return static_cast<size_t>(id_);
        }

    protected:

        RenderId(uint64_t id)
        {
            id_ = id;
        }

        uint64_t id_;

        friend class GeneratorId;
    };
}

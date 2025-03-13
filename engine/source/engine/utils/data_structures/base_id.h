#pragma once

#include "hash.h"

#include <deque>

#include <type_traits>
#include <limits>


namespace ds
{
    template <typename T>
    class BaseID
    {
        static_assert(std::is_integral_v<T>, "T must be an integral type");

    public:
        using StorageType = T;

    public:
        BaseID() = default;
        explicit BaseID(StorageType value)
            : m_value(value) {}

        void Invalidate() noexcept { m_value = TEXTURE_ID_INVALID; }
        bool IsValid() const noexcept { return m_value != TEXTURE_ID_INVALID; }
        
        StorageType Value() const noexcept { return m_value; }
        void SetValue(StorageType value) noexcept { m_value = value; }

        uint64_t Hash() const noexcept { return m_value; }

        bool operator==(BaseID other) const noexcept { return m_value == other.m_value; }
        bool operator!=(BaseID other) const noexcept { return m_value != other.m_value; }
        bool operator>(BaseID other) const noexcept { return m_value > other.m_value; }
        bool operator<(BaseID other) const noexcept { return m_value < other.m_value; }
        bool operator<=(BaseID other) const noexcept { return m_value <= other.m_value; }
        bool operator>=(BaseID other) const noexcept { return m_value >= other.m_value; }

    private:
        static inline constexpr StorageType TEXTURE_ID_INVALID = std::numeric_limits<StorageType>::max();

    private:
        StorageType m_value = TEXTURE_ID_INVALID;
    };


    template <typename BaseIDType>
    class BaseIDPool
    {
        static_assert(std::is_same_v<BaseIDType, BaseID<BaseIDType::StorageType>>);
    public:
        using IDType = BaseIDType;

    public:
        IDType Allocate() noexcept;
        void Deallocate(IDType& ID) noexcept;

        void Reset() noexcept;

    private:
        std::deque<IDType> m_idFreeList;
        IDType m_nextAllocatedID = IDType{0};
    };
}

template <typename T>
inline uint64_t amHash(const ds::BaseID<T>& ID) noexcept
{
    return ID.Hash();
}


#include "base_id.hpp"
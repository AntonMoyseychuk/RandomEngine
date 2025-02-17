#pragma once

#include "hash.h"

#include <type_traits>
#include <limits>


template <typename T>
class BaseID
{
    static_assert(std::is_integral_v<T>, "T must be an integral type");

public:
    using InternalType = T;

public:
    BaseID() = default;
    explicit BaseID(InternalType value)
        : m_value(value) {}

    void Invalidate() noexcept { m_value = TEXTURE_ID_INVALID; }
    bool IsValid() const noexcept { return m_value != TEXTURE_ID_INVALID; }
    
    InternalType Value() const noexcept { return m_value; }
    void SetValue(InternalType value) noexcept { m_value = value; }

    uint64_t Hash() const noexcept { return m_value; }

    bool operator==(BaseID other) const noexcept { return m_value == other.m_value; }
    bool operator!=(BaseID other) const noexcept { return m_value != other.m_value; }
    bool operator>(BaseID other) const noexcept { return m_value > other.m_value; }
    bool operator<(BaseID other) const noexcept { return m_value < other.m_value; }
    bool operator<=(BaseID other) const noexcept { return m_value <= other.m_value; }
    bool operator>=(BaseID other) const noexcept { return m_value >= other.m_value; }

private:
    static inline constexpr InternalType TEXTURE_ID_INVALID = std::numeric_limits<InternalType>::max();

private:
    InternalType m_value = TEXTURE_ID_INVALID;
};


template <typename T>
inline uint64_t amHash(const BaseID<T>& ID) noexcept
{
    return ID.Hash();
}
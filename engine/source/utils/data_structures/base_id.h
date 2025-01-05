#pragma once

#include "hash.h"


class BaseID
{
public:
    BaseID() = default;
    explicit BaseID(uint64_t value)
        : m_value(value) {}

    void Invalidate() noexcept { m_value = TEXTURE_ID_INVALID; }
    bool IsValid() const noexcept { return m_value != TEXTURE_ID_INVALID; }
    
    uint64_t Value() const noexcept { return m_value; }

    uint64_t Hash() const noexcept { return m_value; }

    bool operator==(BaseID other) const noexcept { return m_value == other.m_value; }
    bool operator!=(BaseID other) const noexcept { return m_value != other.m_value; }
    bool operator>(BaseID other) const noexcept { return m_value > other.m_value; }
    bool operator<(BaseID other) const noexcept { return m_value < other.m_value; }
    bool operator<=(BaseID other) const noexcept { return m_value <= other.m_value; }
    bool operator>=(BaseID other) const noexcept { return m_value >= other.m_value; }

private:
    static inline constexpr uint64_t TEXTURE_ID_INVALID = UINT64_MAX;

private:
    uint64_t m_value = TEXTURE_ID_INVALID;
};


inline uint64_t amHash(BaseID ID) noexcept
{
    return ID.Hash();
}
#pragma once

#include <cstdint>


enum class ShaderResourceType : uint32_t
{
    TYPE_SAMPLER_2D
};


template<ShaderResourceType type>
class ShaderResourceBindStruct
{
public:
    constexpr ShaderResourceBindStruct(int32_t location, int32_t binding)
        : m_location(location), m_binding(binding)
    {}

    constexpr ShaderResourceType GetType() const noexcept { return type; }
    constexpr int32_t GetLocation() const noexcept { return m_location; }
    constexpr int32_t GetBinding() const noexcept { return m_binding; }

private:
    int32_t m_location = -1;
    int32_t m_binding = -1;
};
#pragma once

#include <cstdint>


enum class ShaderResourceType : uint32_t
{
    TYPE_BOOL,
    TYPE_INT,
    TYPE_UINT,
    TYPE_FLOAT,
    TYPE_DOUBLE,

    TYPE_SAMPLER_2D,
    TYPE_CONST_BUFFER,
};


template<ShaderResourceType type>
class ShaderResourceBindStruct
{
public:
    constexpr ShaderResourceBindStruct(int32_t location, int32_t binding)
        : m_location(location), m_binding(binding)
    {}

    constexpr int32_t GetLocation() const noexcept { return m_location; }
    constexpr int32_t GetBinding() const noexcept { return m_binding; }

public:
    constexpr static ShaderResourceType GetType() noexcept { return type; }

private:
    int32_t m_location = -1;
    int32_t m_binding = -1;
};


template <typename RESOURCE>
inline constexpr auto GetResourceBinding() noexcept -> const decltype(RESOURCE::_BINDING)&
{
    return RESOURCE::_BINDING;
}


template <typename TEX_RESOURCE>
inline constexpr uint32_t GetTexResourceSamplerIdx() noexcept
{
    return TEX_RESOURCE::_SAMPLER_IDX;
}


template <typename TEX_RESOURCE>
inline constexpr uint32_t GetTexResourceFormat() noexcept
{
    return TEX_RESOURCE::_FORMAT;
}


#define resGetResourceBinding(RESOURCE) GetResourceBinding<RESOURCE>()
#define resGetTexResourceSamplerIdx(TEX_RESOURCE) GetTexResourceSamplerIdx<TEX_RESOURCE>()
#define resGetTexResourceFormat(TEX_RESOURCE) GetTexResourceFormat<TEX_RESOURCE>()
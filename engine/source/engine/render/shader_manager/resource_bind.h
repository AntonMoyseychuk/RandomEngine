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
            
    /*    TYPE_IVEC2,
        TYPE_IVEC3,
        TYPE_IVEC4,
            
        TYPE_UVEC2,
        TYPE_UVEC3,
        TYPE_UVEC4,
            
        TYPE_FVEC2,
        TYPE_FVEC3,
        TYPE_FVEC4,
            
        TYPE_DVEC2,
        TYPE_DVEC3,
        TYPE_DVEC4,
            
        TYPE_MAT2X2,
        TYPE_MAT2X3,
        TYPE_MAT2X4,
            
        TYPE_MAT3X2,
        TYPE_MAT3X3,
        TYPE_MAT3X4,
            
        TYPE_MAT4X2,
        TYPE_MAT4X3,
        TYPE_MAT4X4,
            
        TYPE_DMAT2X2,
        TYPE_DMAT2X3,
        TYPE_DMAT2X4,
            
        TYPE_DMAT3X2,
        TYPE_DMAT3X3,
        TYPE_DMAT3X4,
            
        TYPE_DMAT4X2,
        TYPE_DMAT4X3,
        TYPE_DMAT4X4,
            
        TYPE_SAMPLER_1D,
        TYPE_SAMPLER_2D,
        TYPE_SAMPLER_3D,
        TYPE_SAMPLER_CUBE,

        TYPE_IMAGE_1D,
        TYPE_IMAGE_2D,
        TYPE_IMAGE_3D,
        TYPE_IMAGE_CUBE,

        TYPE_IMAGE_2D_RECT,
        
        TYPE_IMAGE_BUFFER,

        TYPE_IMAGE_1D_ARRAY,
        TYPE_IMAGE_2D_ARRAY,

        TYPE_IMAGE_2D_MULTISAMPLE,
        TYPE_IMAGE_2D_MULTISAMPLE_ARRAY,

        TYPE_COUNT,
        TYPE_INVALID = TYPE_COUNT,*/
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
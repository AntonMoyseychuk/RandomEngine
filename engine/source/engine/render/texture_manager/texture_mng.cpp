#include "pch.h"
#include "texture_mng.h"

#include "utils/debug/assertion.h"
#include "utils/data_structures/hash.h"

#include "render/platform/OpenGL/opengl_driver.h"

#include "auto/auto_texture_constants.h"
#include "auto/auto_registers_common.h"


struct TextureSamplerStateCreateInfo
{
    uint32_t wrapModeS = 0;
    uint32_t wrapModeT = 0;
    uint32_t wrapModeR = 0;
    uint32_t minFiltering = 0;
    uint32_t magFiltering = 0;
};


enum class TextureFormat
{
    FORMAT_R8,
    FORMAT_R8_SNORM,
    FORMAT_R16,
    FORMAT_R16_SNORM,
    FORMAT_RG8,
    FORMAT_RG8_SNORM,
    FORMAT_RG16,
    FORMAT_RG16_SNORM,
    FORMAT_RGB8_SNORM,
    FORMAT_RGB16_SNORM,
    FORMAT_RGBA8,
    FORMAT_RGBA8_SNORM,
    FORMAT_RGBA16,
    FORMAT_SRGB8,
    FORMAT_SRGB8_ALPHA8,
    FORMAT_R16F,
    FORMAT_RG16F,
    FORMAT_RGB16F,
    FORMAT_RGBA16F,
    FORMAT_R32F,
    FORMAT_RG32F,
    FORMAT_RGB32F,
    FORMAT_RGBA32F,
    FORMAT_R8I,
    FORMAT_R8UI,
    FORMAT_R16I,
    FORMAT_R16UI,
    FORMAT_R32I,
    FORMAT_R32UI,
    FORMAT_RG8UI,
    FORMAT_RG16I,
    FORMAT_RG16UI,
    FORMAT_RG32UI,
    FORMAT_RGB8I,
    FORMAT_RGB8UI,
    FORMAT_RGB16I,
    FORMAT_RGB16UI,
    FORMAT_RGB32I,
    FORMAT_RGB32UI,
    FORMAT_RGBA8I,
    FORMAT_RGBA16I,
    FORMAT_RGBA16UI,
    FORMAT_RGBA32I,
    FORMAT_RGBA32UI,

    FORMAT_DEPTH16,
    FORMAT_DEPTH24,
    FORMAT_DEPTH32,
    FORMAT_STENCIL1,
    FORMAT_STENCIL4,
    FORMAT_STENCIL8,
    FORMAT_STENCIL16,
    FORMAT_DEPTH24_STENCIL8,
    FORMAT_DEPTH32_STENCIL8,
    
    FORMAT_INVALID,
    FORMAT_COUNT = FORMAT_INVALID,
};


static std::unique_ptr<TextureManager> pTextureMngInst = nullptr;


TextureSamplerState::TextureSamplerState(TextureSamplerState &&other) noexcept
{
#if defined(ENG_DEBUG)
    std::swap(m_dbgName, other.m_dbgName);
#endif
    std::swap(m_renderID, other.m_renderID);
}


TextureSamplerState &TextureSamplerState::operator=(TextureSamplerState &&other) noexcept
{
    Destroy();

#if defined(ENG_DEBUG)
    std::swap(m_dbgName, other.m_dbgName);
#endif
    std::swap(m_renderID, other.m_renderID);

    return *this;
}


void TextureSamplerState::Bind(uint32_t unit) noexcept
{
    glBindSampler(unit, m_renderID);
}


bool TextureSamplerState::Init(const void* pCreateInfo, ds::StrID dbgName) noexcept
{
    ENG_ASSERT(pCreateInfo != nullptr, "pCreateInfo is nullptr");

    if (IsValid()) {
        ENG_LOG_WARN("Recreating of \'{}\' sampler by \'{}\'", m_dbgName.CStr(), dbgName.CStr());
        Destroy();
    }

#if defined(ENG_DEBUG)
    m_dbgName = dbgName;
#endif

    const TextureSamplerStateCreateInfo& createInfo = *reinterpret_cast<const TextureSamplerStateCreateInfo*>(pCreateInfo);

    glCreateSamplers(1, &m_renderID);
    glSamplerParameteri(m_renderID, GL_TEXTURE_MIN_FILTER, createInfo.minFiltering);
    glSamplerParameteri(m_renderID, GL_TEXTURE_MAG_FILTER, createInfo.magFiltering);
    glSamplerParameteri(m_renderID, GL_TEXTURE_WRAP_S, createInfo.wrapModeS);
    glSamplerParameteri(m_renderID, GL_TEXTURE_WRAP_T, createInfo.wrapModeT);
    glSamplerParameteri(m_renderID, GL_TEXTURE_WRAP_R, createInfo.wrapModeR);

    return true;
}


void TextureSamplerState::Destroy() noexcept
{
#if defined(ENG_DEBUG)
    m_dbgName = "_INVALID_";
#endif

    glDeleteSamplers(1, &m_renderID);
    m_renderID = 0;
}


static TextureFormat ConvertShaderTexResourceFormat(uint32_t resFormat) noexcept
{
    switch(resFormat) {
        case TEXTURE_FORMAT_R8: return TextureFormat::FORMAT_R8;
        case TEXTURE_FORMAT_R8_SNORM: return TextureFormat::FORMAT_R8_SNORM;
        case TEXTURE_FORMAT_R16: return TextureFormat::FORMAT_R16;
        case TEXTURE_FORMAT_R16_SNORM: return TextureFormat::FORMAT_R16_SNORM;
        case TEXTURE_FORMAT_RG8: return TextureFormat::FORMAT_RG8;
        case TEXTURE_FORMAT_RG8_SNORM: return TextureFormat::FORMAT_RG8_SNORM;
        case TEXTURE_FORMAT_RG16: return TextureFormat::FORMAT_RG16;
        case TEXTURE_FORMAT_RG16_SNORM: return TextureFormat::FORMAT_RG16_SNORM;
        case TEXTURE_FORMAT_RGB8_SNORM: return TextureFormat::FORMAT_RGB8_SNORM;
        case TEXTURE_FORMAT_RGB16_SNORM: return TextureFormat::FORMAT_RGB16_SNORM;
        case TEXTURE_FORMAT_RGBA8: return TextureFormat::FORMAT_RGBA8;
        case TEXTURE_FORMAT_RGBA8_SNORM: return TextureFormat::FORMAT_RGBA8_SNORM;
        case TEXTURE_FORMAT_RGBA16: return TextureFormat::FORMAT_RGBA16;
        case TEXTURE_FORMAT_SRGB8: return TextureFormat::FORMAT_SRGB8;
        case TEXTURE_FORMAT_SRGB8_ALPHA8: return TextureFormat::FORMAT_SRGB8_ALPHA8;
        case TEXTURE_FORMAT_R16F: return TextureFormat::FORMAT_R16F;
        case TEXTURE_FORMAT_RG16F: return TextureFormat::FORMAT_RG16F;
        case TEXTURE_FORMAT_RGB16F: return TextureFormat::FORMAT_RGB16F;
        case TEXTURE_FORMAT_RGBA16F: return TextureFormat::FORMAT_RGBA16F;
        case TEXTURE_FORMAT_R32F: return TextureFormat::FORMAT_R32F;
        case TEXTURE_FORMAT_RG32F: return TextureFormat::FORMAT_RG32F;
        case TEXTURE_FORMAT_RGB32F: return TextureFormat::FORMAT_RGB32F;
        case TEXTURE_FORMAT_RGBA32F: return TextureFormat::FORMAT_RGBA32F;
        case TEXTURE_FORMAT_R8I: return TextureFormat::FORMAT_R8I;
        case TEXTURE_FORMAT_R8UI: return TextureFormat::FORMAT_R8UI;
        case TEXTURE_FORMAT_R16I: return TextureFormat::FORMAT_R16I;
        case TEXTURE_FORMAT_R16UI: return TextureFormat::FORMAT_R16UI;
        case TEXTURE_FORMAT_R32I: return TextureFormat::FORMAT_R32I;
        case TEXTURE_FORMAT_R32UI: return TextureFormat::FORMAT_R32UI;
        case TEXTURE_FORMAT_RG8UI: return TextureFormat::FORMAT_RG8UI;
        case TEXTURE_FORMAT_RG16I: return TextureFormat::FORMAT_RG16I;
        case TEXTURE_FORMAT_RG16UI: return TextureFormat::FORMAT_RG16UI;
        case TEXTURE_FORMAT_RG32UI: return TextureFormat::FORMAT_RG32UI;
        case TEXTURE_FORMAT_RGB8I: return TextureFormat::FORMAT_RGB8I;
        case TEXTURE_FORMAT_RGB8UI: return TextureFormat::FORMAT_RGB8UI;
        case TEXTURE_FORMAT_RGB16I: return TextureFormat::FORMAT_RGB16I;
        case TEXTURE_FORMAT_RGB16UI: return TextureFormat::FORMAT_RGB16UI;
        case TEXTURE_FORMAT_RGB32I: return TextureFormat::FORMAT_RGB32I;
        case TEXTURE_FORMAT_RGB32UI: return TextureFormat::FORMAT_RGB32UI;
        case TEXTURE_FORMAT_RGBA8I: return TextureFormat::FORMAT_RGBA8I;
        case TEXTURE_FORMAT_RGBA16I: return TextureFormat::FORMAT_RGBA16I;
        case TEXTURE_FORMAT_RGBA16UI: return TextureFormat::FORMAT_RGBA16UI;
        case TEXTURE_FORMAT_RGBA32I: return TextureFormat::FORMAT_RGBA32I;
        case TEXTURE_FORMAT_RGBA32UI: return TextureFormat::FORMAT_RGBA32UI;
        case TEXTURE_FORMAT_DEPTH16: return TextureFormat::FORMAT_DEPTH16;
        case TEXTURE_FORMAT_DEPTH24: return TextureFormat::FORMAT_DEPTH24;
        case TEXTURE_FORMAT_DEPTH32: return TextureFormat::FORMAT_DEPTH32;
        case TEXTURE_FORMAT_STENCIL1: return TextureFormat::FORMAT_STENCIL1;
        case TEXTURE_FORMAT_STENCIL4: return TextureFormat::FORMAT_STENCIL4;
        case TEXTURE_FORMAT_STENCIL8: return TextureFormat::FORMAT_STENCIL8;
        case TEXTURE_FORMAT_STENCIL16: return TextureFormat::FORMAT_STENCIL16;
        case TEXTURE_FORMAT_DEPTH24_STENCIL8: return TextureFormat::FORMAT_DEPTH24_STENCIL8;
        case TEXTURE_FORMAT_DEPTH32_STENCIL8: return TextureFormat::FORMAT_DEPTH32_STENCIL8;
        default: return TextureFormat::FORMAT_INVALID;
    }
}


static GLenum GetTextureInternalGLFormat(TextureFormat format) noexcept
{
    switch (format) {
        case TextureFormat::FORMAT_R8: return GL_R8;
        case TextureFormat::FORMAT_R8_SNORM: return GL_R8_SNORM;
        case TextureFormat::FORMAT_R16: return GL_R16;
        case TextureFormat::FORMAT_R16_SNORM: return GL_R16_SNORM;
        case TextureFormat::FORMAT_RG8: return GL_RG8;
        case TextureFormat::FORMAT_RG8_SNORM: return GL_RG8_SNORM;
        case TextureFormat::FORMAT_RG16: return GL_RG16;
        case TextureFormat::FORMAT_RG16_SNORM: return GL_RG16_SNORM;
        case TextureFormat::FORMAT_RGB8_SNORM: return GL_RGB8_SNORM;
        case TextureFormat::FORMAT_RGB16_SNORM: return GL_RGB16_SNORM;
        case TextureFormat::FORMAT_RGBA8: return GL_RGBA8;
        case TextureFormat::FORMAT_RGBA8_SNORM: return GL_RGBA8_SNORM;
        case TextureFormat::FORMAT_RGBA16: return GL_RGBA16;
        case TextureFormat::FORMAT_SRGB8: return GL_SRGB8;
        case TextureFormat::FORMAT_SRGB8_ALPHA8: return GL_SRGB8_ALPHA8;
        case TextureFormat::FORMAT_R16F: return GL_R16F;
        case TextureFormat::FORMAT_RG16F: return GL_RG16F;
        case TextureFormat::FORMAT_RGB16F: return GL_RGB16F;
        case TextureFormat::FORMAT_RGBA16F: return GL_RGBA16F;
        case TextureFormat::FORMAT_R32F: return GL_R32F;
        case TextureFormat::FORMAT_RG32F: return GL_RG32F;
        case TextureFormat::FORMAT_RGB32F: return GL_RGB32F;
        case TextureFormat::FORMAT_RGBA32F: return GL_RGBA32F;
        case TextureFormat::FORMAT_R8I: return GL_R8I;
        case TextureFormat::FORMAT_R8UI: return GL_R8UI;
        case TextureFormat::FORMAT_R16I: return GL_R16I;
        case TextureFormat::FORMAT_R16UI: return GL_R16UI;
        case TextureFormat::FORMAT_R32I: return GL_R32I;
        case TextureFormat::FORMAT_R32UI: return GL_R32UI;
        case TextureFormat::FORMAT_RG8UI: return GL_RG8UI;
        case TextureFormat::FORMAT_RG16I: return GL_RG16I;
        case TextureFormat::FORMAT_RG16UI: return GL_RG16UI;
        case TextureFormat::FORMAT_RG32UI: return GL_RG32UI;
        case TextureFormat::FORMAT_RGB8I: return GL_RGB8I;
        case TextureFormat::FORMAT_RGB8UI: return GL_RGB8UI;
        case TextureFormat::FORMAT_RGB16I: return GL_RGB16I;
        case TextureFormat::FORMAT_RGB16UI: return GL_RGB16UI;
        case TextureFormat::FORMAT_RGB32I: return GL_RGB32I;
        case TextureFormat::FORMAT_RGB32UI: return GL_RGB32UI;
        case TextureFormat::FORMAT_RGBA8I: return GL_RGBA8I;
        case TextureFormat::FORMAT_RGBA16I: return GL_RGBA16I;
        case TextureFormat::FORMAT_RGBA16UI: return GL_RGBA16UI;
        case TextureFormat::FORMAT_RGBA32I: return GL_RGBA32I;
        case TextureFormat::FORMAT_RGBA32UI: return GL_RGBA32UI;
        case TextureFormat::FORMAT_DEPTH16: return GL_DEPTH_COMPONENT16;
        case TextureFormat::FORMAT_DEPTH24: return GL_DEPTH_COMPONENT24;
        case TextureFormat::FORMAT_DEPTH32: return GL_DEPTH_COMPONENT32F;
        case TextureFormat::FORMAT_STENCIL1: return GL_STENCIL_INDEX1;
        case TextureFormat::FORMAT_STENCIL4: return GL_STENCIL_INDEX4;
        case TextureFormat::FORMAT_STENCIL8: return GL_STENCIL_INDEX8;
        case TextureFormat::FORMAT_STENCIL16: return GL_STENCIL_INDEX16;
        case TextureFormat::FORMAT_DEPTH24_STENCIL8: return GL_DEPTH24_STENCIL8;
        case TextureFormat::FORMAT_DEPTH32_STENCIL8: return GL_DEPTH32F_STENCIL8;
        default: return GL_NONE;
    };
}


static GLenum GetTextureInputDataGLFormat(TextureInputDataFormat format) noexcept
{
    switch (format) {
        case TextureInputDataFormat::INPUT_FORMAT_R: return GL_RED;
        case TextureInputDataFormat::INPUT_FORMAT_RG: return GL_RG;
        case TextureInputDataFormat::INPUT_FORMAT_RGB: return GL_RGB;
        case TextureInputDataFormat::INPUT_FORMAT_BGR: return GL_RGB;
        case TextureInputDataFormat::INPUT_FORMAT_RGBA: return GL_RGBA;
        case TextureInputDataFormat::INPUT_FORMAT_DEPTH: return GL_DEPTH_COMPONENT;
        case TextureInputDataFormat::INPUT_FORMAT_STENCIL: return GL_STENCIL_INDEX;
        default: return GL_NONE;
    };
}


static GLenum GetTextureInputDataGLType(TextureInputDataType type) noexcept
{
    switch (type) {
        case TextureInputDataType::INPUT_TYPE_UNSIGNED_BYTE: return GL_UNSIGNED_BYTE;
        case TextureInputDataType::INPUT_TYPE_BYTE: return GL_BYTE;
        case TextureInputDataType::INPUT_TYPE_UNSIGNED_SHORT: return GL_UNSIGNED_SHORT;
        case TextureInputDataType::INPUT_TYPE_SHORT: return GL_SHORT;
        case TextureInputDataType::INPUT_TYPE_UNSIGNED_INT: return GL_UNSIGNED_INT;
        case TextureInputDataType::INPUT_TYPE_INT: return GL_INT;
        case TextureInputDataType::INPUT_TYPE_FLOAT: return GL_FLOAT;
        default: return GL_NONE;
    };
}


Texture::Texture(Texture &&other) noexcept
{
    std::swap(m_name, other.m_name);
    std::swap(m_ID, other.m_ID);
    std::swap(m_type, other.m_type);
    std::swap(m_levelsCount, other.m_levelsCount);
    std::swap(m_width, other.m_width);
    std::swap(m_height, other.m_height);
    std::swap(m_depth, other.m_depth);
    std::swap(m_renderID, other.m_renderID);
}


Texture& Texture::operator=(Texture &&other) noexcept
{
    Destroy();

    std::swap(m_name, other.m_name);
    std::swap(m_ID, other.m_ID);
    std::swap(m_type, other.m_type);
    std::swap(m_levelsCount, other.m_levelsCount);
    std::swap(m_width, other.m_width);
    std::swap(m_height, other.m_height);
    std::swap(m_depth, other.m_depth);
    std::swap(m_renderID, other.m_renderID);

    return *this;
}


void Texture::Bind(uint32_t unit) noexcept
{
    ENG_ASSERT_GRAPHICS_API(IsValid(), "Attempt to bind invalid texture");
    glBindTextureUnit(unit, m_renderID);
}


bool Texture::IsValid() const noexcept
{
    return m_ID.IsValid() && m_renderID != 0;
}


bool Texture::IsType1D() const noexcept
{
    return m_type == GL_TEXTURE_1D;
}


bool Texture::IsType2D() const noexcept
{
    return m_type == GL_TEXTURE_2D;
}


bool Texture::IsType3D() const noexcept
{
    return m_type == GL_TEXTURE_3D;
}


bool Texture::IsTypeBuffer() const noexcept
{
    return m_type == GL_TEXTURE_BUFFER;
}


bool Texture::IsTypeCubeMap() const noexcept
{
    return m_type == GL_TEXTURE_CUBE_MAP;
}


bool Texture::IsTypeArray1D() const noexcept
{
    return m_type == GL_TEXTURE_1D_ARRAY;
}


bool Texture::IsTypeArray2D() const noexcept
{
    return m_type == GL_TEXTURE_2D_ARRAY;
}


bool Texture::IsTypeCubeMapArray() const noexcept
{
    return m_type == GL_TEXTURE_CUBE_MAP_ARRAY;
}


bool Texture::IsTypeMultisample2D() const noexcept
{
    return m_type == GL_TEXTURE_2D_MULTISAMPLE;
}


bool Texture::IsTypeMultisampleArray2D() const noexcept
{
    return m_type == GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
}


uint64_t Texture::Hash() const noexcept
{
    ds::HashBuilder builder;

    builder.AddValue(m_name);
    builder.AddValue(m_type);
    builder.AddValue(m_width);
    builder.AddValue(m_height);
    builder.AddValue(m_depth);
    builder.AddValue(m_renderID);

    return builder.Value();
}


bool Texture::Create(const Texture2DCreateInfo &createInfo) noexcept
{
    ENG_ASSERT(!IsValid(), "Attempt to create already valid texture: {}", m_name.CStr());
    ENG_ASSERT(m_ID.IsValid(), "Texture \'{}\' ID is invalid. You must initialize only textures which were returned by TextureManager", m_name.CStr());

    m_type = GL_TEXTURE_2D;
    m_levelsCount = 1 + createInfo.mipmapsCount;
    m_width = createInfo.width;
    m_height = createInfo.height;
    m_depth = 1;

    glCreateTextures(m_type, 1, &m_renderID);

    const TextureFormat convertedFormat = ConvertShaderTexResourceFormat(createInfo.format);
    ENG_ASSERT(convertedFormat != TextureFormat::FORMAT_INVALID, "Invalid reflected texture \'{}\' format: \'{}\'", m_name.CStr(), createInfo.format);

    const GLenum internalFormat = GetTextureInternalGLFormat(convertedFormat);
    ENG_ASSERT("Invalid texture \'{}\' format", m_name.CStr());

    glTextureStorage2D(m_renderID, m_levelsCount, internalFormat, m_width, m_height);
    
    const void* pData = createInfo.inputData.pData;

    if (!pData) {
        return true;
    }

    const GLenum inputDataFormat = GetTextureInputDataGLFormat(createInfo.inputData.format);
    ENG_ASSERT(inputDataFormat != GL_NONE, "Invalid texture input data format: {}", static_cast<uint32_t>(createInfo.inputData.format));

    const GLenum inputDataType = GetTextureInputDataGLType(createInfo.inputData.dataType);
    ENG_ASSERT(inputDataType != GL_NONE, "Invalid texture input data type: {}", static_cast<uint32_t>(createInfo.inputData.dataType));

    glTextureSubImage2D(m_renderID, 0, 0, 0, m_width, m_height, inputDataFormat, inputDataType, pData);

    if (createInfo.mipmapsCount > 0) {
        glGenerateTextureMipmap(m_renderID);
    }

    return true;
}


void Texture::Destroy() noexcept
{
    if (!IsValid()) {
        return;
    }

    glDeleteTextures(1, &m_renderID);

    m_type = 0;
    m_levelsCount = 0;
    m_width = 0;
    m_height = 0;
    m_depth = 0;
    m_renderID = 0;
}


TextureManager& TextureManager::GetInstance() noexcept
{
    ENG_ASSERT(engIsTextureManagerInitialized(), "Texture manager is not initialized");
    return *pTextureMngInst;
}


TextureManager::~TextureManager()
{
    Terminate();
}


Texture* TextureManager::RegisterTexture2D(ds::StrID name) noexcept
{
    ENG_ASSERT(GetTextureByName(name) == nullptr, "Attempt to register already registered 2D texture: {}", name.CStr());

    const TextureID textureID = AllocateTextureID();
    const uint64_t index = textureID.Value();

    Texture* pTex = &m_texturesStorage[index];

    ENG_ASSERT(!pTex->IsValid(), "Valid texture was returned during registration");

    pTex->m_name = name;
    pTex->m_ID = textureID;

    m_textureNameToStorageIndexMap[name] = index;

    return pTex;
}


Texture* TextureManager::GetTextureByName(ds::StrID name) noexcept
{
    const auto indexIt = m_textureNameToStorageIndexMap.find(name);
    return indexIt != m_textureNameToStorageIndexMap.cend() ? &m_texturesStorage[indexIt->second] : nullptr;
}


void TextureManager::UnregisterTexture(ds::StrID name) noexcept
{
    Texture* pTex = GetTextureByName(name);

    UnregisterTexture(pTex);
}


void TextureManager::UnregisterTexture(Texture* pTex) noexcept
{
    if (!pTex) {
        return;
    }

    if (pTex->IsValid()) {
        ENG_LOG_WARN("Unregistration of texture \'{}\' while it's steel valid. Prefer to destroy textures manually", pTex->GetName().CStr());
        pTex->Destroy();
    }

    DeallocateTextureID(pTex->m_ID);

    m_textureNameToStorageIndexMap.erase(pTex->m_name);

    pTex->m_name = "_INVALID_";
    pTex->m_ID.Invalidate();
}


TextureSamplerState *TextureManager::GetSampler(uint32_t samplerIdx) noexcept
{
    return IsValidSamplerIdx(samplerIdx) ? &m_textureSamplersStorage[samplerIdx] : nullptr;
}


bool TextureManager::IsValidSamplerIdx(uint32_t samplerIdx) const noexcept
{
    return samplerIdx < m_textureSamplersStorage.size();
}


bool TextureManager::Init() noexcept
{
    if (IsInitialized()) {
        return true;
    }

    m_texturesStorage.resize(COMMON_MAX_TEXTURES_COUNT);
    m_textureNameToStorageIndexMap.reserve(COMMON_MAX_TEXTURES_COUNT);

    InitializeSamplers();

    m_nextAllocatedID = TextureID(0);

    m_isInitialized = true;

    return true;
}


void TextureManager::Terminate() noexcept
{
    m_texturesStorage.clear();

    m_textureNameToStorageIndexMap.clear();

    m_textureIDFreeList.clear();

    DestroySamplers();

    m_nextAllocatedID = TextureID(0);

    m_isInitialized = false;
}


TextureID TextureManager::AllocateTextureID() noexcept
{
    if (m_textureIDFreeList.empty()) {
        ENG_ASSERT(m_nextAllocatedID.Value() < m_texturesStorage.size() - 1, "Texture storage overflow");

        const TextureID textureID = m_nextAllocatedID;
        m_nextAllocatedID = TextureID(m_nextAllocatedID.Value() + 1);

        return textureID;
    }

    const TextureID textureID = m_textureIDFreeList.front();
    m_textureIDFreeList.pop_front();
        
    return textureID;
}


void TextureManager::DeallocateTextureID(TextureID ID) noexcept
{
    if (ID < m_nextAllocatedID && std::find(m_textureIDFreeList.cbegin(), m_textureIDFreeList.cend(), ID) == m_textureIDFreeList.cend()) {
        m_textureIDFreeList.emplace_back(ID);
    }
}


void TextureManager::InitializeSamplers() noexcept
{
    std::array<TextureSamplerStateCreateInfo, COMMON_SMP_COUNT> samplerStateCreateInfos;
    std::array<ds::StrID, COMMON_SMP_COUNT> samplerDbgNames;

    samplerStateCreateInfos[COMMON_SMP_REPEAT_NEAREST_IDX].wrapModeS = GL_REPEAT;
    samplerStateCreateInfos[COMMON_SMP_REPEAT_NEAREST_IDX].wrapModeT = GL_REPEAT;
    samplerStateCreateInfos[COMMON_SMP_REPEAT_NEAREST_IDX].wrapModeR = GL_REPEAT;
    samplerStateCreateInfos[COMMON_SMP_REPEAT_NEAREST_IDX].minFiltering = GL_NEAREST;
    samplerStateCreateInfos[COMMON_SMP_REPEAT_NEAREST_IDX].magFiltering = GL_NEAREST;

    samplerStateCreateInfos[COMMON_SMP_REPEAT_MIP_NEAREST_IDX].wrapModeS = GL_REPEAT;
    samplerStateCreateInfos[COMMON_SMP_REPEAT_MIP_NEAREST_IDX].wrapModeT = GL_REPEAT;
    samplerStateCreateInfos[COMMON_SMP_REPEAT_MIP_NEAREST_IDX].wrapModeR = GL_REPEAT;
    samplerStateCreateInfos[COMMON_SMP_REPEAT_MIP_NEAREST_IDX].minFiltering = GL_NEAREST_MIPMAP_NEAREST;
    samplerStateCreateInfos[COMMON_SMP_REPEAT_MIP_NEAREST_IDX].magFiltering = GL_NEAREST;

    samplerStateCreateInfos[COMMON_SMP_REPEAT_LINEAR_IDX].wrapModeS = GL_REPEAT;
    samplerStateCreateInfos[COMMON_SMP_REPEAT_LINEAR_IDX].wrapModeT = GL_REPEAT;
    samplerStateCreateInfos[COMMON_SMP_REPEAT_LINEAR_IDX].wrapModeR = GL_REPEAT;
    samplerStateCreateInfos[COMMON_SMP_REPEAT_LINEAR_IDX].minFiltering = GL_LINEAR;
    samplerStateCreateInfos[COMMON_SMP_REPEAT_LINEAR_IDX].magFiltering = GL_LINEAR;

    samplerStateCreateInfos[COMMON_SMP_REPEAT_MIP_LINEAR_IDX].wrapModeS = GL_REPEAT;
    samplerStateCreateInfos[COMMON_SMP_REPEAT_MIP_LINEAR_IDX].wrapModeT = GL_REPEAT;
    samplerStateCreateInfos[COMMON_SMP_REPEAT_MIP_LINEAR_IDX].wrapModeR = GL_REPEAT;
    samplerStateCreateInfos[COMMON_SMP_REPEAT_MIP_LINEAR_IDX].minFiltering = GL_LINEAR_MIPMAP_LINEAR;
    samplerStateCreateInfos[COMMON_SMP_REPEAT_MIP_LINEAR_IDX].magFiltering = GL_LINEAR;

    samplerStateCreateInfos[COMMON_SMP_MIRRORED_NEAREST_IDX].wrapModeS = GL_MIRRORED_REPEAT;
    samplerStateCreateInfos[COMMON_SMP_MIRRORED_NEAREST_IDX].wrapModeT = GL_MIRRORED_REPEAT;
    samplerStateCreateInfos[COMMON_SMP_MIRRORED_NEAREST_IDX].wrapModeR = GL_MIRRORED_REPEAT;
    samplerStateCreateInfos[COMMON_SMP_MIRRORED_NEAREST_IDX].minFiltering = GL_NEAREST;
    samplerStateCreateInfos[COMMON_SMP_MIRRORED_NEAREST_IDX].magFiltering = GL_NEAREST;

    samplerStateCreateInfos[COMMON_SMP_MIRRORED_MIP_NEAREST_IDX].wrapModeS = GL_MIRRORED_REPEAT;
    samplerStateCreateInfos[COMMON_SMP_MIRRORED_MIP_NEAREST_IDX].wrapModeT = GL_MIRRORED_REPEAT;
    samplerStateCreateInfos[COMMON_SMP_MIRRORED_MIP_NEAREST_IDX].wrapModeR = GL_MIRRORED_REPEAT;
    samplerStateCreateInfos[COMMON_SMP_MIRRORED_MIP_NEAREST_IDX].minFiltering = GL_NEAREST_MIPMAP_NEAREST;
    samplerStateCreateInfos[COMMON_SMP_MIRRORED_MIP_NEAREST_IDX].magFiltering = GL_NEAREST;

    samplerStateCreateInfos[COMMON_SMP_MIRRORED_LINEAR_IDX].wrapModeS = GL_MIRRORED_REPEAT;
    samplerStateCreateInfos[COMMON_SMP_MIRRORED_LINEAR_IDX].wrapModeT = GL_MIRRORED_REPEAT;
    samplerStateCreateInfos[COMMON_SMP_MIRRORED_LINEAR_IDX].wrapModeR = GL_MIRRORED_REPEAT;
    samplerStateCreateInfos[COMMON_SMP_MIRRORED_LINEAR_IDX].minFiltering = GL_LINEAR;
    samplerStateCreateInfos[COMMON_SMP_MIRRORED_LINEAR_IDX].magFiltering = GL_LINEAR;

    samplerStateCreateInfos[COMMON_SMP_MIRRORED_MIP_LINEAR_IDX].wrapModeS = GL_MIRRORED_REPEAT;
    samplerStateCreateInfos[COMMON_SMP_MIRRORED_MIP_LINEAR_IDX].wrapModeT = GL_MIRRORED_REPEAT;
    samplerStateCreateInfos[COMMON_SMP_MIRRORED_MIP_LINEAR_IDX].wrapModeR = GL_MIRRORED_REPEAT;
    samplerStateCreateInfos[COMMON_SMP_MIRRORED_MIP_LINEAR_IDX].minFiltering = GL_LINEAR_MIPMAP_LINEAR;
    samplerStateCreateInfos[COMMON_SMP_MIRRORED_MIP_LINEAR_IDX].magFiltering = GL_LINEAR;

    samplerStateCreateInfos[COMMON_SMP_CLAMP_NEAREST_IDX].wrapModeS = GL_CLAMP_TO_EDGE;
    samplerStateCreateInfos[COMMON_SMP_CLAMP_NEAREST_IDX].wrapModeT = GL_CLAMP_TO_EDGE;
    samplerStateCreateInfos[COMMON_SMP_CLAMP_NEAREST_IDX].wrapModeR = GL_CLAMP_TO_EDGE;
    samplerStateCreateInfos[COMMON_SMP_CLAMP_NEAREST_IDX].minFiltering = GL_NEAREST;
    samplerStateCreateInfos[COMMON_SMP_CLAMP_NEAREST_IDX].magFiltering = GL_NEAREST;

    samplerStateCreateInfos[COMMON_SMP_CLAMP_MIP_NEAREST_IDX].wrapModeS = GL_CLAMP_TO_EDGE;
    samplerStateCreateInfos[COMMON_SMP_CLAMP_MIP_NEAREST_IDX].wrapModeT = GL_CLAMP_TO_EDGE;
    samplerStateCreateInfos[COMMON_SMP_CLAMP_MIP_NEAREST_IDX].wrapModeR = GL_CLAMP_TO_EDGE;
    samplerStateCreateInfos[COMMON_SMP_CLAMP_MIP_NEAREST_IDX].minFiltering = GL_NEAREST_MIPMAP_NEAREST;
    samplerStateCreateInfos[COMMON_SMP_CLAMP_MIP_NEAREST_IDX].magFiltering = GL_NEAREST;

    samplerStateCreateInfos[COMMON_SMP_CLAMP_LINEAR_IDX].wrapModeS = GL_CLAMP_TO_EDGE;
    samplerStateCreateInfos[COMMON_SMP_CLAMP_LINEAR_IDX].wrapModeT = GL_CLAMP_TO_EDGE;
    samplerStateCreateInfos[COMMON_SMP_CLAMP_LINEAR_IDX].wrapModeR = GL_CLAMP_TO_EDGE;
    samplerStateCreateInfos[COMMON_SMP_CLAMP_LINEAR_IDX].minFiltering = GL_LINEAR;
    samplerStateCreateInfos[COMMON_SMP_CLAMP_LINEAR_IDX].magFiltering = GL_LINEAR;

    samplerStateCreateInfos[COMMON_SMP_CLAMP_MIP_LINEAR_IDX].wrapModeS = GL_CLAMP_TO_EDGE;
    samplerStateCreateInfos[COMMON_SMP_CLAMP_MIP_LINEAR_IDX].wrapModeT = GL_CLAMP_TO_EDGE;
    samplerStateCreateInfos[COMMON_SMP_CLAMP_MIP_LINEAR_IDX].wrapModeR = GL_CLAMP_TO_EDGE;
    samplerStateCreateInfos[COMMON_SMP_CLAMP_MIP_LINEAR_IDX].minFiltering = GL_LINEAR_MIPMAP_LINEAR;
    samplerStateCreateInfos[COMMON_SMP_CLAMP_MIP_LINEAR_IDX].magFiltering = GL_LINEAR;

#if defined(ENG_DEBUG)
    samplerDbgNames[COMMON_SMP_REPEAT_NEAREST_IDX] = "repeat_nearest";
    samplerDbgNames[COMMON_SMP_REPEAT_MIP_NEAREST_IDX] = "repeat_mip_nearest";
    samplerDbgNames[COMMON_SMP_REPEAT_LINEAR_IDX] = "repeat_linear";
    samplerDbgNames[COMMON_SMP_REPEAT_MIP_LINEAR_IDX] = "repeat_mip_linear";
    samplerDbgNames[COMMON_SMP_MIRRORED_NEAREST_IDX] = "mirrored_nearest";
    samplerDbgNames[COMMON_SMP_MIRRORED_MIP_NEAREST_IDX] = "mirrored_mip_nearest";
    samplerDbgNames[COMMON_SMP_MIRRORED_LINEAR_IDX] = "mirrored_linear";
    samplerDbgNames[COMMON_SMP_MIRRORED_MIP_LINEAR_IDX] = "mirrored_mip_linear";
    samplerDbgNames[COMMON_SMP_CLAMP_NEAREST_IDX] = "clamp_nearest";
    samplerDbgNames[COMMON_SMP_CLAMP_MIP_NEAREST_IDX] = "clamp_mip_nearest";
    samplerDbgNames[COMMON_SMP_CLAMP_LINEAR_IDX] = "clamp_linear";
    samplerDbgNames[COMMON_SMP_CLAMP_MIP_LINEAR_IDX] = "clamp_mip_linear";
#endif

    m_textureSamplersStorage.resize(COMMON_SMP_COUNT);

    for (uint32_t samplerIdx = 0; samplerIdx < COMMON_SMP_COUNT; ++samplerIdx) {
        const TextureSamplerStateCreateInfo* pCreateInfo = &samplerStateCreateInfos[samplerIdx];

        ENG_MAYBE_UNUSED bool samplerIniitalized = m_textureSamplersStorage[samplerIdx].Init(pCreateInfo, samplerDbgNames[samplerIdx]);
        ENG_ASSERT(samplerIniitalized, "Sampler \'{}\' initialization failed", samplerDbgNames[samplerIdx].CStr());
    }
}


void TextureManager::DestroySamplers() noexcept
{
    for (uint32_t samplerIdx = 0; samplerIdx < COMMON_SMP_COUNT; ++samplerIdx) {
        m_textureSamplersStorage[samplerIdx].Destroy();
    }
}


bool TextureManager::IsInitialized() const noexcept
{
    return m_isInitialized;
}


uint64_t amHash(const Texture& texture) noexcept
{
    return texture.Hash();
}


bool engInitTextureManager() noexcept
{   
    if (engIsTextureManagerInitialized()) {
        ENG_LOG_WARN("Texture manager is already initialized!");
        return true;
    }

    pTextureMngInst = std::unique_ptr<TextureManager>(new TextureManager);

    if (!pTextureMngInst) {
        ENG_ASSERT_FAIL("Failed to allocate memory for texture manager");
        return false;
    }

    if (!pTextureMngInst->Init()) {
        ENG_ASSERT_FAIL("Failed to initialized texture manager");
        return false;
    }

    return true;
}


void engTerminateTextureManager() noexcept
{
    pTextureMngInst = nullptr;
}


bool engIsTextureManagerInitialized() noexcept
{
    return pTextureMngInst && pTextureMngInst->IsInitialized();
}
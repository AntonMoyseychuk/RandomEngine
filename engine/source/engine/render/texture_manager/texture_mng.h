#pragma once

#include "utils/data_structures/strid.h"

#include <deque>
#include <vector>


class TextureSampler
{
    friend class TextureManager;

public:
    TextureSampler(const TextureSampler& other) = delete;
    TextureSampler& operator=(const TextureSampler& other) = delete;
    TextureSampler(TextureSampler&& other) noexcept = delete;
    TextureSampler& operator=(TextureSampler&& other) noexcept = delete;

    // ~TextureSampler() { Destroy(); }

private:
    // TextureSampler() = default;

    bool Init() noexcept;
    void Destroy() noexcept;

    bool IsValid() const noexcept { return m_renderID != 0; }

    uint32_t GetRenderID() const noexcept { return m_renderID; }

private:
    uint32_t m_renderID = 0;
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
    
    FORMAT_INVALID,
    FORMAT_COUNT = FORMAT_INVALID,
};


enum class TextureInputDataFormat
{
    INPUT_FORMAT_R,
    INPUT_FORMAT_RG,
    INPUT_FORMAT_RGB,
    INPUT_FORMAT_BGR,
    INPUT_FORMAT_RGBA, 
    INPUT_FORMAT_DEPTH,
    INPUT_FORMAT_STENCIL,
    
    INPUT_FORMAT_INVALID,
    INPUT_FORMAT_COUNT = INPUT_FORMAT_INVALID,
};


enum class TextureInputDataType
{
    INPUT_TYPE_UNSIGNED_BYTE,
    INPUT_TYPE_BYTE,
    INPUT_TYPE_UNSIGNED_SHORT,
    INPUT_TYPE_SHORT,
    INPUT_TYPE_UNSIGNED_INT,
    INPUT_TYPE_INT,
    INPUT_TYPE_FLOAT,
    
    INPUT_TYPE_INVALID,
    INPUT_TYPE_COUNT = INPUT_TYPE_INVALID,
};


struct TextureInputData
{
    const void* pData = nullptr;

    TextureInputDataFormat format = TextureInputDataFormat::INPUT_FORMAT_INVALID;
    TextureInputDataType dataType = TextureInputDataType::INPUT_TYPE_INVALID;
};


struct Texture2DCreateInfo
{
    TextureInputData inputData = {};
    TextureFormat format;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t mipmapsCount = 0;
};


class Texture
{
    friend class TextureManager;

public:
    Texture(const Texture& other) = delete;
    Texture& operator=(const Texture& other) = delete;

    Texture() = default;
    ~Texture() { Destroy(); }

    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    bool IsValid() const noexcept;

    bool IsType1D() const noexcept;
    bool IsType2D() const noexcept;
    bool IsType3D() const noexcept;
    bool IsTypeBuffer() const noexcept;
    bool IsTypeCubeMap() const noexcept;
    bool IsTypeArray1D() const noexcept;
    bool IsTypeArray2D() const noexcept;
    bool IsTypeCubeMapArray() const noexcept;
    bool IsTypeMultisample2D() const noexcept;
    bool IsTypeMultisampleArray2D() const noexcept;

    uint64_t Hash() const noexcept;

    uint32_t GetRenderID() const noexcept { return m_renderID; }

private:
    bool Init(ds::StrID name, const Texture2DCreateInfo& createInfo) noexcept;
    void Destroy() noexcept;

private:
    ds::StrID m_name = "";
    
    uint16_t m_type = 0;
    uint16_t m_levelsCount = 0;
    
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    uint32_t m_depth = 0;

    uint32_t m_renderID = 0;
};


using TextureID = uint64_t;

inline constexpr TextureID TEXTURE_ID_INVALID = UINT64_MAX;


class TextureManager
{
    friend bool engInitTextureManager() noexcept;
    friend void engTerminateTextureManager() noexcept;
    friend bool engIsTextureManagerInitialized() noexcept;

public:
    static TextureManager& GetInstance() noexcept;

public:
    TextureManager(const TextureManager& other) = delete;
    TextureManager& operator=(const TextureManager& other) = delete;

    ~TextureManager();

    TextureID AllocateTexture2D(ds::StrID name, const Texture2DCreateInfo& createInfo) noexcept;

    Texture* GetTextureByID(const TextureID& ID) noexcept;
    Texture* GetTextureByName(ds::StrID name) noexcept;
    
    void DeallocateTexture(ds::StrID name);
    void DeallocateTexture(const TextureID& ID);

    bool IsValidTextureID(const TextureID& ID) const noexcept;
    
private:
    TextureManager() = default;

    TextureManager(TextureManager&& other) noexcept = default;
    TextureManager& operator=(TextureManager&& other) noexcept = default;

    bool Init() noexcept;
    void Terminate() noexcept;

    TextureID AllocateTextureID() noexcept;
    void DeallocateTextureID(const TextureID& ID) noexcept;

    void InitializeSamplers() noexcept;
    void DestroySamplers() noexcept;

    bool IsInitialized() const noexcept;

private:
    std::vector<Texture> m_texturesStorage;

    std::vector<ds::StrID> m_textureIDToNameVector;
    std::unordered_map<ds::StrID, TextureID> m_textureNameToIDMap;

    std::deque<TextureID> m_textureIDFreeList;

    TextureID m_nextAllocatedID = 0;

    bool m_isInitialized = false;
};


uint64_t amHash(const Texture& texture) noexcept;


bool engInitTextureManager() noexcept;
void engTerminateTextureManager() noexcept;
bool engIsTextureManagerInitialized() noexcept;
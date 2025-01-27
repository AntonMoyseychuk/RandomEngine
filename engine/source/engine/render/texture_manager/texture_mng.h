#pragma once

#include "utils/data_structures/strid.h"
#include "utils/data_structures/base_id.h"

#include "core.h"

#include <deque>
#include <vector>


class TextureSamplerState
{
    friend class TextureManager;

public:
    TextureSamplerState() = default;

    TextureSamplerState(const TextureSamplerState& other) = delete;
    TextureSamplerState& operator=(const TextureSamplerState& other) = delete;
    
    TextureSamplerState(TextureSamplerState&& other) noexcept;
    TextureSamplerState& operator=(TextureSamplerState&& other) noexcept;

    ~TextureSamplerState() { Destroy(); }

    void Bind(uint32_t unit) noexcept;

    bool IsValid() const noexcept { return m_renderID != 0; }

    uint32_t GetRenderID() const noexcept { return m_renderID; }

private:
    bool Init(const void* pCreateInfo, ds::StrID dbgName) noexcept;
    void Destroy() noexcept;

private:
#if defined(ENG_DEBUG)
    ds::StrID m_dbgName = "_INVALID_";
#endif

    uint32_t m_renderID = 0;
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
    uint32_t format; // Reflected from shader
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t mipmapsCount = 0;
};


using TextureID = BaseID<uint32_t>;


class Texture
{
    friend class TextureManager;

public:
    Texture() = default;
    ~Texture() { Destroy(); }

    Texture(const Texture& other) = delete;
    Texture& operator=(const Texture& other) = delete;

    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    bool Create(const Texture2DCreateInfo& createInfo) noexcept;
    void Destroy() noexcept;

    void Bind(uint32_t unit) noexcept;

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

    ds::StrID GetName() const noexcept { return m_name; }
    TextureID GetID() const noexcept { return m_ID; }
    uint32_t GetLevelsCount() const noexcept { return m_levelsCount; }
    uint32_t GetWidth() const noexcept { return m_width; }
    uint32_t GetHeight() const noexcept { return m_height; }
    uint32_t GetDepth() const noexcept { return m_depth; }
    uint32_t GetRenderID() const noexcept { return m_renderID; }

private:
    ds::StrID m_name = "_INVALID_";
    
    uint32_t m_type = 0;
    uint32_t m_levelsCount = 0;
    
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    uint32_t m_depth = 0;

    uint32_t m_renderID = 0;
    TextureID m_ID;
};


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
    TextureManager(TextureManager&& other) noexcept = delete;
    TextureManager& operator=(TextureManager&& other) noexcept = delete;

    ~TextureManager();

    Texture* RegisterTexture2D(ds::StrID name) noexcept;
    Texture* GetTextureByName(ds::StrID name) noexcept;
    
    void UnregisterTexture(ds::StrID name) noexcept;
    void UnregisterTexture(Texture* pTex) noexcept;

    TextureSamplerState* GetSampler(uint32_t samplerIdx) noexcept;

    bool IsValidSamplerIdx(uint32_t samplerIdx) const noexcept;
    
private:
    TextureManager() = default;

    bool Init() noexcept;
    void Terminate() noexcept;

    TextureID AllocateTextureID() noexcept;
    void DeallocateTextureID(TextureID ID) noexcept;

    void InitializeSamplers() noexcept;
    void DestroySamplers() noexcept;

    bool IsInitialized() const noexcept;

private:
    std::vector<TextureSamplerState> m_textureSamplersStorage;
    std::vector<Texture> m_texturesStorage;

    std::unordered_map<ds::StrID, uint64_t> m_textureNameToStorageIndexMap;

    std::deque<TextureID> m_textureIDFreeList;

    TextureID m_nextAllocatedID = TextureID{0};

    bool m_isInitialized = false;
};


uint64_t amHash(const Texture& texture) noexcept;


bool engInitTextureManager() noexcept;
void engTerminateTextureManager() noexcept;
bool engIsTextureManagerInitialized() noexcept;
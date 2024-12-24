#pragma once

#include "utils/data_structures/strid.h"

#include <deque>


enum class TextureType : uint32_t
{
    TYPE_1D,                   // 1-dimensional images.
    TYPE_2D,                   // 2-dimensional images.
    TYPE_3D,                   // 3-dimensional images.
    TYPE_RECTANGLE,            // 2-dimensional (only one image. No mipmapping). Texture coordinates used for these textures are not normalized.
    TYPE_BUFFER,               // 1-dimensional (only one image. No mipmapping). The storage for this data comes from a Buffer Object.
    TYPE_CUBE_MAP,             // There are exactly 6 distinct sets of 2D images, each image being of the same size and must be of a square size.
    TYPE_1D_ARRAY,             // It contains multiple sets of 1-dimensional images. The array length is part of the texture's size.
    TYPE_2D_ARRAY,             // It contains multiple sets of 2-dimensional images. The array length is part of the texture's size.
    TYPE_CUBE_MAP_ARRAY,       // It contains multiple sets of cube maps. The array length * 6 (number of cube faces) is part of the texture size.
    TYPE_2D_MULTISAMPLE,       // 2-dimensional (only one image. No mipmapping). Each pixel in these images contains multiple samples instead of just one value.
    TYPE_2D_MULTISAMPLE_ARRAY, // Combines 2D array and 2D multisample types. (No mipmapping).
    
    TYPE_INVALID,
    TYPE_COUNT = TYPE_INVALID,
};


struct TextureCreateInfo
{
    const void** pData = nullptr;
    const size_t* dataSizeInBytes = nullptr;

    TextureType type;

    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t depth = 0;
};


class TextureParams
{
    friend class TextureManager;
    friend class Texture;

public:
    uint64_t Hash() const noexcept;

private:
    ds::StrID m_name = "";
    
    uint32_t m_internalType = 0;
    
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    uint32_t m_depth = 0;
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

    void Bind(uint32_t unit = 0) const noexcept;
    void Unbind(uint32_t unit = 0) const noexcept;

    bool IsValid() const noexcept;

    uint64_t Hash() const noexcept;

    uint32_t GetRenderID() const noexcept { return m_renderID; }

private:
    bool Init(const TextureCreateInfo& createInfo) noexcept;
    void Destroy() noexcept;

private:
    const TextureParams* m_pParams = nullptr;

    uint32_t m_renderID = 0;
};


using TextureID = uint64_t;


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
    
private:
    TextureManager() = default;

    TextureManager(TextureManager&& other) noexcept = default;
    TextureManager& operator=(TextureManager&& other) noexcept = default;

    bool Init() noexcept;
    void Terminate() noexcept;

    TextureID AllocateProgramID() noexcept;
    void DeallocateProgramID(const TextureID& ID) noexcept;

    bool IsInitialized() const noexcept;

private:
    std::vector<Texture> m_texturesStorage;
    std::vector<Texture> m_textureParamsStorage;

    std::unordered_map<TextureID, ds::StrID> m_textureIDToNameMap;
    std::unordered_map<ds::StrID, TextureID> m_textureNameToIDMap;

    std::deque<TextureID> m_textureIDFreeList;

    TextureID m_nextAllocatedID = 0;

    bool m_isInitialized = false;
};


uint64_t amHash(const TextureParams& params) noexcept;
uint64_t amHash(const Texture& texture) noexcept;


bool engInitTextureManager() noexcept;
void engTerminateTextureManager() noexcept;
bool engIsTextureManagerInitialized() noexcept;
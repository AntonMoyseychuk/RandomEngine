#include "pch.h"
#include "texture_mng.h"

#include "utils/debug/assertion.h"
#include "utils/data_structures/hash.h"

#include "core.h"

#include "engine/render/platform/OpenGL/opengl_driver.h"


static constexpr size_t ENG_MAX_TEXTURES_COUNT = 4096; // TODO: make it configurable


static std::unique_ptr<TextureManager> g_pTextureMng = nullptr;


uint64_t TextureParams::Hash() const noexcept
{
    ds::HashBuilder builder;

    builder.AddValue(m_name);
    builder.AddValue(m_internalType);
    builder.AddValue(m_width);
    builder.AddValue(m_height);
    builder.AddValue(m_depth);

    return builder.Value();
}


Texture::Texture(Texture &&other) noexcept
{
    std::swap(m_pParams, other.m_pParams);
    std::swap(m_renderID, other.m_renderID);
}


Texture& Texture::operator=(Texture &&other) noexcept
{
    Destroy();

    std::swap(m_pParams, other.m_pParams);
    std::swap(m_renderID, other.m_renderID);

    return *this;
}


void Texture::Bind(uint32_t unit) const noexcept
{
    glBindTextureUnit(unit, m_renderID);
}


void Texture::Unbind(uint32_t unit) const noexcept
{
    drvUnbindTextureUnit(unit, m_renderID);
}


bool Texture::IsValid() const noexcept
{
    return m_pParams != nullptr && m_renderID != 0;
}


uint64_t Texture::Hash() const noexcept
{
    ds::HashBuilder builder;
    
    if (m_pParams) {
        builder.AddValue(*m_pParams);
    }

    builder.AddValue(m_renderID);

    return builder.Value();
}


bool Texture::Init(const TextureCreateInfo &createInfo) noexcept
{
    ENG_ASSERT_GRAPHICS_API_FAIL("{} is not implemented yet", __FUNCTION__);
    return false;
}


void Texture::Destroy() noexcept
{
    if (IsValid()) {
        m_pParams = nullptr;

        glDeleteTextures(1, &m_renderID);
        m_renderID = 0;
    }
}


TextureManager& TextureManager::GetInstance() noexcept
{
    ENG_ASSERT_GRAPHICS_API(engIsTextureManagerInitialized(), "Texture manager is not initialized");
    return *g_pTextureMng;
}


TextureManager::~TextureManager()
{
    Terminate();
}


bool TextureManager::Init() noexcept
{
    if (IsInitialized()) {
        return true;
    }

    m_texturesStorage.resize(ENG_MAX_TEXTURES_COUNT);
    m_textureParamsStorage.resize(ENG_MAX_TEXTURES_COUNT);

    m_isInitialized = true;

    return true;
}


void TextureManager::Terminate() noexcept
{
    m_texturesStorage.clear();
    m_textureParamsStorage.clear();

    m_textureIDToNameMap.clear();
    m_textureNameToIDMap.clear();

    m_textureIDFreeList.clear();

    m_nextAllocatedID = 0;

    m_isInitialized = false;
}


TextureID TextureManager::AllocateProgramID() noexcept
{
    if (m_textureIDFreeList.empty()) {
        TextureID textureID = m_nextAllocatedID;
        ++m_nextAllocatedID;

        return textureID;
    }

    TextureID textureID = m_textureIDFreeList.front();
    m_textureIDFreeList.pop_front();
        
    return textureID;
}


void TextureManager::DeallocateProgramID(const TextureID &ID) noexcept
{
    if (ID < m_nextAllocatedID && std::find(m_textureIDFreeList.cbegin(), m_textureIDFreeList.cend(), ID) == m_textureIDFreeList.cend()) {
        m_textureIDFreeList.emplace_back(ID);
    }
}


bool TextureManager::IsInitialized() const noexcept
{
    return m_isInitialized;
}


uint64_t amHash(const TextureParams& params) noexcept
{
    return params.Hash();
}


uint64_t amHash(const Texture& texture) noexcept
{
    return texture.Hash();
}


bool engInitTextureManager() noexcept
{   
    if (engIsTextureManagerInitialized()) {
        ENG_LOG_GRAPHICS_API_WARN("Texture manager is already initialized!");
        return true;
    }

    g_pTextureMng = std::unique_ptr<TextureManager>(new TextureManager);

    if (!g_pTextureMng) {
        ENG_ASSERT_GRAPHICS_API_FAIL("Failed to allocate memory for texture manager");
        return false;
    }

    if (!g_pTextureMng->Init()) {
        ENG_ASSERT_GRAPHICS_API_FAIL("Failed to initialized texture manager");
        return false;
    }

    return true;
}


void engTerminateTextureManager() noexcept
{
    g_pTextureMng = nullptr;
}


bool engIsTextureManagerInitialized() noexcept
{
    return g_pTextureMng && g_pTextureMng->IsInitialized();
}
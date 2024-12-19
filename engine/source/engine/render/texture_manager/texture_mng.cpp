#include "pch.h"
#include "texture_mng.h"

#include "utils/debug/assertion.h"

#include "engine/render/platform/OpenGL/opengl_driver.h"


static constexpr size_t ENG_PREALLOCATEDTEXTURES_COUNT = 4096; // TODO: make it configurable


static std::unique_ptr<TextureManager> g_pTextureMng = nullptr;


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

    m_isInitialized = true;

    return true;
}


void TextureManager::Terminate() noexcept
{
}


bool TextureManager::IsInitialized() const noexcept
{
    return m_isInitialized;
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

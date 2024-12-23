#pragma once

#include "utils/data_structures/strid.h"

#include <unordered_map>


class Texture
{
    friend class TextureManager;
public:

private:
    ds::StrID m_name = "";
    uint32_t m_renderID = 0;
};


using TextureID = ds::StrID;


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

    bool IsInitialized() const noexcept;

private:
    std::unordered_map<TextureID, Texture> m_textureStorage;

    bool m_isInitialized = false;
};


bool engInitTextureManager() noexcept;
void engTerminateTextureManager() noexcept;
bool engIsTextureManagerInitialized() noexcept;
#pragma once


class Texture
{
    friend class TextureManager;
public:

private:

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

    ~TextureManager();
    
private:
    TextureManager() = default;

    TextureManager(TextureManager&& other) noexcept = default;
    TextureManager& operator=(TextureManager&& other) noexcept = default;

    bool Init() noexcept;
    void Terminate() noexcept;

    bool IsInitialized() const noexcept;

private:
    bool m_isInitialized = false;
};


bool engInitTextureManager() noexcept;
void engTerminateTextureManager() noexcept;
bool engIsTextureManagerInitialized() noexcept;
#pragma once

#include "engine/render/texture_manager/texture_mng.h"

#include "core.h"


enum class RTTextureID : uint32_t
{
    RT_TEX_GBUFFER_ALBEDO,
    RT_TEX_GBUFFER_NORMAL,
    RT_TEX_GBUFFER_SPECULAR,

    RT_TEX_COMMON_DEPTH,

    RT_TEX_COUNT,
    RT_TEX_INVALID = RT_TEX_COUNT
};


enum class RTRenderBufferID : uint32_t
{
    RT_RENDERBUFFER_COMMON_DEPTH,

    RT_RENDERBUFFER_DEFAULT,

    RT_RENDERBUFFER_COUNT = RT_RENDERBUFFER_DEFAULT,
    RT_RENDERBUFFER_INVALID = RT_RENDERBUFFER_COUNT,
};


enum class RTFrameBufferID : uint32_t
{
    RT_FRAMEBUFFER_GBUFFER,

    RT_FRAMEBUFFER_DEFAULT,

    RT_FRAMEBUFFER_COUNT = RT_FRAMEBUFFER_DEFAULT,
    RT_FRAMEBUFFER_INVALID = RT_FRAMEBUFFER_COUNT,
};


enum class RenderBufferFormat : uint32_t
{
    FORMAT_INVALID,

    FORMAT_DEPTH_32,
    FORMAT_DEPTH_24,
    FORMAT_DEPTH_16,

    FORMAT_STENCIL_8,
    FORMAT_STENCIL_4,
    FORMAT_STENCIL_1,

    FORMAT_DEPTH_24_STENCIL_8,
    FORMAT_DEPTH_32_STENCIL_8,
};


enum class RenderBufferAttachmentType : uint32_t
{
    TYPE_DEPTH_ATTACHMENT,
    TYPE_STENCIL_ATTACHMENT,
    TYPE_DEPTH_STENCIL_ATTACHMENT,

    TYPE_COUNT,
    TYPE_INVALID = TYPE_COUNT,
};


struct RenderBufferCreateInfo
{
    RenderBufferFormat format = RenderBufferFormat::FORMAT_INVALID;
    uint32_t width = 0;
    uint32_t height = 0;
};


class RenderBuffer
{
    friend class RenderTargetManager;

public:
    RenderBuffer() = default;
    ~RenderBuffer() { Destroy(); }

    RenderBuffer(RenderBuffer&& other) noexcept;
    RenderBuffer& operator=(RenderBuffer&& other) noexcept;

    ds::StrID GetName() const noexcept;
    RenderBufferFormat GetFormat() const noexcept;
    uint32_t GetWidth() const noexcept { return m_width; }
    uint32_t GetHeight() const noexcept { return m_height; }
    uint32_t GetRenderID() const noexcept { return m_renderID; }

    bool IsValid() const noexcept { return m_renderID != 0; }

private:
    RenderBuffer(const RenderBuffer& other) = delete;
    RenderBuffer& operator=(const RenderBuffer& other) = delete;

    bool Init(ds::StrID dbgName, const RenderBufferCreateInfo& createInfo) noexcept;
    void Destroy() noexcept;

    bool Recreate(ds::StrID dbgName, const RenderBufferCreateInfo& createInfo) noexcept;

private:
#if defined(ENG_DEBUG)
    ds::StrID m_dbgName = "";
#endif
    uint32_t m_format = 0;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    uint32_t m_renderID = 0;
};


struct ColorAttachment
{
    Texture* pTexure = nullptr;
    RTTextureID id = RTTextureID::RT_TEX_INVALID;
    uint32_t index = UINT32_MAX;
};


struct RenderBufferAttachment
{
    RenderBuffer* pRenderBuffer = nullptr;
    RTRenderBufferID id = RTRenderBufferID::RT_RENDERBUFFER_INVALID;
    RenderBufferAttachmentType type = RenderBufferAttachmentType::TYPE_INVALID;
};


struct FramebufferCreateInfo
{
    const ColorAttachment* pColorAttachments = nullptr;
    const RenderBufferAttachment* pRenderBufferAttachments = nullptr;

    uint32_t colorAttachmentsCount = 0;
    uint32_t renderBufferAttachmentsCount = 0;

    RTFrameBufferID id = RTFrameBufferID::RT_FRAMEBUFFER_INVALID;
};


class FrameBuffer
{
    friend class RenderTargetManager;

public:
    FrameBuffer(const FrameBuffer& other) = delete;
    FrameBuffer& operator=(const FrameBuffer& other) = delete;

    FrameBuffer() = default;
    ~FrameBuffer();

    FrameBuffer(FrameBuffer&& other) noexcept;
    FrameBuffer& operator=(FrameBuffer&& other) noexcept;

    void Bind() noexcept;

    bool IsValid() const noexcept;

    ds::StrID GetName() const noexcept;
    RTFrameBufferID GetID() const noexcept;
    uint32_t GetRenderID() const noexcept { return m_renderID; }

private:
    bool Init(ds::StrID dbgName, const FramebufferCreateInfo& createInfo) noexcept;
    void Destroy() noexcept;

    bool Recreate(ds::StrID dbgName, const FramebufferCreateInfo& createInfo) noexcept;

    bool CheckCompleteStatus() const noexcept;

private:
#if defined(ENG_DEBUG)
    std::vector<ColorAttachment> m_colorAttachments;
    std::vector<RenderBufferAttachment> m_renderBufferAttachments;

    ds::StrID m_dbgName = "";
    RTFrameBufferID m_ID = RTFrameBufferID::RT_FRAMEBUFFER_INVALID;
#endif

    uint32_t m_renderID = 0;
};


class RenderTargetManager
{
    friend bool engInitRenderTargetManager() noexcept;
    friend void engTerminateRenderTargetManager() noexcept;
    friend bool engIsRenderTargetManagerInitialized() noexcept;

public:
    static RenderTargetManager& GetInstance() noexcept;

public:
    ~RenderTargetManager();

    RenderTargetManager(const RenderTargetManager& other) = delete;
    RenderTargetManager& operator=(const RenderTargetManager& other) = delete;

    Texture* GetRTTexture(RTTextureID texID) noexcept;
    void BindFramebuffer(RTFrameBufferID framebufferID) noexcept;

private:
    RenderTargetManager() = default;

    RenderTargetManager(RenderTargetManager&& other) noexcept = default;
    RenderTargetManager& operator=(RenderTargetManager&& other) noexcept = default;

    bool Init() noexcept;
    void Terminate() noexcept;

    void ClearFrameBuffers() noexcept;

    void OnWindowResizedEvent(uint32_t width, uint32_t height) noexcept;
    void RecreateFrameBuffers(uint32_t width, uint32_t height) noexcept;

    bool IsInitialized() const noexcept { return m_isInitialized; }

private:
    std::vector<FrameBuffer> m_frameBuffers;

    std::vector<Texture*> m_RTTextures;
    std::vector<RenderBuffer> m_RTRenderBuffers;

    bool m_isInitialized = false;
};


bool engInitRenderTargetManager() noexcept;
void engTerminateRenderTargetManager() noexcept;
bool engIsRenderTargetManagerInitialized() noexcept;
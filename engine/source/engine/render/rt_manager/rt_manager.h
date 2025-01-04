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


enum class RTFrameBufferID : uint32_t
{
    RT_FRAMEBUFFER_GBUFFER,

    RT_FRAMEBUFFER_DEFAULT,

    RT_FRAMEBUFFER_COUNT = RT_FRAMEBUFFER_DEFAULT,
    RT_FRAMEBUFFER_INVALID = RT_FRAMEBUFFER_COUNT,
};


enum class FrameBufferAttachmentType : uint32_t
{
    TYPE_COLOR_ATTACHMENT,
    TYPE_DEPTH_ATTACHMENT,
    TYPE_STENCIL_ATTACHMENT,
    TYPE_DEPTH_STENCIL_ATTACHMENT,

    TYPE_COUNT,
    TYPE_INVALID = TYPE_COUNT,
};


struct FrameBufferAttachment
{
    Texture* pTexure = nullptr;
    FrameBufferAttachmentType type = FrameBufferAttachmentType::TYPE_INVALID;
    
    uint32_t index = UINT32_MAX; // Ignores if type is not TYPE_COLOR_ATTACHMENT
};


struct FramebufferCreateInfo
{
    const FrameBufferAttachment* pAttachments = nullptr;
    uint32_t attachmentsCount = 0;
    RTFrameBufferID ID = RTFrameBufferID::RT_FRAMEBUFFER_INVALID;
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
    RTFrameBufferID GetID() const noexcept { return m_ID; }
    uint32_t GetRenderID() const noexcept { return m_renderID; }

private:
    bool Init(ds::StrID dbgName, const FramebufferCreateInfo& createInfo) noexcept;
    void Destroy() noexcept;

    bool Recreate(ds::StrID dbgName, const FramebufferCreateInfo& createInfo) noexcept;

    bool CheckCompleteStatus() const noexcept;

private:
#if defined(ENG_DEBUG)
    std::vector<FrameBufferAttachment> m_attachments;
    ds::StrID m_dbgName = "";
#endif

    RTFrameBufferID m_ID = RTFrameBufferID::RT_FRAMEBUFFER_INVALID;
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

    bool m_isInitialized = false;
};


bool engInitRenderTargetManager() noexcept;
void engTerminateRenderTargetManager() noexcept;
bool engIsRenderTargetManagerInitialized() noexcept;
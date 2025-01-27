#pragma once

#include "render/texture_manager/texture_mng.h"
#include "event_system/event_dispatcher.h"

#include "core.h"


enum class RTTextureID : uint32_t
{
    RT_TEX_GBUFFER_ALBEDO,
    RT_TEX_GBUFFER_NORMAL,
    RT_TEX_GBUFFER_SPECULAR,

    RT_TEX_COMMON_DEPTH,

    RT_TEX_COUNT,
    RT_TEX_INVALID,
};


enum class RTFrameBufferID : uint32_t
{
    RT_FRAMEBUFFER_DEFAULT,
    RT_FRAMEBUFFER_GBUFFER,

    RT_FRAMEBUFFER_COUNT,
    RT_FRAMEBUFFER_INVALID,
};


enum class FrameBufferAttachmentType : uint32_t
{
    TYPE_COLOR_ATTACHMENT,
    TYPE_DEPTH_ATTACHMENT,
    TYPE_STENCIL_ATTACHMENT,
    TYPE_DEPTH_STENCIL_ATTACHMENT,

    TYPE_COUNT,
    TYPE_INVALID
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
    FrameBuffer() = default;
    ~FrameBuffer();

    FrameBuffer(const FrameBuffer& other) = delete;
    FrameBuffer& operator=(const FrameBuffer& other) = delete;

    FrameBuffer(FrameBuffer&& other) noexcept;
    FrameBuffer& operator=(FrameBuffer&& other) noexcept;

    void Bind() noexcept;

    void Clear(float r, float g, float b, float a, float depth, int32_t stencil) noexcept;
    void ClearColor(uint32_t index, float r, float g, float b, float a) noexcept;
    void ClearDepth(float depth) noexcept;
    void ClearStencil(int32_t stencil) noexcept;
    void ClearDepthStencil(float depth, int32_t stencil) noexcept;

    bool IsValid() const noexcept;

    bool HasDepthAttachment() const noexcept { return GetDepthAttachmentCount() > 0; }
    bool HasStencilAttachment() const noexcept { return GetStencilAttachmentCount() > 0; }

    ds::StrID GetName() const noexcept;
    
    uint32_t GetColorAttachmentsCount() const noexcept { return m_attachmentsState.colorAttachmentsCount; }
    uint32_t GetDepthAttachmentCount() const noexcept { return m_attachmentsState.depthAttachmentsCount; }
    uint32_t GetStencilAttachmentCount() const noexcept { return m_attachmentsState.stencilAttachmentsCount; }
    uint32_t GetAttachmentsCount() const noexcept { return GetColorAttachmentsCount() + GetDepthAttachmentCount() + GetStencilAttachmentCount(); }
    
    RTFrameBufferID GetID() const noexcept { return m_ID; }
    uint32_t GetRenderID() const noexcept { return m_renderID; }

    uint64_t Hash() const noexcept;

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

    struct AttachmentsState
    {
        uint32_t colorAttachmentsCount : 30;
        uint32_t depthAttachmentsCount : 1;
        uint32_t stencilAttachmentsCount : 1;
    } m_attachmentsState;

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
    RenderTargetManager(RenderTargetManager&& other) noexcept = delete;
    RenderTargetManager& operator=(RenderTargetManager&& other) noexcept = delete;

    Texture* GetRTTexture(RTTextureID texID) noexcept;
    FrameBuffer* GetFrameBuffer(RTFrameBufferID framebufferID) noexcept;
    void BindFrameBuffer(RTFrameBufferID framebufferID) noexcept;

    void ClearFrameBuffer(RTFrameBufferID framebufferID, float r, float g, float b, float a, float depth, int32_t stencil) noexcept;
    void ClearFrameBufferColor(RTFrameBufferID framebufferID, uint32_t index, float r, float g, float b, float a) noexcept;
    void ClearFrameBufferDepth(RTFrameBufferID framebufferID, float depth) noexcept;
    void ClearFrameBufferStencil(RTFrameBufferID framebufferID, int32_t stencil) noexcept;
    void ClearFrameBufferDepthStencil(RTFrameBufferID framebufferID, float depth, int32_t stencil) noexcept;

private:
    RenderTargetManager() = default;

    bool Init() noexcept;
    void Terminate() noexcept;

    void ClearFrameBuffersStorage() noexcept;

    void OnWindowResizedEvent(uint32_t width, uint32_t height) noexcept;
    void RecreateFrameBuffers(uint32_t width, uint32_t height) noexcept;

    bool IsInitialized() const noexcept { return m_isInitialized; }

private:
    std::vector<FrameBuffer> m_frameBufferStorage;
    std::vector<Texture*> m_RTTextureStorage;

    EventListenerID m_frameBufferResizeEventListenerID;

    bool m_isInitialized = false;
};


uint64_t amHash(const FrameBuffer& frameBuffer) noexcept;


bool engInitRenderTargetManager() noexcept;
void engTerminateRenderTargetManager() noexcept;
bool engIsRenderTargetManagerInitialized() noexcept;
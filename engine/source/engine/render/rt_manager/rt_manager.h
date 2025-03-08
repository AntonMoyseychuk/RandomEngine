#pragma once

#include "render/texture_manager/texture_mng.h"
#include "core/event_system/event_dispatcher.h"

#include "core.h"


enum class RTTextureID : uint32_t
{
    GBUFFER_ALBEDO,
    GBUFFER_NORMAL,
    GBUFFER_SPECULAR,

    COMMON_DEPTH,

    COMMON_COLOR,

    COUNT,
    INVALID,
};


enum class RTFrameBufferID : uint16_t
{
    GBUFFER,
    POST_PROCESS,

    COUNT,
    INVALID,
};


enum class FrameBufferAttachmentType : uint32_t
{
    COLOR_ATTACHMENT,
    DEPTH_ATTACHMENT,
    STENCIL_ATTACHMENT,
    DEPTH_STENCIL_ATTACHMENT,

    COUNT,
    INVALID
};


struct FrameBufferAttachment
{
    Texture*                  pTexure;
    FrameBufferAttachmentType type;
    uint32_t                  index; // Ignores if type is not COLOR_ATTACHMENT
};


struct FramebufferCreateInfo
{
    const FrameBufferAttachment* pAttachments;
    uint32_t                     attachmentsCount;
    RTFrameBufferID              ID;
};


class FrameBuffer
{
    friend class RenderTargetManager;

public:
    static constexpr size_t GetMaxColorAttachmentsCount() noexcept { return MAX_COLOR_ATTACHMENTS; }
    static constexpr size_t GetMaxDepthAttachmentsCount() noexcept { return MAX_DEPTH_ATTACHMENTS; }
    static constexpr size_t GetMaxStencilAttachmentsCount() noexcept { return MAX_STENCIL_ATTACHMENTS; }
    static constexpr size_t GetMaxDepthStencilAttachmentsCount() noexcept { return MAX_DEPTH_STENCIL_ATTACHMENTS; }
    static constexpr size_t GetMaxAttachmentsCount() noexcept { return MAX_ATTACHMENTS; }

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

    bool HasMergedDepthStencilAttachment() const noexcept { return m_attachmentsState.hasMergedDepthStencilAttachement; }

    void SetDebugName(ds::StrID name) noexcept;
    ds::StrID GetDebugName() const noexcept;
    
    uint32_t GetColorAttachmentsCount() const noexcept { return m_attachmentsState.colorAttachmentsCount; }
    uint32_t GetDepthAttachmentCount() const noexcept { return m_attachmentsState.depthAttachmentsCount; }
    uint32_t GetStencilAttachmentCount() const noexcept { return m_attachmentsState.stencilAttachmentsCount; }
    uint32_t GetAttachmentsCount() const noexcept;
    
    RTFrameBufferID GetID() const noexcept { return m_ID; }
    uint32_t GetRenderID() const noexcept { return m_renderID; }

    uint64_t Hash() const noexcept;

private:
    bool Create(const FramebufferCreateInfo& createInfo) noexcept;
    void Destroy() noexcept;

    bool Recreate(const FramebufferCreateInfo& createInfo) noexcept;

    bool CheckCompleteStatus() const noexcept;

    bool IsValidID() const noexcept;

private:
    static constexpr size_t MAX_COLOR_ATTACHMENTS         = 8;
    static constexpr size_t MAX_DEPTH_ATTACHMENTS         = 1;
    static constexpr size_t MAX_STENCIL_ATTACHMENTS       = 1;
    static constexpr size_t MAX_DEPTH_STENCIL_ATTACHMENTS = 1;
    static constexpr size_t MAX_ATTACHMENTS = MAX_COLOR_ATTACHMENTS + MAX_DEPTH_ATTACHMENTS + MAX_STENCIL_ATTACHMENTS;

private:
    struct AttachmentsState
    {
        uint16_t colorAttachmentsCount : 13;
        uint16_t depthAttachmentsCount : 1;
        uint16_t stencilAttachmentsCount : 1;
        uint16_t hasMergedDepthStencilAttachement : 1;
    };

    static_assert(sizeof(AttachmentsState) == sizeof(uint16_t));

#if defined(ENG_DEBUG)
    std::array<FrameBufferAttachment, MAX_ATTACHMENTS> m_attachments;
    ds::StrID m_dbgName = "_INVALID_";
#endif

    uint32_t         m_renderID = 0;
    AttachmentsState m_attachmentsState;
    RTFrameBufferID  m_ID = RTFrameBufferID::INVALID;
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
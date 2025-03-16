#include "pch.h"
#include "rt_manager.h"

#include "render/platform/OpenGL/opengl_driver.h"

#include "engine/engine.h"
#include "core/window_system/window_system.h"

#include "utils/data_structures/hash.h"

#include "auto/registers_common.h"


static std::unique_ptr<RenderTargetManager> pRenderTargetMngInst = nullptr;


static bool IsValidRenderTargetFrameBufferID(RTFrameBufferID ID) noexcept
{
    return ID < RTFrameBufferID::COUNT;
}


static void ClearFrameBufferColorInternal(uint32_t renderID, uint32_t index, const float* pColor) noexcept
{
    glClearNamedFramebufferfv(renderID, GL_COLOR, index, pColor);
}


static void ClearFrameBufferDepthInternal(uint32_t renderID, float depth) noexcept
{
    glClearNamedFramebufferfv(renderID, GL_DEPTH, 0, &depth);
}


static void ClearFrameBufferStencilInternal(uint32_t renderID, int32_t stencil) noexcept
{
    glClearNamedFramebufferiv(renderID, GL_STENCIL, 0, &stencil);
}


static void ClearFrameBufferDepthStencilInternal(uint32_t renderID, float depth, int32_t stencil) noexcept
{
    glClearNamedFramebufferfi(renderID, GL_DEPTH_STENCIL, 0, depth, stencil);
}


static void ClearFrameBufferInternal(uint32_t renderID, uint32_t colorAttachmentsCount, const float* pColor, const float *depthOpt, const int32_t *stencilOpt) noexcept
{
    for (uint32_t index = 0; index < colorAttachmentsCount; ++index) {
        ClearFrameBufferColorInternal(renderID, index, pColor);
    }

    if (depthOpt && stencilOpt) {
        ClearFrameBufferDepthStencilInternal(renderID, *depthOpt, *stencilOpt);
        return;
    }

    if (depthOpt) {
        ClearFrameBufferDepthInternal(renderID, *depthOpt);
        return;
    }

    if (stencilOpt) {
        ClearFrameBufferStencilInternal(renderID, *stencilOpt);
    }
}


static GLenum GetFrameBufferAttachmentGLType(const FrameBufferAttachment& attachment)
{
    switch(attachment.type) {
        case FrameBufferAttachmentType::COLOR_ATTACHMENT:
            ENG_ASSERT_GRAPHICS_API(attachment.index < FrameBuffer::GetMaxColorAttachmentsCount(), "Invalid color attachment index");
            return GL_COLOR_ATTACHMENT0 + attachment.index;
        case FrameBufferAttachmentType::DEPTH_ATTACHMENT:
            return GL_DEPTH_ATTACHMENT;
        case FrameBufferAttachmentType::STENCIL_ATTACHMENT:
            return GL_STENCIL_ATTACHMENT;
        case FrameBufferAttachmentType::DEPTH_STENCIL_ATTACHMENT:
            return GL_DEPTH_STENCIL_ATTACHMENT;
        default:
            return GL_NONE;
    }
};


FrameBuffer::~FrameBuffer()
{
    Destroy();
}


FrameBuffer::FrameBuffer(FrameBuffer &&other) noexcept
{
#if defined(ENG_DEBUG)
    std::swap(m_attachments, other.m_attachments);
    std::swap(m_dbgName, other.m_dbgName);
#endif

    std::swap(m_attachmentsState, other.m_attachmentsState);
    std::swap(m_ID, other.m_ID);
    std::swap(m_renderID, other.m_renderID);
}


FrameBuffer &FrameBuffer::operator=(FrameBuffer &&other) noexcept
{
    Destroy();

#if defined(ENG_DEBUG)
    std::swap(m_attachments, other.m_attachments);
    std::swap(m_dbgName, other.m_dbgName);
#endif

    std::swap(m_attachmentsState, other.m_attachmentsState);
    std::swap(m_ID, other.m_ID);
    std::swap(m_renderID, other.m_renderID);

    return *this;
}


void FrameBuffer::Bind() noexcept
{
    ENG_ASSERT_GRAPHICS_API(IsValid(), "Attempt to bind invalid framebuffer");
    glBindFramebuffer(GL_FRAMEBUFFER, m_renderID);
}


void FrameBuffer::Clear(float r, float g, float b, float a, float depth, int32_t stencil) noexcept
{
    ENG_ASSERT_GRAPHICS_API(IsValid(), "Frame buffer is invalid");

    const float pColor[4] = { r, g, b, a };

    const float* pDepth = HasDepthAttachment() ? &depth : nullptr;
    const int32_t* pStencil = HasStencilAttachment() ? &stencil : nullptr;

    ClearFrameBufferInternal(m_renderID, GetColorAttachmentsCount(), pColor, pDepth, pStencil);
}

void FrameBuffer::ClearColor(uint32_t index, float r, float g, float b, float a) noexcept
{
    ENG_ASSERT_GRAPHICS_API(IsValid(), "Frame buffer is invalid");
    
    if (index < GetColorAttachmentsCount()) {
        const float pColor[4] = { r, g, b, a };
        ClearFrameBufferColorInternal(m_renderID, index, pColor);
    }
}


void FrameBuffer::ClearDepth(float depth) noexcept
{
    ENG_ASSERT_GRAPHICS_API(IsValid(), "Frame buffer is invalid");
    
    if (HasDepthAttachment()) {
        ClearFrameBufferDepthInternal(m_renderID, depth);
    }
}


void FrameBuffer::ClearStencil(int32_t stencil) noexcept
{
    ENG_ASSERT_GRAPHICS_API(IsValid(), "Frame buffer is invalid");
    
    if (HasStencilAttachment()) {
        ClearFrameBufferStencilInternal(m_renderID, stencil);
    }
}

void FrameBuffer::ClearDepthStencil(float depth, int32_t stencil) noexcept
{
    ENG_ASSERT_GRAPHICS_API(IsValid(), "Frame buffer is invalid");
    
    const bool hasDepthAttachment = HasDepthAttachment();
    const bool hasStencilAttachment = HasStencilAttachment();

    if (hasDepthAttachment && hasStencilAttachment) {
        ClearFrameBufferDepthStencilInternal(m_renderID, depth, stencil);
        return;
    }

    if (hasDepthAttachment) {
        ClearFrameBufferDepthInternal(m_renderID, depth);
        return;
    }

    if (hasStencilAttachment) {
        ClearFrameBufferStencilInternal(m_renderID, stencil);
    }
}


bool FrameBuffer::IsValid() const noexcept
{
    return m_renderID != 0 && IsValidID();
}


uint64_t FrameBuffer::Hash() const noexcept
{
    ds::HashBuilder builder;

    builder.AddValue(m_ID);
    builder.AddValue(m_renderID);

    return builder.Value();
}


void FrameBuffer::SetDebugName(ds::StrID name) noexcept
{
#if defined(ENG_DEBUG)
    m_dbgName = name;
#endif
}


ds::StrID FrameBuffer::GetDebugName() const noexcept
{
#if defined(ENG_DEBUG)
    return m_dbgName;
#else
    return "";
#endif
}


uint32_t FrameBuffer::GetAttachmentsCount() const noexcept
{
    uint32_t result = GetColorAttachmentsCount();
    result += GetDepthAttachmentCount() + (HasMergedDepthStencilAttachment() ? 0 : GetStencilAttachmentCount());

    return result;
}


bool FrameBuffer::Create(const FramebufferCreateInfo &createInfo) noexcept
{
    ENG_ASSERT(!IsValid(), "Attempt to create already valid frame buffer: {}", m_dbgName.CStr());
    
    return Recreate(createInfo);
}


void FrameBuffer::Destroy() noexcept
{
    glDeleteFramebuffers(1, &m_renderID);

#if defined(ENG_DEBUG)
    m_attachments.fill({nullptr, FrameBufferAttachmentType::INVALID, 0});
    m_dbgName = "_INVALID_";
#endif

    m_attachmentsState.colorAttachmentsCount = 0;
    m_attachmentsState.depthAttachmentsCount = 0;
    m_attachmentsState.stencilAttachmentsCount = 0;
    m_attachmentsState.hasMergedDepthStencilAttachement = 0;

    m_ID = RTFrameBufferID::INVALID;
    m_renderID = 0;
}


bool FrameBuffer::Recreate(const FramebufferCreateInfo& createInfo) noexcept
{
    ENG_ASSERT_GRAPHICS_API(engIsTextureManagerInitialized(), "Texture manager must be initialized before framebuffers initializing");
    
    ENG_ASSERT_GRAPHICS_API(IsValidRenderTargetFrameBufferID(createInfo.ID), "Invalid frame buffer ID");
    ENG_ASSERT_GRAPHICS_API(createInfo.attachmentsCount > 0 && createInfo.attachmentsCount <= GetMaxAttachmentsCount(), "Invalid attachments count");
    ENG_ASSERT_GRAPHICS_API(createInfo.pAttachments, "Attachements are nullptr (\'{}\')", m_dbgName.CStr());

    if (IsValid()) {
        Destroy();
    }

    glCreateFramebuffers(1, &m_renderID);

    uint32_t attachmentsWidth = 0;
    uint32_t attachmentsHeight = 0;

    bool isFirstAttachment = true;

    for (size_t attachmentIdx = 0; attachmentIdx < createInfo.attachmentsCount; ++attachmentIdx) {
        const FrameBufferAttachment& attachment = createInfo.pAttachments[attachmentIdx];
        Texture* pTex = attachment.pTexure;

        const uint32_t texWidth = pTex->GetWidth();
        const uint32_t texHeight = pTex->GetHeight();

        ENG_ASSERT_GRAPHICS_API(pTex, "Attachment {} of \'{}\' framebuffer is nullptr", attachmentIdx, m_dbgName.CStr());
        ENG_ASSERT_GRAPHICS_API(pTex->IsValid(), "Invalid color attachment");
        ENG_ASSERT_GRAPHICS_API(pTex->IsType2D(), "Invalid color attachment type. Only 2D textures are supported for now");

        if (isFirstAttachment) {
            attachmentsWidth = texWidth;
            attachmentsHeight = texHeight;
            isFirstAttachment = false;
        } else {
            ENG_ASSERT_GRAPHICS_API(attachmentsWidth == texWidth && attachmentsHeight == texHeight, "Attachments dimensions must be equal");
        }

    #if defined(ENG_DEBUG)
        m_attachments[attachmentIdx] = attachment;
    #endif

        switch (attachment.type) {
            case FrameBufferAttachmentType::COLOR_ATTACHMENT:
                ++m_attachmentsState.colorAttachmentsCount;
                break;
            case FrameBufferAttachmentType::DEPTH_ATTACHMENT:
                m_attachmentsState.depthAttachmentsCount = 1;
                break;
            case FrameBufferAttachmentType::STENCIL_ATTACHMENT:
                m_attachmentsState.stencilAttachmentsCount = 1;
                break;
            case FrameBufferAttachmentType::DEPTH_STENCIL_ATTACHMENT:
                m_attachmentsState.depthAttachmentsCount = 1;
                m_attachmentsState.stencilAttachmentsCount = 1;
                m_attachmentsState.hasMergedDepthStencilAttachement = true;
                break;
            default:
                ENG_ASSERT_FAIL("Invalid frame buffer attachment type");
                break;
        }

        glNamedFramebufferTexture(m_renderID, GetFrameBufferAttachmentGLType(attachment), pTex->GetRenderID(), 0);
    }

    if (!CheckCompleteStatus()) {
        ENG_ASSERT_GRAPHICS_API_FAIL("FrameBuffer \'{}\' is incomplete");
        Destroy();
        return false;
    }

    m_ID = createInfo.ID;

    return true;
}


bool FrameBuffer::CheckCompleteStatus() const noexcept
{
    const GLenum frameBufferStatus = glCheckNamedFramebufferStatus(m_renderID, GL_FRAMEBUFFER);

    switch(frameBufferStatus) {
        case GL_FRAMEBUFFER_COMPLETE:
            return true;
        case GL_FRAMEBUFFER_UNDEFINED: 
            ENG_LOG_GRAPHICS_API_ERROR("Specified framebuffer is the default read or draw framebuffer, but the default framebuffer does not exist");
            return false;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            ENG_LOG_GRAPHICS_API_ERROR("Any of the framebuffer attachment points are framebuffer incomplete");
            return false;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            ENG_LOG_GRAPHICS_API_ERROR("Framebuffer does not have at least one image attached to it");
            return false;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            ENG_LOG_GRAPHICS_API_ERROR("Value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for any color attachment point(s) named by GL_DRAW_BUFFERi");
            return false;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            ENG_LOG_GRAPHICS_API_ERROR("GL_READ_BUFFER is not GL_NONE and the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for the color attachment point named by GL_READ_BUFFER");
            return false;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            ENG_LOG_GRAPHICS_API_ERROR("Combination of internal formats of the attached images violates an implementation-dependent set of restrictions");
            return false;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            ENG_LOG_GRAPHICS_API_ERROR(
                "Value of GL_RENDERBUFFER_SAMPLES is not the same for all attached renderbuffers;"
                "if the value of GL_TEXTURE_SAMPLES is the not same for all attached textures;"
                "or, if the attached images are a mix of renderbuffers and textures,"
                "the value of GL_RENDERBUFFER_SAMPLES does not match the value of GL_TEXTURE_SAMPLES");
            ENG_LOG_GRAPHICS_API_ERROR("Or Value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not the same for all attached textures;"
                "or, if the attached images are a mix of renderbuffers and textures, "
                "the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not GL_TRUE for all attached textures.");
            return false;
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
            ENG_LOG_GRAPHICS_API_ERROR("Any framebuffer attachment is layered, and any populated attachment is not layered,"
            "or if all populated color attachments are not from textures of the same target.");
            return false;
            
        default:
            ENG_ASSERT_GRAPHICS_API_FAIL("Unknown frame buffer status");
            return false;
    }
}


bool FrameBuffer::IsValidID() const noexcept
{
    return IsValidRenderTargetFrameBufferID(m_ID);
}


RenderTargetManager &RenderTargetManager::GetInstance() noexcept
{
    ENG_ASSERT(engIsRenderTargetManagerInitialized(), "Render target manager is not initialized");
    return *pRenderTargetMngInst;
}


RenderTargetManager::~RenderTargetManager()
{
    Terminate();
}


Texture *RenderTargetManager::GetRTTexture(RTTextureID texID) noexcept
{
    ENG_ASSERT_GRAPHICS_API(texID < RTTextureID::COUNT, "Invalid RT texture ID");

    Texture* pRTTexture = m_RTTextureStorage[static_cast<size_t>(texID)];
    ENG_ASSERT_GRAPHICS_API(pRTTexture != nullptr, "RT texture is nullptr");
    
    return pRTTexture;
}


FrameBuffer *RenderTargetManager::GetFrameBuffer(RTFrameBufferID framebufferID) noexcept
{
    ENG_ASSERT_GRAPHICS_API(IsValidRenderTargetFrameBufferID(framebufferID), "Invalid frame buffer ID");
    
    FrameBuffer* pRTFrameBuffer = &m_frameBufferStorage[static_cast<size_t>(framebufferID)];

    return pRTFrameBuffer->IsValid() ? pRTFrameBuffer : nullptr;
}


void RenderTargetManager::BindFrameBuffer(RTFrameBufferID framebufferID) noexcept
{
    ENG_ASSERT_GRAPHICS_API(IsValidRenderTargetFrameBufferID(framebufferID), "Invalid frame buffer ID");
    
    m_frameBufferStorage[static_cast<size_t>(framebufferID)].Bind();
}


void RenderTargetManager::ClearFrameBuffer(RTFrameBufferID framebufferID, float r, float g, float b, float a, float depth, int32_t stencil) noexcept
{
    ENG_ASSERT_GRAPHICS_API(IsValidRenderTargetFrameBufferID(framebufferID), "Invalid frame buffer ID");
    
    const size_t frameBufferIdx = static_cast<size_t>(framebufferID);
    FrameBuffer& frameBuffer = m_frameBufferStorage[frameBufferIdx];

    frameBuffer.Clear(r, g, b, a, depth, stencil);
}


void RenderTargetManager::ClearFrameBufferColor(RTFrameBufferID framebufferID, uint32_t index, float r, float g, float b, float a) noexcept
{
    ENG_ASSERT_GRAPHICS_API(IsValidRenderTargetFrameBufferID(framebufferID), "Invalid frame buffer ID");
    
    const size_t frameBufferIdx = static_cast<size_t>(framebufferID);
    FrameBuffer& frameBuffer = m_frameBufferStorage[frameBufferIdx];

    frameBuffer.ClearColor(index, r, g, b, a);
}


void RenderTargetManager::ClearFrameBufferDepth(RTFrameBufferID framebufferID, float depth) noexcept
{
    ENG_ASSERT_GRAPHICS_API(IsValidRenderTargetFrameBufferID(framebufferID), "Invalid frame buffer ID");
    
    const size_t frameBufferIdx = static_cast<size_t>(framebufferID);
    FrameBuffer& frameBuffer = m_frameBufferStorage[frameBufferIdx];

    frameBuffer.ClearDepth(depth);
}

void RenderTargetManager::ClearFrameBufferStencil(RTFrameBufferID framebufferID, int32_t stencil) noexcept
{
    ENG_ASSERT_GRAPHICS_API(IsValidRenderTargetFrameBufferID(framebufferID), "Invalid frame buffer ID");
    
    const size_t frameBufferIdx = static_cast<size_t>(framebufferID);
    FrameBuffer& frameBuffer = m_frameBufferStorage[frameBufferIdx];

    frameBuffer.ClearStencil(stencil);
}

void RenderTargetManager::ClearFrameBufferDepthStencil(RTFrameBufferID framebufferID, float depth, int32_t stencil) noexcept
{
    ENG_ASSERT_GRAPHICS_API(IsValidRenderTargetFrameBufferID(framebufferID), "Invalid frame buffer ID");
    
    const size_t frameBufferIdx = static_cast<size_t>(framebufferID);
    FrameBuffer& frameBuffer = m_frameBufferStorage[frameBufferIdx];

    frameBuffer.ClearDepthStencil(depth, stencil);
}


bool RenderTargetManager::Init() noexcept
{
    if (IsInitialized()) {
        return true;
    }

    es::EventDispatcher& dispatcher = es::EventDispatcher::GetInstance();

    m_frameBufferResizeEventListenerID = dispatcher.Subscribe<EventFramebufferResized>([this](const void* pEvent) {
        const EventFramebufferResized& event = es::EventCast<EventFramebufferResized>(pEvent);
        const uint32_t width = event.GetWidth();
        const uint32_t height = event.GetHeight();
        
        if (width > 0 && height > 0) {
            OnWindowResizedEvent(width, height);
        }
    });

    m_isInitialized = true;

    return true;
}


void RenderTargetManager::Terminate() noexcept
{
    ClearFrameBuffersStorage();

    es::EventDispatcher& dispatcher = es::EventDispatcher::GetInstance();
    dispatcher.Unsubscribe(m_frameBufferResizeEventListenerID);
    
    m_isInitialized = false;
}


void RenderTargetManager::ClearFrameBuffersStorage() noexcept
{
    TextureManager& texManager = TextureManager::GetInstance();

    for (Texture* pTex : m_RTTextureStorage) {
        if (pTex) {
            pTex->Destroy();
            texManager.UnregisterTexture(pTex);
        }
    }
    m_RTTextureStorage.fill(nullptr);

    for (FrameBuffer& framebuffer : m_frameBufferStorage) {
        framebuffer.Destroy();
    }
}


void RenderTargetManager::PrepareRTTextureStorage(const RTTextureCreateInfoArray& rtTexDescs) noexcept
{
    TextureManager& texManager = TextureManager::GetInstance();

    for (size_t rtIdx = 0; rtIdx < rtTexDescs.size(); ++rtIdx) {
        const RTTextureIntermediateCreateInfo& rtTexDesc = rtTexDescs[rtIdx];

        Texture2DCreateInfo texCreateInfo = {};
        texCreateInfo.format = rtTexDesc.format;
        texCreateInfo.width = rtTexDesc.width;
        texCreateInfo.height = rtTexDesc.height;
        texCreateInfo.mipmapsCount = rtTexDesc.mipsCount;
    
        Texture* pTex = texManager.RegisterTexture2D(rtTexDesc.name);
        ENG_ASSERT(pTex, "Failed to register texture: {}", rtTexDesc.name.CStr());
        pTex->Create(texCreateInfo);
        ENG_ASSERT(pTex->IsValid(), "Failed to create texture: {}", rtTexDesc.name.CStr());
    
        m_RTTextureStorage[rtIdx] = pTex;
    }
}


void RenderTargetManager::PrepareRTFrameBufferStorage(const RTFrameBufferCreateInfoArray &fbDescs) noexcept
{
    for (size_t fbIdx = 0; fbIdx < fbDescs.size(); ++fbIdx) {
        const RTFrameBufferIntermediateCreateInfo& fbDesc = fbDescs[fbIdx];

        FramebufferCreateInfo fbCreateInfo = {};
        fbCreateInfo.ID = static_cast<RTFrameBufferID>(fbIdx);
        fbCreateInfo.pAttachments = fbDesc.pAttachments;
        fbCreateInfo.attachmentsCount = fbDesc.attachmentsCount;

        FrameBuffer& frameBuffer = m_frameBufferStorage[fbIdx];

        if (!frameBuffer.Create(fbCreateInfo)) {
            ENG_ASSERT_GRAPHICS_API_FAIL("Failed to initialize \'{}\' frame buffer", fbDesc.name.CStr());
        }

        frameBuffer.SetDebugName(fbDesc.name);
    }
}


void RenderTargetManager::OnWindowResizedEvent(uint32_t width, uint32_t height) noexcept
{
    RecreateFrameBuffers(width, height);
}


void RenderTargetManager::RecreateFrameBuffers(uint32_t width, uint32_t height) noexcept
{
    ClearFrameBuffersStorage();

    RTTextureCreateInfoArray frameBufferAttachmentDescs;

    frameBufferAttachmentDescs[size_t(RTTextureID::GBUFFER_ALBEDO)]   = { resGetTexResourceFormat(GBUFFER_ALBEDO_TEX), width, height, 0, ds::StrID("_GBUFFER_ALBEDO_") };
    frameBufferAttachmentDescs[size_t(RTTextureID::GBUFFER_NORMAL)]   = { resGetTexResourceFormat(GBUFFER_NORMAL_TEX), width, height, 0, ds::StrID("_GBUFFER_NORMAL_") };
    frameBufferAttachmentDescs[size_t(RTTextureID::GBUFFER_SPECULAR)] = { resGetTexResourceFormat(GBUFFER_SPECULAR_TEX), width, height, 0, ds::StrID("_GBUFFER_SPECULAR_") };
    
    frameBufferAttachmentDescs[size_t(RTTextureID::COMMON_DEPTH)] = { resGetTexResourceFormat(COMMON_DEPTH_TEX), width, height, 0, ds::StrID("_COMMON_DEPTH_") };
    
    frameBufferAttachmentDescs[size_t(RTTextureID::COMMON_COLOR)] = { resGetTexResourceFormat(COMMON_COLOR_TEX), width, height, 0, ds::StrID("_COMMON_COLOR_") };

    PrepareRTTextureStorage(frameBufferAttachmentDescs);

    RTFrameBufferCreateInfoArray frameBufferDescs;

    FrameBufferAttachment pGBufferAttachments[] = { 
        { m_RTTextureStorage[size_t(RTTextureID::GBUFFER_ALBEDO)], FrameBufferAttachmentType::COLOR_ATTACHMENT, 0 },
        { m_RTTextureStorage[size_t(RTTextureID::GBUFFER_NORMAL)], FrameBufferAttachmentType::COLOR_ATTACHMENT, 1 },
        { m_RTTextureStorage[size_t(RTTextureID::GBUFFER_SPECULAR)], FrameBufferAttachmentType::COLOR_ATTACHMENT, 2 },
        { m_RTTextureStorage[size_t(RTTextureID::COMMON_DEPTH)], FrameBufferAttachmentType::DEPTH_ATTACHMENT, 0 },
    };
    frameBufferDescs[size_t(RTFrameBufferID::GBUFFER)] = { pGBufferAttachments, _countof(pGBufferAttachments), ds::StrID("_GBUFFER_") };

    FrameBufferAttachment pPostProcessAttachments[] = { 
        { m_RTTextureStorage[size_t(RTTextureID::COMMON_COLOR)], FrameBufferAttachmentType::COLOR_ATTACHMENT, 0 },
    };
    frameBufferDescs[size_t(RTFrameBufferID::POST_PROCESS)] = { pPostProcessAttachments, _countof(pPostProcessAttachments), ds::StrID("_POST_PROCESS_") };

    PrepareRTFrameBufferStorage(frameBufferDescs);
}


uint64_t amHash(const FrameBuffer& frameBuffer) noexcept
{
    return frameBuffer.Hash();
}


bool engInitRenderTargetManager() noexcept
{
    if (engIsRenderTargetManagerInitialized()) {
        ENG_LOG_WARN("Render target manager is already initialized!");
        return true;
    }

    ENG_ASSERT(engIsTextureManagerInitialized(), "Texture manager must be initialized before render target manager!");

    pRenderTargetMngInst = std::unique_ptr<RenderTargetManager>(new RenderTargetManager);

    if (!pRenderTargetMngInst) {
        ENG_ASSERT_FAIL("Failed to allocate memory for render target manager");
        return false;
    }

    if (!pRenderTargetMngInst->Init()) {
        ENG_ASSERT_FAIL("Failed to initialized render target manager");
        return false;
    }

    return true;
}


void engTerminateRenderTargetManager() noexcept
{
    pRenderTargetMngInst = nullptr;
}


bool engIsRenderTargetManagerInitialized() noexcept
{
    return pRenderTargetMngInst && pRenderTargetMngInst->IsInitialized();
}

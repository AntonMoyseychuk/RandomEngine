#include "pch.h"
#include "rt_manager.h"

#include "engine/event_system/event_dispatcher.h"
#include "engine/engine.h"

#include "engine/render/platform/OpenGL/opengl_driver.h"

#include "utils/data_structures/hash.h"

#include "engine/auto/auto_texture_incl.h"
#include "engine/auto/auto_registers_common.h"


static std::unique_ptr<RenderTargetManager> s_pRenderTargetMng = nullptr;

static int32_t MAX_COLOR_ATTACHMENTS = 0;


GLenum EngineRenderBufferFormatToGLFormat(RenderBufferFormat format) noexcept
{
    switch(format) {
        case RenderBufferFormat::FORMAT_INVALID: return GL_NONE;
        case RenderBufferFormat::FORMAT_DEPTH_32: return GL_DEPTH_COMPONENT32F;
        case RenderBufferFormat::FORMAT_DEPTH_24: return GL_DEPTH_COMPONENT24;
        case RenderBufferFormat::FORMAT_DEPTH_16: return GL_DEPTH_COMPONENT16;
        case RenderBufferFormat::FORMAT_STENCIL_8: return GL_STENCIL_INDEX8;
        case RenderBufferFormat::FORMAT_STENCIL_4: return GL_STENCIL_INDEX4;
        case RenderBufferFormat::FORMAT_STENCIL_1: return GL_STENCIL_INDEX1;
        case RenderBufferFormat::FORMAT_DEPTH_24_STENCIL_8: return GL_DEPTH24_STENCIL8;
        case RenderBufferFormat::FORMAT_DEPTH_32_STENCIL_8: return GL_DEPTH32F_STENCIL8;
        default:
            ENG_ASSERT_GRAPHICS_API_FAIL("Invalid engine render buffer format");
            return GL_NONE;
    }
}


RenderBufferFormat GLRenderBufferFormatToEngineFormat(GLenum format) noexcept
{
    switch(format) {
        case GL_NONE: return RenderBufferFormat::FORMAT_INVALID;
        case GL_DEPTH_COMPONENT32F: return RenderBufferFormat::FORMAT_DEPTH_32;
        case GL_DEPTH_COMPONENT24: return RenderBufferFormat::FORMAT_DEPTH_24;
        case GL_DEPTH_COMPONENT16: return RenderBufferFormat::FORMAT_DEPTH_16;
        case GL_STENCIL_INDEX8: return RenderBufferFormat::FORMAT_STENCIL_8;
        case GL_STENCIL_INDEX4: return RenderBufferFormat::FORMAT_STENCIL_4;
        case GL_STENCIL_INDEX1: return RenderBufferFormat::FORMAT_STENCIL_1;
        case GL_DEPTH24_STENCIL8: return RenderBufferFormat::FORMAT_DEPTH_24_STENCIL_8;
        case GL_DEPTH32F_STENCIL8: return RenderBufferFormat::FORMAT_DEPTH_32_STENCIL_8;
        default:
            ENG_ASSERT_GRAPHICS_API_FAIL("Invalid GL render buffer format");
            return RenderBufferFormat::FORMAT_INVALID;
    }
}


ds::StrID RenderBuffer::GetName() const noexcept
{
#if defined(ENG_DEBUG)
    return m_dbgName;
#else
    return "";
#endif
}


RenderBufferFormat RenderBuffer::GetFormat() const noexcept
{
    return GLRenderBufferFormatToEngineFormat(m_format);
}


RenderBuffer::RenderBuffer(RenderBuffer &&other) noexcept
{
#if defined(ENG_DEBUG)
    std::swap(m_dbgName, other.m_dbgName);
#endif
    std::swap(m_format, other.m_format);
    std::swap(m_width, other.m_width);
    std::swap(m_height, other.m_height);
    std::swap(m_renderID, other.m_renderID);
}


RenderBuffer &RenderBuffer::operator=(RenderBuffer &&other) noexcept
{
    Destroy();

#if defined(ENG_DEBUG)
    std::swap(m_dbgName, other.m_dbgName);
#endif
    std::swap(m_format, other.m_format);
    std::swap(m_width, other.m_width);
    std::swap(m_height, other.m_height);
    std::swap(m_renderID, other.m_renderID);

    return *this;
}


bool RenderBuffer::Init(ds::StrID dbgName, const RenderBufferCreateInfo &createInfo) noexcept
{
#if defined(ENG_DEBUG)
    if (IsValid()) {
        if (m_dbgName != dbgName) {
            ENG_LOG_GRAPHICS_API_WARN("Reinitializing of \'{}\' render buffer to \'{}\'", m_dbgName.CStr(), dbgName.CStr());
        } else {
            ENG_LOG_GRAPHICS_API_WARN("Reinitializing of \'{}\' render buffer", m_dbgName.CStr());
        }
    }
#endif

    return Recreate(dbgName, createInfo);
}


void RenderBuffer::Destroy() noexcept
{
    glDeleteRenderbuffers(1, &m_renderID);

#if defined(ENG_DEBUG)
    m_dbgName = "";
#endif
    m_format = 0;
    m_width = 0;
    m_height = 0;
    m_renderID = 0;
}


bool RenderBuffer::Recreate(ds::StrID dbgName, const RenderBufferCreateInfo &createInfo) noexcept
{
    const GLenum internalFormat = EngineRenderBufferFormatToGLFormat(createInfo.format);
    if (internalFormat == GL_NONE) {
        return false;
    }

    if (IsValid()) {
        Destroy();
    }

    glCreateRenderbuffers(1, &m_renderID);
    glNamedRenderbufferStorage(m_renderID, internalFormat, createInfo.width, createInfo.height);

#if defined(ENG_DEBUG)
    m_dbgName = dbgName;
#endif
    m_format = internalFormat;
    m_width = createInfo.width;
    m_height = createInfo.height;
    
    return true;
}


FrameBuffer::~FrameBuffer()
{
    Destroy();
}


FrameBuffer::FrameBuffer(FrameBuffer &&other) noexcept
{
#if defined(ENG_DEBUG)
    std::swap(m_colorAttachments, other.m_colorAttachments);
    std::swap(m_renderBufferAttachments, other.m_renderBufferAttachments);

    std::swap(m_dbgName, other.m_dbgName);
    std::swap(m_ID, other.m_ID);
#endif

    std::swap(m_renderID, other.m_renderID);
}


FrameBuffer &FrameBuffer::operator=(FrameBuffer &&other) noexcept
{
    Destroy();

#if defined(ENG_DEBUG)
    std::swap(m_colorAttachments, other.m_colorAttachments);
    std::swap(m_renderBufferAttachments, other.m_renderBufferAttachments);

    std::swap(m_dbgName, other.m_dbgName);
    std::swap(m_ID, other.m_ID);
#endif

    std::swap(m_renderID, other.m_renderID);

    return *this;
}


void FrameBuffer::Bind() noexcept
{
    ENG_ASSERT_GRAPHICS_API(IsValid(), "Attempt to bind invalid framebuffer");
    glBindFramebuffer(GL_FRAMEBUFFER, m_renderID);
}


bool FrameBuffer::IsValid() const noexcept
{
#if defined(ENG_DEBUG)
    return m_renderID != 0 && m_ID != RTFrameBufferID::RT_FRAMEBUFFER_INVALID;
#else
    return m_renderID != 0;
#endif
}


ds::StrID FrameBuffer::GetName() const noexcept
{
#if defined(ENG_DEBUG)
    return m_dbgName;
#else
    return "";
#endif
}


RTFrameBufferID FrameBuffer::GetID() const noexcept
{
#if defined(ENG_DEBUG)
    return m_ID;
#else
    return RTFrameBufferID::RT_FRAMEBUFFER_INVALID;
#endif
}


bool FrameBuffer::Init(ds::StrID dbgName, const FramebufferCreateInfo &createInfo) noexcept
{
#if defined(ENG_DEBUG)
    if (IsValid()) {
        if (m_dbgName != dbgName) {
            ENG_LOG_GRAPHICS_API_WARN("Reinitializing of \'{}\' framebuffer to \'{}\'", m_dbgName.CStr(), dbgName.CStr());
        } else {
            ENG_LOG_GRAPHICS_API_WARN("Reinitializing of \'{}\' framebuffer", m_dbgName.CStr());
        }
    }
#endif

    return Recreate(dbgName, createInfo);
}


void FrameBuffer::Destroy() noexcept
{
    glDeleteFramebuffers(1, &m_renderID);

#if defined(ENG_DEBUG)
    m_colorAttachments.clear();
    m_renderBufferAttachments.clear();

    m_dbgName = "";
    m_ID = RTFrameBufferID::RT_FRAMEBUFFER_INVALID;
#endif

    m_renderID = 0;
}


bool FrameBuffer::Recreate(ds::StrID dbgName, const FramebufferCreateInfo & createInfo) noexcept
{
    ENG_ASSERT_GRAPHICS_API(engIsTextureManagerInitialized(), "Texture manager must be initialized before framebuffers initializing");
    ENG_ASSERT_GRAPHICS_API(createInfo.colorAttachmentsCount > 0 && createInfo.colorAttachmentsCount <= MAX_COLOR_ATTACHMENTS, 
        "Invalid color attachments count");

    ENG_ASSERT_GRAPHICS_API(createInfo.pColorAttachments || createInfo.pRenderBufferAttachments, "All attachements are nullptr (\'{}\')", dbgName.CStr());
    
    ENG_ASSERT_GRAPHICS_API(createInfo.id != RTFrameBufferID::RT_FRAMEBUFFER_INVALID, "Framebufer id is invalid");

    if (createInfo.colorAttachmentsCount > 0) {
        ENG_ASSERT_GRAPHICS_API(createInfo.pColorAttachments, 
        "Color attachements count is greater than 0 but attachments array is nullptr (\'{}\')", dbgName.CStr());
    }

    if (createInfo.pColorAttachments != nullptr) {
        ENG_ASSERT_GRAPHICS_API(createInfo.colorAttachmentsCount > 0, 
        "Color attachements array is not nullptr but attachments count is 0 (\'{}\')", dbgName.CStr());
    }

    if (createInfo.renderBufferAttachmentsCount > 0) {
        ENG_ASSERT_GRAPHICS_API(createInfo.pRenderBufferAttachments, 
        "Render buffer attachements count is greater than 0 but attachments array is nullptr (\'{}\')", dbgName.CStr());
    }

    if (createInfo.pRenderBufferAttachments != nullptr) {
        ENG_ASSERT_GRAPHICS_API(createInfo.renderBufferAttachmentsCount > 0, 
        "Render buffer attachements array is not nullptr but attachments count is 0 (\'{}\')", dbgName.CStr());
    }

    if (IsValid()) {
        Destroy();
    }

    glCreateFramebuffers(1, &m_renderID);

    uint32_t attachmentsWidth = 0;
    uint32_t attachmentsHeight = 0;

    bool isFirstColorAttachment = true;

#if defined(ENG_DEBUG)
    m_colorAttachments.reserve(createInfo.colorAttachmentsCount);
#endif

    for (size_t colorAttachmentIdx = 0; colorAttachmentIdx < createInfo.colorAttachmentsCount; ++colorAttachmentIdx) {
        const ColorAttachment& colorAttachment = createInfo.pColorAttachments[colorAttachmentIdx];
        Texture* pTex = colorAttachment.pTexure;

        ENG_ASSERT_GRAPHICS_API(colorAttachment.index != UINT32_MAX, "Invalid color attachment index");

        ENG_ASSERT_GRAPHICS_API(pTex, "Color attachment {} of \'{}\' framebuffer is nullptr", colorAttachmentIdx, dbgName.CStr());
        ENG_ASSERT_GRAPHICS_API(pTex->IsValid(), "Invalid color attachment");
        ENG_ASSERT_GRAPHICS_API(pTex->IsType2D(), "Invalid color attachment type. Only 2D textures are supported for now");

        if (isFirstColorAttachment) {
            attachmentsWidth = pTex->GetWidth();
            attachmentsHeight = pTex->GetHeight();
            isFirstColorAttachment = false;
        } else {
            ENG_ASSERT_GRAPHICS_API(attachmentsWidth == pTex->GetWidth() && attachmentsHeight == pTex->GetHeight(),
                "Color attachments dimensions must be equal");
        }

    #if defined(ENG_DEBUG)
        m_colorAttachments.emplace_back(colorAttachment);
    #endif

        glNamedFramebufferTexture(m_renderID, GL_COLOR_ATTACHMENT0 + colorAttachment.index, pTex->GetRenderID(), 0);
    }

#if defined(ENG_DEBUG)
    m_renderBufferAttachments.reserve(createInfo.renderBufferAttachmentsCount);
#endif

    for (size_t renderBufferAttachmentIdx = 0; renderBufferAttachmentIdx < createInfo.renderBufferAttachmentsCount; ++renderBufferAttachmentIdx) {
        const RenderBufferAttachment& renderBufferAttachment = createInfo.pRenderBufferAttachments[renderBufferAttachmentIdx];
        RenderBuffer* pRenderBuffer = renderBufferAttachment.pRenderBuffer;

        ENG_ASSERT_GRAPHICS_API(pRenderBuffer, "Render buffer attachment {} of \'{}\' framebuffer is nullptr", renderBufferAttachmentIdx, dbgName.CStr());
        ENG_ASSERT_GRAPHICS_API(pRenderBuffer->IsValid(), "Invalid render buffer attachment type");

        if (isFirstColorAttachment) {
            attachmentsWidth = pRenderBuffer->GetWidth();
            attachmentsHeight = pRenderBuffer->GetHeight();
            isFirstColorAttachment = false;
        } else {
            ENG_ASSERT_GRAPHICS_API(attachmentsWidth == pRenderBuffer->GetWidth() && attachmentsHeight == pRenderBuffer->GetHeight(),
                "Render buffer attachments dimensions must be equal and the same as color attachments if exists");
        }

    #if defined(ENG_DEBUG)
        m_renderBufferAttachments.emplace_back(renderBufferAttachment);
    #endif

        const GLenum renderBufferAttachmentInternal = [](RenderBufferAttachmentType type) -> GLenum
        {
            switch (type) {
                case RenderBufferAttachmentType::TYPE_DEPTH_ATTACHMENT: return GL_DEPTH_ATTACHMENT;
                case RenderBufferAttachmentType::TYPE_STENCIL_ATTACHMENT: return GL_STENCIL_ATTACHMENT;
                case RenderBufferAttachmentType::TYPE_DEPTH_STENCIL_ATTACHMENT: return GL_DEPTH_STENCIL_ATTACHMENT;
                
                default:
                    ENG_ASSERT_GRAPHICS_API_FAIL("Invalid render buffer attachemtn type");
                    return GL_NONE;
            }
        }(renderBufferAttachment.type);

        glNamedFramebufferRenderbuffer(m_renderID, renderBufferAttachmentInternal, GL_RENDERBUFFER, pRenderBuffer->GetRenderID());
    }

    if (!CheckCompleteStatus()) {
        ENG_ASSERT_GRAPHICS_API_FAIL("FrameBuffer \'{}\' is incomplete");
        Destroy();
        return false;
    }

#if defined(ENG_DEBUG)
    m_dbgName = dbgName;
    m_ID = createInfo.id;
#endif

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


RenderTargetManager &RenderTargetManager::GetInstance() noexcept
{
    ENG_ASSERT_GRAPHICS_API(engIsRenderTargetManagerInitialized(), "Render target manager is not initialized");
    return *s_pRenderTargetMng;
}


RenderTargetManager::~RenderTargetManager()
{
    Terminate();
}


Texture *RenderTargetManager::GetRTTexture(RTTextureID texID) noexcept
{
    ENG_ASSERT_GRAPHICS_API(texID < RTTextureID::RT_TEX_COUNT, "Invalid RT texture ID");

    Texture* pRTTexture = m_RTTextures[static_cast<size_t>(texID)];
    ENG_ASSERT_GRAPHICS_API(pRTTexture != nullptr, "RT texture is nullptr");
    
    return pRTTexture;
}


void RenderTargetManager::BindFramebuffer(RTFrameBufferID framebufferID) noexcept
{
    if (framebufferID == RTFrameBufferID::RT_FRAMEBUFFER_DEFAULT) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    ENG_ASSERT_GRAPHICS_API(framebufferID < RTFrameBufferID::RT_FRAMEBUFFER_COUNT, "Invalid RT framebuffer ID");
    m_frameBuffers[static_cast<size_t>(framebufferID)].Bind();
}


bool RenderTargetManager::Init() noexcept
{
    if (IsInitialized()) {
        return true;
    }

    EventDispatcher& dispatcher = EventDispatcher::GetInstance();

    dispatcher.Subscribe(EventListener::Create<EventFramebufferResized>([this](const void* pEvent) {
            const EventFramebufferResized& event = CastEventTo<EventFramebufferResized>(pEvent);
            const uint32_t width = event.GetWidth();
            const uint32_t height = event.GetHeight();

            if (width > 0 && height > 0) {
                OnWindowResizedEvent(width, height);
            }
        }
    ));

    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &MAX_COLOR_ATTACHMENTS);

    m_isInitialized = true;

    return true;
}


void RenderTargetManager::Terminate() noexcept
{
    ClearFrameBuffers();

    m_isInitialized = false;
}


void RenderTargetManager::ClearFrameBuffers() noexcept
{
    for (FrameBuffer& framebuffer : m_frameBuffers) {
        framebuffer.Destroy();
    }
    m_frameBuffers.clear();

    for (RenderBuffer& renderBuffer : m_RTRenderBuffers) {
        renderBuffer.Destroy();
    }
    m_RTRenderBuffers.clear();

    TextureManager& texManager = TextureManager::GetInstance();

    for (Texture* pTex : m_RTTextures) {
        texManager.DeallocateTexture(pTex->GetName());
    }
    m_RTTextures.clear();
}


void RenderTargetManager::OnWindowResizedEvent(uint32_t width, uint32_t height) noexcept
{
    RecreateFrameBuffers(width, height);
}


void RenderTargetManager::RecreateFrameBuffers(uint32_t width, uint32_t height) noexcept
{
    ClearFrameBuffers();

    TextureManager& texManager = TextureManager::GetInstance();

    m_RTTextures.resize(static_cast<size_t>(RTTextureID::RT_TEX_COUNT));

    ColorAttachment gbufferAlbedoColorAttachment = {};

    {
        Texture2DCreateInfo gbufferAlbedoTextureCreateInfo = {};
        gbufferAlbedoTextureCreateInfo.format = resGetTexResourceFormat(GBUFFER_ALBEDO_TEX);
        gbufferAlbedoTextureCreateInfo.width = width;
        gbufferAlbedoTextureCreateInfo.height = height;
        gbufferAlbedoTextureCreateInfo.mipmapsCount = 0;
        gbufferAlbedoTextureCreateInfo.inputData = {};
        
        ds::StrID gbufferAlbedoTexName = "_GBUFFER_ALBEDO_";
        TextureID gbufferAlbedoTexID = texManager.AllocateTexture2D(gbufferAlbedoTexName, gbufferAlbedoTextureCreateInfo);
        Texture* pGbufferAlbedoTex = texManager.GetTextureByID(gbufferAlbedoTexID);
        ENG_ASSERT_GRAPHICS_API(pGbufferAlbedoTex, "Failed to allocate \'{}\' texture", gbufferAlbedoTexName.CStr());
        m_RTTextures[static_cast<size_t>(RTTextureID::RT_TEX_GBUFFER_ALBEDO)] = pGbufferAlbedoTex;

        gbufferAlbedoColorAttachment.pTexure = pGbufferAlbedoTex;
        gbufferAlbedoColorAttachment.id = RTTextureID::RT_TEX_GBUFFER_ALBEDO;
        gbufferAlbedoColorAttachment.index = 0;
    }

    ColorAttachment gbufferNormalColorAttachment = {};

    {
        Texture2DCreateInfo gbufferNormalTextureCreateInfo = {};
        gbufferNormalTextureCreateInfo.format = resGetTexResourceFormat(GBUFFER_NORMAL_TEX);
        gbufferNormalTextureCreateInfo.width = width;
        gbufferNormalTextureCreateInfo.height = height;
        gbufferNormalTextureCreateInfo.mipmapsCount = 0;
        gbufferNormalTextureCreateInfo.inputData = {};

        ds::StrID gbufferNormalTexName = "_GBUFFER_NORMAL_";
        TextureID gbufferNormalTexID = texManager.AllocateTexture2D(gbufferNormalTexName, gbufferNormalTextureCreateInfo);
        Texture* pGbufferNormalTex = texManager.GetTextureByID(gbufferNormalTexID);
        ENG_ASSERT_GRAPHICS_API(pGbufferNormalTex, "Failed to allocate \'{}\' texture", gbufferNormalTexName.CStr());
        m_RTTextures[static_cast<size_t>(RTTextureID::RT_TEX_GBUFFER_NORMAL)] = pGbufferNormalTex;

        gbufferNormalColorAttachment.pTexure = pGbufferNormalTex;
        gbufferNormalColorAttachment.id = RTTextureID::RT_TEX_GBUFFER_NORMAL;
        gbufferNormalColorAttachment.index = 1;
    }

    ColorAttachment gbufferSpecColorAttachment = {};

    {
        Texture2DCreateInfo gbufferSpecularTextureCreateInfo = {};
        gbufferSpecularTextureCreateInfo.format = resGetTexResourceFormat(GBUFFER_SPECULAR_TEX);
        gbufferSpecularTextureCreateInfo.width = width;
        gbufferSpecularTextureCreateInfo.height = height;
        gbufferSpecularTextureCreateInfo.mipmapsCount = 0;
        gbufferSpecularTextureCreateInfo.inputData = {};

        ds::StrID gbufferSpecularTexName = "_GBUFFER_SPECULAR_";
        TextureID gbufferSpecularTexID = texManager.AllocateTexture2D(gbufferSpecularTexName, gbufferSpecularTextureCreateInfo);
        Texture* pGbufferSpecTex = texManager.GetTextureByID(gbufferSpecularTexID);
        ENG_ASSERT_GRAPHICS_API(pGbufferSpecTex, "Failed to allocate \'{}\' texture", gbufferSpecularTexName.CStr());
        m_RTTextures[static_cast<size_t>(RTTextureID::RT_TEX_GBUFFER_SPECULAR)] = pGbufferSpecTex;

        gbufferSpecColorAttachment.pTexure = pGbufferSpecTex;
        gbufferSpecColorAttachment.id = RTTextureID::RT_TEX_GBUFFER_SPECULAR;
        gbufferSpecColorAttachment.index = 2;
    }

    {
        Texture2DCreateInfo commonDepthTextureCreateInfo = {};
        commonDepthTextureCreateInfo.format = resGetTexResourceFormat(COMMON_DEPTH_TEX);
        commonDepthTextureCreateInfo.width = width;
        commonDepthTextureCreateInfo.height = height;
        commonDepthTextureCreateInfo.mipmapsCount = 0;
        commonDepthTextureCreateInfo.inputData = {};

        ds::StrID commonDepthTexName = "_COMMON_DEPTH_";
        TextureID commonDepthTexID = texManager.AllocateTexture2D(commonDepthTexName, commonDepthTextureCreateInfo);
        Texture* pCommonDepthTex = texManager.GetTextureByID(commonDepthTexID);
        ENG_ASSERT_GRAPHICS_API(pCommonDepthTex, "Failed to allocate \'{}\' texture", commonDepthTexName.CStr());
        m_RTTextures[static_cast<size_t>(RTTextureID::RT_TEX_COMMON_DEPTH)] = pCommonDepthTex;
    }

    m_RTRenderBuffers.resize(static_cast<size_t>(RTRenderBufferID::RT_RENDERBUFFER_COUNT));

    RenderBufferAttachment commonDepthRenderBufferAttachment = {};

    {
        RenderBufferCreateInfo renderBufferCommonDepthCreateInfo = {};
        renderBufferCommonDepthCreateInfo.format = RenderBufferFormat::FORMAT_DEPTH_32;
        renderBufferCommonDepthCreateInfo.width = width;
        renderBufferCommonDepthCreateInfo.height = height;

        ds::StrID commonDepthRenderBufferName = "_COMMON_DEPTH_RENDER_BUFFER_";
        const size_t commonDepthRenderBufferIdx = static_cast<size_t>(RTRenderBufferID::RT_RENDERBUFFER_COMMON_DEPTH);
        const bool isCommonDepthRenderBufferInitialized = 
            m_RTRenderBuffers[commonDepthRenderBufferIdx].Init(commonDepthRenderBufferName, renderBufferCommonDepthCreateInfo);
        ENG_ASSERT_GRAPHICS_API(isCommonDepthRenderBufferInitialized, "Failed to initialize render buffer \'{}\'", commonDepthRenderBufferName.CStr());

        commonDepthRenderBufferAttachment.pRenderBuffer = &m_RTRenderBuffers[commonDepthRenderBufferIdx];
        commonDepthRenderBufferAttachment.id = RTRenderBufferID::RT_RENDERBUFFER_COMMON_DEPTH;
        commonDepthRenderBufferAttachment.type = RenderBufferAttachmentType::TYPE_DEPTH_ATTACHMENT;
    }

    m_frameBuffers.resize(static_cast<size_t>(RTFrameBufferID::RT_FRAMEBUFFER_COUNT));

    {
        FramebufferCreateInfo gbufferFrameBufferCreateInfo = {};
        gbufferFrameBufferCreateInfo.id = RTFrameBufferID::RT_FRAMEBUFFER_GBUFFER;

        ColorAttachment pColorAttachments[] = { 
            gbufferAlbedoColorAttachment, gbufferNormalColorAttachment, gbufferSpecColorAttachment
        };

        RenderBufferAttachment pRenderBufferAttachments[] = {
            commonDepthRenderBufferAttachment
        };

        gbufferFrameBufferCreateInfo.pColorAttachments = pColorAttachments;
        gbufferFrameBufferCreateInfo.colorAttachmentsCount = _countof(pColorAttachments);
        gbufferFrameBufferCreateInfo.pRenderBufferAttachments = pRenderBufferAttachments;
        gbufferFrameBufferCreateInfo.renderBufferAttachmentsCount = _countof(pRenderBufferAttachments);

        ds::StrID gbufferFrameBufferName = "_GBUFFER_FRAMEBUFFER_";
        ENG_MAYBE_UNUSED bool isgbufferFrameBufferInitialized = 
            m_frameBuffers[static_cast<size_t>(RTFrameBufferID::RT_FRAMEBUFFER_GBUFFER)].Init(gbufferFrameBufferName, gbufferFrameBufferCreateInfo);
        ENG_ASSERT_GRAPHICS_API(isgbufferFrameBufferInitialized, "Failed to initialize frame buffer \'{}\'", gbufferFrameBufferName.CStr());
    }
}


bool engInitRenderTargetManager() noexcept
{
    if (engIsRenderTargetManagerInitialized()) {
        ENG_LOG_GRAPHICS_API_WARN("Render target manager is already initialized!");
        return true;
    }

    ENG_ASSERT_GRAPHICS_API(engIsTextureManagerInitialized(), "Texture manager must be initialized before render target manager!");

    s_pRenderTargetMng = std::unique_ptr<RenderTargetManager>(new RenderTargetManager);

    if (!s_pRenderTargetMng) {
        ENG_ASSERT_GRAPHICS_API_FAIL("Failed to allocate memory for render target manager");
        return false;
    }

    if (!s_pRenderTargetMng->Init()) {
        ENG_ASSERT_GRAPHICS_API_FAIL("Failed to initialized render target manager");
        return false;
    }

    return true;
}


void engTerminateRenderTargetManager() noexcept
{
    s_pRenderTargetMng = nullptr;
}


bool engIsRenderTargetManagerInitialized() noexcept
{
    return s_pRenderTargetMng && s_pRenderTargetMng->IsInitialized();
}

#include "pch.h"
#include "pipeline_mng.h"

#include "utils/debug/assertion.h"
#include "utils/data_structures/hash.h"

#include "render/platform/OpenGL/opengl_driver.h"


static constexpr size_t ENG_MAX_PIPELINES_COUNT = 8192; // TODO: make it configurable

static std::unique_ptr<PipelineManager> pPipelineMngInst = nullptr;


static constexpr GLenum CompressedBlendFactorToGLEnum(uint32_t compressedBlendFactor) noexcept
{
    switch(static_cast<BlendFactor>(compressedBlendFactor)) {
        case BlendFactor::FACTOR_ZERO: return GL_ZERO;
        case BlendFactor::FACTOR_ONE: return GL_ONE;
        case BlendFactor::FACTOR_SRC_COLOR: return GL_SRC_COLOR;
        case BlendFactor::FACTOR_ONE_MINUS_SRC_COLOR: return GL_ONE_MINUS_SRC_COLOR;
        case BlendFactor::FACTOR_DST_COLOR: return GL_DST_COLOR;
        case BlendFactor::FACTOR_ONE_MINUS_DST_COLOR: return GL_ONE_MINUS_DST_COLOR;
        case BlendFactor::FACTOR_SRC_ALPHA: return GL_SRC_ALPHA;
        case BlendFactor::FACTOR_ONE_MINUS_SRC_ALPHA: return GL_ONE_MINUS_SRC_ALPHA;
        case BlendFactor::FACTOR_DST_ALPHA: return GL_DST_ALPHA;
        case BlendFactor::FACTOR_ONE_MINUS_DST_ALPHA: return GL_ONE_MINUS_DST_ALPHA;
        case BlendFactor::FACTOR_CONSTANT_COLOR: return GL_CONSTANT_COLOR;
        case BlendFactor::FACTOR_ONE_MINUS_CONSTANT_COLOR: return GL_ONE_MINUS_CONSTANT_COLOR;
        case BlendFactor::FACTOR_CONSTANT_ALPHA: return GL_CONSTANT_ALPHA;
        case BlendFactor::FACTOR_ONE_MINUS_CONSTANT_ALPHA: return GL_ONE_MINUS_CONSTANT_ALPHA;
        default:
            ENG_ASSERT_GRAPHICS_API_FAIL("Invalid compressedBlendFactor");
            return GL_NONE;
    }
}


static constexpr GLenum CompressedBlendOpToGLEnum(uint32_t compressedBlendOp) noexcept
{
    switch(static_cast<BlendOp>(compressedBlendOp)) {
        case BlendOp::BLEND_OP_ADD: return GL_FUNC_ADD;
        case BlendOp::BLEND_OP_SUBTRACT: return GL_FUNC_SUBTRACT;
        case BlendOp::BLEND_OP_REVERSE_SUBTRACT: return GL_FUNC_REVERSE_SUBTRACT;
        case BlendOp::BLEND_OP_MIN: return GL_MIN;
        case BlendOp::BLEND_OP_MAX: return GL_MAX;
        default:
            ENG_ASSERT_GRAPHICS_API_FAIL("Invalid compressedBlendOp");
            return GL_NONE;
    }
}


static constexpr GLenum CompressedCompareFuncToGLEnum(uint32_t compressedCompareFunc) noexcept
{
    switch(static_cast<CompareFunc>(compressedCompareFunc)) {
        case CompareFunc::FUNC_NEVER: return GL_NEVER;
        case CompareFunc::FUNC_ALWAYS: return GL_ALWAYS;
        case CompareFunc::FUNC_LESS: return GL_LESS;
        case CompareFunc::FUNC_GREATER: return GL_GREATER;
        case CompareFunc::FUNC_EQUAL: return GL_EQUAL;
        case CompareFunc::FUNC_LEQUAL: return GL_LEQUAL;
        case CompareFunc::FUNC_GEQUAL: return GL_GEQUAL;
        case CompareFunc::FUNC_NOTEQUAL: return GL_NOTEQUAL;
        default:
            ENG_ASSERT_GRAPHICS_API_FAIL("Invalid compressedCompareFunc");
            return GL_NONE;
    }
}


static constexpr GLenum CompressedStencilOpToGLEnum(uint32_t compressedStencilOp) noexcept
{
    switch(static_cast<StencilOp>(compressedStencilOp)) {
        case StencilOp::STENCIL_OP_KEEP: return GL_KEEP;
        case StencilOp::STENCIL_OP_ZERO: return GL_ZERO;
        case StencilOp::STENCIL_OP_INCREMENT: return GL_INCR;
        case StencilOp::STENCIL_OP_DECREMENT: return GL_DECR;
        case StencilOp::STENCIL_OP_INVERT: return GL_INVERT;
        case StencilOp::STENCIL_OP_REPLACE: return GL_REPLACE;
        case StencilOp::STENCIL_OP_INCREMENT_WRAP: return GL_INCR_WRAP;
        case StencilOp::STENCIL_OP_DECREMENT_WRAP: return GL_DECR_WRAP;
        default:
            ENG_ASSERT_GRAPHICS_API_FAIL("Invalid compressedStencilOp");
            return GL_NONE;
    }
}


static constexpr GLenum CompressedLogicOpToGLEnum(uint32_t compressedLogicOp) noexcept
{
    switch(static_cast<LogicOp>(compressedLogicOp)) {
        case LogicOp::LOGIC_OP_CLEAR: return GL_CLEAR;
        case LogicOp::LOGIC_OP_AND: return GL_AND;
        case LogicOp::LOGIC_OP_AND_REVERSE: return GL_AND_REVERSE;
        case LogicOp::LOGIC_OP_COPY: return GL_COPY;
        case LogicOp::LOGIC_OP_AND_INVERTED: return GL_AND_INVERTED;
        case LogicOp::LOGIC_OP_NO_OP: return GL_NOOP;
        case LogicOp::LOGIC_OP_XOR: return GL_XOR;
        case LogicOp::LOGIC_OP_OR: return GL_OR;
        case LogicOp::LOGIC_OP_NOR: return GL_NOR;
        case LogicOp::LOGIC_OP_EQUIVALENT: return GL_EQUIV;
        case LogicOp::LOGIC_OP_INVERT: return GL_INVERT;
        case LogicOp::LOGIC_OP_OR_REVERSE: return GL_OR_REVERSE;
        case LogicOp::LOGIC_OP_COPY_INVERTED: return GL_COPY_INVERTED;
        case LogicOp::LOGIC_OP_OR_INVERTED: return GL_OR_INVERTED;
        case LogicOp::LOGIC_OP_NAND: return GL_NAND;
        case LogicOp::LOGIC_OP_SET: return GL_SET;
        default:
            ENG_ASSERT_GRAPHICS_API_FAIL("Invalid compressedLogicOp");
            return GL_NONE;
    }
}


static constexpr bool IsBlendFactorConstant(GLenum factor) noexcept
{
    switch(factor) {
        case GL_CONSTANT_COLOR: return true;
        case GL_ONE_MINUS_CONSTANT_COLOR: return true;
        case GL_CONSTANT_ALPHA: return true;
        case GL_ONE_MINUS_CONSTANT_ALPHA: return true;
        default: return false;
    }
}


static void EnablePolygonOffset(PolygonMode mode) noexcept
{
    switch(mode) {
        case PolygonMode::POLYGON_MODE_FILL:
            glEnable(GL_POLYGON_OFFSET_FILL);
            break;
        case PolygonMode::POLYGON_MODE_LINE:
            glEnable(GL_POLYGON_OFFSET_LINE);
            break;
        case PolygonMode::POLYGON_MODE_POINT:
            glEnable(GL_POLYGON_OFFSET_POINT);
            break;
        default:
            ENG_ASSERT_GRAPHICS_API_FAIL("Invalid polygon mode");
            break;
    }
}


static void DisablePolygonOffset(PolygonMode mode) noexcept
{
    switch(mode) {
        case PolygonMode::POLYGON_MODE_FILL:
            glDisable(GL_POLYGON_OFFSET_FILL);
            break;
        case PolygonMode::POLYGON_MODE_LINE:
            glDisable(GL_POLYGON_OFFSET_LINE);
            break;
        case PolygonMode::POLYGON_MODE_POINT:
            glDisable(GL_POLYGON_OFFSET_POINT);
            break;
        default:
            ENG_ASSERT_GRAPHICS_API_FAIL("Invalid polygon mode");
            break;
    }
}


static void SetupPolygonOffset(PolygonMode mode, float biasConstFactor, float biasSlopeFactor, float biasClamp, bool enabled) noexcept
{
#if defined(ENG_USE_INVERTED_Z)
    biasConstFactor = -biasConstFactor;
    biasSlopeFactor = -biasSlopeFactor;
    biasClamp       = -biasClamp;
#endif

    if (enabled) {
        EnablePolygonOffset(mode);
        glPolygonOffsetClamp(biasConstFactor, biasSlopeFactor, biasClamp);
    } else {
        DisablePolygonOffset(mode);
    }
}


static void SetupFaceStencilTesting(GLenum face, GLuint mask, uint32_t sfOp, uint32_t spdfOp, uint32_t spdpOp, bool enabled)
{
    if (enabled) {
        glStencilMaskSeparate(face, mask);

        const GLenum frontFaceStencilFailOp = CompressedStencilOpToGLEnum(sfOp);
        const GLenum frontFaceStencilPassDepthFailOp = CompressedStencilOpToGLEnum(spdfOp);
        const GLenum frontFaceStencilPassDepthPassOp = CompressedStencilOpToGLEnum(spdpOp);
        glStencilOpSeparate(face, frontFaceStencilFailOp, frontFaceStencilPassDepthFailOp, frontFaceStencilPassDepthPassOp);
    } else {
        glStencilMaskSeparate(face, 0x00);
    }
}


static void SetupFaceCulling(CullMode mode) noexcept
{
    switch(mode) {
        case CullMode::CULL_MODE_NONE:
            glDisable(GL_CULL_FACE);
            break;
        case CullMode::CULL_MODE_FRONT:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            break;
        case CullMode::CULL_MODE_BACK:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            break;
        case CullMode::CULL_MODE_FRONT_AND_BACK:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT_AND_BACK);
            break;
        default:
            ENG_ASSERT_GRAPHICS_API_FAIL("Invalid cull face mode");
            break;
    }
}


Pipeline::Pipeline(Pipeline &&other) noexcept
{
    std::swap(m_frameBufferColorAttachmentClearColors, other.m_frameBufferColorAttachmentClearColors);
    std::swap(m_compressedColorBlendAttachmentStates, other.m_compressedColorBlendAttachmentStates);
    std::swap(m_blendConstants[0], other.m_blendConstants[0]);
    std::swap(m_blendConstants[1], other.m_blendConstants[1]);
    std::swap(m_blendConstants[2], other.m_blendConstants[2]);
    std::swap(m_blendConstants[3], other.m_blendConstants[3]);

    std::swap(m_pFrameBuffer, other.m_pFrameBuffer);
    std::swap(m_pShaderProgram, other.m_pShaderProgram);

    std::swap(m_compressedGlobalState, other.m_compressedGlobalState);

    std::swap(m_ID, other.m_ID);

    std::swap(m_depthBiasConstantFactor, other.m_depthBiasConstantFactor);
    std::swap(m_depthBiasClamp, other.m_depthBiasClamp);
    std::swap(m_depthBiasSlopeFactor, other.m_depthBiasSlopeFactor);
    std::swap(m_lineWidth, other.m_lineWidth);
    std::swap(m_depthClearValue, other.m_depthClearValue);
    std::swap(m_stencilClearValue, other.m_stencilClearValue);
    std::swap(m_stencilFrontMask, other.m_stencilFrontMask);
    std::swap(m_stencilBackMask, other.m_stencilBackMask);
}


Pipeline &Pipeline::operator=(Pipeline &&other) noexcept
{
    Destroy();

    std::swap(m_frameBufferColorAttachmentClearColors, other.m_frameBufferColorAttachmentClearColors);
    std::swap(m_compressedColorBlendAttachmentStates, other.m_compressedColorBlendAttachmentStates);
    std::swap(m_blendConstants[0], other.m_blendConstants[0]);
    std::swap(m_blendConstants[1], other.m_blendConstants[1]);
    std::swap(m_blendConstants[2], other.m_blendConstants[2]);
    std::swap(m_blendConstants[3], other.m_blendConstants[3]);

    std::swap(m_pFrameBuffer, other.m_pFrameBuffer);
    std::swap(m_pShaderProgram, other.m_pShaderProgram);

    std::swap(m_compressedGlobalState, other.m_compressedGlobalState);

    std::swap(m_ID, other.m_ID);

    std::swap(m_depthBiasConstantFactor, other.m_depthBiasConstantFactor);
    std::swap(m_depthBiasClamp, other.m_depthBiasClamp);
    std::swap(m_depthBiasSlopeFactor, other.m_depthBiasSlopeFactor);
    std::swap(m_lineWidth, other.m_lineWidth);
    std::swap(m_depthClearValue, other.m_depthClearValue);
    std::swap(m_stencilClearValue, other.m_stencilClearValue);
    std::swap(m_stencilFrontMask, other.m_stencilFrontMask);
    std::swap(m_stencilBackMask, other.m_stencilBackMask);

    return *this;
}


void Pipeline::ClearFrameBuffer() noexcept
{
    ENG_ASSERT(IsValid(), "Pipeline is invalid");

    for (uint32_t colorAttachmentIdx = 0; colorAttachmentIdx < m_frameBufferColorAttachmentClearColors.size(); ++colorAttachmentIdx) {
        const FrameBufferColorAttachmentClearColor& clearColor = m_frameBufferColorAttachmentClearColors[colorAttachmentIdx];
        
        m_pFrameBuffer->ClearColor(colorAttachmentIdx, clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    }

    m_pFrameBuffer->ClearDepthStencil(m_depthClearValue, m_stencilClearValue);
}


void Pipeline::Bind() noexcept
{
    ENG_ASSERT(IsValid(), "Pipeline is invalid");

    m_pFrameBuffer->Bind();
    m_pShaderProgram->Bind();

    const CompressedGlobalState& state = m_compressedGlobalState;

    const GLenum frontFace = state.frontFace == uint64_t(FrontFace::FRONT_FACE_CLOCKWISE) ? GL_CW : GL_CCW;
    glFrontFace(frontFace);

    SetupFaceCulling(static_cast<CullMode>(state.cullMode));

    SetupDepthTesting();
    SetupStencilTesting();
    SetupColorBlending();

    if (state.polygonMode == uint64_t(PolygonMode::POLYGON_MODE_LINE)) {
        glLineWidth(m_lineWidth);
    }


    {
        // TODO: FIX THIS SHIT!!!

        static constexpr uint32_t MAX_COLOR_ATTACHMENTS = 8;
    
        const uint32_t frameBufferColorAttachmentsCount = m_pFrameBuffer->GetColorAttachmentsCount();
        ENG_ASSERT(frameBufferColorAttachmentsCount <= MAX_COLOR_ATTACHMENTS, "Invalid color attachments count");
    
        std::array<GLenum, MAX_COLOR_ATTACHMENTS> drawBuffers = { GL_NONE };
        for (size_t i = 0; i < frameBufferColorAttachmentsCount; ++i) {
            drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
        }
    
        glDrawBuffers(frameBufferColorAttachmentsCount, drawBuffers.data());
    }
}


uint64_t Pipeline::Hash() const noexcept
{
    if (!IsValid()) {
        return UINT64_MAX;
    }

    ds::HashBuilder builder;

    for (const FrameBufferColorAttachmentClearColor& color : m_frameBufferColorAttachmentClearColors) {
        builder.AddValue(color.r);
        builder.AddValue(color.g);
        builder.AddValue(color.b);
        builder.AddValue(color.a);
    }

    for (const CompressedColorAttachmentBlendState& state : m_compressedColorBlendAttachmentStates) {
        const uint32_t stateAsUInt32 = *reinterpret_cast<const uint32_t*>(&state);
        builder.AddValue(stateAsUInt32);
    }

    builder.AddValue(m_blendConstants[0]);
    builder.AddValue(m_blendConstants[1]);
    builder.AddValue(m_blendConstants[2]);
    builder.AddValue(m_blendConstants[3]);

    builder.AddValue(m_ID);
    builder.AddValue(*m_pFrameBuffer);
    builder.AddValue(*m_pShaderProgram);

    const uint64_t globalStateAsUInt64 = *reinterpret_cast<const uint64_t*>(&m_compressedGlobalState);
    builder.AddValue(globalStateAsUInt64);

    builder.AddValue(m_depthBiasConstantFactor);
    builder.AddValue(m_depthBiasClamp);
    builder.AddValue(m_depthBiasSlopeFactor);

    builder.AddValue(m_depthClearValue);
    builder.AddValue(m_stencilClearValue);
    builder.AddValue(m_stencilFrontMask);
    builder.AddValue(m_stencilBackMask);

    builder.AddValue(m_lineWidth);

    return builder.Value();
}


bool Pipeline::IsValid() const noexcept
{
    return m_ID.IsValid() && m_pFrameBuffer && m_pFrameBuffer->IsValid() && m_pShaderProgram && m_pShaderProgram->IsValid();
}


const FrameBuffer& Pipeline::GetFrameBuffer() noexcept
{
    ENG_ASSERT(IsValid(), "Pipeline is invalid");
    return *m_pFrameBuffer;
}


const ShaderProgram& Pipeline::GetShaderProgram() noexcept
{
    ENG_ASSERT(IsValid(), "Pipeline is invalid");
    return *m_pShaderProgram;
}


void Pipeline::SetupColorBlending() noexcept
{
    static constexpr uint32_t colorMaskRBit = static_cast<uint32_t>(ColorComponentFlags::COLOR_COMPONENT_R_BIT);
    static constexpr uint32_t colorMaskGBit = static_cast<uint32_t>(ColorComponentFlags::COLOR_COMPONENT_G_BIT);
    static constexpr uint32_t colorMaskBBit = static_cast<uint32_t>(ColorComponentFlags::COLOR_COMPONENT_B_BIT);
    static constexpr uint32_t colorMaskABit = static_cast<uint32_t>(ColorComponentFlags::COLOR_COMPONENT_A_BIT);

    bool isAnyBlendFactorConstant = false;

    for (uint32_t index = 0; index < m_compressedColorBlendAttachmentStates.size(); ++index) {
        const CompressedColorAttachmentBlendState& state = m_compressedColorBlendAttachmentStates[index];
        
        const GLboolean rMask = state.colorWriteMask & colorMaskRBit ? GL_TRUE : GL_FALSE;
        const GLboolean gMask = state.colorWriteMask & colorMaskGBit ? GL_TRUE : GL_FALSE;
        const GLboolean bMask = state.colorWriteMask & colorMaskBBit ? GL_TRUE : GL_FALSE;
        const GLboolean aMask = state.colorWriteMask & colorMaskABit ? GL_TRUE : GL_FALSE;

        glColorMaski(state.attachmentIndex, rMask, gMask, bMask, aMask);

        if (state.blendEnable) {
            glEnablei(GL_BLEND, state.attachmentIndex);

            const GLenum srcRGBBlendFactor = CompressedBlendFactorToGLEnum(state.srcRGBBlendFactor);
            const GLenum dstRGBBlendFactor = CompressedBlendFactorToGLEnum(state.dstRGBBlendFactor);
            const GLenum srcAlphaBlendFactor = CompressedBlendFactorToGLEnum(state.srcAlphaBlendFactor);
            const GLenum dstAlphaBlendFactor = CompressedBlendFactorToGLEnum(state.dstAlphaBlendFactor);

            const GLenum rgbBlendOp = CompressedBlendOpToGLEnum(state.rgbBlendOp);
            const GLenum alphaBlendOp = CompressedBlendOpToGLEnum(state.alphaBlendOp);

            glBlendEquationSeparatei(state.attachmentIndex, rgbBlendOp, alphaBlendOp);
            glBlendFuncSeparatei(state.attachmentIndex, srcRGBBlendFactor, dstRGBBlendFactor, srcAlphaBlendFactor, dstAlphaBlendFactor);

            isAnyBlendFactorConstant = isAnyBlendFactorConstant
                || IsBlendFactorConstant(srcRGBBlendFactor)
                || IsBlendFactorConstant(dstRGBBlendFactor)
                || IsBlendFactorConstant(srcAlphaBlendFactor)
                || IsBlendFactorConstant(dstAlphaBlendFactor);
        } else {
            glDisablei(GL_BLEND, state.attachmentIndex);
        }
    }

    const float blendConstRed = m_blendConstants[0] * isAnyBlendFactorConstant;
    const float blendConstGreen = m_blendConstants[1] * isAnyBlendFactorConstant;
    const float blendConstBlue = m_blendConstants[2] * isAnyBlendFactorConstant;
    const float blendConstAlpha = m_blendConstants[3] * isAnyBlendFactorConstant;

    glBlendColor(blendConstRed, blendConstGreen, blendConstBlue, blendConstAlpha);

    if (m_compressedGlobalState.colorBlendLogicOpEnable) {
        glEnable(GL_COLOR_LOGIC_OP);

        const GLenum colorLogicOp = CompressedLogicOpToGLEnum(m_compressedGlobalState.colorBlendLogicOp);
        glLogicOp(colorLogicOp);
    } else {
        glDisable(GL_COLOR_LOGIC_OP);
    }
}


void Pipeline::SetupDepthTesting() noexcept
{
    const CompressedGlobalState& state = m_compressedGlobalState;

    if (state.depthTestEnable) {
        glEnable(GL_DEPTH_TEST);

        if (state.depthWriteEnable) {
            glDepthMask(GL_TRUE);

            const PolygonMode polygonMode = static_cast<PolygonMode>(state.polygonMode);
            SetupPolygonOffset(polygonMode, m_depthBiasConstantFactor, m_depthBiasSlopeFactor, m_depthBiasClamp, state.depthBiasEnabled);

            const GLenum depthCompareFunc = CompressedCompareFuncToGLEnum(state.depthCompareFunc);
            glDepthFunc(depthCompareFunc);
        } else {
            glDepthMask(GL_FALSE);
        }
    } else {
        glDisable(GL_DEPTH_TEST);
    }
}


void Pipeline::SetupStencilTesting() noexcept
{
    if (m_compressedGlobalState.stencilTestEnable) {
        glEnable(GL_STENCIL_TEST);

        SetupFaceStencilTesting(
            GL_FRONT, 
            m_stencilFrontMask, 
            m_compressedGlobalState.frontFaceStencilFailOp, 
            m_compressedGlobalState.frontFaceStencilPassDepthFailOp, 
            m_compressedGlobalState.frontFaceStencilPassDepthPassOp, 
            m_compressedGlobalState.stencilFrontWriteEnable);

        SetupFaceStencilTesting(
            GL_BACK, 
            m_stencilBackMask, 
            m_compressedGlobalState.backFaceStencilFailOp, 
            m_compressedGlobalState.backFaceStencilPassDepthFailOp, 
            m_compressedGlobalState.backFaceStencilPassDepthPassOp, 
            m_compressedGlobalState.stencilBackWriteEnable);
    } else {
        glDisable(GL_STENCIL_TEST);
    }
}


bool Pipeline::Create(const PipelineCreateInfo &createInfo) noexcept
{
    ENG_ASSERT(!IsValid(), "Attempt to create already valid pipeline (ID: {})", m_ID.Value());
    ENG_ASSERT(m_ID.IsValid(), "Pipeline ID is invalid. You must initialize only pipelines which were returned by PipelineManager");

    ENG_ASSERT(createInfo.pInputAssemblyState,     "pInputAssemblyState is nullptr");
    ENG_ASSERT(createInfo.pRasterizationState,     "pRasterizationState is nullptr");
    ENG_ASSERT(createInfo.pDepthStencilState,      "pDepthStencilState is nullptr");
    ENG_ASSERT(createInfo.pColorBlendState,        "pColorBlendState is nullptr");
    ENG_ASSERT(createInfo.pFrameBufferClearValues, "pFrameBufferClearValues is nullptr");

    ENG_ASSERT(createInfo.pFrameBuffer && createInfo.pFrameBuffer->IsValid(), "Invalid frame buffer");
    ENG_ASSERT(createInfo.pShaderProgram && createInfo.pShaderProgram->IsValid(), "Invalid shader program");

    if (createInfo.pColorBlendState->attachmentCount > 0) {
        ENG_ASSERT(createInfo.pColorBlendState->pAttachmentStates, "pAttachmentStates is nullptr but attachmentCount is greater than 0");
        ENG_ASSERT(createInfo.pFrameBuffer->GetAttachmentsCount() == createInfo.pColorBlendState->attachmentCount, 
            "Frame buffer attachments count is not equal to pColorBlendState->attachmentCount");
    }

    if (createInfo.pFrameBufferClearValues->colorAttachmentsCount > 0) {
        ENG_ASSERT(createInfo.pFrameBufferClearValues->pColorAttachmentClearColors, 
            "pColorAttachmentClearColors is nullptr but colorAttachmentsCount is greater than 0");
    }

    const FrameBufferClearValues& frameBufferClearValues = *createInfo.pFrameBufferClearValues;
    m_frameBufferColorAttachmentClearColors.resize(frameBufferClearValues.colorAttachmentsCount);

    for (size_t i = 0; i < frameBufferClearValues.colorAttachmentsCount; ++i) {
        m_frameBufferColorAttachmentClearColors[i] = frameBufferClearValues.pColorAttachmentClearColors[i];
    }

    m_depthClearValue = frameBufferClearValues.depthClearValue;
    m_stencilClearValue = frameBufferClearValues.stencilClearValue;

    const InputAssemblyStateCreateInfo& inputAssemblyState = *createInfo.pInputAssemblyState;
    m_compressedGlobalState.primitiveTopology = static_cast<uint32_t>(inputAssemblyState.topology);

    const RasterizationStateCreateInfo& rasterizationState = *createInfo.pRasterizationState;
    m_compressedGlobalState.frontFace = static_cast<uint32_t>(rasterizationState.frontFace);
    m_compressedGlobalState.polygonMode = static_cast<uint32_t>(rasterizationState.polygonMode);
    m_compressedGlobalState.cullMode = static_cast<uint32_t>(rasterizationState.cullMode);
    m_compressedGlobalState.depthBiasEnabled = rasterizationState.depthBiasEnable;
    m_depthBiasConstantFactor = rasterizationState.depthBiasConstantFactor;
    m_depthBiasClamp = rasterizationState.depthBiasClamp;
    m_depthBiasSlopeFactor = rasterizationState.depthBiasSlopeFactor;
    m_lineWidth = rasterizationState.lineWidth;

    const DepthStencilStateCreateInfo& depthStencilState = *createInfo.pDepthStencilState;
    m_compressedGlobalState.depthCompareFunc = static_cast<uint32_t>(depthStencilState.depthCompareFunc);
    m_compressedGlobalState.frontFaceStencilFailOp = static_cast<uint32_t>(depthStencilState.frontFaceStencilFailOp);
    m_compressedGlobalState.frontFaceStencilPassDepthPassOp = static_cast<uint32_t>(depthStencilState.frontFaceStencilPassDepthPassOp);
    m_compressedGlobalState.frontFaceStencilPassDepthFailOp = static_cast<uint32_t>(depthStencilState.frontFaceStencilPassDepthFailOp);
    m_compressedGlobalState.backFaceStencilFailOp = static_cast<uint32_t>(depthStencilState.backFaceStencilFailOp);
    m_compressedGlobalState.backFaceStencilPassDepthPassOp = static_cast<uint32_t>(depthStencilState.backFaceStencilPassDepthPassOp);
    m_compressedGlobalState.backFaceStencilPassDepthFailOp = static_cast<uint32_t>(depthStencilState.backFaceStencilPassDepthFailOp);
    m_compressedGlobalState.depthTestEnable = depthStencilState.depthTestEnable;
    m_compressedGlobalState.depthWriteEnable = depthStencilState.depthWriteEnable;
    m_compressedGlobalState.stencilTestEnable = depthStencilState.stencilTestEnable;
    m_compressedGlobalState.stencilFrontWriteEnable = depthStencilState.stencilFrontWriteEnable;
    m_compressedGlobalState.stencilBackWriteEnable = depthStencilState.stencilBackWriteEnable;
    m_stencilFrontMask = depthStencilState.stencilFrontMask;
    m_stencilBackMask = depthStencilState.stencilBackMask;

    const ColorBlendStateCreateInfo& colorBlendState = *createInfo.pColorBlendState;
    m_compressedColorBlendAttachmentStates.resize(colorBlendState.attachmentCount);
    
    for (size_t i = 0; i < colorBlendState.attachmentCount; ++i) {
        CompressedColorAttachmentBlendState& compressedState = m_compressedColorBlendAttachmentStates[i];
        const ColorBlendAttachmentState& state = colorBlendState.pAttachmentStates[i];

        ENG_ASSERT(state.attachmentIndex < powl(2, BITS_PER_COLOR_ATTACHMENT_INDEX), "Attachment index is greate than maximum value");

        compressedState.attachmentIndex = state.attachmentIndex;

        const uint32_t writeMask = static_cast<uint32_t>(state.colorWriteMask);
        static constexpr uint32_t writeMaskRBit = static_cast<uint32_t>(ColorComponentFlags::COLOR_COMPONENT_R_BIT);
        static constexpr uint32_t writeMaskGBit = static_cast<uint32_t>(ColorComponentFlags::COLOR_COMPONENT_G_BIT);
        static constexpr uint32_t writeMaskBBit = static_cast<uint32_t>(ColorComponentFlags::COLOR_COMPONENT_B_BIT);
        static constexpr uint32_t writeMaskABit = static_cast<uint32_t>(ColorComponentFlags::COLOR_COMPONENT_A_BIT);
        compressedState.colorWriteMask |= (writeMask & writeMaskRBit);
        compressedState.colorWriteMask |= (writeMask & writeMaskGBit);
        compressedState.colorWriteMask |= (writeMask & writeMaskBBit);
        compressedState.colorWriteMask |= (writeMask & writeMaskABit);
        
        compressedState.srcRGBBlendFactor = static_cast<uint32_t>(state.srcRGBBlendFactor);
        compressedState.dstRGBBlendFactor = static_cast<uint32_t>(state.dstRGBBlendFactor);
        compressedState.rgbBlendOp = static_cast<uint32_t>(state.rgbBlendOp);
        compressedState.srcAlphaBlendFactor = static_cast<uint32_t>(state.srcAlphaBlendFactor);
        compressedState.dstAlphaBlendFactor = static_cast<uint32_t>(state.dstAlphaBlendFactor);
        compressedState.alphaBlendOp = static_cast<uint32_t>(state.alphaBlendOp);
        compressedState.blendEnable = state.blendEnable;
    }

    m_blendConstants[0] = colorBlendState.blendConstants[0];
    m_blendConstants[1] = colorBlendState.blendConstants[1];
    m_blendConstants[2] = colorBlendState.blendConstants[2];
    m_blendConstants[3] = colorBlendState.blendConstants[3];
    m_compressedGlobalState.colorBlendLogicOp = static_cast<uint32_t>(colorBlendState.logicOp);
    m_compressedGlobalState.colorBlendLogicOpEnable = colorBlendState.logicOpEnable;

    m_pFrameBuffer = createInfo.pFrameBuffer;
    m_pShaderProgram = createInfo.pShaderProgram;

    return true;
}


void Pipeline::Destroy()
{
    if (!IsValid()) {
        return;
    }

    m_frameBufferColorAttachmentClearColors.clear();
    m_compressedColorBlendAttachmentStates.clear();
    m_blendConstants[0] = 0.f;
    m_blendConstants[1] = 0.f;
    m_blendConstants[2] = 0.f;
    m_blendConstants[3] = 0.f;

    m_pFrameBuffer = nullptr;
    m_pShaderProgram = nullptr;

    memset(&m_compressedGlobalState, 0, sizeof(m_compressedGlobalState));

    m_ID.Invalidate();

    m_depthBiasConstantFactor = 0.f;
    m_depthBiasClamp = 0.f;
    m_depthBiasSlopeFactor = 0.f;
    m_lineWidth = 0.f;
    m_depthClearValue = 0.f;
    m_stencilClearValue = 0;
    m_stencilFrontMask = 0;
    m_stencilBackMask = 0;
}


PipelineManager& PipelineManager::GetInstance() noexcept
{
    ENG_ASSERT(engIsRenderPipelineInitialized(), "Render pipeline manager is not initialized");
    return *pPipelineMngInst;
}


Pipeline* PipelineManager::RegisterPipeline() noexcept
{
    const PipelineID pipelineID = AllocatePipelineID();
    Pipeline* pPipeline = &m_pipelineStorage[pipelineID.Value()];

    ENG_ASSERT(!pPipeline->IsValid(), "Valid graphics pipeline was returned during registration");

    pPipeline->m_ID = pipelineID;

    return pPipeline;
}


void PipelineManager::UnregisterPipeline(Pipeline* pPipeline) noexcept
{
    if (!pPipeline) {
        return;
    }

    if (pPipeline->IsValid()) {
        ENG_LOG_WARN("Unregistration of pipeline \'{}\' while it's steel valid. Prefer to destroy buffers manually", pPipeline->m_ID.Value());
        pPipeline->Destroy();
    }

    DeallocatePipelineID(pPipeline->m_ID);

    pPipeline->m_ID.Invalidate();
}


bool PipelineManager::Init() noexcept
{   
    if (IsInitialized()) {
        return true;
    }

    m_pipelineStorage.resize(ENG_MAX_PIPELINES_COUNT);

#if defined(ENG_USE_INVERTED_Z)
    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
#endif
    
    m_isInitialized = true;

    return true;
}


void PipelineManager::Terminate() noexcept
{
    for (Pipeline& pipeline : m_pipelineStorage) {
        pipeline.Destroy();
    }

    m_isInitialized = false;
}


PipelineID PipelineManager::AllocatePipelineID() noexcept
{
    if (m_pipelineIDFreeList.empty()) {
        ENG_ASSERT(m_nextAllocatedID.Value() < m_pipelineStorage.size() - 1, "Pipeline storage overflow");

        const PipelineID pipelineID = m_nextAllocatedID;
        m_nextAllocatedID = PipelineID(m_nextAllocatedID.Value() + 1);

        return pipelineID;
    }

    const PipelineID pipelineID = m_pipelineIDFreeList.front();
    m_pipelineIDFreeList.pop_front();
        
    return pipelineID;
}


void PipelineManager::DeallocatePipelineID(PipelineID ID) noexcept
{
    if (ID < m_nextAllocatedID && std::find(m_pipelineIDFreeList.cbegin(), m_pipelineIDFreeList.cend(), ID) == m_pipelineIDFreeList.cend()) {
        m_pipelineIDFreeList.emplace_back(ID);
    }
}


uint64_t amHash(const Pipeline &pipeline) noexcept
{
    return pipeline.Hash();
}


bool engInitPipelineManager() noexcept
{
    if (engIsRenderPipelineInitialized()) {
        ENG_LOG_WARN("Pipeline manager is already initialized!");
        return true;
    }

    ENG_ASSERT(engIsRenderTargetManagerInitialized(), "Render target manager must be initialized before pipeline target manager!");
    ENG_ASSERT(engIsShaderManagerInitialized(), "Shader manager must be initialized before pipeline target manager!");

    pPipelineMngInst = std::unique_ptr<PipelineManager>(new PipelineManager);

    if (!pPipelineMngInst) {
        ENG_ASSERT_FAIL("Failed to allocate memory for pipeline manager");
        return false;
    }

    if (!pPipelineMngInst->Init()) {
        ENG_ASSERT_FAIL("Failed to initialized pipeline manager");
        return false;
    }

    return true;
}


void engTerminatePipelineManager() noexcept
{
    pPipelineMngInst = nullptr;
}


bool engIsRenderPipelineInitialized() noexcept
{
    return pPipelineMngInst && pPipelineMngInst->IsInitialized();
}

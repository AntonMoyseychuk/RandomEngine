#include "pch.h"
#include "render_system.h"

#include "engine/engine.h"

#include "render/texture_manager/texture_mng.h"
#include "render/rt_manager/rt_manager.h"
#include "render/shader_manager/shader_mng.h"
#include "render/pipeline_manager/pipeline_mng.h"
#include "render/mem_manager/buffer_manager.h"
#include "render/mesh_manager/mesh_manager.h"

#include "utils/file/file.h"
#include "utils/debug/assertion.h"
#include "utils/timer/timer.h"

#include "render/platform/OpenGL/opengl_driver.h"

#include "auto/auto_texture_constants.h"
#include "auto/auto_registers_common.h"


static std::unique_ptr<RenderSystem> g_pRenderSystem = nullptr;


RenderSystem& RenderSystem::GetInstance() noexcept
{
    ENG_ASSERT_GRAPHICS_API(engIsRenderSystemInitialized(), "Render system is not initialized");
    return *g_pRenderSystem;
}


void RenderSystem::BeginFrame() noexcept
{
}


void RenderSystem::EndFrame() noexcept
{

}


void RenderSystem::RunDepthPrepass() noexcept
{

}


void RenderSystem::RunGBufferPass() noexcept
{

}


void RenderSystem::RunColorPass() noexcept
{
    static Timer timer;
    timer.Tick();

    Window& window = *WindowSystem::GetInstance().GetWindowByTag(WINDOW_TAG_MAIN);
    TextureManager& texManager = TextureManager::GetInstance();
    ShaderManager& shaderManager = ShaderManager::GetInstance();
    RenderTargetManager& rtManager = RenderTargetManager::GetInstance();
    PipelineManager& pipelineManager = PipelineManager::GetInstance();
    MemoryBufferManager& memBufferManager = MemoryBufferManager::GetInstance();

    static bool isInitialized = false;
    static ShaderProgram* pGBufferProgram = nullptr;
    static ShaderProgram* pMergeProgram = nullptr;

    static Texture* pTestTexture = nullptr;
    static TextureSamplerState* pTestTextureSampler = nullptr;

    static uint32_t vao = 0;

    static Texture* pGBufferAlbedoTex = nullptr;
    static TextureSamplerState* pGBufferAlbedoSampler = nullptr;
    static Texture* pGBufferNormalTex = nullptr;
    static Texture* pGBufferSpecTex = nullptr;
    static Texture* pCommonDepthTex = nullptr;

    static Pipeline* pGBufferPipeline = nullptr;
    static Pipeline* pMergePipeline = nullptr;

    static MemoryBuffer* pCommonConstBuffer = nullptr;

    if (!isInitialized) {
        static constexpr const char* SHADER_INCLUDE_DIR = "D:\\Studies\\Graphics\\random-graphics\\engine\\source\\shaders\\include";
        static const char* GBUFFER_DEFINES[] = {
        #if defined(ENG_DEBUG)
            "ENV_DEBUG",
        #endif
            "PASS_GBUFFER"
        };

        ShaderStageCreateInfo vsStageCreateInfo = {};

        vsStageCreateInfo.type = ShaderStageType::VERTEX;

        const std::vector<char> vsSourceCode = ReadTextFile("D:\\Studies\\Graphics\\random-graphics\\engine\\source\\shaders\\source\\base\\base.vs");
        vsStageCreateInfo.pSourceCode = vsSourceCode.data();
        vsStageCreateInfo.codeSize = vsSourceCode.size();

        vsStageCreateInfo.pDefines = GBUFFER_DEFINES;
        vsStageCreateInfo.definesCount = _countof(GBUFFER_DEFINES);

        vsStageCreateInfo.pIncludeParentPath = SHADER_INCLUDE_DIR;


        ShaderStageCreateInfo psStageCreateInfo = {};

        psStageCreateInfo.type = ShaderStageType::PIXEL;

        const std::vector<char> psSourceCode = ReadTextFile("D:\\Studies\\Graphics\\random-graphics\\engine\\source\\shaders\\source\\base\\base.fs");
        psStageCreateInfo.pSourceCode = psSourceCode.data();
        psStageCreateInfo.codeSize = psSourceCode.size();

        psStageCreateInfo.pDefines = GBUFFER_DEFINES;
        psStageCreateInfo.definesCount = _countof(GBUFFER_DEFINES);

        psStageCreateInfo.pIncludeParentPath = SHADER_INCLUDE_DIR;


        const ShaderStageCreateInfo* pGBufferStages[] = { &vsStageCreateInfo, &psStageCreateInfo };

        ShaderProgramCreateInfo gBufferPassProgramCreateInfo = {};
        gBufferPassProgramCreateInfo.pStageCreateInfos = pGBufferStages;
        gBufferPassProgramCreateInfo.stageCreateInfosCount = _countof(pGBufferStages);

        pGBufferProgram = shaderManager.RegisterShaderProgram("Pass_GBuffer");
        ENG_ASSERT(pGBufferProgram, "Failed to register GBUFFER shader program");
        pGBufferProgram->Create(gBufferPassProgramCreateInfo);
        ENG_ASSERT(pGBufferProgram, "Failed to create GBUFFER shader program");

        static const char* MERGE_DEFINES[] = {
        #if defined(ENG_DEBUG)
            "ENV_DEBUG",
        #endif
            "PASS_MERGE"
        };

        vsStageCreateInfo.pDefines = MERGE_DEFINES;
        vsStageCreateInfo.definesCount = _countof(MERGE_DEFINES);

        psStageCreateInfo.pDefines = MERGE_DEFINES;
        psStageCreateInfo.definesCount = _countof(MERGE_DEFINES);

        const ShaderStageCreateInfo* pMergeStages[] = { &vsStageCreateInfo, &psStageCreateInfo };

        ShaderProgramCreateInfo gMergePassProgramCreateInfo = {};
        gMergePassProgramCreateInfo.pStageCreateInfos = pMergeStages;
        gMergePassProgramCreateInfo.stageCreateInfosCount = _countof(pMergeStages);

        pMergeProgram = shaderManager.RegisterShaderProgram("Pass_Merge");
        ENG_ASSERT(pMergeProgram, "Failed to register MERGE shader program");
        pMergeProgram->Create(gMergePassProgramCreateInfo);
        ENG_ASSERT(pMergeProgram, "Failed to create MERGE shader program");


        constexpr size_t texWidth = 256;
        constexpr size_t texWidthDiv2 = texWidth / 2;
        constexpr size_t texHeight = 256;
        constexpr size_t texHeightDiv2 = texHeight / 2;
        constexpr size_t texSizeInPixels = texWidth * texHeight;
        constexpr size_t texComponentsCount = 4;
        constexpr size_t texSizeInBytes = texSizeInPixels * texComponentsCount;
        
        constexpr uint8_t texColors[4][4] = {
            { 255,   0,   0, 255 },
            {   0, 255,   0, 255 },
            {   0,   0, 255, 255 },
            {   255, 0, 255, 255 },
        };

        uint8_t pTexData[texSizeInBytes] = {};
        for (size_t y = 0; y < texHeight; ++y) {
            for (size_t x = 0; x < texWidth; ++x) {
                const size_t colorIdx = (y / (texHeightDiv2 - 1) + ((y / (texHeightDiv2 - 1)) % 2)) + x / texWidthDiv2;

                const size_t pixelIdx = (y * texWidth + x);

                pTexData[texComponentsCount * pixelIdx + 0] = texColors[colorIdx][0];
                pTexData[texComponentsCount * pixelIdx + 1] = texColors[colorIdx][1];
                pTexData[texComponentsCount * pixelIdx + 2] = texColors[colorIdx][2];
                pTexData[texComponentsCount * pixelIdx + 3] = texColors[colorIdx][3];
            }
        }

        Texture2DCreateInfo texCreateInfo = {};
        texCreateInfo.format = resGetTexResourceFormat(TEST_TEXTURE);
        texCreateInfo.width = texWidth;
        texCreateInfo.height = texHeight;
        texCreateInfo.mipmapsCount = 0;
        texCreateInfo.inputData.format = TextureInputDataFormat::INPUT_FORMAT_RGBA;
        texCreateInfo.inputData.dataType = TextureInputDataType::INPUT_TYPE_UNSIGNED_BYTE;

        texCreateInfo.inputData.pData = pTexData;

        ds::StrID testTexName = "TEST_TEXTURE";
        pTestTexture = texManager.RegisterTexture2D(testTexName);
        ENG_ASSERT(pTestTexture, "Failed to register texture: {}", testTexName.CStr());
        pTestTexture->Create(texCreateInfo);
        ENG_ASSERT(pTestTexture->IsValid(), "Failed to create texture: {}", testTexName.CStr());

        pTestTextureSampler = texManager.GetSampler(resGetTexResourceSamplerIdx(TEST_TEXTURE));

        pGBufferAlbedoTex = rtManager.GetRTTexture(RTTextureID::RT_TEX_GBUFFER_ALBEDO);
        pGBufferNormalTex = rtManager.GetRTTexture(RTTextureID::RT_TEX_GBUFFER_NORMAL);
        pGBufferSpecTex = rtManager.GetRTTexture(RTTextureID::RT_TEX_GBUFFER_SPECULAR);
        pCommonDepthTex = rtManager.GetRTTexture(RTTextureID::RT_TEX_COMMON_DEPTH);

        pGBufferAlbedoSampler = texManager.GetSampler(resGetTexResourceSamplerIdx(GBUFFER_ALBEDO_TEX));


        PipelineInputAssemblyStateCreateInfo gBufferInputAssemblyState = {};
        gBufferInputAssemblyState.topology = PrimitiveTopology::TOPOLOGY_TRIANGLES;

        PipelineRasterizationStateCreateInfo gBufferRasterizationState = {};
        gBufferRasterizationState.cullMode = CullMode::CULL_MODE_BACK;
        gBufferRasterizationState.depthBiasEnable = false;
        gBufferRasterizationState.frontFace = FrontFace::FRONT_FACE_COUNTER_CLOCKWISE;
        gBufferRasterizationState.polygonMode = PolygonMode::POLYGON_MODE_FILL;

        PipelineDepthStencilStateCreateInfo gBufferDepthStencilState = {};
        gBufferDepthStencilState.depthTestEnable = false;
        gBufferDepthStencilState.stencilTestEnable = false;

        PipelineColorBlendStateCreateInfo gBufferColorBlendState = {};

        PipelineFrameBufferClearValues gBufferFrameBufferClearValues = {};
        const PipelineFrameBufferColorAttachmentClearColor pGBufferColorAttachmentClearColors[] = {
            { 1.f, 1.f, 0.f, 0.f },
            { 1.f, 1.f, 0.f, 0.f },
            { 1.f, 1.f, 0.f, 0.f }
        };
        gBufferFrameBufferClearValues.pColorAttachmentClearColors = pGBufferColorAttachmentClearColors;
        gBufferFrameBufferClearValues.colorAttachmentsCount = _countof(pGBufferColorAttachmentClearColors);

        PipelineCreateInfo gBufferPipelineCreateInfo = {};
        gBufferPipelineCreateInfo.pInputAssemblyState = &gBufferInputAssemblyState;
        gBufferPipelineCreateInfo.pRasterizationState = &gBufferRasterizationState;
        gBufferPipelineCreateInfo.pDepthStencilState = &gBufferDepthStencilState;
        gBufferPipelineCreateInfo.pColorBlendState = &gBufferColorBlendState;
        gBufferPipelineCreateInfo.pFrameBufferClearValues = &gBufferFrameBufferClearValues;
        gBufferPipelineCreateInfo.pFrameBuffer = rtManager.GetFrameBuffer(RTFrameBufferID::RT_FRAMEBUFFER_GBUFFER);
        gBufferPipelineCreateInfo.pShaderProgram = pGBufferProgram;

        PipelineID gBufferPipelineID = pipelineManager.RegisterPipeline(gBufferPipelineCreateInfo);
        ENG_ASSERT(pipelineManager.IsValidPipeline(gBufferPipelineID), "Failed to register GBUFFER pipeline");
        pGBufferPipeline = pipelineManager.GetPipeline(gBufferPipelineID);


        PipelineInputAssemblyStateCreateInfo mergeInputAssemblyState = {};
        mergeInputAssemblyState.topology = PrimitiveTopology::TOPOLOGY_TRIANGLES;

        PipelineRasterizationStateCreateInfo mergeRasterizationState = {};
        mergeRasterizationState.cullMode = CullMode::CULL_MODE_BACK;
        mergeRasterizationState.depthBiasEnable = false;
        mergeRasterizationState.frontFace = FrontFace::FRONT_FACE_COUNTER_CLOCKWISE;
        mergeRasterizationState.polygonMode = PolygonMode::POLYGON_MODE_FILL;

        PipelineDepthStencilStateCreateInfo mergeDepthStencilState = {};
        mergeDepthStencilState.depthTestEnable = false;
        mergeDepthStencilState.stencilTestEnable = false;

        PipelineColorBlendStateCreateInfo mergeColorBlendState = {};

        PipelineFrameBufferClearValues mergeFrameBufferClearValues = {};
        const PipelineFrameBufferColorAttachmentClearColor pMergeColorAttachmentClearColors[] = {
            { 0.f, 0.f, 0.f, 0.f }
        };
        mergeFrameBufferClearValues.pColorAttachmentClearColors = pMergeColorAttachmentClearColors;
        mergeFrameBufferClearValues.colorAttachmentsCount = _countof(pMergeColorAttachmentClearColors);

        PipelineCreateInfo mergePipelineCreateInfo = {};
        mergePipelineCreateInfo.pInputAssemblyState = &mergeInputAssemblyState;
        mergePipelineCreateInfo.pRasterizationState = &mergeRasterizationState;
        mergePipelineCreateInfo.pDepthStencilState = &mergeDepthStencilState;
        mergePipelineCreateInfo.pColorBlendState = &mergeColorBlendState;
        mergePipelineCreateInfo.pFrameBufferClearValues = &mergeFrameBufferClearValues;
        mergePipelineCreateInfo.pFrameBuffer = rtManager.GetFrameBuffer(RTFrameBufferID::RT_FRAMEBUFFER_DEFAULT);
        mergePipelineCreateInfo.pShaderProgram = pMergeProgram;

        PipelineID mergePipelineID = pipelineManager.RegisterPipeline(mergePipelineCreateInfo);
        ENG_ASSERT(pipelineManager.IsValidPipeline(mergePipelineID), "Failed to register merge pipeline");
        pMergePipeline = pipelineManager.GetPipeline(mergePipelineID);

        glCreateVertexArrays(1, &vao);
        glBindVertexArray(vao);

        MemoryBufferCreateInfo commonConstBufferCreateInfo = {};
        commonConstBufferCreateInfo.type = MemoryBufferType::TYPE_CONSTANT_BUFFER;
        commonConstBufferCreateInfo.dataSize = sizeof(COMMON_CB);
        commonConstBufferCreateInfo.elementSize = sizeof(COMMON_CB);
        commonConstBufferCreateInfo.creationFlags = static_cast<MemoryBufferCreationFlags>(
            BUFFER_CREATION_FLAG_DYNAMIC_STORAGE | BUFFER_CREATION_FLAG_READABLE | BUFFER_CREATION_FLAG_WRITABLE);
        commonConstBufferCreateInfo.pData = nullptr;

        pCommonConstBuffer = memBufferManager.RegisterBuffer("__COMMON_CB__");
        ENG_ASSERT(pCommonConstBuffer, "Failed to register common const buffer");
        pCommonConstBuffer->Create(commonConstBufferCreateInfo);
        ENG_ASSERT(pCommonConstBuffer->IsValid(), "Failed to create common const buffer");
        
        pCommonConstBuffer->BindIndexed(resGetResourceBinding(COMMON_CB).GetBinding());

        isInitialized = true;
    }

    const float elapsedTime = timer.GetElapsedTimeInMillisec();
    const float deltaTime = timer.GetDeltaTimeInMillisec();

    char title[256];
    sprintf_s(title, "%.3f ms | %.1f FPS", deltaTime, 1000.f / deltaTime);
    window.SetTitle(title);
    
    glViewport(0, 0, window.GetFramebufferWidth(), window.GetFramebufferHeight());

    {
        pGBufferPipeline->ClearFrameBuffer();
        pGBufferPipeline->Bind();

        COMMON_CB* pCommonUBO = static_cast<COMMON_CB*>(pCommonConstBuffer->MapWrite());
        ENG_ASSERT(pCommonUBO, "pCommonUBO is nullptr");
        pCommonUBO->COMMON_ELAPSED_TIME = elapsedTime / 4000.f;
        pCommonUBO->COMMON_DELTA_TIME = deltaTime;
        pCommonConstBuffer->Unmap();

        pTestTexture->Bind(resGetResourceBinding(TEST_TEXTURE).GetBinding());
        pTestTextureSampler->Bind(resGetResourceBinding(TEST_TEXTURE).GetBinding());

        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);
    }

    {
        pMergePipeline->ClearFrameBuffer();
        pMergePipeline->Bind();

        pGBufferAlbedoTex->Bind(resGetResourceBinding(GBUFFER_ALBEDO_TEX).GetBinding());
        pGBufferAlbedoSampler->Bind(resGetResourceBinding(GBUFFER_ALBEDO_TEX).GetBinding());

        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);
    }
}


void RenderSystem::RunPostprocessingPass() noexcept
{

}


RenderSystem::~RenderSystem()
{
    Terminate();
}


bool RenderSystem::Init() noexcept
{
    if (IsInitialized()) {
        return true;
    }

    if (!engInitOpenGLDriver()) {
        return false;
    }

    if (!engInitShaderManager()) {
        return false;
    }

    if (!engInitTextureManager()) {
        return false;
    }

    if (!engInitRenderTargetManager()) {
        return false;
    }

    if (!engInitPipelineManager()) {
        return false;
    }

    if (!engInitMemoryBufferManager()) {
        return false;
    }

    m_isInitialized = true;

    return true;
}
    
    
void RenderSystem::Terminate() noexcept
{
    engTerminateMemoryBufferManager();
    engTerminatePipelineManager();
    engTerminateRenderTargetManager();
    engTerminateTextureManager();
    engTerminateShaderManager();

    m_isInitialized = false;
}


bool RenderSystem::IsInitialized() const noexcept
{
    return m_isInitialized;
}


bool engInitRenderSystem() noexcept
{
    if (engIsRenderSystemInitialized()) {
        ENG_LOG_GRAPHICS_API_WARN("Render system is already initialized!");
        return true;
    }

    if (!engIsWindowSystemInitialized()) {
        ENG_ASSERT_GRAPHICS_API_FAIL("Window system must be initialized before render system");
        return false;
    }

    g_pRenderSystem = std::unique_ptr<RenderSystem>(new RenderSystem);
    if (!g_pRenderSystem) {
        ENG_ASSERT_GRAPHICS_API_FAIL("Failed to create render system");
        return false;
    }

    if (!g_pRenderSystem->Init()) {
        ENG_ASSERT_GRAPHICS_API_FAIL("Failed to initialized render system");
        return false;
    }

    return true;
}


void engTerminateRenderSystem() noexcept
{
    g_pRenderSystem = nullptr;
}


bool engIsRenderSystemInitialized() noexcept
{
    return g_pRenderSystem && g_pRenderSystem->IsInitialized();
}
#include "pch.h"
#include "render_system.h"

#include "engine/engine.h"

#include "render/texture_manager/texture_mng.h"
#include "render/rt_manager/rt_manager.h"
#include "render/shader_manager/shader_mng.h"
#include "render/pipeline_manager/pipeline_mng.h"
#include "render/mem_manager/buffer_manager.h"
#include "render/mesh_manager/mesh_manager.h"

#include "core/camera/camera_manager.h"

#include "utils/file/file.h"
#include "utils/debug/assertion.h"
#include "utils/timer/timer.h"

#include "render/platform/OpenGL/opengl_driver.h"

#include "auto/registers_common.h"


static std::unique_ptr<RenderSystem> pRenderSysInst = nullptr;


#define INIT_CALL(CALL, ...) if (!CALL(__VA_ARGS__)) { return false; } 


RenderSystem& RenderSystem::GetInstance() noexcept
{
    ENG_ASSERT(engIsRenderSystemInitialized(), "Render system is not initialized");
    return *pRenderSysInst;
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

    static Window& window = engGetMainWindow();
    static TextureManager& texManager = TextureManager::GetInstance();
    static ShaderManager& shaderManager = ShaderManager::GetInstance();
    static RenderTargetManager& rtManager = RenderTargetManager::GetInstance();
    static PipelineManager& pipelineManager = PipelineManager::GetInstance();
    static MemoryBufferManager& memBufferManager = MemoryBufferManager::GetInstance();
    static MeshDataManager& meshDataManager = MeshDataManager::GetInstance();
    static MeshManager& meshManager = MeshManager::GetInstance();
    static CameraManager& cameraManager = CameraManager::GetInstance();

    static bool isInitialized = false;
    static ShaderProgram* pGBufferProgram = nullptr;
    static ShaderProgram* pMergeProgram = nullptr;

    static Texture* pTestTexture = nullptr;
    static TextureSamplerState* pTestTextureSampler = nullptr;

    static MeshObj* pCubeMeshObj = nullptr;

    static Texture* pGBufferAlbedoTex = nullptr;
    static TextureSamplerState* pGBufferAlbedoSampler = nullptr;
    static Texture* pGBufferNormalTex = nullptr;
    static Texture* pGBufferSpecTex = nullptr;
    static Texture* pCommonDepthTex = nullptr;

    static Pipeline* pGBufferPipeline = nullptr;
    static Pipeline* pMergePipeline = nullptr;

    static MemoryBuffer* pCommonConstBuffer = nullptr;
    static MemoryBuffer* pCameraConstBuffer = nullptr;

    static Camera* pMainCam = nullptr;

    if (!isInitialized) {
        srand(time(0));

        static constexpr const char* SHADER_INCLUDE_DIR = ENG_ENGINE_DIR "/source/shaders/include";
        static const char* GBUFFER_DEFINES[] = {
        #if defined(ENG_DEBUG)
            "ENV_DEBUG",
        #endif
            "PASS_GBUFFER"
        };

        ShaderStageCreateInfo vsStageCreateInfo = {};

        vsStageCreateInfo.type = ShaderStageType::VERTEX;

        const std::vector<char> vsSourceCode = ReadTextFile(ENG_ENGINE_DIR "/source/shaders/source/base/base.vs");
        vsStageCreateInfo.pSourceCode = vsSourceCode.data();
        vsStageCreateInfo.codeSize = vsSourceCode.size();

        vsStageCreateInfo.pDefines = GBUFFER_DEFINES;
        vsStageCreateInfo.definesCount = _countof(GBUFFER_DEFINES);

        vsStageCreateInfo.pIncludeParentPath = SHADER_INCLUDE_DIR;


        ShaderStageCreateInfo psStageCreateInfo = {};

        psStageCreateInfo.type = ShaderStageType::PIXEL;

        const std::vector<char> psSourceCode = ReadTextFile(ENG_ENGINE_DIR "/source/shaders/source/base/base.fs");
        psStageCreateInfo.pSourceCode = psSourceCode.data();
        psStageCreateInfo.codeSize = psSourceCode.size();

        psStageCreateInfo.pDefines = GBUFFER_DEFINES;
        psStageCreateInfo.definesCount = _countof(GBUFFER_DEFINES);

        psStageCreateInfo.pIncludeParentPath = SHADER_INCLUDE_DIR;


        const ShaderStageCreateInfo* pGBufferStages[] = { &vsStageCreateInfo, &psStageCreateInfo };

        ShaderProgramCreateInfo gBufferPassProgramCreateInfo = {};
        gBufferPassProgramCreateInfo.pStageCreateInfos = pGBufferStages;
        gBufferPassProgramCreateInfo.stageCreateInfosCount = _countof(pGBufferStages);

        pGBufferProgram = shaderManager.RegisterShaderProgram();
        ENG_ASSERT(pGBufferProgram, "Failed to register GBUFFER shader program");
        pGBufferProgram->Create(gBufferPassProgramCreateInfo);
        ENG_ASSERT(pGBufferProgram, "Failed to create GBUFFER shader program");
        pGBufferProgram->SetDebugName("Pass_GBuffer");

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

        pMergeProgram = shaderManager.RegisterShaderProgram();
        ENG_ASSERT(pMergeProgram, "Failed to register MERGE shader program");
        pMergeProgram->Create(gMergePassProgramCreateInfo);
        ENG_ASSERT(pMergeProgram, "Failed to create MERGE shader program");
        pMergeProgram->SetDebugName("Pass_Merge");


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
        gBufferDepthStencilState.depthTestEnable = true;
        gBufferDepthStencilState.depthWriteEnable = true;
        gBufferDepthStencilState.depthCompareFunc = CompareFunc::FUNC_LEQUAL;
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

        pGBufferPipeline = pipelineManager.RegisterPipeline();
        ENG_ASSERT(pGBufferPipeline, "Failed to register GBUFFER pipeline");
        pGBufferPipeline->Create(gBufferPipelineCreateInfo);
        ENG_ASSERT(pGBufferPipeline->IsValid(), "Failed to create GBUFFER pipeline");


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

        pMergePipeline = pipelineManager.RegisterPipeline();
        ENG_ASSERT(pMergePipeline, "Failed to register MERGE pipeline");
        pMergePipeline->Create(mergePipelineCreateInfo);
        ENG_ASSERT(pMergePipeline->IsValid(), "Failed to create MERGE pipeline");


        const MeshVertexAttribDesc pVertexAttribDescs[] = {
            MeshVertexAttribDesc {  0 * sizeof(float), MeshVertexAttribDataType::TYPE_FLOAT, 0, 3, false },
            MeshVertexAttribDesc {  3 * sizeof(float), MeshVertexAttribDataType::TYPE_FLOAT, 1, 4, false },
            MeshVertexAttribDesc {  7 * sizeof(float), MeshVertexAttribDataType::TYPE_FLOAT, 2, 3, false },
            MeshVertexAttribDesc { 10 * sizeof(float), MeshVertexAttribDataType::TYPE_FLOAT, 3, 2, false },
        };

        MeshVertexLayoutCreateInfo cubeVertexLayoutCreateInfo = {};
        cubeVertexLayoutCreateInfo.pVertexAttribDescs = pVertexAttribDescs;
        cubeVertexLayoutCreateInfo.vertexAttribDescsCount = _countof(pVertexAttribDescs);

        MeshVertexLayout* pCubeVertexLayout = meshDataManager.RegisterVertexLayout(cubeVertexLayoutCreateInfo);
        ENG_ASSERT(pCubeVertexLayout && pCubeVertexLayout->IsValid(), "Failed to register cube mesh vertex layout");

        MeshGPUBufferData* pCubeBufferData = meshDataManager.RegisterGPUBufferData("cube");
        ENG_ASSERT(pCubeBufferData, "Failed to register cube mesh GPU data");

        constexpr float CUBE_HALF_SIZE = 0.5f;

        const float pCubeRawVertexData[] = {
            // position                                     //color             // normal      // UV
            // Front face
           -CUBE_HALF_SIZE,-CUBE_HALF_SIZE, CUBE_HALF_SIZE, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f,
           -CUBE_HALF_SIZE, CUBE_HALF_SIZE, CUBE_HALF_SIZE, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 1.f,
            CUBE_HALF_SIZE, CUBE_HALF_SIZE, CUBE_HALF_SIZE, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 1.f, 1.f,
            CUBE_HALF_SIZE,-CUBE_HALF_SIZE, CUBE_HALF_SIZE, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f,

            // Back face
            CUBE_HALF_SIZE,-CUBE_HALF_SIZE,-CUBE_HALF_SIZE, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f,-1.f, 0.f, 0.f,
            CUBE_HALF_SIZE, CUBE_HALF_SIZE,-CUBE_HALF_SIZE, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f,-1.f, 0.f, 1.f,
           -CUBE_HALF_SIZE, CUBE_HALF_SIZE,-CUBE_HALF_SIZE, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f,-1.f, 1.f, 1.f,
           -CUBE_HALF_SIZE,-CUBE_HALF_SIZE,-CUBE_HALF_SIZE, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f,-1.f, 1.f, 0.f,

            // Left face
           -CUBE_HALF_SIZE,-CUBE_HALF_SIZE,-CUBE_HALF_SIZE, 0.f, 0.f, 1.f, 1.f,-1.f, 0.f, 0.f, 0.f, 0.f,
           -CUBE_HALF_SIZE, CUBE_HALF_SIZE,-CUBE_HALF_SIZE, 0.f, 0.f, 1.f, 1.f,-1.f, 0.f, 0.f, 0.f, 1.f,
           -CUBE_HALF_SIZE, CUBE_HALF_SIZE, CUBE_HALF_SIZE, 0.f, 0.f, 1.f, 1.f,-1.f, 0.f, 0.f, 1.f, 1.f,
           -CUBE_HALF_SIZE,-CUBE_HALF_SIZE, CUBE_HALF_SIZE, 0.f, 0.f, 1.f, 1.f,-1.f, 0.f, 0.f, 1.f, 0.f,

            // Right face
            CUBE_HALF_SIZE,-CUBE_HALF_SIZE, CUBE_HALF_SIZE, 1.f, 1.f, 0.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f,
            CUBE_HALF_SIZE, CUBE_HALF_SIZE, CUBE_HALF_SIZE, 1.f, 1.f, 0.f, 1.f, 1.f, 0.f, 0.f, 0.f, 1.f,
            CUBE_HALF_SIZE, CUBE_HALF_SIZE,-CUBE_HALF_SIZE, 1.f, 1.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f,
            CUBE_HALF_SIZE,-CUBE_HALF_SIZE,-CUBE_HALF_SIZE, 1.f, 1.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 0.f,

            // Top face
           -CUBE_HALF_SIZE, CUBE_HALF_SIZE, CUBE_HALF_SIZE, 1.f, 0.f, 1.f, 1.f, 0.f, 1.f, 0.f, 0.f, 0.f,
           -CUBE_HALF_SIZE, CUBE_HALF_SIZE,-CUBE_HALF_SIZE, 1.f, 0.f, 1.f, 1.f, 0.f, 1.f, 0.f, 0.f, 1.f,
            CUBE_HALF_SIZE, CUBE_HALF_SIZE,-CUBE_HALF_SIZE, 1.f, 0.f, 1.f, 1.f, 0.f, 1.f, 0.f, 1.f, 1.f,
            CUBE_HALF_SIZE, CUBE_HALF_SIZE, CUBE_HALF_SIZE, 1.f, 0.f, 1.f, 1.f, 0.f, 1.f, 0.f, 1.f, 0.f,

            // Bottom face
           -CUBE_HALF_SIZE,-CUBE_HALF_SIZE,-CUBE_HALF_SIZE, 0.f, 1.f, 1.f, 1.f, 0.f,-1.f, 0.f, 0.f, 0.f,
           -CUBE_HALF_SIZE,-CUBE_HALF_SIZE, CUBE_HALF_SIZE, 0.f, 1.f, 1.f, 1.f, 0.f,-1.f, 0.f, 0.f, 1.f,
            CUBE_HALF_SIZE,-CUBE_HALF_SIZE, CUBE_HALF_SIZE, 0.f, 1.f, 1.f, 1.f, 0.f,-1.f, 0.f, 1.f, 1.f,
            CUBE_HALF_SIZE,-CUBE_HALF_SIZE,-CUBE_HALF_SIZE, 0.f, 1.f, 1.f, 1.f, 0.f,-1.f, 0.f, 1.f, 0.f,
        };

        const uint8_t cubeIndices[] = {
            0, 2, 1, 0, 3, 2,
            4, 6, 5, 4, 7, 6,
            8, 10, 9, 8, 11, 10,
            12, 14, 13, 12, 15, 14,
            16, 18, 17, 16, 19, 18,
            20, 22, 21, 20, 23, 22
        };

        MeshGPUBufferDataCreateInfo cubeGPUDataCreateInfo = {};
        cubeGPUDataCreateInfo.pVertexData = pCubeRawVertexData;
        cubeGPUDataCreateInfo.vertexDataSize = sizeof(pCubeRawVertexData);
        cubeGPUDataCreateInfo.vertexSize = sizeof(pCubeRawVertexData) / 24;
        cubeGPUDataCreateInfo.pIndexData = cubeIndices;
        cubeGPUDataCreateInfo.indexDataSize = sizeof(cubeIndices);
        cubeGPUDataCreateInfo.indexSize = sizeof(cubeIndices[0]);

        pCubeBufferData->Create(cubeGPUDataCreateInfo);

        pCubeMeshObj = meshManager.RegisterMeshObj("cube");
        ENG_ASSERT(pCubeMeshObj, "Failed to register cube mesh object");
        pCubeMeshObj->Create(pCubeVertexLayout, pCubeBufferData);
        ENG_ASSERT(pCubeMeshObj->IsValid(), "Failed to create cube mesh object");

        pCubeMeshObj->Bind();


        MemoryBufferCreateInfo commonConstBufferCreateInfo = {};
        commonConstBufferCreateInfo.type = MemoryBufferType::TYPE_CONSTANT_BUFFER;
        commonConstBufferCreateInfo.dataSize = sizeof(COMMON_CB);
        commonConstBufferCreateInfo.elementSize = sizeof(COMMON_CB);
        commonConstBufferCreateInfo.creationFlags = static_cast<MemoryBufferCreationFlags>(
            BUFFER_CREATION_FLAG_DYNAMIC_STORAGE | BUFFER_CREATION_FLAG_READABLE | BUFFER_CREATION_FLAG_WRITABLE);
        commonConstBufferCreateInfo.pData = nullptr;

        pCommonConstBuffer = memBufferManager.RegisterBuffer();
        ENG_ASSERT(pCommonConstBuffer, "Failed to register common const buffer");
        pCommonConstBuffer->Create(commonConstBufferCreateInfo);
        ENG_ASSERT(pCommonConstBuffer->IsValid(), "Failed to create common const buffer");
        pCommonConstBuffer->SetDebugName("__COMMON_CB__");
        
        pCommonConstBuffer->BindIndexed(resGetResourceBinding(COMMON_CB).GetBinding());

        MemoryBufferCreateInfo cameraConstBufferCreateInfo = {};
        cameraConstBufferCreateInfo.type = MemoryBufferType::TYPE_CONSTANT_BUFFER;
        cameraConstBufferCreateInfo.dataSize = sizeof(COMMON_CAMERA_CB);
        cameraConstBufferCreateInfo.elementSize = sizeof(COMMON_CAMERA_CB);
        cameraConstBufferCreateInfo.creationFlags = static_cast<MemoryBufferCreationFlags>(
            BUFFER_CREATION_FLAG_DYNAMIC_STORAGE | BUFFER_CREATION_FLAG_WRITABLE);
        cameraConstBufferCreateInfo.pData = nullptr;

        pCameraConstBuffer = MemoryBufferManager::GetInstance().RegisterBuffer();
        ENG_ASSERT(pCameraConstBuffer, "Failed to register camera const buffer");
        pCameraConstBuffer->Create(cameraConstBufferCreateInfo);
        ENG_ASSERT(pCameraConstBuffer->IsValid(), "Failed to create camera const buffer");
        pCameraConstBuffer->SetDebugName("__COMMON_CAMERA_CB__");
        
        pCameraConstBuffer->BindIndexed(resGetResourceBinding(COMMON_CAMERA_CB).GetBinding());
        
        pMainCam = cameraManager.RegisterCamera();
        ENG_ASSERT(pMainCam && pMainCam->IsRegistered(), "Failed to register camera");
        
        pMainCam->SetPerspProjection();
        
        pMainCam->SetZNear(0.1f);
        pMainCam->SetZFar(100.f);

        pMainCam->SetPosition(glm::vec3(0.f, 0.f, 2.f));
        pMainCam->SetRotation(glm::quatLookAt(glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 1.f, 0.f)));

        pMainCam->SetAspectRatio(window.GetFramebufferWidth(), window.GetFramebufferHeight());
        pMainCam->SetFovDegress(90.f);

        cameraManager.SubscribeCamera<EventFramebufferResized>(*pMainCam, [](const void* pEvent) {
            const EventFramebufferResized& event = CastEventTo<EventFramebufferResized>(pEvent);
            const uint32_t width = event.GetWidth();
            const uint32_t height = event.GetHeight();
    
            const float halfWidth = width * 0.5f;
            const float halfHeight = height * 0.5f;
            
            if (width > 0 && height > 0) {
                pMainCam->SetAspectRatio(width, height);
                
                pMainCam->SetOrthoLeft(-halfWidth);
                pMainCam->SetOrthoRight(halfWidth);
                pMainCam->SetOrthoBottom(-halfHeight);
                pMainCam->SetOrthoTop(halfHeight);
            }
        });

        isInitialized = true;

        return;
    }

    const float elapsedTime = timer.GetElapsedTimeInMillisec();
    const float deltaTime = timer.GetDeltaTimeInMillisec();

    char title[256];
    sprintf_s(title, "%.3f ms | %.1f FPS", deltaTime, 1000.f / deltaTime);
    window.SetTitle(title);
    
    glViewport(0, 0, window.GetFramebufferWidth(), window.GetFramebufferHeight());

    COMMON_CAMERA_CB* pCamConstBuff = pCameraConstBuffer->MapWrite<COMMON_CAMERA_CB>();
    ENG_ASSERT(pCamConstBuff, "Failed to map camera const buffer");

    const glm::mat4x4 cameraViewMat = glm::transpose(pMainCam->GetViewMatrix());
    constexpr size_t commonViewMatSize = sizeof(pCamConstBuff->COMMON_VIEW_MATRIX);
    memcpy_s(pCamConstBuff->COMMON_VIEW_MATRIX, commonViewMatSize, &cameraViewMat, commonViewMatSize);
    
    const glm::mat4x4 cameraProjMat = glm::transpose(pMainCam->GetProjectionMatrix());
    constexpr size_t commonProjMatSize = sizeof(pCamConstBuff->COMMON_PROJ_MATRIX);
    memcpy_s(&pCamConstBuff->COMMON_PROJ_MATRIX, commonProjMatSize, &cameraProjMat, commonProjMatSize);
    
    pCamConstBuff->COMMON_VIEW_Z_NEAR = pMainCam->GetZNear();
    pCamConstBuff->COMMON_VIEW_Z_FAR = pMainCam->GetZFar();

    pCameraConstBuffer->Unmap();

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

        glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, 0, 1);
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

    INIT_CALL(engInitOpenGLDriver);
    INIT_CALL(engInitShaderManager);
    INIT_CALL(engInitTextureManager);
    INIT_CALL(engInitRenderTargetManager);
    INIT_CALL(engInitPipelineManager);
    INIT_CALL(engInitMemoryBufferManager);
    INIT_CALL(engInitMeshManager);

    m_isInitialized = true;

    return true;
}
    
    
void RenderSystem::Terminate() noexcept
{
    engTerminateMeshManager();
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

    pRenderSysInst = std::unique_ptr<RenderSystem>(new RenderSystem);
    if (!pRenderSysInst) {
        ENG_ASSERT_GRAPHICS_API_FAIL("Failed to create render system");
        return false;
    }

    if (!pRenderSysInst->Init()) {
        ENG_ASSERT_GRAPHICS_API_FAIL("Failed to initialized render system");
        return false;
    }

    return true;
}


void engTerminateRenderSystem() noexcept
{
    pRenderSysInst = nullptr;
}


bool engIsRenderSystemInitialized() noexcept
{
    return pRenderSysInst && pRenderSysInst->IsInitialized();
}
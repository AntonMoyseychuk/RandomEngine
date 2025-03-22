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
#include "core/window_system/window_system.h"

#include "utils/file/file.h"
#include "utils/debug/assertion.h"
#include "utils/timer/timer.h"

#include "render/platform/OpenGL/opengl_driver.h"

#include "auto/auto_registers_common.h"


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
    static Input& input = window.GetInput();
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
    static ShaderProgram* pPostProcProgram = nullptr;

    static Texture* pTestTexture = nullptr;
    static TextureSamplerState* pTestTextureSampler = nullptr;

    static MeshObj* pCubeMeshObj = nullptr;

    static Texture* pGBufferAlbedoTex = nullptr;
    static Texture* pGBufferNormalTex = nullptr;
    static Texture* pGBufferSpecTex = nullptr;
    static Texture* pCommonDepthTex = nullptr;
    static TextureSamplerState* pGBufferAlbedoSampler = nullptr;
    static TextureSamplerState* pGBufferNormalSampler = nullptr;
    static TextureSamplerState* pGBufferSpecSampler = nullptr;
    static TextureSamplerState* pGBufferDepthSampler = nullptr;

    static Pipeline* pGBufferPipeline = nullptr;
    static Pipeline* pPostProcPipeline = nullptr;

    static MemoryBuffer* pCommonConstBuffer = nullptr;
    static MemoryBuffer* pCameraConstBuffer = nullptr;

    static Camera* pMainCam = nullptr;

    if (!isInitialized) {
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

        static const char* POST_PROCESS_DEFINES[] = {
        #if defined(ENG_DEBUG)
            "ENV_DEBUG",
        #endif
            "PASS_POST_PROCESS"
        };

        vsStageCreateInfo.pDefines = POST_PROCESS_DEFINES;
        vsStageCreateInfo.definesCount = _countof(POST_PROCESS_DEFINES);

        psStageCreateInfo.pDefines = POST_PROCESS_DEFINES;
        psStageCreateInfo.definesCount = _countof(POST_PROCESS_DEFINES);

        const ShaderStageCreateInfo* pPostProcStages[] = { &vsStageCreateInfo, &psStageCreateInfo };

        ShaderProgramCreateInfo gPostProcPassProgramCreateInfo = {};
        gPostProcPassProgramCreateInfo.pStageCreateInfos = pPostProcStages;
        gPostProcPassProgramCreateInfo.stageCreateInfosCount = _countof(pPostProcStages);

        pPostProcProgram = shaderManager.RegisterShaderProgram();
        ENG_ASSERT(pPostProcProgram, "Failed to register POST PROCESS shader program");
        pPostProcProgram->Create(gPostProcPassProgramCreateInfo);
        ENG_ASSERT(pPostProcProgram, "Failed to create POST PROCESS shader program");
        pPostProcProgram->SetDebugName("Pass_Post_Process");


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

        pGBufferAlbedoTex = rtManager.GetRTTexture(RTTextureID::GBUFFER_ALBEDO);
        pGBufferNormalTex = rtManager.GetRTTexture(RTTextureID::GBUFFER_NORMAL);
        pGBufferSpecTex = rtManager.GetRTTexture(RTTextureID::GBUFFER_SPECULAR);
        pCommonDepthTex = rtManager.GetRTTexture(RTTextureID::COMMON_DEPTH);

        pGBufferAlbedoSampler = texManager.GetSampler(resGetTexResourceSamplerIdx(GBUFFER_ALBEDO_TEX));
        pGBufferNormalSampler = texManager.GetSampler(resGetTexResourceSamplerIdx(GBUFFER_NORMAL_TEX));
        pGBufferSpecSampler = texManager.GetSampler(resGetTexResourceSamplerIdx(GBUFFER_SPECULAR_TEX));
        pGBufferDepthSampler = texManager.GetSampler(resGetTexResourceSamplerIdx(COMMON_DEPTH_TEX));


        InputAssemblyStateCreateInfo gBufferInputAssemblyState = {};
        gBufferInputAssemblyState.topology = PrimitiveTopology::TOPOLOGY_TRIANGLES;

        RasterizationStateCreateInfo gBufferRasterizationState = {};
        gBufferRasterizationState.cullMode = CullMode::CULL_MODE_BACK;
        gBufferRasterizationState.depthBiasEnable = false;
        gBufferRasterizationState.frontFace = FrontFace::FRONT_FACE_COUNTER_CLOCKWISE;
        gBufferRasterizationState.polygonMode = PolygonMode::POLYGON_MODE_FILL;

        DepthStencilStateCreateInfo gBufferDepthStencilState = {};
        gBufferDepthStencilState.depthTestEnable = true;
        gBufferDepthStencilState.depthWriteEnable = true;
        gBufferDepthStencilState.depthCompareFunc = CompareFunc::FUNC_GREATER;
        gBufferDepthStencilState.stencilTestEnable = false;

        ColorBlendStateCreateInfo gBufferColorBlendState = {};
        
        ColorBlendAttachmentState gBufferAlbedoBlendState = {};
        gBufferAlbedoBlendState.colorWriteMask.value = ColorComponentFlags::MASK_ALL;
        ColorBlendAttachmentState gBufferNormalBlendState = {};
        gBufferNormalBlendState.colorWriteMask.value = ColorComponentFlags::MASK_ALL;
        ColorBlendAttachmentState gBufferSpecularBlendState = {};
        gBufferSpecularBlendState.colorWriteMask.value = ColorComponentFlags::MASK_ALL;
        
        ColorBlendAttachmentState gBufferColorAttachmentsBlendStates[] = { gBufferAlbedoBlendState, gBufferNormalBlendState, gBufferSpecularBlendState };
        gBufferColorBlendState.pAttachmentStates = gBufferColorAttachmentsBlendStates;
        gBufferColorBlendState.attachmentCount = _countof(gBufferColorAttachmentsBlendStates);

        FrameBufferClearValues gBufferFrameBufferClearValues = {};
        const FrameBufferColorAttachmentClearColor pGBufferColorAttachmentClearColors[] = {
            { 1.f, 1.f, 0.f, 0.f },
            { 0.f, 0.f, 0.f, 0.f },
            { 0.f, 0.f, 0.f, 0.f }
        };
        gBufferFrameBufferClearValues.pColorAttachmentClearColors = pGBufferColorAttachmentClearColors;
        gBufferFrameBufferClearValues.colorAttachmentsCount = _countof(pGBufferColorAttachmentClearColors);
        gBufferFrameBufferClearValues.depthClearValue = 0.f;

        PipelineCreateInfo gBufferPipelineCreateInfo = {};
        gBufferPipelineCreateInfo.pInputAssemblyState = &gBufferInputAssemblyState;
        gBufferPipelineCreateInfo.pRasterizationState = &gBufferRasterizationState;
        gBufferPipelineCreateInfo.pDepthStencilState = &gBufferDepthStencilState;
        gBufferPipelineCreateInfo.pColorBlendState = &gBufferColorBlendState;
        gBufferPipelineCreateInfo.pFrameBufferClearValues = &gBufferFrameBufferClearValues;
        gBufferPipelineCreateInfo.pFrameBuffer = rtManager.GetFrameBuffer(RTFrameBufferID::GBUFFER);
        gBufferPipelineCreateInfo.pShaderProgram = pGBufferProgram;

        pGBufferPipeline = pipelineManager.RegisterPipeline();
        ENG_ASSERT(pGBufferPipeline, "Failed to register GBUFFER pipeline");
        pGBufferPipeline->Create(gBufferPipelineCreateInfo);
        ENG_ASSERT(pGBufferPipeline->IsValid(), "Failed to create GBUFFER pipeline");


        InputAssemblyStateCreateInfo postProcInputAssemblyState = {};
        postProcInputAssemblyState.topology = PrimitiveTopology::TOPOLOGY_TRIANGLES;

        RasterizationStateCreateInfo postProcRasterizationState = {};
        postProcRasterizationState.cullMode = CullMode::CULL_MODE_BACK;
        postProcRasterizationState.depthBiasEnable = false;
        postProcRasterizationState.frontFace = FrontFace::FRONT_FACE_COUNTER_CLOCKWISE;
        postProcRasterizationState.polygonMode = PolygonMode::POLYGON_MODE_FILL;

        DepthStencilStateCreateInfo postProcDepthStencilState = {};
        postProcDepthStencilState.depthTestEnable = false;
        postProcDepthStencilState.stencilTestEnable = false;

        ColorBlendStateCreateInfo postProcColorBlendState = {};

        ColorBlendAttachmentState postProcColorBlendAttachmentState = {};
        postProcColorBlendAttachmentState.colorWriteMask.value = ColorComponentFlags::MASK_ALL;

        ColorBlendAttachmentState postProcColorAttachmentsBlendStates[] = { postProcColorBlendAttachmentState };
        postProcColorBlendState.pAttachmentStates = postProcColorAttachmentsBlendStates;
        postProcColorBlendState.attachmentCount = _countof(postProcColorAttachmentsBlendStates);

        FrameBufferClearValues postProcFrameBufferClearValues = {};
        const FrameBufferColorAttachmentClearColor pPostProcColorAttachmentClearColors[] = {
            { 0.f, 0.f, 0.f, 0.f },
        };
        postProcFrameBufferClearValues.pColorAttachmentClearColors = pPostProcColorAttachmentClearColors;
        postProcFrameBufferClearValues.colorAttachmentsCount = _countof(pPostProcColorAttachmentClearColors);

        PipelineCreateInfo postProcPipelineCreateInfo = {};
        postProcPipelineCreateInfo.pInputAssemblyState = &postProcInputAssemblyState;
        postProcPipelineCreateInfo.pRasterizationState = &postProcRasterizationState;
        postProcPipelineCreateInfo.pDepthStencilState = &postProcDepthStencilState;
        postProcPipelineCreateInfo.pColorBlendState = &postProcColorBlendState;
        postProcPipelineCreateInfo.pFrameBufferClearValues = &postProcFrameBufferClearValues;
        postProcPipelineCreateInfo.pFrameBuffer = rtManager.GetFrameBuffer(RTFrameBufferID::POST_PROCESS);
        postProcPipelineCreateInfo.pShaderProgram = pPostProcProgram;

        pPostProcPipeline = pipelineManager.RegisterPipeline();
        ENG_ASSERT(pPostProcPipeline, "Failed to register POST PROCESS pipeline");
        pPostProcPipeline->Create(postProcPipelineCreateInfo);
        ENG_ASSERT(pPostProcPipeline->IsValid(), "Failed to create POST PROCESS pipeline");


        const MeshVertexAttribDesc pVertexAttribDescs[] = {
            MeshVertexAttribDesc { 0 * sizeof(float), MeshVertexAttribDataType::TYPE_FLOAT, 0, 3, false },
            MeshVertexAttribDesc { 3 * sizeof(float), MeshVertexAttribDataType::TYPE_FLOAT, 1, 3, false },
            MeshVertexAttribDesc { 6 * sizeof(float), MeshVertexAttribDataType::TYPE_FLOAT, 2, 2, false },
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
            // position                                     // normal                       // UV
            // Front face
           -CUBE_HALF_SIZE,-CUBE_HALF_SIZE, CUBE_HALF_SIZE, -0.57735f, -0.57735f, 0.57735f, 0.f, 0.f,
           -CUBE_HALF_SIZE, CUBE_HALF_SIZE, CUBE_HALF_SIZE, -0.57735f,  0.57735f, 0.57735f, 0.f, 1.f,
            CUBE_HALF_SIZE, CUBE_HALF_SIZE, CUBE_HALF_SIZE,  0.57735f,  0.57735f, 0.57735f, 1.f, 1.f,
            CUBE_HALF_SIZE,-CUBE_HALF_SIZE, CUBE_HALF_SIZE,  0.57735f, -0.57735f, 0.57735f, 1.f, 0.f,

            // Back face
            CUBE_HALF_SIZE,-CUBE_HALF_SIZE,-CUBE_HALF_SIZE,  0.57735f, -0.57735f, -0.57735f, 0.f, 0.f,
            CUBE_HALF_SIZE, CUBE_HALF_SIZE,-CUBE_HALF_SIZE,  0.57735f,  0.57735f, -0.57735f, 0.f, 1.f,
           -CUBE_HALF_SIZE, CUBE_HALF_SIZE,-CUBE_HALF_SIZE, -0.57735f,  0.57735f, -0.57735f, 1.f, 1.f,
           -CUBE_HALF_SIZE,-CUBE_HALF_SIZE,-CUBE_HALF_SIZE, -0.57735f, -0.57735f, -0.57735f, 1.f, 0.f,

            // Left face
           -CUBE_HALF_SIZE,-CUBE_HALF_SIZE,-CUBE_HALF_SIZE, -0.57735f, -0.57735f, -0.57735f, 0.f, 0.f,
           -CUBE_HALF_SIZE, CUBE_HALF_SIZE,-CUBE_HALF_SIZE, -0.57735f,  0.57735f, -0.57735f, 0.f, 1.f,
           -CUBE_HALF_SIZE, CUBE_HALF_SIZE, CUBE_HALF_SIZE, -0.57735f,  0.57735f,  0.57735f, 1.f, 1.f,
           -CUBE_HALF_SIZE,-CUBE_HALF_SIZE, CUBE_HALF_SIZE, -0.57735f, -0.57735f,  0.57735f, 1.f, 0.f,

            // Right face
            CUBE_HALF_SIZE,-CUBE_HALF_SIZE, CUBE_HALF_SIZE,  0.57735f, -0.57735f,  0.57735f, 0.f, 0.f,
            CUBE_HALF_SIZE, CUBE_HALF_SIZE, CUBE_HALF_SIZE,  0.57735f,  0.57735f,  0.57735f, 0.f, 1.f,
            CUBE_HALF_SIZE, CUBE_HALF_SIZE,-CUBE_HALF_SIZE,  0.57735f,  0.57735f, -0.57735f, 1.f, 1.f,
            CUBE_HALF_SIZE,-CUBE_HALF_SIZE,-CUBE_HALF_SIZE,  0.57735f, -0.57735f, -0.57735f, 1.f, 0.f,

            // Top face
           -CUBE_HALF_SIZE, CUBE_HALF_SIZE, CUBE_HALF_SIZE, -0.57735f,  0.57735f,  0.57735f, 0.f, 0.f,
           -CUBE_HALF_SIZE, CUBE_HALF_SIZE,-CUBE_HALF_SIZE, -0.57735f,  0.57735f, -0.57735f, 0.f, 1.f,
            CUBE_HALF_SIZE, CUBE_HALF_SIZE,-CUBE_HALF_SIZE,  0.57735f,  0.57735f, -0.57735f, 1.f, 1.f,
            CUBE_HALF_SIZE, CUBE_HALF_SIZE, CUBE_HALF_SIZE,  0.57735f,  0.57735f,  0.57735f, 1.f, 0.f,

            // Bottom face
           -CUBE_HALF_SIZE,-CUBE_HALF_SIZE,-CUBE_HALF_SIZE, -0.57735f, -0.57735f, -0.57735f, 0.f, 0.f,
           -CUBE_HALF_SIZE,-CUBE_HALF_SIZE, CUBE_HALF_SIZE, -0.57735f, -0.57735f,  0.57735f, 0.f, 1.f,
            CUBE_HALF_SIZE,-CUBE_HALF_SIZE, CUBE_HALF_SIZE,  0.57735f, -0.57735f,  0.57735f, 1.f, 1.f,
            CUBE_HALF_SIZE,-CUBE_HALF_SIZE,-CUBE_HALF_SIZE,  0.57735f, -0.57735f, -0.57735f, 1.f, 0.f,
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


        MemoryBufferCreateInfo commonConstBufferCreateInfo = {};
        commonConstBufferCreateInfo.type = MemoryBufferType::TYPE_CONSTANT_BUFFER;
        commonConstBufferCreateInfo.dataSize = sizeof(COMMON_DYN_CB);
        commonConstBufferCreateInfo.elementSize = sizeof(COMMON_DYN_CB);
        commonConstBufferCreateInfo.creationFlags = static_cast<MemoryBufferCreationFlags>(
            BUFFER_CREATION_FLAG_DYNAMIC_STORAGE | BUFFER_CREATION_FLAG_READABLE | BUFFER_CREATION_FLAG_WRITABLE);
        commonConstBufferCreateInfo.pData = nullptr;

        pCommonConstBuffer = memBufferManager.RegisterBuffer();
        ENG_ASSERT(pCommonConstBuffer, "Failed to register common const buffer");
        pCommonConstBuffer->Create(commonConstBufferCreateInfo);
        ENG_ASSERT(pCommonConstBuffer->IsValid(), "Failed to create common const buffer");
        pCommonConstBuffer->SetDebugName("__COMMON_DYN_CB__");

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
        
        pMainCam->SetZNear(0.01f);
        pMainCam->SetZFar(100.f);

        pMainCam->SetPosition(glm::vec3(0.f, 0.f, 2.f));
        pMainCam->SetRotation(glm::quatLookAt(-M3D_AXIS_Z, M3D_AXIS_Y));

        pMainCam->SetAspectRatio(window.GetFramebufferWidth(), window.GetFramebufferHeight());
        pMainCam->SetFovDegress(90.f);

        cameraManager.SubscribeCamera<EventFramebufferResized>(*pMainCam, [](const void* pEvent) {
            const EventFramebufferResized& event = es::EventCast<EventFramebufferResized>(pEvent);
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

    static bool isStrIDMemLoged = false;
    if (!isStrIDMemLoged) {
        ENG_LOG_INFO("StrID memory: {}/{} KB", ds::StrID::GetStorageSize() / 1024.f, ds::StrID::GetStorageCapacity() / 1024.f);
        isStrIDMemLoged = true;
    }

    const float elapsedTime = timer.GetElapsedTimeInSec();
    const float deltaTime = timer.GetDeltaTimeInSec();

    char title[256];
    sprintf_s(title, "%.3f ms | %.1f FPS", deltaTime, 1.f / deltaTime);
    window.SetTitle(title);

    glm::vec3 offset(0.f);
    
    offset += (float)input.IsKeyPressedOrHold(KeyboardKey::KEY_W) * (-pMainCam->GetZDir());
    offset += (float)input.IsKeyPressedOrHold(KeyboardKey::KEY_S) * (pMainCam->GetZDir());
    offset += (float)input.IsKeyPressedOrHold(KeyboardKey::KEY_D) * (pMainCam->GetXDir());
    offset += (float)input.IsKeyPressedOrHold(KeyboardKey::KEY_A) * (-pMainCam->GetXDir());
    offset += (float)input.IsKeyPressedOrHold(KeyboardKey::KEY_E) * (pMainCam->GetYDir());
    offset += (float)input.IsKeyPressedOrHold(KeyboardKey::KEY_Q) * (-pMainCam->GetYDir());

    if (!amIsZero(offset)) {
        pMainCam->Move(glm::normalize(offset) * deltaTime);
    }

    const float fovDegrees = pMainCam->GetFovDegrees() - input.GetMouseWheelDy();
    if (camIsFovDegreesValid(fovDegrees)) {
        pMainCam->SetFovDegress(fovDegrees);
    }

    COMMON_CAMERA_CB* pCamConstBuff = pCameraConstBuffer->MapWrite<COMMON_CAMERA_CB>();
    ENG_ASSERT(pCamConstBuff, "Failed to map camera const buffer");

    const glm::mat4x4 cameraViewMat = glm::transpose(pMainCam->GetViewMatrix());
    constexpr size_t commonViewMatSize = sizeof(pCamConstBuff->COMMON_VIEW_MATRIX);
    memcpy_s(pCamConstBuff->COMMON_VIEW_MATRIX, commonViewMatSize, &cameraViewMat, commonViewMatSize);
    
    const glm::mat4x4 cameraProjMat = glm::transpose(pMainCam->GetProjectionMatrix());
    constexpr size_t commonProjMatSize = sizeof(pCamConstBuff->COMMON_PROJ_MATRIX);
    memcpy_s(&pCamConstBuff->COMMON_PROJ_MATRIX, commonProjMatSize, &cameraProjMat, commonProjMatSize);

    const glm::mat4x4 cameraViewProjMat = glm::transpose(pMainCam->GetViewProjectionMatrix());
    constexpr size_t commonViewProjMatSize = sizeof(pCamConstBuff->COMMON_VIEW_PROJ_MATRIX);
    memcpy_s(&pCamConstBuff->COMMON_VIEW_PROJ_MATRIX, commonViewProjMatSize, &cameraViewProjMat, commonViewProjMatSize);
    
    float camZNear = pMainCam->GetZNear();
    float camZFar = pMainCam->GetZFar();
#if defined(ENG_USE_INVERTED_Z)
    std::swap(camZNear, camZFar);
#endif

    pCamConstBuff->COMMON_VIEW_Z_NEAR = camZNear;
    pCamConstBuff->COMMON_VIEW_Z_FAR = camZFar;

    pCameraConstBuffer->Unmap();

    glViewport(0, 0, window.GetFramebufferWidth(), window.GetFramebufferHeight());

    {
        pGBufferPipeline->ClearFrameBuffer();
        pGBufferPipeline->Bind();

        COMMON_DYN_CB* pCommonUBO = static_cast<COMMON_DYN_CB*>(pCommonConstBuffer->MapWrite());
        ENG_ASSERT(pCommonUBO, "pCommonUBO is nullptr");
        
        pCommonUBO->COMMON_ELAPSED_TIME  = elapsedTime;
        pCommonUBO->COMMON_DELTA_TIME    = deltaTime;
        pCommonUBO->COMMON_SCREEN_WIDTH  = (float)window.GetFramebufferWidth();
        pCommonUBO->COMMON_SCREEN_HEIGHT = (float)window.GetFramebufferHeight();
        
        pCommonConstBuffer->Unmap();

        pCommonConstBuffer->BindIndexed(resGetResourceBinding(COMMON_DYN_CB).GetBinding());

        pTestTexture->Bind(resGetResourceBinding(TEST_TEXTURE).GetBinding());
        pTestTextureSampler->Bind(resGetResourceBinding(TEST_TEXTURE).GetBinding());

        pCubeMeshObj->Bind();

        glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, 0, 1);
    }

    {
        pPostProcPipeline->ClearFrameBuffer();
        pPostProcPipeline->Bind();

        pGBufferAlbedoTex->Bind(resGetResourceBinding(GBUFFER_ALBEDO_TEX).GetBinding());
        pGBufferAlbedoSampler->Bind(resGetResourceBinding(GBUFFER_ALBEDO_TEX).GetBinding());
            
        pGBufferNormalTex->Bind(resGetResourceBinding(GBUFFER_NORMAL_TEX).GetBinding());
        pGBufferNormalSampler->Bind(resGetResourceBinding(GBUFFER_NORMAL_TEX).GetBinding());
            
        pGBufferSpecTex->Bind(resGetResourceBinding(GBUFFER_SPECULAR_TEX).GetBinding());
        pGBufferSpecSampler->Bind(resGetResourceBinding(GBUFFER_SPECULAR_TEX).GetBinding());
            
        pCommonDepthTex->Bind(resGetResourceBinding(COMMON_DEPTH_TEX).GetBinding());
        pGBufferDepthSampler->Bind(resGetResourceBinding(COMMON_DEPTH_TEX).GetBinding());

        pCommonConstBuffer->BindIndexed(resGetResourceBinding(COMMON_DYN_CB).GetBinding());

        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);
    }

    {
        const FrameBuffer* pPostProcFrameBuffer = rtManager.GetFrameBuffer(RTFrameBufferID::POST_PROCESS);
        
        glBlitNamedFramebuffer(pPostProcFrameBuffer->GetRenderID(), 0, 0, 0, window.GetWidth(), window.GetHeight(),
            0, 0, window.GetWidth(), window.GetHeight(), GL_COLOR_BUFFER_BIT, GL_LINEAR);
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
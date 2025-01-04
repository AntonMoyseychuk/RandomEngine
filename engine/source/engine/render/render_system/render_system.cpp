#include "pch.h"
#include "render_system.h"

#include "engine/engine.h"

#include "engine/render/texture_manager/texture_mng.h"
#include "engine/render/rt_manager/rt_manager.h"
#include "engine/render/shader_manager/shader_mng.h"

#include "utils/file/file.h"
#include "utils/debug/assertion.h"
#include "utils/timer/timer.h"

#include "engine/render/platform/OpenGL/opengl_driver.h"

#include "engine/auto/auto_texture_incl.h"
#include "engine/auto/auto_registers_common.h"


static std::unique_ptr<RenderSystem> g_pRenderSystem = nullptr;


#if defined(ENG_DEBUG) && defined(ENG_LOGGING_ENABLED)
static void GLAPIENTRY OpenGLMessageCallback(GLenum source, GLenum type, GLuint id,
    GLenum severity, GLsizei length, const GLchar* pMessage, const void* userParam)
{
    const char* pSourceStr = [source]() -> const char* {
		switch (source) {
            case GL_DEBUG_SOURCE_API: return "API";
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "WINDOW SYSTEM";
            case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
            case GL_DEBUG_SOURCE_THIRD_PARTY: return "THIRD PARTY";
            case GL_DEBUG_SOURCE_APPLICATION: return "APPLICATION";
            case GL_DEBUG_SOURCE_OTHER: return "OTHER";
            default: return "UNDEFINED SOURCE";
		}
	}();

	const char* pTypeStr = [type]() -> const char* {
		switch (type) {
            case GL_DEBUG_TYPE_ERROR: return "ERROR";
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
            case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
            case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
            case GL_DEBUG_TYPE_MARKER: return "MARKER";
            case GL_DEBUG_TYPE_OTHER: return "OTHER";
            default: return "UNDEFINED TYPE";
		}
	}();

    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        ENG_ASSERT_GRAPHICS_API_FAIL("[{}] ({}): {}", pSourceStr, pTypeStr, pMessage);
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        ENG_LOG_GRAPHICS_API_WARN("[{}] ({}): {}", pSourceStr, pTypeStr, pMessage);
        break;
    case GL_DEBUG_SEVERITY_LOW:
        ENG_LOG_GRAPHICS_API_INFO("[{}] ({}): {}", pSourceStr, pTypeStr, pMessage);
        break;
    }
}
#endif


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

    Window& window = Engine::GetInstance().GetWindow();
    RenderTargetManager& rtManager = RenderTargetManager::GetInstance();

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

    if (!isInitialized) {
        srand(time(0));

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
        gBufferPassProgramCreateInfo.dbgName = "Pass_GBuffer";

        gBufferPassProgramCreateInfo.pStageCreateInfos = pGBufferStages;
        gBufferPassProgramCreateInfo.stageCreateInfosCount = _countof(pGBufferStages);

        ProgramID gBufferPassProgramID = ShaderManager::GetInstance().RegisterShaderProgram(gBufferPassProgramCreateInfo);
        ENG_ASSERT_GRAPHICS_API(ShaderManager::GetInstance().IsValidProgramID(gBufferPassProgramID), "Failed to register GBUFFER shader program");

        pGBufferProgram = ShaderManager::GetInstance().GetShaderProgramByID(gBufferPassProgramID);

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
        gMergePassProgramCreateInfo.dbgName = "Pass_Merge";

        gMergePassProgramCreateInfo.pStageCreateInfos = pMergeStages;
        gMergePassProgramCreateInfo.stageCreateInfosCount = _countof(pMergeStages);

        ProgramID gMergePassProgramID = ShaderManager::GetInstance().RegisterShaderProgram(gMergePassProgramCreateInfo);
        ENG_ASSERT_GRAPHICS_API(ShaderManager::GetInstance().IsValidProgramID(gMergePassProgramID), "Failed to register MERGE shader program");

        pMergeProgram = ShaderManager::GetInstance().GetShaderProgramByID(gMergePassProgramID);


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

        TextureID textureID = TextureManager::GetInstance().AllocateTexture2D("TEST_TEXTURE", texCreateInfo);
        ENG_ASSERT_GRAPHICS_API(TextureManager::GetInstance().IsValidTextureID(textureID), "Failed to register texture");

        pTestTexture = TextureManager::GetInstance().GetTextureByID(textureID);
        pTestTextureSampler = TextureManager::GetInstance().GetSampler(resGetTexResourceSamplerIdx(TEST_TEXTURE));
        
        glCreateVertexArrays(1, &vao);
        glBindVertexArray(vao);

        pGBufferAlbedoTex = rtManager.GetRTTexture(RTTextureID::RT_TEX_GBUFFER_ALBEDO);
        pGBufferNormalTex = rtManager.GetRTTexture(RTTextureID::RT_TEX_GBUFFER_NORMAL);
        pGBufferSpecTex = rtManager.GetRTTexture(RTTextureID::RT_TEX_GBUFFER_SPECULAR);
        pCommonDepthTex = rtManager.GetRTTexture(RTTextureID::RT_TEX_COMMON_DEPTH);

        pGBufferAlbedoSampler = TextureManager::GetInstance().GetSampler(resGetTexResourceSamplerIdx(GBUFFER_ALBEDO_TEX));

        isInitialized = true;
    }

    
    glViewport(0, 0, window.GetFramebufferWidth(), window.GetFramebufferHeight());

    {
        rtManager.BindFrameBuffer(RTFrameBufferID::RT_FRAMEBUFFER_GBUFFER);
        rtManager.ClearFrameBuffer(RTFrameBufferID::RT_FRAMEBUFFER_GBUFFER, 1.f, 1.f, 0.f, 1.f, 0.f, 0);

        pGBufferProgram->Bind();

        const float elapsedTime = timer.GetElapsedTimeInSec();
        pGBufferProgram->SetLocalSrvFloat(resGetResourceBinding(COMMON_ELAPSED_TIME), elapsedTime / 2.f);    

        pTestTexture->Bind(resGetResourceBinding(TEST_TEXTURE).GetBinding());
        pTestTextureSampler->Bind(resGetResourceBinding(TEST_TEXTURE).GetBinding());

        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);
    }

    {
        rtManager.BindFrameBuffer(RTFrameBufferID::RT_FRAMEBUFFER_DEFAULT);
        rtManager.ClearFrameBuffer(RTFrameBufferID::RT_FRAMEBUFFER_DEFAULT, 1.f, 1.f, 0.f, 1.f, 0.f, 0);
        
        pMergeProgram->Bind();

        pGBufferAlbedoTex->Bind(resGetResourceBinding(GBUFFER_ALBEDO_TEX).GetBinding());
        pGBufferAlbedoSampler->Bind(resGetResourceBinding(GBUFFER_ALBEDO_TEX).GetBinding());

        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);
    }

    const float deltaTime = timer.GetDeltaTimeInMillisec();

    char title[256];
    sprintf_s(title, "%.3f ms | %.1f FPS", deltaTime, 1000.f / deltaTime);
    window.SetTitle(title);
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

#if defined(ENG_DEBUG) && defined(ENG_LOGGING_ENABLED)
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(OpenGLMessageCallback, nullptr);
#endif

    if (!engInitShaderManager()) {
        return false;
    }

    if (!engInitTextureManager()) {
        return false;
    }

    if (!engInitRenderTargetManager()) {
        return false;
    }

    m_isInitialized = true;

    return true;
}
    
    
void RenderSystem::Terminate() noexcept
{
    m_isInitialized = false;

    engTerminateRenderTargetManager();
    engTerminateTextureManager();
    engTerminateShaderManager();
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
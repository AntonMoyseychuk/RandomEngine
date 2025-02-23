#include "pch.h"
#include "camera_manager.h"

#include "engine/window_system/window_system.h"

#include "utils/debug/assertion.h"

#include "auto/auto_texture_constants.h"
#include "auto/auto_registers_common.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/vector_query.hpp>


static std::unique_ptr<CameraManager> pCameraMngInst = nullptr;


#define ASSERT_CAMERA_MNG_INIT_STATUS() ENG_ASSERT(engIsCameraManagerInitialized(), "Camera manager is not initialized")
#define ASSERT_CAMERA_INIT_STATUS() ENG_ASSERT(IsInitialized(), "Camera is not initialized")


const CameraPerspectiveCreateInfo& CameraCreateInfo::GetPerspectiveProjParams() const noexcept
{
    ENG_ASSERT(IsPerspectiveProj(), "Attempt to get perspective projection create info from ortho");
    return std::get<CameraPerspectiveCreateInfo>(projParams);
}


const CameraOrthoCreateInfo& CameraCreateInfo::GetOrthoProjParams() const noexcept
{
    ENG_ASSERT(IsOrthoProj(), "Attempt to get ortho projection create info from perspective");
    return std::get<CameraOrthoCreateInfo>(projParams);
}


Camera::~Camera()
{
    Terminate();
}


const glm::mat4x4& Camera::GetViewMatrix() const noexcept
{
    ASSERT_CAMERA_INIT_STATUS();
    return m_matWCS;
}


const glm::mat4x4& Camera::GetProjectionMatrix() const noexcept
{
    ASSERT_CAMERA_INIT_STATUS();
    return m_matProjection;
}


float Camera::GetFovDegrees() const noexcept
{
    ASSERT_CAMERA_INIT_STATUS();
    return m_fovDegrees;
}


float Camera::GetAspectRatio() const noexcept
{
    ASSERT_CAMERA_INIT_STATUS();
    return m_aspectRatio;
}


float Camera::GetZNear() const noexcept
{
    ASSERT_CAMERA_INIT_STATUS();
    return m_zNear;
}


float Camera::GetZFar() const noexcept
{
    ASSERT_CAMERA_INIT_STATUS();
    return m_zFar;
}


glm::vec3 Camera::GetXDir() const noexcept
{
    ASSERT_CAMERA_INIT_STATUS();
    return m_matWCS[0];
}


glm::vec3 Camera::GetYDir() const noexcept
{
    ASSERT_CAMERA_INIT_STATUS();
    return m_matWCS[1];
}


glm::vec3 Camera::GetZDir() const noexcept
{
    ASSERT_CAMERA_INIT_STATUS();
    return m_matWCS[2];
}


glm::vec3 Camera::GetPosition() const noexcept
{
    ASSERT_CAMERA_INIT_STATUS();
    return -m_matWCS[3];
}


uint32_t Camera::GetIndex() const noexcept
{
    ASSERT_CAMERA_INIT_STATUS();
    return m_idx;
}


bool Camera::Create(uint32_t index, const CameraCreateInfo& createInfo) noexcept
{
    if (IsInitialized()) {
        ENG_LOG_WARN("Camera {} is already initialized", index);
        return true;
    }

    ENG_ASSERT(CameraManager::IsCameraIdxValid(index), "Invalid camera index: {}", index);

    if (!CreateViewMatrix(createInfo.GetViewParams())) {
        ENG_ASSERT_FAIL("Failed to create camera {} view matrix", index);
        return false;
    }

    bool isProjInitialized = false;

    if (createInfo.IsPerspectiveProj()) {
        isProjInitialized = CreatePerspectiveProj(createInfo.GetPerspectiveProjParams());
    } else if (createInfo.IsOrthoProj()) {
        isProjInitialized = CreateOrthoProj(createInfo.GetOrthoProjParams());
    } else {
        ENG_ASSERT_FAIL("Invalid camera create info type");
    }

    if (!isProjInitialized) {
        ENG_ASSERT_FAIL("Failed to create camera {} projection matrix", index);
        return false;
    }

    m_idx = index;
    m_flags.set(CameraFlagBits::FLAG_IS_INITIALIZED, true);

    return true;
}


bool Camera::CreateViewMatrix(const CameraViewCreateInfo& createInfo) noexcept
{
    m_position = createInfo.position;
    m_rotation = createInfo.rotation;

    RecalcViewMatrix();

    return true;
}


bool Camera::CreatePerspectiveProj(const CameraPerspectiveCreateInfo &createInfo) noexcept
{
    static constexpr float EPS = glm::epsilon<float>();

    if (createInfo.aspectRatio < EPS) {
        ENG_ASSERT_FAIL("Camera create info aspect ratio is zero");
        return false;
    }

    if (glm::abs(createInfo.zFar - createInfo.zNear) < EPS) {
        ENG_ASSERT_FAIL("Camera create info zNear {} is equal to zFar {}", createInfo.zNear, createInfo.zFar);
        return false;
    }

    m_fovDegrees = createInfo.fovDegrees;
    m_aspectRatio = createInfo.aspectRatio;
    m_zNear = createInfo.zNear;
    m_zFar = createInfo.zFar;

    m_flags.set(CameraFlagBits::FLAG_IS_PERSPECTIVE_PROJ);

    RecalcProjMatrix();

    return true;
}


bool Camera::CreateOrthoProj(const CameraOrthoCreateInfo &createInfo) noexcept
{
    static constexpr glm::vec3 vecEPS(glm::epsilon<float>());

    if (glm::all(glm::lessThanEqual(glm::abs(createInfo.lbf - createInfo.rtn), vecEPS))) {
        ENG_ASSERT_FAIL("Camera ortho create info is zero sized box");
        return false;
    }

    m_left = createInfo.lbf.x;
    m_right = createInfo.rtn.x;
    m_top = createInfo.rtn.y;
    m_bottom = createInfo.lbf.y;

    m_zNear = createInfo.rtn.z;
    m_zFar = createInfo.lbf.z;

    m_flags.set(CameraFlagBits::FLAG_IS_PERSPECTIVE_PROJ, false);

    RecalcProjMatrix();

    return true;
}


void Camera::Update(float dt) noexcept
{
    if (IsProjMatrixRecalcRequested()) {
        RecalcProjMatrix();
        ClearProjRecalcRequest();
    }

    if (IsViewMatrixRecalcRequested()) {
        RecalcViewMatrix();
        ClearViewMatrixRecalcRequest();
    }
}


void Camera::RecalcProjMatrix() noexcept
{
    if (IsPerspectiveProj()) {
        m_matProjection = glm::perspective(glm::radians(m_fovDegrees), m_aspectRatio, m_zNear, m_zFar);
    } else if (IsOrthoProj()) {
        m_matProjection = glm::ortho(m_left, m_right, m_bottom, m_top, m_zNear, m_zFar);
    }   
}


void Camera::RecalcViewMatrix() noexcept
{
    m_matWCS = glm::mat4_cast(m_rotation);
    m_matWCS *= glm::translate(glm::identity<glm::mat4x4>(), -m_position);
}


void Camera::Terminate() noexcept
{
    m_matProjection = glm::identity<glm::mat4x4>();
    m_matWCS = glm::identity<glm::mat4x4>();

    m_rotation = glm::identity<glm::quat>();
    m_position = glm::vec3(0.f);

    m_fovDegrees = 0.f;
    m_aspectRatio = 0.f;

    m_left = 0.f;
    m_right = 0.f;
    m_top = 0.f;
    m_bottom = 0.f;

    m_zNear = 0.f;
    m_zFar = 0.f;

    m_flags = {};
    m_idx = UINT16_MAX;
}


CameraManager& CameraManager::GetInstance() noexcept
{
    ASSERT_CAMERA_MNG_INIT_STATUS();
    return *pCameraMngInst;
}


CameraManager::~CameraManager()
{
    Terminate();
}


void CameraManager::Update(float dt) noexcept
{
    for (Camera& cam : m_cameraStorage) {
        cam.Update(dt);
    }
}


void CameraManager::PrepareGPUData(const Camera& cam) noexcept
{
    m_pConstBuffer->BindIndexed(resGetResourceBinding(COMMON_CAMERA_CB).GetBinding());

    COMMON_CAMERA_CB* pConstBuff = m_pConstBuffer->MapWrite<COMMON_CAMERA_CB>();
    ENG_ASSERT_GRAPHICS_API(pConstBuff, "Failed to map camera const buffer");

    const glm::mat4x4& cameraViewMat = cam.GetViewMatrix();
    memcpy_s(&pConstBuff->COMMON_VIEW_MATRIX_00, 16 * sizeof(float), &cameraViewMat, sizeof(cameraViewMat));
    const glm::mat4x4& cameraProjMat = cam.GetProjectionMatrix();
    memcpy_s(&pConstBuff->COMMON_PROJ_MATRIX_00, 16 * sizeof(float), &cameraProjMat, sizeof(cameraProjMat));
    
    m_pConstBuffer->Unmap();
}


Camera &CameraManager::GetCamera(uint32_t idx) noexcept
{
    ASSERT_CAMERA_MNG_INIT_STATUS();
    ENG_ASSERT(idx < MAX_CAM_COUNT, "Invalid camera index: {}", idx);

    return m_cameraStorage[idx];
}


bool CameraManager::Init() noexcept
{
    if (IsInitialized()) {
        return true;
    }

    ENG_ASSERT(engIsWindowSystemInitialized(), "Window system must be initialized before camera manager initialization");
    const Window& mainWindow = engGetMainWindow();

    m_cameraStorage.resize(MAX_CAM_COUNT);
    m_cameraEventListenersStorage.resize(MAX_CAM_COUNT);

    CameraCreateInfo mainCamCreateInfo = {};

    CameraViewCreateInfo mainCamViewCreateInfo = {};
    mainCamViewCreateInfo.position = glm::vec3(0.f, 0.f, 2.f);
    mainCamViewCreateInfo.rotation = glm::quatLookAt(glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 1.f, 0.f));

    mainCamCreateInfo.SetViewParams(mainCamViewCreateInfo);

    CameraPerspectiveCreateInfo projParams = {};
    projParams.aspectRatio = float(mainWindow.GetFramebufferWidth()) / mainWindow.GetFramebufferHeight();
    projParams.fovDegrees = 90.f;
    projParams.zNear = 0.1f;
    projParams.zFar = 100.f;

    mainCamCreateInfo.SetPerspective(projParams);
    
    for (uint32_t i = 0; i < m_cameraStorage.size(); ++i) {
        Camera& cam = m_cameraStorage[i];
        
        if (!cam.Create(i, mainCamCreateInfo)) {
            ENG_ASSERT_FAIL("Failed to create camera: {}");
            return false;
        }
    }

    Camera& mainCam = m_cameraStorage[MAIN_CAM_IDX];
    
    SubscribeCameraToEvent<EventFramebufferResized>(mainCam, [&mainCam](const void* pEvent) {
        const EventFramebufferResized& event = CastEventTo<EventFramebufferResized>(pEvent);
        const uint32_t width = event.GetWidth();
        const uint32_t height = event.GetHeight();

        const float halfWidth = width * 0.5f;
        const float halfHeight = height * 0.5f;
        
        if (width > 0 && height > 0) {
            mainCam.m_aspectRatio = float(width) / height;
            
            mainCam.m_left   = -halfWidth;
            mainCam.m_right  = halfWidth;
            mainCam.m_top    = -halfHeight;
            mainCam.m_bottom = halfHeight;

            mainCam.RequestRecalcProjMatrix();
        }
    }, "_MAIN_CAM_FRAMEBUF_RESIZE_CALLBACK_");

    if (!InitGPUData()) {
        return false;
    }

    m_isInitialized = true;

    return true;
}


void CameraManager::Terminate() noexcept
{
    TerminateGPUData();

    EventDispatcher& dispatcher = EventDispatcher::GetInstance();
    for (const CameraEventListenersStorage& storage : m_cameraEventListenersStorage) {
        for (const CameraEventListenerDesc& desc : storage) {
            dispatcher.Unsubscribe(desc.ID);
        }
    }
    m_cameraEventListenersStorage.clear();
    m_cameraStorage.clear();
    
    m_isInitialized = false;
}


bool CameraManager::InitGPUData() noexcept
{
    if (IsInitialized()) {
        return true;
    }

    MemoryBufferCreateInfo constBufferCreateInfo = {};
    constBufferCreateInfo.type = MemoryBufferType::TYPE_CONSTANT_BUFFER;
    constBufferCreateInfo.dataSize = sizeof(COMMON_CAMERA_CB);
    constBufferCreateInfo.elementSize = sizeof(COMMON_CAMERA_CB);
    constBufferCreateInfo.creationFlags = static_cast<MemoryBufferCreationFlags>(
        BUFFER_CREATION_FLAG_DYNAMIC_STORAGE | BUFFER_CREATION_FLAG_WRITABLE);
    constBufferCreateInfo.pData = nullptr;

    m_pConstBuffer = MemoryBufferManager::GetInstance().RegisterBuffer();
    ENG_ASSERT(m_pConstBuffer, "Failed to register common camera const buffer");
    m_pConstBuffer->Create(constBufferCreateInfo);
    ENG_ASSERT(m_pConstBuffer->IsValid(), "Failed to create common const buffer");
    m_pConstBuffer->SetDebugName("__COMMON_CAMERA_CB__");

    return true;
}


void CameraManager::TerminateGPUData() noexcept
{
    if (!IsInitialized()) {
        return;
    }

    m_pConstBuffer->Destroy();
    MemoryBufferManager::GetInstance().UnregisterBuffer(m_pConstBuffer);
}


Camera& engGetMainCamera() noexcept
{
    ASSERT_CAMERA_MNG_INIT_STATUS();
    return pCameraMngInst->GetCamera(CameraManager::GetMainCameraIdx());
}


bool engInitCameraManager() noexcept
{
    if (engIsCameraManagerInitialized()) {
        ENG_LOG_WARN("Camera manager is already initialized!");
        return true;
    }

    pCameraMngInst = std::unique_ptr<CameraManager>(new CameraManager);

    if (!pCameraMngInst) {
        ENG_ASSERT_FAIL("Failed to allocate memory for camera manager");
        return false;
    }

    if (!pCameraMngInst->Init()) {
        ENG_ASSERT_FAIL("Failed to initialized camera manager");
        return false;
    }

    return true;
}


void engTerminateCameraManager() noexcept
{
    pCameraMngInst = nullptr;
}


bool engIsCameraManagerInitialized() noexcept
{
    return pCameraMngInst && pCameraMngInst->IsInitialized();
}
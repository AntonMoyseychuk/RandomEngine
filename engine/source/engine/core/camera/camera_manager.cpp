#include "pch.h"
#include "camera_manager.h"

#include "engine/window_system/window_system.h"

#include "utils/debug/assertion.h"

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


const glm::mat4x4 &Camera::GetProjectionMatrix() const noexcept
{
    ASSERT_CAMERA_INIT_STATUS();
    return m_matProjection;
}


float Camera::GetFov() const noexcept
{
    ASSERT_CAMERA_INIT_STATUS();
    return m_fov;
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
    if (!glm::isNormalized(createInfo.upDirection, glm::epsilon<float>())) {
        ENG_ASSERT_FAIL("Up vector must be normalized");
        return false;
    }

    m_matWCS = glm::lookAt(createInfo.position, createInfo.lookAtPoint, createInfo.upDirection);

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

    m_fov = createInfo.fov;
    m_aspectRatio = createInfo.aspectRatio;
    m_zNear = createInfo.zNear;
    m_zFar = createInfo.zFar;

    m_flags.set(CameraFlagBits::FLAG_IS_PERSPECTIVE_PROJ);

    RequestRecalcProjMatrix();

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

    RequestRecalcProjMatrix();

    return true;
}


void Camera::Update(float dt) noexcept
{
    RecalcProj();
}


void Camera::RecalcProj() noexcept
{
    if (!IsProjMatrixRecalcRequested()) {
        return;
    }

    if (IsPerspectiveProj()) {
        m_matProjection = glm::perspective(m_fov, m_aspectRatio, m_zNear, m_zFar);
    } else if (IsOrthoProj()) {
        m_matProjection = glm::ortho(m_left, m_right, m_bottom, m_top, m_zNear, m_zFar);
    }

    m_flags.reset(CameraFlagBits::FLAG_NEED_RECALC_PROJ);
}


void Camera::Terminate() noexcept
{
    m_matProjection = glm::identity<glm::mat4x4>();
    m_matWCS = glm::identity<glm::mat4x4>();

    m_fov = 0.f;
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
    mainCamViewCreateInfo.position = glm::vec3(0.f, 0.f, 3.f);
    mainCamViewCreateInfo.lookAtPoint = glm::vec3(0.f);
    mainCamViewCreateInfo.upDirection = glm::vec3(0.f, 1.f, 0.f);

    mainCamCreateInfo.SetViewParams(mainCamViewCreateInfo);

    CameraPerspectiveCreateInfo projParams = {};
    projParams.aspectRatio = float(mainWindow.GetFramebufferWidth()) / mainWindow.GetFramebufferHeight();
    projParams.fov = glm::radians(90.f);
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

    m_isInitialized = true;

    return true;
}


void CameraManager::Terminate() noexcept
{
    m_cameraStorage.clear();

    EventDispatcher& dispatcher = EventDispatcher::GetInstance();
    for (const CameraEventListenersStorage& storage : m_cameraEventListenersStorage) {
        for (const CameraEventListenerDesc& desc : storage) {
            dispatcher.Unsubscribe(desc.ID);
        }
    }
    m_cameraEventListenersStorage.clear();
    
    m_isInitialized = false;
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
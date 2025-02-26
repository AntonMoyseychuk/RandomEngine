#include "pch.h"
#include "camera_manager.h"

#include "engine/window_system/window_system.h"

#include "utils/debug/assertion.h"


static std::unique_ptr<CameraManager> pCameraMngInst = nullptr;


#define ASSERT_CAMERA_MNG_INIT_STATUS() ENG_ASSERT(engIsCameraManagerInitialized(), "Camera manager is not initialized")


Camera::~Camera()
{
    Destroy();
}


void Camera::Destroy() noexcept
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
}


void Camera::SetPerspProjection() noexcept
{
    m_flags.set(FLAG_IS_ORTHO_PROJ, false);
    RequestRecalcProjMatrix();
}


void Camera::SetOrthoProjection() noexcept
{
    m_flags.set(FLAG_IS_ORTHO_PROJ, true);
    RequestRecalcProjMatrix();
}


void Camera::SetFovDegress(float degrees) noexcept
{
    ENG_ASSERT(camIsFovDegreesValid(degrees), "degress can't be multiple of PI or less than zero");

    m_fovDegrees = degrees;
    RequestRecalcProjMatrix();
}


void Camera::SetAspectRatio(float aspect) noexcept
{
    ENG_ASSERT(aspect > glm::epsilon<float>(), "aspect can't be less or equal to zero");

    m_aspectRatio = aspect;
    RequestRecalcProjMatrix();
}


void Camera::SetAspectRatio(uint32_t width, uint32_t height) noexcept
{
    ENG_ASSERT(height != 0, "height can't be equal to zero");

    const float aspectRatio = float(width) / float(height);
    SetAspectRatio(aspectRatio);
}


void Camera::SetZNear(float zNear) noexcept
{
    ENG_ASSERT(abs(m_zFar - zNear) > glm::epsilon<float>(), "Can't set Z Near equal to Z Far");
    m_zNear = zNear;

    RequestRecalcProjMatrix();
}


void Camera::SetZFar(float zFar) noexcept
{
    ENG_ASSERT(abs(zFar - m_zNear) > glm::epsilon<float>(), "Can't set Z Far equal to Z Near");
    m_zFar = zFar;
    
    RequestRecalcProjMatrix();
}


void Camera::SetOrthoLeft(float left) noexcept
{
    ENG_ASSERT(abs(m_right - left) > glm::epsilon<float>(), "Can't set left equal to right");
    
    m_left = left;
    RequestRecalcProjMatrix();
}


void Camera::SetOrthoRight(float right) noexcept
{
    ENG_ASSERT(abs(right - m_left) > glm::epsilon<float>(), "Can't set right equal to left");
    
    m_right = right;
    RequestRecalcProjMatrix();
}


void Camera::SetOrthoTop(float top) noexcept
{
    ENG_ASSERT(abs(top - m_bottom) > glm::epsilon<float>(), "Can't set top equal to bottom");
    
    m_top = top;
    RequestRecalcProjMatrix();
}


void Camera::SetOrthoBottom(float bottom) noexcept
{
    ENG_ASSERT(abs(m_top - bottom) > glm::epsilon<float>(), "Can't set bottom equal to top");
    
    m_bottom = bottom;
    RequestRecalcProjMatrix();
}


void Camera::SetRotation(const glm::quat& rotation) noexcept
{
    m_rotation = rotation;
    RequestRecalcViewMatrix();
}


void Camera::SetPosition(const glm::vec3& position) noexcept
{
    m_position = position;
    RequestRecalcViewMatrix();
}


void Camera::Update(float dt) noexcept
{
    const Input& input = engGetMainWindow().GetInput();
    
    glm::vec3 offset(0.f);

    if (input.IsKeyPressedOrHold(KeyboardKey::KEY_W)) {
        offset -= GetZDir() * dt;
    }
    
    if (input.IsKeyPressedOrHold(KeyboardKey::KEY_S)) {
        offset += GetZDir() * dt;
    }

    if (input.IsKeyPressedOrHold(KeyboardKey::KEY_D)) {
        offset += GetXDir() * dt;
    }
    
    if (input.IsKeyPressedOrHold(KeyboardKey::KEY_A)) {
        offset -= GetXDir() * dt;
    }

    if (input.IsKeyPressedOrHold(KeyboardKey::KEY_E)) {
        offset += GetYDir() * dt;
    }
    
    if (input.IsKeyPressedOrHold(KeyboardKey::KEY_Q)) {
        offset -= GetYDir() * dt;
    }

    if (!glm::isNull(offset, glm::epsilon<float>())) {
        m_position += glm::normalize(offset) * dt;
        RequestRecalcViewMatrix();
    }

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
    if (IsPerspProj()) {
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


CameraManager& CameraManager::GetInstance() noexcept
{
    ASSERT_CAMERA_MNG_INIT_STATUS();
    return *pCameraMngInst;
}


CameraManager::~CameraManager()
{
    Terminate();
}


Camera* CameraManager::RegisterCamera() noexcept
{
    const CameraID camID = AllocateCameraID();
    Camera* pCam = &m_camerasStorage[camID.Value()];

    ENG_ASSERT(!pCam->IsRegistered(), "Already registered camera was returned during registration");
    
    pCam->m_ID = camID;

    return pCam;
}


void CameraManager::UnregisterCamera(Camera* pCam) noexcept
{
    if (!pCam) {
        return;
    }

    pCam->Destroy();
    DeallocateCameraID(pCam->m_ID);

    pCam->m_ID.Invalidate();
}


void CameraManager::Update(float dt) noexcept
{
    for (Camera& cam : m_camerasStorage) {
        cam.Update(dt);
    }
}


bool CameraManager::Init() noexcept
{
    if (IsInitialized()) {
        return true;
    }

    m_camerasStorage.resize(MAX_CAM_COUNT);
    m_cameraEventListenersStorage.resize(MAX_CAM_COUNT);

    m_nextAllocatedID = CameraID(0);

    m_isInitialized = true;

    return true;
}


void CameraManager::Terminate() noexcept
{
    EventDispatcher& dispatcher = EventDispatcher::GetInstance();
    for (const CameraEventListenersStorage& storage : m_cameraEventListenersStorage) {
        for (const CameraEventListenerDesc& desc : storage) {
            dispatcher.Unsubscribe(desc.ID);
        }
    }
    m_cameraEventListenersStorage.clear();
    m_camerasStorage.clear();

    m_cameraIDFreeList.clear();
    m_nextAllocatedID = CameraID(0);
    
    m_isInitialized = false;
}


CameraID CameraManager::AllocateCameraID() noexcept
{
    if (m_cameraIDFreeList.empty()) {
        ENG_ASSERT(m_nextAllocatedID.Value() < m_camerasStorage.size() - 1, "Memory buffer storage overflow");

        const CameraID camID = m_nextAllocatedID;
        m_nextAllocatedID = CameraID(m_nextAllocatedID.Value() + 1);

        return camID;
    }

    const CameraID camID = m_cameraIDFreeList.front();
    m_cameraIDFreeList.pop_front();
        
    return camID;
}


void CameraManager::DeallocateCameraID(CameraID ID) noexcept
{
    if (ID < m_nextAllocatedID && std::find(m_cameraIDFreeList.cbegin(), m_cameraIDFreeList.cend(), ID) == m_cameraIDFreeList.cend()) {
        m_cameraIDFreeList.emplace_back(ID);
    }
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
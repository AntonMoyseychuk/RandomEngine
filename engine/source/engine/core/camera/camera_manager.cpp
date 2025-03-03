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
    m_matViewProjection = M3D_MAT4_IDENTITY;
    m_matProjection     = M3D_MAT4_IDENTITY;
    m_matWCS            = M3D_MAT4_IDENTITY;
    
    m_rotation = M3D_QUAT_IDENTITY;
    m_position = M3D_ZEROF3;
    
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
    if (!IsPerspProj()) {
        m_flags.set(FLAG_IS_ORTHO_PROJ, false);
        RequestRecalcProjMatrix();
    }
}


void Camera::SetOrthoProjection() noexcept
{
    if (!IsOrthoProj()) {
        m_flags.set(FLAG_IS_ORTHO_PROJ, true);
        RequestRecalcProjMatrix();
    }
}


void Camera::SetFovDegress(float degrees) noexcept
{
    if (!amAreEqual(m_fovDegrees, degrees)) {
        ENG_ASSERT(camIsFovDegreesValid(degrees), "degress can't be multiple of PI or less than zero");

        m_fovDegrees = degrees;
        RequestRecalcProjMatrix();
    }
}


void Camera::SetAspectRatio(float aspect) noexcept
{
    if (!amAreEqual(m_aspectRatio, aspect)) {
        ENG_ASSERT(aspect > M3D_EPS, "aspect can't be less or equal to zero");

        m_aspectRatio = aspect;
        RequestRecalcProjMatrix();
    }
}


void Camera::SetAspectRatio(uint32_t width, uint32_t height) noexcept
{
    ENG_ASSERT(height != 0, "height can't be equal to zero");

    const float aspectRatio = float(width) / float(height);
    SetAspectRatio(aspectRatio);
}


void Camera::SetZNear(float zNear) noexcept
{
    if (!amAreEqual(m_zNear, zNear)) {
        ENG_ASSERT(abs(m_zFar - zNear) > M3D_EPS, "Can't set Z Near equal to Z Far");
    
        m_zNear = zNear;
        RequestRecalcProjMatrix();
    }
}


void Camera::SetZFar(float zFar) noexcept
{
    if (!amAreEqual(m_zFar, zFar)) {
        ENG_ASSERT(abs(zFar - m_zNear) > M3D_EPS, "Can't set Z Far equal to Z Near");
    
        m_zFar = zFar;
        RequestRecalcProjMatrix();
    }
}


void Camera::SetOrthoLeft(float left) noexcept
{
    if (!amAreEqual(m_left, left)) {
        ENG_ASSERT(abs(m_right - left) > M3D_EPS, "Can't set left equal to right");
    
        m_left = left;
        RequestRecalcProjMatrix();
    }
}


void Camera::SetOrthoRight(float right) noexcept
{
    if (!amAreEqual(m_right, right)) {
        ENG_ASSERT(abs(right - m_left) > M3D_EPS, "Can't set right equal to left");
    
        m_right = right;
        RequestRecalcProjMatrix();
    }
}


void Camera::SetOrthoTop(float top) noexcept
{
    if (!amAreEqual(m_top, top)) {
        ENG_ASSERT(abs(top - m_bottom) > M3D_EPS, "Can't set top equal to bottom");
    
        m_top = top;
        RequestRecalcProjMatrix();
    }
}


void Camera::SetOrthoBottom(float bottom) noexcept
{
    if (!amAreEqual(m_bottom, bottom)) {
        ENG_ASSERT(abs(m_top - bottom) > M3D_EPS, "Can't set bottom equal to top");
    
        m_bottom = bottom;
        RequestRecalcProjMatrix();
    }
}


void Camera::Move(const glm::vec3& offset) noexcept
{
    if (!amIsZero(offset)) {
        m_position += offset;
        RequestRecalcViewMatrix();
    }
}


void Camera::MoveAlongDir(const glm::vec3& dir, float distance) noexcept
{
    if (!amIsZero(distance)) {
        ENG_ASSERT(amIsNormalized(dir), "dir must be normalized vector");
    
        m_position += dir * distance;
        RequestRecalcViewMatrix();
    }
}


void Camera::Rotate(const glm::quat& rotation) noexcept
{
    if (!amAreEqual(rotation, M3D_QUAT_IDENTITY)) {
        ENG_ASSERT(amIsNormalized(rotation), "rotation quaternion must be normalized");

        m_rotation = glm::normalize(rotation * m_rotation);
        RequestRecalcViewMatrix();
    }
}


void Camera::RotateAxis(const glm::vec3 &axis, float degrees) noexcept
{
    if (!amIsZero(degrees)) {
        m_rotation = glm::normalize(glm::angleAxis(glm::radians(degrees), axis) * m_rotation);
        RequestRecalcViewMatrix();
    }
}


void Camera::RotatePitchYawRoll(float pitchDegrees, float yawDegrees, float rollDegrees) noexcept
{
    if (!amIsZero(glm::vec3(pitchDegrees, yawDegrees, rollDegrees))) {
        const glm::quat rotPitch = glm::angleAxis(glm::radians(pitchDegrees), M3D_AXIS_X);
        const glm::quat rotYaw   = glm::angleAxis(glm::radians(yawDegrees),   M3D_AXIS_Y);
        const glm::quat rotRoll  = glm::angleAxis(glm::radians(rollDegrees),  M3D_AXIS_Z);

        m_rotation = glm::normalize(rotRoll * rotYaw * rotPitch * m_rotation);
        RequestRecalcViewMatrix();
    }
}


void Camera::SetRotation(const glm::quat& rotation) noexcept
{
    if (!amAreEqual(m_rotation, rotation)) {
        m_rotation = rotation;
        RequestRecalcViewMatrix();
    }
}


void Camera::SetPosition(const glm::vec3& position) noexcept
{
    if (!amAreEqual(m_position, position)) {
        m_position = position;
        RequestRecalcViewMatrix();
    }
}


void Camera::Update(float dt) noexcept
{
    bool shouldRecalcViewProjMat = false;

    if (IsViewMatrixRecalcRequested()) {
        RecalcViewMatrix();
        ClearViewMatrixRecalcRequest();
        shouldRecalcViewProjMat = true;
    }

    if (IsProjMatrixRecalcRequested()) {
        RecalcProjMatrix();
        ClearProjRecalcRequest();
        shouldRecalcViewProjMat = true;
    }

    if (shouldRecalcViewProjMat) {
        RecalcViewProjMatrix();
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
    m_matWCS = glm::mat4_cast(m_rotation) * glm::translate(M3D_MAT4_IDENTITY, -m_position);
}


void Camera::RecalcViewProjMatrix() noexcept
{
    m_matViewProjection = m_matProjection * m_matWCS;
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
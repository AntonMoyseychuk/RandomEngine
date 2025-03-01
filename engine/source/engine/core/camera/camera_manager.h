#pragma once

#include "core/event_system/event_dispatcher.h"
#include "engine/window_system/window_system_events.h"

#include "utils/data_structures/strid.h"
#include "utils/data_structures/base_id.h"

#include "utils/math/glm_math.h"

#include <vector>
#include <deque>
#include <bitset>


using CameraID = ds::BaseID<uint16_t>;


class Camera
{
    friend class CameraManager;

public:
    Camera() = default;
    ~Camera();

    void Destroy() noexcept;

    void SetPerspProjection() noexcept;
    void SetOrthoProjection() noexcept;

    void SetFovDegress(float degrees) noexcept;
    void SetAspectRatio(float aspect) noexcept;
    void SetAspectRatio(uint32_t width, uint32_t height) noexcept;
    void SetZNear(float zNear) noexcept;
    void SetZFar(float zFar) noexcept;

    void SetOrthoLeft(float left) noexcept;
    void SetOrthoRight(float right) noexcept;
    void SetOrthoTop(float top) noexcept;
    void SetOrthoBottom(float bottom) noexcept;

    void Move(const glm::vec3& offset) noexcept;
    void MoveAlongDir(const glm::vec3& dir, float distance) noexcept;

    void Rotate(const glm::vec3& axis, float degrees) noexcept;
    void Rotate(float pitchDegrees, float yawDegrees, float rollDegrees = 0.f) noexcept;

    void SetRotation(const glm::quat& rotation) noexcept;
    void SetPosition(const glm::vec3& position) noexcept;

    float GetFovDegrees() const noexcept { return m_fovDegrees; }
    float GetAspectRatio() const noexcept { return m_aspectRatio; }
    float GetZNear() const noexcept { return m_zNear; }
    float GetZFar() const noexcept { return m_zFar; }

    float GetOrthoLeft() const noexcept { return m_left; }
    float GetOrthoRight() const noexcept { return m_right; }
    float GetOrthoTop() const noexcept { return m_top; }
    float GetOrthoBottom() const noexcept { return m_bottom; }

    glm::vec3 GetXDir() const noexcept { return glm::vec3(m_matWCS[0].x, m_matWCS[1].x, m_matWCS[2].x); }
    glm::vec3 GetYDir() const noexcept { return glm::vec3(m_matWCS[0].y, m_matWCS[1].y, m_matWCS[2].y); }
    glm::vec3 GetZDir() const noexcept { return glm::vec3(m_matWCS[0].z, m_matWCS[1].z, m_matWCS[2].z); }
    glm::vec3 GetPosition() const noexcept { return -m_matWCS[3]; }

    CameraID GetID() const noexcept { return m_ID; }

    const glm::mat4x4& GetViewMatrix() const noexcept { return m_matWCS; }
    const glm::mat4x4& GetProjectionMatrix() const noexcept { return m_matProjection; }
    const glm::mat4x4& GetViewProjectionMatrix() const noexcept { return m_matViewProjection; }

    bool IsRegistered() const noexcept { return m_ID.IsValid(); }
    bool IsPerspProj() const noexcept { return !IsOrthoProj(); }
    bool IsOrthoProj() const noexcept { return m_flags.test(CameraFlagBits::FLAG_IS_ORTHO_PROJ); }

    bool IsProjMatrixRecalcRequested() const noexcept { return m_flags.test(CameraFlagBits::FLAG_NEED_RECALC_PROJ_MAT); }
    bool IsViewMatrixRecalcRequested() const noexcept { return m_flags.test(CameraFlagBits::FLAG_NEED_RECALC_VIEW_MAT); }
    
    bool IsNeedRecalcViewProjMatrix() const noexcept { return IsViewMatrixRecalcRequested() || IsProjMatrixRecalcRequested(); }

    void RequestRecalcProjMatrix() noexcept { m_flags.set(CameraFlagBits::FLAG_NEED_RECALC_PROJ_MAT); }
    void RequestRecalcViewMatrix() noexcept { m_flags.set(CameraFlagBits::FLAG_NEED_RECALC_VIEW_MAT); }

private:
    void Update(float dt) noexcept;

    void ClearProjRecalcRequest() noexcept { m_flags.reset(CameraFlagBits::FLAG_NEED_RECALC_PROJ_MAT); }
    void ClearViewMatrixRecalcRequest() noexcept { m_flags.reset(CameraFlagBits::FLAG_NEED_RECALC_VIEW_MAT); }

    void RecalcProjMatrix() noexcept;
    void RecalcViewMatrix() noexcept;
    void RecalcViewProjMatrix() noexcept;

private:
    enum CameraFlagBits
    {
        FLAG_IS_ORTHO_PROJ,
        FLAG_NEED_RECALC_PROJ_MAT,
        FLAG_NEED_RECALC_VIEW_MAT,

        FLAG_COUNT,
    };

    using CameraFlags = std::bitset<16>;
    static_assert(CameraFlagBits::FLAG_COUNT < CameraFlags().size());

    glm::mat4x4 m_matViewProjection = M3D_MAT4_IDENTITY;
    glm::mat4x4 m_matProjection     = M3D_MAT4_IDENTITY;
    glm::mat4x4 m_matWCS            = M3D_MAT4_IDENTITY;

    glm::quat m_rotation = M3D_QUAT_IDENTITY;
    glm::vec3 m_position = M3D_ZEROF3;

    // perspective
    float m_fovDegrees = 0.f;
    float m_aspectRatio = 0.f;

    // ortho
    float m_left = 0.f;
    float m_right = 0.f;
    float m_top = 0.f;
    float m_bottom = 0.f;

    float m_zNear = 0.f;
    float m_zFar = 0.f;

    CameraFlags m_flags = {};
    CameraID m_ID;
};


class CameraManager
{
    friend bool engInitCameraManager() noexcept;
    friend void engTerminateCameraManager() noexcept;
    friend bool engIsCameraManagerInitialized() noexcept;

public:
    static CameraManager& GetInstance() noexcept;

    static constexpr uint32_t GetMaxCamerasCount() noexcept { return MAX_CAM_COUNT; }
    static constexpr uint32_t GetMaxCameraEventListenersCount() noexcept { return MAX_CAM_EVENT_LISTENERS_COUNT; }

public:
    CameraManager(const CameraManager& other) = delete;
    CameraManager& operator=(const CameraManager& other) = delete;
    CameraManager(CameraManager&& other) noexcept = delete;
    CameraManager& operator=(CameraManager&& other) noexcept = delete;

    ~CameraManager();

    Camera* RegisterCamera() noexcept;
    void UnregisterCamera(Camera* pCam) noexcept;

    void Update(float dt) noexcept;

    template <typename EventType>
    void SubscribeCamera(const Camera& cam, const EventListener::CallbackType& callback) noexcept;

    template <typename EventType>
    void UnsubscribeCamera(const Camera& cam) noexcept;

    template <typename EventType>
    bool IsCameraSubscribed(const Camera& cam) const noexcept;

    bool IsInitialized() const noexcept { return m_isInitialized; }

private:
    CameraManager() = default;

    bool Init() noexcept;
    void Terminate() noexcept;

    CameraID AllocateCameraID() noexcept;
    void DeallocateCameraID(CameraID ID) noexcept;

    template <typename EventType>
    uint32_t GetCameraEventListenerIndex(const Camera& cam) const noexcept;

private:
    static inline constexpr uint32_t MAX_CAM_EVENT_LISTENERS_COUNT = 8;
    static inline constexpr uint32_t MAX_CAM_COUNT = 8;

private:
    struct CameraEventListenerDesc
    {
        EventListenerID ID;
        uint64_t eventTypeHash = UINT64_MAX;
    };

    using CameraEventListenersStorage = std::array<CameraEventListenerDesc, MAX_CAM_EVENT_LISTENERS_COUNT>;

    std::vector<Camera> m_camerasStorage;
    std::vector<CameraEventListenersStorage> m_cameraEventListenersStorage;

    std::deque<CameraID> m_cameraIDFreeList;
    CameraID m_nextAllocatedID = CameraID{0};
    
    bool m_isInitialized = false;
};


constexpr inline bool camIsFovDegreesValid(float degrees) noexcept
{
    return degrees > M3D_EPS && degrees < 180.f;
}


bool engInitCameraManager() noexcept;
void engTerminateCameraManager() noexcept;
bool engIsCameraManagerInitialized() noexcept;


#include "camera_manager.hpp"
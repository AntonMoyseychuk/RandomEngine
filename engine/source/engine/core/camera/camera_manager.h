#pragma once

#include "core/event_system/event_dispatcher.h"
#include "engine/window_system/window_system_events.h"

#include "utils/data_structures/strid.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <vector>
#include <array>
#include <bitset>
#include <variant>
#include <optional>


struct CameraViewCreateInfo
{
    glm::quat rotation;
    glm::vec3 position;
};


struct CameraPerspectiveCreateInfo
{
    float fovDegrees;
    float aspectRatio;
    float zNear;
    float zFar;
};


struct CameraOrthoCreateInfo
{
    glm::vec3 lbf; // left, bottom, far
    glm::vec3 rtn; // right, top, near
};


class CameraCreateInfo
{
public:
    void SetPerspective(const CameraPerspectiveCreateInfo& perspCreateInfo) noexcept { projParams = perspCreateInfo; }
    void SetOrtho(const CameraOrthoCreateInfo& orthoCreateInfo) noexcept { projParams = orthoCreateInfo; }
    void SetViewParams(const CameraViewCreateInfo& viewCreateInfo) noexcept { viewParams = viewCreateInfo; }

    constexpr bool IsPerspectiveProj() const noexcept { return std::holds_alternative<CameraPerspectiveCreateInfo>(projParams); }
    constexpr bool IsOrthoProj() const noexcept { return std::holds_alternative<CameraOrthoCreateInfo>(projParams); }

    const CameraPerspectiveCreateInfo& GetPerspectiveProjParams() const noexcept;
    const CameraOrthoCreateInfo& GetOrthoProjParams() const noexcept;

    const CameraViewCreateInfo& GetViewParams() const noexcept { return viewParams; }

private:
    using CameraProjParamsType = std::variant<CameraPerspectiveCreateInfo, CameraOrthoCreateInfo>;

    CameraViewCreateInfo viewParams;
    CameraProjParamsType projParams;
};


class Camera
{
    friend class CameraManager;

public:
    Camera() = default;
    ~Camera();

    const glm::mat4x4& GetViewMatrix() const noexcept;
    const glm::mat4x4& GetProjectionMatrix() const noexcept;

    float GetFovDegrees() const noexcept;
    float GetAspectRatio() const noexcept;
    float GetZNear() const noexcept;
    float GetZFar() const noexcept;

    glm::vec3 GetXDir() const noexcept;
    glm::vec3 GetYDir() const noexcept;
    glm::vec3 GetZDir() const noexcept;
    glm::vec3 GetPosition() const noexcept;

    uint32_t GetIndex() const noexcept;

    bool IsInitialized() const noexcept { return m_flags.test(CameraFlagBits::FLAG_IS_INITIALIZED); }
    bool IsPerspectiveProj() const noexcept { return m_flags.test(CameraFlagBits::FLAG_IS_PERSPECTIVE_PROJ); }
    bool IsOrthoProj() const noexcept { return !IsPerspectiveProj(); }

    bool IsProjMatrixRecalcRequested() const noexcept { return m_flags.test(CameraFlagBits::FLAG_NEED_RECALC_PROJ_MAT); }
    bool IsViewMatrixRecalcRequested() const noexcept { return m_flags.test(CameraFlagBits::FLAG_NEED_RECALC_VIEW_MAT); }

    void RequestRecalcProjMatrix() noexcept { m_flags.set(CameraFlagBits::FLAG_NEED_RECALC_PROJ_MAT); }
    void RequestRecalcViewMatrix() noexcept { m_flags.set(CameraFlagBits::FLAG_NEED_RECALC_VIEW_MAT); }

private:
    bool Create(uint32_t index, const CameraCreateInfo& createInfo) noexcept;
    void Destroy() noexcept;

    bool CreateViewMatrix(const CameraViewCreateInfo& createInfo) noexcept;

    bool CreatePerspectiveProj(const CameraPerspectiveCreateInfo& createInfo) noexcept;
    bool CreateOrthoProj(const CameraOrthoCreateInfo& createInfo) noexcept;

    void Update(float dt) noexcept;

    void ClearProjRecalcRequest() noexcept { m_flags.reset(CameraFlagBits::FLAG_NEED_RECALC_PROJ_MAT); }
    void ClearViewMatrixRecalcRequest() noexcept { m_flags.reset(CameraFlagBits::FLAG_NEED_RECALC_VIEW_MAT); }

    void RecalcProjMatrix() noexcept;
    void RecalcViewMatrix() noexcept;

private:
    enum CameraFlagBits
    {
        FLAG_IS_INITIALIZED,
        FLAG_IS_PERSPECTIVE_PROJ,
        FLAG_NEED_RECALC_PROJ_MAT,
        FLAG_NEED_RECALC_VIEW_MAT,

        FLAG_COUNT,
    };

    using CameraFlags = std::bitset<16>;
    static_assert(CameraFlagBits::FLAG_COUNT < CameraFlags().size());

    glm::mat4x4 m_matProjection = glm::identity<glm::mat4x4>();
    glm::mat4x4 m_matWCS = glm::identity<glm::mat4x4>();

    glm::quat m_rotation = glm::identity<glm::quat>();
    glm::vec3 m_position = glm::vec3(0.f);

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
    uint16_t m_idx = UINT16_MAX;
};


class CameraManager
{
    friend bool engInitCameraManager() noexcept;
    friend void engTerminateCameraManager() noexcept;
    friend bool engIsCameraManagerInitialized() noexcept;

public:
    static CameraManager& GetInstance() noexcept;

    static constexpr bool IsCameraIdxValid(uint32_t index) noexcept { return index < MAX_CAM_COUNT; }
    static constexpr uint32_t GetMainCameraIdx() noexcept { return MAIN_CAM_IDX; }
    static constexpr uint32_t GetMaxCamerasCount() noexcept { return MAX_CAM_COUNT; }
    static constexpr uint32_t GetMaxCameraEventListenersCount() noexcept { return MAX_CAM_EVENT_LISTENERS_COUNT; }

public:
    CameraManager(const CameraManager& other) = delete;
    CameraManager& operator=(const CameraManager& other) = delete;
    CameraManager(CameraManager&& other) noexcept = delete;
    CameraManager& operator=(CameraManager&& other) noexcept = delete;

    ~CameraManager();

    void Update(float dt) noexcept;

    Camera& GetCamera(uint32_t idx) noexcept;

    bool IsInitialized() const noexcept { return m_isInitialized; }

    template <typename EventType>
    void SubscribeCamera(const Camera& cam, const EventListener::CallbackType& callback, ds::StrID debugName = "") noexcept;

    template <typename EventType>
    void UnsubscribeCamera(const Camera& cam) noexcept;

    template <typename EventType>
    bool IsCameraSubscribed(const Camera& cam) const noexcept;

private:
    CameraManager() = default;

    bool Init() noexcept;
    void Terminate() noexcept;

    template <typename EventType>
    uint32_t GetCameraEventListenerIndex(const Camera& cam) const noexcept;

private:
    static inline constexpr uint32_t MAIN_CAM_IDX = 0;
    static inline constexpr uint32_t MAX_CAM_COUNT = MAIN_CAM_IDX + 1;
    static inline constexpr uint32_t MAX_CAM_EVENT_LISTENERS_COUNT = 8;

private:
    struct CameraEventListenerDesc
    {
        EventListenerID ID;
        uint64_t eventTypeHash = UINT64_MAX;
    };

    using CameraEventListenersStorage = std::array<CameraEventListenerDesc, MAX_CAM_EVENT_LISTENERS_COUNT>;

    std::vector<Camera> m_cameraStorage;
    std::vector<CameraEventListenersStorage> m_cameraEventListenersStorage;
    
    bool m_isInitialized = false;
};


Camera& engGetMainCamera() noexcept;


bool engInitCameraManager() noexcept;
void engTerminateCameraManager() noexcept;
bool engIsCameraManagerInitialized() noexcept;


#include "camera_manager.hpp"
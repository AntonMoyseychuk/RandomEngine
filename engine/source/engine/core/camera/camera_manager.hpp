template <typename EventType>
inline void CameraManager::SubscribeCamera(const Camera &cam, const es::ListenerCallback& callback) noexcept
{
    if (IsCameraSubscribed<EventType>(cam)) {
        return;
    }

    es::EventDispatcher& dispatcher = es::EventDispatcher::GetInstance();

    CameraEventListenersStorage& listenersStorage = m_cameraEventListenersStorage[cam.GetID().Value()];

    for (es::ListenerID& ID : listenersStorage) {
        if (!ID.IsValid()) {
            ID = dispatcher.Subscribe<EventType>(callback);
            return;
        }
    }

    ENG_ASSERT_FAIL("Failed to subscribe camera");
}


template <typename EventType>
inline void CameraManager::UnsubscribeCamera(const Camera &cam) noexcept
{
    const uint32_t cameraEventListenerIndex = GetCameraEventListenerIndex<EventType>(cam);
    if (cameraEventListenerIndex >= MAX_CAM_EVENT_LISTENERS_COUNT) {
        return;
    }

    es::ListenerID& ID = m_cameraEventListenersStorage[cam.GetIndex()][cameraEventListenerIndex];

    Window::EventDispatcher* pDispatcher = WindowSystem::GetInstance().GetWindowByTag(WINDOW_TAG_MAIN)->GetEventDispatcher();
    pDispatcher->Unsubscribe(desc.ID);
}


template <typename EventType>
inline bool CameraManager::IsCameraSubscribed(const Camera &cam) const noexcept
{
    return GetCameraEventListenerIndex<EventType>(cam) != MAX_CAM_EVENT_LISTENERS_COUNT;
}


template <typename EventType>
inline uint32_t CameraManager::GetCameraEventListenerIndex(const Camera &cam) const noexcept
{
    if (!cam.IsRegistered()) {
        return MAX_CAM_EVENT_LISTENERS_COUNT;
    }

    const CameraEventListenersStorage& storage = m_cameraEventListenersStorage[cam.GetID().Value()];

    const auto eventTypeIdx = es::EventDispatcher::GetEventTypeIndex<EventType>();

    const auto listenerIt = std::find_if(storage.cbegin(), storage.cend(), [eventTypeIdx](const es::ListenerID& ID) {
        return ID.GetEventTypeIndex() == eventTypeIdx;
    });

    return listenerIt != storage.cend() ? std::distance(storage.cbegin(), listenerIt) : MAX_CAM_EVENT_LISTENERS_COUNT;
}
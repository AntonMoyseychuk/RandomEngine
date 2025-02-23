template <typename EventType>
inline void CameraManager::SubscribeCamera(const Camera &cam, const EventListener::CallbackType& callback, ds::StrID debugName) noexcept
{
    if (IsCameraSubscribed<EventType>(cam)) {
        return;
    }

    CameraEventListenersStorage& listenersStorage = m_cameraEventListenersStorage[cam.GetIndex()];
    CameraEventListenerDesc* pDesc = nullptr;

    for (CameraEventListenerDesc& desc : listenersStorage) {
        if (!desc.ID.IsValid()) {
            pDesc = &desc;
            break;
        }
    }

    if (pDesc) {
        EventDispatcher& dispatcher = EventDispatcher::GetInstance();

        pDesc->eventTypeHash = Event<EventType>::GetTypeHash();
        pDesc->ID = dispatcher.Subscribe<EventType>(callback);
        dispatcher.SetListenerDebugName(pDesc->ID, debugName);
    }
}


template <typename EventType>
inline void CameraManager::UnsubscribeCamera(const Camera &cam) noexcept
{
    const uint32_t cameraEventListenerIndex = GetCameraEventListenerIndex<EventType>(cam);
    if (cameraEventListenerIndex >= MAX_CAM_EVENT_LISTENERS_COUNT) {
        return;
    }

    CameraEventListenerDesc& desc = m_cameraEventListenersStorage[cam.GetIndex()][cameraEventListenerIndex];

    EventDispatcher::GetInstance().Unsubscribe(desc.ID);

    desc.ID.Invalidate();
    desc.eventTypeHash = UINT64_MAX;
}


template <typename EventType>
inline bool CameraManager::IsCameraSubscribed(const Camera &cam) const noexcept
{
    return GetCameraEventListenerIndex<EventType>(cam) != MAX_CAM_EVENT_LISTENERS_COUNT;
}


template <typename EventType>
inline uint32_t CameraManager::GetCameraEventListenerIndex(const Camera &cam) const noexcept
{
    if (!cam.IsInitialized()) {
        return MAX_CAM_EVENT_LISTENERS_COUNT;
    }

    const CameraEventListenersStorage& listenersStorage = m_cameraEventListenersStorage[cam.GetIndex()];
    
    const uint64_t eventTypeHash = Event<EventType>::GetTypeHash();
    
    const auto listenerIt = std::find_if(listenersStorage.cbegin(), listenersStorage.cend(), [eventTypeHash](const CameraEventListenerDesc& desc) {
        return desc.eventTypeHash == eventTypeHash;
    });

    return listenerIt != listenersStorage.cend() ? std::distance(listenersStorage.cbegin(), listenerIt) : MAX_CAM_EVENT_LISTENERS_COUNT;
}
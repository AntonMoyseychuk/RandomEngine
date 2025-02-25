template <typename EventType>
inline void CameraManager::SubscribeCamera(const Camera &cam, const EventListener::CallbackType& callback) noexcept
{
    if (IsCameraSubscribed<EventType>(cam)) {
        return;
    }

    CameraEventListenersStorage& listenersStorage = m_cameraEventListenersStorage[cam.GetID().Value()];
    CameraEventListenerDesc* pDesc = nullptr;

    for (CameraEventListenerDesc& desc : listenersStorage) {
        if (!desc.ID.IsValid()) {
            pDesc = &desc;
            break;
        }
    }

    if (pDesc) {
        EventDispatcher& dispatcher = EventDispatcher::GetInstance();

        const type_info& typeID = Event<EventType>::GetTypeID();
        
        pDesc->eventTypeHash = typeID.hash_code();
        pDesc->ID = dispatcher.Subscribe<EventType>(callback);
    #if defined(ENG_DEBUG)
        char debugName[512];
        sprintf_s(debugName, "_cam_%d_%s_callback_", cam.GetID().Value(), typeID.name());

        dispatcher.SetListenerDebugName(pDesc->ID, debugName);
    #endif
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
    if (!cam.IsRegistered()) {
        return MAX_CAM_EVENT_LISTENERS_COUNT;
    }

    const CameraEventListenersStorage& listenersStorage = m_cameraEventListenersStorage[cam.GetID().Value()];
    
    const uint64_t eventTypeHash = Event<EventType>::GetTypeID().hash_code();
    
    const auto listenerIt = std::find_if(listenersStorage.cbegin(), listenersStorage.cend(), [eventTypeHash](const CameraEventListenerDesc& desc) {
        return desc.eventTypeHash == eventTypeHash;
    });

    return listenerIt != listenersStorage.cend() ? std::distance(listenersStorage.cbegin(), listenerIt) : MAX_CAM_EVENT_LISTENERS_COUNT;
}
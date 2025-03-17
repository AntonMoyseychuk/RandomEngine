namespace es
{
    inline uint64_t EventDispatcher::AllocateEventTypeIndex() noexcept
    {
        static uint64_t index = 0;
        return index++;
    }


    template <typename EventType>
    inline uint64_t EventDispatcher::GetEventTypeIndex() noexcept
    {
        static const uint64_t index = AllocateEventTypeIndex();
        return index;
    }


    template <typename EventType>
    inline ListenerID EventDispatcher::Subscribe(const ListenerCallback& listenerCallback) noexcept
    {
        static const auto eventTypeIndex = GetEventTypeIndex<EventType>();
        ENG_ASSERT(eventTypeIndex < m_storages.size(), "Event dispatcher available event types count overflow");

        ListenersStorage& listenersStorage = m_storages[eventTypeIndex];
        const auto callbackStorageIndex = listenersStorage.Add(listenerCallback);

        return ListenerID(eventTypeIndex, callbackStorageIndex);
    }


    template <typename EventType, typename... Args>
    inline void EventDispatcher::Notify(Args&&... args) noexcept
    {
        static const auto eventTypeIndex = GetEventTypeIndex<EventType>();
        ENG_ASSERT(eventTypeIndex < m_storages.size(), "Event dispatcher available event types count overflow");

        const EventType event(std::forward<Args>(args)...);
        m_storages[eventTypeIndex].Notify(&event);
    }
}
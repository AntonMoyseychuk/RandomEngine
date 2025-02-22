template <typename EventType>
EventListenerID EventDispatcher::Subscribe(const EventListener::CallbackType& eventCallback) noexcept
{
    const std::type_index eventTypeIndex = Event<EventType>::GetTypeID();
    EventListenerID listenerID(eventTypeIndex);
    
    std::vector<EventListener>& listenersCollection = m_listenersMap[listenerID.TypeIndexHash()];

    if (listenersCollection.capacity() == 0) {
        static constexpr size_t LISTENERS_RESERVE_COUNT = 64ull;
        listenersCollection.reserve(LISTENERS_RESERVE_COUNT);
    }
    
    for (uint64_t i = 0; i < listenersCollection.size(); ++i) {
        if (!listenersCollection[i].IsValid()) {
            listenerID.SetValue(i);
            break;
        }
    }
    
    if (listenerID.IsValid()) {
        listenersCollection[listenerID.Value()] = EventListener(listenerID, eventCallback);
    } else {
        listenerID.SetValue(listenersCollection.size());
        listenersCollection.emplace_back(listenerID, eventCallback);
    }

    return listenerID;
}


template <typename EventType, typename... Args>
inline void EventDispatcher::Notify(Args &&...args) noexcept
{
    auto listenersCollectionIt = m_listenersMap.find(Event<EventType>::GetTypeID().hash_code());
        
    if (listenersCollectionIt != m_listenersMap.cend()) {
        EventType event(std::forward<Args>(args)...);
        
        for (EventListener& listener : listenersCollectionIt->second) {
            listener.Excecute(&event);
        }
    }
}
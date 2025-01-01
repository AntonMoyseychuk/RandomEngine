
template <typename EventType, typename... Args>
inline void EventDispatcher::Notify(Args &&...args) noexcept
{
    auto listenersCollectionIt = m_listenersMap.find(Event<EventType>::GetTypeID());
        
    if (listenersCollectionIt != m_listenersMap.cend()) {
        EventType event(std::forward<Args>(args)...);
        
        for (EventListener& listener : listenersCollectionIt->second) {
            listener.Excecute(&event);
        }
    }
}
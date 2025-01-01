template <typename T>
inline void EventDispatcher::Subscribe(const EventListener &listener) noexcept
{
    std::vector<EventListener>& listenersCollection = m_listenersMap[listener.GetTypeID()];

    if (listenersCollection.empty()) {
        static constexpr size_t LISTENERS_RESERVE_COUNT = 64ull;
        listenersCollection.reserve(LISTENERS_RESERVE_COUNT);
    }

    listenersCollection.emplace_back(listener);
}


template <typename EventT, typename... Args>
inline void EventDispatcher::Notify(Args &&...args) noexcept
{
    auto listenersCollectionIt = m_listenersMap.find(Event<EventT>::GetTypeID());
        
    if (listenersCollectionIt != m_listenersMap.cend()) {
        EventT event(std::forward<Args>(args)...);
        
        for (EventListener& listener : listenersCollectionIt->second) {
            listener.Excecute(&event);
        }
    }
}
#include "pch.h"
#include "event_dispatcher.h"

#include "utils/debug/assertion.h"


EventDispatcher& EventDispatcher::GetInstance()
{
    static EventDispatcher dispatcher;
    return dispatcher;
}


void EventDispatcher::Subscribe(const EventListener& listener) noexcept
{
    std::vector<EventListener>& listenersCollection = m_listenersMap[listener.GetTypeID()];

    if (listenersCollection.empty()) {
        static constexpr size_t LISTENERS_RESERVE_COUNT = 64ull;
        listenersCollection.reserve(LISTENERS_RESERVE_COUNT);
    }

    listenersCollection.emplace_back(listener);
}


EventDispatcher::EventDispatcher()
{
    static constexpr size_t EVENTS_TYPES_RESERVE_SIZE = 128ull;
    
    m_listenersMap.reserve(EVENTS_TYPES_RESERVE_SIZE);
}
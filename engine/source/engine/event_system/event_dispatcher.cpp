#include "pch.h"
#include "event_dispatcher.h"

#include "utils/debug/assertion.h"


EventDispatcher& EventDispatcher::GetInstance()
{
    static EventDispatcher dispatcher;
    return dispatcher;
}


EventDispatcher::EventDispatcher()
{
    static constexpr size_t EVENTS_TYPES_RESERVE_SIZE = 128ull;
    
    m_listenersMap.reserve(EVENTS_TYPES_RESERVE_SIZE);
}
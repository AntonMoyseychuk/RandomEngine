#pragma once

#include <vector>
#include <unordered_map>

#include <functional>
#include <typeindex>

#include <cstdint>

#include "utils/debug/assertion.h"


template<typename T>
class Event
{
public:
    using EventType = T;

public:
    static std::type_index GetTypeID() noexcept
    {
        static const std::type_index ID(typeid(EventType));
        return ID;
    }

public:
    template <typename... Args>
    Event(Args&&... args)
        : m_event{std::forward<Args>(args)...} {}

    const EventType& Get() const noexcept { return m_eventRef; }

private:
    EventType m_event;
};


class EventListener
{
    friend class EventDispatcher;

public:
    using CallbackType = std::function<void(const void* pEvent)>;

public:
    template<typename T>
    static EventListener Create(const CallbackType& callback) noexcept { return EventListener(Event<T>::GetTypeID(), callback); }

public:
    std::type_index GetTypeID() const noexcept { return m_typeID; }

private:
    EventListener(std::type_index typeID, const CallbackType& callback)
        : m_callback(callback), m_typeID(typeID) {}

    void Excecute(const void* pEvent) const noexcept { m_callback(pEvent); }

private:
    CallbackType m_callback;
    std::type_index m_typeID;
};


class EventDispatcher
{
public:
    static EventDispatcher& GetInstance();

public:
    EventDispatcher(const EventDispatcher& dispatcher) = delete;
    EventDispatcher& operator=(const EventDispatcher& dispatcher) = delete;
    EventDispatcher(EventDispatcher&& dispatcher) noexcept = delete;
    EventDispatcher& operator=(EventDispatcher&& dispatcher) noexcept = delete;

    void Subscribe(const EventListener& listener) noexcept;

    template<typename EventType, typename... Args>
    void Notify(Args&&... args) noexcept;

private:
    EventDispatcher();

private:
    std::unordered_map<std::type_index, std::vector<EventListener>> m_listenersMap; 
};


template<typename T>
inline const T& CastEventTo(const void* pEvent) noexcept
{
    ENG_ASSERT(pEvent != nullptr, "pEvent is nullptr");
    return *reinterpret_cast<const T*>(pEvent);
}


#include "event_dispatcher.hpp"
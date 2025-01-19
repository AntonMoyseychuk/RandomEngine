#pragma once

#include <vector>
#include <unordered_map>

#include <functional>
#include <typeindex>

#include <cstdint>

#include "utils/debug/assertion.h"
#include "utils/data_structures/base_id.h"
#include "utils/data_structures/strid.h"

#include "core.h"


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


class EventListenerID : public BaseID
{
    friend class EventDispatcher;

public:
    EventListenerID() = default;
    explicit EventListenerID(std::type_index eventTypeIndex);
    EventListenerID(uint64_t ID, std::type_index eventTypeIndex);
    EventListenerID(uint64_t ID, uint64_t eventTypeIndexHash);
    
    uint64_t TypeIndexHash() const noexcept { return m_eventTypeIndexHash; }

    void Invalidate() noexcept;
    bool IsValid() const noexcept { return BaseID::IsValid() && m_eventTypeIndexHash != UINT64_MAX; }

    uint64_t Hash() const noexcept;

    bool operator==(const EventListenerID& other) const noexcept { return BaseID::operator==(other) && m_eventTypeIndexHash == other.m_eventTypeIndexHash; }
    bool operator!=(const EventListenerID& other) const noexcept { return BaseID::operator!=(other) && m_eventTypeIndexHash != other.m_eventTypeIndexHash; }
    bool operator>(const EventListenerID& other) const noexcept { return BaseID::operator>(other) && m_eventTypeIndexHash > other.m_eventTypeIndexHash; }
    bool operator<(const EventListenerID& other) const noexcept { return BaseID::operator<(other) && m_eventTypeIndexHash < other.m_eventTypeIndexHash; }
    bool operator<=(const EventListenerID& other) const noexcept { return BaseID::operator<=(other) && m_eventTypeIndexHash <= other.m_eventTypeIndexHash; }
    bool operator>=(const EventListenerID& other) const noexcept { return BaseID::operator>=(other) && m_eventTypeIndexHash >= other.m_eventTypeIndexHash; }

private:
    uint64_t m_eventTypeIndexHash = UINT64_MAX;
};


class EventListener
{
    friend class EventDispatcher;

public:
    using CallbackType = std::function<void(const void* pEvent)>;

public:
    EventListener() = default;
    EventListener(EventListenerID ID, const CallbackType& callback);

    void SetDebugName(ds::StrID name) noexcept;
    ds::StrID GetDebugName(ds::StrID name) const noexcept;

    const EventListenerID& GetID() const noexcept { return m_ID; }
    bool IsValid() const noexcept { return m_ID.IsValid() && m_callback; }

    void Invalidate() noexcept;

private:
    void Excecute(const void* pEvent) const noexcept;

private:
#if defined(ENG_DEBUG)
    ds::StrID m_dbgName = "";
#endif
    EventListenerID m_ID;
    CallbackType m_callback;
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

    template <typename EventType>
    EventListenerID Subscribe(const EventListener::CallbackType& eventCallback) noexcept;

    void SetListenerDebugName(const EventListenerID& listenerID, ds::StrID name) noexcept;
    
    void Unsubscribe(EventListenerID listenerID) noexcept;

    template<typename EventType, typename... Args>
    void Notify(Args&&... args) noexcept;

private:
    EventDispatcher();

private:
    std::unordered_map<uint64_t, std::vector<EventListener>> m_listenersMap; 
};


template<typename T>
inline const T& CastEventTo(const void* pEvent) noexcept
{
    ENG_ASSERT(pEvent != nullptr, "pEvent is nullptr");
    return *reinterpret_cast<const T*>(pEvent);
}


#include "event_dispatcher.hpp"
#pragma once

#include <vector>
#include <unordered_map>

#include <functional>

#include <cstdint>

#include "utils/debug/assertion.h"
#include "utils/data_structures/base_id.h"

#include "core.h"


namespace es
{
    class ListenerID
    {
        friend class EventDispatcher;

    public:
        using UnderlyingType = uint32_t;

    public:
        ListenerID();
        ListenerID(UnderlyingType eventTypeIndex, UnderlyingType storageIndex);

        UnderlyingType GetEventTypeIndex() const noexcept { return m_eventTypeIndex; }
        UnderlyingType GetStorageIndex() const noexcept { return m_storageIndex; }
        
        void Invalidate() noexcept;
        bool IsValid() const noexcept { return m_eventTypeIndex != INVALID_EVENT_TYPE_IDX && m_storageIndex != INVALID_STORAGE_IDX; }

        bool operator==(const ListenerID& other) const noexcept { return m_eventTypeIndex == other.m_eventTypeIndex && m_storageIndex == other.m_storageIndex; }
        bool operator!=(const ListenerID& other) const noexcept { return !operator==(other); }
        bool operator>(const ListenerID& other) const noexcept { return m_eventTypeIndex > other.m_eventTypeIndex && m_storageIndex > other.m_storageIndex; }
        bool operator<(const ListenerID& other) const noexcept { return m_eventTypeIndex < other.m_eventTypeIndex && m_storageIndex < other.m_storageIndex; }
        bool operator<=(const ListenerID& other) const noexcept { return operator<(other) || operator==(other); }
        bool operator>=(const ListenerID& other) const noexcept { return operator>(other) || operator==(other); }

    private:
        template<uint32_t BASE, uint32_t POWER>
        struct ConstexprPow
        {
            static constexpr uint32_t value = BASE * ConstexprPow<BASE, POWER - 1>::value;
        };

        template<uint32_t BASE>
        struct ConstexprPow<BASE, 0>
        {
            static constexpr uint32_t value = 1;
        };

    private:
        static inline constexpr uint32_t BITS_PER_EVENT_TYPE_IDX = 10;
        static inline constexpr uint32_t BITS_PER_STORAGE_IDX = 22;
    
    public:
        static inline constexpr UnderlyingType MAX_EVENT_TYPE_IDX = ConstexprPow<2, BITS_PER_EVENT_TYPE_IDX>::value - 1;
        static inline constexpr UnderlyingType MAX_STORAGE_IDX = ConstexprPow<2, BITS_PER_STORAGE_IDX>::value - 1;
        
    private:
        static inline constexpr UnderlyingType INVALID_EVENT_TYPE_IDX = MAX_EVENT_TYPE_IDX;
        static inline constexpr UnderlyingType INVALID_STORAGE_IDX = MAX_STORAGE_IDX;

    private:
        struct {
            UnderlyingType m_eventTypeIndex : BITS_PER_EVENT_TYPE_IDX;
            UnderlyingType m_storageIndex : BITS_PER_STORAGE_IDX;
        };
    };


    using ListenerCallback = std::function<void(const void* pEvent)>;

    class ListenersStorage
    {
        friend class EventDispatcher;
        
    public:
        void Reserve(uint64_t capacity) noexcept { m_storage.reserve(capacity); }

        uint64_t Add(const ListenerCallback& callback) noexcept;
        void Remove(uint64_t index) noexcept;

        void Notify(const void* pEvent) noexcept;

        void Reset() noexcept;

        uint64_t GetSize() const noexcept { return m_storage.size(); }
        uint64_t GetCapacity() noexcept { return m_storage.capacity(); }

    private:
    static inline constexpr uint64_t MAX_LISTENERS_STORAGE_CAPACITY = ListenerID::MAX_STORAGE_IDX + 1;
        static inline const ListenerCallback DEFAULT_EVENT_CALLBACK = [](const void*) -> void {}; 

    private:
        using ListenerIndex = ds::BaseID<ListenerID::UnderlyingType>;
        using ListenerIndexPool = ds::BaseIDPool<ListenerIndex>;

        std::vector<ListenerCallback> m_storage;
        ListenerIndexPool m_idxPool;
    };


    class EventDispatcher
    {
    public:
        static EventDispatcher& GetInstance();

        template <typename EventType>
        static uint64_t GetEventTypeIndex() noexcept
        {
            static uint64_t index = AllocateEventTypeIndex();
            return index;
        }
    
    public:
        EventDispatcher(const EventDispatcher& dispatcher) = delete;
        EventDispatcher& operator=(const EventDispatcher& dispatcher) = delete;
        EventDispatcher(EventDispatcher&& dispatcher) noexcept = delete;
        EventDispatcher& operator=(EventDispatcher&& dispatcher) noexcept = delete;
    
        template <typename EventType>
        ListenerID Subscribe(const ListenerCallback& listenerCallback) noexcept;
        
        void Unsubscribe(ListenerID& listenerID) noexcept;

        template<typename EventType, typename... Args>
        void Notify(Args&&... args) noexcept;

        void Reset() noexcept;

    private:
        EventDispatcher();

        static uint64_t AllocateEventTypeIndex() noexcept
        {
            static uint64_t index = 0;
            return index++;
        }

    private:
        static inline constexpr uint64_t MAX_EVENT_TYPES_COUNT = ListenerID::MAX_EVENT_TYPE_IDX + 1;

    private:
        std::array<ListenersStorage, MAX_EVENT_TYPES_COUNT> m_storages;
    };


    template<typename T>
    inline const T& EventCast(const void* pEvent) noexcept
    {
        ENG_ASSERT(pEvent != nullptr, "pEvent is nullptr");
        return *reinterpret_cast<const T*>(pEvent);
    }
}

#include "event_dispatcher.hpp"
#include "pch.h"

#include "event_dispatcher.h"


namespace es
{
    ListenerID::ListenerID()
        : m_eventTypeIndex(INVALID_EVENT_TYPE_IDX), m_storageIndex(INVALID_STORAGE_IDX)
    {
    }
    
    
    void ListenerID::Invalidate() noexcept
    {
        m_eventTypeIndex = INVALID_EVENT_TYPE_IDX;
        m_storageIndex = INVALID_STORAGE_IDX;
    }

    
    ListenerID::ListenerID(UnderlyingType eventTypeIndex, UnderlyingType storageIndex)
        : m_eventTypeIndex(eventTypeIndex), m_storageIndex(storageIndex)
    {
    }


    uint64_t ListenersStorage::Add(const ListenerCallback& callback) noexcept
    {
        ENG_ASSERT(bool(callback), "Invalid callback");

        const uint64_t idx = m_idxPool.Allocate().Value();
        ENG_ASSERT(idx < MAX_LISTENERS_STORAGE_CAPACITY, "Listeners limit has been reached");

        if (idx >= m_storage.capacity()) {
            ENG_LOG_WARN("Event dispatcher: listeners storage ({}) reallocation", idx);
            m_storage.emplace_back(callback);
        } else if (idx >= m_storage.size()) {
            m_storage.emplace_back(callback);
        } else {
            m_storage[idx] = callback;
        }
        
        return idx;
    }


    void ListenersStorage::Remove(uint64_t index) noexcept
    {
        ListenerIndex listenerIdx(index);

        if (!m_idxPool.IsAllocated(listenerIdx)) {
            return;
        }

        if (index == m_storage.size() - 1) {
            m_storage.pop_back();
        } else {
            m_storage[index] = DEFAULT_EVENT_CALLBACK;
        }
        
        m_idxPool.Deallocate(listenerIdx);
    }


    void ListenersStorage::Notify(const void* pEvent) noexcept
    {
        ENG_ASSERT(pEvent, "pEvent is nullptr");

        for (const ListenerCallback& callback : m_storage) {
            callback(pEvent);
        }
    }


    inline void ListenersStorage::Reset() noexcept
    {
        m_storage.clear();
        m_idxPool.Reset();
    }


    EventDispatcher& EventDispatcher::GetInstance()
    {
        static EventDispatcher dispatcher;
        return dispatcher;
    }


    void EventDispatcher::Unsubscribe(ListenerID& listenerID) noexcept
    {
        if (!listenerID.IsValid()) {
            return;
        }
        
        ListenersStorage& listenersStorage = m_storages[listenerID.GetEventTypeIndex()];

        listenersStorage.Remove(listenerID.GetStorageIndex());
        listenerID.Invalidate();
    }


    void EventDispatcher::Reset() noexcept
    {
        for (ListenersStorage& storage : m_storages) {
            storage.Reset();
        }
    }


    EventDispatcher::EventDispatcher()
    {
        for (ListenersStorage& storage : m_storages) {
            storage.Reserve(1024);
        }
    }
}
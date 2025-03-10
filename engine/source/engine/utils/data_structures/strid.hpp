namespace ds 
{
    template <typename ElemT>
    inline StrIDDataStorage<ElemT>::StrIDDataStorage()
    {
        m_strBufLocations.reserve(PREALLOCATED_IDS_COUNT);
        m_storage.resize(PREALLOCATED_STORAGE_SIZE);
    }


    template <typename ElemT>
    inline uint64_t StrIDDataStorage<ElemT>::Store(const StrIDDataStorage<ElemT>::StringViewType &str) noexcept
    {
        if (!str.data()) {
            return INVALID_ID_HASH;
        }

        const uint64_t id = amHash(str);

        if (!IsExist(id)) {
            StringBufLocation& newStrBufLocation = m_strBufLocations[id];
            
            newStrBufLocation.length = str.length() + 1; // including null terminator

            if (m_lastAllocatedID != INVALID_ID_HASH) {
                const StringBufLocation& lastStrBufLocation = m_strBufLocations[m_lastAllocatedID];

                newStrBufLocation.offset = lastStrBufLocation.offset + lastStrBufLocation.length;
            }

            const uint64_t newSize = newStrBufLocation.offset + newStrBufLocation.length;
            
            if (newSize > m_storage.size()) {
                ENG_ASSERT_FAIL("StrID storage overflow, think about increasing the size of the storage buffer");
                m_storage.resize(newSize * 2ull);
            }

            std::copy_n(str.begin(), newStrBufLocation.length - 1, m_storage.begin() + newStrBufLocation.offset);

            m_size = newSize;
            m_lastAllocatedID = id;
        }

        return id;
    }


    template <typename ElemT>
    inline const typename StrIDDataStorage<ElemT>::ElementType* StrIDDataStorage<ElemT>::Load(uint64_t id) const noexcept
    {
        const auto strBufLocationIt = m_strBufLocations.find(id);

        return strBufLocationIt == m_strBufLocations.cend() ? nullptr : m_storage.data() + strBufLocationIt->second.offset;
    }


    template <typename ElemT>
    inline StrIDImpl<ElemT>::StrIDImpl(const ElementType *str)
        : m_id(s_storage.Store(str))
    {
    #if defined(ENG_DEBUG)
        m_pStr = s_storage.Load(m_id);
    #endif
    }
    
    
    template <typename ElemT>
    inline StrIDImpl<ElemT>::StrIDImpl(const StringType &str)
        : m_id(s_storage.Store(str))
    {
    #if defined(ENG_DEBUG)
        m_pStr = s_storage.Load(m_id);
    #endif
    }
    
    
    template <typename ElemT>
    inline StrIDImpl<ElemT>::StrIDImpl(const StringViewType &str)
        : m_id(s_storage.Store(str))
    {
    #if defined(ENG_DEBUG)
        m_pStr = s_storage.Load(m_id);
    #endif
    }


    template <typename ElemT>
    inline StrIDImpl<ElemT>& StrIDImpl<ElemT>::operator=(const typename StrIDImpl<ElemT>::ElementType *str) noexcept
    {
        m_id = s_storage.Store(str);
    #if defined(ENG_DEBUG)
        m_pStr = s_storage.Load(m_id);
    #endif

        return *this;
    }
    
    
    template <typename ElemT>
    inline const ElemT* StrIDImpl<ElemT>::CStr() const noexcept
    {
    #if defined(ENG_DEBUG)
        return m_pStr;
    #else
        return s_storage.Load(m_id);
    #endif
    }
}
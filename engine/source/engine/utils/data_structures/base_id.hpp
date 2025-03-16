namespace ds
{
    template <typename BaseIDType>
    inline typename BaseIDPool<BaseIDType>::IDType BaseIDPool<BaseIDType>::Allocate() noexcept
    {
        if (m_idFreeList.empty()) {
            const IDType ID = m_nextAllocatedID;
            m_nextAllocatedID = IDType(m_nextAllocatedID.Value() + 1);
    
            return ID;
        }
    
        const IDType ID = m_idFreeList.front();
        m_idFreeList.pop_front();
            
        return ID;
    }


    template <typename BaseIDType>
    inline void BaseIDPool<BaseIDType>::Deallocate(IDType& ID) noexcept
    {
        if (ID < m_nextAllocatedID && std::find(m_idFreeList.cbegin(), m_idFreeList.cend(), ID) == m_idFreeList.cend()) {
            m_idFreeList.emplace_back(ID);
        }

        ID.Invalidate();
    }


    template <typename BaseIDType>
    inline void BaseIDPool<BaseIDType>::Reset() noexcept
    {
        m_idFreeList.clear();
        m_nextAllocatedID.SetValue(0);
    }


    template <typename BaseIDType>
    inline bool BaseIDPool<BaseIDType>::IsAnyAllocated() const noexcept
    {
        return m_nextAllocatedID > IDType{0} && m_idFreeList.size() < m_nextAllocatedID.Value();
    }


    template <typename BaseIDType>
    inline bool BaseIDPool<BaseIDType>::IsAllocated(const IDType& ID) const noexcept
    {
        return ID < m_nextAllocatedID && std::find(m_idFreeList.cbegin(), m_idFreeList.cend(), ID) == m_idFreeList.cend();
    }
}

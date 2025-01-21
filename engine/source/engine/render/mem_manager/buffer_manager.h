#pragma once

#include "core.h"

#include "utils/data_structures/base_id.h"
#include "utils/data_structures/strid.h"

#include <deque>


enum class MemoryBufferType : uint16_t
{
    TYPE_VERTEX_BUFFER,
    TYPE_INDEX_BUFFER,
    TYPE_CONSTANT_BUFFER,
    TYPE_UNORDERED_ACCESS_BUFFER,

    TYPE_COUNT,
    TYPE_INVALID,
};


enum MemoryBufferCreationFlags : uint16_t
{
    BUFFER_CREATION_FLAG_ZERO = 0x0,
    BUFFER_CREATION_FLAG_DYNAMIC_STORAGE = 0x1,
    BUFFER_CREATION_FLAG_READABLE = 0x2,
    BUFFER_CREATION_FLAG_WRITABLE = 0x4,
    BUFFER_CREATION_FLAG_PERSISTENT = 0x8,      // Allows mapping a buffer that can remain mapped while the GPU is using it
    BUFFER_CREATION_FLAG_COHERENT = 0x10,       // Allows for more efficient synchronization between client and server access to the buffer
    BUFFER_CREATION_FLAG_CLIENT_STORAGE = 0x20  // Indicates that the driver should prefer backing the data with system memory rather than GPU memory
};


struct MemoryBufferCreateInfo
{
    const void*               pData;
    uint64_t                  dataSize;
    uint64_t                  elementSize;

    MemoryBufferType          type;
    MemoryBufferCreationFlags creationFlags;
};


class MemoryBuffer
{
    friend class MemoryBufferManager;

public:
    MemoryBuffer() = default;

    MemoryBuffer(const MemoryBuffer& other) = delete;
    MemoryBuffer& operator=(const MemoryBuffer& other) = delete;
    
    MemoryBuffer(MemoryBuffer&& other) noexcept;
    MemoryBuffer& operator=(MemoryBuffer&& other) noexcept;

    void FillSubdata(size_t offset, size_t size, const void* pData) noexcept;
    void Clear(size_t offset, size_t size, const void* pData) noexcept;
    void Clear() noexcept;

    void Bind() noexcept;
    void BindIndexed(uint32_t index) noexcept;

    const void* MapRead() noexcept;
    void* MapWrite() noexcept;
    void* MapReadWrite() noexcept;
    bool Unmap() const noexcept;

    bool IsValidRenderID() const noexcept { return m_renderID != 0; }
    bool IsValidType() const noexcept { return m_type != MemoryBufferType::TYPE_INVALID; }
    bool IsValid() const noexcept { return IsValidRenderID() && IsValidType(); }

    ds::StrID GetName() const noexcept;

    uint64_t GetSize() const noexcept { return m_size; }
    uint64_t GetElementSize() const noexcept { return m_elementSize; }
    uint64_t GetElementCount() const noexcept;

    MemoryBufferType GetType() const noexcept { return m_type; }
    bool IsVertexBuffer() const noexcept { return m_type == MemoryBufferType::TYPE_VERTEX_BUFFER; }
    bool IsIndexBuffer() const noexcept { return m_type == MemoryBufferType::TYPE_INDEX_BUFFER; }
    bool IsConstantBuffer() const noexcept { return m_type == MemoryBufferType::TYPE_CONSTANT_BUFFER; }
    bool IsUnorderedAccessBuffer() const noexcept { return m_type == MemoryBufferType::TYPE_UNORDERED_ACCESS_BUFFER; }

    MemoryBufferCreationFlags GetCreationFlags() const noexcept { return m_creationFlags; }
    bool IsDynamicStorage() const noexcept { return m_creationFlags & BUFFER_CREATION_FLAG_DYNAMIC_STORAGE; }
    bool IsClientStorage() const noexcept { return m_creationFlags & BUFFER_CREATION_FLAG_CLIENT_STORAGE; }
    bool IsReadable() const noexcept { return m_creationFlags & BUFFER_CREATION_FLAG_READABLE; }
    bool IsWritable() const noexcept { return m_creationFlags & BUFFER_CREATION_FLAG_WRITABLE; }
    bool IsPersistent() const noexcept { return m_creationFlags & BUFFER_CREATION_FLAG_PERSISTENT; }
    bool IsCoherent() const noexcept { return m_creationFlags & BUFFER_CREATION_FLAG_COHERENT; }

private:
    bool Init(ds::StrID name, const MemoryBufferCreateInfo& createInfo) noexcept;
    void Destroy() noexcept;

private:
#if defined(ENG_DEBUG)
    ds::StrID m_name = "";
#endif

    uint64_t                  m_size = 0;
    uint64_t                  m_elementSize = 0;
    MemoryBufferType          m_type = MemoryBufferType::TYPE_INVALID;
    MemoryBufferCreationFlags m_creationFlags = MemoryBufferCreationFlags::BUFFER_CREATION_FLAG_ZERO;
    uint32_t                  m_renderID = 0;
};


using BufferID = BaseID;


class MemoryBufferManager
{
    friend bool engInitMemoryBufferManager() noexcept;
    friend void engTerminateMemoryBufferManager() noexcept;
    friend bool engIsMemoryBufferManagerInitialized() noexcept;

public:
    static MemoryBufferManager& GetInstance() noexcept;

public:
    MemoryBufferManager(const MemoryBufferManager& other) = delete;
    MemoryBufferManager& operator=(const MemoryBufferManager& other) = delete;
    MemoryBufferManager(MemoryBufferManager&& other) noexcept = delete;
    MemoryBufferManager& operator=(MemoryBufferManager&& other) noexcept = delete;

    ~MemoryBufferManager();

    BufferID AllocateBuffer(ds::StrID name, const MemoryBufferCreateInfo& createInfo) noexcept;
    void DeallocateBuffer(BufferID ID);

    MemoryBuffer* GetBuffer(BufferID ID) noexcept;

    bool IsValidBuffer(BufferID ID) const noexcept;

private:
    MemoryBufferManager() = default;

    bool Init() noexcept;
    void Terminate() noexcept;

    BufferID AllocateBufferID() noexcept;
    void DeallocateBufferID(BufferID ID) noexcept;

    bool IsInitialized() const noexcept;

private:
    std::vector<MemoryBuffer> m_buffersStorage;

    std::deque<BufferID> m_memBufferIDFreeList;
    BufferID m_nextAllocatedID = BufferID{0};

    bool m_isInitialized = false;
};


bool engInitMemoryBufferManager() noexcept;
void engTerminateMemoryBufferManager() noexcept;
bool engIsMemoryBufferManagerInitialized() noexcept;
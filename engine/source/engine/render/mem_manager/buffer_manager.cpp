#include "pch.h"
#include "buffer_manager.h"

#include "utils/debug/assertion.h"

#include "render/platform/OpenGL/opengl_driver.h"


static std::unique_ptr<MemoryBufferManager> g_pMemoryBufferMng = nullptr;

static constexpr size_t MAX_MEM_BUFFER_COUNT = 4096;


static GLbitfield GetMemoryBufferCreationFlagsGL(MemoryBufferCreationFlags flags) noexcept
{
    GLbitfield result = BUFFER_CREATION_FLAG_ZERO;

    result |= (flags & BUFFER_CREATION_FLAG_DYNAMIC_STORAGE ? GL_DYNAMIC_STORAGE_BIT : 0);
    result |= (flags & BUFFER_CREATION_FLAG_READABLE ? GL_MAP_READ_BIT : 0);
    result |= (flags & BUFFER_CREATION_FLAG_WRITABLE ? GL_MAP_WRITE_BIT : 0);
    result |= (flags & BUFFER_CREATION_FLAG_PERSISTENT ? GL_MAP_PERSISTENT_BIT : 0);
    result |= (flags & BUFFER_CREATION_FLAG_COHERENT ? GL_MAP_COHERENT_BIT : 0);
    result |= (flags & BUFFER_CREATION_FLAG_CLIENT_STORAGE ? GL_CLIENT_STORAGE_BIT : 0);
    
    return result;
}


static GLenum TranslateMemoryBufferTypeToGL(MemoryBufferType type) noexcept
{
    switch(type) {
        case MemoryBufferType::TYPE_VERTEX_BUFFER:           return GL_ARRAY_BUFFER;
        case MemoryBufferType::TYPE_INDEX_BUFFER:            return GL_ELEMENT_ARRAY_BUFFER;
        case MemoryBufferType::TYPE_CONSTANT_BUFFER:         return GL_UNIFORM_BUFFER;
        case MemoryBufferType::TYPE_UNORDERED_ACCESS_BUFFER: return GL_SHADER_STORAGE_BUFFER;
        default:
            ENG_ASSERT_FAIL("Invalid memory buffer type");
            return GL_NONE;
    }
}


static bool IsBufferIndexedBindable(MemoryBufferType type) noexcept
{
    switch(type) {
        case MemoryBufferType::TYPE_VERTEX_BUFFER:           return false;
        case MemoryBufferType::TYPE_INDEX_BUFFER:            return false;
        case MemoryBufferType::TYPE_CONSTANT_BUFFER:         return true;
        case MemoryBufferType::TYPE_UNORDERED_ACCESS_BUFFER: return true;
        default:
            ENG_ASSERT_FAIL("Invalid memory buffer type");
            return GL_NONE;
    }
}


MemoryBuffer::MemoryBuffer(MemoryBuffer &&other) noexcept
{
#if defined(ENG_DEBUG)
    std::swap(m_name, other.m_name);
#endif
    std::swap(m_ID, other.m_ID);
    std::swap(m_size, other.m_size);
    std::swap(m_renderID, other.m_renderID);
    std::swap(m_elementSize, other.m_elementSize);
    std::swap(m_type, other.m_type);
    std::swap(m_creationFlags, other.m_creationFlags);
}


MemoryBuffer &MemoryBuffer::operator=(MemoryBuffer&& other) noexcept
{
    Destroy();

#if defined(ENG_DEBUG)
    std::swap(m_name, other.m_name);
#endif
    std::swap(m_ID, other.m_ID);
    std::swap(m_size, other.m_size);
    std::swap(m_renderID, other.m_renderID);
    std::swap(m_elementSize, other.m_elementSize);
    std::swap(m_type, other.m_type);
    std::swap(m_creationFlags, other.m_creationFlags);

    return *this;
}


void MemoryBuffer::FillSubdata(size_t offset, size_t size, const void *pData) noexcept
{
    ENG_ASSERT(IsValid(), "Memory buffer \'{}\' is invalid", m_name.CStr());
    ENG_ASSERT(IsDynamicStorage(), "Memory buffer \'{}\' was not created with BUFFER_CREATION_FLAG_DYNAMIC_STORAGE flag", m_name.CStr());
    glNamedBufferSubData(m_renderID, offset, size, pData);
}


void MemoryBuffer::Clear(size_t offset, size_t size, const void *pData) noexcept
{
    ENG_ASSERT(IsValid(), "Memory buffer \'{}\' is invalid", m_name.CStr());
    glClearNamedBufferSubData(m_renderID, GL_R32F, offset, size, GL_RED, GL_FLOAT, pData);
}


void MemoryBuffer::Clear() noexcept
{
    Clear(0, m_size, nullptr);
}


void MemoryBuffer::Bind() noexcept
{
    ENG_ASSERT(IsValid(), "Memory buffer \'{}\' is invalid", m_name.CStr());

    const GLenum target = TranslateMemoryBufferTypeToGL(m_type);
    glBindBuffer(target, m_renderID);
}


void MemoryBuffer::BindIndexed(uint32_t index) noexcept
{
    ENG_ASSERT(IsValid(), "Memory buffer \'{}\' is invalid", m_name.CStr());
    ENG_ASSERT(IsBufferIndexedBindable(m_type), "Memory buffer \'{}\' is not indexed bindable", m_name.CStr());

    const GLenum target = TranslateMemoryBufferTypeToGL(m_type);
    glBindBufferBase(target, index, m_renderID);
}


const void* MemoryBuffer::MapRead() noexcept
{
    ENG_ASSERT(IsValid(), "Memory buffer \'{}\' is invalid", m_name.CStr());
    ENG_ASSERT(IsReadable(), "Memory buffer \'{}\' was not created with BUFFER_CREATION_FLAG_READABLE flag", m_name.CStr());
    
    return glMapNamedBuffer(m_renderID, GL_READ_ONLY);
}


void* MemoryBuffer::MapWrite() noexcept
{
    ENG_ASSERT(IsValid(), "Memory buffer \'{}\' is invalid", m_name.CStr());
    ENG_ASSERT(IsDynamicStorage(), "Memory buffer \'{}\' was not created with GL_DYNAMIC_STORAGE_BIT flag", m_name.CStr());
    ENG_ASSERT(IsWritable(), "Memory buffer \'{}\' was not created with BUFFER_CREATION_FLAG_WRITABLE flag", m_name.CStr());
    
    return glMapNamedBuffer(m_renderID, GL_WRITE_ONLY);
}


void* MemoryBuffer::MapReadWrite() noexcept
{
    ENG_ASSERT(IsValid(), "Memory buffer \'{}\' is invalid", m_name.CStr());
    ENG_ASSERT(IsDynamicStorage(), "Memory buffer \'{}\' was not created with GL_DYNAMIC_STORAGE_BIT flag", m_name.CStr());
    ENG_ASSERT(IsWritable(), "Memory buffer \'{}\' was not created with BUFFER_CREATION_FLAG_WRITABLE flag", m_name.CStr());
    ENG_ASSERT(IsReadable(), "Memory buffer \'{}\' was not created with BUFFER_CREATION_FLAG_READABLE flag", m_name.CStr());

    return glMapNamedBuffer(m_renderID, GL_READ_WRITE);
}


bool MemoryBuffer::Unmap() const noexcept
{
    ENG_ASSERT(IsValid(), "Memory buffer \'{}\' is invalid", m_name.CStr());
    
    return glUnmapNamedBuffer(m_renderID);
}


bool MemoryBuffer::IsValid() const noexcept
{
    return m_ID.IsValid() && m_type != MemoryBufferType::TYPE_INVALID && m_renderID != 0;
}


ds::StrID MemoryBuffer::GetName() const noexcept
{
#if defined(ENG_DEBUG)
    return m_name;
#else
    return "";
#endif
}


uint64_t MemoryBuffer::GetElementCount() const noexcept
{
    ENG_ASSERT(m_elementSize != 0, "Element size is 0");
    ENG_ASSERT(m_size % m_elementSize == 0, "Buffer size must be multiple of element size");
    
    return m_size / m_elementSize;
}


bool MemoryBuffer::Create(const MemoryBufferCreateInfo& createInfo) noexcept
{
    ENG_ASSERT(m_ID.IsValid(), "Buffer ID is invalid. You must initialize only buffers which were returned by MemoryBufferManager");
    ENG_ASSERT(createInfo.type != MemoryBufferType::TYPE_INVALID && createInfo.type < MemoryBufferType::TYPE_COUNT,
        "Invalid \'{}\' buffer create info type", m_name.CStr());

    ENG_ASSERT(createInfo.dataSize > 0, "Invalid \'{}\' buffer create info data size", m_name.CStr());
    ENG_ASSERT(createInfo.elementSize > 0, "Invalid \'{}\' buffer create info data element size", m_name.CStr());
    ENG_ASSERT(createInfo.dataSize % createInfo.elementSize == 0, "Data size is must be multiple of element size");

    if (IsValid()) {
        ENG_LOG_WARN("Recreating \'{}\' buffer", m_name.CStr());
        Destroy();
    }

    glCreateBuffers(1, &m_renderID);

    const GLbitfield creationFlags = GetMemoryBufferCreationFlagsGL(createInfo.creationFlags);
    glNamedBufferStorage(m_renderID, createInfo.dataSize, createInfo.pData, creationFlags);

    m_size = createInfo.dataSize;
    m_elementSize = createInfo.elementSize;
    m_type = createInfo.type;
    m_creationFlags = createInfo.creationFlags;

    return true;
}


void MemoryBuffer::Destroy() noexcept
{
    if (!IsValid()) {
        return;
    }

    glDeleteBuffers(1, &m_renderID);

    m_size = 0;
    m_elementSize = 0;
    m_type = MemoryBufferType::TYPE_INVALID;
    m_creationFlags = MemoryBufferCreationFlags::BUFFER_CREATION_FLAG_ZERO;
    m_renderID = 0;
}


MemoryBufferManager& MemoryBufferManager::GetInstance() noexcept
{
    ENG_ASSERT(engIsMemoryBufferManagerInitialized(), "Memory buffer manager is not initialized");
    return *g_pMemoryBufferMng;
}


MemoryBufferManager::~MemoryBufferManager()
{
    Terminate();
}


MemoryBuffer* MemoryBufferManager::RegisterBuffer(ds::StrID name) noexcept
{
    ENG_ASSERT(m_nextAllocatedID.Value() < m_buffersStorage.size() - 1, "Memory buffer storage overflow");

    const BufferID bufferID = AllocateBufferID();
    MemoryBuffer* pBuffer = &m_buffersStorage[bufferID.Value()];

    ENG_ASSERT(!pBuffer->IsValid(), "Valid buffer was returned during registration");

#if defined(ENG_DEBUG)
    pBuffer->m_name = name;
#endif
    pBuffer->m_ID = bufferID;

    return pBuffer;
}


void MemoryBufferManager::UnregisterBuffer(MemoryBuffer* pBuffer)
{
    if (!pBuffer) {
        return;
    }

    if (pBuffer->IsValid()) {
        ENG_LOG_WARN("Unregistration of buffer \'{}\' while it's steel valid. Prefer to destroy buffers manually", pBuffer->GetName().CStr());
        pBuffer->Destroy();
    }

    DeallocateBufferID(pBuffer->m_ID);

#if defined(ENG_DEBUG)
    pBuffer->m_name = "";
#endif
    pBuffer->m_ID.Invalidate();
}


bool MemoryBufferManager::Init() noexcept
{
    if (IsInitialized()) {
        return true;
    }

    m_buffersStorage.resize(MAX_MEM_BUFFER_COUNT);
    m_nextAllocatedID = BufferID(0);
    m_isInitialized = true;

    return true;
}


void MemoryBufferManager::Terminate() noexcept
{
    m_buffersStorage.clear();
    m_memBufferIDFreeList.clear();
    m_nextAllocatedID = BaseID(0);
    m_isInitialized = false;
}


BufferID MemoryBufferManager::AllocateBufferID() noexcept
{
    if (m_memBufferIDFreeList.empty()) {
        const BufferID bufferID = m_nextAllocatedID;
        m_nextAllocatedID = BufferID(m_nextAllocatedID.Value() + 1);

        return bufferID;
    }

    const BufferID bufferID = m_memBufferIDFreeList.front();
    m_memBufferIDFreeList.pop_front();
        
    return bufferID;
}


void MemoryBufferManager::DeallocateBufferID(BufferID ID) noexcept
{
    if (ID < m_nextAllocatedID && std::find(m_memBufferIDFreeList.cbegin(), m_memBufferIDFreeList.cend(), ID) == m_memBufferIDFreeList.cend()) {
        m_memBufferIDFreeList.emplace_back(ID);
    }
}


bool MemoryBufferManager::IsInitialized() const noexcept
{
    return m_isInitialized;
}


bool engInitMemoryBufferManager() noexcept
{
    if (engIsMemoryBufferManagerInitialized()) {
        ENG_LOG_WARN("Memory buffer manager is already initialized!");
        return true;
    }

    g_pMemoryBufferMng = std::unique_ptr<MemoryBufferManager>(new MemoryBufferManager);

    if (!g_pMemoryBufferMng) {
        ENG_ASSERT_FAIL("Failed to allocate memory for memory buffer manager");
        return false;
    }

    if (!g_pMemoryBufferMng->Init()) {
        ENG_ASSERT_FAIL("Failed to initialized memory buffer manager");
        return false;
    }

    return true;
}


void engTerminateMemoryBufferManager() noexcept
{
    g_pMemoryBufferMng = nullptr;
}


bool engIsMemoryBufferManagerInitialized() noexcept
{
    return g_pMemoryBufferMng && g_pMemoryBufferMng->IsInitialized();
}
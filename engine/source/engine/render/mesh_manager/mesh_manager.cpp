#include "pch.h"
#include "mesh_manager.h"

#include "render/platform/OpenGL/opengl_driver.h"


static constexpr size_t MAX_MESH_OBJ_COUNT = 8192;
 
static int32_t G_MAX_VERTEX_ATTRIBS_COUNT = 0;
static std::unique_ptr<MeshManager> g_pMeshMng = nullptr;


static uint64_t GetAttribDataTypeSizeInBytes(MeshVertexAttribDataType type) noexcept
{
    switch (type) {
        case MeshVertexAttribDataType::TYPE_UNSIGNED_BYTE: return 1ULL;
        case MeshVertexAttribDataType::TYPE_BYTE: return 1ULL;
        case MeshVertexAttribDataType::TYPE_UNSIGNED_SHORT: return 2ULL;
        case MeshVertexAttribDataType::TYPE_SHORT: return 2ULL;
        case MeshVertexAttribDataType::TYPE_UNSIGNED_INT: return 4ULL;
        case MeshVertexAttribDataType::TYPE_INT: return 4ULL;
        case MeshVertexAttribDataType::TYPE_FLOAT: return 4ULL;
        
        default:
            ENG_ASSERT_FAIL("Invalid vertex attrib data type");
            return UINT64_MAX;
    }
}


static GLenum GetAttribDataGLType(MeshVertexAttribDataType type) noexcept
{
    switch (type) {
        case MeshVertexAttribDataType::TYPE_UNSIGNED_BYTE: return GL_UNSIGNED_BYTE;
        case MeshVertexAttribDataType::TYPE_BYTE: return GL_BYTE;
        case MeshVertexAttribDataType::TYPE_UNSIGNED_SHORT: return GL_UNSIGNED_SHORT;
        case MeshVertexAttribDataType::TYPE_SHORT: return GL_SHORT;
        case MeshVertexAttribDataType::TYPE_UNSIGNED_INT: return GL_UNSIGNED_INT;
        case MeshVertexAttribDataType::TYPE_INT: return GL_INT;
        case MeshVertexAttribDataType::TYPE_FLOAT: return GL_FLOAT;
        
        default:
            ENG_ASSERT_FAIL("Invalid vertex attrib data type");
            return UINT64_MAX;
    }
}


MeshObj::MeshObj(MeshObj&& other) noexcept
{
    std::swap(m_name, other.m_name);
    std::swap(m_pVertexGPUBuffer, other.m_pVertexGPUBuffer);
    std::swap(m_pIndexGPUBuffer, other.m_pIndexGPUBuffer);
    std::swap(m_ID, other.m_ID);
    std::swap(m_vaoRenderID, other.m_vaoRenderID);
}


MeshObj& MeshObj::operator=(MeshObj&& other) noexcept
{
    Destroy();

    std::swap(m_name, other.m_name);
    std::swap(m_pVertexGPUBuffer, other.m_pVertexGPUBuffer);
    std::swap(m_pIndexGPUBuffer, other.m_pIndexGPUBuffer);
    std::swap(m_ID, other.m_ID);
    std::swap(m_vaoRenderID, other.m_vaoRenderID);

    return *this;
}


bool MeshObj::Create(const MeshObjCreateInfo& createInfo) noexcept
{
    ENG_ASSERT(m_ID.IsValid(), "Mesh object \'{}\' ID is invalid. You must initialize only mesh objects which were returned by MeshManager", m_name.CStr());

    ENG_ASSERT(createInfo.pVertexAttribDescs, "Mesh object \'{}\' createInfo.pVertexAttribDescs is nullptr", m_name.CStr());
    ENG_ASSERT(createInfo.vertexAttribDescsCount >= 1 && createInfo.vertexAttribDescsCount <= G_MAX_VERTEX_ATTRIBS_COUNT, 
        "Mesh object \'{}\' createInfo.vertexAttribDescsCount must be at least 1 and less or equal {}", m_name.CStr(), G_MAX_VERTEX_ATTRIBS_COUNT);

    ENG_ASSERT(createInfo.pVertexData, "Mesh object \'{}\' createInfo.pVertexData is nullptr", m_name.CStr());
    ENG_ASSERT(createInfo.vertexDataSize > 0, "Mesh object \'{}\' createInfo.vertexDataSize is zero", m_name.CStr());

    if (IsValid()) {
        ENG_LOG_WARN("Recreating \'{}\' mesh object", m_name.CStr());
        Destroy();
    }

    glCreateVertexArrays(1, &m_vaoRenderID);

    uint64_t vertexSize = 0;

    for (uint64_t i = 0; i < createInfo.vertexAttribDescsCount; ++i) {
        const MeshVertexAttribDesc& vertAttribDesc = createInfo.pVertexAttribDescs[i];

        vertexSize += vertAttribDesc.elementsCount * GetAttribDataTypeSizeInBytes(vertAttribDesc.dataType);

        glVertexArrayAttribBinding(m_vaoRenderID, vertAttribDesc.index, 0);

        const GLenum type = GetAttribDataGLType(vertAttribDesc.dataType);

        ENG_ASSERT(vertAttribDesc.elementsCount <= 4, "Invalid elements count");
        glVertexArrayAttribFormat(m_vaoRenderID, vertAttribDesc.index, vertAttribDesc.elementsCount, type, vertAttribDesc.isNormalized, vertAttribDesc.offset);
        
        glEnableVertexArrayAttrib(m_vaoRenderID, vertAttribDesc.index);
    }

    MemoryBufferManager& memBufferMng = MemoryBufferManager::GetInstance();

#if defined(ENG_DEBUG)
    const std::string vertBuffName = fmt::format("{}_{}", m_name.CStr(), "VERT_BUFF");
#else
    const char* vertBuffName = "";
#endif

    m_pVertexGPUBuffer = memBufferMng.RegisterBuffer(vertBuffName);
    ENG_ASSERT(m_pVertexGPUBuffer, "Failed to register \'{}\' vertex buffer", vertBuffName.c_str());
    
    MemoryBufferCreateInfo vertBuffCreateInfo = {};
    vertBuffCreateInfo.type          = MemoryBufferType::TYPE_VERTEX_BUFFER;
    vertBuffCreateInfo.creationFlags = BUFFER_CREATION_FLAG_ZERO;
    vertBuffCreateInfo.pData         = createInfo.pVertexData;
    vertBuffCreateInfo.dataSize      = createInfo.vertexDataSize;
    vertBuffCreateInfo.elementSize   = vertexSize;
    
    m_pVertexGPUBuffer->Create(vertBuffCreateInfo);
    ENG_ASSERT(m_pVertexGPUBuffer->IsValid(), "Failed to create \'{}\' vertex buffer", vertBuffName.c_str());

    glVertexArrayVertexBuffer(m_vaoRenderID, 0, m_pVertexGPUBuffer->GetRenderID(), 0, vertexSize);

    if (createInfo.pIndexData) {
    #if defined(ENG_DEBUG)
        const std::string indexBuffName = fmt::format("{}_{}", m_name.CStr(), "INDEX_BUFF");
    #else
        const char* indexBuffName = "";
    #endif

        m_pIndexGPUBuffer = memBufferMng.RegisterBuffer(indexBuffName);
        ENG_ASSERT(m_pIndexGPUBuffer, "Failed to register \'{}\' index buffer", indexBuffName.c_str());
        
        MemoryBufferCreateInfo indexBuffCreateInfo = {};
        indexBuffCreateInfo.type          = MemoryBufferType::TYPE_INDEX_BUFFER;
        indexBuffCreateInfo.creationFlags = BUFFER_CREATION_FLAG_ZERO;
        indexBuffCreateInfo.pData         = createInfo.pIndexData;
        indexBuffCreateInfo.dataSize      = createInfo.indexDataSize;
        indexBuffCreateInfo.elementSize   = createInfo.indexSize;
        
        m_pIndexGPUBuffer->Create(indexBuffCreateInfo);
        ENG_ASSERT(m_pIndexGPUBuffer->IsValid(), "Failed to create \'{}\' index buffer", indexBuffName.c_str());
    
        glVertexArrayElementBuffer(m_vaoRenderID, m_pIndexGPUBuffer->GetRenderID());
    }

    return true;
}


void MeshObj::Destroy() noexcept
{
    if (!IsValid()) {
        return;
    }

    m_name = "";

    m_pVertexGPUBuffer->Destroy();
    MemoryBufferManager::GetInstance().UnregisterBuffer(m_pVertexGPUBuffer);
    m_pVertexGPUBuffer = nullptr;

    if (m_pIndexGPUBuffer) {
        m_pIndexGPUBuffer->Destroy();
        MemoryBufferManager::GetInstance().UnregisterBuffer(m_pIndexGPUBuffer);
        m_pIndexGPUBuffer = nullptr;
    }
    
    m_ID.Invalidate();

    glDeleteVertexArrays(1, &m_vaoRenderID);
    m_vaoRenderID = 0;
}


void MeshObj::Bind() const noexcept
{
    ENG_ASSERT(IsValid(), "Mesh object \'{}\' is invalid", m_name.CStr());
    glBindVertexArray(m_vaoRenderID);
}


bool MeshObj::IsValid() const noexcept
{
    return m_ID.IsValid() && m_vaoRenderID != 0 && m_pVertexGPUBuffer && m_pVertexGPUBuffer->IsValid();
}


MeshManager &MeshManager::GetInstance() noexcept
{
    ENG_ASSERT(engIsMeshManagerInitialized(), "Mesh manager is not initialized");
    return *g_pMeshMng;
}


MeshManager::~MeshManager()
{
    Terminate();
}


MeshObj* MeshManager::RegisterMeshObj(ds::StrID name) noexcept
{
    MeshObj* pCachedMeshObj = GetMeshObjByName(name);
    if (pCachedMeshObj != nullptr) {
        return pCachedMeshObj;
    }

    ENG_ASSERT(m_nextAllocatedID.Value() < m_meshObjStorage.size() - 1, "Mesh objects storage overflow");

    const MeshID meshID = AllocateMeshID();
    const uint64_t index = meshID.Value();

    MeshObj* pMeshObj = &m_meshObjStorage[index];

    ENG_ASSERT(!pMeshObj->IsValid(), "Valid mesh object was returned during registration");

    pMeshObj->m_name = name;
    pMeshObj->m_ID = meshID;

    m_meshStorageIndexToNameVector[index] = name;
    m_meshNameToStorageIndexMap[name] = index;

    return pMeshObj;
}


void MeshManager::UnregisterMeshObj(ds::StrID name)
{
    MeshObj* pMeshObj = GetMeshObjByName(name);

    UnregisterMeshObj(pMeshObj);
}


void MeshManager::UnregisterMeshObj(MeshObj* pObj)
{
    if (!pObj) {
        return;
    }

    if (pObj->IsValid()) {
        ENG_LOG_WARN("Unregistration of mesh object \'{}\' while it's steel valid. Prefer to mesh objects manually", pObj->GetName().CStr());
        pObj->Destroy();
    }

    DeallocateMeshID(pObj->m_ID);

    const uint64_t index = pObj->m_ID.Value();

    m_meshStorageIndexToNameVector[index] = "__EMPTY__";
    m_meshNameToStorageIndexMap.erase(pObj->m_name);

    pObj->m_name = "";
    pObj->m_ID.Invalidate();
}


MeshObj *MeshManager::GetMeshObjByName(ds::StrID name) noexcept
{
    const auto indexIt = m_meshNameToStorageIndexMap.find(name);
    return indexIt != m_meshNameToStorageIndexMap.cend() ? &m_meshObjStorage[indexIt->second] : nullptr;
}


bool MeshManager::Init() noexcept
{
    if (IsInitialized()) {
        return true;
    }

    m_meshObjStorage.resize(MAX_MESH_OBJ_COUNT);
    m_meshStorageIndexToNameVector.resize(MAX_MESH_OBJ_COUNT, "__EMPTY__");
    m_meshNameToStorageIndexMap.reserve(MAX_MESH_OBJ_COUNT);

    m_nextAllocatedID = MeshID(0);

    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &G_MAX_VERTEX_ATTRIBS_COUNT);

    m_isInitialized = true;

    return true;
}


void MeshManager::Terminate() noexcept
{
    m_meshObjStorage.clear();

    m_meshStorageIndexToNameVector.clear();
    m_meshNameToStorageIndexMap.clear();

    m_meshIDFreeList.clear();
    m_nextAllocatedID = MeshID(0);

    m_isInitialized = false;
}


MeshID MeshManager::AllocateMeshID() noexcept
{
    if (m_meshIDFreeList.empty()) {
        const MeshID meshID = m_nextAllocatedID;
        m_nextAllocatedID = MeshID(m_nextAllocatedID.Value() + 1);

        return meshID;
    }

    const MeshID meshID = m_meshIDFreeList.front();
    m_meshIDFreeList.pop_front();
        
    return meshID;
}


void MeshManager::DeallocateMeshID(MeshID ID) noexcept
{
    if (ID < m_nextAllocatedID && std::find(m_meshIDFreeList.cbegin(), m_meshIDFreeList.cend(), ID) == m_meshIDFreeList.cend()) {
        m_meshIDFreeList.emplace_back(ID);
    }
}


bool engInitMeshManager() noexcept
{
    ENG_ASSERT(engIsMemoryBufferManagerInitialized(), "Memory buffers manager must be initialized before mesh manager");

    if (engIsMeshManagerInitialized()) {
        ENG_LOG_WARN("Mesh manager is already initialized!");
        return true;
    }

    g_pMeshMng = std::unique_ptr<MeshManager>(new MeshManager);

    if (!g_pMeshMng) {
        ENG_ASSERT_FAIL("Failed to allocate memory for mesh manager");
        return false;
    }

    if (!g_pMeshMng->Init()) {
        ENG_ASSERT_FAIL("Failed to initialized mesh manager");
        return false;
    }

    return true;
}


void engTerminateMeshManager() noexcept
{
    ENG_ASSERT(engIsMemoryBufferManagerInitialized(), "Memory buffers manager must be still initialized while mesh manager terminating");

    g_pMeshMng = nullptr;
}


bool engIsMeshManagerInitialized() noexcept
{
    return g_pMeshMng && g_pMeshMng->IsInitialized();
}

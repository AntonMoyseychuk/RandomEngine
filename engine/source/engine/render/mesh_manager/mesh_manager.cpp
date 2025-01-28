#include "pch.h"
#include "mesh_manager.h"

#include "render/platform/OpenGL/opengl_driver.h"

static std::unique_ptr<MeshManager> pMeshMngInst = nullptr;
static std::unique_ptr<MeshDataManager> pMeshDataMngInst = nullptr;

static MemoryBufferManager* pMemBuffMngInst = nullptr;

static constexpr uint64_t MAX_MESH_OBJ_COUNT = 8192;
static constexpr uint64_t MAX_GPU_BUFF_DATA_COUNT = 8192;
static constexpr uint64_t MAX_VERT_BUFF_LAYOUT_COUNT = 8192;


static uint64_t amHash(const MeshVertexLayoutCreateInfo& layoutCreateInfo) noexcept
{
    ds::HashBuilder builder;

    for (uint64_t i = 0; i < layoutCreateInfo.vertexAttribDescsCount; ++i) {
        const MeshVertexAttribDesc& attribDesc = layoutCreateInfo.pVertexAttribDescs[i];

        builder.AddValue(attribDesc.offset);
        builder.AddValue(attribDesc.dataType);
        builder.AddValue(attribDesc.index);
        builder.AddValue(attribDesc.elementsCount);
        builder.AddValue(attribDesc.isNormalized);
    }

    return builder.Value();
}


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
            return GL_NONE;
    }
}



bool MeshVertexLayout::IsValid() const noexcept
{
    return m_hash != UINT64_MAX && m_ID.IsValid() && HasActiveAttributes();
}


uint32_t MeshVertexLayout::GetAttribOffset(uint64_t i) const noexcept
{
    ENG_ASSERT(i < m_activeAttribsCount, "Vertex layout out of active attribs range: {}", i);
    return m_layout[i].offset;
}


MeshVertexAttribDataType MeshVertexLayout::GetAttribDataType(uint64_t i) const noexcept
{
    ENG_ASSERT(i < m_activeAttribsCount, "Vertex layout out of active attribs range: {}", i);
    return static_cast<MeshVertexAttribDataType>(m_layout[i].dataType);
}


uint32_t MeshVertexLayout::GetAttribIndex(uint64_t i) const noexcept
{
    ENG_ASSERT(i < m_activeAttribsCount, "Vertex layout out of active attribs range: {}", i);
    return m_layout[i].index;
}


uint32_t MeshVertexLayout::GetAttribElementCount(uint64_t i) const noexcept
{
    ENG_ASSERT(i < m_activeAttribsCount, "Vertex layout out of active attribs range: {}", i);
    return m_layout[i].elementsCount;
}

bool MeshVertexLayout::IsAttribNormalized(uint64_t i) const noexcept
{
    ENG_ASSERT(i < m_activeAttribsCount, "Vertex layout out of active attribs range: {}", i);
    return m_layout[i].isNormalized;
}


bool MeshVertexLayout::IsAttribActive(uint64_t i) const noexcept
{
    ENG_ASSERT(i < m_layout.size(), "Vertex layout out of range: {}", i);
    return m_layout[i].isActive;
}


MeshVertexLayout::~MeshVertexLayout()
{
    Destroy();
}


void MeshVertexLayout::Create(const MeshVertexLayoutCreateInfo &createInfo) noexcept
{
    ENG_ASSERT(!IsValid(), "Attempt to create already valid mesh vertex layout (ID: {})", m_ID.Value());
    ENG_ASSERT(m_ID.IsValid(), "Mesh vertex layout ID is invalid. You must initialize only layouts which were returned by MeshDataManager");

    ENG_ASSERT(createInfo.pVertexAttribDescs, "pVertexAttribDescs is nullptr");
    ENG_ASSERT(createInfo.vertexAttribDescsCount >= 1 && createInfo.vertexAttribDescsCount <= MAX_VERTEX_ATTRIBS_COUNT, 
        "vertexAttribDescsCount must be at least 1 and less or equal {}", MAX_VERTEX_ATTRIBS_COUNT);

    std::bitset<MAX_VERTEX_ATTRIBS_COUNT> busyIndexes = {};

    for (uint64_t i = 0; i < createInfo.vertexAttribDescsCount; ++i) {
        const MeshVertexAttribDesc& desc = createInfo.pVertexAttribDescs[i];

        ENG_ASSERT(desc.dataType < MeshVertexAttribDataType::TYPE_COUNT, "Invalid vertex attribute {} data type", i);
        ENG_ASSERT(desc.index < MAX_VERTEX_ATTRIBS_COUNT, "Invalid vertex attribute {} index {}", i, desc.index);
        ENG_ASSERT(busyIndexes.test(desc.index) == false, "Vertex attribute {} index {} is already busy", i, desc.index);
        ENG_ASSERT(desc.elementsCount >= 1 && desc.elementsCount <= MAX_VERTEX_ATTRIB_ELEMENTS_COUNT, "Invalid vertex attribute {} elements count {}", i, desc.elementsCount);

        VertexAttribDescInternal& layoutAttribDesc = m_layout[desc.index];
        layoutAttribDesc.offset = desc.offset;
        layoutAttribDesc.dataType = static_cast<uint32_t>(desc.dataType);
        layoutAttribDesc.index = desc.index;
        layoutAttribDesc.elementsCount = desc.elementsCount;
        layoutAttribDesc.isNormalized = desc.isNormalized;
        layoutAttribDesc.isActive = true;

        busyIndexes.set(desc.index);
        ++m_activeAttribsCount;
    }
}


void MeshVertexLayout::Destroy() noexcept
{
    memset(m_layout.data(), 0, sizeof(m_layout));
    m_activeAttribsCount = 0;
}


MeshGPUBufferData::~MeshGPUBufferData()
{
    Destroy();
}


const MemoryBuffer& MeshGPUBufferData::GetVertexBuffer() const noexcept
{
    ENG_ASSERT(IsVertexBufferValid(), "Mesh GPU vertex buffer is invalid");
    return *m_pVertexGPUBuffer;
}


const MemoryBuffer& MeshGPUBufferData::GetIndexBuffer() const noexcept
{
    ENG_ASSERT(IsIndexBufferValid(), "Mesh GPU index buffer is invalid");
    return *m_pIndexGPUBuffer;
}


bool MeshGPUBufferData::IsVertexBufferValid() const noexcept
{
    return m_pVertexGPUBuffer && m_pVertexGPUBuffer->IsValid();
}


bool MeshGPUBufferData::IsIndexBufferValid() const noexcept
{
    return m_pIndexGPUBuffer && m_pIndexGPUBuffer->IsValid();
}


bool MeshGPUBufferData::IsValid() const noexcept
{
    return IsVertexBufferValid() && IsIndexBufferValid();
}


bool MeshGPUBufferData::Create(const MeshGPUBufferDataCreateInfo& createInfo) noexcept
{
    ENG_ASSERT(!IsValid(), "Attempt to create already valid mesh GPU buffer data: {}", m_name.CStr());
    ENG_ASSERT(m_ID.IsValid(), "Mesh ID is invalid. You must initialize only mesh objects which were returned by MeshGPUBufferData");

    ENG_ASSERT(createInfo.pVertexData, "Mesh GPU buffer data \'{}\' createInfo.pVertexData is nullptr", m_name.CStr());
    ENG_ASSERT(createInfo.vertexDataSize > 0, "Mesh GPU buffer data \'{}\' createInfo.vertexDataSize is zero", m_name.CStr());
    ENG_ASSERT(createInfo.vertexSize > 0, "Mesh GPU buffer data \'{}\' createInfo.vertexSize is zero", m_name.CStr());
    ENG_ASSERT(createInfo.vertexDataSize % createInfo.vertexSize == 0, 
        "Mesh GPU buffer data \'{}\' createInfo.vertexDataSize must be multiple of createInfo.vertexSize", m_name.CStr());

    ENG_ASSERT(createInfo.pIndexData, "Mesh GPU buffer data \'{}\' createInfo.pIndexData is nullptr", m_name.CStr());
    ENG_ASSERT(createInfo.indexDataSize > 0, "Mesh GPU buffer data \'{}\' createInfo.indexDataSize is zero", m_name.CStr());
    ENG_ASSERT(createInfo.indexSize > 0, "Mesh GPU buffer data \'{}\' createInfo.indexSize is zero", m_name.CStr());
    ENG_ASSERT(createInfo.indexDataSize % createInfo.indexSize == 0, 
        "Mesh GPU buffer data \'{}\' createInfo.indexDataSize must be multiple of createInfo.indexSize", m_name.CStr());

    ENG_ASSERT(!IsValid(), "Trying to recreate already valid mesh GPU buffer data \'{}\'", m_name.CStr());

    char vertBufName[512] = { 0 };
    sprintf_s(vertBufName, "%s_VERT_BUF", m_name.CStr());

    m_pVertexGPUBuffer = pMemBuffMngInst->RegisterBuffer();
    ENG_ASSERT(m_pVertexGPUBuffer, "Failed to register \'{}\' buffer", vertBufName);
    
    MemoryBufferCreateInfo vertBuffCreateInfo = {};
    vertBuffCreateInfo.type          = MemoryBufferType::TYPE_VERTEX_BUFFER;
    vertBuffCreateInfo.creationFlags = BUFFER_CREATION_FLAG_ZERO;
    vertBuffCreateInfo.pData         = createInfo.pVertexData;
    vertBuffCreateInfo.dataSize      = createInfo.vertexDataSize;
    vertBuffCreateInfo.elementSize   = createInfo.vertexSize;
    
    m_pVertexGPUBuffer->Create(vertBuffCreateInfo);
    ENG_ASSERT(m_pVertexGPUBuffer->IsValid(), "Failed to create \'{}\' buffer", vertBufName);

    m_pVertexGPUBuffer->SetDebugName(vertBufName);
    
    char indexBufName[512] = { 0 };
    sprintf_s(indexBufName, "%s_IDX_BUF", m_name.CStr());

    m_pIndexGPUBuffer = pMemBuffMngInst->RegisterBuffer();
    ENG_ASSERT(m_pIndexGPUBuffer, "Failed to register \'{}\' buffer", indexBufName);
        
    MemoryBufferCreateInfo indexBuffCreateInfo = {};
    indexBuffCreateInfo.type          = MemoryBufferType::TYPE_INDEX_BUFFER;
    indexBuffCreateInfo.creationFlags = BUFFER_CREATION_FLAG_ZERO;
    indexBuffCreateInfo.pData         = createInfo.pIndexData;
    indexBuffCreateInfo.dataSize      = createInfo.indexDataSize;
    indexBuffCreateInfo.elementSize   = createInfo.indexSize;
        
    m_pIndexGPUBuffer->Create(indexBuffCreateInfo);
    ENG_ASSERT(m_pIndexGPUBuffer->IsValid(), "Failed to create \'{}\' buffer", indexBufName);
    
    m_pIndexGPUBuffer->SetDebugName(indexBufName);

    return true;
}


void MeshGPUBufferData::Destroy() noexcept
{
    if (!IsValid()) {
        return;
    }

    m_pVertexGPUBuffer->Destroy();
    m_pIndexGPUBuffer->Destroy();
    
    pMemBuffMngInst->UnregisterBuffer(m_pVertexGPUBuffer);
    pMemBuffMngInst->UnregisterBuffer(m_pIndexGPUBuffer);
}


MeshDataManager& MeshDataManager::GetInstance() noexcept
{
    ENG_ASSERT(engIsMeshDataManagerInitialized(), "Mesh data manager is not initialized");
    return *pMeshDataMngInst;
}


MeshDataManager::~MeshDataManager()
{
    Terminate();
}


MeshVertexLayout* MeshDataManager::RegisterVertexLayout(const MeshVertexLayoutCreateInfo& createInfo) noexcept
{
    const uint64_t createInfoHash = amHash(createInfo);

    ENG_ASSERT(FindVertexLayoutByHash(createInfoHash) == nullptr, "Attempt to register already registred vertex layout");

    ENG_ASSERT(m_nextAllocatedVertLayoutID.Value() < m_vertexLayoutStorage.size() - 1, "Vertex buffer layout storage overflow");

    const MeshVertexLayoutID layoutID = AllocateVertexLayoutID();
    MeshVertexLayout* pLayout = &m_vertexLayoutStorage[layoutID.Value()];

    pLayout->m_hash = createInfoHash;
    pLayout->m_ID = layoutID;
    pLayout->Create(createInfo);

    ENG_ASSERT(pLayout->IsValid(), "Failed to create vertex layout");

    m_vertexLayoutHashToStorageIndexMap[createInfoHash] = layoutID.Value();

    return pLayout;
}


void MeshDataManager::UnregisterVertexLayout(MeshVertexLayout* pLayout) noexcept
{
    if (!pLayout) {
        return;
    }

    if (pLayout->IsValid()) {
        ENG_LOG_WARN("Unregistration of vertex buffer layout \'{}\' while it's steel valid. Prefer to destroy buffers manually", pLayout->m_ID.Value());
        pLayout->Destroy();
    }

    DeallocateVertexLayoutID(pLayout->m_ID);
    m_vertexLayoutHashToStorageIndexMap.erase(pLayout->m_hash);

    pLayout->m_ID.Invalidate();
    pLayout->m_hash = UINT64_MAX;
}


MeshGPUBufferData* MeshDataManager::RegisterGPUBufferData(ds::StrID name) noexcept
{
    ENG_ASSERT(GetGPUBufferDataByName(name) == nullptr, "Attempt to register already registred mesh GPU buffer data: {}", name.CStr());

    const MeshGPUBufferDataID dataID = AllocateGPUBufferDataID();
    const uint64_t index = dataID.Value();

    MeshGPUBufferData* pData = &m_GPUBufferDataStorage[index];

    ENG_ASSERT(!pData->IsValid(), "Valid GPU buffer data was returned during registration");

    pData->m_name = name;
    pData->m_ID = dataID;

    m_GPUBufferDataNameToStorageIndexMap[name] = index;

    return pData;
}


void MeshDataManager::UnregisterGPUBufferData(ds::StrID name) noexcept
{
    MeshGPUBufferData* pData = GetGPUBufferDataByName(name);
    UnregisterGPUBufferData(pData);
}


void MeshDataManager::UnregisterGPUBufferData(MeshGPUBufferData* pData) noexcept
{
    if (!pData) {
        return;
    }

    if (pData->IsValid()) {
        ENG_LOG_WARN("Unregistration of GPU buffer data \'{}\' while it's steel valid. Prefer to destroy buffers manually", pData->m_name.CStr());
        pData->Destroy();
    }

    DeallocateGPUBufferDataID(pData->m_ID);
    m_GPUBufferDataNameToStorageIndexMap.erase(pData->m_name);

    pData->m_name = "_INVALID_";
    pData->m_ID.Invalidate();
}


bool MeshDataManager::Init() noexcept
{
    if (IsInitialized()) {
        return true;
    }

    m_vertexLayoutStorage.resize(MAX_VERT_BUFF_LAYOUT_COUNT);
    m_GPUBufferDataStorage.resize(MAX_GPU_BUFF_DATA_COUNT);

    m_vertexLayoutHashToStorageIndexMap.reserve(MAX_VERT_BUFF_LAYOUT_COUNT);
    m_GPUBufferDataNameToStorageIndexMap.reserve(MAX_GPU_BUFF_DATA_COUNT);

    m_nextAllocatedVertLayoutID = MeshVertexLayoutID(0);
    m_nextAllocatedGPUBufDataID = MeshGPUBufferDataID(0);

    m_isInitialized = true;

    return true;
}


void MeshDataManager::Terminate() noexcept
{
    m_vertexLayoutStorage.clear();
    m_GPUBufferDataStorage.clear();

    m_vertexLayoutHashToStorageIndexMap.clear();
    m_GPUBufferDataNameToStorageIndexMap.clear();

    m_meshVertexLayoutIDFreeList.clear();
    m_meshGPUBufferDataIDFreeList.clear();
    
    m_nextAllocatedVertLayoutID = MeshVertexLayoutID(0);
    m_nextAllocatedGPUBufDataID = MeshGPUBufferDataID(0);

    m_isInitialized = false;
}


MeshVertexLayout *MeshDataManager::FindVertexLayoutByHash(uint64_t hash) noexcept
{
    const auto indexIt = m_vertexLayoutHashToStorageIndexMap.find(hash);
    return indexIt != m_vertexLayoutHashToStorageIndexMap.cend() ? &m_vertexLayoutStorage[indexIt->second] : nullptr;
}


MeshVertexLayoutID MeshDataManager::AllocateVertexLayoutID() noexcept
{
    if (m_meshVertexLayoutIDFreeList.empty()) {
        ENG_ASSERT(m_nextAllocatedVertLayoutID.Value() < m_vertexLayoutStorage.size() - 1, "Vertex buffer layout storage overflow");

        const MeshVertexLayoutID layoutID = m_nextAllocatedVertLayoutID;
        m_nextAllocatedVertLayoutID = BufferID(m_nextAllocatedVertLayoutID.Value() + 1);

        return layoutID;
    }

    const MeshVertexLayoutID layoutID = m_meshVertexLayoutIDFreeList.front();
    m_meshVertexLayoutIDFreeList.pop_front();
        
    return layoutID;
}


void MeshDataManager::DeallocateVertexLayoutID(MeshVertexLayoutID ID) noexcept
{
    if (ID < m_nextAllocatedVertLayoutID && 
        std::find(m_meshVertexLayoutIDFreeList.cbegin(), m_meshVertexLayoutIDFreeList.cend(), ID) == m_meshVertexLayoutIDFreeList.cend())
    {
        m_meshVertexLayoutIDFreeList.emplace_back(ID);
    }
}


MeshGPUBufferDataID MeshDataManager::AllocateGPUBufferDataID() noexcept
{
    if (m_meshGPUBufferDataIDFreeList.empty()) {
        ENG_ASSERT(m_nextAllocatedVertLayoutID.Value() < m_vertexLayoutStorage.size() - 1, "Vertex buffer layout storage overflow");

        const MeshGPUBufferDataID gpuBuffDataID = m_nextAllocatedGPUBufDataID;
        m_nextAllocatedGPUBufDataID = MeshGPUBufferDataID(m_nextAllocatedGPUBufDataID.Value() + 1);

        return gpuBuffDataID;
    }

    const MeshGPUBufferDataID gpuBuffDataID = m_meshGPUBufferDataIDFreeList.front();
    m_meshGPUBufferDataIDFreeList.pop_front();
        
    return gpuBuffDataID;
}


void MeshDataManager::DeallocateGPUBufferDataID(MeshGPUBufferDataID ID) noexcept
{
    if (ID < m_nextAllocatedGPUBufDataID && 
        std::find(m_meshGPUBufferDataIDFreeList.cbegin(), m_meshGPUBufferDataIDFreeList.cend(), ID) == m_meshGPUBufferDataIDFreeList.cend())
    {
        m_meshGPUBufferDataIDFreeList.emplace_back(ID);
    }
}


MeshGPUBufferData* MeshDataManager::GetGPUBufferDataByName(ds::StrID name) noexcept
{
    const auto indexIt = m_GPUBufferDataNameToStorageIndexMap.find(name);
    return indexIt != m_GPUBufferDataNameToStorageIndexMap.cend() ? &m_GPUBufferDataStorage[indexIt->second] : nullptr;
}


bool engInitMeshDataManager() noexcept
{
    if (engIsMeshDataManagerInitialized()) {
        ENG_LOG_WARN("Mesh data manager is already initialized!");
        return true;
    }

    pMeshDataMngInst = std::unique_ptr<MeshDataManager>(new MeshDataManager);

    if (!pMeshDataMngInst) {
        ENG_ASSERT_FAIL("Failed to allocate memory for mesh data manager");
        return false;
    }

    if (!pMeshDataMngInst->Init()) {
        ENG_ASSERT_FAIL("Failed to initialized mesh data manager");
        return false;
    }

    return true;
}


void engTerminateMeshDataManager() noexcept
{
    pMeshDataMngInst = nullptr;
}


bool engIsMeshDataManagerInitialized() noexcept
{
    return pMeshDataMngInst && pMeshDataMngInst->IsInitialized();
}


MeshObj::~MeshObj()
{
    Destroy();
}


MeshObj::MeshObj(MeshObj&& other) noexcept
{
    std::swap(m_vaoRenderID, other.m_vaoRenderID);
    std::swap(m_ID, other.m_ID);
    std::swap(m_name, other.m_name);
    std::swap(m_pVertexLayout, other.m_pVertexLayout);
    std::swap(m_pBufferData, other.m_pBufferData);
}


MeshObj& MeshObj::operator=(MeshObj&& other) noexcept
{
    Destroy();

    std::swap(m_vaoRenderID, other.m_vaoRenderID);
    std::swap(m_ID, other.m_ID);
    std::swap(m_name, other.m_name);
    std::swap(m_pVertexLayout, other.m_pVertexLayout);
    std::swap(m_pBufferData, other.m_pBufferData);

    return *this;
}


bool MeshObj::Create(MeshVertexLayout* pLayoutDesc, MeshGPUBufferData* pMeshData) noexcept
{
    ENG_ASSERT(!IsValid(), "Attempt to create already valid mesh object: {}", m_name.CStr());
    ENG_ASSERT(m_ID.IsValid(), "Mesh object \'{}\' ID is invalid. You must initialize only mesh objects which were returned by MeshManager", m_name.CStr());

    ENG_ASSERT(pLayoutDesc && pLayoutDesc->IsValid(), "Mesh object \'{}\' invalid GPU buffer data", m_name.CStr());
    ENG_ASSERT(pMeshData && pMeshData->IsValid(), "Mesh object \'{}\' invalid vertex layout", m_name.CStr());

    m_pVertexLayout = pLayoutDesc;
    m_pBufferData = pMeshData;

    glCreateVertexArrays(1, &m_vaoRenderID);

    for (uint32_t i = 0; i < m_pVertexLayout->GetActiveAttribsCount(); ++i) {
        const uint32_t index                    = m_pVertexLayout->GetAttribIndex(i);
        const MeshVertexAttribDataType dataType = m_pVertexLayout->GetAttribDataType(i);
        const uint32_t elementsCount            = m_pVertexLayout->GetAttribElementCount(i);
        const uint32_t offset                   = m_pVertexLayout->GetAttribOffset(i);
        const bool isNormalized                 = m_pVertexLayout->IsAttribNormalized(i);

        glVertexArrayAttribBinding(m_vaoRenderID, index, 0);

        const GLenum type = GetAttribDataGLType(dataType);
        glVertexArrayAttribFormat(m_vaoRenderID, index, elementsCount, type, isNormalized, offset);
        
        glEnableVertexArrayAttrib(m_vaoRenderID, index);
    }

    const MemoryBuffer& vertBuf = m_pBufferData->GetVertexBuffer();
    glVertexArrayVertexBuffer(m_vaoRenderID, 0, vertBuf.GetRenderID(), 0, vertBuf.GetElementSize());

    const MemoryBuffer& indexBuf = m_pBufferData->GetIndexBuffer();
    glVertexArrayElementBuffer(m_vaoRenderID, indexBuf.GetRenderID());

    return true;
}


void MeshObj::Destroy() noexcept
{
    if (!IsValid()) {
        return; 
    }

    glDeleteVertexArrays(1, &m_vaoRenderID);
    m_vaoRenderID = 0;
    m_ID.Invalidate();
    m_name = "_INVALID_";
    m_pVertexLayout = nullptr;
    m_pBufferData = nullptr;
}


void MeshObj::Bind() const noexcept
{
    ENG_ASSERT(IsValid(), "Mesh object \'{}\' is invalid", m_name.CStr());
    glBindVertexArray(m_vaoRenderID);
}


bool MeshObj::IsVertexLayoutValid() const noexcept
{
    return m_pVertexLayout && m_pVertexLayout->IsValid();
}


bool MeshObj::IsGPUBufferDataValid() const noexcept
{
    return m_pBufferData && m_pBufferData->IsValid();
}


bool MeshObj::IsValid() const noexcept
{
    return m_vaoRenderID != 0 && m_ID.IsValid() && IsVertexLayoutValid() && IsGPUBufferDataValid();
}


MeshManager& MeshManager::GetInstance() noexcept
{
    ENG_ASSERT(engIsMeshManagerInitialized(), "Mesh manager is not initialized");
    return *pMeshMngInst;
}


MeshManager::~MeshManager()
{
    Terminate();
}


MeshObj* MeshManager::RegisterMeshObj(ds::StrID name) noexcept
{
    ENG_ASSERT(GetMeshObjByName(name) == nullptr, "Attempt to create already valid mesh object: {}", name.CStr());

    ENG_ASSERT(m_nextAllocatedID.Value() < m_meshObjStorage.size() - 1, "Mesh objects storage overflow");

    const MeshID meshID = AllocateMeshID();
    const uint64_t index = meshID.Value();

    MeshObj* pMeshObj = &m_meshObjStorage[index];

    ENG_ASSERT(!pMeshObj->IsValid(), "Valid mesh object was returned during registration");

    pMeshObj->m_name = name;
    pMeshObj->m_ID = meshID;

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

    m_meshNameToStorageIndexMap.erase(pObj->m_name);

    pObj->m_name = "_INVALID_";
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

    if (!engInitMeshDataManager()) {
        return false;
    }

    m_meshObjStorage.resize(MAX_MESH_OBJ_COUNT);
    m_meshNameToStorageIndexMap.reserve(MAX_MESH_OBJ_COUNT);

    m_nextAllocatedID = MeshID(0);

    int32_t maxVertexAttribsCount = 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribsCount);
    ENG_ASSERT(static_cast<uint64_t>(maxVertexAttribsCount) <= MeshVertexLayout::MAX_VERTEX_ATTRIBS_COUNT, 
        "Invalid max mesh vertex layout attribs count constanst value");

    m_isInitialized = true;

    return true;
}


void MeshManager::Terminate() noexcept
{
    m_meshObjStorage.clear();

    m_meshNameToStorageIndexMap.clear();

    m_meshIDFreeList.clear();
    m_nextAllocatedID = MeshID(0);

    engTerminateMeshDataManager();

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

    pMemBuffMngInst = &MemoryBufferManager::GetInstance();
    pMeshMngInst = std::unique_ptr<MeshManager>(new MeshManager);

    if (!pMeshMngInst) {
        ENG_ASSERT_FAIL("Failed to allocate memory for mesh manager");
        return false;
    }

    if (!pMeshMngInst->Init()) {
        ENG_ASSERT_FAIL("Failed to initialized mesh manager");
        return false;
    }

    return true;
}


void engTerminateMeshManager() noexcept
{
    ENG_ASSERT(engIsMemoryBufferManagerInitialized(), "Memory buffers manager must be still initialized while mesh manager terminating");

    pMeshMngInst = nullptr;
    pMemBuffMngInst = nullptr;
}


bool engIsMeshManagerInitialized() noexcept
{
    return pMeshMngInst && pMeshMngInst->IsInitialized();
}

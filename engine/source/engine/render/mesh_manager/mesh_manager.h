#pragma once

#include "render/mem_manager/buffer_manager.h"

#include "utils/data_structures/strid.h"


enum class MeshVertexAttribDataType : uint8_t
{
    TYPE_UNSIGNED_BYTE,
    TYPE_BYTE,
    TYPE_UNSIGNED_SHORT,
    TYPE_SHORT,
    TYPE_UNSIGNED_INT,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_HALF_FLOAT,
    TYPE_DOUBLE,
    
    TYPE_COUNT
};


struct MeshVertexAttribDesc
{
    uint16_t                 offset;
    MeshVertexAttribDataType dataType;
    uint8_t                  index;
    uint8_t                  elementsCount;
    bool                     isNormalized;
};


struct MeshVertexLayoutCreateInfo
{
    const MeshVertexAttribDesc* pVertexAttribDescs;
    uint64_t                    vertexAttribDescsCount;
};


using MeshVertexLayoutID = BaseID<uint32_t>;


class MeshVertexLayout
{
    friend class MeshDataManager;

public:
    MeshVertexLayout() = default;
    ~MeshVertexLayout();

    MeshVertexAttribDataType GetAttribDataType(uint64_t i) const noexcept;
    uint32_t GetAttribIndex(uint64_t i) const noexcept;
    uint32_t GetAttribOffset(uint64_t i) const noexcept;
    uint32_t GetAttribElementCount(uint64_t i) const noexcept;
    bool IsAttribNormalized(uint64_t i) const noexcept;
    bool IsAttribActive(uint64_t i) const noexcept;    

    MeshVertexLayoutID GetID() const noexcept { return m_ID; }
    uint64_t Hash() const noexcept { return m_hash; }

    uint32_t GetActiveAttribsCount() const noexcept { return m_activeAttribsCount; }

    bool HasActiveAttributes() const noexcept { return m_activeAttribsCount > 0; }
    bool IsValid() const noexcept;

private:
    void Create(const MeshVertexLayoutCreateInfo& createInfo) noexcept;
    void Destroy() noexcept;

public:
    static inline constexpr uint64_t MAX_VERTEX_ATTRIBS_COUNT = 16ULL;
    static inline constexpr uint64_t MAX_VERTEX_ATTRIB_ELEMENTS_COUNT = 4ULL;

private:
    static constexpr size_t BITS_PER_INDEX = [](uint64_t maxIndexValue) -> size_t
    {
        size_t bits = 0;

        do {
            ++bits;
            maxIndexValue /= 2;
        } while (maxIndexValue >= 2);

        return bits;
    }(MAX_VERTEX_ATTRIBS_COUNT);

    struct VertexAttribDescInternal
    {
        uint32_t offset : 16;
        uint32_t dataType : 4;
        uint32_t index : BITS_PER_INDEX;
        uint32_t elementsCount : 3;
        uint32_t isNormalized : 1;
        uint32_t isActive : 1;
    };

    static_assert(sizeof(VertexAttribDescInternal) == sizeof(uint32_t));

    std::array<VertexAttribDescInternal, MAX_VERTEX_ATTRIBS_COUNT> m_layout;
    uint64_t m_hash = UINT64_MAX;
    MeshVertexLayoutID m_ID;
    uint32_t m_activeAttribsCount = 0;
};


struct MeshGPUBufferDataCreateInfo
{
    const void*                 pVertexData;
    uint64_t                    vertexDataSize;
    uint64_t                    vertexSize;

    const void*                 pIndexData;
    uint64_t                    indexDataSize;
    uint64_t                    indexSize;
};


using MeshGPUBufferDataID = BaseID<uint64_t>;


class MeshGPUBufferData
{
    friend class MeshDataManager;

public:
    MeshGPUBufferData() = default;
    ~MeshGPUBufferData();

    bool Create(const MeshGPUBufferDataCreateInfo& createInfo) noexcept;
    void Destroy() noexcept;

    ds::StrID GetName() const noexcept { return m_name; }
    MeshGPUBufferDataID GetID() const noexcept { return m_ID; }

    const MemoryBuffer& GetVertexBuffer() const noexcept;
    const MemoryBuffer& GetIndexBuffer() const noexcept;

    bool IsVertexBufferValid() const noexcept;
    bool IsIndexBufferValid() const noexcept;

    bool IsValid() const noexcept;

private:
    MeshGPUBufferDataID m_ID;
    ds::StrID           m_name = "_INVALID_";

    MemoryBuffer*       m_pVertexGPUBuffer = nullptr;
    MemoryBuffer*       m_pIndexGPUBuffer = nullptr;
};


class MeshDataManager
{
    friend bool engInitMeshDataManager() noexcept;
    friend void engTerminateMeshDataManager() noexcept;
    friend bool engIsMeshDataManagerInitialized() noexcept;

public:
    static MeshDataManager& GetInstance() noexcept;

public:
    ~MeshDataManager();

    MeshDataManager(const MeshDataManager& other) = delete;
    MeshDataManager& operator=(const MeshDataManager& other) = delete;
    MeshDataManager(MeshDataManager&& other) noexcept = delete;
    MeshDataManager& operator=(MeshDataManager&& other) noexcept = delete;

    MeshVertexLayout* RegisterVertexLayout(const MeshVertexLayoutCreateInfo& createInfo) noexcept;
    void UnregisterVertexLayout(MeshVertexLayout* pLayout) noexcept;

    MeshGPUBufferData* RegisterGPUBufferData(ds::StrID name) noexcept;
    void UnregisterGPUBufferData(ds::StrID name) noexcept;
    void UnregisterGPUBufferData(MeshGPUBufferData* pData) noexcept;

    MeshGPUBufferData* GetGPUBufferDataByName(ds::StrID name) noexcept;

    bool IsInitialized() const noexcept { return m_isInitialized; }

private:
    MeshDataManager() = default;

    bool Init() noexcept;
    void Terminate() noexcept;

    MeshVertexLayout* FindVertexLayoutByHash(uint64_t hash) noexcept;

    MeshVertexLayoutID AllocateVertexLayoutID() noexcept;
    void DeallocateVertexLayoutID(MeshVertexLayoutID ID) noexcept;

    MeshGPUBufferDataID AllocateGPUBufferDataID() noexcept;
    void DeallocateGPUBufferDataID(MeshGPUBufferDataID ID) noexcept;

private:
    std::vector<MeshVertexLayout> m_vertexLayoutStorage;
    std::vector<MeshGPUBufferData> m_GPUBufferDataStorage;

    std::unordered_map<uint64_t, uint64_t> m_vertexLayoutHashToStorageIndexMap;
    std::unordered_map<ds::StrID, uint64_t> m_GPUBufferDataNameToStorageIndexMap;

    std::deque<MeshVertexLayoutID> m_meshVertexLayoutIDFreeList;
    std::deque<MeshGPUBufferDataID> m_meshGPUBufferDataIDFreeList;
    
    MeshVertexLayoutID m_nextAllocatedVertLayoutID = MeshVertexLayoutID{0};
    MeshGPUBufferDataID m_nextAllocatedGPUBufDataID = MeshGPUBufferDataID{0};

    bool m_isInitialized = false;
};


using MeshID = BaseID<uint32_t>;


class MeshObj
{
    friend class MeshManager;

public:
    MeshObj() = default;
    ~MeshObj();

    MeshObj(const MeshObj& other) = delete;
    MeshObj& operator=(const MeshObj& other) = delete;
    
    MeshObj(MeshObj&& other) noexcept;
    MeshObj& operator=(MeshObj&& other) noexcept;

    bool Create(MeshVertexLayout* pLayoutDesc, MeshGPUBufferData* pMeshData) noexcept;
    void Destroy() noexcept;

    void Bind() const noexcept;

    bool IsVertexLayoutValid() const noexcept;
    bool IsGPUBufferDataValid() const noexcept;

    bool IsValid() const noexcept;

    ds::StrID GetName() const noexcept { return m_name; }
    MeshID GetID() const noexcept { return m_ID; }

private:
    uint32_t m_vaoRenderID = 0;
    MeshID m_ID;

    ds::StrID m_name = "_INVALID_";

    MeshVertexLayout* m_pVertexLayout = nullptr;
    MeshGPUBufferData* m_pBufferData = nullptr;
};


class MeshManager
{
    friend bool engInitMeshManager() noexcept;
    friend void engTerminateMeshManager() noexcept;
    friend bool engIsMeshManagerInitialized() noexcept;

public:
    static MeshManager& GetInstance() noexcept;

public:
    MeshManager(const MeshManager& other) = delete;
    MeshManager& operator=(const MeshManager& other) = delete;
    MeshManager(MeshManager&& other) noexcept = delete;
    MeshManager& operator=(MeshManager&& other) noexcept = delete;

    ~MeshManager();

    MeshObj* RegisterMeshObj(ds::StrID name) noexcept;
    void UnregisterMeshObj(ds::StrID name);
    void UnregisterMeshObj(MeshObj* pObj);

    MeshObj* GetMeshObjByName(ds::StrID name) noexcept;

    bool IsInitialized() const noexcept { return m_isInitialized; }

private:
    MeshManager() = default;

    bool Init() noexcept;
    void Terminate() noexcept;

    MeshID AllocateMeshID() noexcept;
    void DeallocateMeshID(MeshID ID) noexcept;

private:
    std::vector<MeshObj> m_meshObjStorage;
    std::unordered_map<ds::StrID, uint64_t> m_meshNameToStorageIndexMap;

    std::deque<MeshID> m_meshIDFreeList;
    MeshID m_nextAllocatedID = MeshID{0};

    bool m_isInitialized = false;
};


bool engIsMeshDataManagerInitialized() noexcept;


bool engInitMeshManager() noexcept;
void engTerminateMeshManager() noexcept;
bool engIsMeshManagerInitialized() noexcept;
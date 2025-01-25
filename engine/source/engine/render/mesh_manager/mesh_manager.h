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
    
    TYPE_COUNT
};


struct MeshVertexAttribDesc
{
    uint64_t                    offset;
    MeshVertexAttribDataType    dataType;
    uint8_t                     index;
    uint8_t                     elementsCount;
    bool                        isNormalized;
};


struct MeshObjCreateInfo
{
    const MeshVertexAttribDesc* pVertexAttribDescs;
    uint64_t                    vertexAttribDescsCount;

    const void*                 pVertexData;
    uint64_t                    vertexDataSize;

    const void*                 pIndexData;
    uint64_t                    indexDataSize;
    uint64_t                    indexSize;
};


using MeshID = BaseID<uint32_t>;


class MeshObj
{
    friend class MeshManager;

public:
    MeshObj() = default;
    ~MeshObj() { Destroy(); }

    MeshObj(const MeshObj& other) = delete;
    MeshObj& operator=(const MeshObj& other) = delete;
    
    MeshObj(MeshObj&& other) noexcept;
    MeshObj& operator=(MeshObj&& other) noexcept;

    bool Create(const MeshObjCreateInfo& createInfo) noexcept;
    void Destroy() noexcept;

    void Bind() const noexcept;

    bool IsValid() const noexcept;

    ds::StrID GetName() const noexcept { return m_name; }
    MeshID GetID() const noexcept { return m_ID; }

private:
    ds::StrID m_name = "";

    MemoryBuffer* m_pVertexGPUBuffer = nullptr;
    MemoryBuffer* m_pIndexGPUBuffer = nullptr;
    
    MeshID m_ID;
    uint32_t m_vaoRenderID = 0;
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

    std::vector<ds::StrID> m_meshStorageIndexToNameVector;
    std::unordered_map<ds::StrID, uint64_t> m_meshNameToStorageIndexMap;

    std::deque<MeshID> m_meshIDFreeList;
    MeshID m_nextAllocatedID = MeshID{0};

    bool m_isInitialized = false;
};


bool engInitMeshManager() noexcept;
void engTerminateMeshManager() noexcept;
bool engIsMeshManagerInitialized() noexcept;
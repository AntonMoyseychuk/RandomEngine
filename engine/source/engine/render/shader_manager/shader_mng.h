#pragma once

#include "utils/file/file.h"
#include "utils/data_structures/strid.h"

#include "resource_bind.h"

#include "core.h"

#include <deque>

#include <limits>
#include <cstdint>


enum class ShaderStageType : uint32_t
{
    VERTEX,
    PIXEL,
    COUNT
};


struct ShaderStageCreateInfo
{
    const char* pSourceCode;
    const char** pDefines;
    const char* pIncludeParentPath;
    uint32_t codeSize;
    uint32_t definesCount;
    ShaderStageType type;
};


struct ShaderProgramCreateInfo
{
    const ShaderStageCreateInfo** pStageCreateInfos;
    size_t stageCreateInfosCount;

    ds::StrID dbgName = "";
};


class ShaderProgram
{
    friend class ShaderManager;

public:
    ShaderProgram(const ShaderProgram& other) = delete;
    ShaderProgram& operator=(const ShaderProgram& other) = delete;

    ShaderProgram() = default;
    ~ShaderProgram() { Destroy(); }

    ShaderProgram(ShaderProgram&& other) noexcept;
    ShaderProgram& operator=(ShaderProgram&& other) noexcept;

    void Bind() const noexcept;

    ENG_DEPRECATED("Prefer to use uniform buffers")
    void SetLocalSrvBool(const ShaderResourceBindStruct<ShaderResourceType::TYPE_BOOL>& bind, bool value) noexcept;
    ENG_DEPRECATED("Prefer to use uniform buffers")
    void SetLocalSrvInt(const ShaderResourceBindStruct<ShaderResourceType::TYPE_INT>& bind, int32_t value) noexcept;
    ENG_DEPRECATED("Prefer to use uniform buffers")
    void SetLocalSrvUInt(const ShaderResourceBindStruct<ShaderResourceType::TYPE_UINT>& bind, uint32_t value) noexcept;
    ENG_DEPRECATED("Prefer to use uniform buffers")
    void SetLocalSrvFloat(const ShaderResourceBindStruct<ShaderResourceType::TYPE_FLOAT>& bind, float value) noexcept;
    ENG_DEPRECATED("Prefer to use uniform buffers")
    void SetLocalSrvDouble(const ShaderResourceBindStruct<ShaderResourceType::TYPE_DOUBLE>& bind, double value) noexcept;

    bool IsValidRenderID() const noexcept { return m_renderID != 0; }
    bool IsValid() const noexcept { return IsValidRenderID(); }
    
    uint64_t Hash() const noexcept;

    uint32_t GetRenderID() const noexcept { return m_renderID; }

private:
    bool Init(const ShaderProgramCreateInfo& createInfo) noexcept;
    void Destroy() noexcept;

    bool GetLinkingStatus() const noexcept;

private:
#if defined(ENG_DEBUG)
    ds::StrID m_dbgName = "";
#endif

    uint32_t m_renderID = 0;
};


using ProgramID = uint64_t;

inline constexpr ProgramID PROGRAM_ID_INVALID = UINT64_MAX;


class ShaderManager
{
    friend bool engInitShaderManager() noexcept;
    friend void engTerminateShaderManager() noexcept;
    friend bool engIsShaderManagerInitialized() noexcept;

public:
    static ShaderManager& GetInstance() noexcept;
    
public:
    ShaderManager(const ShaderManager& other) = delete;
    ShaderManager& operator=(const ShaderManager& other) = delete;

    ~ShaderManager();

    ProgramID RegisterShaderProgram(const ShaderProgramCreateInfo& createInfo) noexcept;
    void UnregisterShaderProgram(ProgramID ID) noexcept;
    
    ShaderProgram* GetShaderProgramByID(ProgramID ID) noexcept;
    
    bool IsValidProgramID(ProgramID ID) const noexcept;

private:
    ShaderManager() = default;

    ShaderManager(ShaderManager&& other) noexcept = default;
    ShaderManager& operator=(ShaderManager&& other) noexcept = default;

    bool Init() noexcept;
    void Terminate() noexcept;

    ProgramID AllocateProgramID() noexcept;
    void DeallocateProgramID(ProgramID ID) noexcept;

    bool IsInitialized() const noexcept;

private:
    struct ShaderCreateInfoHashHasher
    {
        uint64_t operator()(uint64_t hash) const noexcept { return hash; }
    };
    
    std::vector<ShaderProgram> m_shaderProgramsStorage;
    
    std::vector<uint64_t> m_shaderProgramIDToCreateInfoHashVector;
    std::unordered_map<uint64_t, ProgramID, ShaderCreateInfoHashHasher> m_shaderProgramCreateInfoHashToIDMap;

    std::deque<ProgramID> m_programIDFreeList;

    ProgramID m_nextAllocatedID = 0;

    bool m_isInitialized = false;
};


uint64_t amHash(const ShaderStageCreateInfo& stageCreateInfo) noexcept;
uint64_t amHash(const ShaderProgramCreateInfo& programCreateInfo) noexcept;
uint64_t amHash(const ShaderProgram& program) noexcept;


bool engInitShaderManager() noexcept;
void engTerminateShaderManager() noexcept;
bool engIsShaderManagerInitialized() noexcept;
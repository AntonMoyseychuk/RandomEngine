#pragma once

#include "utils/file/file.h"
#include "utils/data_structures/strid.h"

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
    void Unbind() const noexcept;

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
    void UnregisterShaderProgram(const ProgramID& id) noexcept;
    
    ShaderProgram* GetShaderProgramByID(const ProgramID& id) noexcept;
    
    bool IsValidProgramID(const ProgramID& id) const noexcept;

private:
    ShaderManager() = default;

    ShaderManager(ShaderManager&& other) noexcept = default;
    ShaderManager& operator=(ShaderManager&& other) noexcept = default;

    bool Init() noexcept;
    void Terminate() noexcept;

    ProgramID AllocateProgramID() noexcept;
    void DeallocateProgramID(const ProgramID& ID) noexcept;

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
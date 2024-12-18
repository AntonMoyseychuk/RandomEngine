#pragma once

#include "utils/file/file.h"

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
    uint32_t codeSize;
    uint32_t definesCount;
    ShaderStageType type;
};


struct ShaderProgramCreateInfo
{
    const ShaderStageCreateInfo** pStageCreateInfos;
    size_t stageCreateInfosCount;
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

    bool IsValid() const noexcept { return m_id != 0; }
    uint64_t Hash() const noexcept;

private:
    bool Init(const ShaderProgramCreateInfo& createInfo) noexcept;
    void Destroy() noexcept;

    bool GetLinkingStatus() const noexcept;

private:
    uint32_t m_id = 0;
};


class ProgramID
{
    friend class ShaderManager;

public:
    ProgramID() = default;
    ProgramID(uint64_t id) : m_id(id) {}

    operator uint64_t() const noexcept { return m_id; }

    bool IsValid() const noexcept { return m_id != INVALID_ID; }

private:
    ProgramID& operator=(uint64_t newID) noexcept { m_id = newID; return *this; }

    uint64_t Hash() const noexcept { return m_id; }

private:
    static inline constexpr size_t INVALID_ID = std::numeric_limits<uint64_t>::max();

private:
    uint64_t m_id = INVALID_ID;
};


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

private:
    ShaderManager() = default;

    ShaderManager(ShaderManager&& other) noexcept = default;
    ShaderManager& operator=(ShaderManager&& other) noexcept = default;

    bool Init() noexcept;
    void Terminate() noexcept;

    bool IsInitialized() const noexcept;

private:
    struct ProgramIDHasher
    {
        uint64_t operator()(const ProgramID& id) const noexcept { return id.Hash(); }
    };

    std::unordered_map<ProgramID, ShaderProgram, ProgramIDHasher> m_shaderProgramsStorage;

    bool m_isInitialized = false;
};


uint64_t amHash(const ShaderStageCreateInfo& stageCreateInfo) noexcept;
uint64_t amHash(const ShaderProgramCreateInfo& programCreateInfo) noexcept;


bool engInitShaderManager() noexcept;
void engTerminateShaderManager() noexcept;
bool engIsShaderManagerInitialized() noexcept;
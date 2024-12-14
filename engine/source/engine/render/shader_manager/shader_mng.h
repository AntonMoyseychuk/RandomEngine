#pragma once

#include "utils/file/file.h"

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
    const ShaderStageCreateInfo* pStages;
    size_t stagesCount;
};


class ShaderProgram
{
    friend class ShaderManager;

public:
    ~ShaderProgram() { Destroy(); }

    bool IsValid() const noexcept { return m_id != 0; }

private:
    ShaderProgram() = default;

    bool Init(const ShaderProgramCreateInfo& createInfo) noexcept;
    void Destroy() noexcept;

    bool GetLinkingStatus() const noexcept;

private:
    uint32_t m_id = 0;
};


class ShaderManager
{
    friend bool engInitShaderManager() noexcept;
    friend void engTerminateShaderManager() noexcept;
    friend bool engIsShaderManagerInitialized() noexcept;

public:
    static ShaderManager& GetInstance() noexcept;
    
public:
    ~ShaderManager();

private:
    ShaderManager() = default;

    bool Init() noexcept;
    void Terminate() noexcept;

    bool IsInitialized() const noexcept;

private:
    bool m_isInitialized = false; // TEMP
};


bool engInitShaderManager() noexcept;
void engTerminateShaderManager() noexcept;
bool engIsShaderManagerInitialized() noexcept;
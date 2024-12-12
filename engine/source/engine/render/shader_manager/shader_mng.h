#pragma once

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
    uint32_t codeSize;
    ShaderStageType type;
};


struct ShaderProgramCreateInfo
{
    const ShaderStageCreateInfo* pStages;
    size_t stagesCount;
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
#pragma once

#include "utils/file/file.h"
#include "utils/data_structures/strid.h"

#include <limits>
#include <cstdint>

#include "core.h"


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
#if defined(ENG_DEBUG)
    ds::StrID dbgName = "";
#endif

    const ShaderStageCreateInfo** pStageCreateInfos;
    size_t stageCreateInfosCount;
};


class ProgramUniform
{
    friend class ShaderManager;
    friend class ShaderProgram;
    friend class ProgramUniformStorage;

public:
    enum Type : uint32_t
    {
        TYPE_BOOL,
        TYPE_INT,
        TYPE_UINT,
        TYPE_FLOAT,
        TYPE_DOUBLE,
            
        TYPE_IVEC2,
        TYPE_IVEC3,
        TYPE_IVEC4,
            
        TYPE_UVEC2,
        TYPE_UVEC3,
        TYPE_UVEC4,
            
        TYPE_FVEC2,
        TYPE_FVEC3,
        TYPE_FVEC4,
            
        TYPE_DVEC2,
        TYPE_DVEC3,
        TYPE_DVEC4,
            
        TYPE_MAT2X2,
        TYPE_MAT2X3,
        TYPE_MAT2X4,
            
        TYPE_MAT3X2,
        TYPE_MAT3X3,
        TYPE_MAT3X4,
            
        TYPE_MAT4X2,
        TYPE_MAT4X3,
        TYPE_MAT4X4,
            
        TYPE_DMAT2X2,
        TYPE_DMAT2X3,
        TYPE_DMAT2X4,
            
        TYPE_DMAT3X2,
        TYPE_DMAT3X3,
        TYPE_DMAT3X4,
            
        TYPE_DMAT4X2,
        TYPE_DMAT4X3,
        TYPE_DMAT4X4,
            
        TYPE_SAMPLER_1D,
        TYPE_SAMPLER_2D,
        TYPE_SAMPLER_3D,
        TYPE_SAMPLER_CUBE,

        TYPE_IMAGE_1D,
        TYPE_IMAGE_2D,
        TYPE_IMAGE_3D,
        TYPE_IMAGE_CUBE,

        TYPE_IMAGE_2D_RECT,
        
        TYPE_IMAGE_BUFFER,

        TYPE_IMAGE_1D_ARRAY,
        TYPE_IMAGE_2D_ARRAY,

        TYPE_IMAGE_2D_MULTISAMPLE,
        TYPE_IMAGE_2D_MULTISAMPLE_ARRAY,

        TYPE_COUNT,
        TYPE_INVALID = TYPE_COUNT,
    };

public:
    ds::StrID GetName() const noexcept { return m_name; }
    int32_t GetLocation() const noexcept { return m_location; }

#if defined(ENG_DEBUG)
    uint32_t GetCount() const noexcept { return m_count; }
    Type GetType() const noexcept { return m_type; }
#endif

private:
    ds::StrID m_name;
    int32_t m_location = -1;

#if defined(ENG_DEBUG)
    uint32_t m_count = UINT32_MAX;
    Type m_type = TYPE_INVALID;
#endif
};


class ProgramUniformStorage
{
    friend class ShaderManager;
    friend class ShaderProgram;

public:
    using UniformsContainerType = std::vector<ProgramUniform>;
    using StorageConstIterType = UniformsContainerType::const_iterator;

public:
    bool HasUniform(ds::StrID uniformName) const noexcept;
    const ProgramUniform* GetUniform(ds::StrID uniformName) const noexcept;

    void SetUniformBool(ds::StrID uniformName, bool value) noexcept;
    void SetUniformBool2(ds::StrID uniformName, bool x, bool y) noexcept;
    void SetUniformBool3(ds::StrID uniformName, bool x, bool y, bool z) noexcept;
    void SetUniformBool4(ds::StrID uniformName, bool x, bool y, bool z, bool w) noexcept;

    void SetUniformInt(ds::StrID uniformName, int32_t value) noexcept;
    void SetUniformInt2(ds::StrID uniformName, int32_t x, int32_t y) noexcept;
    void SetUniformInt3(ds::StrID uniformName, int32_t x, int32_t y, int32_t z) noexcept;
    void SetUniformInt4(ds::StrID uniformName, int32_t x, int32_t y, int32_t z, int32_t w) noexcept;

    void SetUniformUInt(ds::StrID uniformName, uint32_t value) noexcept;
    void SetUniformUInt2(ds::StrID uniformName, uint32_t x, uint32_t y) noexcept;
    void SetUniformUInt3(ds::StrID uniformName, uint32_t x, uint32_t y, uint32_t z) noexcept;
    void SetUniformUInt4(ds::StrID uniformName, uint32_t x, uint32_t y, uint32_t z, uint32_t w) noexcept;

    void SetUniformFloat(ds::StrID uniformName, float value) noexcept;
    void SetUniformFloat2(ds::StrID uniformName, float x, float y) noexcept;
    void SetUniformFloat3(ds::StrID uniformName, float x, float y, float z) noexcept;
    void SetUniformFloat4(ds::StrID uniformName, float x, float y, float z, float w) noexcept;

    void SetUniformDouble(ds::StrID uniformName, double value) noexcept;
    void SetUniformDouble2(ds::StrID uniformName, double x, double y) noexcept;
    void SetUniformDouble3(ds::StrID uniformName, double x, double y, double z) noexcept;
    void SetUniformDouble4(ds::StrID uniformName, double x, double y, double z, double w) noexcept;

    void SetUniformFloat2x2(ds::StrID uniformName, const float* pMatrData, uint32_t matrCount, bool transpose) noexcept;
    void SetUniformFloat3x3(ds::StrID uniformName, const float* pMatrData, uint32_t matrCount, bool transpose) noexcept;
    void SetUniformFloat4x4(ds::StrID uniformName, const float* pMatrData, uint32_t matrCount, bool transpose) noexcept;
    void SetUniformFloat2x3(ds::StrID uniformName, const float* pMatrData, uint32_t matrCount, bool transpose) noexcept;
    void SetUniformFloat3x2(ds::StrID uniformName, const float* pMatrData, uint32_t matrCount, bool transpose) noexcept;
    void SetUniformFloat2x4(ds::StrID uniformName, const float* pMatrData, uint32_t matrCount, bool transpose) noexcept;
    void SetUniformFloat4x2(ds::StrID uniformName, const float* pMatrData, uint32_t matrCount, bool transpose) noexcept;
    void SetUniformFloat3x4(ds::StrID uniformName, const float* pMatrData, uint32_t matrCount, bool transpose) noexcept;
    void SetUniformFloat4x3(ds::StrID uniformName, const float* pMatrData, uint32_t matrCount, bool transpose) noexcept;

    bool IsEmpty() const noexcept { return m_uniforms.empty(); }

    StorageConstIterType cbegin() const noexcept { return m_uniforms.cbegin(); }
    StorageConstIterType cend() const noexcept { return m_uniforms.cend(); }

private:
    UniformsContainerType m_uniforms;
    const ShaderProgram* m_pOwner = nullptr;
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

    ProgramUniformStorage& GetUniformStorage() noexcept;

    bool IsValidRenderID() const noexcept { return m_renderID != 0; }
    bool IsValidUniformStorage() const noexcept { return m_pUniformStorage != nullptr; }
    bool IsValid() const noexcept { return IsValidRenderID() && IsValidUniformStorage(); }
    
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

    ProgramUniformStorage* m_pUniformStorage = nullptr;
    uint32_t m_renderID = 0;
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

    const ProgramUniformStorage* GetShaderProgramUniformStorageByID(const ProgramID& id) const noexcept;

private:
    ShaderManager() = default;

    ShaderManager(ShaderManager&& other) noexcept = default;
    ShaderManager& operator=(ShaderManager&& other) noexcept = default;

    bool Init() noexcept;
    void Terminate() noexcept;

    void FillProgramUniformStorage(const ShaderProgram& program, ProgramUniformStorage& storage) noexcept;

    bool IsInitialized() const noexcept;

private:
    struct ProgramIDHasher
    {
        uint64_t operator()(const ProgramID& id) const noexcept { return id.Hash(); }
    };

    std::unordered_map<ProgramID, ShaderProgram, ProgramIDHasher> m_shaderProgramsStorage;
    std::unordered_map<ProgramID, ProgramUniformStorage, ProgramIDHasher> m_uniformsStorage;

    bool m_isInitialized = false;
};


uint64_t amHash(const ShaderStageCreateInfo& stageCreateInfo) noexcept;
uint64_t amHash(const ShaderProgramCreateInfo& programCreateInfo) noexcept;


bool engInitShaderManager() noexcept;
void engTerminateShaderManager() noexcept;
bool engIsShaderManagerInitialized() noexcept;
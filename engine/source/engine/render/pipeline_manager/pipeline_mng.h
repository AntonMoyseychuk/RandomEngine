#pragma once

#include "render/rt_manager/rt_manager.h"
#include "render/shader_manager/shader_mng.h"

#include "utils/data_structures/strid.h"
#include "utils/data_structures/base_id.h"

#include <vector>
#include <unordered_map>


enum class CompareFunc : uint32_t
{
    FUNC_NEVER,
    FUNC_ALWAYS,
    FUNC_LESS,
    FUNC_GREATER,
    FUNC_EQUAL,
    FUNC_LEQUAL,
    FUNC_GEQUAL,
    FUNC_NOTEQUAL,

    FUNC_COUNT
};


enum class StencilOp : uint32_t
{
    STENCIL_OP_KEEP,            // Don't modify the current value (default)
    STENCIL_OP_ZERO,            // Set current value to zero
    STENCIL_OP_INCREMENT,       // Increment the current value, saturating if it would overflow
    STENCIL_OP_DECREMENT,       // Decrement the current value, setting to zero if it would underflow
    STENCIL_OP_INVERT,          // Invert the current value
    STENCIL_OP_REPLACE,         // Replace the current value with the masked fragment value
    STENCIL_OP_INCREMENT_WRAP,  // Increment the current value, wrapping if it would overflow
    STENCIL_OP_DECREMENT_WRAP,  // Decrement the current value, wrapping if it would underflow

    STENCIL_OP_COUNT
};


enum class PolygonMode : uint32_t
{
    POLYGON_MODE_FILL,
    POLYGON_MODE_LINE,
    POLYGON_MODE_POINT,

    POLYGON_MODE_COUNT,
};


enum class CullMode : uint32_t
{
    CULL_MODE_NONE,
    CULL_MODE_FRONT,
    CULL_MODE_BACK,
    CULL_MODE_FRONT_AND_BACK,

    CULL_MODE_COUNT
};


enum class FrontFace : uint32_t
{
    FRONT_FACE_COUNTER_CLOCKWISE,
    FRONT_FACE_CLOCKWISE,
    
    FRONT_FACE_COUNT, 
};


enum class PrimitiveTopology : uint32_t
{
    TOPOLOGY_POINTS,
    TOPOLOGY_LINE_STRIP,
    TOPOLOGY_LINE_LOOP,
    TOPOLOGY_LINES,
    TOPOLOGY_LINE_STRIP_ADJACENCY,
    TOPOLOGY_LINES_ADJACENCY,
    TOPOLOGY_TRIANGLE_STRIP,
    TOPOLOGY_TRIANGLE_FAN,
    TOPOLOGY_TRIANGLES,
    TOPOLOGY_TRIANGLE_STRIP_ADJACENCY,
    TOPOLOGY_TRIANGLES_ADJACENCY,
    TOPOLOGY_PATCHES,

    TOPOLOGY_COUNT,
};


enum class ColorComponentFlags : uint32_t
{
    COLOR_COMPONENT_R_BIT = 0x1,
    COLOR_COMPONENT_G_BIT = 0x2,
    COLOR_COMPONENT_B_BIT = 0x4,
    COLOR_COMPONENT_A_BIT = 0x8,

    COLOR_COMPONENT_COUNT = 4
};


enum class BlendFactor : uint32_t
{
    FACTOR_ZERO,
    FACTOR_ONE,
    FACTOR_SRC_COLOR,
    FACTOR_ONE_MINUS_SRC_COLOR,
    FACTOR_DST_COLOR,
    FACTOR_ONE_MINUS_DST_COLOR,
    FACTOR_SRC_ALPHA,
    FACTOR_ONE_MINUS_SRC_ALPHA,
    FACTOR_DST_ALPHA,
    FACTOR_ONE_MINUS_DST_ALPHA,
    FACTOR_CONSTANT_COLOR,
    FACTOR_ONE_MINUS_CONSTANT_COLOR,
    FACTOR_CONSTANT_ALPHA,
    FACTOR_ONE_MINUS_CONSTANT_ALPHA,
    
    // FACTOR_SRC1_COLOR = 15,
    // FACTOR_ONE_MINUS_SRC1_COLOR = 16,
    // FACTOR_SRC1_ALPHA = 17,
    // FACTOR_ONE_MINUS_SRC1_ALPHA = 18,

    FACTOR_COUNT
};


enum class BlendOp : uint32_t
{
    BLEND_OP_ADD,
    BLEND_OP_SUBTRACT,
    BLEND_OP_REVERSE_SUBTRACT,
    BLEND_OP_MIN,
    BLEND_OP_MAX,

    BLEND_OP_COUNT
};


enum class LogicOp : uint32_t
{
    LOGIC_OP_CLEAR,
    LOGIC_OP_AND,
    LOGIC_OP_AND_REVERSE,
    LOGIC_OP_COPY,
    LOGIC_OP_AND_INVERTED,
    LOGIC_OP_NO_OP,
    LOGIC_OP_XOR,
    LOGIC_OP_OR,
    LOGIC_OP_NOR,
    LOGIC_OP_EQUIVALENT,
    LOGIC_OP_INVERT,
    LOGIC_OP_OR_REVERSE,
    LOGIC_OP_COPY_INVERTED,
    LOGIC_OP_OR_INVERTED,
    LOGIC_OP_NAND,
    LOGIC_OP_SET,

    LOGIC_OP_COUNT
};


struct PipelineInputAssemblyStateCreateInfo
{
    PrimitiveTopology topology;
};


struct PipelineRasterizationStateCreateInfo
{
    FrontFace   frontFace;
    PolygonMode polygonMode;
    CullMode    cullMode;
    float       depthBiasConstantFactor;
    float       depthBiasClamp;
    float       depthBiasSlopeFactor;
    float       lineWidth;
    bool        depthBiasEnable;
};


struct PipelineDepthStencilStateCreateInfo
{
    CompareFunc depthCompareFunc;
    StencilOp   frontFaceStencilFailOp;
    StencilOp   frontFaceStencilPassDepthPassOp;
    StencilOp   frontFaceStencilPassDepthFailOp;
    StencilOp   backFaceStencilFailOp;
    StencilOp   backFaceStencilPassDepthPassOp;
    StencilOp   backFaceStencilPassDepthFailOp;
    uint32_t    stencilFrontMask;
    uint32_t    stencilBackMask;
    bool        depthTestEnable;
    bool        depthWriteEnable;
    bool        stencilTestEnable;
    bool        stencilFrontWriteEnable;
    bool        stencilBackWriteEnable;
};


struct PipelineColorBlendAttachmentState
{
    uint32_t            attachmentIndex;
    BlendFactor         srcRGBBlendFactor;
    BlendFactor         dstRGBBlendFactor;
    BlendOp             rgbBlendOp;
    BlendFactor         srcAlphaBlendFactor;
    BlendFactor         dstAlphaBlendFactor;
    BlendOp             alphaBlendOp;
    ColorComponentFlags colorWriteMask;
    bool                blendEnable;
};


struct PipelineColorBlendStateCreateInfo
{
    const PipelineColorBlendAttachmentState* pAttachmentStates;
    uint32_t                                 attachmentCount;
    float                                    blendConstants[4];
    LogicOp                                  logicOp;
    bool                                     logicOpEnable;
};


struct PipelineFrameBufferColorAttachmentClearColor
{
    float r, g, b, a;
};


struct PipelineFrameBufferClearValues
{
    const PipelineFrameBufferColorAttachmentClearColor* pColorAttachmentClearColors;
    uint32_t                                            colorAttachmentsCount;
    float                                               depthClearValue;
    int32_t                                             stencilClearValue;
};


struct PipelineCreateInfo
{
    const PipelineInputAssemblyStateCreateInfo* pInputAssemblyState = nullptr;
    const PipelineRasterizationStateCreateInfo* pRasterizationState = nullptr;
    const PipelineDepthStencilStateCreateInfo*  pDepthStencilState = nullptr;
    const PipelineColorBlendStateCreateInfo*    pColorBlendState = nullptr;
    const PipelineFrameBufferClearValues*       pFrameBufferClearValues = nullptr;
    FrameBuffer*                                pFrameBuffer = nullptr;
    ShaderProgram*                              pShaderProgram = nullptr;
};


using PipelineID = BaseID<uint32_t>;


class Pipeline
{
    friend class PipelineManager;

public:
    Pipeline() = default;
    ~Pipeline() { Destroy(); }

    Pipeline(const Pipeline& other) = delete;
    Pipeline& operator=(const Pipeline& other) = delete;

    Pipeline(Pipeline&& other) noexcept;
    Pipeline& operator=(Pipeline&& other) noexcept;

    bool Create(const PipelineCreateInfo& createInfo) noexcept;
    void Destroy();

    void ClearFrameBuffer() noexcept;
    void Bind() noexcept;

    uint64_t Hash() const noexcept;

    bool IsValid() const noexcept;

    const FrameBuffer& GetFrameBuffer() noexcept;
    const ShaderProgram& GetShaderProgram() noexcept;

private:
    static inline constexpr uint32_t BITS_PER_COLOR_ATTACHMENT_INDEX = 5;
    static inline constexpr uint32_t BITS_PER_COLOR_ATTACHMENT_COLOR_WRITE_MASK = 4;
    static inline constexpr uint32_t BITS_PER_COLOR_ATTACHMENT_BLEND_FACTOR = 4;
    static inline constexpr uint32_t BITS_PER_COLOR_ATTACHMENT_BLEND_OP = 3;
    static inline constexpr uint32_t BITS_PER_BLEND_LOGIC_OP = 5;
    static inline constexpr uint32_t BITS_PER_PRIMITIVE_TOPOLOGY = 4;
    static inline constexpr uint32_t BITS_PER_CULL_MODE = 4;
    static inline constexpr uint32_t BITS_PER_DEPTH_COMPARE_FUNC = 4;
    static inline constexpr uint32_t BITS_PER_STENCIL_OP = 4;
    static inline constexpr uint32_t BITS_PER_POLYGON_MODE = 2;

private:
    std::vector<PipelineFrameBufferColorAttachmentClearColor> m_frameBufferColorAttachmentClearColors;

    struct CompressedColorBlendAttachmentState
    {
        uint32_t attachmentIndex : BITS_PER_COLOR_ATTACHMENT_INDEX;
        uint32_t colorWriteMask : BITS_PER_COLOR_ATTACHMENT_COLOR_WRITE_MASK;
        uint32_t srcRGBBlendFactor : BITS_PER_COLOR_ATTACHMENT_BLEND_FACTOR;
        uint32_t dstRGBBlendFactor : BITS_PER_COLOR_ATTACHMENT_BLEND_FACTOR;
        uint32_t srcAlphaBlendFactor : BITS_PER_COLOR_ATTACHMENT_BLEND_FACTOR;
        uint32_t dstAlphaBlendFactor : BITS_PER_COLOR_ATTACHMENT_BLEND_FACTOR;
        uint32_t rgbBlendOp : BITS_PER_COLOR_ATTACHMENT_BLEND_OP;
        uint32_t alphaBlendOp : BITS_PER_COLOR_ATTACHMENT_BLEND_OP;
        uint32_t blendEnable : 1;
    };

    static_assert(sizeof(CompressedColorBlendAttachmentState) == sizeof(uint32_t));

    std::vector<CompressedColorBlendAttachmentState> m_compressedColorBlendAttachmentStates;

    float m_blendConstants[4] = { 0.f };

    FrameBuffer* m_pFrameBuffer = nullptr;
    ShaderProgram* m_pShaderProgram = nullptr;

    struct CompressedGlobalState
    {
        uint64_t colorBlendLogicOp : BITS_PER_BLEND_LOGIC_OP;
        uint64_t primitiveTopology : BITS_PER_PRIMITIVE_TOPOLOGY;
        uint64_t cullMode : BITS_PER_CULL_MODE;
        uint64_t depthCompareFunc : BITS_PER_DEPTH_COMPARE_FUNC;
        uint64_t frontFaceStencilFailOp : BITS_PER_STENCIL_OP;
        uint64_t frontFaceStencilPassDepthPassOp : BITS_PER_STENCIL_OP;
        uint64_t frontFaceStencilPassDepthFailOp : BITS_PER_STENCIL_OP;
        uint64_t backFaceStencilFailOp : BITS_PER_STENCIL_OP;
        uint64_t backFaceStencilPassDepthPassOp : BITS_PER_STENCIL_OP;
        uint64_t backFaceStencilPassDepthFailOp : BITS_PER_STENCIL_OP;
        uint64_t polygonMode : BITS_PER_POLYGON_MODE;
        uint64_t frontFace : 1;
        uint64_t depthBiasEnabled : 1;
        uint64_t depthTestEnable : 1;
        uint64_t depthWriteEnable : 1;
        uint64_t stencilTestEnable : 1;
        uint64_t colorBlendLogicOpEnable : 1;
        uint64_t stencilFrontWriteEnable : 1;
        uint64_t stencilBackWriteEnable : 1;
    } m_compressedGlobalState;

    static_assert(sizeof(CompressedGlobalState) == sizeof(uint64_t));

    PipelineID m_ID;

    float m_depthBiasConstantFactor = 0.f;
    float m_depthBiasClamp = 0.f;
    float m_depthBiasSlopeFactor = 0.f;

    float m_depthClearValue = 0.f;
    int32_t m_stencilClearValue = 0;
    uint32_t m_stencilFrontMask = 0;
    uint32_t m_stencilBackMask = 0;

    float m_lineWidth = 0.f;
};


class PipelineManager
{
    friend bool engInitPipelineManager() noexcept;
    friend void engTerminatePipelineManager() noexcept;
    friend bool engIsRenderPipelineInitialized() noexcept;

public:
    static PipelineManager& GetInstance() noexcept;

public:
    PipelineManager(const PipelineManager& other) = delete;
    PipelineManager& operator=(const PipelineManager& other) = delete;
    PipelineManager(PipelineManager&& other) noexcept = delete;
    PipelineManager& operator=(PipelineManager&& other) noexcept = delete;

    Pipeline* RegisterPipeline() noexcept;
    void UnregisterPipeline(Pipeline* pPipeline) noexcept;
    
private:
    PipelineManager() = default;

    bool Init() noexcept;
    void Terminate() noexcept;

    PipelineID AllocatePipelineID() noexcept;
    void DeallocatePipelineID(PipelineID ID) noexcept;

    bool IsInitialized() const noexcept { return m_isInitialized; }

private:
    std::vector<Pipeline> m_pipelineStorage;

    std::deque<PipelineID> m_pipelineIDFreeList;

    PipelineID m_nextAllocatedID = PipelineID{0};

    bool m_isInitialized = false;
};


uint64_t amHash(const Pipeline& pipeline) noexcept;


bool engInitPipelineManager() noexcept;
void engTerminatePipelineManager() noexcept;
bool engIsRenderPipelineInitialized() noexcept;
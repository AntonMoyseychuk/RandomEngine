#include "shadergen.h"

#include "logging/log.h"

#include <regex>
#include <fstream>
#include <sstream>


static std::vector<char> ReadTextFile(const fs::path& filepath) noexcept
{
    if (!fs::exists(filepath)) {
        SH_LOG_CRITICAL("File {} doesn't exist", filepath.string().c_str());
        return {};
    }

    std::ifstream file(filepath, std::ios_base::ate);
    if (!file.is_open()) {
        SH_LOG_CRITICAL("Failed to open {} file", filepath.string().c_str());
        return {};
    }

    std::vector<char> outData;

    const size_t fileSize = (size_t)file.tellg();
    if (fileSize == 0) {
        file.close();
        return {};
    }
    
    outData.resize(fileSize);

    file.seekg(0);
    file.read(reinterpret_cast<char*>(outData.data()), fileSize);

    file.close();

    return outData;
}


static void WriteTextFile(const fs::path& filepath, const char* pData, size_t size) noexcept
{
    if (pData == nullptr) {
        SH_LOG_WARN("Attempt to write nullptr data to file {}", filepath.string().c_str());
        return;
    }

    std::ofstream file(filepath, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        SH_LOG_CRITICAL("File writing error. Failed to open {} file", filepath.string().c_str());
        return;
    }

    file.write(reinterpret_cast<const char*>(pData), size);
    file.close();
}


static const char* TranslateGLSLToEngineConstantPrimitiveType(const std::string& type) noexcept
{
    static const std::unordered_map<std::string, std::string> GLSLToEnginePrimitiveTypeMap = {
        { "bool", "bool" },
        { "int", "int32_t" },
        { "uint", "uint32_t" },
        { "float", "float" },
        { "double", "double" },
        
        { "vec2", "glm::vec2" },
        { "vec3", "glm::vec3" },
        { "vec4", "glm::vec4" },
        { "ivec2", "glm::ivec2" },
        { "ivec3", "glm::ivec3" },
        { "ivec4", "glm::ivec4" },
        { "bvec2", "glm::bvec2" },
        { "bvec3", "glm::bvec3" },
        { "bvec4", "glm::bvec4" },

        { "mat2", "glm::mat2" },
        { "mat3", "glm::mat3" },
        { "mat4", "glm::mat4" },
        { "mat2x3", "glm::mat2x3" },
        { "mat2x4", "glm::mat2x4" },
        { "mat3x2", "glm::mat3x2" },
        { "mat3x4", "glm::mat3x4" },
        { "mat4x2", "glm::mat4x2" },
        { "mat4x3", "glm::mat4x3" },

        { "dmat2", "glm::dmat2" },
        { "dmat3", "glm::dmat3" },
        { "dmat4", "glm::dmat4" },
        { "dmat2x3", "glm::dmat2x3" },
        { "dmat2x4", "glm::dmat2x4" },
        { "dmat3x2", "glm::dmat3x2" },
        { "dmat3x4", "glm::dmat3x4" },
        { "dmat4x2", "glm::dmat4x2" },
        { "dmat4x3", "glm::dmat4x3" },
    };

    const auto typeIt = GLSLToEnginePrimitiveTypeMap.find(type);
    
    return typeIt != GLSLToEnginePrimitiveTypeMap.cend() ? typeIt->second.c_str() : nullptr;
}


static const char* TranslateGLSLToEnginePrimitiveResourceType(const std::string& type) noexcept
{
    static const std::unordered_map<std::string, std::string> GLSLToEnginePrimResTypeMap = {
        { "bool", "ShaderResourceType::TYPE_BOOL" },
        { "int", "ShaderResourceType::TYPE_INT" },
        { "uint", "ShaderResourceType::TYPE_UINT" },
        { "float", "ShaderResourceType::TYPE_FLOAT" },
        { "double", "ShaderResourceType::TYPE_DOUBLE" },
    };

    const auto typeIt = GLSLToEnginePrimResTypeMap.find(type);
    
    return typeIt != GLSLToEnginePrimResTypeMap.cend() ? typeIt->second.c_str() : nullptr;
}


static const char* TranslateGLSLToEngineNonPrimitiveResourceType(const std::string& type) noexcept
{
    static const std::unordered_map<std::string, std::string> GLSLToEngineNonPrimResTypeMap = {
        { "sampler2D", "ShaderResourceType::TYPE_SAMPLER_2D" }
    };

    const auto typeIt = GLSLToEngineNonPrimResTypeMap.find(type);
    
    return typeIt != GLSLToEngineNonPrimResTypeMap.cend() ? typeIt->second.c_str() : nullptr;
}


static void RemoveTextComments(std::string& text) noexcept
{
    static std::regex COMMENTS_PATTERN(R"(//.*?$|/\\*[^]*?\\*/)");
    text = std::regex_replace(text.data(), COMMENTS_PATTERN, "");
}


static void PushHeaderToStream(std::stringstream& ss) noexcept
{
    ss << 
        "#pragma once"
        "\n// ----------- This is auto file, don't modify! -----------"
        "\n"
        "\n#include \"render/shader_manager/resource_bind.h\""
        "\n#include \"utils/math/common_math.h\""
        "\n"
        "\n";
}


static std::vector<std::cmatch> FindPatternMatches(const std::regex& pattern, const char* pFileContent, size_t fileSize) noexcept
{
    auto beginIt = std::cregex_iterator(pFileContent, pFileContent + fileSize, pattern);
    auto endIt = std::cregex_iterator();

    std::vector<std::cmatch> matches;
    matches.reserve(std::distance(beginIt, endIt));

    for (auto it = beginIt; it != endIt; ++it) {
        matches.emplace_back(*it);
    }

    return matches;
}


static std::vector<std::cmatch> FindIncludesDeclMatches(const char* pFileContent, size_t fileSize) noexcept
{
    static std::regex INCLUDE_DECL_PATTERN(R"(REFLECT_INCLUDE\(([^,]+)\))");
    
    return FindPatternMatches(INCLUDE_DECL_PATTERN, pFileContent, fileSize);
}


static void PushIncludesToStream(std::stringstream& ss, const char* pFileContent, size_t fileSize) noexcept
{
    const std::vector<std::cmatch> includesDeclMatches = FindIncludesDeclMatches(pFileContent, fileSize);

    for (const std::cmatch& match : includesDeclMatches) {
        const std::string includeName = match[1].str();

        ss << "#include \"auto_" << includeName << ".h\"\n";
    }

    if (!includesDeclMatches.empty()) {
        ss << '\n';
    }
}


static std::vector<std::cmatch> FindConstVarsDeclMatches(const char* pFileContent, size_t fileSize) noexcept
{
    static std::regex CONSTANT_PATTERN(R"(DECLARE_CONSTANT\(([^,]+), ([^,]+), ([^,]+)\))");
    
    return FindPatternMatches(CONSTANT_PATTERN, pFileContent, fileSize);
}


static void PushConstVarsDeclToStream(std::stringstream& ss, const char* pFileContent, size_t fileSize) noexcept
{
    const std::vector<std::cmatch> constantDeclMatches = FindConstVarsDeclMatches(pFileContent, fileSize);

    for (const std::cmatch& match : constantDeclMatches) {     
        const std::string type = match[1].str();   
        const std::string name = match[2].str();
        
        const char* pType = TranslateGLSLToEngineConstantPrimitiveType(type);
        if (!pType) {
            SH_LOG_ERROR("Unknown constant variable {} type: {}", name.c_str(), type.c_str());
            continue;
        }
        
        const std::string value = match[3].str();

        ss << "inline constexpr " << pType << ' ' << name << " = " << value << ";\n";
    }

    if (!constantDeclMatches.empty()) {
        ss << '\n';
    }
}


static std::vector<std::cmatch> FindSrvVarsDeclMatches(const char* pFileContent, size_t fileSize) noexcept
{
    static std::regex SRV_VAR_PATTERN(R"(DECLARE_SRV_VARIABLE\(([^,]+), ([^,]+), ([^,]+), ([^,]+)\))");

    return FindPatternMatches(SRV_VAR_PATTERN, pFileContent, fileSize);
}


static void PushSrvVarsDeclToStream(std::stringstream& ss, const char* pFileContent, size_t fileSize) noexcept
{
    const std::vector<std::cmatch> srvVarsDeclMatches = FindSrvVarsDeclMatches(pFileContent, fileSize);

    for (const std::cmatch& match : srvVarsDeclMatches) {
        const std::string type = match[1].str();
        const std::string name = match[2].str();
        

        const char* pType = TranslateGLSLToEnginePrimitiveResourceType(type);
        if (!pType) {
            SH_LOG_ERROR("Unknown shader resource view (SRV) variable {} type: {}", name.c_str(), type.c_str());
            continue;
        }

        const std::string location = match[3].str();

        ss <<
        "struct " << name << " {\n"
        "    inline static constexpr ShaderResourceBindStruct<" << pType << "> _BINDING = { " << location << ", -1 };\n"
        "};\n"
        "\n";
    }

    if (!srvVarsDeclMatches.empty()) {
        ss << '\n';
    }
}


static std::vector<std::cmatch> FindSrvTextureDeclMatches(const char* pFileContent, size_t fileSize) noexcept
{
    static std::regex SRV_TEXTURE_PATTERN(R"(DECLARE_SRV_TEXTURE\(([^,]+), ([^,]+), ([^,]+), ([^,]+), ([^,]+)\))");
    
    return FindPatternMatches(SRV_TEXTURE_PATTERN, pFileContent, fileSize);
}


static void PushSrvTextureDeclToStream(std::stringstream& ss, const char* pFileContent, size_t fileSize) noexcept
{
    const std::vector<std::cmatch> srvTexturesDeclMatches = FindSrvTextureDeclMatches(pFileContent, fileSize);

    for (const std::cmatch& match : srvTexturesDeclMatches) {
        const std::string type = match[1].str();
        const std::string name = match[2].str();

        const char* pType = TranslateGLSLToEngineNonPrimitiveResourceType(type);
        if (!pType) {
            SH_LOG_ERROR("Unknown texture variable {} type: {}", name.c_str(), type.c_str());
            continue;
        } 

        const std::string binding = match[3].str();
        const std::string format = match[4].str();
        const std::string samplerIdx = match[5].str();

        ss <<
        "struct " << name << " {\n"
        "    inline static constexpr ShaderResourceBindStruct<" << pType << "> _BINDING = { -1, " << binding << " };\n"
        "    inline static constexpr uint32_t _SAMPLER_IDX = " << samplerIdx << ";\n"
        "    inline static constexpr uint32_t _FORMAT = " << format << ";\n"
        "};\n"
        "\n";
    }

    if (!srvTexturesDeclMatches.empty()) {
        ss << '\n';
    }
}


static std::vector<std::cmatch> FindConstBufferDeclMatches(const char* pFileContent, size_t fileSize) noexcept
{
    static std::regex CB_PATTERN(R"(DECLARE_CBV\(([^,]+), ([^,]+)\)\s*\{\s*([^{}]+)\s*\})");
    
    return FindPatternMatches(CB_PATTERN, pFileContent, fileSize);
}


static std::vector<std::cmatch> FindConstBufferMembersDeclMatches(const char* pCBContent, size_t cbContentSize) noexcept
{
    static std::regex CB_CONTENT_PATTERN(R"(\b([a-zA-Z0-9]+)\s+([a-zA-Z0-9_]+)(\[[^\]]*\])?(?=\s*;))");
    
    return FindPatternMatches(CB_CONTENT_PATTERN, pCBContent, cbContentSize);
}


static void PushConstBufferDeclToStream(std::stringstream& ss, const char* pFileContent, size_t fileSize) noexcept
{
    const std::vector<std::cmatch> constBuffDeclMatches = FindConstBufferDeclMatches(pFileContent, fileSize);

    for (const std::cmatch& cbMatch : constBuffDeclMatches) {
        const std::string constBufferName = cbMatch[1].str();
        const std::string binding = cbMatch[2].str();
        const std::string content = cbMatch[3].str();

        const std::vector<std::cmatch> constBuffContentsMatches = FindConstBufferMembersDeclMatches(content.c_str(), content.size());
        
        ss <<
        "struct " << constBufferName << " {\n"
        "    inline static constexpr ShaderResourceBindStruct<ShaderResourceType::TYPE_CONST_BUFFER>" << "_BINDING = { -1, " << binding << " };\n";
        
        if (!constBuffContentsMatches.empty()) {
            ss << '\n';
        }

        for (const std::cmatch& memberMatch : constBuffContentsMatches) {
            const std::string type = memberMatch[1].str();   
            const std::string varName = memberMatch[2].str();
            
            const char* pType = TranslateGLSLToEngineConstantPrimitiveType(type);
            if (!pType) {
                SH_LOG_ERROR("Unknown const buffer {} variable {} type: {}", constBufferName.c_str(), varName.c_str(), type.c_str());
                continue;
            }

            ss << "    " << pType << ' ' << varName;

            const std::string memberArrayCapture = memberMatch[3].str();
            if (!memberArrayCapture.empty()) {
                ss << memberArrayCapture;
            }

            ss << ";\n";
        }

        ss << "};\n"
        "\n";
    }

    if (!constBuffDeclMatches.empty()) {
        ss << '\n';
    }
}


#define CHECK_ARG_NOT_NULL(arg, index) \
    if ((arg) == nullptr) { \
        SH_LOG_CRITICAL("argv[{}] is nullptr", index); \
        return false; \
    }


static constexpr const char* SHGEN_INPUT_FILE_FLAG = "-i";
static constexpr const char* SHGEN_OUTPUT_FILE_FLAG = "-o";


static ShaderGen::InputFlag GetInputFlag(const char* pArg) noexcept
{
    if (strcmp(pArg, SHGEN_INPUT_FILE_FLAG) == 0) {
        return ShaderGen::InputFlag::INPUT_FILE;
    } else if (strcmp(pArg, SHGEN_OUTPUT_FILE_FLAG) == 0) {
        return ShaderGen::InputFlag::OUTPUT_FILE;
    } else {
        return ShaderGen::InputFlag::INVALID;
    }
}


ShaderGen::~ShaderGen()
{
    Terminate();
}


bool ShaderGen::Init(int argc, char *argv[]) noexcept
{
    shInitLogger();
    return ParseCMDLine(argc, argv);
}


void ShaderGen::Terminate() noexcept
{
    m_inputFilePaths.clear();
    m_outputFilePaths.clear();
    shTerminateLogger();
}


void ShaderGen::Run() noexcept
{
    for (uint64_t i = 0; i < m_inputFilePaths.size(); ++i) {
        const fs::path& inputPath = m_inputFilePaths[i];
        const fs::path& outputPath = m_outputFilePaths[i];

        Generate(inputPath, outputPath);
    }
}


bool ShaderGen::ParseCMDLine(int argc, char* argv[]) noexcept
{
    if (!argv) {
        SH_LOG_CRITICAL("Invlid Shader Gen argv argument");
        return false;
    }

    if (argc < 2) { // shadergen.exe -i input_file.fx -o output_file.h
        SH_LOG_CRITICAL("Shader Gen must accept at least one input file path and one output file path");
        return false;
    }

    static constexpr uint64_t EXPRESION_SIZE = 2;
    
    const uint64_t argCount = (uint64_t)argc;

    for (uint64_t i = 1; i < argCount; i += EXPRESION_SIZE) {
        const char* pFlag = argv[i];
        CHECK_ARG_NOT_NULL(pFlag, i);

        if (i + EXPRESION_SIZE > argCount) {
            SH_LOG_CRITICAL("Missed argument for {} flag", pFlag);
            return false;
        }

        const InputFlag flag = GetInputFlag(pFlag);

        if (flag == InputFlag::INVALID) {
            SH_LOG_CRITICAL("Undefined CMD flag: {}", pFlag);
            return false;
        }

        const char* pArg = argv[i + 1];
        CHECK_ARG_NOT_NULL(pArg, i + 1);

        ProcessInputFlag(flag, pArg);
    }

    if (m_inputFilePaths.size() != m_outputFilePaths.size()) {
        SH_LOG_CRITICAL("Input and output files count are not equal");
        return false;
    }

    return true;
}


void ShaderGen::ProcessInputFlag(InputFlag cmd, const char *pArg) noexcept
{
    switch (cmd) {
        case InputFlag::INPUT_FILE:
            m_inputFilePaths.emplace_back(pArg);
            break;
        case InputFlag::OUTPUT_FILE:
            m_outputFilePaths.emplace_back(pArg);
            break;
        default:
            break;
    }
}


void ShaderGen::Generate(const fs::path& inputFilePath, const fs::path& outputFilePath) noexcept
{
    SH_LOG_INFO("Processing {} file", inputFilePath.string().c_str());

    std::vector<char> inputFileContent = ReadTextFile(inputFilePath);
    if (inputFileContent.empty()) {
        return;
    }

    std::string inputText(inputFileContent.begin(), inputFileContent.end());

    RemoveTextComments(inputText);

    std::stringstream ss;

    PushHeaderToStream(ss);
    PushIncludesToStream(ss, inputText.c_str(), inputText.length() + 1);
    PushConstVarsDeclToStream(ss, inputText.c_str(), inputText.length() + 1);
    PushSrvVarsDeclToStream(ss, inputText.c_str(), inputText.length() + 1);
    PushSrvTextureDeclToStream(ss, inputText.c_str(), inputText.length() + 1);
    PushConstBufferDeclToStream(ss, inputText.c_str(), inputText.length() + 1);

    ss << '\n';

    const std::string outputContent = ss.str();
    WriteTextFile(outputFilePath, outputContent.data(), outputContent.size());
}

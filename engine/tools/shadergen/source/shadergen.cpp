#include "shadergen.h"

#include <regex>
#include <fstream>
#include <sstream>

#include <vector>
#include <unordered_map>

#include <cassert>


enum CMD_ARGS
{
    CMD_ARG_SHADERGEN_FILEPATH,
    CMD_ARG_INTPUT_FILE,
    CMD_ARG_OUTPUT_DIR,

    CMD_ARG_COUNT
};


namespace fs = std::filesystem;




static const char* TranslateGLSLToEngineResourceType(const fs::path& filepath, const std::string& type) noexcept
{
    static const std::unordered_map<std::string, const char*> GLSLToEngineResTypeMap = {
        { "sampler2D", "ShaderResourceType::TYPE_SAMPLER_2D" },
        { "bool", "ShaderResourceType::TYPE_BOOL" },
        { "int", "ShaderResourceType::TYPE_INT" },
        { "uint", "ShaderResourceType::TYPE_UINT" },
        { "float", "ShaderResourceType::TYPE_FLOAT" },
        { "double", "ShaderResourceType::TYPE_DOUBLE" },
    };

    const auto typeIt = GLSLToEngineResTypeMap.find(type);

    if (typeIt == GLSLToEngineResTypeMap.cend()) {
        fprintf_s(stderr, "\n-- [SHADERGEN] Invalid shader resource view (SRV) type: %s (%s)\n", type.c_str(), filepath.string().c_str());
        return nullptr;
    }
    
    return typeIt->second;
}


static const char* TranslateGLSLToEngineConstantPrimitiveType(const fs::path& filepath, const std::string& type) noexcept
{
    static const std::unordered_map<std::string, const char*> GLSLToEnginePrimitiveTypeMap = {
        { "bool", "bool" },
        { "int", "int32_t" },
        { "uint", "uint32_t" },
        { "float", "float" },
        { "double", "double" },
    };

    const auto typeIt = GLSLToEnginePrimitiveTypeMap.find(type);

    if (typeIt == GLSLToEnginePrimitiveTypeMap.cend()) {
        fprintf_s(stderr, "\n-- [SHADERGEN] Invalid constant primitive type: %s (%s)\n", type.c_str(), filepath.string().c_str());
        return nullptr;
    }
    
    return typeIt->second;
}


static std::vector<char> ReadTextFile(const fs::path& filepath) noexcept
{
    if (!fs::exists(filepath)) {
        fprintf_s(stderr, "\n-- [SHADERGEN] File %s doesn't exist.\n", filepath.string().c_str());
        assert(fs::exists(filepath));
        return {};
    }

    std::ifstream file(filepath, std::ios_base::ate);
    if (!file.is_open()) {
        fprintf_s(stderr, "\n-- [SHADERGEN] Failed to open %s file.\n", filepath.string().c_str());
        assert(file.is_open());
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
        fprintf_s(stderr, "\n-- [SHADERGEN] File %s writing warning: pData is nullptr\n", filepath.string().c_str());
        assert(pData != nullptr);
        return;
    }

    std::ofstream file(filepath, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        fprintf_s(stderr, "\n-- [SHADERGEN] File writing error. Failed to open %s file.\n", filepath.string().c_str());
        assert(file.is_open());
        return;
    }

    file.write(reinterpret_cast<const char*>(pData), size);

    file.close();
}


static bool CheckFileContentInput(const fs::path& filepath, const char* pFileContent, size_t fileSize) noexcept
{
    if (!pFileContent || fileSize == 0) {
        fprintf_s(stderr, "\n-- [SHADERGEN] Warning: %s shader file is empty\n", filepath.string().c_str());
        return false;
    }

    return true;
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


static std::vector<std::cmatch> FindConstantDeclarationMatches(const char* pFileContent, size_t fileSize) noexcept
{
    static std::regex CONSTANT_PATTERN(R"(DECLARE_CONSTANT\(([^,]+), ([^,]+), ([^,]+)\))");
    
    return FindPatternMatches(CONSTANT_PATTERN, pFileContent, fileSize);
}


static std::vector<std::cmatch> FindSrvVariableDeclarationMatches(const char* pFileContent, size_t fileSize) noexcept
{
    static std::regex SRV_VAR_PATTERN(R"(DECLARE_SRV_VARIABLE\(([^,]+), ([^,]+), ([^,]+), ([^,]+)\))");

    return FindPatternMatches(SRV_VAR_PATTERN, pFileContent, fileSize);
}


static std::vector<std::cmatch> FindSrvTextureDeclarationMatches(const char* pFileContent, size_t fileSize) noexcept
{
    static std::regex SRV_TEXTURE_PATTERN(R"(DECLARE_SRV_TEXTURE\(([^,]+), ([^,]+), ([^,]+), ([^,]+)\))");
    
    return FindPatternMatches(SRV_TEXTURE_PATTERN, pFileContent, fileSize);
}


static void FillConstantDeclaration(std::stringstream& ss, const char* pFileContent, size_t fileSize, const fs::path& filepath) noexcept
{
    if (!CheckFileContentInput(filepath, pFileContent, fileSize)) {
        return;
    }

    const std::vector<std::cmatch> constantDeclMatches = FindConstantDeclarationMatches(pFileContent, fileSize);

    for (const std::cmatch& match : constantDeclMatches) {        
        const char* pType = TranslateGLSLToEngineConstantPrimitiveType(filepath, match[1].str());
        assert(pType);

        ss << "inline constexpr " << pType << ' ' << match[2].str() << " = " << match[3].str() << ";\n";
    }

    if (!constantDeclMatches.empty()) {
        ss << "\n\n";
    }
}


static void FillSrvVariablesDeclaration(std::stringstream& ss, const char* pFileContent, size_t fileSize, const fs::path& filepath) noexcept
{
    if (!CheckFileContentInput(filepath, pFileContent, fileSize)) {
        return;
    }

    const std::vector<std::cmatch> srvVarsDeclMatches = FindSrvVariableDeclarationMatches(pFileContent, fileSize);

    for (const std::cmatch& match : srvVarsDeclMatches) {        
        const char* pType = TranslateGLSLToEngineResourceType(filepath, match[1].str());
        assert(pType);

        ss <<
        "struct " << match[2].str() << " {\n"
        "    inline static constexpr ShaderResourceBindStruct<" << pType << "> _BINDING = { " << match[3].str() << ", 0 };\n"
        "};\n"
        "\n";
    }

    if (!srvVarsDeclMatches.empty()) {
        ss << "\n";
    }
}


static void FillSrvTextureDeclaration(std::stringstream& ss, const char* pFileContent, size_t fileSize, const fs::path& filepath) noexcept
{
    if (!CheckFileContentInput(filepath, pFileContent, fileSize)) {
        return;
    }

    const std::vector<std::cmatch> srvTexturesDeclMatches = FindSrvTextureDeclarationMatches(pFileContent, fileSize);

    for (const std::cmatch& match : srvTexturesDeclMatches) {
        const char* pType = TranslateGLSLToEngineResourceType(filepath, match[1].str());
        assert(pType);

        ss <<
        "struct " << match[2].str() << " {\n"
        "    inline static constexpr ShaderResourceBindStruct<" << pType << "> _BINDING = { " << match[3].str() << ", " << match[4].str() << " };\n"
        "};\n"
        "\n";
    }

    if (!srvTexturesDeclMatches.empty()) {
        ss << "\n";
    }
}


bool ValidateCommandLineArgs(int argc, char* argv[]) noexcept
{
    if (argc != CMD_ARG_COUNT) {
        fprintf_s(stderr, "\n-- [SHADERGEN] Error: Usage: %s \"input_fx_file\" \"output_directory\"\n", argv[CMD_ARG_SHADERGEN_FILEPATH]);
        assert(argc == CMD_ARG_COUNT);
        return false;
    }

    const fs::path inputFilepath = argv[CMD_ARG_INTPUT_FILE];
    if (!fs::exists(inputFilepath)) {
        fprintf_s(stderr, "\n-- [SHADERGEN] Error: \"%s\" doesn't exist\n", argv[CMD_ARG_INTPUT_FILE]);
        assert(fs::exists(inputFilepath));
        return false;
    }

    return true;
}


ShaderGenInputParams GetInputParams(int argc, char* argv[]) noexcept
{
    ShaderGenInputParams inputParams = {};

    inputParams.inputFilepath = argv[CMD_ARG_INTPUT_FILE];
    inputParams.outputDirectory = argv[CMD_ARG_OUTPUT_DIR];

    if (!fs::exists(inputParams.outputDirectory)) {
        fs::create_directories(inputParams.outputDirectory);
    }

    return inputParams;
}


void GenerateAutoFile(const ShaderGenInputParams& inputParams) noexcept
{
    const std::vector<char> inputFileContent = ReadTextFile(inputParams.inputFilepath);

    std::stringstream ss;

    ss << 
    "#pragma once\n"
    "// ----------- This is auto file, don't modify! -----------\n"
    "\n"
    "#include \"engine/render/shader_manager/resource_bind.h\"\n"
    "\n"
    "\n";

    FillConstantDeclaration(ss, inputFileContent.data(), inputFileContent.size(), inputParams.inputFilepath);
    FillSrvVariablesDeclaration(ss, inputFileContent.data(), inputFileContent.size(), inputParams.inputFilepath);
    FillSrvTextureDeclaration(ss, inputFileContent.data(), inputFileContent.size(), inputParams.inputFilepath);
    
    const std::filesystem::path outputFilepath = inputParams.outputDirectory / 
        ("auto_" + inputParams.inputFilepath.filename().replace_extension(".h").string());

    const std::string outputContent = ss.str();
    WriteTextFile(outputFilepath, outputContent.data(), outputContent.size());
}
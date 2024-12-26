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


static std::unordered_map<std::string, std::string> g_glslToEngineResourceTypeMap = {
    { "sampler2D", "ShaderResourceType::TYPE_SAMPLER_2D" }
};


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


static std::vector<std::cmatch> FindSrvTextureDeclarationMatches(const std::vector<char>& fileContent) noexcept
{
    static std::regex SRV_TEXTURE_PATTERN(R"(DECLARE_SRV_TEXTURE\(([^,]+), ([^,]+), ([^,]+), ([^,]+)\))");
    
    auto beginIt = std::cregex_iterator(fileContent.data(), fileContent.data() + fileContent.size(), SRV_TEXTURE_PATTERN);
    auto endIt = std::cregex_iterator();

    std::vector<std::cmatch> matches;
    matches.reserve(std::distance(beginIt, endIt));

    for (auto it = beginIt; it != endIt; ++it) {
        matches.emplace_back(*it);
    }

    return matches;
}


static void FillSrvTextureDeclaration(std::stringstream& ss, const std::vector<char>& fileData, const fs::path& filepath) noexcept
{
    if (fileData.empty()) {
        fprintf_s(stderr, "\n-- [SHADERGEN] Warning: %s shader file is empty\n", filepath.string().c_str());
        return;
    }

    const std::vector<std::cmatch> srvTexturesDeclMatches = FindSrvTextureDeclarationMatches(fileData);

    for (const std::cmatch& match : srvTexturesDeclMatches) {
        const std::string resType = match[1].str();
        
        const auto engineResTypeIt = g_glslToEngineResourceTypeMap.find(resType);

        if (engineResTypeIt == g_glslToEngineResourceTypeMap.cend()) {
            fprintf_s(stderr, "\n-- [SHADERGEN] Invalid resuorce type: %s (%s)\n", resType.c_str(), filepath.string().c_str());
            assert(false);
            return;
        }

        ss <<
        "struct " << match[2].str() << " {\n"
        "    inline static constexpr ShaderResourceBindStruct<" << engineResTypeIt->second << "> _SRV_BIND = {" << match[3].str() << ", " << match[4].str() <<  "};\n"
        "};"
        "\n";
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

    FillSrvTextureDeclaration(ss, inputFileContent, inputParams.outputDirectory);
    
    const std::filesystem::path outputFilepath = inputParams.outputDirectory / 
        ("auto_" + inputParams.inputFilepath.filename().replace_extension(".h").string());

    const std::string outputContent = ss.str();
    WriteTextFile(outputFilepath, outputContent.data(), outputContent.size());
}
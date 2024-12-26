#pragma once

#include <filesystem>

struct ShaderGenInputParams
{
    std::filesystem::path inputFilepath;
    std::filesystem::path outputDirectory;
};


bool ValidateCommandLineArgs(int argc, char* argv[]) noexcept;
ShaderGenInputParams GetInputParams(int argc, char* argv[]) noexcept;
void GenerateAutoFile(const ShaderGenInputParams& inputParams) noexcept;
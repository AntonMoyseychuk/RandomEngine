#pragma once

// SHADERGEN command line arguments:
// * -s -> input file path
// * -o -> output file path

// Example: shadergen.exe -s path/to/source/file0.fx -o path/to/output/file0.h -s path/to/source/file1.fx -o path/to/output/file1.h


#include <filesystem>
#include <vector>

namespace fs = std::filesystem;


class ShaderGen
{
public:
    enum class InputFlag
    {
        INVALID,
        INPUT_FILE,
        OUTPUT_FILE
    };

public:
    ShaderGen() = default;
    ~ShaderGen();

    bool Init(int argc, char* argv[]) noexcept;
    void Terminate() noexcept;
    void Run() noexcept;

private:
    bool ParseCMDLine(int argc, char* argv[]) noexcept;
    void ProcessInputFlag(InputFlag flag, const char* pArg) noexcept;

    void Generate(const fs::path& inputFilePath, const fs::path& outputFilePath) noexcept;

private:
    std::vector<fs::path> m_inputFilePaths;
    std::vector<fs::path> m_outputFilePaths;
};
#include "shadergen.h"


int main(int argc, char* argv[])
{
    if (!ValidateCommandLineArgs(argc, argv)) {
        return -1;
    } 

    const ShaderGenInputParams inputParams = GetInputParams(argc, argv);
    GenerateAutoFile(inputParams);
}
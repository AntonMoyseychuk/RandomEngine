#include "shadergen/shadergen.h"


int main(int argc, char* argv[])
{
    ShaderGen shgen;

    if (!shgen.Init(argc, argv)) {
        return -1;
    }

    shgen.Run();
    shgen.Terminate();

    return 0;
}
#include "application/application.h"

#include <cstdio>


int main(int argc, char* argv[])
{
    std::unique_ptr<Application> pApp = std::make_unique<Application>();
    
    if (!(pApp && pApp->Init("Random Graphics", 1280, 720))) {
        fprintf_s(stderr, "Application initialization failed!\n");
        exit(EXIT_FAILURE);
    }

    pApp->Run();
    pApp->Terminate();

    return EXIT_SUCCESS;
}
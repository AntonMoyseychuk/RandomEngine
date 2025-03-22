#include "log.h"

#include <cstdio>


struct ShaderGenLoggerTag {};


static constexpr const char* SH_LOGGER_PATTERN = "[%l] [%n] [%H:%M:%S:%e]: %^%v%$";

static bool s_isInitialized = false;


void shInitLogger() noexcept
{
    if (s_isInitialized) {
        return;
    }

    if (!logg::InitLogSystem()) {
        puts("Unexpected problems occurred during the initialization of the shadergen log system.\n");
        return;
    }
    
    logg::Logger* pLogger = logg::LogSystem::GetInstance().CreateLogger<ShaderGenLoggerTag>("SHADERGEN");
    pLogger->SetPattern(SH_LOGGER_PATTERN);
    pLogger->SetLevel(logg::Logger::Level::TRACE);
}

void shTerminateLogger() noexcept
{
    logg::TerminateLogSystem();
    s_isInitialized = false;
}


logg::Logger* shGetLogger() noexcept
{
    return logg::LogSystem::GetInstance().GetLogger<ShaderGenLoggerTag>();
}

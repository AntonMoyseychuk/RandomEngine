#include "pch.h"
#include "eng_log_sys.h"

#include <cstdio>


#define AM_MAKE_COLORED_TEXT(color, text) color text ENG_OUTPUT_COLOR_RESET_ASCII_CODE


static constexpr const char* LOGGER_PATTERN = "[%^%L%$] [%n] [%H:%M:%S:%e]: %v";


void engInitLogSystem() noexcept
{
#if defined(ENG_LOGGING_ENABLED)
    if (!logg::InitLogSystem()) {
        puts(AM_MAKE_COLORED_TEXT(ENG_OUTPUT_COLOR_RED_ASCII_CODE, "Unexpected problems occurred during the initialization of the log system.\n"));
        return;
    }

    logg::LogSystem& logSystemInst = logg::LogSystem::GetInstance();

    logg::LoggerCreateInfo loggerCreateInfo = {};
    loggerCreateInfo.pattern = LOGGER_PATTERN;
    
    loggerCreateInfo.name = "CORE";
    logSystemInst.CreateLogger<EngineGeneralLoggerTag>(loggerCreateInfo);

    loggerCreateInfo.name = "WINDOW";
    logSystemInst.CreateLogger<EngineWindowLoggerTag>(loggerCreateInfo);

    loggerCreateInfo.name = "OPEN_GL";
    logSystemInst.CreateLogger<EngineGraphicsApiLoggerTag>(loggerCreateInfo);
#endif
}


void engTerminateLogSystem() noexcept
{
#if defined(ENG_LOGGING_ENABLED)
    logg::TerminateLogSystem();
#endif
}


bool engIsLogSystemInitialized() noexcept
{
#if defined(ENG_LOGGING_ENABLED)
    return logg::IsLogSystemInitialized();
#else
    return true;
#endif
}
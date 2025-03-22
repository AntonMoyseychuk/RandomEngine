#include "pch.h"
#include "eng_log_sys.h"

#include <cstdio>


#define ENG_MAKE_COLORED_TEXT(color, text) color text ENG_OUTPUT_COLOR_RESET_ASCII_CODE


static constexpr const char* ENG_LOGGER_PATTERN = "[%l] [%n] [%H:%M:%S:%e]: %^%v%$";


void engInitLogSystem() noexcept
{
#if defined(ENG_LOGGING_ENABLED)
    if (!logg::InitLogSystem()) {
        puts(ENG_MAKE_COLORED_TEXT(ENG_OUTPUT_COLOR_RED_ASCII_CODE, "Unexpected problems occurred during the initialization of the log system.\n"));
        return;
    }

    logg::LogSystem& logSystemInst = logg::LogSystem::GetInstance();
    
    logg::Logger* pCoreLogger = logSystemInst.CreateLogger<EngineGeneralLoggerTag>("CORE");
    pCoreLogger->SetPattern(ENG_LOGGER_PATTERN);
    pCoreLogger->SetLevel(logg::Logger::Level::TRACE);

    logg::Logger* pWndLogger = logSystemInst.CreateLogger<EngineWindowLoggerTag>("WINDOW");
    pWndLogger->SetPattern(ENG_LOGGER_PATTERN);
    pWndLogger->SetLevel(logg::Logger::Level::TRACE);

    logg::Logger* pGraphicsApiLogger = logSystemInst.CreateLogger<EngineGraphicsApiLoggerTag>("OPEN_GL");
    pGraphicsApiLogger->SetPattern(ENG_LOGGER_PATTERN);
    pGraphicsApiLogger->SetLevel(logg::Logger::Level::TRACE);
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
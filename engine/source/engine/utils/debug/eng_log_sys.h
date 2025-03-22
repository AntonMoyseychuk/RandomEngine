#pragma once

#include "core.h"
#include "utils/log_system/log_system.h"


#define ENG_OUTPUT_COLOR_RESET_ASCII_CODE      "\033[0m"
#define ENG_OUTPUT_COLOR_BLACK_ASCII_CODE      "\033[30m"
#define ENG_OUTPUT_COLOR_RED_ASCII_CODE        "\033[31m"
#define ENG_OUTPUT_COLOR_GREEN_ASCII_CODE      "\033[32m"
#define ENG_OUTPUT_COLOR_YELLOW_ASCII_CODE     "\033[33m"
#define ENG_OUTPUT_COLOR_BLUE_ASCII_CODE       "\033[34m"
#define ENG_OUTPUT_COLOR_MAGENTA_ASCII_CODE    "\033[35m"
#define ENG_OUTPUT_COLOR_CYAN_ASCII_CODE       "\033[36m"
#define ENG_OUTPUT_COLOR_WHITE_ASCII_CODE      "\033[37m"


void engInitLogSystem() noexcept;
void engTerminateLogSystem() noexcept;

bool engIsLogSystemInitialized() noexcept;


template <typename TAG>
inline logg::Logger* engGetTagedLogger() noexcept
{
    return engIsLogSystemInitialized() ? logg::LogSystem::GetInstance().GetLogger<TAG>() : nullptr;
}


struct EngineGeneralLoggerTag {};
struct EngineWindowLoggerTag {};
struct EngineGraphicsApiLoggerTag {};


#if defined(ENG_LOGGING_ENABLED)
    #define ENG_LOG_TRACE(format, ...)  engGetTagedLogger<EngineGeneralLoggerTag>()->Trace(format, __VA_ARGS__)
    #define ENG_LOG_DEBUG(format, ...)  engGetTagedLogger<EngineGeneralLoggerTag>()->Debug(format, __VA_ARGS__)
    #define ENG_LOG_INFO(format, ...)  engGetTagedLogger<EngineGeneralLoggerTag>()->Info(format, __VA_ARGS__)
    #define ENG_LOG_WARN(format, ...)  engGetTagedLogger<EngineGeneralLoggerTag>()->Warn(format, __VA_ARGS__)
    #define ENG_LOG_ERROR(format, ...) engGetTagedLogger<EngineGeneralLoggerTag>()->Error(format, __VA_ARGS__)
    #define ENG_LOG_CRITICAL(format, ...) engGetTagedLogger<EngineGeneralLoggerTag>()->Critical(format, __VA_ARGS__)

    #define ENG_LOG_WINDOW_TRACE(format, ...)  engGetTagedLogger<EngineWindowLoggerTag>()->Trace(format, __VA_ARGS__)
    #define ENG_LOG_WINDOW_DEBUG(format, ...)  engGetTagedLogger<EngineWindowLoggerTag>()->Debug(format, __VA_ARGS__)
    #define ENG_LOG_WINDOW_INFO(format, ...)  engGetTagedLogger<EngineWindowLoggerTag>()->Info(format, __VA_ARGS__)
    #define ENG_LOG_WINDOW_WARN(format, ...)  engGetTagedLogger<EngineWindowLoggerTag>()->Warn(format, __VA_ARGS__)
    #define ENG_LOG_WINDOW_ERROR(format, ...) engGetTagedLogger<EngineWindowLoggerTag>()->Error(format, __VA_ARGS__)
    #define ENG_LOG_WINDOW_CRITICAL(format, ...) engGetTagedLogger<EngineWindowLoggerTag>()->Critical(format, __VA_ARGS__)

    #define ENG_LOG_GRAPHICS_API_TRACE(format, ...)  engGetTagedLogger<EngineGraphicsApiLoggerTag>()->Trace(format, __VA_ARGS__)
    #define ENG_LOG_GRAPHICS_API_DEBUG(format, ...)  engGetTagedLogger<EngineGraphicsApiLoggerTag>()->Debug(format, __VA_ARGS__)
    #define ENG_LOG_GRAPHICS_API_INFO(format, ...)  engGetTagedLogger<EngineGraphicsApiLoggerTag>()->Info(format, __VA_ARGS__)
    #define ENG_LOG_GRAPHICS_API_WARN(format, ...)  engGetTagedLogger<EngineGraphicsApiLoggerTag>()->Warn(format, __VA_ARGS__)
    #define ENG_LOG_GRAPHICS_API_ERROR(format, ...) engGetTagedLogger<EngineGraphicsApiLoggerTag>()->Error(format, __VA_ARGS__)
    #define ENG_LOG_GRAPHICS_API_CRITICAL(format, ...) engGetTagedLogger<EngineGraphicsApiLoggerTag>()->Critical(format, __VA_ARGS__)
#else
    #define ENG_LOG_TRACE(format, ...)
    #define ENG_LOG_DEBUG(format, ...)
    #define ENG_LOG_INFO(format, ...)
    #define ENG_LOG_WARN(format, ...)
    #define ENG_LOG_ERROR(format, ...)
    #define ENG_LOG_CRITICAL(format, ...)

    #define ENG_LOG_WINDOW_TRACE(format, ...)
    #define ENG_LOG_WINDOW_DEBUG(format, ...)
    #define ENG_LOG_WINDOW_INFO(format, ...)
    #define ENG_LOG_WINDOW_WARN(format, ...)
    #define ENG_LOG_WINDOW_ERROR(format, ...)
    #define ENG_LOG_WINDOW_CRITICAL(format, ...)

    #define ENG_LOG_GRAPHICS_API_TRACE(format, ...)
    #define ENG_LOG_GRAPHICS_API_DEBUG(format, ...)
    #define ENG_LOG_GRAPHICS_API_INFO(format, ...)
    #define ENG_LOG_GRAPHICS_API_WARN(format, ...)
    #define ENG_LOG_GRAPHICS_API_ERROR(format, ...)
    #define ENG_LOG_GRAPHICS_API_CRITICAL(format, ...)
#endif
#pragma once

#include "log_system/log_system.h"


void shInitLogger() noexcept;
void shTerminateLogger() noexcept;

logg::Logger* shGetLogger() noexcept;


#define SH_LOG_TRACE(format, ...)  shGetLogger()->Trace(format, __VA_ARGS__)
#define SH_LOG_DEBUG(format, ...)  shGetLogger()->Debug(format, __VA_ARGS__)
#define SH_LOG_INFO(format, ...)  shGetLogger()->Info(format, __VA_ARGS__)
#define SH_LOG_WARN(format, ...)  shGetLogger()->Warn(format, __VA_ARGS__)
#define SH_LOG_ERROR(format, ...) shGetLogger()->Error(format, __VA_ARGS__)
#define SH_LOG_CRITICAL(format, ...) shGetLogger()->Critical(format, __VA_ARGS__)
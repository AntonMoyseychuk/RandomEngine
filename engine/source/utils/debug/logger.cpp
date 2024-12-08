#include "pch.h"

#include "logger.h"


using LoggerTypeStrArray = std::array<const char*, (size_t)Logger::Type::COUNT>;


static constexpr const char* LoggerTypeToStr(Logger::Type type) noexcept
{
    switch (type) {
        case Logger::Type::APPLICATION:
            return "APP";
        case Logger::Type::GRAPHICS_API:
            return "OPEN_GL";
        case Logger::Type::WINDOW_SYSTEM:
            return "WINDOW";
        default:
            return "INVALID";
    }
}


static LoggerTypeStrArray GetLoggerTypeStrs() noexcept
{
    LoggerTypeStrArray typeStrs;

    for (size_t type = 0; type < static_cast<size_t>(Logger::Type::COUNT); ++type) {
        typeStrs[type] = LoggerTypeToStr(static_cast<Logger::Type>(type));
    }

    return typeStrs;
}


static constexpr const char* AM_LOGGER_PATTERN = "[%^%L%$] [%n] [%H:%M:%S:%e]: %v"; 


std::shared_ptr<spdlog::logger> Logger::CreateDefaultSpdlogger() noexcept
{
    if (s_pDefaultLogger) {
        return s_pDefaultLogger;
    }

    static const std::string DEFAULT_LOGGER_NAME = "DEFAULT";
    static const std::string DEFAULT_LOGGER_PATTERN = "[%^%L%$] [%n] [%H:%M:%S:%e]: %v";

    std::shared_ptr<spdlog::logger> logger = spdlog::stdout_color_st(DEFAULT_LOGGER_NAME);
    logger->set_pattern(DEFAULT_LOGGER_PATTERN);

    return logger;
}


Logger* Logger::Instance() noexcept
{
    return s_pInst.get();
}

std::shared_ptr<spdlog::logger> Logger::GetDefaultLogger() noexcept
{
    return s_pDefaultLogger;
}


bool Logger::Init()
{
    if (IsInitialized()) {
        ENG_LOG_WARN("Log system is already initialized");
        return true;
    }

    s_pDefaultLogger = CreateDefaultSpdlogger();

    s_pInst = std::unique_ptr<Logger>(new Logger);

    return s_pInst != nullptr;
}


void Logger::Terminate() noexcept
{
    s_pDefaultLogger = nullptr;
    s_pInst = nullptr;
}


bool Logger::IsInitialized() noexcept
{
    return s_pInst != nullptr && s_pDefaultLogger != nullptr;
}


Logger::Logger()
{
    m_loggers.reserve((size_t)Logger::Type::COUNT);
    
    for (const char* pLoggerName : GetLoggerTypeStrs()) {
        std::shared_ptr<spdlog::logger> logger = spdlog::stdout_color_st(pLoggerName);
        logger->set_pattern(AM_LOGGER_PATTERN);

        m_loggers.emplace_back(logger);
    }
}


bool Logger::IsCustomLogger(Type type) const noexcept
{
    return (size_t)type < m_loggers.size() && m_loggers[(size_t)type] != nullptr;
}


void engInitLogSystem() noexcept
{
#if defined(ENG_LOGGING_ENABLED)
    if (!Logger::Init()) {
        ENG_LOG_WARN("Unexpected problems occurred during the initialization of the log system. Using default logger.\n");
    }
#endif
}


void engTerminateLogSystem() noexcept
{
#if defined(ENG_LOGGING_ENABLED)
    Logger::Terminate();
#endif
}


bool engIsLogSystemInitialized() noexcept
{
#if defined(ENG_LOGGING_ENABLED)
    return Logger::IsInitialized();
#else
    return true;
#endif
}

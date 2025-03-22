#include "log_system.h"

#include <algorithm>


namespace logg
{
    static spdlog::level::level_enum LoggerLevelToSPDLogLevel(Logger::Level level)
    {
        switch(level) {
            case Logger::Level::TRACE: return spdlog::level::trace;
            case Logger::Level::DEBUG: return spdlog::level::debug;
            case Logger::Level::INFO: return spdlog::level::info;
            case Logger::Level::WARN: return spdlog::level::warn;
            case Logger::Level::ERROR: return spdlog::level::err;
            case Logger::Level::CRITICAL: return spdlog::level::critical;
            case Logger::Level::OFF: return spdlog::level::off;
            default: return spdlog::level::critical;
        }
    }


    static constexpr uint64_t MAX_LOGGERS_COUNT = 32;
    std::unique_ptr<LogSystem> pLogSystemInst = nullptr;


    Logger::~Logger()
    {
        Destroy();
    }


    bool Logger::IsValid() const noexcept
    {
        return m_pSpdLoggerInst != nullptr && m_index != INVALID_IDX;
    }

    
    void Logger::SetPattern(std::string pattern) noexcept
    {
        if (IsValid()) {
            m_pSpdLoggerInst->set_pattern(pattern);
        }
    }

    
    void Logger::SetLevel(Level level) noexcept
    {
        if (IsValid()) {
            m_pSpdLoggerInst->set_level(LoggerLevelToSPDLogLevel(level));
        }
    }


    bool Logger::Create(const std::string& name) noexcept
    {
        if (IsValid()) {
            return true;
        }

        assert(m_index != INVALID_IDX);
        m_pSpdLoggerInst = spdlog::stdout_color_mt(name);

        return m_pSpdLoggerInst != nullptr;
    }
    
    
    void Logger::Destroy() noexcept
    {
        m_pSpdLoggerInst = nullptr;
    }
    

    LogSystem& LogSystem::GetInstance() noexcept
    {
        return *pLogSystemInst;
    }
    
    
    void LogSystem::DestroyLogger(Logger& logger) noexcept
    {
        if (!IsInitialized()) {
            return;
        }

        if (!logger.IsValid()) {
            return;
        }

        const uint64_t index = logger.m_index;

        Logger* pLogger = &m_loggerStorage[index];
        assert(pLogger == &logger);
            
        pLogger->Destroy();
        pLogger->InvalidateIndex();
    }


    bool LogSystem::Init() noexcept
    {
        if (IsInitialized()) {
            return true;
        }

        m_loggerStorage.resize(MAX_LOGGERS_COUNT);
        m_isInitalized = true;

        return true;
    }

    
    void LogSystem::Terminate() noexcept
    {
        m_loggerStorage.clear();
        m_isInitalized = false;
    }


    bool InitLogSystem() noexcept
    {
        if (IsLogSystemInitialized()) {
            return true;
        }

        pLogSystemInst = std::unique_ptr<LogSystem>(new LogSystem);
        
        return pLogSystemInst && pLogSystemInst->Init();
    }
    
    
    void TerminateLogSystem() noexcept
    {
        pLogSystemInst = nullptr;
    }


    bool IsLogSystemInitialized() noexcept
    {
        return pLogSystemInst && pLogSystemInst->IsInitialized();
    }
}
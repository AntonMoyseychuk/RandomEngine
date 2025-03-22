#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <vector>
#include <array>
#include <cstdint>
#include <memory>
#include <string>


namespace logg
{
    struct LoggerCreateInfo
    {
        std::string name;
        std::string pattern;
    };


    class Logger
    {
        friend class LogSystem;

    public:
        Logger() = default;
        ~Logger();

        Logger(const Logger& logger) = delete;
        Logger& operator=(const Logger& logger) = delete;

        Logger(Logger&& logger) noexcept = default;
        Logger& operator=(Logger&& logger) noexcept = default;

        bool IsValid() const noexcept;

        void SetPattern(std::string pattern) noexcept;

        template <typename... Args>
        void Info(std::string_view format, Args&&... args) noexcept;

        template <typename... Args>
        void Warn(std::string_view format, Args&&... args) noexcept;

        template <typename... Args>
        void Error(std::string_view format, Args&&... args) noexcept;

    private:
        bool Create(const LoggerCreateInfo& createInfo) noexcept;
        void Destroy() noexcept;

        void SetIndex(uint64_t index) noexcept { m_index = index; }
        uint64_t GetIndex() const noexcept { return m_index; }
        void InvalidateIndex() noexcept { m_index = INVALID_IDX; }

    private:
        static inline constexpr uint64_t MAX_NAME_LENGTH = 63Ui64;    
        static inline constexpr uint64_t NAME_BUFFER_SIZE = MAX_NAME_LENGTH + 1Ui64;
        static inline constexpr uint64_t INVALID_IDX = UINT64_MAX;

    private:
        std::shared_ptr<spdlog::logger> m_pSpdLoggerInst = nullptr;
        uint64_t m_index = INVALID_IDX;
    };


    class LogSystem
    {
        friend bool InitLogSystem() noexcept;
        friend void TerminateLogSystem() noexcept;
        friend bool IsLogSystemInitialized() noexcept;

    public:
        static LogSystem& GetInstance() noexcept;

    public:
        LogSystem(const LogSystem& other) = delete;
        LogSystem& operator=(const LogSystem& other) = delete;
        LogSystem(LogSystem&& other) noexcept = delete;
        LogSystem& operator=(LogSystem&& other) noexcept = delete;

        template <typename TAG>
        Logger* CreateLogger(const LoggerCreateInfo& createInfo) noexcept;

        void DestroyLogger(Logger& logger) noexcept;

        template <typename TAG>
        Logger* GetLogger() noexcept;

        bool IsInitialized() const noexcept { return m_isInitalized; }

    private:
        static uint64_t AllocateLoggerTagIndex() noexcept;

        template <typename TAG>
        static uint64_t GetLoggerTagIndex() noexcept;

    private:
        LogSystem() = default;

        bool Init() noexcept;
        void Terminate() noexcept;

    private:
        std::vector<Logger> m_loggerStorage;

        bool m_isInitalized = false;
    };


    bool InitLogSystem() noexcept;
    void TerminateLogSystem() noexcept;
    
    bool IsLogSystemInitialized() noexcept;
}

#include "log_system.hpp"
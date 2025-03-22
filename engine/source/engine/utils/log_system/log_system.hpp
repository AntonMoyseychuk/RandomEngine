namespace logg
{
    template <typename... Args>
    inline void Logger::Info(std::string_view format, Args &&...args) noexcept
    {
        assert(IsValid());
        m_pSpdLoggerInst->info(format, std::forward<Args>(args)...);
    }


    template <typename... Args>
    inline void Logger::Warn(std::string_view format, Args &&...args) noexcept
    {
        assert(IsValid());
        m_pSpdLoggerInst->warn(format, std::forward<Args>(args)...);
    }


    template <typename... Args>
    inline void Logger::Error(std::string_view format, Args &&...args) noexcept
    {
        assert(IsValid());
        m_pSpdLoggerInst->error(format, std::forward<Args>(args)...);
    }
    
    
    template <typename TAG>
    inline Logger* LogSystem::CreateLogger(const LoggerCreateInfo &createInfo) noexcept
    {
        if (!IsInitialized()) {
            return nullptr;
        }

        const uint64_t index = GetLoggerTagIndex<TAG>();
        assert(index < m_loggerStorage.size());

        Logger* pLogger = &m_loggerStorage[index];
        
        pLogger->SetIndex(index);
        
        if (pLogger->Create(createInfo)) {
            return pLogger;
        }

        pLogger->Destroy();
        pLogger->InvalidateIndex();

        return nullptr;
    }


    template <typename TAG>
    inline Logger* LogSystem::GetLogger() noexcept
    {
        const uint64_t index = GetLoggerTagIndex<TAG>();
        assert(index < m_loggerStorage.size());

        Logger* pLogger = &m_loggerStorage[index];
        assert(pLogger->IsValid());

        return pLogger;
    }


    inline uint64_t LogSystem::AllocateLoggerTagIndex() noexcept
    {
        static uint64_t index = 0;
        return index++;
    }


    template <typename TAG>
    inline uint64_t LogSystem::GetLoggerTagIndex() noexcept
    {
        static uint64_t index = AllocateLoggerTagIndex();
        return index;
    }
}
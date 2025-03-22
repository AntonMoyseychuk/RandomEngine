#pragma once

#include "eng_log_sys.h"


namespace detail
{
    template <typename LoggerTag, typename... Args>
    void AssertImpl(const char* pFile, uint64_t line, std::string_view format, Args&&... args) noexcept;
}

#if defined(ENG_ASSERTION_ENABLED)
    #define ENG_ASSERT(condition, format, ...) \
        if (!(condition)) { \
            detail::AssertImpl<EngineGeneralLoggerTag>(__FILE__, __LINE__, format, __VA_ARGS__); \
        }

    #define ENG_ASSERT_GRAPHICS_API(condition, format, ...) \
        if (!(condition)) { \
            detail::AssertImpl<EngineGraphicsApiLoggerTag>(__FILE__, __LINE__, format, __VA_ARGS__); \
        }

    #define ENG_ASSERT_WINDOW(condition, format, ...) \
        if (!(condition)) { \
            detail::AssertImpl<EngineWindowLoggerTag>(__FILE__, __LINE__, format, __VA_ARGS__); \
        }
    
    #define ENG_ASSERT_FAIL(format, ...)              ENG_ASSERT(false, format, __VA_ARGS__)
    #define ENG_ASSERT_GRAPHICS_API_FAIL(format, ...) ENG_ASSERT_GRAPHICS_API(false, format, __VA_ARGS__)
    #define ENG_ASSERT_WINDOW_FAIL(format, ...)       ENG_ASSERT_WINDOW(false, format, __VA_ARGS__)
#else
    #define ENG_ASSERT(condition, formatt, ...)
    #define ENG_ASSERT_GRAPHICS_API(condition, formatt, ...)
    #define ENG_ASSERT_WINDOW(condition, formatt, ...)

    #define ENG_ASSERT_FAIL(formatt, ...)
    #define ENG_ASSERT_GRAPHICS_API_FAIL(formatt, ...)
    #define ENG_ASSERT_WINDOW_FAIL(formatt, ...)
#endif


namespace detail
{
    template <typename LoggerTag, typename... Args>
    inline void AssertImpl(const char* pFile, uint64_t line, std::string_view format, Args &&...args) noexcept
    {
        logg::Logger* pLogger = engGetTagedLogger<LoggerTag>();

        static constexpr uint64_t MAX_FORMAT_LENGTH = 1024;
        char pNewFormat[MAX_FORMAT_LENGTH] = { 0 };
        sprintf_s(pNewFormat, "%s [{}:{}]", format.data());

        pLogger->Critical(pNewFormat, std::forward<Args>(args)..., pFile, line);
        ENG_DEBUG_BREAK();
    }
}
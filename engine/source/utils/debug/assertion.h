#pragma once

#include "logger.h"


void AssertImpl(bool condition, Logger::Type loggerType, const char* file, const char* function, uint32_t line, const char* conditionStr, const char* message);


#if defined(ENG_ASSERTION_ENABLED)
    #define ENG_ASSERT(condition, formatt, ...)              AssertImpl(condition, Logger::Type::APPLICATION, __FILE__, __FUNCTION__, __LINE__, "condtion: " #condition, fmt::format(formatt, __VA_ARGS__).c_str())
    #define ENG_ASSERT_GRAPHICS_API(condition, formatt, ...) AssertImpl(condition, Logger::Type::GRAPHICS_API, __FILE__, __FUNCTION__, __LINE__, "condtion: " #condition, fmt::format(formatt, __VA_ARGS__).c_str())
    #define ENG_ASSERT_WINDOW(condition, formatt, ...)       AssertImpl(condition, Logger::Type::WINDOW_SYSTEM, __FILE__, __FUNCTION__, __LINE__, "condtion: " #condition, fmt::format(formatt, __VA_ARGS__).c_str())

    #define ENG_ASSERT_FAIL(formatt, ...)              ENG_ASSERT(false, formatt, __VA_ARGS__)
    #define ENG_ASSERT_GRAPHICS_API_FAIL(formatt, ...) ENG_ASSERT_GRAPHICS_API(false, formatt, __VA_ARGS__)
    #define ENG_ASSERT_WINDOW_FAIL(formatt, ...)       ENG_ASSERT_WINDOW(false, formatt, __VA_ARGS__)
#else
    #define ENG_ASSERT(condition, formatt, ...)
    #define ENG_ASSERT_GRAPHICS_API(condition, formatt, ...)
    #define ENG_ASSERT_WINDOW(condition, formatt, ...)

    #define ENG_ASSERT_FAIL(formatt, ...)
    #define ENG_ASSERT_GRAPHICS_API_FAIL(formatt, ...)
    #define ENG_ASSERT_WINDOW_FAIL(formatt, ...)
#endif
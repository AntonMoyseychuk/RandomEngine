#pragma once


#if defined(_WIN32) || defined(_WIN64)
  #define ENG_OS_WINDOWS
#else
  #error Currently, only Windows is supported
#endif


#if defined(NDEBUG)
  #define ENG_RELEASE
#else
  #define ENG_DEBUG
#endif


#if defined(ENG_DEBUG)
  #define ENG_ASSERTION_ENABLED
#endif

#if defined(ENG_DEBUG)
  #define ENG_LOGGING_ENABLED
#endif


#if defined(_MSC_VER)
  #define ENG_DEBUG_BREAK() __debugbreak()
#elif defined(__clang__)
  #define ENG_DEBUG_BREAK() __builtin_trap()
#else
  #error Currently, only MSVC and Clang are supported
#endif


#if defined(_MSC_VER) && !defined(__INLINING__)
  #define ENG_FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
  #if __cplusplus >= 201703L
    #define ENG_FORCE_INLINE [[nodiscard]] __attribute__((always_inline))
  #else
    #define ENG_FORCE_INLINE __attribute__((always_inline))
  #endif
#else
  #define ENG_FORCE_INLINE inline
#endif


#define ENG_PRAGMA_OPTIMIZE_OFF _Pragma("optimize(\"\", off)")
#define ENG_PRAGMA_OPTIMIZE_ON  _Pragma("optimize(\"\", on)")


#if __cplusplus == 201703L
  // From C++17
  #define ENG_MAYBE_UNUSED        [[maybe_unused]]
  #define ENG_DEPRECATED(message) [[deprecated(message)]]
#else
  #if __cplusplus == 201402L
    // From C++14
    #define ENG_DEPRECATED(message) [[deprecated(message)]]
  #else
    #define ENG_DEPRECATED(message)
  #endif

  #define ENG_MAYBE_UNUSED
#endif
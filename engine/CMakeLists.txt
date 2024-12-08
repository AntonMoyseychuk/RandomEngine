cmake_minimum_required(VERSION 3.29.3 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


project(engine LANGUAGES C CXX)

set(ENGINE_DIR ${PROJECT_SOURCE_DIR})
set(ENGINE_SOURCE_DIR ${ENGINE_DIR}/source)
set(ENGINE_INCLUDE_DIR ${ENGINE_DIR}/include)

set(ENGINE_THIRDPARTY_DIR ${ENGINE_DIR}/thirdparty)
set(ENGINE_THIRDPARTY_GLFW_DIR ${ENGINE_THIRDPARTY_DIR}/glfw)
set(ENGINE_THIRDPARTY_GLAD_DIR ${ENGINE_THIRDPARTY_DIR}/glad)
set(ENGINE_THIRDPARTY_SPDLOG_DIR ${ENGINE_THIRDPARTY_DIR}/spdlog)


add_subdirectory(${ENGINE_THIRDPARTY_GLFW_DIR})
add_subdirectory(${ENGINE_THIRDPARTY_GLAD_DIR})
add_subdirectory(${ENGINE_THIRDPARTY_SPDLOG_DIR})


file(GLOB_RECURSE ENGINE_SRC_FILES CONFIGURE_DEPENDS ${ENGINE_SOURCE_DIR}/*.cpp)

add_library(engine STATIC ${ENGINE_SRC_FILES})


target_precompile_headers(engine PRIVATE ${ENGINE_SOURCE_DIR}/pch.h)


target_compile_options(engine PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Wno-gnu-zero-variadic-macro-arguments -Wno-gnu-anonymous-struct -Wno-nested-anon-types>
)


target_include_directories(engine 
    PUBLIC ${ENGINE_INCLUDE_DIR}

    PRIVATE ${ENGINE_SOURCE_DIR}

    PRIVATE ${GLFW_INCLUDE_DIRS}
    PRIVATE ${GLAD_INCLUDE_DIRS}
    PRIVATE ${spdlog_INCLUDE_DIRS})


target_link_directories(engine
    PRIVATE ${spdlog_LIBRARY_DIRS})


target_link_libraries(engine PRIVATE glfw glad spdlog)
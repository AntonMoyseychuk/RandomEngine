cmake_minimum_required(VERSION 3.29.3 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


project(shadergen LANGUAGES CXX)

set(SHADERGEN_DIR ${PROJECT_SOURCE_DIR})
set(SHADERGEN_SOURCE_DIR ${SHADERGEN_DIR}/source)

file(GLOB_RECURSE SHADERGEN_SRC_FILES CONFIGURE_DEPENDS ${SHADERGEN_SOURCE_DIR}/*.cpp)

add_executable(shadergen ${SHADERGEN_SRC_FILES})


target_compile_options(shadergen PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Wno-gnu-zero-variadic-macro-arguments -Wno-gnu-anonymous-struct -Wno-nested-anon-types>
)
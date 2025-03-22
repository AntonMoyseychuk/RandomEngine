cmake_minimum_required(VERSION 3.29.3 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


project(shadergen LANGUAGES CXX)

set(SHADERGEN_DIR ${PROJECT_SOURCE_DIR})
set(SHADERGEN_SOURCE_DIR ${SHADERGEN_DIR}/source)

set(SHADERGEN_THIRDPARTY_DIR ${SHADERGEN_DIR}/thirdparty)
set(SHADERGEN_THIRDPARTY_LOG_SYS_DIR ${SHADERGEN_THIRDPARTY_DIR}/log_system)


add_subdirectory(${SHADERGEN_THIRDPARTY_LOG_SYS_DIR})


file(GLOB_RECURSE SHADERGEN_SRC_FILES CONFIGURE_DEPENDS 
    ${SHADERGEN_SOURCE_DIR}/*.cpp
    ${SHADERGEN_SOURCE_DIR}/*.h
    ${SHADERGEN_SOURCE_DIR}/*.hpp)

add_executable(shadergen ${SHADERGEN_SRC_FILES})

set(SHADERGEN_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin/tools/shadergen")

set_target_properties(shadergen
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${SHADERGEN_OUTPUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${SHADERGEN_OUTPUT_DIR}
)

set(SHADERGEN_OUTPUT_DIR ${SHADERGEN_OUTPUT_DIR} PARENT_SCOPE)

target_compile_options(shadergen PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Wno-gnu-zero-variadic-macro-arguments -Wno-gnu-anonymous-struct -Wno-nested-anon-types>
)

target_include_directories(shadergen
    PRIVATE ${SHADERGEN_SOURCE_DIR}
    PRIVATE ${SHADERGEN_THIRDPARTY_LOG_SYS_DIR}/include)

target_link_libraries(shadergen PRIVATE log_system)
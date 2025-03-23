cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


project(engine LANGUAGES C CXX)

set(ENGINE_DIR ${PROJECT_SOURCE_DIR})
set(ENGINE_SOURCE_DIR ${ENGINE_DIR}/source)
set(ENGINE_INCLUDE_DIR ${ENGINE_DIR}/include)


set(ENGINE_SHADERS_DIR ${ENGINE_SOURCE_DIR}/shaders)
set(ENGINE_SHADERS_SOURCE_DIR ${ENGINE_SHADERS_DIR}/source)
set(ENGINE_SHADERS_INCLUDE_DIR ${ENGINE_SHADERS_DIR}/include)

set(ENGINE_THIRDPARTY_DIR ${ENGINE_DIR}/thirdparty)
set(ENGINE_THIRDPARTY_GLAD_DIR ${ENGINE_THIRDPARTY_DIR}/glad)

set(ENGINE_TOOLS_DIR ${ENGINE_DIR}/tools)
set(ENGINE_SHADERGEN_DIR ${ENGINE_TOOLS_DIR}/shadergen)

add_subdirectory(${ENGINE_THIRDPARTY_GLAD_DIR})
add_subdirectory(${ENGINE_SHADERGEN_DIR})


include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
    log_system
    GIT_REPOSITORY https://github.com/AntonMoyseychuk/LogSystem.git
    GIT_TAG        "HEAD"
)
FetchContent_MakeAvailable(log_system)


# Shader files for c++ files auto generation
set(ENGINE_SHADER_AUTOGEN_FILES
    ${ENGINE_SHADERS_INCLUDE_DIR}/resource_constants.fx
    ${ENGINE_SHADERS_INCLUDE_DIR}/registers_common.fx
)

set(ENGINE_CXX_AUTO_FILES_OUTPUT_DIR "${ENGINE_SOURCE_DIR}/engine/auto")

if (NOT EXISTS ${ENGINE_CXX_AUTO_FILES_OUTPUT_DIR})
    file(MAKE_DIRECTORY ${ENGINE_CXX_AUTO_FILES_OUTPUT_DIR})
endif()


set(SHADERGEN_INPUT_ARGS)
set(ENGINE_CXX_AUTO_FILES)

foreach(FILE ${ENGINE_SHADER_AUTOGEN_FILES})
    get_filename_component(INPUT_FILE_NAME ${FILE} NAME_WE)

    set(OUTPUT_FILE_PATH ${ENGINE_CXX_AUTO_FILES_OUTPUT_DIR}/auto_${INPUT_FILE_NAME}.h)

    list(APPEND SHADERGEN_INPUT_ARGS "-i" "${FILE}" "-o" "${OUTPUT_FILE_PATH}")
    list(APPEND ENGINE_CXX_AUTO_FILES ${OUTPUT_FILE_PATH})
endforeach()


set(SHADERGEN_EXEC_FILEPATH ${SHADERGEN_OUTPUT_DIR}/shadergen)

add_custom_command(PRE_BUILD
    OUTPUT ${ENGINE_CXX_AUTO_FILES}
    COMMAND ${SHADERGEN_EXEC_FILEPATH}
    ARGS ${SHADERGEN_INPUT_ARGS}
    DEPENDS ${ENGINE_SHADER_AUTOGEN_FILES} ${SHADERGEN_EXEC_FILEPATH}
)


# Creating engine lib
file(GLOB_RECURSE ENGINE_SRC_FILES CONFIGURE_DEPENDS 
    ${ENGINE_SOURCE_DIR}/*.cpp
    ${ENGINE_SOURCE_DIR}/*.h
    ${ENGINE_SOURCE_DIR}/*.hpp)

add_library(engine STATIC ${ENGINE_SRC_FILES} ${ENGINE_CXX_AUTO_FILES})


target_compile_options(engine PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Wno-gnu-zero-variadic-macro-arguments -Wno-gnu-anonymous-struct -Wno-nested-anon-types>
)


# Setting target options
set(ENGINE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin/engine")

set_target_properties(engine
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${ENGINE_OUTPUT_DIR}
    LIBRARY_OUTPUT_DIRECTORY_DEBUG ${ENGINE_OUTPUT_DIR}
    LIBRARY_OUTPUT_DIRECTORY_RELEASE ${ENGINE_OUTPUT_DIR}
)


target_precompile_headers(engine PRIVATE ${ENGINE_SOURCE_DIR}/pch.h)


target_include_directories(engine 
    PUBLIC ${ENGINE_INCLUDE_DIR}

    PRIVATE ${ENGINE_SOURCE_DIR}
    PRIVATE ${ENGINE_SOURCE_DIR}/engine

    PRIVATE ${GLFW_INCLUDE_DIRS}
    PRIVATE ${GLAD_INCLUDE_DIRS}
    PRIVATE ${ENGINE_THIRDPARTY_LOG_SYS_DIR}/include)


target_link_libraries(engine PRIVATE glfw glad glm::glm log_system)


target_compile_definitions(engine 
    PRIVATE ENG_ENGINE_DIR="${ENGINE_DIR}"
    
    PRIVATE ${AM_GRAPHICS_API})
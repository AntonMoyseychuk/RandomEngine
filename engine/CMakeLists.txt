cmake_minimum_required(VERSION 3.29.3 FATAL_ERROR)

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
set(ENGINE_THIRDPARTY_GLFW_DIR ${ENGINE_THIRDPARTY_DIR}/glfw)
set(ENGINE_THIRDPARTY_GLAD_DIR ${ENGINE_THIRDPARTY_DIR}/glad)
set(ENGINE_THIRDPARTY_SPDLOG_DIR ${ENGINE_THIRDPARTY_DIR}/spdlog)


set(ENGINE_TOOLS_DIR ${ENGINE_DIR}/tools)
set(ENGINE_SHADERGEN_DIR ${ENGINE_TOOLS_DIR}/shadergen)


add_subdirectory(${ENGINE_THIRDPARTY_GLFW_DIR})
add_subdirectory(${ENGINE_THIRDPARTY_GLAD_DIR})
add_subdirectory(${ENGINE_THIRDPARTY_SPDLOG_DIR})

add_subdirectory(${ENGINE_SHADERGEN_DIR})


# Shader files for c++ files auto generation
set(ENGINE_SHADER_AUTOGEN_FILES
    ${ENGINE_SHADERS_INCLUDE_DIR}/registers_common.fx
)

set(ENGINE_CXX_AUTO_FILES_OUTPUT_DIR "${ENGINE_SOURCE_DIR}/engine/auto")

if (NOT EXISTS ${ENGINE_CXX_AUTO_FILES_OUTPUT_DIR})
    file(MAKE_DIRECTORY ${ENGINE_CXX_AUTO_FILES_OUTPUT_DIR})
endif()

set(ENGINE_CXX_AUTO_FILES)


set(SHADERGEN_EXEC_FILEPATH ${SHADERGEN_OUTPUT_DIR}/shadergen)

foreach(FILE ${ENGINE_SHADER_AUTOGEN_FILES})
    get_filename_component(FILE_NAME ${FILE} NAME_WE)

    add_custom_command(PRE_BUILD
        OUTPUT ${ENGINE_CXX_AUTO_FILES_OUTPUT_DIR}/auto_${FILE_NAME}.h
        COMMAND ${SHADERGEN_EXEC_FILEPATH}
        ARGS ${FILE} ${ENGINE_CXX_AUTO_FILES_OUTPUT_DIR}
        DEPENDS ${FILE}
        COMMENT "Generating ${FILE_NAME}.h..."
    )
    
    list(APPEND ENGINE_CXX_AUTO_FILES ${ENGINE_CXX_AUTO_FILES_OUTPUT_DIR}/auto_${FILE_NAME}.h)
endforeach()


# Creating engine lib
file(GLOB_RECURSE ENGINE_SRC_FILES CONFIGURE_DEPENDS 
    ${ENGINE_SOURCE_DIR}/*.cpp
    ${ENGINE_SOURCE_DIR}/*.h
    ${ENGINE_SOURCE_DIR}/*.hpp)

add_library(engine STATIC ${ENGINE_SRC_FILES} ${ENGINE_CXX_AUTO_FILES})

# Setting target options
set(ENGINE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin/engine")

set_target_properties(engine
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${ENGINE_OUTPUT_DIR}
)

set_target_properties(engine
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY_DEBUG ${ENGINE_OUTPUT_DIR}
    LIBRARY_OUTPUT_DIRECTORY_RELEASE ${ENGINE_OUTPUT_DIR}
)

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

target_link_directories(engine PRIVATE ${spdlog_LIBRARY_DIRS})

target_link_libraries(engine PRIVATE glfw glad spdlog)
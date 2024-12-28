cmake_minimum_required(VERSION 3.29.3 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


project(game LANGUAGES C CXX)


add_subdirectory(${PROJECT_SOURCE_DIR}/engine)

set(GAME_DIR ${PROJECT_SOURCE_DIR}/game)
set(GAME_SOURCE_DIR ${GAME_DIR}/source)


file(GLOB_RECURSE GAME_SRC_FILES CONFIGURE_DEPENDS 
    ${GAME_SOURCE_DIR}/*.cpp
    ${GAME_SOURCE_DIR}/*.h
    ${GAME_SOURCE_DIR}/*.hpp)

add_executable(game ${GAME_SRC_FILES})

# Setting target options
set(GAME_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin/game")

set_target_properties(game
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${GAME_OUTPUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${GAME_OUTPUT_DIR}
)

target_include_directories(game PRIVATE ${GAME_SOURCE_DIR})

target_link_libraries(game PRIVATE engine)

target_precompile_headers(game PRIVATE ${GAME_SOURCE_DIR}/pch.h)
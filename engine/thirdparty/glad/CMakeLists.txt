cmake_minimum_required(VERSION 3.29.3 FATAL_ERROR)

project(glad LANGUAGES C)


set(GLAD_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include)


file(GLOB_RECURSE GLAD_SRC_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.c)


add_library(glad STATIC ${GLAD_SRC_FILES})


target_include_directories(glad PUBLIC ${GLAD_INCLUDE_DIRS})
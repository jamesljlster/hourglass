cmake_minimum_required(VERSION 2.8)

# Set variables
set(PROJECT_NAME CMake_Template)

# Environment variables
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Installation directory")
set(BUILD_TEST OFF CACHE BOOL "Enable building test")

# Set project
project(${PROJECT_NAME} ${PROJECT_LANGUAGE})

# Include dependences cmake setting
include(${CMAKE_CURRENT_SOURCE_DIR}/deps/deps.cmake)

# Include library cmake setting
include(${CMAKE_CURRENT_SOURCE_DIR}/lib/lib.cmake)

# Include utility cmake setting
include(${CMAKE_CURRENT_SOURCE_DIR}/util/util.cmake)

# Build test
if(${BUILD_TEST})
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)
endif()


cmake_minimum_required(VERSION 2.8)

# Set variables
set(PROJECT_NAME hourglass)
set(PROJECT_LANGUAGE C CXX)

# Environment variables
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Installation directory")
set(BUILD_TEST OFF CACHE BOOL "Enable building test")

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Build confugure" FORCE)
if(DEFINED CMAKE_BUILD_TYPE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${CMAKE_CONFIGURATION_TYPES})
endif()
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

# Dependences paths
set(DEPS_PATHS
	${CMAKE_CURRENT_SOURCE_DIR}/deps/lstm/src
	)

# Library paths
set(LIB_PATHS
	${CMAKE_CURRENT_SOURCE_DIR}/lib/trasvc
	)

# Utility paths
set(UTIL_PATHS
	#${CMAKE_CURRENT_SOURCE_DIR}/util/util1
	#${CMAKE_CURRENT_SOURCE_DIR}/util/util2
	)

if(${BUILD_TEST})
	set(UTIL_PATHS ${UTIL_PATHS} ${CMAKE_CURRENT_SOURCE_DIR}/test)
endif()

# Set project
project(${PROJECT_NAME} ${PROJECT_LANGUAGE})

# Include cmake files
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/deps.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/lib.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/util.cmake)


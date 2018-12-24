cmake_minimum_required(VERSION 2.8)

# Disable in-source builds to prevent source tree corruption.
if(" ${CMAKE_SOURCE_DIR}" STREQUAL " ${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "\nFATAL: In-source builds are not allowed.\n"
        "You should create a separate directory for build files."
        )
endif()

# Set variables
set(PROJECT_NAME hourglass)
set(PROJECT_LANGUAGE C CXX)

# Environment variables
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Installation directory")
option(BUILD_TEST "Enable building test" OFF)
option(BUILD_HG_MONITOR "Enable building Hourglass Monitor" ON)
option(INSTALL_CONFIG "Install default configure files" OFF)

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
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/tcpmgr/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/args/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/laneft/src
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/SimplePID/src
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/ModConfig/src
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/CSV_DataProc/src
    )

# Library paths
set(LIB_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/trasvc
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/ftsvc
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/wclt
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/tracker
    )

# Utility paths
set(UTIL_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/util/trasvr
    ${CMAKE_CURRENT_SOURCE_DIR}/util/rand_model
    ${CMAKE_CURRENT_SOURCE_DIR}/util/pid_tracker
    ${CMAKE_CURRENT_SOURCE_DIR}/util/lstm_tracker
    ${CMAKE_CURRENT_SOURCE_DIR}/util/lstm_trainer
    ${CMAKE_CURRENT_SOURCE_DIR}/util/learner
    )

if(${BUILD_HG_MONITOR})
    set(UTIL_PATHS ${UTIL_PATHS} ${CMAKE_CURRENT_SOURCE_DIR}/util/hourglass_monitor)
endif()

if(${BUILD_TEST})
    set(UTIL_PATHS ${UTIL_PATHS} ${CMAKE_CURRENT_SOURCE_DIR}/test)
endif()

if(${INSTALL_CONFIG})
    set(UTIL_PATHS ${UTIL_PATHS} ${CMAKE_CURRENT_SOURCE_DIR}/config)
endif()

# Set project
project(${PROJECT_NAME} ${PROJECT_LANGUAGE})

# Include cmake files
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/deps.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/lib.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/util.cmake)


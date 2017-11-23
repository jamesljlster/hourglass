# Project Dependences Configuration

# Backup and set build type to release
set(CMAKE_BUILD_TYPE_BAK ${CMAKE_BUILD_TYPE})
set(CMAKE_BUILD_TYPE Release)

# Include subdirectories
include_directories(${DEPS_PATHS})

# Find other dependences
set(OpenCV_STATIC OFF CACHE BOOL "Using OpenCV static linking library")
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

find_path(SERIALPORT_INCLUDE_DIR libserialport.h
	"/usr/include"
	"/usr/local/include"
	)
find_library(SERIALPORT_LIB libserialport.a
	"/usr/lib"
	"/usr/local/lib"
	)
include_directories(${SERIALPORT_INCLUDE_DIR})

# Add subdirectory
foreach(DEPS_PATH ${DEPS_PATHS})
	add_subdirectory(${DEPS_PATH})
endforeach()

# Restore origin build type
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE_BAK})


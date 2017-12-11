# Minimal QT Build
# CMake cross-platform build system recipe
# 2010 Ryan Pavlik <rpavlik at iastate.edu> <abiryan at ryand.net>
# http://academic.cleardefinition.com
# Iowa State University HCI Graduate Program/VRAC

cmake_minimum_required(VERSION 2.6.2 FATAL_ERROR)

# Set package properties
project(qtExample)

###
# Perform build configuration of dependencies

# Set up QT4 and required components
find_package(Qt5 COMPONENTS QtCore QtGui QtOpenGL REQUIRED)
include(${QT_USE_FILE})

###
# All the project files
set(SOURCES
     main.cpp
     mywindow.cpp
     progressbar.cpp
     trainingdisplay.cpp
     TheApp.cpp)

set(HEADERS
     datastorage.h
     train.h)

set(MOCS
     mywindow.h
     progressbar.h
     TheApp.h)

set(UIS ui/TheApp.ui)

set(RESOURCES theappresources.qrc)

###
# Build and link the project

# Let QT pre-process the files and add the generated files to the source list
qt4_wrap_ui(GENERATED_SOURCES ${UIS})
qt4_add_resources(GENERATED_SOURCES ${RESOURCES})
qt4_wrap_cpp(GENERATED_SOURCES ${MOCS})

source_group("Generated Sources - Do Not Edit" FILES ${GENERATED_SOURCES})

# Make sure the compiler can find the pre-processed files from qt4_wrap_ui
include_directories("${CMAKE_BINARY_DIR}")

# Build the app!
add_executable(qt-example
     MACOSX_BUNDLE
     WIN32
     # source files that are actually built direclty
     ${SOURCES}
     ${GENERATED_SOURCES}

     # items included so they show up in your IDE
     ${HEADERS}
     ${UIS}
     ${MOCS}
     ${RESOURCES})

# Link the app!
target_link_libraries(qt-example ${QT_LIBRARIES})

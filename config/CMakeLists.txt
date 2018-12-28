cmake_minimum_required(VERSION 2.8)

# Install config files
file(GLOB PROJECT_FCLS ${CMAKE_CURRENT_SOURCE_DIR}/*.cfg)
install(FILES ${PROJECT_FCLS} DESTINATION "${CMAKE_INSTALL_PREFIX}/config")

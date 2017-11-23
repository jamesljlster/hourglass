
# Set paths
set(LIB_PATHS
	${CMAKE_CURRENT_SOURCE_DIR}/lib/lib1
	${CMAKE_CURRENT_SOURCE_DIR}/lib/lib2
	)

# Include subdirectories
include_directories(${LIB_PATHS})

# Add subdirectiories
foreach(LIB_PATH ${LIB_PATHS})
	add_subdirectory(${LIB_PATH})
endforeach()

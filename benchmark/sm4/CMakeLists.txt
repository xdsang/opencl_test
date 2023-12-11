#cmake less version
cmake_minimum_required(VERSION 3.10)

set(MODULE_NAME sm4)

set(inc
	${CMAKE_CURRENT_SOURCE_DIR}
    ../common/inc
    ../shared/inc
)

set(${MODULE_NAME}_SOURCES 
	main.cpp
    ../common/src/oclUtils.cpp
    ../shared/src/shrUtils.cpp
)

add_definitions(-DSM4_CL_FILE="${CMAKE_CURRENT_SOURCE_DIR}/sm4_inno.cl")
add_definitions("-g")
include_directories(${inc})
include(../../CMakeCommon.txt)

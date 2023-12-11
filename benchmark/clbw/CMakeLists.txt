#cmake less version
cmake_minimum_required(VERSION 3.20)

set(MODULE_NAME clbw)

set(inc
	${CMAKE_CURRENT_SOURCE_DIR}
	../common/inc
    ../shared/inc
)

set(${MODULE_NAME}_SOURCES 
    clbw.cpp
	../common/src/oclUtils.cpp
    ../shared/src/shrUtils.cpp
)

add_definitions(-DBandwidth_Kernels_CL="${CMAKE_CURRENT_SOURCE_DIR}/bandwidth_kernels.cl")
add_definitions("-g")
include_directories(${inc})
include(../../CMakeCommon.txt)

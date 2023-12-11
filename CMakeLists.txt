#cmake less version
cmake_minimum_required(VERSION 3.10)
project(benchmarks)

list(APPEND CLConform_LIBRARIES ${OPENCL_LIBRARIES})
if(ANDROID)
    list(APPEND CLConform_LIBRARIES m)
endif()
if(NOT DEFINED LINK_PTHREAD)
    if(ANDROID OR WIN32)
        set(LINK_PTHREAD OFF)
    else()
        set(LINK_PTHREAD ON)
    endif()
endif()
if(LINK_PTHREAD)
    list(APPEND CLConform_LIBRARIES pthread)
endif()

set(proj_root_path ${CMAKE_CURRENT_SOURCE_DIR})

# target options
option(BUILD_CLBW "Build clbw benchmark or not" ON)
option(BUILD_SM4  "Build sm4 benchmark or not" ON)

# benchmark
add_subdirectory(benchmark)

#ifndef __OPENCL_H__
#define __OPENCL_H__

// #define CL_TARGET_OPENCL_VERSION 200
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#define MAX_WG_SIZE 256

#include <string.h>

#include <CL/cl.h>
#include <CL/cl.hpp>

#define oclCheckError(_func_) {                         \
    cl_int result = _func_;                             \
    if (result != CL_SUCCESS)                           \
    std::cerr << getErrorString(result) << std::endl;   \
}

const char* getErrorString(cl_int error);

char *ReadKernelSourceFile(const char *filename, size_t *length);

cl_context Create_PDC(cl_device_id *device);

cl_platform_id GetPlatform(int id);

cl_device_id GetDevice(cl_platform_id platform);

cl_context CreateContext(cl_platform_id platform);

cl_command_queue CreateCommandQueue(cl_context context, cl_device_id *device);

bool CreateMemObjects_Copy(cl_context context, cl_mem memObjects[3], 
        unsigned int *key, int len, int threadsize);

bool CreateMemObjects_Map(cl_context context, cl_mem memObjects[3], unsigned char *key, int len);

cl_program CreateProgram(cl_context context, cl_device_id device, const char* fileName);

cl_kernel CreateKernel(cl_kernel kernel, cl_program program, cl_mem memObjects[3], const char *func);

void oclCleanup(cl_context context, cl_command_queue commandQueue,
        cl_mem memObjects[3], cl_program program, cl_kernel kernel);

typedef struct
{
    std::string deviceName;
    std::string driverVersion;

    uint numCUs;
    uint maxWGSize;
    uint64_t maxAllocSize;
    uint64_t maxGlobalSize;
    uint maxClockFreq;

    bool halfSupported;
    bool doubleSupported;
    cl_device_type  deviceType;

    // Test specific options
    uint gloalBWIters;
    uint64_t globalBWMaxSize;
    uint computeWgsPerCU;
    uint computeDPWgsPerCU;
    uint computeIters;
    uint transferBWIters;
    uint kernelLatencyIters;
    uint64_t transferBWMaxSize;
} device_info_t;

device_info_t getDeviceInfo(cl::Device &d);

void trimString(std::string &str);

inline float timeInUS(cl::Event &timeEvent)
{
    cl_ulong start = timeEvent.getProfilingInfo<CL_PROFILING_COMMAND_START>() / 1000;
    cl_ulong end = timeEvent.getProfilingInfo<CL_PROFILING_COMMAND_END>() / 1000;

    return (float)((int)end - (int)start);
}
#endif

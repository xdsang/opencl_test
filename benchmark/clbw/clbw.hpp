#pragma once
#include <stdio.h>
#include <iostream>
#include <vector>
#include <utility>
#include <mutex>
#include <atomic>
#include <limits>
#include <CL/cl.hpp>

#include "shrUtils.h"
#include "oclUtils.h"

#define CL_SUCCESS 0
#define CL_FAILED -1

static const char *helpStr =
    "\n clbw [OPTIONS]"
    "\n"
    "\n OPTIONS:"
    "\n  -p, --platform num          choose platform [default: all platforms]"
    "\n  -d, --device num            choose device [default: 0]"
    "\n  -tn, --testName name        choose test name"
    "\n  -bwi, --global-BWIters      selectively run global bandwidth test"
    "\n  -ms, --memory-size          specify the data size"
    "\n  -h, --help                  display help message"
    "\n";

class clBW
{
private:
    typedef enum _CLBWTYPE
    {
        CL_BW_TYPE_FLOAT = 0,
        CL_BW_TYPE_FLOAT2,
        CL_BW_TYPE_FLOAT4,
        CL_BW_TYPE_FLOAT8,
        CL_BW_TYPE_FLOAT16,
        CL_BW_TYPE_MAX
    } CLBWType;

    bool forcePlatform = false, forcePlatformName, forceDevice, forceDeviceName;
    bool forceTest = false, useEventTimer = false;
    int specifiedPlatform = 0, specifiedDevice = 0;
    char *specifiedPlatformName;
    char *specifiedDeviceName;
    char *specifiedTestName;
    uint bwItersl = 50, memSizeKB = 0;
    bool doubleCore;

    std::vector<cl::Platform> platforms;
    int m_platforms_size = 0;

    std::mutex m_mtx;
    std::atomic<int> m_threads_counter;
    int numItems;
    std::vector<std::pair<double, double>> m_timeVec;
public:
    clBW()
    {
        numItems = std::numeric_limits<int>::max();
    }
    ~clBW() { /* do nothing */ }
public:
    int getPlatform(uint idx);
    int getDevice(uint pla_id, uint dev_id);
    int getBWType(char *dataType);

    cl_ulong run_kernel(cl::CommandQueue& queue, cl::Kernel& kernel, int gsize, int lsize);
    cl::Program CreateProgram(cl::Context& context, cl::Device& device, std::string prefix, std::string name);
    int parseArgs(int argc, char **argv);
    int runGlobalBandwidthTest(int platformId, int typeId);
    int start();
};

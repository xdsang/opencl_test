#include <fstream>
#include <cmath>
#include <assert.h>
#include <thread>

#include "clbw.hpp"

#define FETCH_PER_WI 16
#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))

static const char *gDataType[] = {"v1", "v2", "v4", "v8", "v16"};

using namespace shr;

int clBW::parseArgs(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0))
        {
            std::cout << helpStr << std::endl;
            exit(0);
        }
        else if ((strcmp(argv[i], "-p") == 0) || (strcmp(argv[i], "--platform") == 0))
        {
            if ((i + 1) < argc)
            {
                forcePlatform = true;
                specifiedPlatform = strtoul(argv[i + 1], NULL, 0);
                i++;
            }
        }
        else if ((strcmp(argv[i], "-d") == 0) || (strcmp(argv[i], "--device") == 0))
        {
            if ((i + 1) < argc)
            {
                forceDevice = true;
                specifiedDevice = strtoul(argv[i + 1], NULL, 0);
                i++;
            }
        }
        else if ((strcmp(argv[i], "-pn") == 0) || (strcmp(argv[i], "--platformName") == 0))
        {
            if ((i + 1) < argc)
            {
                forcePlatformName = true;
                specifiedPlatformName = argv[i + 1];
                i++;
            }
        }
        else if ((strcmp(argv[i], "-dn") == 0) || (strcmp(argv[i], "--deviceName") == 0))
        {
            if ((i + 1) < argc)
            {
                forceDeviceName = true;
                specifiedDeviceName = argv[i + 1];
                i++;
            }
        }
        else if ((strcmp(argv[i], "-tn") == 0) || (strcmp(argv[i], "--testName") == 0))
        {
            if ((i + 1) < argc)
            {
                forceTest = true;
                specifiedTestName = argv[i + 1];
                i++;
            }
        }
        else if (strcmp(argv[i], "--use-event-timer") == 0)
        {
            useEventTimer = true;
        }
        else if (strcmp(argv[i], "-bwi") == 0 || strcmp(argv[i], "--global-BWIters") == 0)
        {
            bwItersl = strtoul(argv[i + 1], NULL, 0);
        }
        else if (strcmp(argv[i], "-ms") == 0 || strcmp(argv[i], "--memory-size") == 0)
        {
            memSizeKB = strtoul(argv[i + 1], NULL, 0);
        }
    }
    return 0;
}

cl_ulong clBW::run_kernel(cl::CommandQueue& queue, cl::Kernel& kernel, int gsize, int lsize)	// us
{
    float timed = 0;
    cl::NDRange global_size(gsize);
    cl::NDRange local_size(lsize);

    if (useEventTimer)
    {
        for (uint i = 0; i < bwItersl; i++)
        {
            cl::Event timeEvent;

            queue.enqueueNDRangeKernel(kernel, cl::NullRange, global_size, local_size, NULL, &timeEvent);
            queue.finish();
            timed += timeInUS(timeEvent);
        }
    }
    else
    {
        for (uint i = 0; i < bwItersl; i++)
        {
            queue.enqueueNDRangeKernel(kernel, cl::NullRange, global_size, local_size);
            queue.flush();
        }
        queue.finish();
    }

    return (timed / static_cast<float>(bwItersl));
}

cl::Program clBW::CreateProgram(cl::Context& context, cl::Device& device, std::string prefix, std::string name)
{
    // Read the program source
    std::ifstream sourceFile(name.c_str());
    std::string sourceCode(std::istreambuf_iterator<char>(sourceFile), (std::istreambuf_iterator<char>()));
    sourceCode = prefix + "\n" + sourceCode;
    cl::Program::Sources source;
    source.push_back({ sourceCode.c_str(), sourceCode.length() });

    // Make program from the source code
    cl::Program program(context, source);

    return program;
}

int clBW::getBWType(char *dataType)
{
    if (!forceTest)
    {
        memset(dataType, 1, CL_BW_TYPE_MAX*sizeof(char));
    }
    else
    {
        char *type;
        memset(dataType, 0, CL_BW_TYPE_MAX*sizeof(char));
        type = strtok(specifiedTestName, " ");
        while (type != NULL)
        {
            for (int i=0; i<CL_BW_TYPE_MAX; i++)
            {
                if (strcmp(gDataType[i], type) == 0)
                {
                    dataType[i] = 1;
                    break;
                }
            }
            type = strtok(NULL, " ");
        }
    }
    return 0;
}

int clBW::runGlobalBandwidthTest(int platformId, int typeId)
{
    std::vector<cl::Device> devices;
    platforms[platformId].getDevices(CL_DEVICE_TYPE_GPU, &devices);

    if (devices.empty())
    {
        std::cerr << "Error: No OpenCL devices found" << std::endl;
        return CL_FAILED;
    }
    auto device = devices[0];
    auto context = cl::Context(device);
    auto commandQueue = cl::CommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE);

    auto devInfo = getDeviceInfo(device);

    if (memSizeKB == 0)
    {
        uint64_t maxItems = devInfo.maxAllocSize / sizeof(float) / 4;
        int numItems_tmp = roundToMultipleOf(maxItems, (devInfo.maxWGSize * FETCH_PER_WI * 16), devInfo.globalBWMaxSize);
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            numItems = numItems_tmp < numItems? numItems_tmp : numItems;
        }
    }
    else
    {
        numItems = memSizeKB * 1024 / 4;
    }

    float *arr = new float[numItems];
    populateArray(arr, numItems);

    cl::Buffer input_buf = cl::Buffer(context, CL_MEM_READ_ONLY, (numItems * sizeof(float)));
    cl::Buffer output_buf = cl::Buffer(context, CL_MEM_WRITE_ONLY, (numItems * sizeof(float)));
    commandQueue.enqueueWriteBuffer(input_buf, CL_TRUE, 0, (numItems * sizeof(float)), arr);

    cl::Program program = CreateProgram(context, device, "", Bandwidth_Kernels_CL);

    m_mtx.lock();
    if (program.build({ device }) != CL_SUCCESS)
    {
        std::cerr << "Error: Kernel build failed" << std::endl;
        std::cerr << "Build log:\n" << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;
        exit(0);
    }
    m_mtx.unlock();

    char kernel_name_go[127] = {0};
    snprintf(kernel_name_go, sizeof(kernel_name_go)/sizeof(kernel_name_go[0]), 
            "%s%s%s", "global_bandwidth_", gDataType[typeId], "_global_offset");

    uint global_size, local_size;
    int data_type = pow(2, typeId);

    // data_type might be float, float2, float4...dispatch [numItems] floats into FETCH_PER_WI block
    global_size = (numItems / data_type) / FETCH_PER_WI;
    if (memSizeKB == 0) { local_size  = devInfo.maxWGSize; }
    else { local_size  = 16 / data_type; }

    cl::Kernel kernel_go(program, kernel_name_go);
    kernel_go.setArg(0, input_buf), kernel_go.setArg(1, output_buf);

    Timer timer;
    m_threads_counter.fetch_add(1);
    while (m_threads_counter.load() != m_platforms_size) { /* do nothing */ }

    double tick = timer.seconds();
    run_kernel(commandQueue, kernel_go, global_size, local_size);
    double tock = timer.seconds();

    double iElapse = tock - tick;
    printf("platformId: %d, tick: %f, tock: %f, iElapse: %f s\n", platformId, tick, tock, iElapse);

    if (forcePlatform == true)
    {
        m_timeVec[0] = std::make_pair(tick, tock);
    }
    else
    {
        m_timeVec[platformId] = std::make_pair(tick, tock);
    }

    if (arr) { delete [] arr; }

    return 0;
}

int clBW::start()
{
    cl::Platform::get(&platforms);
    if (platforms.empty())
    {
        std::cerr << "Error: No OpenCL platforms found" << std::endl;
        return CL_FAILED;
    }

    m_platforms_size = platforms.size();

    if (forcePlatform && specifiedPlatform >= m_platforms_size) 
    {
        printf("Platform id error!!!\n");
        return -1;
    }

    printf("Global memory bandwidth (GBPS), ");

    char bDataType[CL_BW_TYPE_MAX];
    getBWType(bDataType);

    if (forcePlatform)
    {
        printf("Using platform %d\n", specifiedPlatform);
        m_platforms_size = 1;
    }
    else
    {
        printf("Using all %d platform(s).\n", m_platforms_size);
    }

    m_timeVec.resize(m_platforms_size);

    for (size_t type = 0; type < CL_BW_TYPE_MAX; type++)
    {
        if (bDataType[type] == 0) continue;
        
        std::vector<std::thread> thread_vec;
        thread_vec.reserve(m_platforms_size);
        m_threads_counter = 0;
        
        for (int i = 0; i < m_platforms_size; i++)
        {
            if (forcePlatform == true)
            {
                thread_vec.emplace_back(std::thread(&clBW::runGlobalBandwidthTest, this, specifiedPlatform, type));
                break;
            }
            thread_vec.emplace_back(std::thread(&clBW::runGlobalBandwidthTest, this, i, type));
        }
        
        for (auto &thread: thread_vec) { thread.join(); }

        Timer timer;
        double seconds = timer.elapse(m_timeVec);
        seconds /= static_cast<float>(bwItersl);

        // compute GBytesPersecond
        auto GBytes          = (float)numItems * sizeof(float) * 2 * m_platforms_size/1024/1024/1024;
        auto GBytesPersecond = GBytes / seconds;

        auto data_type = (int)pow(2, type);
        printf("Data type: float%d, Data size: %0.2f GB, Run time: %f s, Bandwidth: %0.2f GB/s\n", data_type, GBytes, seconds, GBytesPersecond);
        printf("\n");
    }

    
    std::vector<cl::Device> devices;
    platforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);
    auto devInfo = getDeviceInfo(devices[0]);

    std::cout << "Driver version\t: " << devInfo.driverVersion << std::endl;
    std::cout << "Clock frequency\t: " << devInfo.maxClockFreq << " MHz" << std::endl;
    
    return CL_SUCCESS;
}

int main(int argc, char *argv[])
{
    clBW clbw;

    clbw.parseArgs(argc, argv);
    clbw.start();

    return 0;
}
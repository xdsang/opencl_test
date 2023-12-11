#include "oclUtils.h"
#include "shrUtils.h"
#include <iostream>

char *ReadKernelSourceFile(const char *filename, size_t *length)
{
    FILE *file = NULL;
    size_t sourceLength;
    char *sourceString;
    int ret;
    file = fopen(filename, "rb");
    if (file == NULL)
    {
        printf("%s at %d: Can't open %s\n", __FILE__, __LINE__ - 2, filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    sourceLength = ftell(file);
    fseek(file, 0, SEEK_SET);
    sourceString = (char*)malloc(sourceLength+1);

    ret = fread(sourceString, sourceLength, 1, file);
    if (ret == 0)
    {
        printf("%s at %d: Can't read Source %s\n", __FILE__, __LINE__ - 2, filename);
        return NULL;
    }

    fclose(file);
    if (length != 0)
    {
        *length = sourceLength;
    }
    sourceString[sourceLength] = '\0';
    
    return sourceString;
}

cl_context Create_PDC(cl_device_id *device)
{
    cl_int status;
    cl_uint numPlatforms;
    cl_platform_id firstPlatformID;
    cl_context context = NULL;

    status = clGetPlatformIDs(1, &firstPlatformID, &numPlatforms);
    if (status != CL_SUCCESS || numPlatforms <= 0)
    {
        printf("Failed to find any OpenCL platforms.\n");
        return NULL;
    }

    status = clGetDeviceIDs(firstPlatformID, CL_DEVICE_TYPE_GPU, 1, device, NULL);
    if (status != CL_SUCCESS)
    {
        printf("There is no GPU.\n");
        return NULL;
    }

    context = clCreateContext(NULL, 1, device, NULL, NULL, &status);
    if (status != CL_SUCCESS)
    {
        printf("Create context error.\n");
        return NULL;
    }

    return context;
}

cl_platform_id GetPlatform(int id)
{
    cl_int status;
    cl_uint num_platform;
    cl_platform_id platform;
    char platformName[100]; 

    status = clGetPlatformIDs(0, NULL, &num_platform);
    if (status != CL_SUCCESS || num_platform <= 0)
    {
        perror("Couldn't create platform");
        return NULL;
    }

    cl_platform_id* platforms = new cl_platform_id[num_platform];
    status = clGetPlatformIDs(num_platform, platforms, NULL);

    platform = platforms[id];

    status = clGetPlatformInfo(platform,
        CL_PLATFORM_VENDOR, sizeof(platformName), platformName, NULL);

    delete[] platforms;

    return platform;
}

cl_device_id GetDevice(cl_platform_id platform)
{
    cl_int status;
    cl_uint num_device;
    cl_device_id device;
    char deviceName[100];

    status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &num_device);
    if (status != CL_SUCCESS || num_device == 0)
    {
        printf("There is no GPU, trying GPU...\n");
        return NULL;
    }

    cl_device_id* devices = new cl_device_id[num_device];
    status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, num_device, devices, NULL);

    for (unsigned i = 0; i < num_device; i++)
    {
        status = clGetDeviceInfo(devices[i],
            CL_DEVICE_NAME, sizeof(deviceName), deviceName, NULL);

        device = devices[i];
    }

    delete [] devices;
    
    return device;
}

cl_context CreateContext(cl_platform_id platform)
{
    cl_int status;
    cl_context context = NULL;
    // size_t device_Size = 0;
    cl_context_properties prpos[] = 
    {
        CL_CONTEXT_PLATFORM, 
        (cl_context_properties)platform, 
        0
    };

    context = clCreateContextFromType(prpos, CL_DEVICE_TYPE_GPU, NULL, NULL, &status);
    if (status != CL_SUCCESS)
    {
        printf("Create Context Error\n");
    }

    return context;
}

cl_command_queue CreateCommandQueue(cl_context context, cl_device_id *device)
{
    cl_command_queue commandQueue = NULL;

    commandQueue = clCreateCommandQueue(context, device[0], CL_QUEUE_PROFILING_ENABLE, NULL);
    if (commandQueue == NULL)
    {
        printf("Failed to create commandqueue for Device 0\n");
        return NULL;
    }

    return commandQueue;
}

bool CreateMemObjects_Copy(cl_context context, cl_mem memObjects[3], 
        unsigned int *rk, int len, int threadsize)
{
    memObjects[0] = clCreateBuffer(context, CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR,
            sizeof(unsigned int) * 32,  rk, NULL);
    memObjects[1] = clCreateBuffer(context, CL_MEM_READ_ONLY  | CL_MEM_HOST_WRITE_ONLY,
            sizeof(char) * len * threadsize, NULL, NULL);
    memObjects[2] = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY,
            sizeof(char) * len * threadsize, NULL, NULL);

    if (memObjects[0] == NULL || memObjects[1] == NULL || memObjects[2] == NULL)
    {
        printf("Error Creating memory objects");
        return false;
    }

    return true;
}

bool CreateMemObjects_Map(cl_context context, cl_mem memObjects[3], unsigned char *key, int len)
{
    memObjects[0] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            sizeof(char)*16, key, NULL);
    memObjects[1]  = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
            sizeof(char)*len*16, NULL, NULL);
    memObjects[2] = clCreateBuffer(context, CL_MEM_READ_WRITE,
            sizeof(char)*len*16, NULL, NULL);

    if (memObjects[0] == NULL || memObjects[1] == NULL || memObjects[2] == NULL)
    {
        printf("Error Creating memory objects");
        return false;
    }

    return true;
}

cl_program CreateProgram(cl_context context, cl_device_id device, const char* fileName)
{
    cl_int status;
    cl_program program;
    size_t program_length;

    char *const source = ReadKernelSourceFile(fileName, &program_length);

    program = clCreateProgramWithSource(context, 1, (const char**)&source, NULL, NULL);
    if (program == NULL)
    {
        printf("Failed to create CL program from source.\n");
        return NULL;
    }

    status = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (status != CL_SUCCESS)
    {
        return NULL;
    }

    return program;
}

cl_kernel CreateKernel(cl_kernel kernel, cl_program program, cl_mem memObjects[3], const char *func)
{
    cl_int status;

    kernel = clCreateKernel(program, func, NULL);

    status = clSetKernelArg(kernel, 0, sizeof(cl_mem), &memObjects[0]);
    status |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &memObjects[1]);
    status |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &memObjects[2]);

    return kernel;
}

void oclCleanup(cl_context context, cl_command_queue commandQueue,
        cl_mem memObjects[3], cl_program program, cl_kernel kernel)
{
    for (int i = 0; i < 3; i++)
    {
        if (memObjects[i] != 0)
            clReleaseMemObject(memObjects[i]);
    }

    if (commandQueue != 0)
        clReleaseCommandQueue(commandQueue);
    
    if (program != 0)
        clReleaseProgram(program);

    if (kernel != 0)
        clReleaseKernel(kernel);

    if (context != 0)
        clReleaseContext(context);
}

inline const char* getErrorString(cl_int error)
{
    switch(error)
    {
    // run-time and JIT compiler errors
    case 0: return "CL_SUCCESS";
    case -1: return "CL_DEVICE_NOT_FOUND";
    case -2: return "CL_DEVICE_NOT_AVAILABLE";
    case -3: return "CL_COMPILER_NOT_AVAILABLE";
    case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
    case -5: return "CL_OUT_OF_RESOURCES";
    case -6: return "CL_OUT_OF_HOST_MEMORY";
    case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
    case -8: return "CL_MEM_COPY_OVERLAP";
    case -9: return "CL_IMAGE_FORMAT_MISMATCH";
    case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
    case -11: return "CL_BUILD_PROGRAM_FAILURE";
    case -12: return "CL_MAP_FAILURE";
    case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
    case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
    case -15: return "CL_COMPILE_PROGRAM_FAILURE";
    case -16: return "CL_LINKER_NOT_AVAILABLE";
    case -17: return "CL_LINK_PROGRAM_FAILURE";
    case -18: return "CL_DEVICE_PARTITION_FAILED";
    case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

    // compile-time errors
    case -30: return "CL_INVALID_VALUE";
    case -31: return "CL_INVALID_DEVICE_TYPE";
    case -32: return "CL_INVALID_PLATFORM";
    case -33: return "CL_INVALID_DEVICE";
    case -34: return "CL_INVALID_CONTEXT";
    case -35: return "CL_INVALID_QUEUE_PROPERTIES";
    case -36: return "CL_INVALID_COMMAND_QUEUE";
    case -37: return "CL_INVALID_HOST_PTR";
    case -38: return "CL_INVALID_MEM_OBJECT";
    case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
    case -40: return "CL_INVALID_IMAGE_SIZE";
    case -41: return "CL_INVALID_SAMPLER";
    case -42: return "CL_INVALID_BINARY";
    case -43: return "CL_INVALID_BUILD_OPTIONS";
    case -44: return "CL_INVALID_PROGRAM";
    case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
    case -46: return "CL_INVALID_KERNEL_NAME";
    case -47: return "CL_INVALID_KERNEL_DEFINITION";
    case -48: return "CL_INVALID_KERNEL";
    case -49: return "CL_INVALID_ARG_INDEX";
    case -50: return "CL_INVALID_ARG_VALUE";
    case -51: return "CL_INVALID_ARG_SIZE";
    case -52: return "CL_INVALID_KERNEL_ARGS";
    case -53: return "CL_INVALID_WORK_DIMENSION";
    case -54: return "CL_INVALID_WORK_GROUP_SIZE";
    case -55: return "CL_INVALID_WORK_ITEM_SIZE";
    case -56: return "CL_INVALID_GLOBAL_OFFSET";
    case -57: return "CL_INVALID_EVENT_WAIT_LIST";
    case -58: return "CL_INVALID_EVENT";
    case -59: return "CL_INVALID_OPERATION";
    case -60: return "CL_INVALID_GL_OBJECT";
    case -61: return "CL_INVALID_BUFFER_SIZE";
    case -62: return "CL_INVALID_MIP_LEVEL";
    case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
    case -64: return "CL_INVALID_PROPERTY";
    case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
    case -66: return "CL_INVALID_COMPILER_OPTIONS";
    case -67: return "CL_INVALID_LINKER_OPTIONS";
    case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";

    // extension errors
    case -1000: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
    case -1001: return "CL_PLATFORM_NOT_FOUND_KHR";
    case -1002: return "CL_INVALID_D3D10_DEVICE_KHR";
    case -1003: return "CL_INVALID_D3D10_RESOURCE_KHR";
    case -1004: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
    case -1005: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
    default: return (std::string("Unknown OpenCL error:") + std::to_string(error)).c_str();
    }
}

device_info_t getDeviceInfo(cl::Device &d)
{
    device_info_t devInfo;

    devInfo.deviceName = d.getInfo<CL_DEVICE_NAME>();
    devInfo.driverVersion = d.getInfo<CL_DRIVER_VERSION>();
    trimString(devInfo.deviceName);
    trimString(devInfo.driverVersion);

    devInfo.numCUs = (uint)d.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
    std::vector<size_t> maxWIPerDim;
    maxWIPerDim = d.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>();
    devInfo.maxWGSize = (uint)maxWIPerDim[0];

    devInfo.maxWGSize = std::min(devInfo.maxWGSize, (uint)MAX_WG_SIZE);
    devInfo.maxAllocSize = static_cast<uint64_t>(d.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>());
    devInfo.maxGlobalSize = static_cast<uint64_t>(d.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>());
    devInfo.maxClockFreq = static_cast<uint>(d.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>());
    devInfo.doubleSupported = false;
    devInfo.halfSupported = false;

    std::string extns = d.getInfo<CL_DEVICE_EXTENSIONS>();

    if ((extns.find("cl_khr_fp16") != std::string::npos))
    {
        devInfo.halfSupported = true;
    }

    if ((extns.find("cl_khr_fp64") != std::string::npos) || (extns.find("cl_amd_fp64") != std::string::npos))
    {
        devInfo.doubleSupported = true;
    }

    devInfo.deviceType = d.getInfo<CL_DEVICE_TYPE>();

    if (devInfo.deviceType & CL_DEVICE_TYPE_CPU)
    {
        devInfo.gloalBWIters = 20;
        devInfo.globalBWMaxSize = 1 << 27;
        devInfo.computeWgsPerCU = 512;
        devInfo.computeDPWgsPerCU = 256;
        devInfo.computeIters = 10;
        devInfo.transferBWMaxSize = 1 << 27;
    }
    else
    { // GPU
        devInfo.gloalBWIters = 50;
        devInfo.globalBWMaxSize = 1 << 29;
        devInfo.computeWgsPerCU = 2048;
        devInfo.computeDPWgsPerCU = 512;
        devInfo.computeIters = 30;
        devInfo.transferBWMaxSize = 1 << 29;
    }
    devInfo.transferBWIters = 20;
    devInfo.kernelLatencyIters = 20000;

    return devInfo;
}

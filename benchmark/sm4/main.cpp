#define CL_TARGET_OPENCL_VERSION 200
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define DEAD_LOOP
#include <CL/cl.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <limits>
#include <vector>
using std::vector;
#include <utility>

#include "oclUtils.h"
#include "shrUtils.h"
#include "sm_c.h"
#include "argparser.h"

std::atomic<int> threadCount { 0 };
std::mutex stdout_mtx;
vector<pair<double, double>> timeInfo;

using uchar = unsigned char;
using namespace shr;

static void verify_result(uchar *result, uchar *out_c, int size)
{
    bool flag = true;
    for (int i = 0; i < size; i++)
    {
        if (result[i] != out_c[i])
        {
            printf("error=%d,result[0]=%d,out_c[0]=%d\n", i, result[i], out_c[i]);
            flag = false;
            break;
        }
    }

    if (flag)
    {
        printf("************GPU result verify PASS************\n");
    }
    else
    {
        printf("************GPU result verify Failed**********\n");
    }
}

inline void memCleanup(uchar *input, uchar *result, uchar *out_c)
{
    if (input != NULL)
    {
        free(input);
        input = NULL;
    }
    
    if (result != NULL)
    {
        free(result);
        result = NULL;
    }

    if (out_c != NULL)
    {
        free(out_c);
        out_c = NULL;
    }
}

void deal(const CmdArgs &cmdOption, const int pid, const int tid)
{
    auto &verify_c              = cmdOption.verify_c;           // default: false
    auto &GROUPS                = cmdOption.groups;             // default: 1
    auto &loop_count            = cmdOption.loop_num;           // default: 1024
    auto &total_size_MB         = cmdOption.total_size_MB;      // default: 8
    auto &usingPlatformNum      = cmdOption.platforms;          // default: 1
    auto &dataSizePerGPUThread  = cmdOption.per_thread;         // default: 128
    auto &cpuThreadsPerPlatform = cmdOption.threads;            // default: 1

    auto data_size_inBytes      = (cmdOption.total_size_MB * 1024 * 1024);
    auto globalCPUThreadId      = (usingPlatformNum == 1) ? tid : (pid * cpuThreadsPerPlatform + tid);

    cl_int status;
    cl_platform_id platform;
    cl_device_id device = 0;
    cl_context context = 0;
    cl_command_queue commandQueues[GROUPS] = { 0 };
    cl_mem memObjects[GROUPS][3] = { 0 };
    cl_program program = 0;
    cl_kernel kernel = 0;
    cl_event events[GROUPS][3] = { 0 }; // 0: exec, 1: write, 2: read;

    size_t gX = data_size_inBytes / dataSizePerGPUThread;

    size_t inputSize  = data_size_inBytes;
    size_t outputSize = data_size_inBytes;

    uchar key[16] = {0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,0xfe,0xdc,0xba,0x98,0x76,0x54,0x32,0x10};

    uchar *input[GROUPS];
    uchar *result[GROUPS];
    for (int g = 0; g < GROUPS; g++)
    {
        input[g]  = (uchar*)malloc(inputSize);
        if (input[g] == NULL)
        {
            perror("input");
            return;
        }

        result[g] = (uchar*)malloc(outputSize);
        if (result[g] == NULL)
        {
            perror("result");
            return;
        }

        for (int i = 0; i < gX * (dataSizePerGPUThread / 16); i++)
        {
            memcpy(input[g] + i * 16, key, 16);
        }
    }

    uchar *out_c = (uchar*)malloc(outputSize);
    if (out_c == NULL)
    {
        perror("out_c");
        return;
    }

    unsigned int rk[32] = { 0 };
    unsigned int MK[4] = { 0x01234567, 0x89abcdef, 0xfedcba98, 0x76543210 };
    sm4_setkey_IN(rk, MK);

    // get the platform in a sequential way
    stdout_mtx.lock();
    platform = GetPlatform(pid);
    stdout_mtx.unlock();

    device   = GetDevice(platform);
    context  = CreateContext(platform);

    for (int g = 0; g < GROUPS; g++)
    {
        commandQueues[g] = CreateCommandQueue(context, &device);
        assert(commandQueues[g] != NULL);

        if (!CreateMemObjects_Copy(context, memObjects[g], rk, gX, dataSizePerGPUThread))
        {
            for (int i = 0; i <= g; i++)
                oclCleanup(context, commandQueues[i], memObjects[i], program, kernel);
            perror("Couldn't create memObjects");
            return;
        }
    }

    program = CreateProgram(context, device, SM4_CL_FILE);
    if (program == NULL)
    {
        perror("Couldn't create a program");
        return;
    }

    kernel  = clCreateKernel(program, "encrypt", NULL);
    if (kernel == NULL)
    {
        perror("Couldn't create a kernel");
        return;
    }

    size_t globalWorkSize[1] = { gX };
    size_t localWorkSize[1] = { 128 };

    printf("pid: #%d, tid: _%d, platform: %p, device: %p, context: %p\n", pid, pid, platform, device, context);

    threadCount.fetch_add(1);
    // Block until all threads reach this point
    while (threadCount.load() != usingPlatformNum * cpuThreadsPerPlatform) { }

    Timer timer;
    timer.start();
#ifdef DEAD_LOOP
    size_t while_count{ 1 };
    while (true)
#endif
    {
        double tick = timer.seconds();

        for (int i = 0; i < loop_count + GROUPS; i++)
        {
            int g = i % GROUPS;

            if (i >= GROUPS)
            {
                status = clWaitForEvents(1, &events[g][2]);
                if (status != CL_COMPLETE)
                {
                    perror("read buffer failed!");
                    return;
                }
                clReleaseEvent(events[g][2]);

                if (verify_c)
                {
                    crypt_c_c(gX * dataSizePerGPUThread, input[g], out_c, MK);
                    verify_result(result[g], out_c, gX*dataSizePerGPUThread);
                }
            }

            // If i >= loop_count, stop pushing cmds to cmd_queue
            // and make sure the rest cmd_queues are all finished
            if (i >= loop_count) continue;

            status = clSetKernelArg(kernel, 0, sizeof(int), &dataSizePerGPUThread);
            assert(status == CL_SUCCESS);
            status = clSetKernelArg(kernel, 1, sizeof(cl_mem), &memObjects[g][1]);
            assert(status == CL_SUCCESS);
            status = clSetKernelArg(kernel, 2, sizeof(cl_mem), &memObjects[g][2]);
            assert(status == CL_SUCCESS);
            status = clSetKernelArg(kernel, 3, sizeof(cl_mem), &memObjects[g][0]);
            assert(status == CL_SUCCESS);

            if ((status = clEnqueueWriteBuffer(commandQueues[g], memObjects[g][1], CL_FALSE, 0,
                        inputSize, input[g], 0, NULL,  NULL)) != CL_SUCCESS)
            {
                perror("clenqueuewritebuffer error");
                return;
            }

            if ((status = clEnqueueNDRangeKernel(commandQueues[g], kernel, 1, NULL,
                        globalWorkSize, localWorkSize, 0, NULL, NULL)) != CL_SUCCESS)
            {
                perror("error queuing kernel for exectution");
                return;
            }

            if ((status = clEnqueueReadBuffer(commandQueues[g], memObjects[g][2], CL_FALSE, 0,
                        outputSize, result[g], 0, NULL, &events[g][2])) != CL_SUCCESS)
            {
                perror("clenqueuereadbuffer error");
                return;
            }

            clFlush(commandQueues[g]);
        }

        for (int g= 0; g < GROUPS; g++)
        {
            clFinish(commandQueues[g]);
        }

        double tock = timer.seconds();

        timeInfo[globalCPUThreadId] = std::make_pair(tick, tock);

#ifdef DEAD_LOOP
        double elapse = tock - tick;
        double speed = getSpeedInGbps(outputSize, elapse, loop_count);

        stdout_mtx.lock();
        // printf("Output_%d: ", pid);
        // printBuf(result[0]+outputSize-32, 32);
        printf("Platform_%d, TheadId_%d, Run Time: %0.6f s   Speed: %0.6f Gbps\n", pid, tid, elapse, speed);
        printf("While_count: %ld, ", while_count++);
        timer.elapse();
        timer.showCurrentTime();
        stdout_mtx.unlock();

        // Flush the input values
        for (int g = 0; g < GROUPS; g++)
        {
            memset(result[g], 0, outputSize);
            for (int i = 0; i < gX*(dataSizePerGPUThread/16); i++)
            {
                input[g][i*16]++;
            }
        }
#endif // end of DEAD_LOOP
    }

    for (int i = 0; i < GROUPS; i++)
    {
        memCleanup(input[i], result[i], out_c);
        oclCleanup(context, commandQueues[i], memObjects[i], program, kernel);
    }
}

// Main function
// **********************************************************************
int main(int argc, char *argv[])
{
    CmdArgs cmdOption;
    parse(argc, argv, &cmdOption);

    const int usingPlatformNum      = cmdOption.platforms;
    const int cpuThreadsPerPlatform = cmdOption.threads;
    const int totalCPUThreads       = usingPlatformNum * cpuThreadsPerPlatform;
    const int specifiedPlatformIdx  = cmdOption.default_platform_idx;

    // Every CPU-thread need mark the start point & end point
    timeInfo.resize(totalCPUThreads);

    vector<std::thread> threadVec(totalCPUThreads);
    for (int pid = 0, wid = 0; pid < usingPlatformNum; pid++)
    {
        if(specifiedPlatformIdx >= 0) pid = specifiedPlatformIdx;
        for (int tid = 0; tid < cpuThreadsPerPlatform; tid++)
        {
            threadVec[wid++] = std::thread(deal, std::ref(cmdOption), pid, tid);
        }
    }

    for (auto &thread : threadVec)
    {
        thread.join();
    }

    // Get elapse time during the first thread start and the last thread end
    // Note: This is total time for N-loop "writes, executes, and reads"
    Timer timer;
    double iElaps = timer.elapse(timeInfo);

    size_t global_data_size = (cmdOption.total_size_MB * 1024 * 1024) * totalCPUThreads;

    double speed = getSpeedInGbps(global_data_size, iElaps, cmdOption.loop_num);
    printf("Run Time: %0.6f s   Speed: %0.6f Gbps\n", iElaps, speed);

    return 0;
}

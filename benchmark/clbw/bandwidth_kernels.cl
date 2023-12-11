#define FETCH_PER_WI  16

// Kernels fetching by global_size offset
__kernel void global_bandwidth_v1_global_offset(__global float *A, __global float *B)
{
    int id = get_global_id(0);
    float sum = 0;

    for(int i=0; i<4; i++)
    {
        B[id] = sum;
        sum += A[id];   id += get_global_size(0);
        B[id] = sum;
        sum += A[id];   id += get_global_size(0);
        B[id] = sum;

        sum += A[id];   id += get_global_size(0);
        B[id] = sum;
        sum += A[id];   id += get_global_size(0);
    }
}

__kernel void global_bandwidth_v2_global_offset(__global float2 *A, __global float *B)
{
    int id = get_global_id(0);
    float2 sum = 0;

    for(int i=0; i<4; i++)
    {
        B[id] = (sum.S0) + (sum.S1);
        sum += A[id];   id += get_global_size(0);
        B[id] = (sum.S0) + (sum.S1);
        sum += A[id];   id += get_global_size(0);
        B[id] = (sum.S0) + (sum.S1);

        sum += A[id];   id += get_global_size(0);
        B[id] = (sum.S0) + (sum.S1);
        sum += A[id];   id += get_global_size(0);
    }
}

__kernel void global_bandwidth_v4_global_offset(__global float4 *A, __global float *B)
{
    int id = get_global_id(0);
    float4 sum = 0;

    for(int i=0; i<4; i++)
    {
        B[id] = (sum.S0) + (sum.S1) + (sum.S2) + (sum.S3);
        sum += A[id];   id += get_global_size(0);
        B[id] = (sum.S0) + (sum.S1) + (sum.S2) + (sum.S3);
        sum += A[id];   id += get_global_size(0);
        B[id] = (sum.S0) + (sum.S1) + (sum.S2) + (sum.S3);

        sum += A[id];   id += get_global_size(0);
        B[id] = (sum.S0) + (sum.S1) + (sum.S2) + (sum.S3);
        sum += A[id];   id += get_global_size(0);
    }
}

__kernel void global_bandwidth_v8_global_offset(__global float8 *A, __global float *B)
{
    int id = get_global_id(0);
    float8 sum = 0;

    for(int i=0; i<4; i++)
    {
        B[id] = (sum.S0) + (sum.S1) + (sum.S2) + (sum.S3) + (sum.S4) + (sum.S5) + (sum.S6) + (sum.S7);
        sum += A[id];   id += get_global_size(0);
        B[id] = (sum.S0) + (sum.S1) + (sum.S2) + (sum.S3) + (sum.S4) + (sum.S5) + (sum.S6) + (sum.S7);
        sum += A[id];   id += get_global_size(0);
        B[id] = (sum.S0) + (sum.S1) + (sum.S2) + (sum.S3) + (sum.S4) + (sum.S5) + (sum.S6) + (sum.S7);

        sum += A[id];   id += get_global_size(0);
        B[id] = (sum.S0) + (sum.S1) + (sum.S2) + (sum.S3) + (sum.S4) + (sum.S5) + (sum.S6) + (sum.S7);
        sum += A[id];   id += get_global_size(0);
    }
}

__kernel void global_bandwidth_v16_global_offset(__global float16 *A, __global float *B)
{
    int id = get_global_id(0);
    float16 sum = 0;

    for(int i=0; i<4; i++)
    {
        B[id] = (sum.S0) + (sum.S1) + (sum.S2) + (sum.S3) + (sum.S4) + (sum.S5) + (sum.S6) + (sum.S7);
     + (sum.S8) + (sum.S9) + (sum.SA) + (sum.SB) + (sum.SC) + (sum.SD) + (sum.SE) + (sum.SF);
        sum += A[id];   id += get_global_size(0);
        B[id] = (sum.S0) + (sum.S1) + (sum.S2) + (sum.S3) + (sum.S4) + (sum.S5) + (sum.S6) + (sum.S7);
     + (sum.S8) + (sum.S9) + (sum.SA) + (sum.SB) + (sum.SC) + (sum.SD) + (sum.SE) + (sum.SF);
        sum += A[id];   id += get_global_size(0);
        B[id] = (sum.S0) + (sum.S1) + (sum.S2) + (sum.S3) + (sum.S4) + (sum.S5) + (sum.S6) + (sum.S7);
     + (sum.S8) + (sum.S9) + (sum.SA) + (sum.SB) + (sum.SC) + (sum.SD) + (sum.SE) + (sum.SF);
        sum += A[id];   id += get_global_size(0);
        B[id] = (sum.S0) + (sum.S1) + (sum.S2) + (sum.S3) + (sum.S4) + (sum.S5) + (sum.S6) + (sum.S7);
     + (sum.S8) + (sum.S9) + (sum.SA) + (sum.SB) + (sum.SC) + (sum.SD) + (sum.SE) + (sum.SF);
        sum += A[id];   id += get_global_size(0);
    }
}

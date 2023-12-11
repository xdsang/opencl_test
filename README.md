[[_TOC_]]

# Innosilicon Compute Benchmarks

This repo includes common compute benchmarks used to benchmark Innosilicon Fantasy GPUs' computation capabilities.

# Preface

For internal use, especially when the Internet connection to GitHub is unstabled, you are recommended to use a proxy for GitHub:

```bash
git config --global url."https://gh-proxy.com/https://github.com/".insteadOf "https://github.com/"
```

Run `cat ~/.gitconfig` to check whether the configuration is set or not.

```bash
[user]
        email = xxx@innosilicon.com.cn
        name = xxx
[url "https://gh-proxy.com/https://github.com/"]
        insteadOf = https://github.com/
```

Bear in mind that the `gh-proxy.com` domain name may change from time to time, so please ensure you are using the latest one.

# clpeak

clpeak is a benchmarking tool to measure peak capabilities of OpenCL devices.

To build:

```bash
cd clpeak

mkdir build && cd build

cmake ..

cmake --build .
```

To run:

```bash
cd clpeak/build

./clpeak
```

Output example (Fantasy G1P 1xMC4):

```bash
Platform: InnoGPU
  Device: Fantasy I*
    Driver version  : 1.19@6345021 (Linux x64)
    Compute units   : 16
    Clock frequency : 1000 MHz

    Global memory bandwidth (GBPS)
      float   : 37.34
      float2  : 42.51
      float4  : 43.28
      float8  : 41.46
      float16 : 43.46

    Single-precision compute (GFLOPS)
      float   : 4736.07
      float2  : 4666.03
      float4  : 4631.32
      float8  : 4489.02
      float16 : 4467.39

    Half-precision compute (GFLOPS)
      half   : 4437.94
      half2  : 4663.50
      half4  : 4630.09
      half8  : 4486.17
      half16 : 4255.69

    No double precision support! Skipped

    Integer compute (GIOPS)
      int   : 698.53
      int2  : 698.12
      int4  : 697.59
      int8  : 699.49
      int16 : 700.08

    Integer compute Fast 24bit (GIOPS)
      int   : 698.60
      int2  : 698.24
      int4  : 697.71
      int8  : 699.61
      int16 : 700.18

    Transfer bandwidth (GBPS)
      enqueueWriteBuffer              : 6.96
      enqueueReadBuffer               : 126.63
      enqueueWriteBuffer non-blocking : 6.96
      enqueueReadBuffer non-blocking  : 128.66
      enqueueMapBuffer(for read)      : 10400.22
        memcpy from mapped ptr        : 0.03
      enqueueUnmap(after write)       : 11597.51
        memcpy to mapped ptr          : 6.96

    Kernel launch latency : 14.03 us
```

# LuxMark

LuxMark is an OpenCL benchmark based on LuxCoreRender and written using LuxCore API.

Note:

1. **The LuxMark program requires X-org and desktop being enabled. You won't be able to run it through SSH connections.**
2. **The LuxMark program requires x86 environment, it doesn't work on Arm.**

## Download

Download the LuxMark v3.1 binaries:

```bash
wget https://github.com/LuxCoreRender/LuxMark/releases/download/luxmark_v3.1/luxmark-linux64-v3.1.tar.bz2

# Use gh-proxy.com as proxy if accessing GitHub is slow
wget https://gh-proxy.com/https://github.com/LuxCoreRender/LuxMark/releases/download/luxmark_v3.1/luxmark-linux64-v3.1.tar.bz2
```

## Install and Set Dependencies

Install GLUT:

```bash
sudo apt-get install freeglut3
```

Set libembree dynamic library's path:

```bash
# Replace with the real path to luxmark-v3.1/lib/libembree.so on your machine
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/root/xxx/luxmark-v3.1/lib
```

## Run LuxMark

```bash
cd luxmark-v3.1
./luxmark
```

## Reference

LuxMark v3.1 wiki: https://wiki.luxcorerender.org/LuxMark_v3

# clbw

clbw (OpenCL BandWidth) is a benchmark tool extracted from clpeak and extended for better performance on Fantasy GPUs.

To build and run clbw, please refer to [benchmark/clbw/README.md](benchmark/clbw/README.md).

# sm4

To build and run sm4, please refer to [benchmark/sm4/README.md](benchmark/sm4/README.md).
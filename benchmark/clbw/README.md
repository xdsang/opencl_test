# 目的
测试显存带宽（GPU Core 与 GDDR 之间的读写速率），涉及的数据类型有：float、float2、float4、float8、float16。

# 原理
```
所有平台，几乎在同一时刻，读写相同大小的数据，操作完成后，返回操作开始时间和结束时间；
速率统计：单个平台读写的数据量 * 平台个数 / 最大时间戳
```

# 编译

在compute_benchmarks/目录下，进行编译：

```
mkdir build
cd build
cmake ..
make -j8
```

编译完成，【可执行文件】在目录`compute_benchmarks/build/benchmark/clbw`下面。

# 示例
默认类型: float、float2、float4、float8、float16
默认平台: all platforms

## 1 One GPU Card

### 1.1 One platform(1 x mc1)

```./clbw -h```
查看详细命令参数及使用方法

```./clbw```
使用所有平台（1个），测试类型包括float、float2、float4、float8、float16

```./clbw -tn "v1 v2"```
使用所有平台（1个），测试类型包括float和float2

### 1.2 Multiple platforms(4 x mc1)

```./clbw```
使用所有平台，测试类型包括float、float2、float4、float8、float16

```./clbw -p 0```
使用platform 0，测试类型包括float、float2、float4、float8、float16

```./clbw -p 1```
使用platform 1，测试类型包括float、float2、float4、float8、float16

```./clbw -tn "v1 v2" -p 0```
使用platform 0，测试类型包括float和float2

```./clbw -tn "v1 v2"```
使用所有平台，测试类型包括float和float2

其中，在GDDR 6，16G的G1P卡上，使用(4 x mc1)模式时，`./clbw`的结果如下，可供参考：
```
Global memory bandwidth (GBPS), Using all 4 platform(s).
platformId: 2, tick: 13797.604285, tock: 13798.162139, iElapse: 0.557854 s
platformId: 0, tick: 13797.604285, tock: 13798.162971, iElapse: 0.558686 s
platformId: 1, tick: 13797.604285, tock: 13798.177660, iElapse: 0.573375 s
platformId: 3, tick: 13797.604285, tock: 13798.178365, iElapse: 0.574080 s
Data type: float1, Data size: 1.37 GB, Run time: 0.011482 s, Bandwidth: 119.08 GB/s

platformId: 2, tick: 13798.383187, tock: 13798.676152, iElapse: 0.292965 s
platformId: 0, tick: 13798.383187, tock: 13798.676633, iElapse: 0.293445 s
platformId: 1, tick: 13798.383188, tock: 13798.733341, iElapse: 0.350153 s
platformId: 3, tick: 13798.383187, tock: 13798.734278, iElapse: 0.351091 s
Data type: float2, Data size: 1.37 GB, Run time: 0.007022 s, Bandwidth: 194.71 GB/s

platformId: 2, tick: 13798.928270, tock: 13799.164216, iElapse: 0.235946 s
platformId: 0, tick: 13798.928270, tock: 13799.164517, iElapse: 0.236248 s
platformId: 1, tick: 13798.928270, tock: 13799.246445, iElapse: 0.318176 s
platformId: 3, tick: 13798.928270, tock: 13799.246988, iElapse: 0.318719 s
Data type: float4, Data size: 1.37 GB, Run time: 0.006374 s, Bandwidth: 214.48 GB/s

platformId: 0, tick: 13799.443559, tock: 13799.657511, iElapse: 0.213951 s
platformId: 2, tick: 13799.443559, tock: 13799.657630, iElapse: 0.214071 s
platformId: 1, tick: 13799.443559, tock: 13799.744705, iElapse: 0.301146 s
platformId: 3, tick: 13799.443559, tock: 13799.745022, iElapse: 0.301462 s
Data type: float8, Data size: 1.37 GB, Run time: 0.006029 s, Bandwidth: 226.76 GB/s

platformId: 2, tick: 13799.926588, tock: 13800.143616, iElapse: 0.217028 s
platformId: 0, tick: 13799.926588, tock: 13800.144216, iElapse: 0.217628 s
platformId: 3, tick: 13799.926588, tock: 13800.220068, iElapse: 0.293480 s
platformId: 1, tick: 13799.926588, tock: 13800.220516, iElapse: 0.293928 s
Data type: float16, Data size: 1.37 GB, Run time: 0.005879 s, Bandwidth: 232.57 GB/s

Driver version  : 1.19@6345021
Clock frequency : 1000 MHz
```


## 2 Multiple Cards

### 2.1 One platform per card

```./clbw -p 0```
使用platform 0，测试类型包括float、float2、float4、float8、float16

```./clbw -p 1```
使用platform 1，测试类型包括float、float2、float4、float8、float16

```./clbw -p 1 -tn v1```
使用platform 1，测试类型包括float

```./clbw -p 1 -tn "v1 v2```
使用platform 1，测试类型包括float、float2

注意：
`./clbw`会启用所有平台进行测试，目前这种方式，统计处理的数据还有很大误差。建议在多卡机器上，测试单个平台即可。

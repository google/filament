// RUN: %dxc -spirv -Od -T lib_6_8 -fspv-target-env=vulkan1.3 external %s | FileCheck %s

// Barrier is called from a compute shader

[Shader("compute")]
[NumThreads(5,1,1)]
void node116_barrier_compute()
{
  Barrier(1, 3);
}

// CHECK: [[UINT:%[^ ]*]] = OpTypeInt 32 0
// CHECK: [[U2:%[^ ]*]] = OpConstant [[UINT]] 2
// CHECK-DAG: [[U72:%[^ ]*]] = OpConstant [[UINT]] 72
// CHECK: OpControlBarrier [[U2]] [[U2]] [[U72]]

// RUN: %dxc -spirv -Od -T lib_6_8 -fspv-target-env=vulkan1.3 -enable-16bit-types %s | FileCheck %s

// Barrier is called using a memory type argument

static const int a = 7;
static const int16_t b = 2;

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(16, 1, 1)]
void node117_barrier_memoryarg()
{
  // literal integer flag values
  Barrier(1, 3);

  // static const integer flag values
  Barrier(a, b);

  // AllMemoryBarrier() ->
  Barrier(UAV_MEMORY|GROUP_SHARED_MEMORY|NODE_INPUT_MEMORY|NODE_OUTPUT_MEMORY,
          DEVICE_SCOPE);

  // AllMemoryBarrierWithGroupSync() ->
  Barrier(UAV_MEMORY|GROUP_SHARED_MEMORY|NODE_INPUT_MEMORY|NODE_OUTPUT_MEMORY,
          GROUP_SYNC|DEVICE_SCOPE);

  // DeviceMemoryBarrier() ->
  Barrier(UAV_MEMORY,
          DEVICE_SCOPE);

  // DeviceMemoryBarrierWithGroupSync() ->
  Barrier(UAV_MEMORY,
          GROUP_SYNC|DEVICE_SCOPE);

  // GroupMemoryBarrier() ->
  Barrier(GROUP_SHARED_MEMORY,
          GROUP_SCOPE);

  // GroupMemoryBarrierWithGroupSync() ->
  Barrier(GROUP_SHARED_MEMORY,
          GROUP_SYNC|GROUP_SCOPE);
}


// CHECK: [[UINT:%[^ ]*]] = OpTypeInt 32 0
// CHECK-DAG: [[U2:%[^ ]*]] = OpConstant %uint 2
// CHECK-DAG: [[U5:%[^ ]*]] = OpConstant %uint 5
// CHECK-DAG: [[U72:%[^ ]*]] = OpConstant %uint 72
// CHECK-DAG: [[U264:%[^ ]*]] = OpConstant %uint 264
// CHECK-DAG: [[U328:%[^ ]*]] = OpConstant %uint 328
// CHECK-DAG: [[U4424:%[^ ]*]] = OpConstant %uint 4424

// CHECK: OpControlBarrier [[U2]] [[U2]] [[U72]]
// CHECK: OpMemoryBarrier [[U2]] [[U328]]
// CHECK: OpMemoryBarrier [[U5]] [[U4424]]
// CHECK: OpControlBarrier [[U2]] [[U5]] [[U4424]]
// CHECK: OpMemoryBarrier [[U5]] [[U72]]
// CHECK: OpControlBarrier [[U2]] [[U5]] [[U72]]
// CHECK: OpMemoryBarrier [[U2]] [[U264]]
// CHECK: OpControlBarrier [[U2]] [[U2]] [[U264]]

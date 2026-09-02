// RUN: %dxc -T lib_6_8 -enable-16bit-types %s | FileCheck %s
// ==================================================================
// Barrier is called using a memory type argument
// ==================================================================

static const int a = 7;
static const int b = 3;

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1, 1, 1)]
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
          DEVICE_SCOPE | GROUP_SYNC);

  // DeviceMemoryBarrier() ->
  Barrier(UAV_MEMORY,
          DEVICE_SCOPE);

  // DeviceMemoryBarrierWithGroupSync() ->
  Barrier(UAV_MEMORY,
          DEVICE_SCOPE | GROUP_SYNC);

  // GroupMemoryBarrier() ->
  Barrier(GROUP_SHARED_MEMORY,
          GROUP_SCOPE);

  // GroupMemoryBarrierWithGroupSync() ->
  Barrier(GROUP_SHARED_MEMORY,
          GROUP_SCOPE | GROUP_SYNC);
}

// Shader function
// ------------------------------------------------------------------
// CHECK: define void @node117_barrier_memoryarg()
// CHECK-SAME: {
// CHECK: call void @dx.op.barrierByMemoryType(i32 {{[0-9]+}}, i32 1, i32 3)  ; BarrierByMemoryType(MemoryTypeFlags,SemanticFlags)
// CHECK: call void @dx.op.barrierByMemoryType(i32 {{[0-9]+}}, i32 7, i32 3)  ; BarrierByMemoryType(MemoryTypeFlags,SemanticFlags)
// CHECK: call void @dx.op.barrierByMemoryType(i32 {{[0-9]+}}, i32 15, i32 4)  ; BarrierByMemoryType(MemoryTypeFlags,SemanticFlags)
// CHECK: call void @dx.op.barrierByMemoryType(i32 {{[0-9]+}}, i32 15, i32 5)  ; BarrierByMemoryType(MemoryTypeFlags,SemanticFlags)
// CHECK: call void @dx.op.barrierByMemoryType(i32 {{[0-9]+}}, i32 1, i32 4)  ; BarrierByMemoryType(MemoryTypeFlags,SemanticFlags)
// CHECK: call void @dx.op.barrierByMemoryType(i32 {{[0-9]+}}, i32 1, i32 5)  ; BarrierByMemoryType(MemoryTypeFlags,SemanticFlags)
// CHECK: call void @dx.op.barrierByMemoryType(i32 {{[0-9]+}}, i32 2, i32 2)  ; BarrierByMemoryType(MemoryTypeFlags,SemanticFlags)
// CHECK: call void @dx.op.barrierByMemoryType(i32 {{[0-9]+}}, i32 2, i32 3)  ; BarrierByMemoryType(MemoryTypeFlags,SemanticFlags)

// CHECK:   ret void
// CHECK: }

// Metadata for node
// ------------------------------------------------------------------
// CHECK: !dx.entryPoints = !{
// CHECK-SAME: }
// CHECK: = !{void ()* @node117_barrier_memoryarg, !"node117_barrier_memoryarg", null, null, [[ATTRS:![0-9]+]]}

// Metadata for node attributes
// Arg #1: ShaderKind Tag (8)
// Arg #2: Node (15)
// Arg #3: NodeLaunch Tag (13)
// Arg #4: broadcasting (1)
// ...
// ------------------------------------------------------------------
// CHECK: [[ATTRS]] = !{
// CHECK-SAME: i32 8, i32 15, i32 13, i32 1
// CHECK-SAME: }

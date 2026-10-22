// RUN: %dxc -T lib_6_8 -default-linkage external %s | FileCheck %s
// ==================================================================
// CASE116
// Barrier is called from a compute shader
// ==================================================================

// Shader function
// ------------------------------------------------------------------
// CHECK: define void @node116_barrier_compute()
// CHECK-SAME: {
// CHECK:   call void @dx.op.barrierByMemoryType(i32
// CHECK-SAME: , i32 1, i32 3)
// CHECK:   ret void
// CHECK: }

[Shader("compute")]
[NumThreads(5,1,1)]
void node116_barrier_compute()
{
  Barrier(UAV_MEMORY, GROUP_SYNC|GROUP_SCOPE);
}

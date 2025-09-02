// RUN: %dxc -T lib_6_9 -E main %s | FileCheck %s

RWByteAddressBuffer BAB : register(u1, space0);

[shader("raygeneration")]
void main() {
// CHECK:  call void @dx.op.barrierByMemoryType(i32 244, i32 1, i32 8)
  Barrier(UAV_MEMORY, REORDER_SCOPE);

// CHECK:  call void @dx.op.barrierByMemoryHandle(i32 245, %dx.types.Handle %{{[^ ]+}}, i32 8)
  Barrier(BAB, REORDER_SCOPE);
}

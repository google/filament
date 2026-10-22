// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s | FileCheck %s

// Make sure only 1 barrier.
// CHECK: call void @dx.op.barrier(i32 80, i32 9)
// CHECK-NOT: call void @dx.op.barrier(i32 80, i32 9)

cbuffer X {
  float x;
  uint  n;
}

float test(uint t) {
  float r = x;
  for (uint i=0;i<n;i++) {
    r++;
    if (t>2)
      r += 3;
    GroupMemoryBarrierWithGroupSync();
  }
  return r;
}
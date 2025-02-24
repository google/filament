// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external -Zpr %s | FileCheck %s

// check that matrix lowering succeeds
// CHECK-NOT: Fail to lower matrix load/store.
// make sure no transpose is present
// CHECK-NOT: shufflevector

// Check that compile succeeds
// CHECK: ret %class.matrix.float.3.4

struct Foo {
  float3x4 mat_array[2];
  int i;
};

float3x4 lookup(Foo f, inout float3x4 mat) {
  return f.mat_array[f.i];
}

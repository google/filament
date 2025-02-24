// RUN: %dxc -T lib_6_3 -default-linkage external %s | FileCheck %s

// Make sure this works on intrinsic that returns matrix
// CHECK: call float @dx.op.worldToObject.f32(i32 152, i32 0, i8 1)

float foo() {
  return WorldToObject()._m01;
}

// RUN: %dxc -T lib_6_3 %s | FileCheck %s

// Test that auto-binding-space affects only resources with no
// explicit register nor space binding.

// CHECK: buf_a texture f32 buf T0t4294967295,space4294967295 1
// CHECK: buf_b texture f32 buf T1 t1 1
// CHECK: buf_c texture f32 buf T2t4294967295,space2 1
// CHECK: buf_d texture f32 buf T3 t2,space3 1
Buffer buf_a;
Buffer buf_b : register(t1);
Buffer buf_c : register(space2);
Buffer buf_d : register(t2,space3);

export float foo() {
  return buf_a[0] + buf_b[0] + buf_c[0] + buf_d[0];
}
// RUN: %dxc -E main -T vs_6_0 -auto-binding-space 1 %s | FileCheck %s

// Test that auto-binding-space affects only resources with no
// explicit register nor space binding.

// CHECK: buf_a texture f32 buf T0 t0,space1 1
// CHECK: buf_b texture f32 buf T1        t1 1
// CHECK: buf_c texture f32 buf T2 t0,space2 1
// CHECK: buf_d texture f32 buf T3 t2,space3 1
Buffer buf_a;
Buffer buf_b : register(t1);
Buffer buf_c : register(space2);
Buffer buf_d : register(t2,space3);

float main() : OUT {
  return buf_a[0] + buf_b[0] + buf_c[0] + buf_d[0];
}
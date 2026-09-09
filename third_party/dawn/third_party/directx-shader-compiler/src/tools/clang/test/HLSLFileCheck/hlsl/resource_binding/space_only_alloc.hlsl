// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Test resource allocation with space-only register annotations.

// CHECK-DAG: buf_s0_a texture f32 buf T0        t0 1
// CHECK-DAG: buf_s1_a texture f32 buf T1 t0,space1 1
// CHECK-DAG: buf_s0_b texture f32 buf T2        t1 1
// CHECK-DAG: buf_s1_b texture f32 buf T3 t1,space1 1
Buffer buf_s0_a;
Buffer buf_s1_a : register(space1);
Buffer buf_s0_b;
Buffer buf_s1_b : register(space1);

float main() : OUT {
  return buf_s0_a[0] + buf_s1_a[0] + buf_s0_b[0] + buf_s1_b[0];
}
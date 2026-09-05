// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Identical to count_cbuff.hlsl, except checks for number of br's

// entry
// CHECK: br
// loop iteration
// CHECK: call float @dx.op.dot3
// CHECK: br
// loop iteration
// CHECK: call float @dx.op.dot3
// CHECK: br
// loop iteration, unconditional
// CHECK: call float @dx.op.dot3
// CHECK: br
// return
// CHECK-NOT: br

uint g_cond;
float main(float3 a : A, float3 b : B) : SV_Target {

  float result = 0;
  [unroll(3)]
  for (int i = 0; i < g_cond; i++) {
    result += dot(a*i, b);
  }
  return result;
}


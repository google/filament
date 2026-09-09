// RUN: %dxc -Od -E main -T ps_6_0 -HV 2018 %s | FileCheck %s

// CHECK: call float @dx.op.dot3
// CHECK: call float @dx.op.dot3
// CHECK: call float @dx.op.dot3
// CHECK: call float @dx.op.dot3

// CHECK-NOT: call float @dx.op.dot3

uint g_cond;

float main(float3 a : A, float3 b : B) : SV_Target {
  float result = 0;
  [unroll]
  for (uint j = 0; j < g_cond && j < 4; j++) {
    result += dot(a*j, b);
  }
  return result;
}


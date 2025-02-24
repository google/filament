// RUN: %dxc -Gis -E main -T ps_6_0 %s | FileCheck %s
// CHECK: call float @dx.op.dot3
// CHECK: call float @dx.op.dot3
// CHECK: call float @dx.op.dot3
// CHECK: call float @dx.op.dot3
// CHECK-NOT: call float @dx.op.dot3

float4 main(float3 a : A, float3 b : B) : SV_Target {
  uint result = 1;
  [unroll]
  for (uint i = 0; i < 4; i++) {
    result += dot(a*i, b);
  }
  return float4(result, 0,0, 1);
}



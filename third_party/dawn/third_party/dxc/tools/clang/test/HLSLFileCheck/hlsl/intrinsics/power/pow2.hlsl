// RUN: %dxc -T ps_6_0 -E main %s | FileCheck %s

// Make sure pow 2 not generate log -> mul 2 -> exp.
// Log
// CHECK-NOT:call float @dx.op.unary.f32(i32 23,
// CHECK-NOT:fmul fast float %{{.*}}, 2.000000e+00
// Exp
// CHECK-NOT:call float @dx.op.unary.f32(i32 21,
// CHECK: fmul fast float %[[a:.*]], %[[a]]
// CHECK: fmul fast float %[[b:.*]], %[[b]]
// CHECK: fmul fast float %[[c:.*]], %[[c]]
// CHECK: fmul fast float %[[d:.*]], %[[d]]

float4 main(float a :A, float3 b:B) : SV_Target {
  return float4(pow(a, 2), pow(b,2));
}
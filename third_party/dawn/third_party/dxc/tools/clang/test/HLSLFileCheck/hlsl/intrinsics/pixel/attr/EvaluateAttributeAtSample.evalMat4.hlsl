// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: call float @dx.op.evalCentroid.f32(i32 89, i32 0, i32 0, i8 1)
// CHECK: call float @dx.op.evalCentroid.f32(i32 89, i32 0, i32 1, i8 1)
// CHECK: call float @dx.op.evalCentroid.f32(i32 89, i32 0, i32 0, i8 2)
// CHECK: call float @dx.op.evalCentroid.f32(i32 89, i32 0, i32 1, i8 2)

float4 main(float4x4 a : A) : SV_Target
{
  float3x2 r = EvaluateAttributeCentroid((float3x2)a);

  return float4(r[1], r[2]);
}

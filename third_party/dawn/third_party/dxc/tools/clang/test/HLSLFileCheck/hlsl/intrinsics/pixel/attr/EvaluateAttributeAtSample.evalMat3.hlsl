// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: call float @dx.op.evalCentroid.f32(i32 89, i32 0, i32 1, i8 0)
// CHECK: call float @dx.op.evalCentroid.f32(i32 89, i32 0, i32 3, i8 2)
// CHECK: call float @dx.op.evalCentroid.f32(i32 89, i32 0, i32 2, i8 1)
// CHECK: call float @dx.op.evalCentroid.f32(i32 89, i32 0, i32 0, i8 3)

float4 main(float4x4 a : A) : SV_Target
{
  float4 r = EvaluateAttributeCentroid(a._m01_m23_m12_m30);

  return r;
}
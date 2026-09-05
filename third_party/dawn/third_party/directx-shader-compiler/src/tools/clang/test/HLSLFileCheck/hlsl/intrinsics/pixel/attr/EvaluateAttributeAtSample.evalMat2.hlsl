// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: call float @dx.op.evalCentroid.f32(i32 89, i32 0, i32 1, i8 2)
// CHECK: call float @dx.op.evalCentroid.f32(i32 89, i32 0, i32 1, i8 0)
// CHECK: call float @dx.op.evalCentroid.f32(i32 89, i32 0, i32 1, i8 3)

float4 main(row_major float4x4 a : A) : SV_Target
{
  float3 r = EvaluateAttributeCentroid(a[1].zxw);

  return r.xyzx;
}

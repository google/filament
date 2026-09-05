// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: call float @dx.op.evalCentroid.f32(i32 89, i32 0, i32 2, i8 1)

float4 main(float4x4 a : A) : SV_Target
{
  float r = EvaluateAttributeCentroid(a[1].z);

  return r;
}
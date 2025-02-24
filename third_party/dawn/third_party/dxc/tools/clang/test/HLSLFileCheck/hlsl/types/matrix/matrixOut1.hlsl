// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// fxc o1.xy = 5, 7 o2.xy = 6, 8

// CHECK: dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 0, float 5.000000e+00)
// CHECK: dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 1, float 7.000000e+00)
// CHECK: dx.op.storeOutput.f32(i32 5, i32 1, i32 1, i8 0, float 6.000000e+00)
// CHECK: dx.op.storeOutput.f32(i32 5, i32 1, i32 1, i8 1, float 8.000000e+00)

float4 main(out float2x2 a : A, int4 b : B) : SV_Position
{
  float2x2 m = { 5, 6, 7, 8};
  a = m;
  return b;
}

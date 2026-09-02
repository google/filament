// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -HV 2018 %s | FileCheck %s

// Regression test for GitHub #2377, where conversions of negative integers to
// 16-bit float types would use unsigned-to-float conversion and overflow to undef.

float4 main() : OUT
{
  // CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float -1.000000e+00)
  // CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float -1.000000e+00)
  // CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float -1.000000e+00)
  // CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float -1.000000e+00)
  return float4(min16float(-1), half(-1), float(-1), double(-1));
}
// RUN: %dxc -E main -T ps_6_0 -fcgl %s | FileCheck %s

// CHECK: main
// After lowering, these would turn into multiple abs calls rather than a 4 x float
// CHECK: call <4 x float> @"dx.hl.op..<4 x float> (i32, <4 x float>)"(i32 84,

float4 main(float4 a : A) : SV_TARGET {
  return abs(a*a.yxxx);
}

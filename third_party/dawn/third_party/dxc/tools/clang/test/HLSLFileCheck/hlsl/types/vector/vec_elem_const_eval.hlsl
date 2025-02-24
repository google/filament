// RUN: %dxc -E main -T ps_6_0 -fcgl %s | FileCheck %s

// Note: since we want to check that HLSLVectorElementExpr are constant evaluated,
// we require -fcgl here to turn off further constant folding and other optimizations.

// CHECK: constant <4 x float> <float 1.500000e+00, float 1.500000e+00, float 1.500000e+00, float 1.500000e+00>
// CHECK: constant <4 x float> <float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00>
// CHECK: constant <4 x i32> <i32 3, i32 3, i32 3, i32 3>

static const float4 a = (1.5).xxxx;
static const float4 b = (2).xxxx;
static const int4   c = (3.5).xxxx;

float4 main() : SV_Target {
  return a + b + c;
}

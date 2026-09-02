// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// TODO: create execution test.

// CHECK: [3 x float] [float 5.000000e+00, float 0.000000e+00, float 0.000000e+00]
// CHECK: [3 x float] [float 6.000000e+00, float 0.000000e+00, float 0.000000e+00]
// CHECK: [3 x float] [float 7.000000e+00, float 0.000000e+00, float 0.000000e+00]
// CHECK: [3 x float] [float 8.000000e+00, float 0.000000e+00, float 0.000000e+00]
// CHECK: [16 x float] [float 0.000000e+00, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 0.000000e+00, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 0.000000e+00, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 0.000000e+00, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00]
// CHECK: [16 x float] [float 1.500000e+01, float 1.500000e+01, float 1.500000e+01, float 1.500000e+01, float 1.600000e+01, float 1.600000e+01, float 1.600000e+01, float 1.600000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.800000e+01, float 1.800000e+01, float 1.800000e+01, float 1.800000e+01]
// CHECK: [4 x float] [float 5.000000e+00, float 6.000000e+00, float 7.000000e+00, float 8.000000e+00]
// CHECK: [16 x float] [float 2.500000e+01, float 2.700000e+01, float 2.600000e+01, float 2.800000e+01, float 2.500000e+01, float 2.700000e+01, float 2.600000e+01, float 2.800000e+01, float 2.500000e+01, float 2.700000e+01, float 2.600000e+01, float 2.800000e+01, float 2.500000e+01, float 2.700000e+01, float 2.600000e+01, float 2.800000e+01]

static float4 f0 = {5,6,7,8};
static float4 f1 = 0;
static float4 f2 = {0,0,0,0};
static float4 f3[] = { f0, f1, f2 };
static float4x4 worldMatrix = { {0,0,0,0}, {1,1,1,1}, {2,2,2,2}, {3,3,3,3} };

static float2x2 m1;
static float4x4 m0 = { 15,16,17,18,
                       15,16,17,18,
                       15,16,17,18,
                       15,16,17,18 };

static float2x2 m2[4] = { 25,26,27,28,
                       25,26,27,28,
                       25,26,27,28,
                       25,26,27,28 };


uint i;

float4 main() : SV_TARGET {
  m2[i][1][i] = m0[i][i];
  m1 = m2[i];
  m1[0][1] = 2;
  return f2 + f1 + f0[i] + m2[i]._m00_m01_m00_m10 + m0[i] + m1[i].y + f3[i] + worldMatrix[i];
}

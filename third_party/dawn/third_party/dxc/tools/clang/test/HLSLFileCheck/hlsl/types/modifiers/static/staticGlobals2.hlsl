// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// TODO: create execution test.

// CHECK: @main
uint i;

float4 main() : SV_TARGET {

static float4 f0 = {5,6,7,8};
static float4 f1 = 0;
static float4 f2 = {0,0,0,0};

static float4x4 m0 = { 15,16,17,18,
                       15,16,17,18,
                       15,16,17,18,
                       15,16,17,18 };
  static float2x2 m2[4] = { 25,26,27,28,
                       25,26,27,28,
                       25,26,27,28,
                       25,26,27,28 };

  m2[i][1][i] = m0[i][i];
  return f2 + f1 + f0[i] + m2[1]._m00_m01_m00_m10 + m0[i];
}

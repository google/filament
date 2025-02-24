// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: @main
struct s_f3_f3 {
 float3 f0;
 float3 f1;
};

float2x3 m23;
float3x2 m32;
float3x4 m34;
row_major float2x3 m23r;

struct matA {
float2x3 f2[2];
};

RWStructuredBuffer<matA> buf;

void main() {
  // Direct initialization fails.
  s_f3_f3 sf3f3_all = { float3(1,2,3), uint3(3,2,1) };

  // Initialization list with members.
  s_f3_f3 sf3f3_all_straddle_instances[6] = { float2(1,2), sf3f3_all, float4(1,2,3,4),
     m23, m32, m34 };

  float2x3 f2[2] = { m23, m32 };
  buf[0].f2 = f2;
  float2 t = m23[0].xz;
  t += m23r[0].xz;
  buf[3].f2[0][1] = m23[0];
  matA ma = {f2};
  buf[1] = ma;
}
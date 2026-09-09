// RUN: %dxc -E main -T ps_6_0 -O0 %s | FileCheck %s

// CHECK: main
// TODO: check mat_in after disable inline.

row_major float3x3 m;

StructuredBuffer<float3x3> sm;

float3 mat_in(float3x3 tm, float3 t)
{
  return mul(tm, t);
}

float4 main(float4 a : A, float4 b:B) : SV_TARGET
{
    float3 m0 = mat_in(m, a.xyz);
    float3 m1 = mat_in(sm[b.x], b.xyz);
    return float4(m0+m1, 1);
}


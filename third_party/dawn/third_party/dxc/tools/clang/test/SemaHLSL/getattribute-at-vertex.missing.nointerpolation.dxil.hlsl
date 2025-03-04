// RUN: not %dxc -T ps_6_2 -E main %s 2>&1 | FileCheck %s

struct S1 {
  float3 f1;
};

float compute(float3 value) {
  return GetAttributeAtVertex(value, 0)[0];
}

float4 main(float3 a : A, S1 s1 : B) : SV_Target
{
// CHECK-DAG: error: Attribute A must have nointerpolation mode in order to use GetAttributeAtVertex function.
// CHECK-DAG: error: Attribute B must have nointerpolation mode in order to use GetAttributeAtVertex function.
  float v1 = GetAttributeAtVertex(a, 0)[0];
  float v2 = GetAttributeAtVertex(s1.f1, 0)[0];
  float v3 = compute(a);
  return float4(v1.xx, v2, v3);
}

// RUN: not %dxc -T ps_6_2 -E main %s 2>&1 | FileCheck %s

float compute(nointerpolation float3 value) {
  return GetAttributeAtVertex(value, 0)[0];
}

float4 main(float3 a : A) : SV_Target
{
  // CHECK: error: Attribute A must have nointerpolation mode in order to use GetAttributeAtVertex function.
  float v1 = compute(a);
  return float4(v1.xxxx);
}

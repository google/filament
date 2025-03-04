// REQUIRES: spirv
// RUN: %dxc -T ps_6_2 -E main %s -spirv -verify

float compute(nointerpolation float3 value) {
  return GetAttributeAtVertex(value, 0)[0];
}

float4 main(float3 a : A) : SV_Target
{
  float v1 = compute(a); /* expected-error{{parameter 0 of compute must have a 'nointerpolation' attribute}} */
  return float4(v1.xxxx);
}

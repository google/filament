// REQUIRES: spirv
// RUN: %dxc -T ps_6_2 -E main %s -spirv -verify

struct S1 {
  float3 f1;
};

float compute(float3 value) {
  return GetAttributeAtVertex(value, 0)[0]; /* expected-error{{parameter 0 of GetAttributeAtVertex must have a 'nointerpolation' attribute}} */
}

float4 main(float3 a : A, S1 s1 : B) : SV_Target
{
  float v1 = GetAttributeAtVertex(a, 0)[0]; /* expected-error{{parameter 0 of GetAttributeAtVertex must have a 'nointerpolation' attribute}} */
  float v2 = GetAttributeAtVertex(s1.f1, 0)[0]; /* expected-error{{parameter 0 of GetAttributeAtVertex must have a 'nointerpolation' attribute}} */
  float v3 = compute(a);
  return float4(v1.xx, v2, v3);
}

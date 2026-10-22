// RUN: %dxc -T ps_6_2 -E main %s -verify

// expected-no-diagnostics
float compute(nointerpolation float3 value) {
  return GetAttributeAtVertex(value, 0)[0];
}

float middle(nointerpolation float3 value) {
  return compute(value);
}

float4 main(nointerpolation float3 a : A) : SV_Target
{
  float v1 = middle(a);
  return float4(v1.xxxx);
}

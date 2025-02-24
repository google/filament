// RUN: %dxc -T ps_6_1 -E main  %s -spirv -verify

struct S {
  float4 a : COLOR;
};

float compute(nointerpolation float4 a) {
  return GetAttributeAtVertex(a, 2)[0];
}

float4 main(nointerpolation S s, float4 b : COLOR2) : SV_TARGET
{
  return float4(0,
                compute(b), // expected-error{{parameter 0 of compute must have a 'nointerpolation' attribute}}
                compute(b), // expected-error{{parameter 0 of compute must have a 'nointerpolation' attribute}}
                compute(s.a));
}

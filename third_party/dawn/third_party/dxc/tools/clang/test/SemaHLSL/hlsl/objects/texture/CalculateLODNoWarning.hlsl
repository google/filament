// RUN: %dxc -T lib_6_7 %s -verify

// Make sure no warning for shader model < 6.8
// expected-no-diagnostics

SamplerState s;
Texture1D t;

[shader("vertex")]
float4 vs2(float a:A) : SV_Position {
  return t.CalculateLevelOfDetail(s, a) +
    t.CalculateLevelOfDetailUnclamped(s, a);
}

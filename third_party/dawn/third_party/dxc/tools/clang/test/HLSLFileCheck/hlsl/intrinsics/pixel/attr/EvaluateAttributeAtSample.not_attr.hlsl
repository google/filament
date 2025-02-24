// RUN: %dxc -T ps_6_0 -E main %s -O3 | FileCheck %s

// CHECK: attribute evaluation can only be done on values taken directly from inputs

Texture2D tex0 : register(t0);
SamplerState samp0 : register(s0);

float4 main(float2 uv : TEXCOORD0) : SV_TARGET {
  float4 result = float4(0,0,0,0);
  int i = 0;
  [branch]
  if (uv.x > 0.5) {
    float2 coord = EvaluateAttributeAtSample(result.xy, i);
    result += tex0.Sample(samp0, coord);
  }
  return result;
}





// RUN: not %dxc -T ps_6_0 -E main -HV 2021 -fcgl  %s -spirv 2>&1 | FileCheck %s

SamplerState sampler_a;
SamplerState sampler_b;
Texture2D tex;
bool selector;

float4 main() : SV_Target0 {
// CHECK: error: casting to type 'SamplerState' unimplemente
  return tex.Sample(selector ? sampler_a : sampler_b, (float2)0);
}

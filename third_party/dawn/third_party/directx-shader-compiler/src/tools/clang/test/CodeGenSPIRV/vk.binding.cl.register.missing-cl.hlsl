// RUN: not %dxc -T ps_6_0 -E main -fvk-bind-register t5 1 10 1 -fcgl  %s -spirv  2>&1 | FileCheck %s

Texture2D MyTexture    : register(t5, space1);
SamplerState MySampler : register(s0);

float4 main() : SV_Target {
  return MyTexture.Sample(MySampler, float2(0.1, 0.2));
}

// CHECK: :4:14: error: missing -fvk-bind-register for resource

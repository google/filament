// RUN: not %dxc -T ps_6_0 -E main -fvk-bind-register s10 0 10 0 -fcgl  %s -spirv  2>&1 | FileCheck %s

Texture2D MyTexture;
SamplerState MySampler;

float4 main() : SV_Target {
  return MyTexture.Sample(MySampler, float2(0.1, 0.2));
}

// CHECK: :3:11: error: -fvk-bind-register requires register annotations on all resources

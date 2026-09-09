// RUN: not %dxc -T ps_6_8 -E main -fvk-bind-register s10 0 10 0 -fcgl  %s -spirv  2>&1 | FileCheck %s

SamplerState Sampler : register(s10);

float4 main() : SV_Target {
  Texture2D Texture = ResourceDescriptorHeap[0];
  return Texture.Sample(Sampler, float2(0.1, 0.2));
}

// CHECK: error: -fvk-bind-resource-heap is required when using -fvk-bind-register

// RUN: %dxc -T ps_6_8 -E main -fvk-bind-register s10 0 1 2 -fcgl  -fvk-bind-sampler-heap 3 4 %s -spirv | FileCheck %s

// CHECK: OpDecorate %Texture DescriptorSet 2
// CHECK: OpDecorate %Texture Binding 1
Texture2D Texture : register(s10);

// CHECK: OpDecorate %SamplerDescriptorHeap DescriptorSet 4
// CHECK: OpDecorate %SamplerDescriptorHeap Binding 3

float4 main() : SV_Target {
  SamplerState Sampler = SamplerDescriptorHeap[0];
  return Texture.Sample(Sampler, float2(0.1, 0.2));
}

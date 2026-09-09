// RUN: %dxc -T ps_6_8 -E main -fvk-bind-register s10 0 1 2 -fcgl  -fvk-bind-resource-heap 3 4 %s -spirv | FileCheck %s

// CHECK: OpDecorate %Sampler DescriptorSet 2
// CHECK: OpDecorate %Sampler Binding 1
SamplerState Sampler : register(s10);

// CHECK: OpDecorate %ResourceDescriptorHeap DescriptorSet 4
// CHECK: OpDecorate %ResourceDescriptorHeap Binding 3

float4 main() : SV_Target {
  Texture2D Texture = ResourceDescriptorHeap[0];
  return Texture.Sample(Sampler, float2(0.1, 0.2));
}

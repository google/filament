// RUN: %dxc -T ps_6_6 -E main -fcgl %s -spirv -fvk-bind-sampler-heap 3 4 | FileCheck %s

// CHECK-DAG: OpDecorate %Texture DescriptorSet 0
// CHECK-DAG: OpDecorate %Texture Binding 0
Texture2D Texture;

float4 main() : SV_Target {
// CHECK-DAG: OpDecorate %SamplerDescriptorHeap DescriptorSet 4
// CHECK-DAG: OpDecorate %SamplerDescriptorHeap Binding 3
  SamplerState Sampler = SamplerDescriptorHeap[0];
  return Texture.Sample(Sampler, float2(0, 0));
}


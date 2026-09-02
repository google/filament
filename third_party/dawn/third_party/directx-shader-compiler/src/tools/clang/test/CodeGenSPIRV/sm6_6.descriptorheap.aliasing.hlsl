// RUN: %dxc -T ps_6_6 -spirv %s -fvk-bind-sampler-heap 4 7 -fvk-bind-resource-heap 4 7 | FileCheck %s

// CHECK-DAG: OpDecorate %ResourceDescriptorHeap DescriptorSet 7
// CHECK-DAG: OpDecorate %ResourceDescriptorHeap Binding 4
// CHECK-DAG: OpDecorate %SamplerDescriptorHeap DescriptorSet 7
// CHECK-DAG: OpDecorate %SamplerDescriptorHeap Binding 4

float4 main() : SV_Target {
  SamplerState Sampler = SamplerDescriptorHeap[2];
  Texture2D Texture = ResourceDescriptorHeap[3];
  return Texture.Sample(Sampler, float2(0, 0));
}


// RUN: %dxc -T ps_6_6 -E main %s -spirv | FileCheck %s

// CHECK-DAG: OpDecorate %a DescriptorSet 3
// CHECK-DAG: OpDecorate %a Binding 2
[[vk::binding(2, 3)]]
cbuffer a {
  float4 value;
}

float4 main() : SV_Target {
  RWStructuredBuffer<float4> buffer = ResourceDescriptorHeap[1];
  buffer[0] = value;
  return value;
}


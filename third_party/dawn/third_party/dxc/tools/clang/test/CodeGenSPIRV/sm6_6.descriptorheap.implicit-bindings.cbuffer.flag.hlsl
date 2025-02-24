// RUN: %dxc -T ps_6_6 -E main %s -spirv -fvk-bind-resource-heap 6 7 | FileCheck %s

// CHECK-DAG: OpDecorate %a DescriptorSet 0
// CHECK-DAG: OpDecorate %a Binding 0
cbuffer a {
  float4 value;
}

float4 main() : SV_Target {
// CHECK-DAG: OpDecorate %ResourceDescriptorHeap DescriptorSet 7
// CHECK-DAG: OpDecorate %ResourceDescriptorHeap Binding 6
  RWStructuredBuffer<float4> buffer = ResourceDescriptorHeap[1];
  buffer[0] = value;
  return value;
}


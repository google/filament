// RUN: %dxc -T cs_6_6 -E main -fcgl %s -spirv | FileCheck %s

// CHECK-DAG: OpDecorate %a DescriptorSet 4
// CHECK-DAG: OpDecorate %a Binding 3
// CHECK-DAG: OpDecorate %counter_var_a DescriptorSet 4
// CHECK-DAG: OpDecorate %counter_var_a Binding 4
[[vk::binding(3, 4), vk::counter_binding(4)]]
RWStructuredBuffer<uint> a;

[numthreads(1, 1, 1)]
void main() {
// CHECK-DAG: OpDecorate %ResourceDescriptorHeap DescriptorSet 0
// CHECK-DAG: OpDecorate %ResourceDescriptorHeap Binding 0
// CHECK-DAG: OpDecorate %counter_var_ResourceDescriptorHeap DescriptorSet 0
// CHECK-DAG: OpDecorate %counter_var_ResourceDescriptorHeap Binding 1
  RWStructuredBuffer<uint> b = ResourceDescriptorHeap[1];

  a.IncrementCounter();
  b.IncrementCounter();
}

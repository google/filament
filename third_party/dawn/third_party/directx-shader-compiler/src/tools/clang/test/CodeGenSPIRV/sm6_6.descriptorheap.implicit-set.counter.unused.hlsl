// RUN: %dxc -T cs_6_6 -E main -fcgl %s -spirv | FileCheck %s

// CHECK-DAG: OpDecorate %a DescriptorSet 0
// CHECK-DAG: OpDecorate %a Binding 0
// CHECK-NOT: OpDecorate %counter_var_a DescriptorSet
// CHECK-NOT: OpDecorate %counter_var_a Binding
[[vk::counter_binding(1)]]
RWStructuredBuffer<uint> a;

[numthreads(1, 1, 1)]
void main() {
// CHECK-DAG: OpDecorate %ResourceDescriptorHeap DescriptorSet 0
// CHECK-DAG: OpDecorate %ResourceDescriptorHeap Binding 1
// CHECK-DAG: OpDecorate %counter_var_ResourceDescriptorHeap DescriptorSet 0
// CHECK-DAG: OpDecorate %counter_var_ResourceDescriptorHeap Binding 2
  RWStructuredBuffer<uint> b = ResourceDescriptorHeap[1];

  b.IncrementCounter();
}

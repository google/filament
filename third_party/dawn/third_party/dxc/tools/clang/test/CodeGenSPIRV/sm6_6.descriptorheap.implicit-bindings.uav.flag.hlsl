// RUN: %dxc -T cs_6_6 -E main -fcgl %s -spirv -fvk-bind-resource-heap 6 7 -fvk-bind-counter-heap 8 9 | FileCheck %s

// CHECK-DAG: OpDecorate %a DescriptorSet 0
// CHECK-DAG: OpDecorate %a Binding 0
// CHECK-DAG: OpDecorate %counter_var_a DescriptorSet 0
// CHECK-DAG: OpDecorate %counter_var_a Binding 1
RWStructuredBuffer<uint> a;

[numthreads(1, 1, 1)]
void main() {
// CHECK-DAG: OpDecorate %ResourceDescriptorHeap DescriptorSet 7
// CHECK-DAG: OpDecorate %ResourceDescriptorHeap Binding 6
// CHECK-DAG: OpDecorate %counter_var_ResourceDescriptorHeap DescriptorSet 9
// CHECK-DAG: OpDecorate %counter_var_ResourceDescriptorHeap Binding 8
  RWStructuredBuffer<uint> b = ResourceDescriptorHeap[1];

  a.IncrementCounter();
  b.IncrementCounter();
}


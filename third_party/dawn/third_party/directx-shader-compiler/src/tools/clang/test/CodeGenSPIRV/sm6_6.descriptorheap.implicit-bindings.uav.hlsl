// RUN: %dxc -T cs_6_6 -E main -fcgl %s -spirv | FileCheck %s

// CHECK-DAG: OpDecorate %a DescriptorSet 0
// CHECK-DAG: OpDecorate %a Binding 0
// CHECK-DAG: OpDecorate %counter_var_a DescriptorSet 0
// CHECK-DAG: OpDecorate %counter_var_a Binding 1
RWStructuredBuffer<uint> a;

[numthreads(1, 1, 1)]
void main() {
// CHECK-DAG: OpDecorate %ResourceDescriptorHeap DescriptorSet 0
// CHECK-DAG: OpDecorate %ResourceDescriptorHeap Binding 2
// CHECK-DAG: OpDecorate %counter_var_ResourceDescriptorHeap DescriptorSet 0
// CHECK-DAG: OpDecorate %counter_var_ResourceDescriptorHeap Binding 3
  RWStructuredBuffer<uint> b = ResourceDescriptorHeap[1];

  a.IncrementCounter();
  b.IncrementCounter();
}

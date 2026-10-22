// RUN: %dxc -T cs_6_6 -E main -fcgl %s -spirv | FileCheck %s

RWStructuredBuffer<uint> GetBindlessResource_UIntBuffer() {
     return  ResourceDescriptorHeap[0];
}

[numthreads(1, 1, 1)]
void main() {
// CHECK-DAG: OpDecorate %ResourceDescriptorHeap DescriptorSet 0
// CHECK-DAG: OpDecorate %ResourceDescriptorHeap Binding 0
// CHECK-DAG: OpDecorate %counter_var_ResourceDescriptorHeap DescriptorSet 0
// CHECK-DAG: OpDecorate %counter_var_ResourceDescriptorHeap Binding 1
  RWStructuredBuffer<uint> a = GetBindlessResource_UIntBuffer();

  a.IncrementCounter();
}

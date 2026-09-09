// RUN: %dxc -T cs_6_6 -E main -O3 %s -spirv | FileCheck %s

RWStructuredBuffer<uint> GetBindlessResource_UIntBuffer_inner() {
     return  ResourceDescriptorHeap[1];
}

RWStructuredBuffer<uint> GetBindlessResource_UIntBuffer_outter() {
  return GetBindlessResource_UIntBuffer_inner();
}

RWStructuredBuffer<uint> GetBindlessResource_UIntBuffer() {
     return  ResourceDescriptorHeap[0];
}

[numthreads(1, 1, 1)]
void main() {
  RWStructuredBuffer<uint> a = GetBindlessResource_UIntBuffer();
  RWStructuredBuffer<uint> b = GetBindlessResource_UIntBuffer_outter();
// CHECK-DAG: [[counter_a:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int %counter_var_ResourceDescriptorHeap %uint_0 %uint_0
// CHECK-DAG: [[counter_b:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int %counter_var_ResourceDescriptorHeap %uint_1 %uint_0

  a.IncrementCounter();
  b.IncrementCounter();

// CHECK-DAG: {{%[0-9]+}} = OpAtomicIAdd %int [[counter_a]] %uint_1 %uint_0 %int_1
// CHECK-DAG: {{%[0-9]+}} = OpAtomicIAdd %int [[counter_b]] %uint_1 %uint_0 %int_1
}

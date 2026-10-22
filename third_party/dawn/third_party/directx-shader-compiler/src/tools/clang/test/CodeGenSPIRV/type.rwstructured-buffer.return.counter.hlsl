// RUN: %dxc -T cs_6_6 -E main -O3 %s -spirv | FileCheck %s

RWStructuredBuffer<uint> resource;

RWStructuredBuffer<uint> get_resource_inner() {
     return resource;
}

RWStructuredBuffer<uint> get_resource_outter() {
  return get_resource_inner();
}

RWStructuredBuffer<uint> get_resource() {
     return resource;
}

[numthreads(1, 1, 1)]
void main() {
  RWStructuredBuffer<uint> a = get_resource();
  RWStructuredBuffer<uint> b = get_resource_outter();
// CHECK-DAG: [[counter:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int %counter_var_resource %uint_0

  a.IncrementCounter();
  b.IncrementCounter();

// CHECK-DAG: {{%[0-9]+}} = OpAtomicIAdd %int [[counter]] %uint_1 %uint_0 %int_1
// CHECK-DAG: {{%[0-9]+}} = OpAtomicIAdd %int [[counter]] %uint_1 %uint_0 %int_1
}

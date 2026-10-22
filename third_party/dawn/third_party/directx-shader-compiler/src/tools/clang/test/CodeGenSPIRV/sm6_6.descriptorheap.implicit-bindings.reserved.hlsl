// RUN: %dxc -T cs_6_6 -E main -fcgl %s -spirv | FileCheck %s --check-prefix=CHECK-DEFAULT
// RUN: %dxc -T cs_6_6 -E main -fcgl %s -spirv -fvk-bind-resource-heap 0 0 -fvk-bind-counter-heap 1 0 -fvk-bind-sampler-heap 2 0 | FileCheck %s --check-prefix=CHECK-RESERVED

// CHECK-DEFAULT-DAG: OpDecorate %a DescriptorSet 0
// CHECK-DEFAULT-DAG: OpDecorate %a Binding 0
// CHECK-DEFAULT-DAG: OpDecorate %counter_var_a DescriptorSet 0
// CHECK-DEFAULT-DAG: OpDecorate %counter_var_a Binding 1

// CHECK-RESERVED-DAG: OpDecorate %a DescriptorSet 0
// CHECK-RESERVED-DAG: OpDecorate %a Binding 3
// CHECK-RESERVED-DAG: OpDecorate %counter_var_a DescriptorSet 0
// CHECK-RESERVED-DAG: OpDecorate %counter_var_a Binding 4
RWStructuredBuffer<uint> a;

[numthreads(1, 1, 1)]
void main() {
  a.IncrementCounter();
}

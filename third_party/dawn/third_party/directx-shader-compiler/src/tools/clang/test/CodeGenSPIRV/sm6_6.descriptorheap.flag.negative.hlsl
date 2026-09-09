// RUN: not %dxc -T cs_6_6 -spirv %s 2>&1 -fvk-bind-sampler-heap  -1  2 | FileCheck %s --check-prefix=CHECK-SAMPLER
// RUN: not %dxc -T cs_6_6 -spirv %s 2>&1 -fvk-bind-sampler-heap   1 -1 | FileCheck %s --check-prefix=CHECK-SAMPLER
// RUN: not %dxc -T cs_6_6 -spirv %s 2>&1 -fvk-bind-resource-heap -1  2 | FileCheck %s --check-prefix=CHECK-RESOURCE
// RUN: not %dxc -T cs_6_6 -spirv %s 2>&1 -fvk-bind-resource-heap  1 -1 | FileCheck %s --check-prefix=CHECK-RESOURCE
// RUN: not %dxc -T cs_6_6 -spirv %s 2>&1 -fvk-bind-counter-heap  -1  2 | FileCheck %s --check-prefix=CHECK-COUNTER
// RUN: not %dxc -T cs_6_6 -spirv %s 2>&1 -fvk-bind-counter-heap   1 -1 | FileCheck %s --check-prefix=CHECK-COUNTER

//  CHECK-SAMPLER: expected positive integer for -fvk-bind-sampler-heap, got: -1
// CHECK-RESOURCE: expected positive integer for -fvk-bind-resource-heap, got: -1
//  CHECK-COUNTER: expected positive integer for -fvk-bind-counter-heap, got: -1
[numthreads(1, 1, 1)]
void main() {
  SamplerState Sampler = SamplerDescriptorHeap[0];
}

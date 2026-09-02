// RUN: not %dxc -T cs_6_6 -spirv %s 2>&1 -fvk-bind-sampler-heap 1 | FileCheck %s --check-prefix=CHECK-SAMPLER
// RUN: not %dxc -T cs_6_6 -spirv %s 2>&1 -fvk-bind-sampler-heap   | FileCheck %s --check-prefix=CHECK-SAMPLER
// RUN: not %dxc -T cs_6_6 -spirv %s 2>&1 -fvk-bind-resource-heap 1 | FileCheck %s --check-prefix=CHECK-RESOURCE
// RUN: not %dxc -T cs_6_6 -spirv %s 2>&1 -fvk-bind-resource-heap   | FileCheck %s --check-prefix=CHECK-RESOURCE
// RUN: not %dxc -T cs_6_6 -spirv %s 2>&1 -fvk-bind-counter-heap 1 | FileCheck %s --check-prefix=CHECK-COUNTER
// RUN: not %dxc -T cs_6_6 -spirv %s 2>&1 -fvk-bind-counter-heap   | FileCheck %s --check-prefix=CHECK-COUNTER

// CHECK-SAMPLER: Argument to '-fvk-bind-sampler-heap' is missing.
// CHECK-RESOURCE: Argument to '-fvk-bind-resource-heap' is missing.
// CHECK-COUNTER: Argument to '-fvk-bind-counter-heap' is missing.
[numthreads(1, 1, 1)]
void main() {
  SamplerState Sampler = SamplerDescriptorHeap[0];
}

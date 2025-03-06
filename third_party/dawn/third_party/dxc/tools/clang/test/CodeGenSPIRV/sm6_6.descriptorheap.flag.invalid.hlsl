// RUN: not %dxc -T cs_6_6 -spirv %s 2>&1 -fvk-bind-sampler-heap  a 2 | FileCheck %s --check-prefix=CHECK-SAMPLER-A
// RUN: not %dxc -T cs_6_6 -spirv %s 2>&1 -fvk-bind-sampler-heap  1 b | FileCheck %s --check-prefix=CHECK-SAMPLER-B
// RUN: not %dxc -T cs_6_6 -spirv %s 2>&1 -fvk-bind-resource-heap a 2 | FileCheck %s --check-prefix=CHECK-RESOURCE-A
// RUN: not %dxc -T cs_6_6 -spirv %s 2>&1 -fvk-bind-resource-heap 1 b | FileCheck %s --check-prefix=CHECK-RESOURCE-B
// RUN: not %dxc -T cs_6_6 -spirv %s 2>&1 -fvk-bind-counter-heap  a 2 | FileCheck %s --check-prefix=CHECK-COUNTER-A
// RUN: not %dxc -T cs_6_6 -spirv %s 2>&1 -fvk-bind-counter-heap  1 b | FileCheck %s --check-prefix=CHECK-COUNTER-B

//  CHECK-SAMPLER-A: invalid -fvk-bind-sampler-heap argument: 'a'
// CHECK-RESOURCE-A: invalid -fvk-bind-resource-heap argument: 'a'
//  CHECK-COUNTER-A: invalid -fvk-bind-counter-heap argument: 'a'
//  CHECK-SAMPLER-B: invalid -fvk-bind-sampler-heap argument: 'b'
// CHECK-RESOURCE-B: invalid -fvk-bind-resource-heap argument: 'b'
//  CHECK-COUNTER-B: invalid -fvk-bind-counter-heap argument: 'b'
[numthreads(1, 1, 1)]
void main() {
  SamplerState Sampler = SamplerDescriptorHeap[0];
}

// RUN: %dxc -T ps_6_0 -E main -fcgl -spirv %s | FileCheck %s

RWByteAddressBuffer foo;

void main() {
  float bar;
// CHECK:      [[foo:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %foo %uint_0 {{%[0-9]+}}
// CHECK-NEXT: [[ptr:%[0-9]+]] = OpAtomicIAdd %uint [[foo]] %uint_1 %uint_0 %uint_42
// CHECK-NEXT: [[res:%[0-9]+]] = OpConvertUToF %float [[ptr]]
// CHECK-NEXT:                   OpStore %bar [[res]]
  foo.InterlockedAdd(16, 42, bar);
}

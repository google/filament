// RUN: %dxc -T cs_6_1 -E main -Zi -fcgl  %s -spirv | FileCheck %s

// CHECK:      [[str:%[0-9]+]] = OpString
// CHECK-SAME: spirv.debug.opsource.hlsl
// CHECK:      OpSource HLSL 610 [[str]] "// RUN: %dxc -T cs_6_1 -E main -Zi -fcgl  %s -spirv | FileCheck %s

// Make sure we have the original source code
// CHECK:      numthreads(8, 1, 1)
// CHECK-NEXT: void main()

[numthreads(8, 1, 1)]
void main() {
}

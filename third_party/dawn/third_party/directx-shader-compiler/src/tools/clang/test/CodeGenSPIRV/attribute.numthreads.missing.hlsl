// RUN: not %dxc -T cs_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

// CHECK: 4:6: error: compute entry point must have a valid numthreads attribute
void main() {}

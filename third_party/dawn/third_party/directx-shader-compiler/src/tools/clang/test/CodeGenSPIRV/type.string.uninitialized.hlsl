// RUN: not %dxc -T cs_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

// CHECK: 4:8: error: Found uninitialized string variable.
string first;

[numthreads(1,1,1)]
void main() {}


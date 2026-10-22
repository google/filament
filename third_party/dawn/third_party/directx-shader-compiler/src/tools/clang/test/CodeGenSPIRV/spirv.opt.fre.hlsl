// RUN: not %dxc -T ps_6_0 -E main -spirv -Fre file.ext -fcgl  %s -spirv 2>&1 | FileCheck %s

void main() {}

// CHECK: -Fre is not supported with -spirv

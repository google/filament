// RUN: not %dxc -T ps_6_0 -E main -spirv -Fd file.ext -fcgl  %s -spirv 2>&1 | FileCheck %s

void main() {}

// CHECK: -Fd is not supported with -spirv

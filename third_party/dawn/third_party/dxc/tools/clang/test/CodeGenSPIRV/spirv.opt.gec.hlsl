// RUN: not %dxc -T ps_6_0 -E main -spirv -Gec -fcgl  %s -spirv 2>&1 | FileCheck %s

void main() {}

// CHECK: -Gec is not supported with -spirv

// RUN: not %dxc -T ps_6_0 -E main -O2 -Oconfig=--loop-unroll  %s -spirv 2>&1 | FileCheck %s

void main() {}

// CHECK: -Oconfig should not be used together with -O

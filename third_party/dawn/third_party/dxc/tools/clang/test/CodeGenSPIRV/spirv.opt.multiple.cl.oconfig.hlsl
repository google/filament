// RUN: not %dxc -T ps_6_0 -E main -Oconfig=-O -Oconfig=-Os  %s -spirv 2>&1 | FileCheck %s

void main() {}

// CHECK: -Oconfig should not be specified more than once

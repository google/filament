// RUN: not %dxc -T ps_6_0 -E main -Oconfig=--test-unknown-flag  %s -spirv 2>&1 | FileCheck %s

void main() {}

// CHECK: failed to optimize SPIR-V: Unknown flag '--test-unknown-flag'

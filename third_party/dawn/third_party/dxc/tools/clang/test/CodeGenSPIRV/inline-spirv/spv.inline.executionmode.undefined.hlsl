// RUN: not %dxc -T ps_6_0 -E main -fcgl %s -spirv 2>&1 | FileCheck %s

// CHECK: Invalid execution mode operand: 999999
[[vk::spvexecutionmode(999999)]]
void main() {}

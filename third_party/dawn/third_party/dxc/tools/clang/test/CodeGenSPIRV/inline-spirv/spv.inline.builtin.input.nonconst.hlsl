// RUN: not %dxc -T ps_6_0 -E main -fcgl  %s -spirv 2>&1 | FileCheck %s

// CHECK: error: vk::ext_builtin_input can only be applied to a const-qualified variable
[[vk::ext_builtin_input(/* NumWorkgroups */ 24)]]
static uint3 invalid;

void main() {
}

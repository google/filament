// RUN: not %dxc -T ps_6_0 -E main -fcgl  %s -spirv 2>&1 | FileCheck %s

// CHECK: error: vk::ext_builtin_input cannot be used together with vk::ext_builtin_output
[[vk::ext_builtin_input(/* NumWorkgroups */ 24)]]
[[vk::ext_builtin_output(/* NumWorkgroups */ 24)]]
static uint3 invalid;

void main() {
}

// RUN: not %dxc -T ps_6_0 -E main -fcgl  %s -spirv 2>&1 | FileCheck %s

// CHECK: error: vk::ext_builtin_input and vk::ext_builtin_output can only be applied to a static variable
[[vk::ext_builtin_output(/* NumWorkgroups */ 24)]]
uint3 invalid;

void main() {
}

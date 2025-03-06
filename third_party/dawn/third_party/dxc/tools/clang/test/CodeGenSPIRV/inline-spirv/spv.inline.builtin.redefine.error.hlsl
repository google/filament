// RUN: not %dxc -T ps_6_0 -E main -fcgl  %s -spirv 2>&1 | FileCheck %s

[[vk::ext_builtin_input(/* NumWorkgroups */ 24)]]
static const uint3 gl_NumWorkGroups;

// CHECK: error: cannot redefine builtin 24 as an output
// CHECK: warning: previous definition is here
[[vk::ext_builtin_output(/* NumWorkgroups */ 24)]]
static uint3 invalid;

void main() {
}

// RUN: not %dxc -T ps_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

// CHECK: error: Shaders must not specify more than one of stencil_ref_unchanged_front, stencil_ref_greater_equal_front, and stencil_ref_less_equal_front.

[[vk::early_and_late_tests]]
[[vk::stencil_ref_less_equal_front]]
[[vk::stencil_ref_greater_equal_front]]
void main() {}

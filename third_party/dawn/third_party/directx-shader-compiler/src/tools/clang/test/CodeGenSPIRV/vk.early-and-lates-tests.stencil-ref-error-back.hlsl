// RUN: not %dxc -T ps_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

// CHECK: error: Shaders must not specify more than one of stencil_ref_unchanged_back, stencil_ref_greater_equal_back, and stencil_ref_less_equal_back.

[[vk::early_and_late_tests]]
[[vk::stencil_ref_less_equal_back]]
[[vk::stencil_ref_greater_equal_back]]
void main() {}

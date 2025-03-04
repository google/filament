// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpExtension "SPV_AMD_shader_early_and_late_fragment_tests"
// CHECK: OpExecutionMode %main EarlyAndLateFragmentTestsAMD
// CHECK: OpExecutionMode %main StencilRefLessFrontAMD

[[vk::early_and_late_tests]]
[[vk::stencil_ref_less_equal_front]]
void main() {}

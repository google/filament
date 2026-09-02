// RUN: %dxc -T ps_6_0 -E main -spirv -fcgl -Vd  %s -spirv | FileCheck %s

// CHECK: OpCapability ShaderClockKHR
// CHECK: OpExtension "SPV_KHR_shader_clock"
// CHECK: OpExecutionMode {{%[a-zA-Z0-9_]+}} StencilRefReplacingEXT
// CHECK: OpExecutionMode {{%[a-zA-Z0-9_]+}} SubgroupSize 32
// CHECK: OpDecorate {{%[a-zA-Z0-9_]+}} BuiltIn FragStencilRefEXT

[[vk::ext_decorate(11, 5014)]]
int main() : SV_Target0 {
  [[vk::ext_capability(5055)]]
  [[vk::ext_extension("SPV_KHR_shader_clock")]]
  vk::ext_execution_mode(/*StencilRefReplacingEXT*/5027);

  vk::ext_execution_mode(/*SubgroupSize*/35, 32);
  return 3;
}

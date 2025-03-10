// RUN: %dxc -T ps_6_0 -E main -spirv -fcgl -fspv-target-env=vulkan1.1 -Vd %s | FileCheck %s

// CHECK: OpCapability ShaderClockKHR
// CHECK: OpExtension "SPV_KHR_shader_clock"
// CHECK: OpExecutionModeId {{%[a-zA-Z0-9_]+}} LocalSizeId %uint_8 %uint_8 %uint_8
// CHECK: OpExecutionModeId {{%[a-zA-Z0-9_]+}} LocalSizeHintId %uint_4 %uint_4 %uint_4

int main() : SV_Target0 {
  vk::ext_execution_mode_id(/*LocalSizeId*/38, 8, 8, 8);

  [[vk::ext_capability(5055)]]
  [[vk::ext_extension("SPV_KHR_shader_clock")]]
  vk::ext_execution_mode_id(/*LocalSizeHintId*/39, 4, 4, 4);

  return 3;
}

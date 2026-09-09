// RUN: %dxc -T vs_6_0 -E main -fspv-target-env=vulkan1.1 -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability ShaderClockKHR

// CHECK: OpExtension "SPV_KHR_non_semantic_info"

// CHECK-DAG:   [[glsl_set:%[0-9]+]] = OpExtInstImport "GLSL.std.450"
// CHECK-DAG:  [[debug_set:%[0-9]+]] = OpExtInstImport "NonSemantic.DebugBreak"
// CHECK-DAG: [[minmax_set:%[0-9]+]] = OpExtInstImport "SPV_AMD_shader_trinary_minmax"

struct SInstanceData {
  float4x3 VisualToWorld;
  float4 Output;
};

struct VS_INPUT	{
  float3 Position : POSITION;
  SInstanceData	InstanceData : TEXCOORD4;
};

[[vk::ext_capability(5055)]]
[[vk::ext_extension("SPV_KHR_shader_clock")]]
[[vk::ext_instruction(/* OpReadClockKHR */ 5056)]]
uint64_t ReadClock(uint scope);

[[vk::ext_instruction(/* Sin*/ 13, "GLSL.std.450")]]
float4 spv_sin(float4 v);

[[vk::ext_instruction(/* FMin3AMD */ 1, "SPV_AMD_shader_trinary_minmax")]]
float FMin3AMD(float x, float y, float z);

[[vk::ext_extension("SPV_KHR_non_semantic_info")]]
[[vk::ext_instruction(1, "NonSemantic.DebugBreak")]]
void debugBreak();

float4 main(const VS_INPUT v) : SV_Position {
	SInstanceData	I = v.InstanceData;
  uint64_t clock;
// CHECK: {{%[0-9]+}} = OpExtInst %v4float [[glsl_set]] Sin {{%[0-9]+}}
  I.Output = spv_sin(v.InstanceData.Output);
// CHECK: {{%[0-9]+}} = OpReadClockKHR %ulong %uint_1
  clock = ReadClock(vk::DeviceScope);
// CHECK: {{%[0-9]+}} = OpExtInst %float [[minmax_set]] FMin3AMD {{%[0-9]+}} {{%[0-9]+}} {{%[0-9]+}}
  I.Output.w = FMin3AMD(I.Output.z, I.Output.y, I.Output.z);

  debugBreak();
// CHECK: {{.*}} = OpExtInst %void [[debug_set]] 1

  return I.Output;
}

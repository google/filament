// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability ShaderClockKHR
// CHECK: OpExtension "SPV_KHR_shader_clock"

struct SInstanceData {
  float4x3 VisualToWorld;
  float4 Output;
};

struct VS_INPUT	{
  float3 Position : POSITION;
  SInstanceData	InstanceData : TEXCOORD4;
};

float4 main(const VS_INPUT v) : SV_Position {
	const SInstanceData	I = v.InstanceData;
  uint64_t clock;
// CHECK: {{%[0-9]+}} = OpReadClockKHR %ulong %uint_1
  clock = vk::ReadClock(vk::DeviceScope);
// CHECK: {{%[0-9]+}} = OpReadClockKHR %ulong %uint_3
  clock = vk::ReadClock(vk::SubgroupScope);
  return I.Output;
}

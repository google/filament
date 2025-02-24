// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

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
// CHECK: error: use of undeclared identifier 'vk'
  clock = vk::ReadClock(vk::DeviceScope);
// CHECK: error: use of undeclared identifier 'vk'
  clock = vk::ReadClock(vk::SubgroupScope);
  return I.Output;
}

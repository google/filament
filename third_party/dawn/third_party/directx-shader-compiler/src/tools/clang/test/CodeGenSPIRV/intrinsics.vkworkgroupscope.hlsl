// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

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

// CHECK: OpStore %scope %uint_2
  uint32_t scope = vk::WorkgroupScope;

  return I.Output;
}


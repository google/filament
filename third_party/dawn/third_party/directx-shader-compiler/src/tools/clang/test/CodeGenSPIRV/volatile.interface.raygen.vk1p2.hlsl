// RUN: %dxc -T lib_6_3 -fspv-extension=SPV_NV_ray_tracing -fspv-extension=SPV_KHR_ray_query -fspv-target-env=vulkan1.2 -O0  %s -spirv | FileCheck %s

// CHECK: OpCapability VulkanMemoryModel

// CHECK: OpMemoryModel Logical Vulkan

RaytracingAccelerationStructure rs;

struct Payload
{
  float4 color;
};
struct CallData
{
  float4 data;
};

[shader("raygeneration")]
void main() {

  uint3 a = DispatchRaysIndex();
  uint3 b = DispatchRaysDimensions();

  Payload myPayload = { float4(0.0f,0.0f,0.0f,0.0f) };
  CallData myCallData = { float4(0.0f,0.0f,0.0f,0.0f) };
  RayDesc rayDesc;
  rayDesc.Origin = float3(0.0f, 0.0f, 0.0f);
  rayDesc.Direction = float3(0.0f, 0.0f, -1.0f);
  rayDesc.TMin = 0.0f;
  rayDesc.TMax = 1000.0f;

// CHECK: OpDecorate [[SubgroupSize:%[a-zA-Z0-9_]+]] BuiltIn SubgroupSize
// CHECK: [[SubgroupSize]] = OpVariable %_ptr_Input_uint Input
// CHECK: OpLoad %uint [[SubgroupSize]] Volatile
  TraceRay(rs, 0x0, WaveGetLaneCount(), 0, 1, 0, rayDesc, myPayload);
  CallShader(0, myCallData);
}

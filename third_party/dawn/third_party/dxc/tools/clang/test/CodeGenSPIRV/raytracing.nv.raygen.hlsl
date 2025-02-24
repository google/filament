// RUN: %dxc -T lib_6_3 -fspv-extension=SPV_NV_ray_tracing -fspv-extension=SPV_KHR_ray_query -fcgl  %s -spirv | FileCheck %s
// CHECK:  OpCapability RayTracingNV
// CHECK:  OpExtension "SPV_NV_ray_tracing"
// CHECK:  OpDecorate [[a:%[0-9]+]] BuiltIn LaunchIdKHR
// CHECK:  OpDecorate [[b:%[0-9]+]] BuiltIn LaunchSizeKHR

// CHECK: %accelerationStructureNV = OpTypeAccelerationStructureKHR
// CHECK-NOT: OpTypeAccelerationStructureKHR
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

// CHECK:  OpLoad %v3uint [[a]]
  uint3 a = DispatchRaysIndex();
// CHECK:  OpLoad %v3uint [[b]]
  uint3 b = DispatchRaysDimensions();

  Payload myPayload = { float4(0.0f,0.0f,0.0f,0.0f) };
  CallData myCallData = { float4(0.0f,0.0f,0.0f,0.0f) };
  RayDesc rayDesc;
  rayDesc.Origin = float3(0.0f, 0.0f, 0.0f);
  rayDesc.Direction = float3(0.0f, 0.0f, -1.0f);
  rayDesc.TMin = 0.0f;
  rayDesc.TMax = 1000.0f;
  // CHECK: OpTraceNV {{%[0-9]+}} %uint_0 %uint_255 %uint_0 %uint_1 %uint_0 {{%[0-9]+}} {{%[0-9]+}} {{%[0-9]+}} {{%[0-9]+}} %uint_0
  TraceRay(rs, 0x0, 0xff, 0, 1, 0, rayDesc, myPayload);
  // CHECK: OpExecuteCallableNV %uint_0 %uint_0
  CallShader(0, myCallData);
}

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

//CHECK:      %First = OpVariable %_ptr_Private_int Private %int_0
//CHECK-NEXT: %Second = OpVariable %_ptr_Private_int Private %int_1
enum Number {
  First,
  Second,
};

//CHECK:      [[first:%[0-9]+]] = OpLoad %int %First
//CHECK-NEXT:                  OpStore %foo [[first]]
static ::Number foo = First;

[shader("raygeneration")]
void main() {
//CHECK:      [[second:%[0-9]+]] = OpLoad %int %Second
//CHECK-NEXT:                   OpStore %bar [[second]]
  static ::Number bar = Second;

  uint3 a = DispatchRaysIndex();
  uint3 b = DispatchRaysDimensions();

  Payload myPayload = { float4(0.0f,0.0f,0.0f,0.0f) };
  CallData myCallData = { float4(0.0f,0.0f,0.0f,0.0f) };
  RayDesc rayDesc;
  rayDesc.Origin = float3(0.0f, 0.0f, 0.0f);
  rayDesc.Direction = float3(0.0f, 0.0f, -1.0f);
  rayDesc.TMin = 0.0f;
  rayDesc.TMax = 1000.0f;
  TraceRay(rs, 0x0, 0xff, 0, 1, 0, rayDesc, myPayload);
  CallShader(0, myCallData);
}

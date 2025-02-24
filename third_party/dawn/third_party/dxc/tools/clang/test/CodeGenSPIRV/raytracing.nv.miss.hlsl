// RUN: %dxc -T lib_6_3 -fspv-extension=SPV_NV_ray_tracing -fcgl  %s -spirv | FileCheck %s
// CHECK:  OpCapability RayTracingNV
// CHECK:  OpExtension "SPV_NV_ray_tracing"
// CHECK:  OpDecorate [[a:%[0-9]+]] BuiltIn LaunchIdKHR
// CHECK:  OpDecorate [[b:%[0-9]+]] BuiltIn LaunchSizeKHR
// CHECK:  OpDecorate [[c:%[0-9]+]] BuiltIn WorldRayOriginKHR
// CHECK:  OpDecorate [[d:%[0-9]+]] BuiltIn WorldRayDirectionKHR
// CHECK:  OpDecorate [[e:%[0-9]+]] BuiltIn RayTminKHR
// CHECK:  OpDecorate [[f:%[0-9]+]] BuiltIn IncomingRayFlagsKHR

// CHECK:  OpTypePointer IncomingRayPayloadKHR %Payload
struct Payload
{
  float4 color;
};

[shader("miss")]
void main(inout Payload MyPayload) {

// CHECK:  OpLoad %v3uint [[a]]
  uint3 _1 = DispatchRaysIndex();
// CHECK:  OpLoad %v3uint [[b]]
  uint3 _2 = DispatchRaysDimensions();
// CHECK:  OpLoad %v3float [[c]]
  float3 _3 = WorldRayOrigin();
// CHECK:  OpLoad %v3float [[d]]
  float3 _4 = WorldRayDirection();
// CHECK:  OpLoad %float [[e]]
  float _5 = RayTMin();
// CHECK:  OpLoad %uint [[f]]
  uint _6 = RayFlags();
}

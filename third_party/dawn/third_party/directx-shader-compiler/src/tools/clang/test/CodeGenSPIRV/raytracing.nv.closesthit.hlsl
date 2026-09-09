// RUN: %dxc -T lib_6_3 -fspv-extension=SPV_NV_ray_tracing -fspv-extension=SPV_KHR_ray_query -fcgl  %s -spirv | FileCheck %s
// CHECK:  OpCapability RayTracingNV
// CHECK:  OpExtension "SPV_NV_ray_tracing"
// CHECK:  OpDecorate [[a:%[0-9]+]] BuiltIn LaunchIdKHR
// CHECK:  OpDecorate [[b:%[0-9]+]] BuiltIn LaunchSizeKHR
// CHECK:  OpDecorate [[c:%[0-9]+]] BuiltIn WorldRayOriginKHR
// CHECK:  OpDecorate [[d:%[0-9]+]] BuiltIn WorldRayDirectionKHR
// CHECK:  OpDecorate [[e:%[0-9]+]] BuiltIn RayTminKHR
// CHECK:  OpDecorate [[f:%[0-9]+]] BuiltIn IncomingRayFlagsKHR
// CHECK:  OpDecorate %gl_InstanceID BuiltIn InstanceId
// CHECK:  OpDecorate [[g:%[0-9]+]] BuiltIn InstanceCustomIndexKHR
// CHECK:  OpDecorate %gl_PrimitiveID BuiltIn PrimitiveId
// CHECK:  OpDecorate [[h:%[0-9]+]] BuiltIn ObjectRayOriginKHR
// CHECK:  OpDecorate [[i:%[0-9]+]] BuiltIn ObjectRayDirectionKHR
// CHECK:  OpDecorate [[j:%[0-9]+]] BuiltIn ObjectToWorldKHR
// CHECK:  OpDecorate [[k:%[0-9]+]] BuiltIn WorldToObjectKHR
// CHECK:  OpDecorate [[l:%[0-9]+]] BuiltIn HitKindKHR
// CHECK:  OpDecorate [[m:%[0-9]+]] BuiltIn HitTNV

// CHECK: %accelerationStructureNV = OpTypeAccelerationStructureKHR
// CHECK-NOT: OpTypeAccelerationStructureKHR

// CHECK:  OpTypePointer IncomingRayPayloadKHR %Payload
struct Payload
{
  float4 color;
};
// CHECK:  OpTypePointer HitAttributeKHR %Attribute
struct Attribute
{
  float2 bary;
};

RaytracingAccelerationStructure rs;

[shader("closesthit")]
void main(inout Payload MyPayload, in Attribute MyAttr) {

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
// CHECK:  OpLoad %uint %gl_InstanceID
  uint _7 = InstanceIndex();
// CHECK:  OpLoad %uint [[g]]
  uint _8 = InstanceID();
// CHECK:  OpLoad %uint %gl_PrimitiveID
  uint _9 = PrimitiveIndex();
// CHECK:  OpLoad %v3float [[h]]
  float3 _10 = ObjectRayOrigin();
// CHECK:  OpLoad %v3float [[i]]
  float3 _11 = ObjectRayDirection();
// CHECK: [[matotw:%[0-9]+]] = OpLoad %mat4v3float [[j]]
// CHECK-NEXT: OpTranspose %mat3v4float [[matotw]]
  float3x4 _12 = ObjectToWorld3x4();
// CHECK:  OpLoad %mat4v3float [[j]]
  float4x3 _13 = ObjectToWorld4x3();
// CHECK: [[matwto:%[0-9]+]] = OpLoad %mat4v3float [[k]]
// CHECK-NEXT: OpTranspose %mat3v4float [[matwto]]
  float3x4 _14 = WorldToObject3x4();
// CHECK:  OpLoad %mat4v3float [[k]]
  float4x3 _15 = WorldToObject4x3();
// CHECK:  OpLoad %uint [[l]]
  uint _16 = HitKind();
// CHECK:  OpLoad %float [[m]]
  uint _17 = RayTCurrent();

  Payload myPayload = { float4(0.0f,0.0f,0.0f,0.0f) };
  RayDesc rayDesc;
  rayDesc.Origin = float3(0.0f, 0.0f, 0.0f);
  rayDesc.Direction = float3(0.0f, 0.0f, -1.0f);
  rayDesc.TMin = 0.0f;
  rayDesc.TMax = 1000.0f;
// CHECK: OpTraceNV {{%[0-9]+}} %uint_0 %uint_255 %uint_0 %uint_1 %uint_0 {{%[0-9]+}} {{%[0-9]+}} {{%[0-9]+}} {{%[0-9]+}} %uint_0
  TraceRay(rs, 0x0, 0xff, 0, 1, 0, rayDesc, myPayload);
}

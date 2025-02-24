// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s | FileCheck %s

// Verify RayDesc from cbuffer works
// CHECK: call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59
// CHECK-SAME: i32 2)
// CHECK: call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59
// CHECK-SAME: i32 3)
// CHECK: call void @dx.op.traceRay.struct.Payload(i32 157,

struct Payload {
   float2 t;
   int3 t2;
};

RaytracingAccelerationStructure Acc;

uint RayFlags;
uint InstanceInclusionMask;
uint RayContributionToHitGroupIndex;
uint MultiplierForGeometryContributionToHitGroupIndex;
uint MissShaderIndex;

RayDesc Ray;

float4 emit(inout float2 f2, inout Payload p )  {
  TraceRay(Acc,RayFlags,InstanceInclusionMask,
           RayContributionToHitGroupIndex,
           MultiplierForGeometryContributionToHitGroupIndex,MissShaderIndex, Ray, p);

   return 2.6;
}
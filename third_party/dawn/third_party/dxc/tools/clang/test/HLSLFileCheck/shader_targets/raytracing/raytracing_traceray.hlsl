// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s | FileCheck %s

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


float4 emit(inout float2 f2, RayDesc Ray:R, inout Payload p )  {
  TraceRay(Acc,RayFlags,InstanceInclusionMask,
           RayContributionToHitGroupIndex,
           MultiplierForGeometryContributionToHitGroupIndex,MissShaderIndex, Ray, p);

   return 2.6;
}
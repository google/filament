// RUN: %dxc -T lib_6_3 -auto-binding-space 11 %s | FileCheck %s

//CHECK: User defined type intrinsic arg must be struct

RaytracingAccelerationStructure Acc;

uint RayFlags;
uint InstanceInclusionMask;
uint RayContributionToHitGroupIndex;
uint MultiplierForGeometryContributionToHitGroupIndex;
uint MissShaderIndex;

RayDesc Ray;


float4 emit(inout float2 f2 )  {
  TraceRay(Acc,RayFlags,InstanceInclusionMask,
           RayContributionToHitGroupIndex,
           MultiplierForGeometryContributionToHitGroupIndex,MissShaderIndex , Ray, f2);

   return 2.6;
}
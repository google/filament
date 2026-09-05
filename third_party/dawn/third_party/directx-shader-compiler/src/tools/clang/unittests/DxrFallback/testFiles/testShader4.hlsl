#include "testLib.h"

RaytracingAccelerationStructure accel : register(t0);

struct SomePayload
{
  int val;
};

SHADER_test
void Fallback_TraceRay(
  uint rayFlags,
  uint instanceInclusionMask,
  uint rayContributionToHitGroupIndex,
  uint multiplierForGeometryContributionToHitGroupIndex,
  uint missShaderIndex,
  float originX,
  float originY,
  float originZ,
  float tMin,
  float directionX,
  float directionY,
  float directionZ,
  float tMax,
  uint payloadOffset)
{
  uint oldPayloadOffset = Fallback_TraceRayBegin(rayFlags, float3(originX, originY, originZ), tMin, float3(directionX, directionY, directionZ), tMax, payloadOffset);

  append(originX);
  append(originY);
  append(originZ);
  append(tMin);
  append(directionX);
  append(directionY);
  append(directionZ);
  append(tMax);
  append(rayFlags);
  append(instanceInclusionMask);
  append(rayContributionToHitGroupIndex);
  append(multiplierForGeometryContributionToHitGroupIndex);
  append(missShaderIndex);

  Fallback_TraceRayEnd(oldPayloadOffset);
}

SHADER_test
void full_trace_ray()
{
  RayDesc ray;
  ray.Origin = float3(1, 2, 3);
  ray.TMin = 4;
  ray.Direction = float3(5, 6, 7);
  ray.TMax = 8;
  SomePayload payload = { 10 };
  uint rayFlags = 11;
  uint instanceInclusionMask = 12;
  uint rayContributionToHitGroupIndex = 13;
  uint multiplierForGeometryContributionToHitGroupIndex = 14;
  uint missShaderIndex = 15;

  TraceRay(accel, rayFlags, instanceInclusionMask, rayContributionToHitGroupIndex,
    multiplierForGeometryContributionToHitGroupIndex, missShaderIndex, ray, payload);
}

[numthreads(1, 1, 1)]
void CSMain()
{
  Fallback_Scheduler(initialStateId, 1, 1);
}

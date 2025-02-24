#include "testLib.h"

struct SomePayload
{
  int val;
};

void UndefinedFunction();
void TraceRayTest(RayDesc, uint rayFlags, inout SomePayload);

SHADER_test
void Fallback_TraceRay(RayDesc rd, uint rayFlags, uint payloadOffset)
{
  uint oldPayloadOffset = Fallback_TraceRayBegin(rayFlags, rd.Origin, rd.TMin, rd.Direction, rd.TMax, payloadOffset);

  //UndefinedFunction();
  append(-99);
  append(rd.Origin.x);
  append(rd.Origin.y);
  append(rd.Origin.z);
  append(rd.TMin);
  append(rd.Direction.x);
  append(rd.Direction.y);
  append(rd.Direction.z);
  append(rd.TMax);
  append(rayFlags);

  Fallback_TraceRayEnd(oldPayloadOffset);
}

SHADER_test
void pass_struct()
{
  RayDesc rd;
  rd.Origin = float3(1, 2, 3);
  rd.TMin = 4;
  rd.Direction = float3(5, 6, 7);
  rd.TMax = 8;
  SomePayload payload = { 10 };
  TraceRayTest(rd, 11, payload);
}

[numthreads(1, 1, 1)]
void CSMain()
{
  Fallback_Scheduler(initialStateId, 1, 1);
}

#include "testLib.h"

struct MyPayload
{
  int val;
};

void TraceRayTest(int w, inout MyPayload);
Declare_Fallback_SetPendingAttr(BuiltInTriangleIntersectionAttributes);

SHADER_test
void Fallback_TraceRay(int w, uint payloadOffset)
{
  uint oldPayloadOffset = Fallback_TraceRayBegin(w, float3(w,w,w), w, float3(w,w,w), w, payloadOffset);

  append(-99);
  BuiltInTriangleIntersectionAttributes attr;
  attr.barycentrics = float2(555, 666);
  Fallback_SetPendingAttr(attr);
  Fallback_SetPendingTriVals(w, w, w, w, w, w);
  Fallback_CommitHit();

  Fallback_CallIndirect(consume());

  Fallback_TraceRayEnd(oldPayloadOffset);
}

[shader("raygeneration")]
void raygen()
{
  MyPayload payload = { 1 };
  TraceRayTest(0, payload);
  append(payload.val);
}

[shader("closesthit")]
void ch1(inout MyPayload payload : SV_RayPayload, in BuiltInTriangleIntersectionAttributes attr : SV_IntersectionAttributes)
{
  append(100);
  append(Fallback_RayTCurrent());

  TraceRayTest(1, payload);
  append(Fallback_RayTCurrent());

  TraceRayTest(4, payload);
  append(Fallback_RayTCurrent());

  payload.val += 10;
}

[shader("closesthit")]
void ch2(inout MyPayload payload : SV_RayPayload, in BuiltInTriangleIntersectionAttributes attr : SV_IntersectionAttributes)
{
  append(101);
  append(Fallback_RayTCurrent());
  TraceRayTest(2, payload);
  append(Fallback_RayTCurrent());
  payload.val += 100;
}

[shader("miss")]
void miss1(inout MyPayload payload : SV_RayPayload)
{
  append(102);
  append(Fallback_RayTCurrent());
  TraceRayTest(3, payload);
  append(Fallback_RayTCurrent());
  payload.val += 1000;
}

[shader("miss")]
void miss2(inout MyPayload payload : SV_RayPayload)
{
  append(103);
  append(Fallback_RayTCurrent());
  payload.val += 10000;
}


[numthreads(1, 1, 1)]
void CSMain()
{
  Fallback_Scheduler(initialStateId, 1, 1);
}

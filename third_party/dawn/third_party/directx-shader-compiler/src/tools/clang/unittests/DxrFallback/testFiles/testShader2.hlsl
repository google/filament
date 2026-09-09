#include "testLib.h"

SHADER_test
void indirect()
{
  Fallback_CallIndirect(consume());
}

SHADER_test
void indirect_callee()
{
  append(-99);
}




SHADER_test
void continuation()
{
  append(-99); // mark that we got here
}

SHADER_test
void continuation_in(int val)
{
  append(val); // mark that we got here
}

SHADER_test
void continuation_out64(out int val)
{
  val = 64;
  append(-99); // mark that we got here
}

SHADER_test
void continuation_inout64(inout int val)
{
  append(val); // mark that we got here
  val = 64;
}

void append2(int val)
{
  int slot;
  InterlockedAdd(output[0], 2, slot);
  slot += 1; // to account for the slot counter being at position 0
  output[slot + 0] = val;
  output[slot + 1] = val;
}




void StackDump(int begin, int end)
{
  append(88888888);
  for (int i = begin; i <= end; i++)
    append(Fallback_RuntimeDataLoadInt(i));
  append(88888888);
}

struct MyPayload
{
  int val;
  int depth;
};

struct MyPayload2
{
  int val;
  int val2;
  int depth;
};


struct MyAttributes
{
  int attr0;
  int attr1;
};

//SHADER_test
void TraceRayTest(int which, inout MyPayload);
void TraceRayTest(int which, inout MyPayload2);


[shader("raygeneration")]
void raygen_tri()
{
  MyPayload payload;
  payload.val = 1000;
  payload.depth = 0;

  TraceRayTest(0, payload);

  append(payload.val);
}


[shader("raygeneration")]
void raygen_custom()
{
  MyPayload payload;
  payload.val = 1000;
  payload.depth = 0;
  TraceRayTest(1, payload);
  append(payload.val);

  MyPayload2 payload2;
  payload2.val = payload.val;
  payload2.val2 = 2001;
  payload2.depth = 0;
  TraceRayTest(2, payload2);
  append(payload2.val);
}

[shader("closesthit")]
void chTri(inout MyPayload payload : SV_RayPayload, in BuiltInTriangleIntersectionAttributes attr : SV_IntersectionAttributes)
{
  append(-97);
  append(attr.barycentrics.x);
  append(attr.barycentrics.y);
  continuation();
  payload.val += 10;
}


[shader("closesthit")]
void chCustom1(inout MyPayload payload : SV_RayPayload, in MyAttributes attr : SV_IntersectionAttributes)
{
  append(-96);
  append(attr.attr0);
  append(attr.attr1);
  continuation();
  payload.val += 10;
}

[shader("closesthit")]
void chCustom2(inout MyPayload2 payload2 : SV_RayPayload, in MyAttributes attr : SV_IntersectionAttributes)
{
  append(-96);
  append(attr.attr0);
  append(attr.attr1);
  continuation();
  payload2.val += 100;
}

[shader("miss")]
void miss(inout MyPayload payload : SV_RayPayload)
{
  append(-95);
  payload.val += 1;
}

Declare_Fallback_SetPendingAttr(MyAttributes);

[shader("intersection")][noinline]
void intersection()
{
  append(-95);
  append(RayTCurrent());
  append(Fallback_ShaderRecordOffset());
  append(PrimitiveIndex());
  append(InstanceIndex());
  append(InstanceID());
  MyAttributes attr;
  attr.attr0 = 333;
  attr.attr1 = 444;
  if (ReportHit(12, 77, attr))
    append(500);
  else
    append(600);
}


// Return < 0 for terminate, 0 for ignore, > 0 for accept
int Fallback_ReportHit(float t, uint hitKind)
{
  append(-100);
  continuation();
  Fallback_CommitHit();
  return load(1);
}

Declare_Fallback_SetPendingAttr(BuiltInTriangleIntersectionAttributes);

SHADER_test
void Fallback_TraceRay(int which, uint payloadOffset)
{
  uint oldPayloadOffset = Fallback_TraceRayBegin(0, float3(0, 0, 0), 0, float3(0, 0, 1), 1e34, payloadOffset);
  append(-98);
  if (which == 0)
  {
    BuiltInTriangleIntersectionAttributes attr;
    attr.barycentrics = float2(555, 666);
    Fallback_SetPendingAttr(attr);
    Fallback_CommitHit();
  }
  else if (which == 1)
  {
    // test that we get ending values in intersection, but not for rayTCurrent
    Fallback_SetPendingRayTCurrent(19);
    Fallback_SetPendingCustomVals(20, 21, 22, 23);
    Fallback_CommitHit();
    Fallback_SetPendingRayTCurrent(9);
    Fallback_SetPendingCustomVals(10, 11, 12, 13);
    intersection();
  }
  else if (which == 2)
  {
    // test that we get ending values in intersection, but not for rayTCurrent
    Fallback_SetPendingRayTCurrent(59);
    Fallback_SetPendingCustomVals(60, 61, 62, 63);
    Fallback_CommitHit();
    Fallback_SetPendingRayTCurrent(49);
    Fallback_SetPendingCustomVals(50, 51, 52, 53);

    Fallback_SetInstanceID(63);
    intersection();
  }

  Fallback_CallIndirect(consume());

  Fallback_TraceRayEnd(oldPayloadOffset);
}


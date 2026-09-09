#include "testLib.h"
#define HLSL
#include "testTraversal.h"

struct MyPayload
{
  int val;
  int primIdx;
};

struct MyAttributes
{
  int attr0;
  int attr1;
};

bool Input_LoadHit(out float t, out uint hitKind, out MyAttributes attr)
{
  int val = consume();
  if (val == -1)
    return false;

  t = asfloat(val);
  hitKind = consume();
  attr.attr0 = consume();
  attr.attr1 = consume();
  return true;
}

int Input_LoadAnyHitRet()
{
  return consume();
}

void StackDump(int begin, int end);

RaytracingAccelerationStructure accel : register(t5);

Declare_TraceRayTest(MyPayload);

[shader("raygeneration")]
void raygen()
{
  append(RAYGEN);

  RayDesc ray;
  MyPayload payload;
  payload.val = 1000;
  payload.primIdx = -1;

  //TraceRay(accel,0,0,0,0,0,ray,payload);
  TraceRayTest(0, payload);

  append(payload.val);
  append(payload.primIdx);
}

[shader("anyhit")]
void ahTri(inout MyPayload payload : SV_RayPayload, in BuiltInTriangleIntersectionAttributes attr : SV_IntersectionAttributes)
{
  append(ANYHIT + 0);
  append(attr.barycentrics.x);
  append(attr.barycentrics.y);
  payload.val += 100;

  int anyHitRet = Input_LoadAnyHitRet();
  if (anyHitRet == IGNORE)
    IgnoreHit();

  if (anyHitRet == TERMINATE)
    AcceptHitAndEndSearch();
}

[shader("closesthit")]
void chTri(inout MyPayload payload : SV_RayPayload, in BuiltInTriangleIntersectionAttributes attr : SV_IntersectionAttributes)
{
  append(CLOSESTHIT + 0);
  float2 barycentrics = attr.barycentrics;
  append(barycentrics.x);
  append(barycentrics.y);
  //append(attr.barycentrics.x);
  //append(attr.barycentrics.y);

  payload.val += 10;
  payload.primIdx = PrimitiveIndex();
}

[shader("intersection")]
void intersection()
{
  int print = ENABLE_PRINT;
  append(INTERSECT + 1);

  float t;
  int hitKind;
  MyAttributes attr;
  //int count = 0;
  while (Input_LoadHit(t, hitKind, attr))
  {
    if (print) { append(5000); append(hitKind); append(attr.attr0); append(attr.attr1); append(Fallback_AnyHitStateId()); }
    bool ret = ReportHit(t, hitKind, attr);
    if (print) { append(5001); append(ret); }
    //if(count++ > 5) {append(9999998); return;}
  }
}

[shader("anyhit")]
void ahCustom(inout MyPayload payload : SV_RayPayload, in MyAttributes attr : SV_IntersectionAttributes)
{
  int print = ENABLE_PRINT;
  append(ANYHIT + 1);
  append(attr.attr0);
  append(attr.attr1);
  payload.val += 100;

  int anyHitRet = Input_LoadAnyHitRet();
  if (print) append(anyHitRet);
  if (anyHitRet == IGNORE)
    IgnoreHit();

  if (anyHitRet == TERMINATE)
    AcceptHitAndEndSearch();
}

[shader("closesthit")]
void chCustom(inout MyPayload payload : SV_RayPayload, in MyAttributes attr : SV_IntersectionAttributes)
{
  append(CLOSESTHIT + 1);
  append(attr.attr0);
  append(attr.attr1);

  payload.val += 10;
  payload.primIdx = PrimitiveIndex();
}

[shader("miss")]
void miss(inout MyPayload payload : SV_RayPayload)
{
  append(MISS);

  payload.val += 1;
}


struct PayloadWithArray
{
  int size;
  int vals[10];
};


[shader("raygeneration")]
void raygen_array()
{
  RayDesc ray;
  PayloadWithArray payload;
  payload.size = 10;
  payload.vals[load(3)] = 4;

  //TraceRay(0, payload);
  TraceRay(accel, 0, 0, 0, 0, 0, ray, payload);

  append(payload.vals[2]);
}


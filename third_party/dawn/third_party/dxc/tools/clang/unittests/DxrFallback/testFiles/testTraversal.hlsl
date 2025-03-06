#include "testLib.h"
#define HLSL
#include "testTraversal.h"


void Input_GetLeafRange(out int leafType, out int leafBegin, out int leafEnd)
{
  leafType = consume();
  leafBegin = 0;
  leafEnd = (leafType == LEAF_DONE) ? 0 : 1;
}

void Input_GetInst(out uint instIdx, out uint instId, out uint instFlags)
{
  instIdx = consume();
  instId = consume();
  instFlags = consume();
}

void Input_GetPrimInfo(out uint primIdx, out uint geomIdx, out bool geomOpaque)
{
  int val = consume();
  unpack(val, primIdx, geomIdx, geomOpaque);
}

void Input_IntersectTri(out float t, out float u, out float v, out float d)
{
  t = asfloat(consume());
  u = asfloat(consume());
  v = asfloat(consume());
  d = asfloat(consume());
}

int Input_LoadAnyHit()
{
  return consume();
}

int Input_LoadIntersection()
{
  return consume();
}

int Input_LoadClosestHitOrMiss(bool hit)
{
  int closestHitStateId = consume();
  int missStateId = consume();
  return hit ? closestHitStateId : missStateId;
}

void StackDump(int begin, int end)
{
  append(88888888);
  for (int i = begin; i <= end; i++)
    append(Fallback_RuntimeDataLoadInt(i));
  append(88888888);
}

int InvokeAnyHit(int stateId)
{
  Fallback_SetAnyHitResult(ACCEPT);
  Fallback_CallIndirect(stateId);
  return Fallback_AnyHitResult();
}

int InvokeIntersection(int stateId)
{
  Fallback_SetAnyHitResult(ACCEPT);
  Fallback_CallIndirect(stateId);
  return Fallback_AnyHitResult();
}

// Return < 0 for terminate, 0 for ignore, > 0 for accept
SHADER_internal
int Fallback_ReportHit(float tHit, uint hitKind)
{
  int print = ENABLE_PRINT;
  if (print) { append(4000); append(tHit); append(RayTMin()); append(Fallback_RayTCurrent()); }
  if (tHit < RayTMin() || Fallback_RayTCurrent() <= tHit)
    return 0;

  Fallback_SetPendingRayTCurrent(tHit);
  Fallback_SetPendingHitKind(tHit);
  int stateId = Fallback_AnyHitStateId();
  if (print) { append(4001); append(stateId); }
  int ret = ACCEPT;
  if (stateId > 0)
    ret = InvokeAnyHit(stateId);
  if (ret != IGNORE)
  {
    if (print) append(4002);
    Fallback_CommitHit();
    if (RayFlags() & RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH)
      ret = TERMINATE;
  }
  return ret;
}

Declare_Fallback_SetPendingAttr(BuiltInTriangleIntersectionAttributes);

// Assumptions
// 1 leaf
//
bool Traversal()
{
  uint hgtStartOfs = 0;
  uint hgtStride = 1;
  uint rayOfs = 0;
  uint rayFlags = RayFlags();
  uint instOfs = 0;
  uint instIdx = -1;
  uint instId = 0;
  uint instFlags = 0;
  uint geomStride = 0;

  bool done = false;
  int NO_HIT_SENTINEL = ~0;
  Fallback_SetInstanceIndex(NO_HIT_SENTINEL);
  int count = 0;
  bool print = ENABLE_PRINT;
  if (print) { append(3000); append(Fallback_InstanceIndex()); }
  while (!done)
  {
    // traversal = ray + AS ==> leaf range, done, or continue

    int leafType, leafBegin, leafEnd;
    Input_GetLeafRange(leafType, leafBegin, leafEnd);
    if (print) { append(3010); append(leafType); append(instIdx); }
    if (count++ > 10)
    {
      append(9999999);
      break;
    }

    if (leafBegin < leafEnd) // leaf
    {
      if (print) append(3020);
      // Inputs:
      // Primitive: primIdx
      // Geometry:  geomIdx, opaqueFlag 
      // Instance:  instOfs, instFlags(TRI_CULL_DISABLE, TRI_FRONT_CCW, FORCE_OPAQUE, FORCE_NONOPAQUE)
      // TraceRay:  rayOfs, geomStride, rayFlags(FORCE_OPAQUE, FORCE_NONOPAQUE, TERMINATE_ON_FIRST_HIT, SKIP_CLOSEST_HIT, CULL_BACK_FACING_TRIS, CULL_FRONT_FACING_TRIS, CULL_OPAQUE, CULL_NONOPAQUE)
      // Dispatch:  hgtStartOfs, hgtStride
      //
      // Hit group shader offset:
      //   hgtStartOfs + hgtStride * (rayOfs + instOfs + geomStride * geomIdx)
      //
      // Hit info:
      //   hitGroupRecordOffset, primIdx, instIdx, hitGroupAddr, t, hitKind, t, attributes
      if (leafType == LEAF_INST)
      {
        // Transition to bottom level
        if (print) append(3021);
        Input_GetInst(instIdx, instId, instFlags);
        // object ray = world ray * inverse
      }
      else
      {
        for (int i = leafBegin; i < leafEnd; ++i)
        {
          uint primIdx, geomIdx;
          bool geomOpaque;
          Input_GetPrimInfo(primIdx, geomIdx, geomOpaque);

          bool opaque = isOpaque(geomOpaque, instFlags, rayFlags);
          if (cull(opaque, rayFlags))
            continue;

          uint hitGroupRecordOffset = hgtStartOfs + hgtStride * (rayOfs + instOfs + geomStride * geomIdx);
          if (leafType == LEAF_TRIS) // loop invariant
          {
            float t, u, v, d;
            Input_IntersectTri(t, u, v, d);
            if (instFlags & INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE)
              d = -d;

            float cullFaceDir = computeCullFaceDir(instFlags, rayFlags);
            if (print) { append(3025); append(t); append(RayTMin()); append(Fallback_RayTCurrent()); append(d); append(cullFaceDir); }
            if (t < RayTMin() || t > Fallback_RayTCurrent() || -d * cullFaceDir < 0)
              continue;

            int hitKind = (d > 0) ? HIT_KIND_TRIANGLE_FRONT_FACE : HIT_KIND_TRIANGLE_BACK_FACE;
            Fallback_SetPendingTriVals(hitGroupRecordOffset, primIdx, instIdx, instId, t, hitKind);

            BuiltInTriangleIntersectionAttributes attr;
            attr.barycentrics = float2(u, v);
            Fallback_SetPendingAttr(attr);
            if (opaque)
            {
              if (print) append(3030);
              Fallback_CommitHit();
              done = rayFlags & RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH;
            }
            else
            {
              if (print) append(3040);
              int ahStateId = Input_LoadAnyHit();
              int ret = InvokeAnyHit(ahStateId);
              if (ret != IGNORE)
                Fallback_CommitHit();
              done = (ret == TERMINATE) || (rayFlags & RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH);
            }
          }
          else // (leafType == LEAF_CUSTOM)
          {
            if (print) append(3050);
            Fallback_SetPendingCustomVals(hitGroupRecordOffset, primIdx, instIdx, instId);
            int ahStateId = Input_LoadAnyHit();
            Fallback_SetAnyHitStateId(opaque ? -1 : ahStateId);
            int stateId = Input_LoadIntersection();
            if (print) { append(3051); append(stateId); append(Fallback_AnyHitStateId()); }
            int ret = InvokeIntersection(stateId);
            if (print) { append(3052); append(ret); append(Fallback_InstanceIndex()); }
            done = (ret == TERMINATE) || (rayFlags & RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH);
          }
          if (done)
            break;
        } // for
      } // LEAF_BOTTOM
    }
    else if (leafType == LEAF_DONE)
    {
      if (print) append(3060);
      if (instIdx == -1)
        done = true;
      else
      {
        // transition to top 
        instIdx = -1;
        // object ray = world ray (why?) 
        // done = stack.empty();
      }
    }
  }
  if (print) { append(3070); append(Fallback_InstanceIndex()); }

  return Fallback_InstanceIndex() != NO_HIT_SENTINEL;
}


void Fallback_IgnoreHit()
{
  Fallback_SetAnyHitResult(IGNORE);
}

void Fallback_AcceptHitAndEndSearch()
{
  Fallback_SetAnyHitResult(TERMINATE);
}

SHADER_internal
void Fallback_TraceRay(int param, uint payloadOffset)
{
  append(TRACERAY);

  int rayFlags = consume();
  uint oldPayloadOffset = Fallback_TraceRayBegin(rayFlags, float3(0, 0, 0), 0, float3(0, 0, 1), 1e34, payloadOffset);

  bool hit = Traversal();

  int stateId = 0;
  if (hit && (RayFlags() & RAY_FLAG_SKIP_CLOSEST_HIT_SHADER))
    stateId = 0;
  else
    stateId = Input_LoadClosestHitOrMiss(hit);

  if (stateId)
    Fallback_CallIndirect(stateId);

  Fallback_TraceRayEnd(oldPayloadOffset);
}


[numthreads(1, 1, 1)]
void CSMain()
{
  Fallback_Scheduler(initialStateId, 1, 1);
}




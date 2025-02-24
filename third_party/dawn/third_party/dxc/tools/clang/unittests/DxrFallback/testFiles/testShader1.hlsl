#include "testLib.h"

SHADER_test
void continuation();

SHADER_test
void continuation_in(int val);

SHADER_test
void continuation_out64(out int val);

SHADER_test
void continuation_inout64(inout int val);

SHADER_test
void types()
{
  bool bVal = (load(1) > 0);
  int  ival = load(2);
  uint uval = load(3);
  half hval = load(4);
  float fval = load(5);
  double dval = load(6) + 1e-5;
  int2 ival2 = int2(load(7), load(8));

  continuation();

  verify(bVal ? 1 : 0, 1);
  verify(ival, 2);
  verify(uval, 3);
  verify(hval, 4);
  verify(fval, 5);
  verify((int)dval, 6);
  verify(ival2.x, 7);
  verify(ival2.y, 8);
}

SHADER_test
void no_call()
{
  int val = load(1);
  verify(val, 1);
}

SHADER_test
void no_live_values()
{
  verify(1, 1);
  continuation();
  verify(2, 2);
}

SHADER_test
void single_call()
{
  int val = load(1);
  continuation();
  verify(val, 1);
}

SHADER_test
void multiple_calls()
{
  int val1 = load(1);
  int val2 = load(2);
  continuation();
  verify(val1, 1);

  val1 += load(3); // creates a live alloca
  continuation();  // val2 is live here, but should not be reloaded/saved
  verify(val1, 4);
  verify(val2, 2);
}

SHADER_test
void branch()
{
  int val = load(10);
  if (load(1))
    continuation_out64(val);
  verify(val, 64);
}

SHADER_test
void no_branch()
{
  int val = load(10);
  if (!load(1))
    continuation_out64(val);
  verify(val, 10);
}

SHADER_test
void loop()
{
  int val = load(1);
  continuation();
  verify(val, 1);

  for (int i = 0, n = load(4); i < n; i++)
  {
    continuation();
    val += 1;
  }
  verify(val, 5);
}

SHADER_test
void recursive_rec(int val)
{
  verify(val, val);
  if (val > 0)
    recursive_rec(val - 1);
}

SHADER_test
void recursive()
{
  int val = load(1);
  recursive_rec(load(5));
  verify(val, 1);
}

struct MyStruct
{
  int v1;
  int v2;
};


SHADER_test
void continuation_aggregates(inout MyStruct S, inout uint3 V, inout int A[4])
{
  append(-99);
  append(S.v1);
  append(V.x);
  append(A[0]);
}

SHADER_test
void call_with_aggregates()
{
  MyStruct S;
  S.v1 = load(1);
  S.v2 = load(2);

  uint3 V = uint3(load(3), 0, 0);

  int A[4];
  A[0] = 0;
  A[1] = 1;
  A[2] = load(2);
  A[3] = 3;

  continuation_aggregates(S, V, A);

  append(S.v1);
  append(V.x);
  append(A[2]);
}



SHADER_test
void func_with_args(int arg1, int arg2)
{
  verify(arg1, 1);
  continuation();
  verify(arg1, 1);

  continuation();
  verify(arg2, 2);
}

SHADER_test
void multiple_calls_with_args()
{
  int val = load(3);
  func_with_args(load(1), load(2));
  verify(val, 3);
}

SHADER_test
void single_call_in()
{
  continuation_in(load(10));
}

SHADER_test
void single_call_out()
{
  int val;
  continuation_out64(val);
  verify(val, val);
}


SHADER_test
void single_call_inout()
{
  int val = load(10);
  continuation_inout64(val);
  verify(val, 64);
}



SHADER_test
void continuation_inout_passthru64(inout int val)
{
  append(-98);
  continuation_inout64(val);
}

SHADER_test
void single_call_inout_passthru()
{
  int val = load(10);
  continuation_inout_passthru64(val);
  verify(val, 64);
}


SHADER_test
void use_buffer()
{
  int val = load(10);
  continuation();
  verify(val, load(10));
}


SHADER_test
void lower_intrinsics()
{
  float3 exp_WorldRayOrigin = float3(0, 1, 2);
  Fallback_SetWorldRayOrigin(exp_WorldRayOrigin);

  float3 exp_WorldRayDirection = float3(3, 4, 5);
  Fallback_SetWorldRayDirection(exp_WorldRayDirection);

  float exp_RayTMin = 6;
  Fallback_SetRayTMin(exp_RayTMin);

  float exp_RayTCurrent = 7;
  Fallback_SetRayTCurrent(exp_RayTCurrent);

  uint exp_PrimitiveIndex = 8;
  Fallback_SetPrimitiveIndex(exp_PrimitiveIndex);

  uint exp_InstanceID = 9;
  Fallback_SetInstanceID(exp_InstanceID);

  uint exp_InstanceIndex = 10;
  Fallback_SetInstanceIndex(exp_InstanceIndex);

  float3 exp_ObjectRayOrigin = float3(11, 12, 13);
  Fallback_SetObjectRayOrigin(exp_ObjectRayOrigin);

  float3 exp_ObjectRayDirection = float3(14, 15, 16);
  Fallback_SetObjectRayDirection(exp_ObjectRayDirection);

  row_major float3x4 exp_ObjectToWorld = {
    {17,18,19,20},
    {21,22,23,24},
    {25,26,27,28},
  };
  Fallback_SetObjectToWorld(exp_ObjectToWorld);

  row_major float3x4 exp_WorldToObject = {
    {29,30,31,32},
    {33,34,35,36},
    {37,38,39,40},
  };
  Fallback_SetWorldToObject(exp_WorldToObject);

  uint exp_HitKind = 41;
  Fallback_SetHitKind(exp_HitKind);

  continuation();

  int mismatches = 0;

  float3 worldRayOrigin = WorldRayOrigin();
  mismatches += any(worldRayOrigin != exp_WorldRayOrigin);

  float3 worldRayDirection = WorldRayDirection();
  mismatches += any(worldRayDirection != exp_WorldRayDirection);

  float rayTMin = RayTMin();
  mismatches += (rayTMin != exp_RayTMin);

  float rayTCurrent = RayTCurrent();
  mismatches += (rayTCurrent != exp_RayTCurrent);

  uint primitiveIndex = PrimitiveIndex();
  mismatches += (primitiveIndex != exp_PrimitiveIndex);

  uint instanceID = InstanceID();
  mismatches += (instanceID != exp_InstanceID);

  uint instanceIndex = InstanceIndex();
  mismatches += (instanceIndex != exp_InstanceIndex);

  float3 objectRayOrigin = ObjectRayOrigin();
  mismatches += any(objectRayOrigin != exp_ObjectRayOrigin);

  float3 objectRayDirection = ObjectRayDirection();
  mismatches += any(objectRayDirection != exp_ObjectRayDirection);

  row_major float3x4 objectToWorld = ObjectToWorld();
  mismatches += any(objectToWorld != exp_ObjectToWorld);

  row_major float3x4 worldToObject = WorldToObject();
  mismatches += any(worldToObject != exp_WorldToObject);

  uint hitKind = HitKind();
  mismatches += (hitKind != exp_HitKind);

  verify(mismatches, 0);
}

SHADER_test
void local_array()
{
  int vals[10];
  for (int i = 0; i < 10; ++i)
    vals[i] = i;

  vals[5] = load(5);
  continuation();

  verify(vals[load(4)], 4);
}

[noinline]
void func_with_array_param(inout int val[5])
{
  val[load(2)] = 2;
}

SHADER_test
void array_param()
{
  int val[5];
  continuation();
  func_with_array_param(val);
  append(val[load(2)]);
}

SHADER_test
void array_param2()
{
  row_major float3x4 exp_ObjectToWorld = {
    {17,18,19,20},
    {21,22,23,24},
    {25,26,27,28},
  };
  continuation();
  Fallback_SetObjectToWorld(exp_ObjectToWorld);
}

SHADER_test
void dispatch_idx_and_dims()
{
  uint2 dispatchRaysIndex = DispatchRaysIndex();
  append(dispatchRaysIndex.x);
  append(dispatchRaysIndex.y);

  uint2 dispatchRaysDimensions = DispatchRaysDimensions();
  append(dispatchRaysDimensions.x);
  append(dispatchRaysDimensions.y);
}


[numthreads(1, 1, 1)]
void CSMain()
{
  Fallback_Scheduler(initialStateId, 1, 1);
}

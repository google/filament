// REQUIRES: dxil-1-10

// RUN: not %dxc -E HSMain -T hs_6_0 %s 2>&1 | FileCheck %s
// RUN: not %dxc -E HSMainPassThrough -T hs_6_0 %s 2>&1 | FileCheck %s
// RUN: not %dxc -T lib_6_5 %s 2>&1 | FileCheck %s -check-prefix=LIBCHK

// Test that the proper error for groupshared is produced when compiling in
// patch constant shader functions.

// Neither Hull Shader entry point references the groupshared variable, but both
// patch constant functions do, so we should get errors in both cases.

// CHECK: error: Thread Group Shared Memory not supported from non-compute entry points.
// CHECK: error: Thread Group Shared Memory not supported from non-compute entry points.

// LIBCHK: error: Thread Group Shared Memory not supported from non-compute entry points.
// LIBCHK: error: Thread Group Shared Memory not supported from non-compute entry points.
// LIBCHK: error: Thread Group Shared Memory not supported from non-compute entry points.
// LIBCHK: error: Thread Group Shared Memory not supported from non-compute entry points.

groupshared float2 foo;

RWStructuredBuffer<float4> output;

float getit()
{
  // CSCHK: load float, float addrspace(3)* @[[gsx]]
  // CSCHK: load float, float addrspace(3)* @[[gsy]]
  return foo.x + foo.y;
}

struct PCStruct
{
  float Edges[3]  : SV_TessFactor;
  float Inside : SV_InsideTessFactor;
  float4 test : TEST;
};

struct PosStruct {
  float4 pos : SV_Position;
};

PCStruct HSPatch(InputPatch<PosStruct, 3> ip,
                 OutputPatch<PosStruct, 3> op,
                 uint ix : SV_PrimitiveID)
{
  output[ix] = getit();
  PCStruct a;
  a.Edges[0] = ip[0].pos.w;
  a.Edges[1] = ip[0].pos.w;
  a.Edges[2] = ip[0].pos.w;
  a.Inside = ip[0].pos.w;
  return a;
}

[shader("hull")]
[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("HSPatch")]
PosStruct HSMain(InputPatch<PosStruct, 3> p,
                 uint ix : SV_OutputControlPointID)
{
  PosStruct s;
  s.pos = p[ix].pos;
  return s;
}

PCStruct HSPatch2(InputPatch<PosStruct, 3> ip,
                 OutputPatch<PosStruct, 3> op,
                 uint ix : SV_PrimitiveID)
{
  output[ix] = getit();
  PCStruct a;
  a.Edges[0] = ip[0].pos.w;
  a.Edges[1] = ip[0].pos.w;
  a.Edges[2] = ip[0].pos.w;
  a.Inside = ip[0].pos.w;
  return a;
}

[shader("hull")]
[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("HSPatch2")]
PosStruct HSMainPassThrough(InputPatch<PosStruct, 3> p,
                            uint ix : SV_OutputControlPointID)
{
  // This should create a special pass-through pattern, resulting in a null hull
  // shader function.
  return p[ix];
}

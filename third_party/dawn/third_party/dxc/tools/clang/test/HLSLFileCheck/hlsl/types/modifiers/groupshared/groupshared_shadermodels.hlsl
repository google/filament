// RUN: %dxilver 1.6 | %dxc -E PSMain -T ps_6_0 %s | FileCheck %s
// RUN: %dxilver 1.6 | %dxc -E VSMain -T vs_6_0 %s | FileCheck %s
// RUN: %dxilver 1.6 | %dxc -E GSMain -T gs_6_0 %s | FileCheck %s
// RUN: %dxilver 1.6 | %dxc -E HSMain -T hs_6_0 %s | FileCheck %s
// RUN: %dxilver 1.6 | %dxc -E DSMain -T ds_6_0 %s | FileCheck %s
// RUN: %dxilver 1.6 | %dxc -E CSMain -T lib_6_5 %s | FileCheck %s -check-prefix=LIBCHK
// RUN: %dxilver 1.6 | %dxc -E CSMain -T cs_6_0 %s | FileCheck %s -check-prefix=CSCHK
// RUN: %dxilver 1.6 | %dxc -E MSMain -T ms_6_5 %s | FileCheck %s -check-prefix=CSCHK
// RUN: %dxilver 1.6 | %dxc -E ASMain -T as_6_5 %s | FileCheck %s -check-prefix=CSCHK

// Test that the proper error for groupshared is produced when compiling in non-compute contexts
// and that everything is fine when we are


// CSCHK: @[[gs:.*]] = addrspace(3) global float

// CHECK: error: Thread Group Shared Memory not supported in Shader Model
// CHECK: error: Thread Group Shared Memory not supported in Shader Model
// CHECK: error: Thread Group Shared Memory not supported in Shader Model
// CHECK: error: Thread Group Shared Memory not supported in Shader Model
groupshared float4 foo;

RWStructuredBuffer<float4> output;

int4 getit()
{
  // CSCHK: load float, float addrspace(3)* @[[gs]]
  return foo;
}

// LIBCHK: error: Thread Group Shared Memory not supported from non-compute entry points.
[shader("vertex")]
float4 VSMain(uint ix : SV_VertexID) : OUT {
  output[ix] = getit();
  return 1.0;
}

// LIBCHK: error: Thread Group Shared Memory not supported from non-compute entry points.
[shader("pixel")]
float4 PSMain(uint ix : SV_PrimitiveID) : SV_TARGET {
  output[ix] = getit();
  return 1.0;
}

[shader("compute")]
[NumThreads(32, 32, 1)]
void CSMain(uint ix : SV_GroupIndex) {
  output[ix] = getit();
}

struct payload_t { int nothing; };


[shader("amplification")]
[NumThreads(8, 8, 2)]
void ASMain(uint ix : SV_GroupIndex) {
  output[ix] = getit();
  payload_t p = {0};
  DispatchMesh(1, 1, 1, p);
}

[shader("mesh")]
[NumThreads(8, 8, 2)]
[OutputTopology("triangle")]
void MSMain(uint ix : SV_GroupIndex) {
  output[ix] = getit();
}

struct PosStruct {
  float4 pos : SV_Position;
};

float4 a;

// LIBCHK: error: Thread Group Shared Memory not supported from non-compute entry points.
[shader("geometry")]
[maxvertexcount(1)]
void GSMain(triangle float4 array[3] : SV_Position, uint ix : SV_GSInstanceID,
            inout PointStream<PosStruct> OutputStream)
{
  output[ix] = getit();
  PosStruct s;
  s.pos = a;
  OutputStream.Append(s);
  OutputStream.RestartStrip();
}

struct PCStruct
{
  float Edges[3]  : SV_TessFactor;
  float Inside : SV_InsideTessFactor;
  float4 test : TEST;
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

// LIBCHK: error: Thread Group Shared Memory not supported from non-compute entry points.
[shader("hull")]
[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("HSPatch")]
PosStruct HSMain(InputPatch<PosStruct, 3> p,
                 uint ix : SV_OutputControlPointID)
{
  output[ix] = getit();
  PosStruct s;
  s.pos = p[ix].pos;
  return s;
}

// LIBCHK: error: Thread Group Shared Memory not supported from non-compute entry points.
[shader("domain")]
[domain("tri")]
PosStruct DSMain(const OutputPatch<PosStruct, 3> patch,
                 uint ix : SV_PrimitiveID)
{
  output[ix] = getit();
  PosStruct v;
  v.pos = patch[0].pos;
  return v;
}

struct MyPayload {
  float4 color;
  uint3 pos;
};

struct MyAttributes {
  float2 bary;
  uint id;
};

struct MyParam {
  float2 coord;
  float4 output;
};


RaytracingAccelerationStructure RTAS : register(t5);

// LIBCHK: error: Thread Group Shared Memory not supported from non-compute entry points.
[shader("raygeneration")]
void RGMain()
{
  MyPayload p = (MyPayload)0;
  p.pos = DispatchRaysIndex();
  p.color = getit();
  float3 origin = {0, 0, 0};
  float3 dir = normalize(p.pos / (float)DispatchRaysDimensions());
  RayDesc ray = { origin, 0.125, dir, 128.0};
  TraceRay(RTAS, RAY_FLAG_NONE, 0, 0, 1, 0, ray, p);
}

// LIBCHK: error: Thread Group Shared Memory not supported from non-compute entry points.
[shader("intersection")]
void ISMain()
{
  float hitT = RayTCurrent();
  MyAttributes attr = (MyAttributes)0;
  attr.bary = getit().xy;
  bool bReported = ReportHit(hitT, 0, attr);
}

// LIBCHK: error: Thread Group Shared Memory not supported from non-compute entry points.
[shader("anyhit")]
void AHMain( inout MyPayload payload : SV_RayPayload,
             in MyAttributes attr : SV_IntersectionAttributes )
{
  float3 hitLocation = ObjectRayOrigin() + ObjectRayDirection() * RayTCurrent();
  if (hitLocation.z < attr.bary.x)
    AcceptHitAndEndSearch();         // aborts function
  if (hitLocation.z < attr.bary.y)
    IgnoreHit();   // aborts function
  payload.color += getit();
}

// LIBCHK: error: Thread Group Shared Memory not supported from non-compute entry points.
[shader("closesthit")]
void CHMain( inout MyPayload payload : SV_RayPayload,
             in BuiltInTriangleIntersectionAttributes attr : SV_IntersectionAttributes )
{
  MyParam param = {attr.barycentrics, getit()};
  CallShader(7, param);
  payload.color += param.output;
}


// LIBCHK: error: Thread Group Shared Memory not supported from non-compute entry points.
[shader("miss")]
void MissMain(inout MyPayload payload : SV_RayPayload)
{
  payload.color = getit();
}


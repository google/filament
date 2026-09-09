// RUN: %dxc -E vs -T vs_6_6 %s | FileCheck %s -check-prefixes=CHECKCONST
// RUN: %dxc -E gs -T gs_6_6 %s | FileCheck %s -check-prefixes=CHECKCONST
// RUN: %dxc -E hs -T hs_6_6 %s | FileCheck %s -check-prefixes=CHECKCONST
// RUN: %dxc -E ds -T ds_6_6 %s | FileCheck %s -check-prefixes=CHECKCONST
// RUN: %dxc -E ps -T ps_6_6 %s | FileCheck %s -check-prefixes=CHECK
// RUN: %dxc -E cs -T cs_6_6 %s | FileCheck %s -check-prefixes=CHECKCONST
// RUN: %dxc -E as -T as_6_6 %s | FileCheck %s -check-prefixes=CHECKCONST
// RUN: %dxc -E ms -T ms_6_6 %s | FileCheck %s -check-prefixes=CHECKCONST
// RUN: %dxc -E vs -T vs_6_6 -Od %s | FileCheck %s -check-prefixes=CHECK
// RUN: %dxc -E gs -T gs_6_6 -Od %s | FileCheck %s -check-prefixes=CHECK
// RUN: %dxc -E hs -T hs_6_6 -Od %s | FileCheck %s -check-prefixes=CHECKHS
// RUN: %dxc -E ds -T ds_6_6 -Od %s | FileCheck %s -check-prefixes=CHECK
// RUN: %dxc -E cs -T cs_6_6 -Od %s | FileCheck %s -check-prefixes=CHECK
// RUN: %dxc -E as -T as_6_6 -Od %s | FileCheck %s -check-prefixes=CHECK
// RUN: %dxc -E ms -T ms_6_6 -Od %s | FileCheck %s -check-prefixes=CHECK
// RUN: %dxc -T lib_6_6 %s | FileCheck %s -check-prefixes=CHECKLIB
// RUN: %dxc -T lib_6_6 -fcgl %s | FileCheck %s -check-prefixes=CHECKHLLIB

// RUN: %dxc -E vs -T vs_6_0 %s | FileCheck %s -check-prefixes=CHECKCONST
// RUN: %dxc -E gs -T gs_6_0 %s | FileCheck %s -check-prefixes=CHECKCONST
// RUN: %dxc -E hs -T hs_6_0 %s | FileCheck %s -check-prefixes=CHECKCONST
// RUN: %dxc -E ds -T ds_6_0 %s | FileCheck %s -check-prefixes=CHECKCONST
// RUN: %dxc -E ps -T ps_6_0 %s | FileCheck %s -check-prefixes=CHECKGV
// RUN: %dxc -E cs -T cs_6_0 %s | FileCheck %s -check-prefixes=CHECKCONST
// RUN: %dxc -E as -T as_6_5 %s | FileCheck %s -check-prefixes=CHECKCONST
// RUN: %dxc -E ms -T ms_6_5 %s | FileCheck %s -check-prefixes=CHECKCONST
// RUN: %dxc -E vs -T vs_6_0 -Od %s | FileCheck %s -check-prefixes=CHECKCONST
// RUN: %dxc -E gs -T gs_6_0 -Od %s | FileCheck %s -check-prefixes=CHECKCONST
// RUN: %dxc -E hs -T hs_6_0 -Od %s | FileCheck %s -check-prefixes=CHECKCONST
// RUN: %dxc -E ds -T ds_6_0 -Od %s | FileCheck %s -check-prefixes=CHECKCONST
// RUN: %dxc -E ps -T ps_6_0 -Od %s | FileCheck %s -check-prefixes=CHECKGV
// RUN: %dxc -E cs -T cs_6_0 -Od %s | FileCheck %s -check-prefixes=CHECKCONST
// RUN: %dxc -E as -T as_6_5 -Od %s | FileCheck %s -check-prefixes=CHECKCONST
// RUN: %dxc -E ms -T ms_6_5 -Od %s | FileCheck %s -check-prefixes=CHECKCONST
// RUN: %dxilver 1.6 | %dxc -T lib_6_5 %s | FileCheck %s -check-prefixes=CHECKLIBGV


// Exactly one call
// CHECK define void @{{.*}}()
// CHECK: call i1 @dx.op.isHelperLane.i1(i32 221)
// CHECK-NOT: call i1 @dx.op.isHelperLane.i1(i32 221)


// Exactly two calls for HS and PC func
// CHECKHS define void @{{.*}}()
// CHECKHS: call i1 @dx.op.isHelperLane.i1(i32 221)
// CHECKHS: call i1 @dx.op.isHelperLane.i1(i32 221)
// CHECKHS-NOT: call i1 @dx.op.isHelperLane.i1(i32 221)


// Translated to constant zero, so no call:
// CHECKCONST: define void @{{.*}}()
// CHECKCONST-NOT: call i1 @dx.op.isHelperLane.i1(i32 221)


// No calls simplified for lib target.
// 10 for: vs, gs, hs + pc, ds, cs, as, ms, and exported testfn
// CHECKLIB: call i1 @dx.op.isHelperLane.i1(i32 221)
// CHECKLIB: call i1 @dx.op.isHelperLane.i1(i32 221)
// CHECKLIB: call i1 @dx.op.isHelperLane.i1(i32 221)
// CHECKLIB: call i1 @dx.op.isHelperLane.i1(i32 221)
// CHECKLIB: call i1 @dx.op.isHelperLane.i1(i32 221)
// CHECKLIB: call i1 @dx.op.isHelperLane.i1(i32 221)
// CHECKLIB: call i1 @dx.op.isHelperLane.i1(i32 221)
// CHECKLIB: call i1 @dx.op.isHelperLane.i1(i32 221)
// CHECKLIB: call i1 @dx.op.isHelperLane.i1(i32 221)
// CHECKLIB: call i1 @dx.op.isHelperLane.i1(i32 221)
// CHECKLIB-NOT: call i1 @dx.op.isHelperLane.i1(i32 221)


// One HL call from each function
// 18 functions for HL lib due to entry cloning
// CHECKHLLIB: call i1 @"dx.hl.op.ro.i1 (i32)"(i32 [[id:.*]])
// CHECKHLLIB: call i1 @"dx.hl.op.ro.i1 (i32)"(i32 [[id]])
// CHECKHLLIB: call i1 @"dx.hl.op.ro.i1 (i32)"(i32 [[id]])
// CHECKHLLIB: call i1 @"dx.hl.op.ro.i1 (i32)"(i32 [[id]])
// CHECKHLLIB: call i1 @"dx.hl.op.ro.i1 (i32)"(i32 [[id]])
// CHECKHLLIB: call i1 @"dx.hl.op.ro.i1 (i32)"(i32 [[id]])
// CHECKHLLIB: call i1 @"dx.hl.op.ro.i1 (i32)"(i32 [[id]])
// CHECKHLLIB: call i1 @"dx.hl.op.ro.i1 (i32)"(i32 [[id]])
// CHECKHLLIB: call i1 @"dx.hl.op.ro.i1 (i32)"(i32 [[id]])
// CHECKHLLIB: call i1 @"dx.hl.op.ro.i1 (i32)"(i32 [[id]])
// CHECKHLLIB: call i1 @"dx.hl.op.ro.i1 (i32)"(i32 [[id]])
// CHECKHLLIB: call i1 @"dx.hl.op.ro.i1 (i32)"(i32 [[id]])
// CHECKHLLIB: call i1 @"dx.hl.op.ro.i1 (i32)"(i32 [[id]])
// CHECKHLLIB: call i1 @"dx.hl.op.ro.i1 (i32)"(i32 [[id]])
// CHECKHLLIB: call i1 @"dx.hl.op.ro.i1 (i32)"(i32 [[id]])
// CHECKHLLIB: call i1 @"dx.hl.op.ro.i1 (i32)"(i32 [[id]])
// CHECKHLLIB: call i1 @"dx.hl.op.ro.i1 (i32)"(i32 [[id]])
// CHECKHLLIB: call i1 @"dx.hl.op.ro.i1 (i32)"(i32 [[id]])
// CHECKHLLIB-NOT: call i1 @"dx.hl.op.ro.i1 (i32)"(i32 [[id]])


// CHECKGV:   %[[cov:.*]] = call i32 @dx.op.coverage.i32(i32 91)  ; Coverage()
// CHECKGV:   %[[cmp:.*]] = icmp eq i32 0, %[[cov]]
// CHECKGV:   %[[zext:.*]] = zext i1 %[[cmp]] to i32
// CHECKGV:   store i32 %[[zext]], i32* @dx.ishelper
// CHECKGV:   store i32 1, i32* @dx.ishelper
// CHECKGV-NEXT:   call void @dx.op.discard
// CHECKGV:   %[[load:.*]] = load i32, i32* @dx.ishelper
// CHECKGV:   trunc i32 %[[load]] to i1


// CHECKLIBGV: @dx.ishelper = {{(internal )?}}global i32 0

// CHECKLIBGV-LABEL: define void @cs()
// CHECKLIBGV-NOT: call i32 @dx.op.coverage.i32(i32 91)
// CHECKLIBGV-NOT: store i32 %{{.*}}, i32* @dx.ishelper
// CHECKLIBGV:   %[[load:.*]] = load i32, i32* @dx.ishelper
// CHECKLIBGV:   trunc i32 %[[load]] to i1
// CHECKLIBGV-LABEL: ret void

// CHECKLIBGV-LABEL: define void @as()
// CHECKLIBGV-NOT: call i32 @dx.op.coverage.i32(i32 91)
// CHECKLIBGV-NOT: store i32 %{{.*}}, i32* @dx.ishelper
// CHECKLIBGV:   %[[load:.*]] = load i32, i32* @dx.ishelper
// CHECKLIBGV:   trunc i32 %[[load]] to i1
// CHECKLIBGV-LABEL: ret void

// CHECKLIBGV-LABEL: define <4 x float> @{{.*}}?testfn{{.*}}()
// CHECKLIBGV-NOT: call i32 @dx.op.coverage.i32(i32 91)
// CHECKLIBGV-NOT: store i32 %{{.*}}, i32* @dx.ishelper
// CHECKLIBGV:   %[[load:.*]] = load i32, i32* @dx.ishelper
// CHECKLIBGV:   trunc i32 %[[load]] to i1
// CHECKLIBGV-LABEL: ret <4 x float>

// CHECKLIBGV-LABEL: define void @vs()
// CHECKLIBGV-NOT: call i32 @dx.op.coverage.i32(i32 91)
// CHECKLIBGV-NOT: store i32 %{{.*}}, i32* @dx.ishelper
// CHECKLIBGV:   %[[load:.*]] = load i32, i32* @dx.ishelper
// CHECKLIBGV:   trunc i32 %[[load]] to i1
// CHECKLIBGV-LABEL: ret void

// CHECKLIBGV-LABEL: define void @gs()
// CHECKLIBGV-NOT: call i32 @dx.op.coverage.i32(i32 91)
// CHECKLIBGV-NOT: store i32 %{{.*}}, i32* @dx.ishelper
// CHECKLIBGV:   %[[load:.*]] = load i32, i32* @dx.ishelper
// CHECKLIBGV:   trunc i32 %[[load]] to i1
// CHECKLIBGV-LABEL: ret void

// CHECKLIBGV-LABEL: define void @{{.*}}?pc{{.*}}()
// CHECKLIBGV-NOT: call i32 @dx.op.coverage.i32(i32 91)
// CHECKLIBGV-NOT: store i32 %{{.*}}, i32* @dx.ishelper
// CHECKLIBGV:   %[[load:.*]] = load i32, i32* @dx.ishelper
// CHECKLIBGV:   trunc i32 %[[load]] to i1
// CHECKLIBGV-LABEL: ret void

// CHECKLIBGV-LABEL: define void @hs()
// CHECKLIBGV-NOT: call i32 @dx.op.coverage.i32(i32 91)
// CHECKLIBGV-NOT: store i32 %{{.*}}, i32* @dx.ishelper
// CHECKLIBGV:   %[[load:.*]] = load i32, i32* @dx.ishelper
// CHECKLIBGV:   trunc i32 %[[load]] to i1
// CHECKLIBGV-LABEL: ret void

// CHECKLIBGV-LABEL: define void @ds()
// CHECKLIBGV-NOT: call i32 @dx.op.coverage.i32(i32 91)
// CHECKLIBGV-NOT: store i32 %{{.*}}, i32* @dx.ishelper
// CHECKLIBGV:   %[[load:.*]] = load i32, i32* @dx.ishelper
// CHECKLIBGV:   trunc i32 %[[load]] to i1
// CHECKLIBGV-LABEL: ret void

// CHECKLIBGV-LABEL: define void @ps()
// CHECKLIBGV:   %[[cov:.*]] = call i32 @dx.op.coverage.i32(i32 91)  ; Coverage()
// CHECKLIBGV:   %[[cmp:.*]] = icmp eq i32 0, %[[cov]]
// CHECKLIBGV:   %[[zext:.*]] = zext i1 %[[cmp]] to i32
// CHECKLIBGV:   store i32 %[[zext]], i32* @dx.ishelper
// CHECKLIBGV:   store i32 1, i32* @dx.ishelper
// CHECKLIBGV-NEXT:   call void @dx.op.discard
// CHECKLIBGV:   %[[load:.*]] = load i32, i32* @dx.ishelper
// CHECKLIBGV:   trunc i32 %[[load]] to i1
// CHECKLIBGV-LABEL: ret void

// CHECKLIBGV-LABEL: define void @ms()
// CHECKLIBGV-NOT: call i32 @dx.op.coverage.i32(i32 91)
// CHECKLIBGV-NOT: store i32 %{{.*}}, i32* @dx.ishelper
// CHECKLIBGV:   %[[load:.*]] = load i32, i32* @dx.ishelper
// CHECKLIBGV:   trunc i32 %[[load]] to i1
// CHECKLIBGV-LABEL: ret void

float4 a;

/// Vertex Shader

[shader("vertex")]
float4 vs(): OUT
{
  float4 result = a + IsHelperLane();
  return result;
}

/// Geometry Shader

struct PosStruct {
  float4 pos : SV_Position;
};

[shader("geometry")]
[maxvertexcount(1)]
void gs(triangle float4 array[3] : SV_Position,
        inout PointStream<PosStruct> OutputStream)
{
  float4 result = a + IsHelperLane();
  PosStruct output;
  output.pos = result;
  OutputStream.Append(output);
  OutputStream.RestartStrip();
}

/// Hull Shader and Patch Constant function

struct PCStruct
{
  float Edges[3]  : SV_TessFactor;
  float Inside : SV_InsideTessFactor;
  float4 test : TEST;
};

PCStruct pc(InputPatch<PosStruct, 3> ip,
            OutputPatch<PosStruct, 3> op,
            uint PatchID : SV_PrimitiveID)
{
  float4 result = a + IsHelperLane();
  PCStruct a;
  a.Edges[0] = ip[0].pos.w * result.x;
  a.Edges[1] = ip[0].pos.w * result.y;
  a.Edges[2] = ip[0].pos.w * result.z;
  a.Inside = ip[0].pos.w * result.w;
  return a;
}

[shader("hull")]
[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("pc")]
PosStruct hs(InputPatch<PosStruct, 3> p,
             uint i : SV_OutputControlPointID)
{
  float4 result = a + IsHelperLane();
  PosStruct output;
  output.pos = p[i].pos * result;
  return output;
}

/// Domain Shader

// domain shader that actually outputs the triangle vertices
[shader("domain")]
[domain("tri")]
PosStruct ds(const float3 bary : SV_DomainLocation,
             const OutputPatch<PosStruct, 3> patch)
{
  float4 result = a + IsHelperLane();
  PosStruct v;
  v.pos = patch[0].pos * result;
  return v;
}

/// Pixel Shader

[shader("pixel")]
float4 ps(float f : IN): SV_Target
{
  if (f < 0.0)
    discard;
  float4 result = a + IsHelperLane();
  return ddx(result);
}

/// Compute Shader

RWStructuredBuffer<float4> SB;

[shader("compute")]
[numthreads(14,12,3)]
void cs(uint gidx : SV_GroupIndex)
{
  float4 result = a + IsHelperLane();
  SB[gidx] = result;
}

/// Amplification Shader

groupshared PosStruct pld;

[shader("amplification")]
[numthreads(1, 1, 1)]
void as()
{
  float4 result = a + IsHelperLane();
  pld.pos = result;
  DispatchMesh(1, 1, 1, pld);
}

/// Mesh Shader

[shader("mesh")]
[numthreads(3, 1, 1)]
[outputtopology("triangle")]
void ms(out indices uint3 primIndices[1],
        out vertices PosStruct verts[3],
        in uint tig : SV_GroupIndex)
{
  float4 result = a + IsHelperLane();
  SetMeshOutputCounts(3, 1);
  if (tig == 0)
    primIndices[0] = uint3(0,1,2);
  verts[tig].pos = result;
}

/// Exported function
export
float4 testfn()
{
  float4 result = a + IsHelperLane();
  return result;
}

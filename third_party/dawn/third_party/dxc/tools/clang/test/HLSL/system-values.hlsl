// For use with unit test SystemValueTest.
// Allows for combinations of system values and arbitrary values to be declared
// and used in any signature point and any shader stage just by setting define(s).
// Example:
// fxc system-values.hlsl /E HSMain /T hs_5_1 "/DHSCPIn_Defs=Def_ClipDistance Def_Arb(float,arb,ARB)"

#define GLUE(a,b) a##b
#define Def_Arb_NoSem(t,v) DECLARE(t v) USE(t,v)
#define Def_Arb(t,v,s) DECLARE(t v : s) USE(t,v)
#define Def_ArbAttr(a,t,v,s) DECLARE(a t v : s) USE(t,v)
#define Def_Arb4(t,v,s) DECLARE(GLUE(t,4) v : s) USE(t,v.x) USE(t,v.y) USE(t,v.z) USE(t,v.w)
#define Def_ArbArray2(t,v,s) DECLARE(t v[2] : s) USE(t,v[0]) USE(t,v[1])
#define Def_ArbArray4(t,v,s) DECLARE(t v[4] : s) USE(t,v[0]) USE(t,v[1]) USE(t,v[2]) USE(t,v[3])

#define Def_VertexID DECLARE(uint vid : SV_VertexID) USE(uint, vid)
#define Def_InstanceID DECLARE(uint iid : SV_InstanceID) USE(uint, iid)
#define Def_Position DECLARE(float4 pos : SV_Position) USE(float, pos.x) USE(float, pos.y) USE(float, pos.z) USE(float, pos.w)
#define Def_Coverage DECLARE(uint cov : SV_Coverage) USE(uint, cov)
#define Def_InnerCoverage DECLARE(uint icov : SV_InnerCoverage) USE(uint, icov)
#define Def_PrimitiveID DECLARE(uint pid : SV_PrimitiveID) USE(uint, pid)
#define Def_SampleIndex DECLARE(uint samp : SV_SampleIndex) USE(uint, samp)
#define Def_IsFrontFace DECLARE(bool bff : SV_IsFrontFace) USE(bool, bff)
#define Def_RenderTargetArrayIndex DECLARE(uint rtai : SV_RenderTargetArrayIndex) USE(uint, rtai)
#define Def_ViewportArrayIndex DECLARE(uint vpai : SV_ViewportArrayIndex) USE(uint, vpai)
#define Def_ClipDistance DECLARE(float clipdist : SV_ClipDistance) USE(float, clipdist)
#define Def_CullDistance DECLARE(float culldist : SV_CullDistance) USE(float, culldist)
#define Def_Target DECLARE(float4 target : SV_Target) USE(float, target.x) USE(float, target.y) USE(float, target.z) USE(float, target.w)
#define Def_Depth DECLARE(float depth : SV_Depth) USE(float, depth)
#define Def_DepthLessEqual DECLARE(float depthle : SV_DepthLessEqual) USE(float, depthle)
#define Def_DepthGreaterEqual DECLARE(float depthge : SV_DepthGreaterEqual) USE(float, depthge)
#define Def_StencilRef DECLARE(uint stencilref : SV_StencilRef) USE(uint, stencilref)
#define Def_DispatchThreadID DECLARE(uint3 dtid : SV_DispatchThreadID) USE(uint, dtid.x) USE(uint, dtid.y) USE(uint, dtid.z)
#define Def_GroupID DECLARE(uint3 gid : SV_GroupID) USE(uint, gid.x) USE(uint, gid.y) USE(uint, gid.z)
#define Def_GroupIndex DECLARE(uint gindex : SV_GroupIndex) USE(uint, gindex)
#define Def_GroupThreadID DECLARE(uint3 gtid : SV_GroupThreadID) USE(uint, gtid.x) USE(uint, gtid.y) USE(uint, gtid.z)
#define Def_DomainLocation DECLARE(float2 domainloc : SV_DomainLocation) USE(float, domainloc.x) USE(float, domainloc.y)
#define Def_OutputControlPointID DECLARE(uint ocpid : SV_OutputControlPointID) USE(uint, ocpid)
#define Def_GSInstanceID DECLARE(uint gsiid : SV_GSInstanceID) USE(uint, gsiid)
#define Def_ViewID DECLARE(uint viewID : SV_ViewID) USE(uint, viewID)
#define Def_Barycentrics DECLARE(float3 BaryWeights : SV_Barycentrics) USE(float, BaryWeights.x) USE(float, BaryWeights.y) USE(float, BaryWeights.z)
#define Def_ShadingRate DECLARE(uint rate : SV_ShadingRate) USE(uint, rate)
#define Def_CullPrimitive DECLARE(bool cullprim : SV_CullPrimitive) USE(bool, cullprim)
#define Def_StartVertexLocation DECLARE(int svloc : SV_StartVertexLocation) USE(int, svloc)
#define Def_StartInstanceLocation DECLARE(uint siloc : SV_StartInstanceLocation) USE(uint, siloc)

#define Domain_Quad 0
#define Domain_Tri 1
#define Domain_Isoline 2
#ifndef DOMAIN
  #define DOMAIN Domain_Quad
#endif

#if DOMAIN == Domain_Quad
#define Attribute_Domain [domain("quad")]
#define Def_TessFactor DECLARE(float tfactor[4] : SV_TessFactor) USE(float, tfactor[0]) USE(float, tfactor[1]) USE(float, tfactor[2]) USE(float, tfactor[3])
#define Def_InsideTessFactor DECLARE(float itfactor[2] : SV_InsideTessFactor) USE(float, itfactor[0]) USE(float, itfactor[1])
#endif

#if DOMAIN == Domain_Tri
#define Attribute_Domain [domain("tri")]
#define Def_TessFactor DECLARE(float tfactor[3] : SV_TessFactor) USE(float, tfactor[0]) USE(float, tfactor[1]) USE(float, tfactor[2])
#define Def_InsideTessFactor DECLARE(float itfactor : SV_InsideTessFactor) USE(float, itfactor[0])
#endif

#if DOMAIN == Domain_Isoline
#define Attribute_Domain [domain("isoline")]
#define Def_TessFactor DECLARE(float tfactor[2] : SV_TessFactor) USE(float, tfactor[0]) USE(float, tfactor[1])
#define Def_InsideTessFactor DECLARE(float itfactor : SV_InsideTessFactor) USE(float, itfactor)
#endif


#define DECLARE(decl) decl;
#define USE(t,v)
struct VSIn
{
#ifdef VSIn_Defs
  VSIn_Defs
#endif
};

struct VSOut
{
  float _Arb0 : _Arb0;
#ifdef VSOut_Defs
  VSOut_Defs
#endif
};

struct PCIn
{
#ifdef PCIn_Defs
  PCIn_Defs
#endif
};

struct HSIn
{
#ifdef HSIn_Defs
  HSIn_Defs
#endif
};

struct HSCPIn
{
  float _Arb0 : _Arb0;
#ifdef HSCPIn_Defs
  HSCPIn_Defs
#endif
};

struct HSCPOut
{
  float _Arb0 : _Arb0;
#ifdef HSCPOut_Defs
  HSCPOut_Defs
#endif
};

struct PCOut
{
  float _Arb0 : _Arb0;
  Def_TessFactor
  Def_InsideTessFactor
#ifdef PCOut_Defs
  PCOut_Defs
#endif
};

struct DSIn
{
  Def_TessFactor
  Def_InsideTessFactor
#ifdef DSIn_Defs
  DSIn_Defs
#endif
};

struct DSCPIn
{
#ifdef DSCPIn_Defs
  DSCPIn_Defs
#endif
};

struct DSOut
{
  float _Arb0 : _Arb0;
#ifdef DSOut_Defs
  DSOut_Defs
#endif
};

struct GSVIn
{
  float _Arb0 : _Arb0;
#ifdef GSVIn_Defs
  GSVIn_Defs
#endif
};

struct GSIn
{
#ifdef GSIn_Defs
  GSIn_Defs
#endif
};

struct GSOut
{
  float _Arb0 : _Arb0;
#ifdef GSOut_Defs
  GSOut_Defs
#endif
};

struct PSIn
{
#ifdef PSIn_Defs
  PSIn_Defs
#endif
};

struct PSOut
{
#ifdef PSOut_Defs
  PSOut_Defs
#endif
  float4 out1 : SV_Target1;
};

struct CSIn
{
#ifdef CSIn_Defs
  CSIn_Defs
#endif
};

#undef DECLARE
#undef USE


void VSMain(
#ifdef VSIn_Defs
  in VSIn In, 
#endif
  out VSOut Out)
{
  Out._Arb0 = 0;
#ifdef VSIn_Defs
  #define DECLARE(decl)
  #define USE(t,v) Out._Arb0 += (float)In.v;
    VSIn_Defs
  #undef DECLARE
  #undef USE
#endif

#ifdef VSOut_Defs
  #define DECLARE(decl)
  #define USE(t,v) Out.v = (t)Out._Arb0;
    VSOut_Defs
  #undef DECLARE
  #undef USE
#endif
}

void PCMain(
#ifdef HSCPIn_Defs
  in InputPatch<HSCPIn, 4> InPatch, 
#endif
#ifdef HSCPOut_Defs
  in OutputPatch<HSCPOut, 4> OutPatch, 
#endif
#ifdef PCIn_Defs
  in PCIn In, 
#endif
  out PCOut Out)
{
  Out._Arb0 = 0;
#ifdef HSCPIn_Defs
  #define DECLARE(decl)
  #define USE(t,v) Out._Arb0 += (float)InPatch[1].v;
    HSCPIn_Defs
  #undef DECLARE
  #undef USE
#endif

#ifdef HSCPOut_Defs
  #define DECLARE(decl)
  #define USE(t,v) Out._Arb0 += (float)OutPatch[1].v;
    HSCPOut_Defs
  #undef DECLARE
  #undef USE
#endif

#ifdef PCIn_Defs
  #define DECLARE(decl)
  #define USE(t,v) Out._Arb0 += (float)In.v;
    PCIn_Defs
  #undef DECLARE
  #undef USE
#endif

#define DECLARE(decl)
#define USE(t,v) Out.v = (t)Out._Arb0;
  Def_TessFactor
  Def_InsideTessFactor
  #ifdef PCOut_Defs
    PCOut_Defs
  #endif
#undef DECLARE
#undef USE
}

Attribute_Domain
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("PCMain")]
HSCPOut HSMain(
  in InputPatch<HSCPIn, 4> InPatch
#ifdef HSIn_Defs
  ,in HSIn In
#endif
  )
{
  HSCPOut Out;
  Out._Arb0 = 0;
#ifdef HSCPIn_Defs
  #define DECLARE(decl)
  #define USE(t,v) Out._Arb0 += (float)InPatch[1].v;
    HSCPIn_Defs
  #undef DECLARE
  #undef USE
#endif

#ifdef HSIn_Defs
  #define DECLARE(decl)
  #define USE(t,v) Out._Arb0 += (float)In.v;
    HSIn_Defs
  #undef DECLARE
  #undef USE
#endif

#ifdef HSCPOut_Defs
  #define DECLARE(decl)
  #define USE(t,v) Out.v = (t)Out._Arb0;
    HSCPOut_Defs
  #undef DECLARE
  #undef USE
#endif

  return Out;
}

Attribute_Domain
void DSMain(
#ifdef DSCPIn_Defs
  in OutputPatch<DSCPIn, 4> InPatch,
#endif
  in DSIn In,
  out DSOut Out)
{
  Out._Arb0 = 0;
#ifdef DSCPIn_Defs
  #define DECLARE(decl)
  #define USE(t,v) Out._Arb0 += (float)InPatch[1].v;
    DSCPIn_Defs
  #undef DECLARE
  #undef USE
#endif

#ifdef DSIn_Defs
  #define DECLARE(decl)
  #define USE(t,v) Out._Arb0 += (float)In.v;
    DSIn_Defs
  #undef DECLARE
  #undef USE
#endif

#ifdef DSOut_Defs
  #define DECLARE(decl)
  #define USE(t,v) Out.v = (t)Out._Arb0;
    DSOut_Defs
  #undef DECLARE
  #undef USE
#endif
}

[maxvertexcount(1)]
void GSMain(
  in triangleadj GSVIn VIn[6],
#ifdef GSIn_Defs
//  in GSIn In,
  #define DECLARE(decl) decl,
  #define USE(t,v)
    GSIn_Defs
  #undef DECLARE
  #undef USE
#endif
  inout PointStream<GSOut> Stream)
{
  GSOut Out = (GSOut)0;
#ifdef GSVIn_Defs
  #define DECLARE(decl)
  #define USE(t,v) Out._Arb0 += (float)VIn[1].v;
    GSVIn_Defs
  #undef DECLARE
  #undef USE
#endif

#ifdef GSIn_Defs
  #define DECLARE(decl)
  #define USE(t,v) Out._Arb0 += (float)v;
    GSIn_Defs
  #undef DECLARE
  #undef USE
#endif

#ifdef GSOut_Defs
  #define DECLARE(decl)
  #define USE(t,v) Out.v = (t)Out._Arb0;
    GSOut_Defs
  #undef DECLARE
  #undef USE
#endif
  Stream.Append(Out);
}

void PSMain(
#ifdef PSIn_Defs
  in PSIn In, 
#endif
  out PSOut Out)
{
  Out.out1 = 0;
#ifdef PSIn_Defs
  #define DECLARE(decl)
  #define USE(t,v) Out.out1 += (float4)In.v;
    PSIn_Defs
  #undef DECLARE
  #undef USE
#endif

#ifdef PSOut_Defs
  #define DECLARE(decl)
  #define USE(t,v) Out.v = (t)Out.out1.x;
    PSOut_Defs
  #undef DECLARE
  #undef USE
#endif
}

RWStructuredBuffer<int> Buf;

[numthreads(8,4,2)]
void CSMain(
#ifdef CSIn_Defs
  CSIn In
#endif
  )
{
  int Out = 0;
#ifdef CSIn_Defs
  #define DECLARE(decl)
  #define USE(t,v) Out += (int)In.v;
    CSIn_Defs
  #undef DECLARE
  #undef USE
#endif

#ifdef CSIn_Defs
  #define DECLARE(decl)
  #define USE(t,v) InterlockedAdd(Buf[((uint)In.v) % 256], Out);
    CSIn_Defs
  #undef DECLARE
  #undef USE
#endif
}

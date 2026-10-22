// RUN: %dxc -T lib_6_3 -auto-binding-space 11 %s | FileCheck %s


// Make sure entry function exist.
// CHECK: @cs_main()
// Make sure signatures are lowered.
// CHECK: dx.op.threadId
// CHECK: dx.op.groupId

// Make sure entry function exist.
// CHECK: @gs_main()
// Make sure signatures are lowered.
// CHECK: dx.op.loadInput
// CHECK: dx.op.storeOutput
// CHECK: dx.op.emitStream
// CHECK: dx.op.cutStream

// Make sure entry function exist.
// CHECK: @ds_main()
// Make sure signatures are lowered.
// CHECK: dx.op.loadPatchConstant
// CHECK: dx.op.domainLocation
// CHECK: dx.op.loadInput
// CHECK: dx.op.storeOutput

// Make sure patch constant function exist.
// CHECK: HSPerPatchFunc
// Make sure signatures are lowered.
// CHECK: dx.op.storePatchConstant

// Make sure entry function exist.
// CHECK: @hs_main()
// Make sure signatures are lowered.
// CHECK: dx.op.outputControlPointID
// CHECK: dx.op.loadInput
// CHECK: dx.op.storeOutput

// Make sure entry function exist.
// CHECK: @vs_main()
// Make sure signatures are lowered.
// CHECK: dx.op.loadInput
// Dot4 for clipplane
// CHECK: dx.op.dot4
// CHECK: dx.op.storeOutput

// Make sure entry function exist.
// CHECK: @ps_main()
// Make sure signatures are lowered.
// CHECK: dx.op.loadInput
// CHECK: dx.op.storeOutput
// Finish ps_main
// CHECK: ret void


// Make sure function entrys exist.
// CHECK: !dx.entryPoints = !{{{.*}}, {{.*}}, {{.*}}, {{.*}}, {{.*}}, {{.*}}, {{.*}}}
// Make sure cs doesn't have signature.
// CHECK: !"cs_main", null

void StoreCSOutput(uint2 tid, uint2 gid);

[numthreads(8,8,1)]
void cs_main( uint2 tid : SV_DispatchThreadID, uint2 gid : SV_GroupID, uint2 gtid : SV_GroupThreadID, uint gidx : SV_GroupIndex )
{
    StoreCSOutput(tid, gid);
}

// GS

struct GSOut {
  float2 uv : TEXCOORD0;
  float4 pos : SV_Position;
};

// geometry shader that outputs 3 vertices from a point
[maxvertexcount(3)]
[instance(24)]
void gs_main(InputPatch<GSOut, 2>points, inout PointStream<GSOut> stream) {

  stream.Append(points[0]);

  stream.RestartStrip();
}

// DS
struct PSSceneIn {
  float4 pos : SV_Position;
  float2 tex : TEXCOORD0;
  float3 norm : NORMAL;

uint   RTIndex      : SV_RenderTargetArrayIndex;
};

struct HSPerVertexData {
  // This is just the original vertex verbatim. In many real life cases this would be a
  // control point instead
  PSSceneIn v;
};

struct HSPerPatchData {
  // We at least have to specify tess factors per patch
  // As we're tesselating triangles, there will be 4 tess factors
  // In real life case this might contain face normal, for example
  float edges[3] : SV_TessFactor;
  float inside : SV_InsideTessFactor;
};

// domain shader that actually outputs the triangle vertices
[domain("tri")] PSSceneIn ds_main(const float3 bary
                               : SV_DomainLocation,
                                 const OutputPatch<HSPerVertexData, 3> patch,
                                 const HSPerPatchData perPatchData) {
  PSSceneIn v;

  // Compute interpolated coordinates
  v.pos = patch[0].v.pos * bary.x + patch[1].v.pos * bary.y + patch[2].v.pos * bary.z + perPatchData.edges[1];
  v.tex = patch[0].v.tex * bary.x + patch[1].v.tex * bary.y + patch[2].v.tex * bary.z + perPatchData.edges[0];
  v.norm = patch[0].v.norm * bary.x + patch[1].v.norm * bary.y + patch[2].v.norm * bary.z + perPatchData.inside;
  v.RTIndex = 0;
  return v;
}

// HS

HSPerPatchData HSPerPatchFunc( const InputPatch< PSSceneIn, 3 > points, OutputPatch<HSPerVertexData, 3> outp)
{
    HSPerPatchData d;

    d.edges[ 0 ] = 1;
    d.edges[ 1 ] = 1;
    d.edges[ 2 ] = 1;
    d.inside = 1;

    return d;
}

// hull per-control point shader
[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[patchconstantfunc("HSPerPatchFunc")]
[outputcontrolpoints(3)]
HSPerVertexData hs_main( const uint id : SV_OutputControlPointID,
                               const InputPatch< PSSceneIn, 3 > points)
{
    HSPerVertexData v;

    // Just forward the vertex
    v.v = points[ id ];

	return v;
}

// VS

struct VS_INPUT
{
	float3 vPosition	: POSITION;
	float3 vNormal		: NORMAL;
	float2 vTexcoord	: TEXCOORD0;
};

struct VS_OUTPUT
{
	float3 vNormal		: NORMAL;
	float2 vTexcoord	: TEXCOORD0;
	float4 vPosition	: SV_POSITION;
};

cbuffer VSCB {
float4 plane;
}

[clipplanes(plane)]
VS_OUTPUT vs_main(VS_INPUT Input)
{
	VS_OUTPUT Output;

	Output.vPosition = float4( Input.vPosition, 1.0 );
	Output.vNormal = Input.vNormal;
	Output.vTexcoord = Input.vTexcoord;

       return Output;
}

// PS
[earlydepthstencil]
float4 ps_main(float4 a : A) : SV_TARGET
{
  return a;
}

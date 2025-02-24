// RUN: %dxc -E main -T hs_6_0  %s 2>&1 | FileCheck -input-file=stderr %s

// Same as SimpleHS11.hlsl, except that we only verify StdErr for the warning
// message.

// CHECK: Multiple overloads of patchconstantfunc HSPerPatchFunc.
// CHECK: 87:16: note: This overload was selected

//--------------------------------------------------------------------------------------
// SimpleTessellation.hlsl
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------


struct PSSceneIn
{
    float4 pos     : SV_Position;
    float2 tex     : TEXCOORD0;
    float3 norm    : NORMAL;
    uint   RTIndex : SV_RenderTargetArrayIndex;
};


//////////////////////////////////////////////////////////////////////////////////////////
// Simple forwarding Tessellation shaders

struct HSPerVertexData
{
    // This is just the original vertex verbatim. In many real life cases this would be a
    // control point instead
    PSSceneIn v;
};

struct HSPerPatchData
{
    // We at least have to specify tess factors per patch
    // As we're tesselating triangles, there will be 4 tess factors
    // In real life case this might contain face normal, for example
	float	edges[3] : SV_TessFactor;
	float	inside   : SV_InsideTessFactor;
};

// This function has the same name as the patch constant function, but
// its signature prevents it from being a candidate.
float4 HSPerPatchFunc()
{
    return 1.8;
}

// This overload is a patch constant function candidate because it has an
// output with the SV_TessFactor semantic. However, the compiler should
// *not* select it because there is another overload defined later in this
// translation unit (which is the old compiler's behavior). If it did, then
// the semantic checker will report an error due to this overload's input
// having 32 elements (versus the expected 3).
HSPerPatchData HSPerPatchFunc(const InputPatch< PSSceneIn, 32 > points)
{
  HSPerPatchData d;

  d.edges[0] = -5;
  d.edges[1] = -6;
  d.edges[2] = -7;
  d.inside = -8;

  return d;
}

// hull per-control point shader
[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[patchconstantfunc("HSPerPatchFunc")]
[outputcontrolpoints(3)]
HSPerVertexData main( const uint id : SV_OutputControlPointID,
                      const InputPatch< PSSceneIn, 3 > points )
{
    HSPerVertexData v;

    // Just forward the vertex
    v.v = points[ id ];

	return v;
}

HSPerPatchData HSPerPatchFunc(const InputPatch< PSSceneIn, 3 > points)
{
  HSPerPatchData d;

  d.edges[0] = 1;
  d.edges[1] = 2;
  d.edges[2] = 3;
  d.inside = 4;

  return d;
}
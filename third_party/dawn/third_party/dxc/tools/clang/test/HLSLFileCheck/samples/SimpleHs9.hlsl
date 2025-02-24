// RUN: %dxc -E main -T hs_6_0  %s | FileCheck %s

// CHECK: SV_TessFactor 0
// CHECK: SV_InsideTessFactor 0

// CHECK: define void @main

// CHECK: define void {{.*}}HSPerPatchFunc
// CHECK: dx.op.storePatchConstant.f32{{.*}}float 1.0
// CHECK: dx.op.storePatchConstant.f32{{.*}}float 2.0
// CHECK: dx.op.storePatchConstant.f32{{.*}}float 3.0
// CHECK: dx.op.storePatchConstant.f32{{.*}}float 4.0

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

float4 HSPerPatchFunc()
{
    return 1.8;
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
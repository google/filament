// RUN: %dxc -E main -T hs_6_0  %s | %opt -S -hlsl-dxil-preserve-all-outputs | %FileCheck %s

// CHECK: alloca [3 x float]
// CHECK: alloca float
// CHECK: alloca [8 x float]

// CHECK-NOT: storePatchConstant.f32(i32 106, i32 2, i32 %i

// CHECK: storePatchConstant.f32(i32 106, i32 0, i32 0, i8 0
// CHECK: storePatchConstant.f32(i32 106, i32 0, i32 1, i8 0
// CHECK: storePatchConstant.f32(i32 106, i32 0, i32 2, i8 0

// CHECK: storePatchConstant.f32(i32 106, i32 1, i32 0, i8 0

// CHECK: storePatchConstant.f32(i32 106, i32 2, i32 0, i8 0
// CHECK: storePatchConstant.f32(i32 106, i32 2, i32 1, i8 0
// CHECK: storePatchConstant.f32(i32 106, i32 2, i32 2, i8 0
// CHECK: storePatchConstant.f32(i32 106, i32 2, i32 3, i8 0
// CHECK: storePatchConstant.f32(i32 106, i32 2, i32 4, i8 0
// CHECK: storePatchConstant.f32(i32 106, i32 2, i32 5, i8 0
// CHECK: storePatchConstant.f32(i32 106, i32 2, i32 6, i8 0
// CHECK: storePatchConstant.f32(i32 106, i32 2, i32 7, i8 0


//--------------------------------------------------------------------------------------
// SimpleTessellation.hlsl
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

struct PSSceneIn
{
    float4 pos  : SV_Position;
    float2 tex  : TEXCOORD0;
    float3 norm : NORMAL;

uint   RTIndex      : SV_RenderTargetArrayIndex;
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
	float	edges[ 3 ]	: SV_TessFactor;
	float	inside		: SV_InsideTessFactor;
        float   custom[8]       : CCC;
};

int  count;
float4 c[16];

HSPerPatchData HSPerPatchFunc( const InputPatch< PSSceneIn, 3 > points )
{
    HSPerPatchData d;

    d.edges[ 0 ] = 1;
    d.edges[ 1 ] = 1;
    d.edges[ 2 ] = 1;
    d.inside = 1;
    for (uint i=0;i<count;i++) {
        d.custom[i] = c[i].x;
    }    
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


// RUN: %dxc -E main -T hs_6_0 %s | FileCheck %s

// CHECK: SV_TessFactor            0   w           0  LINEDEN   float   w
// CHECK: SV_TessFactor            1   w           1  LINEDET   float   w

// CHECK: loadInput
// CHECK: loadOutputControlPoint
// CHECK: storePatchConstant
// CHECK: outputControlPointID

//--------------------------------------------------------------------------------------
// SimpleTessellation.hlsl
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

struct VSSceneIn
{
    float3 pos    : POSITION;            
    float3 norm   : NORMAL;            
    float2 tex    : TEXCOORD0;            
};

struct PSSceneIn
{
    float4 pos  : SV_Position;
    float2 tex  : TEXCOORD0;
    float3 norm : NORMAL;
};

cbuffer cb0
{
    float4x4    g_mWorldViewProj;
};

Texture2D        g_txDiffuse : register( t0 );

SamplerState    g_sampler : register( s0 );


//////////////////////////////////////////////////////////////////////////////////////////
// Regular VS/PS rendering

PSSceneIn VSSceneMain( VSSceneIn input )
{
    PSSceneIn output;
    
    output.pos = mul( float4( input.pos, 1.0 ), g_mWorldViewProj );
    output.tex = input.tex;
    output.norm = input.norm;
    
    return output;
}

float4 PSSceneMain( PSSceneIn input ) : SV_Target
{    
    return g_txDiffuse.Sample( g_sampler, input.tex );
}



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
	float	edges[2]	: SV_TessFactor;
};

float4 HSPerPatchFunc()
{
    return 1.8;
}

HSPerPatchData HSPerPatchFunc( const InputPatch< PSSceneIn, 2 > points, const OutputPatch< HSPerVertexData, 2 > opoints  )
{
    HSPerPatchData d;

    d.edges[0] = points[0].tex.x;    
    d.edges[1] = opoints[1].v.tex.x;

    return d;
}

// hull per-control point shader
[domain("isoline")]
[partitioning("fractional_odd")]
[outputtopology("line")]
[patchconstantfunc("HSPerPatchFunc")]
[outputcontrolpoints(2)]
HSPerVertexData main( const uint id : SV_OutputControlPointID,
                               const InputPatch< PSSceneIn, 2 > points )
{
    HSPerVertexData v;

    // Just forward the vertex
    v.v = points[ id ];

	return v;
}



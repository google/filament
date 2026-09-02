// RUN: %dxc -E main -T gs_6_0 %s | FileCheck %s

// CHECK: storeOutput
// CHECK: emitStream
// CHECK: storeOutput
// CHECK: emitStream
// CHECK: storeOutput
// CHECK: emitStream
// CHECK: storeOutput
// CHECK: emitStream
// CHECK: cutStream

//--------------------------------------------------------------------------------------
// File: Particle.hlsl
//
// HLSL file containing shader function to render front-facing particles.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "shader_include.hlsli"

//--------------------------------------------------------------------------------------
// Internal defines
//--------------------------------------------------------------------------------------
#define FIXED_VERTEX_RADIUS 5.0

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct VS_PARTICLE_INPUT
{
    float3 WSPos : POSITION;
};

struct GS_PARTICLE_INPUT
{
    float4 WSPos : POSITION;
};

struct PS_PARTICLE_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};


//--------------------------------------------------------------------------------------
// Geometry Shader to render point sprites
//--------------------------------------------------------------------------------------
[maxvertexcount(4)]
void main(point GS_PARTICLE_INPUT input[1], inout TriangleStream<PS_PARTICLE_INPUT> SpriteStream)
{
    const float3 g_positions[4] =
    {
        float3( -1.0,  1.0, 0.0 ),
        float3(  1.0,  1.0, 0.0 ),
        float3( -1.0, -1.0, 0.0 ),
        float3(  1.0, -1.0, 0.0 ),
    };
    const float2 g_texcoords[4] = 
    { 
        float2( 0.0, 1.0 ), 
        float2( 1.0, 1.0 ),
        float2( 0.0, 0.0 ),
        float2( 1.0, 0.0 ),
    };
    PS_PARTICLE_INPUT output = (PS_PARTICLE_INPUT)0;
    
    // Emit two new triangles
    [unroll]for( int i=0; i<4; ++i )
    {
        float3 position = g_positions[i] * FIXED_VERTEX_RADIUS;
        position = mul( position, (float3x3)g_mInvView ) + input[0].WSPos;
        output.Pos = mul( float4( position, 1.0 ), g_mViewProjection );

        // Pass texture coordinates
        output.Tex = g_texcoords[i];
        
        // Add vertex
        SpriteStream.Append( output );
    }
    SpriteStream.RestartStrip();
}

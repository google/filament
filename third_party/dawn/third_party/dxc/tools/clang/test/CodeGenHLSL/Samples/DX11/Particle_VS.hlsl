// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: loadInput
// CHECK: storeOutput

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
// Vertex Shader to GS
//--------------------------------------------------------------------------------------
GS_PARTICLE_INPUT main( VS_PARTICLE_INPUT input )
{
    GS_PARTICLE_INPUT output = (GS_PARTICLE_INPUT)0;
    
    // Pass world space position to GS
    output.WSPos = float4( input.WSPos, 1.0 );
    
    return output;
}

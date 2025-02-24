// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: sample
// CHECK: discard

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
// Pixel Shader to display constant single color
//--------------------------------------------------------------------------------------
float4 main( PS_PARTICLE_INPUT input ) : SV_TARGET
{
    // Sample particle texture
    float4 vColor = g_baseTexture.Sample( g_samLinear, input.Tex ).wwww;
    
    // Clip fully transparent pixels
    clip( vColor.a - 1.0/255.0 );
    
    // Return color
    return vColor;
}

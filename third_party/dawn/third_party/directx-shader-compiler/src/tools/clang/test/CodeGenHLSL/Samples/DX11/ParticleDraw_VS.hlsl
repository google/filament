// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: bufferLoad
// CHECK: storeOutput

//--------------------------------------------------------------------------------------
// File: ParticleDraw.hlsl
//
// Shaders for rendering the particle as point sprite
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

struct VSParticleIn
{
    float4  color   : COLOR;
    uint    id      : SV_VERTEXID;
};

struct VSParticleDrawOut
{
    float3 pos			: POSITION;
    float4 color		: COLOR;
};

struct GSParticleDrawOut
{
    float2 tex			: TEXCOORD0;
    float4 color		: COLOR;
    float4 pos			: SV_POSITION;
};

struct PSParticleDrawIn
{
    float2 tex			: TEXCOORD0;
    float4 color		: COLOR;
};

struct PosVelo
{
    float4 pos;
    float4 velo;
};

Texture2D		            g_txDiffuse;
StructuredBuffer<PosVelo>   g_bufPosVelo;


SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

cbuffer cb0
{
    row_major float4x4 g_mWorldViewProj;
    row_major float4x4 g_mInvView;    
};

cbuffer cb1
{
    static float g_fParticleRad = 10.0f;   
};

cbuffer cbImmutable
{
    static float3 g_positions[4] =
    {
        float3( -1, 1, 0 ),
        float3( 1, 1, 0 ),
        float3( -1, -1, 0 ),
        float3( 1, -1, 0 ),
    };
    
    static float2 g_texcoords[4] = 
    { 
        float2(0,0), 
        float2(1,0),
        float2(0,1),
        float2(1,1),
    };
};

//
// Vertex shader for drawing the point-sprite particles
//
VSParticleDrawOut main(VSParticleIn input)
{
    VSParticleDrawOut output;
    
    output.pos = g_bufPosVelo[input.id].pos;
    
    float mag = g_bufPosVelo[input.id].velo.w/9;
    output.color = lerp( float4(1,0.1,0.1,1), input.color, mag );
    
    return output;
}



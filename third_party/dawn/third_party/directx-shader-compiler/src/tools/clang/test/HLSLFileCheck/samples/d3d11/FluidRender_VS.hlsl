// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: bufferLoad
// CHECK: Saturate
// CHECK: Round_pi
// CHECK: Round_ni

//--------------------------------------------------------------------------------------
// File: FluidRender.hlsl
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Particle Rendering
//--------------------------------------------------------------------------------------

struct Particle {
    float2 position;
    float2 velocity;
};

struct ParticleDensity {
    float density;
};

StructuredBuffer<Particle> ParticlesRO : register( t0 );
StructuredBuffer<ParticleDensity> ParticleDensityRO : register( t1 );

cbuffer cbRenderConstants : register( b0 )
{
    matrix g_mViewProjection;
    float g_fParticleSize;
};

struct VSParticleOut
{
    float2 position : POSITION;
    float4 color : COLOR;
};

struct GSParticleOut
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};


//--------------------------------------------------------------------------------------
// Visualization Helper
//--------------------------------------------------------------------------------------

static const float4 Rainbow[5] = {
    float4(1, 0, 0, 1), // red
    float4(1, 1, 0, 1), // orange
    float4(0, 1, 0, 1), // green
    float4(0, 1, 1, 1), // teal
    float4(0, 0, 1, 1), // blue
};

float4 VisualizeNumber(float n)
{
    return lerp( Rainbow[ floor(n * 4.0f) ], Rainbow[ ceil(n * 4.0f) ], frac(n * 4.0f) );
}

float4 VisualizeNumber(float n, float lower, float upper)
{
    return VisualizeNumber( saturate( (n - lower) / (upper - lower) ) );
}


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------

VSParticleOut main(uint ID : SV_VertexID)
{
    VSParticleOut Out = (VSParticleOut)0;
    Out.position = ParticlesRO[ID].position;
    Out.color = VisualizeNumber(ParticleDensityRO[ID].density, 1000.0f, 2000.0f);
    return Out;
}

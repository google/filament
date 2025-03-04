// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: threadId
// CHECK: bufferLoad
// CHECK: dot3
// CHECK: FMin
// CHECK: bufferStore

//--------------------------------------------------------------------------------------
// File: FluidCS11.hlsl
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Smoothed Particle Hydrodynamics Algorithm Based Upon:
// Particle-Based Fluid Simulation for Interactive Applications
// Matthias Müller
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Optimized Grid Algorithm Based Upon:
// Broad-Phase Collision Detection with CUDA
// Scott Le Grand
//--------------------------------------------------------------------------------------

struct Particle
{
    float2 position;
    float2 velocity;
};

struct ParticleForces
{
    float2 acceleration;
};

struct ParticleDensity
{
    float density;
};

cbuffer cbSimulationConstants : register( b0 )
{
    uint g_iNumParticles;
    float g_fTimeStep;
    float g_fSmoothlen;
    float g_fPressureStiffness;
    float g_fRestDensity;
    float g_fDensityCoef;
    float g_fGradPressureCoef;
    float g_fLapViscosityCoef;
    float g_fWallStiffness;

    float4 g_vGravity;
    float4 g_vGridDim;
    float3 g_vPlanes[4];
};

//--------------------------------------------------------------------------------------
// Fluid Simulation
//--------------------------------------------------------------------------------------

#define SIMULATION_BLOCK_SIZE 256

//--------------------------------------------------------------------------------------
// Structured Buffers
//--------------------------------------------------------------------------------------
RWStructuredBuffer<Particle> ParticlesRW : register( u0 );
StructuredBuffer<Particle> ParticlesRO : register( t0 );

RWStructuredBuffer<ParticleDensity> ParticlesDensityRW : register( u0 );
StructuredBuffer<ParticleDensity> ParticlesDensityRO : register( t1 );

RWStructuredBuffer<ParticleForces> ParticlesForcesRW : register( u0 );
StructuredBuffer<ParticleForces> ParticlesForcesRO : register( t2 );

RWStructuredBuffer<unsigned int> GridRW : register( u0 );
StructuredBuffer<unsigned int> GridRO : register( t3 );

RWStructuredBuffer<uint2> GridIndicesRW : register( u0 );
StructuredBuffer<uint2> GridIndicesRO : register( t4 );


//--------------------------------------------------------------------------------------
// Integration
//--------------------------------------------------------------------------------------

[numthreads(SIMULATION_BLOCK_SIZE, 1, 1)]
void main( uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex )
{
    const unsigned int P_ID = DTid.x; // Particle ID to operate on
    
    float2 position = ParticlesRO[P_ID].position;
    float2 velocity = ParticlesRO[P_ID].velocity;
    float2 acceleration = ParticlesForcesRO[P_ID].acceleration;
    
    // Apply the forces from the map walls
    [unroll]
    for (unsigned int i = 0 ; i < 4 ; i++)
    {
        float dist = dot(float3(position, 1), g_vPlanes[i]);
        acceleration += min(dist, 0) * -g_fWallStiffness * g_vPlanes[i].xy;
    }
    
    // Apply gravity
    acceleration += g_vGravity.xy;
    
    // Integrate
    velocity += g_fTimeStep * acceleration;
    position += g_fTimeStep * velocity;
    
    // Update
    ParticlesRW[P_ID].position = position;
    ParticlesRW[P_ID].velocity = velocity;
}

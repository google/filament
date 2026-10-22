// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: threadId
// CHECK: bufferLoad
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
// Grid Construction
//--------------------------------------------------------------------------------------

// For simplicity, this sample uses a 16-bit hash based on the grid cell and
// a 16-bit particle ID to keep track of the particles while sorting
// This imposes a limitation of 64K particles and 256x256 grid work
// You could extended the implementation to support large scenarios by using a uint2

float2 GridCalculateCell(float2 position)
{
    return clamp(position * g_vGridDim.xy + g_vGridDim.zw, float2(0, 0), float2(255, 255));
}

unsigned int GridConstuctKey(uint2 xy)
{
    // Bit pack [-----UNUSED-----][----Y---][----X---]
    //                16-bit         8-bit     8-bit
    return dot(xy.yx, uint2(256, 1));
}

unsigned int GridConstuctKeyValuePair(uint2 xy, uint value)
{
    // Bit pack [----Y---][----X---][-----VALUE------]
    //             8-bit     8-bit        16-bit
    return dot(uint3(xy.yx, value), uint3(256*256*256, 256*256, 1));
}

unsigned int GridGetKey(unsigned int keyvaluepair)
{
    return (keyvaluepair >> 16);
}

unsigned int GridGetValue(unsigned int keyvaluepair)
{
    return (keyvaluepair & 0xFFFF);
}


[numthreads(SIMULATION_BLOCK_SIZE, 1, 1)]
void main( uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex )
{
    const unsigned int G_ID = DTid.x; // Grid ID to operate on
    unsigned int G_ID_PREV = (G_ID == 0)? g_iNumParticles : G_ID; G_ID_PREV--;
    unsigned int G_ID_NEXT = G_ID + 1; if (G_ID_NEXT == g_iNumParticles) { G_ID_NEXT = 0; }
    
    unsigned int cell = GridGetKey( GridRO[G_ID] );
    unsigned int cell_prev = GridGetKey( GridRO[G_ID_PREV] );
    unsigned int cell_next = GridGetKey( GridRO[G_ID_NEXT] );
    if (cell != cell_prev)
    {
        // I'm the start of a cell
        GridIndicesRW[cell].x = G_ID;
    }
    if (cell != cell_next)
    {
        // I'm the end of a cell
        GridIndicesRW[cell].y = G_ID + 1;
    }
}

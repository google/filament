// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: threadId
// CHECK: bufferLoad
// CHECK: FMax
// CHECK: FMin
// CHECK: IMax
// CHECK: dot2
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



//--------------------------------------------------------------------------------------
// Density Calculation
//--------------------------------------------------------------------------------------

float CalculateDensity(float r_sq)
{
    const float h_sq = g_fSmoothlen * g_fSmoothlen;
    // Implements this equation:
    // W_poly6(r, h) = 315 / (64 * pi * h^9) * (h^2 - r^2)^3
    // g_fDensityCoef = fParticleMass * 315.0f / (64.0f * PI * fSmoothlen^9)
    return g_fDensityCoef * (h_sq - r_sq) * (h_sq - r_sq) * (h_sq - r_sq);
}




//--------------------------------------------------------------------------------------
// Optimized Grid + Sort Algorithm
//--------------------------------------------------------------------------------------

[numthreads(SIMULATION_BLOCK_SIZE, 1, 1)]
void main( uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex )
{
    const unsigned int P_ID = DTid.x;
    const float h_sq = g_fSmoothlen * g_fSmoothlen;
    float2 P_position = ParticlesRO[P_ID].position;
    
    float density = 0;
    
    // Calculate the density based on neighbors from the 8 adjacent cells + current cell
    int2 G_XY = (int2)GridCalculateCell( P_position );
    for (int Y = max(G_XY.y - 1, 0) ; Y <= min(G_XY.y + 1, 255) ; Y++)
    {
        for (int X = max(G_XY.x - 1, 0) ; X <= min(G_XY.x + 1, 255) ; X++)
        {
            unsigned int G_CELL = GridConstuctKey(uint2(X, Y));
            uint2 G_START_END = GridIndicesRO[G_CELL];
            for (unsigned int N_ID = G_START_END.x ; N_ID < G_START_END.y ; N_ID++)
            {
                float2 N_position = ParticlesRO[N_ID].position;
                
                float2 diff = N_position - P_position;
                float r_sq = dot(diff, diff);
                if (r_sq < h_sq)
                {
                    density += CalculateDensity(r_sq);
                }
            }
        }
    }
    
    ParticlesDensityRW[P_ID].density = density;
}


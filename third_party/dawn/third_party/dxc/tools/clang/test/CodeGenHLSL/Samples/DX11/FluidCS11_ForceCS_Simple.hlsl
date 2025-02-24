// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: threadId
// CHECK: bufferLoad
// CHECK: dot2
// CHECK: Log
// CHECK: Exp
// CHECK: FMax
// CHECK: Sqrt
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
// Force Calculation
//--------------------------------------------------------------------------------------

float CalculatePressure(float density)
{
    // Implements this equation:
    // Pressure = B * ((rho / rho_0)^y  - 1)
    return g_fPressureStiffness * max(pow(density / g_fRestDensity, 3) - 1, 0);
}

float2 CalculateGradPressure(float r, float P_pressure, float N_pressure, float N_density, float2 diff)
{
    const float h = g_fSmoothlen;
    float avg_pressure = 0.5f * (N_pressure + P_pressure);
    // Implements this equation:
    // W_spkiey(r, h) = 15 / (pi * h^6) * (h - r)^3
    // GRAD( W_spikey(r, h) ) = -45 / (pi * h^6) * (h - r)^2
    // g_fGradPressureCoef = fParticleMass * -45.0f / (PI * fSmoothlen^6)
    return g_fGradPressureCoef * avg_pressure / N_density * (h - r) * (h - r) / r * (diff);
}

float2 CalculateLapVelocity(float r, float2 P_velocity, float2 N_velocity, float N_density)
{
    const float h = g_fSmoothlen;
    float2 vel_diff = (N_velocity - P_velocity);
    // Implements this equation:
    // W_viscosity(r, h) = 15 / (2 * pi * h^3) * (-r^3 / (2 * h^3) + r^2 / h^2 + h / (2 * r) - 1)
    // LAPLACIAN( W_viscosity(r, h) ) = 45 / (pi * h^6) * (h - r)
    // g_fLapViscosityCoef = fParticleMass * fViscosity * 45.0f / (PI * fSmoothlen^6)
    return g_fLapViscosityCoef / N_density * (h - r) * vel_diff;
}


//--------------------------------------------------------------------------------------
// Simple N^2 Algorithm
//--------------------------------------------------------------------------------------

[numthreads(SIMULATION_BLOCK_SIZE, 1, 1)]
void main( uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex )
{
    const unsigned int P_ID = DTid.x; // Particle ID to operate on
    
    float2 P_position = ParticlesRO[P_ID].position;
    float2 P_velocity = ParticlesRO[P_ID].velocity;
    float P_density = ParticlesDensityRO[P_ID].density;
    float P_pressure = CalculatePressure(P_density);
    
    const float h_sq = g_fSmoothlen * g_fSmoothlen;
    
    float2 acceleration = float2(0, 0);

    // Calculate the acceleration based on all neighbors
    for (uint N_ID = 0 ; N_ID < g_iNumParticles ; N_ID++)
    {
        float2 N_position = ParticlesRO[N_ID].position;
        
        float2 diff = N_position - P_position;
        float r_sq = dot(diff, diff);
        if (r_sq < h_sq && P_ID != N_ID)
        {
            float2 N_velocity = ParticlesRO[N_ID].velocity;
            float N_density = ParticlesDensityRO[N_ID].density;
            float N_pressure = CalculatePressure(N_density);
            float r = sqrt(r_sq);

            // Pressure Term
            acceleration += CalculateGradPressure(r, P_pressure, N_pressure, N_density, diff);
            
            // Viscosity Term
            acceleration += CalculateLapVelocity(r, P_velocity, N_velocity, N_density);
        }
    }
    
    ParticlesForcesRW[P_ID].acceleration = acceleration / P_density;
}


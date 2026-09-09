// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: loadInput
// CHECK: storeOutput

//--------------------------------------------------------------------------------------
// File: DecalTessellation.hlsl
//
// HLSL file containing shader functions for decal tessellation.
//
// Contributed by the AMD Developer Relations Team
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "AdaptiveTessellation.hlsli"

#define MAX_DECALS 50
#define MIN_PRIM_SIZE 16
#define BACKFACE_EPSILON 0.25
//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------
Texture2D g_DisplacementMap : register( t0 );        // Displacement map for the rendered object
Texture2D g_NormalMap : register( t1 );              // Normal map for the rendered object
Texture2D g_BaseMap : register( t2 );                // Base color map

//--------------------------------------------------------------------------------------
// Samplers
//--------------------------------------------------------------------------------------
SamplerState g_sampleLinear : register( s0 );

//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
cbuffer cbInit : register( b0 )
{
    float4  g_vMaterialColor;            // mesh color
    float4  g_vAmbientColor;             // mesh ambient color
    float4  g_vSpecularColor;            // mesh specular color
    float4  g_vScreenSize;               // x = screen width, y = screen height
    float4  g_vFlags;                    // miscellaneous flags
};

cbuffer cbUpdate : register( b1 )
{
    matrix    g_mWorld;                    // World matrix
    matrix    g_mViewProjection;           // ViewProjection matrix
    matrix    g_mWorldViewProjection;      // WVP matrix
    float4    g_vTessellationFactor;       // x = tessellation factor, z = backface culling, w = adaptive
    float4    g_vDisplacementScaleBias;    // Scale and bias of displacement
    float4    g_vLightPosition;            // 3D light position
    float4    g_vEyePosition;              // 3D world space eye position
};

cbuffer cbDamage : register( b2 )
{
    float4  g_vNormal[MAX_DECALS];              // tangent space normal
    float4  g_vBinormal[MAX_DECALS];            // tangent space binormal
    float4  g_vTangent[MAX_DECALS];             // tangent space tangent
    float4  g_vDecalPositionSize[MAX_DECALS];   // position and size of this decal
}                                                 

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------

// Tessellation vertex shader input
struct VS_INPUT
{
    float3 vPos            : POSITION;
    float3 vNormal        : NORMAL0;
    float2 vTexCoord    : TEXCOORD0;
};

// Tessellation vertex shader output, hull shader input
struct VS_OUTPUT_HS_INPUT
{
    float3 vPosWS        : POSITION;
    float2 vTexCoord    : TEXCOORD0;
    float3 vNormal        : NORMAL0;
};

// Patch constant hull shader output
struct HS_CONSTANT_DATA_OUTPUT
{
    float Edges[3]        : SV_TessFactor;
    float Inside        : SV_InsideTessFactor;
};

// Control point hull shader output
struct HS_CONTROL_POINT_OUTPUT
{
    float3 vWorldPos    : WORLDPOS;
    float2 vTexCoord    : TEXCOORD0;
    float3 vNormal        : NORMAL0;
};

// Domain shader (or no-tessellation vertex shader) output, pixel shader input
struct DS_VS_OUTPUT_PS_INPUT
{
    float4 vPosCS        : SV_POSITION;
    float2 vTexCoord    : TEXCOORD0;
    float3 vNormal        : NORMAL0;
    float3 vNMTexCoord    : TEXCOORD1;
    float3 vLightTS        : LIGHTVECTORTS;
    float3 vLightWS        : LIGHTVECTORWS;    // world space light vector for non-normal mapped pixels
    float3 vViewTS        : VIEWVECTORTS;
    float3 vViewWS        : VIEWVECTORWS;
};

//--------------------------------------------------------------------------------------
// Vertex shader: No Tessellation
//--------------------------------------------------------------------------------------
DS_VS_OUTPUT_PS_INPUT main( VS_INPUT i )
{
    DS_VS_OUTPUT_PS_INPUT Out;
    
    // Propagate the texture coordinate
    Out.vTexCoord = i.vTexCoord;
    Out.vNMTexCoord = float3(0,0,0);
    
    // propagate the vertex normal
    Out.vNormal = i.vNormal;
 
    // Create the light vector
    float4 vPosWS = mul( float4(i.vPos, 1.0), g_mWorld );
    Out.vLightTS = g_vLightPosition.xyz - vPosWS.xyz;
    Out.vLightWS = Out.vLightTS;
    Out.vViewTS = g_vEyePosition.xyz - vPosWS.xyz;
    Out.vViewWS = Out.vViewTS;

    // Transform position to clip space:
    Out.vPosCS = mul( float4(i.vPos, 1.0), g_mWorldViewProjection );
    
    return Out;
}   

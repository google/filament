// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: sample
// CHECK: Rsqrt
// CHECK: sample
// CHECK: dot3
// CHECK: Saturate

//--------------------------------------------------------------------------------------
// File: DetailTessellation.hlsl
//
// HLSL file containing shader functions for detail tessellation.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "shader_include.hlsli"
#include "AdaptiveTessellation.hlsli"

//--------------------------------------------------------------------------------------
// External defines
//--------------------------------------------------------------------------------------
//#define DENSITY_BASED_TESSELLATION 0
//#define DISTANCE_ADAPTIVE_TESSELLATION 0
//#define SCREEN_SPACE_ADAPTIVE_TESSELLATION 0

//--------------------------------------------------------------------------------------
// Internal defines
//--------------------------------------------------------------------------------------
#define PERPIXEL_DIFFUSE_LIGHTING 1
#define DEBUG_VIEW 0

//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------
Texture2D g_DensityTexture : register( t2 );      // Height map density (only used for debug view)
                        
//--------------------------------------------------------------------------------------
// Buffer
//--------------------------------------------------------------------------------------
Buffer<float4> g_DensityBuffer : register( t0 );  // Density vertex buffer

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float3 inPositionOS   : POSITION;
    float2 inTexCoord     : TEXCOORD0;
    float3 vInNormalOS    : NORMAL;
    float3 vInBinormalOS  : BINORMAL;
    float3 vInTangentOS   : TANGENT;
    
    uint   uVertexID      : SV_VERTEXID;
};

struct VS_OUTPUT_NO_TESSELLATION
{
    float2 texCoord  : TEXCOORD0;
    
#if PERPIXEL_DIFFUSE_LIGHTING==1    
    float3 vLightTS  : LIGHTVECTORTS;

#if ADD_SPECULAR==1
    float3 vViewTS   : VIEWVECTORTS;
#endif

#endif

#if PERPIXEL_DIFFUSE_LIGHTING==0 || DEBUG_VIEW>0
    float3 vDiffuseColor : COLOR;
#endif

    float4 vPosition : SV_POSITION;
};

struct VS_OUTPUT_HS_INPUT
{
    float3 vWorldPos : WORLDPOS;
    float3 vNormal   : NORMAL;
            
#if DISTANCE_ADAPTIVE_TESSELLATION==1
    float  fVertexDistanceFactor : VERTEXDISTANCEFACTOR;
#endif

    float2 texCoord  : TEXCOORD0;
    float3 vLightTS  : LIGHTVECTORTS;
    
#if ADD_SPECULAR==1
    float3 vViewTS   : VIEWVECTORTS;
#endif
};

struct HS_CONSTANT_DATA_OUTPUT
{
    float    Edges[3]         : SV_TessFactor;
    float    Inside           : SV_InsideTessFactor;
    
#if DENSITY_BASED_TESSELLATION==1 && DEBUG_VIEW>0
    float    VertexDensity[3] : VERTEX_DENSITY;
#endif
};

struct HS_CONTROL_POINT_OUTPUT
{
    float3 vWorldPos : WORLDPOS;
    float3 vNormal   : NORMAL;
    float2 texCoord  : TEXCOORD0;
    float3 vLightTS  : LIGHTVECTORTS;
#if ADD_SPECULAR==1
    float3 vViewTS   : VIEWVECTORTS; 
#endif
};


struct DS_OUTPUT
{
    float2 texCoord          : TEXCOORD0;

#if PERPIXEL_DIFFUSE_LIGHTING==1    
    float3 vLightTS          : LIGHTVECTORTS;

#if ADD_SPECULAR==1
    float3 vViewTS           : VIEWVECTORTS;
#endif

#endif

#if PERPIXEL_DIFFUSE_LIGHTING==0 || DEBUG_VIEW>0
    float3 vDiffuseColor     : COLOR;
#endif

    float4 vPosition         : SV_POSITION;
};

struct PS_INPUT
{
   float2 texCoord           : TEXCOORD0;

#if PERPIXEL_DIFFUSE_LIGHTING==1    
   float3 vLightTS           : LIGHTVECTORTS;

#if ADD_SPECULAR==1
   float3 vViewTS            : VIEWVECTORTS;
#endif

#endif

#if PERPIXEL_DIFFUSE_LIGHTING==0 || DEBUG_VIEW>0
    float3 vDiffuseColor     : COLOR;
#endif
};




//--------------------------------------------------------------------------------------
// Bump mapping shader
//--------------------------------------------------------------------------------------
float4 main( PS_INPUT i ) : SV_TARGET
{ 
    float4 cResultColor = float4( 0, 0, 0, 1 );
    float3 vViewTS = float3( 0, 0, 0 );
    
#if PERPIXEL_DIFFUSE_LIGHTING==1

    // Normalize tangent space light vector
    float3 vLightTS = normalize( i.vLightTS );

#if ADD_SPECULAR==1
    // Normalize tangent space view vector
    vViewTS = normalize( i.vViewTS );
#endif

    // Compute resulting color for the pixel:
    cResultColor = ComputeIllumination( i.texCoord, vLightTS, vViewTS );
   
#else

    // Sample base map:
    float4 cBaseColor = g_baseTexture.Sample( g_samLinear, i.texCoord );
    
    // Composite the final color:
    float3 cFinalColor = ( g_materialAmbientColor + i.vDiffuseColor ) * cBaseColor; 

    // Per tessellated vertex diffuse lighting: simply interpolate diffuse color at each tessellated vertex
    cResultColor = float4( cFinalColor, 1.0 );
    
#endif

#if DENSITY_BASED_TESSELLATION==1 && DEBUG_VIEW==1
    float fDensity = g_DensityTexture.SampleLevel( g_samLinear, i.texCoord, 0 ).x;
    cResultColor.xyz = cResultColor.xyz*g_vDebugColorMultiply.xyz + fDensity*g_vDebugColorAdd.xyz;
#endif

#if DEBUG_VIEW==2
    cResultColor.xyz = cResultColor.xyz*g_vDebugColorMultiply.xyz + i.vDiffuseColor.xyz*g_vDebugColorAdd.xyz;
#endif
   
   // Return color
   return cResultColor;
}   



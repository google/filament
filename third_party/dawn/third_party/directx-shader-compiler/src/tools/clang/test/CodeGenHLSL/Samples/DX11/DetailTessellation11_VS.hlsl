// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: Rsqrt
// CHECK: storeOutput

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
// Vertex shader: no tessellation
//--------------------------------------------------------------------------------------
VS_OUTPUT_NO_TESSELLATION main( VS_INPUT i )
{
    VS_OUTPUT_NO_TESSELLATION Out;
        
    // Compute position in world space
    float4 vPositionWS = mul( i.inPositionOS.xyz, g_mWorld );
                 
    // Compute denormalized light vector in world space
    float3 vLightWS = g_LightPosition.xyz - vPositionWS.xyz;
    // Need to invert Z for correct lighting
    vLightWS.z = -vLightWS.z;
    
    // Propagate texture coordinate through:
    Out.texCoord = i.inTexCoord * g_fBaseTextureRepeat.x;

    // Transform normal, tangent and binormal vectors from object 
    // space to homogeneous projection space and normalize them
    float3 vNormalWS   = mul( i.vInNormalOS,   (float3x3) g_mWorld );
    float3 vBinormalWS = mul( i.vInBinormalOS, (float3x3) g_mWorld );
    float3 vTangentWS  = mul( i.vInTangentOS,  (float3x3) g_mWorld );
    
    // Normalize tangent space vectors
    vNormalWS   = normalize( vNormalWS );
    vBinormalWS = normalize( vBinormalWS );
    vTangentWS  = normalize( vTangentWS );
    
    // Calculate tangent basis
    float3x3 mWorldToTangent = float3x3( vTangentWS, vBinormalWS, vNormalWS );
    
    // Calculate tangent space light vector
    float3 vLightTS = mul( mWorldToTangent, vLightWS.xyz );
    
#if PERPIXEL_DIFFUSE_LIGHTING==1
    // Per-pixel lighting
    
    // Propagate the light vector (in tangent space)
    Out.vLightTS = vLightTS;
    
#if ADD_SPECULAR==1
    // Compute and output the world view vector (unnormalized)
    float3 vViewWS = g_vEye - vPositionWS;
    Out.vViewTS  = mul( mWorldToTangent, vViewWS  );
#endif

#else
    // Per-vertex lighting

    // Calculate MIP level to fetch from
    float fHeightMapMIPLevel = clamp( ( distance( vPositionWS.xyz, g_vEye ) - 100.0f ) / 100.0f, 0.0f, 6.0f );

    // Fetch normal from normal map
    float4 vNormalHeight = g_nmhTexture.SampleLevel( g_samLinear, Out.texCoord, fHeightMapMIPLevel );
    
    // Calculate diffuse lighting
    float3 vNormalTS = normalize( vNormalHeight.xyz * 2.0 - 1.0 );
    vLightTS = normalize( vLightTS );
    Out.vDiffuseColor = saturate( dot( vNormalTS, vLightTS ) ) * g_materialDiffuseColor;
#endif    

    // Compute position in clipping space
    Out.vPosition = mul( float4( i.inPositionOS.xyz, 1.0 ), g_mWorldViewProjection );
    
    return Out;
}   

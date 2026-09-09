// RUN: %dxc -E main -T ds_6_0 %s | FileCheck %s

// CHECK: domainLocation
// CHECK: domainLocation
// CHECK: domainLocation
// CHECK: Sqrt
// CHECK: FMax
// CHECK: FMin
// CHECK: sampleLevel

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
// Domain Shader
//--------------------------------------------------------------------------------------
[domain("tri")]
DS_OUTPUT main( HS_CONSTANT_DATA_OUTPUT input, float3 BarycentricCoordinates : SV_DomainLocation, 
             const OutputPatch<HS_CONTROL_POINT_OUTPUT, 3> TrianglePatch )
{
    DS_OUTPUT output = (DS_OUTPUT)0;

    // Interpolate world space position with barycentric coordinates
    float3 vWorldPos = BarycentricCoordinates.x * TrianglePatch[0].vWorldPos + 
                       BarycentricCoordinates.y * TrianglePatch[1].vWorldPos + 
                       BarycentricCoordinates.z * TrianglePatch[2].vWorldPos;
    
    // Interpolate world space normal and renormalize it
    float3 vNormal = BarycentricCoordinates.x * TrianglePatch[0].vNormal + 
                     BarycentricCoordinates.y * TrianglePatch[1].vNormal + 
                     BarycentricCoordinates.z * TrianglePatch[2].vNormal;
    vNormal = normalize( vNormal );
    
    // Interpolate other inputs with barycentric coordinates
    output.texCoord = BarycentricCoordinates.x * TrianglePatch[0].texCoord + 
                      BarycentricCoordinates.y * TrianglePatch[1].texCoord + 
                      BarycentricCoordinates.z * TrianglePatch[2].texCoord;
    float3 vLightTS = BarycentricCoordinates.x * TrianglePatch[0].vLightTS + 
                      BarycentricCoordinates.y * TrianglePatch[1].vLightTS + 
                      BarycentricCoordinates.z * TrianglePatch[2].vLightTS;

    // Calculate MIP level to fetch normal from
    float fHeightMapMIPLevel = clamp( ( distance( vWorldPos, g_vEye ) - 100.0f ) / 100.0f, 0.0f, 3.0f);
    
    // Sample normal and height map
    float4 vNormalHeight = g_nmhTexture.SampleLevel( g_samLinear, output.texCoord, fHeightMapMIPLevel );
    
    // Displace vertex along normal
    vWorldPos += vNormal * ( g_vDetailTessellationHeightScale.x * ( vNormalHeight.w-1.0 ) );
    
    // Transform world position with viewprojection matrix
    output.vPosition = mul( float4( vWorldPos.xyz, 1.0 ), g_mViewProjection );
    
#if PERPIXEL_DIFFUSE_LIGHTING==1
    // Per-pixel lighting: pass tangent space light vector to pixel shader
    output.vLightTS = vLightTS;
    
#if ADD_SPECULAR==1
    // Also pass tangent space view vector
    output.vViewTS  = BarycentricCoordinates.x * TrianglePatch[0].vViewTS + 
                      BarycentricCoordinates.y * TrianglePatch[1].vViewTS + 
                      BarycentricCoordinates.z * TrianglePatch[2].vViewTS;
#endif

#else

    // Per-vertex lighting
    float3 vNormalTS = normalize( vNormalHeight.xyz * 2.0 - 1.0 );
    vLightTS = normalize( vLightTS );
    output.vDiffuseColor = saturate( dot( vNormalTS, vLightTS ) ) * g_materialDiffuseColor;

#endif

#if DENSITY_BASED_TESSELLATION==1 && DEBUG_VIEW==1 && PERPIXEL_DIFFUSE_LIGHTING==1
    output.vDiffuseColor = BarycentricCoordinates.x*input.VertexDensity[0] + 
                           BarycentricCoordinates.y*input.VertexDensity[1] + 
                           BarycentricCoordinates.z*input.VertexDensity[2];
#endif

#if DEBUG_VIEW==2
    float fRed =            saturate(   fHeightMapMIPLevel             );
    float fGreen =          saturate( ( fHeightMapMIPLevel-1.0 ) / 2.0 );
    float fBlue =           saturate( ( fHeightMapMIPLevel-2.0 ) / 4.0 );
    output.vDiffuseColor =  float4(fRed, fGreen, fBlue,0);
#endif
    
    return output;
}




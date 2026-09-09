// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: Sqrt
// CHECK: storeOutput

//--------------------------------------------------------------------------------------
// File: POM.hlsl
//
// HLSL file containing shader functions for Parallax Occlusion Mapping.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "shader_include.hlsli"
           
//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float3 inPositionOS  : POSITION;
    float2 inTexCoord    : TEXCOORD0;
    float3 vInNormalOS   : NORMAL;
    float3 vInBinormalOS : BINORMAL;
    float3 vInTangentOS  : TANGENT;
};

struct VS_OUTPUT
{
    float2 texCoord          : TEXCOORD0;
    float3 vLightTS          : TEXCOORD1;   // Light vector in tangent space, denormalized
    float3 vViewTS           : TEXCOORD2;   // View vector in tangent space, denormalized
    float2 vParallaxOffsetTS : TEXCOORD3;   // Parallax offset vector in tangent space
    float3 vNormalWS         : TEXCOORD4;   // Normal vector in world space
    float3 vViewWS           : TEXCOORD5;   // View vector in world space
    
    float4 position          : SV_POSITION; // Output position
};

struct PS_INPUT
{
   float2 texCoord          : TEXCOORD0;
   float3 vLightTS          : TEXCOORD1;   // Light vector in tangent space, denormalized
   float3 vViewTS           : TEXCOORD2;   // View vector in tangent space, denormalized
   float2 vParallaxOffsetTS : TEXCOORD3;   // Parallax offset vector in tangent space
   float3 vNormalWS         : TEXCOORD4;   // Normal vector in world space
   float3 vViewWS           : TEXCOORD5;   // View vector in world space
};


//--------------------------------------------------------------------------------------
// Vertex shader for POM setup
//--------------------------------------------------------------------------------------
VS_OUTPUT main( VS_INPUT i )
{
    VS_OUTPUT Out;
        
    // Transform and output input position 
    Out.position = mul( float4( i.inPositionOS.xyz, 1.0 ), g_mWorldViewProjection );
       
    // Propagate texture coordinate through:
    Out.texCoord = i.inTexCoord * g_fBaseTextureRepeat.x;

    // Transform the normal, tangent and binormal vectors 
    // from object space to homogeneous projection space:
    float3 vNormalWS   = mul( i.vInNormalOS,   (float3x3) g_mWorld );
    float3 vBinormalWS = mul( i.vInBinormalOS, (float3x3) g_mWorld );
    float3 vTangentWS  = mul( i.vInTangentOS,  (float3x3) g_mWorld );
    
    // Propagate the world space vertex normal through:   
    Out.vNormalWS = vNormalWS;
    
    // Normalize tangent space vectors
    vNormalWS   = normalize( vNormalWS );
    vBinormalWS = normalize( vBinormalWS );
    vTangentWS  = normalize( vTangentWS );
    
    // Compute position in world space:
    float4 vPositionWS = mul( i.inPositionOS, g_mWorld );
                 
    // Compute and output the world view vector (unnormalized):
    float3 vViewWS = g_vEye - vPositionWS;
    Out.vViewWS = vViewWS;

    // Compute denormalized light vector in world space:
    float3 vLightWS = g_LightPosition.xyz - vPositionWS.xyz;
    // Need to invert Z for correct lighting
    vLightWS.z = -vLightWS.z;
    
    // Normalize the light and view vectors and transform it to the tangent space:
    float3x3 mWorldToTangent = float3x3( vTangentWS, vBinormalWS, vNormalWS );
       
    // Propagate the view and the light vectors (in tangent space):
    Out.vLightTS = mul( mWorldToTangent, vLightWS );
    Out.vViewTS  = mul( mWorldToTangent, vViewWS  );
       
    // Compute the ray direction for intersecting the 
    // height field profile with current view ray
         
    // Compute initial parallax displacement direction:
    float2 vParallaxDirection = normalize(  Out.vViewTS.xy );
       
    // The length of this vector determines the furthest amount of displacement:
    float fLength         = length( Out.vViewTS );
    float fParallaxLength = sqrt( fLength * fLength - Out.vViewTS.z * Out.vViewTS.z ) / Out.vViewTS.z; 
       
    // Compute the actual reverse parallax displacement vector:
    Out.vParallaxOffsetTS = vParallaxDirection * fParallaxLength;
       
    // Need to scale the amount of displacement to account for different height ranges in height maps.
    Out.vParallaxOffsetTS *= g_fPOMHeightMapScale.x;

   return Out;
}   

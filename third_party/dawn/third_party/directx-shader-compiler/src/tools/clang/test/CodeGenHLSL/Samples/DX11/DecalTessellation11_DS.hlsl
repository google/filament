// RUN: %dxc -E main -T ds_6_0 -HV 2018 %s | FileCheck %s

// CHECK: domainLocation
// CHECK: domainLocation
// CHECK: domainLocation
// CHECK: Sqrt
// CHECK: dot3
// CHECK: dot3
// CHECK: sampleLevel
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
// Domain Shader
//--------------------------------------------------------------------------------------
[domain("tri")]
DS_VS_OUTPUT_PS_INPUT main( HS_CONSTANT_DATA_OUTPUT input, float3 BarycentricCoordinates : SV_DomainLocation, 
             const OutputPatch<HS_CONTROL_POINT_OUTPUT, 3> TrianglePatch )
{
    DS_VS_OUTPUT_PS_INPUT Out;
    
    // Interpolate world space position with barycentric coordinates
    float3 vWorldPos = BarycentricCoordinates.x * TrianglePatch[0].vWorldPos + 
                       BarycentricCoordinates.y * TrianglePatch[1].vWorldPos + 
                       BarycentricCoordinates.z * TrianglePatch[2].vWorldPos;
    
    // Interpolate texture coordinates with barycentric coordinates
    Out.vTexCoord = BarycentricCoordinates.x * TrianglePatch[0].vTexCoord + 
                      BarycentricCoordinates.y * TrianglePatch[1].vTexCoord + 
                      BarycentricCoordinates.z * TrianglePatch[2].vTexCoord;
                      
     // Interpolate normal with barycentric coordinates
    Out.vNormal = BarycentricCoordinates.x * TrianglePatch[0].vNormal + 
                      BarycentricCoordinates.y * TrianglePatch[1].vNormal + 
                      BarycentricCoordinates.z * TrianglePatch[2].vNormal;
   
   
    // Default normal map tex coord and light vector
    Out.vNMTexCoord.z = 0; // z = 0 indicates that this texcoord isn't valid
    Out.vLightWS = g_vLightPosition.xyz - vWorldPos;
    Out.vLightTS = Out.vLightWS;
    Out.vViewWS = g_vEyePosition.xyz - vWorldPos;
    Out.vViewTS = Out.vViewWS;
    
    // See if this vertex is affected by a damage decal
    for (int i = 0; i < MAX_DECALS; i++)
    {
        if (g_vNormal[i].x == 0.0 && g_vNormal[i].y == 0.0 && g_vNormal[i].z == 0.0)
            break;    // the rest of the list is empty

        float3 vHitLocation;
        vHitLocation.x = g_vDecalPositionSize[i].x;
        vHitLocation.y = g_vDecalPositionSize[i].y;
        vHitLocation.z = g_vDecalPositionSize[i].z;
        
        float decalRadius = g_vDecalPositionSize[i].w;
        
        float distanceToHit = distance(vWorldPos, vHitLocation.xyz);
        
        // check if the vertex is within the decal radius
        if (distanceToHit <= decalRadius)
        {
            // rotate the decal tangent space to the vertex normal orientation
            float3 vDecalTangent = g_vTangent[i].xyz;
            float3 vDecalBinormal = g_vBinormal[i].xyz;
            float3 vNormal = normalize(Out.vNormal);
            float3 vBinormal;
            float3 vTangent;
            // find the vector that is closest to being orthogonal to the vertex normal
            if ( abs(dot(vNormal, vDecalTangent)) < abs(dot(vNormal, vDecalBinormal)) )
            {
                vBinormal = normalize(cross(vNormal, vDecalTangent));
                // not necessary to normalize since binormal and normal are orthoganal and unit length
                vTangent = cross(vBinormal, vNormal);
            }
            else
            {
                vTangent = normalize(cross(vNormal, vDecalBinormal));
                // not necessary to normalize since tangent and normal are orthoganal and unit length
                vBinormal = cross(vTangent, vNormal);
            }            
            // tangent space matrix for lighting
            float3x3 mWorldToTangent = float3x3( vTangent, vBinormal, vNormal );
            // tangent space matrix for displacement mapping
            float3x3 mWorldToTangentDM = float3x3( g_vTangent[i].xyz, g_vBinormal[i].xyz, g_vNormal[i].xyz );
            
            // Transform the position into decal tangent space to get the
            // displacement map texture coordinate.
            float3 vWorldPosTrans = vWorldPos - vHitLocation.xyz;
            float3 vDMTexCoord = mul( mWorldToTangentDM, vWorldPosTrans);
            vDMTexCoord /= decalRadius * 2; // scale coord between -0.5 and 0.5
            vDMTexCoord += 0.5; // translate to center (coords between 0 and 1)
            vDMTexCoord.z = 1; // project texcoord onto the x,y plane

            // sample the displacement map for the magnitude of displacement
            float fDisplacement = g_DisplacementMap.SampleLevel( g_sampleLinear, vDMTexCoord.xy, 0 ).r;
            fDisplacement *= g_vDisplacementScaleBias.x;
            fDisplacement += g_vDisplacementScaleBias.y;
            float3 vDirection = -g_vNormal[i].xyz; // hit direction is opposite of tangent space normal
            
            //translate the position
            vWorldPos += vDirection * fDisplacement;
            
            // Create the light vector
            float3 vLightWS = g_vLightPosition.xyz - vWorldPos;
            
            // Create the view vector
            float3 vViewWS = g_vEyePosition.xyz - vWorldPos;

            
            // transform the light vector into tangent space
            Out.vLightTS = mul( mWorldToTangent, vLightWS );
            
            // transform the view vector into tangent space;
            Out.vViewTS = mul( mWorldToTangent, vViewWS );
            
            // Use the same texcoord for the normal map as the displacement map.
            // The z value = 1 will indicate to the pixel shader to use the decal normal map for lighting.
            Out.vNMTexCoord = vDMTexCoord; 
            break;
        }
    }
            
    Out.vPosCS = mul( float4( vWorldPos.xyz, 1.0 ), g_mViewProjection );
    
    return Out;
}

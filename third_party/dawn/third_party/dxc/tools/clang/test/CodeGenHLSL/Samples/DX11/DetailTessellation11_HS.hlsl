// RUN: %dxc -E main -T hs_6_0 %s | FileCheck %s

// CHECK: storePatchConstant
// CHECK: main
// CHECK: outputControlPointID
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
// Hull shader
//--------------------------------------------------------------------------------------
HS_CONSTANT_DATA_OUTPUT ConstantsHS( InputPatch<VS_OUTPUT_HS_INPUT, 3> p, uint PatchID : SV_PrimitiveID )
{
    HS_CONSTANT_DATA_OUTPUT output  = (HS_CONSTANT_DATA_OUTPUT)0;
    float4 vEdgeTessellationFactors = g_vTessellationFactor.xxxy;
    
#if DISTANCE_ADAPTIVE_TESSELLATION==1

    // Calculate edge scale factor from vertex scale factor: simply compute 
    // average tess factor between the two vertices making up an edge
    vEdgeTessellationFactors.x = 0.5 * ( p[1].fVertexDistanceFactor + p[2].fVertexDistanceFactor );
    vEdgeTessellationFactors.y = 0.5 * ( p[2].fVertexDistanceFactor + p[0].fVertexDistanceFactor );
    vEdgeTessellationFactors.z = 0.5 * ( p[0].fVertexDistanceFactor + p[1].fVertexDistanceFactor );
    vEdgeTessellationFactors.w = vEdgeTessellationFactors.x;

    // Multiply them by global tessellation factor
    vEdgeTessellationFactors *= g_vTessellationFactor.xxxy;
    
#elif SCREEN_SPACE_ADAPTIVE_TESSELLATION==1

    // Get the screen space position of each control point
    float2 f2EdgeScreenPosition0 = 
        GetScreenSpacePosition( p[0].vWorldPos.xyz, g_mViewProjection, g_vScreenResolution.x, g_vScreenResolution.y );
    float2 f2EdgeScreenPosition1 = 
        GetScreenSpacePosition( p[1].vWorldPos.xyz, g_mViewProjection, g_vScreenResolution.x, g_vScreenResolution.y );
    float2 f2EdgeScreenPosition2 = 
        GetScreenSpacePosition( p[2].vWorldPos.xyz, g_mViewProjection, g_vScreenResolution.x, g_vScreenResolution.y );
    
    // Calculate edge tessellation factors based on desired screen space tessellation value
    vEdgeTessellationFactors.x = g_vTessellationFactor.w * distance(f2EdgeScreenPosition2, f2EdgeScreenPosition1);
    vEdgeTessellationFactors.y = g_vTessellationFactor.w * distance(f2EdgeScreenPosition2, f2EdgeScreenPosition0);
    vEdgeTessellationFactors.z = g_vTessellationFactor.w * distance(f2EdgeScreenPosition0, f2EdgeScreenPosition1);
    vEdgeTessellationFactors.w = 0.33 * ( vEdgeTessellationFactors.x + vEdgeTessellationFactors.y + vEdgeTessellationFactors.z );
    
#endif

#if DENSITY_BASED_TESSELLATION==1

    // Retrieve edge density from edge density buffer (swizzle required to match vertex ordering)
    vEdgeTessellationFactors *= g_DensityBuffer.Load( PatchID ).yzxw;
    
#endif
    
    // Assign tessellation levels
    output.Edges[0] = vEdgeTessellationFactors.x;
    output.Edges[1] = vEdgeTessellationFactors.y;
    output.Edges[2] = vEdgeTessellationFactors.z;
    output.Inside   = vEdgeTessellationFactors.w;
    
#if FRUSTUM_CULLING_OPTIMIZATION==1
    // View frustum culling
    bool bViewFrustumCull = ViewFrustumCull( p[0].vWorldPos, p[1].vWorldPos, p[2].vWorldPos, g_vFrustumPlaneEquation,
                                             g_vDetailTessellationHeightScale.x );
    if (bViewFrustumCull)
    {
        // Set all tessellation factors to 0 if frustum cull test succeeds
        output.Edges[0] = 0.0;
        output.Edges[1] = 0.0;
        output.Edges[2] = 0.0;
        output.Inside   = 0.0;
    }
#endif

    
    return output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ConstantsHS")]
[maxtessfactor(15.0)]
HS_CONTROL_POINT_OUTPUT main( InputPatch<VS_OUTPUT_HS_INPUT, 3> inputPatch, uint uCPID : SV_OutputControlPointID )
{
    HS_CONTROL_POINT_OUTPUT    output = (HS_CONTROL_POINT_OUTPUT)0;
    
    // Copy inputs to outputs
    output.vWorldPos = inputPatch[uCPID].vWorldPos.xyz;
    output.vNormal =   inputPatch[uCPID].vNormal;
    output.texCoord =  inputPatch[uCPID].texCoord;
    output.vLightTS =  inputPatch[uCPID].vLightTS;
#if ADD_SPECULAR==1
    output.vViewTS =   inputPatch[uCPID].vViewTS;
#endif

    return output;
}


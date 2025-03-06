// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: dot3
// CHECK: Rsqrt
// CHECK: dot3
// CHECK: storeOutput

//--------------------------------------------------------------------------------------
// File: ContactHardeningShadows11.hlsl
//
// These shaders demonstrate the use of the DX11 sm5 instructions
// for fast high quality contact hardening shadows
//
// Contributed by AMD Developer Relations Team
//
// Copyright (c) Microsoft Corporation. All rights reserved.

//--------------------------------------------------------------------------------------

cbuffer cbConstants : register( b0 )
{
    float4x4 g_f4x4WorldViewProjection;        // World * View * Projection matrix
    float4x4 g_f4x4WorldViewProjLight;        // World * ViewLight * Projection Light matrix
    float4   g_vShadowMapDimensions;
    float4   g_f4LightDir;
    float    g_fSunWidth;
    float3   g_f3Padding;
}

//======================================================================================
// Textures and Samplers
//======================================================================================

// Textures
Texture2D         g_txScene     : register( t0 );
Texture2D<float>  g_txShadowMap : register( t1 );

// Samplers
SamplerState                g_SamplePoint       : register( s0 );
SamplerState                g_SampleLinear      : register( s1 );
SamplerComparisonState      g_SamplePointCmp    : register( s2 );

//======================================================================================
// Vertex & Pixel shader structures
//======================================================================================

struct VS_RenderSceneInput
{
    float3 f3Position    : POSITION;  
    float3 f3Normal      : NORMAL;     
    float2 f2TexCoord    : TEXTURE0;
};

struct PS_RenderSceneInput
{
    float4 f4Position   : SV_Position;
    float4 f4Diffuse    : COLOR0; 
    float2 f2TexCoord   : TEXTURE0;
    float4 f4SMC        : TEXTURE1;
};

struct PS_RenderOutput
{
    float4 f4Color    : SV_Target0;
};

#define FILTER_SIZE    11
#define FS  FILTER_SIZE
#define FS2 ( FILTER_SIZE / 2 )

// 4 control matrices for a dynamic cubic bezier filter weights matrix

static const float C3[11][11] = 
                 { { 1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0 }, 
                   { 1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0 },
                   { 1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0 },
                   { 1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0 },
                   { 1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0 },
                   { 1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0 },
                   { 1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0 },
                   { 1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0 },
                   { 1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0 },
                   { 1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0 },
                   { 1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0 },
                   };

static const float C2[11][11] = 
                 { { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 }, 
                   { 0.0,0.2,0.2,0.2,0.2,0.2,0.2,0.2,0.2,0.2,0.0 },
                   { 0.0,0.2,1.0,1.0,1.0,1.0,1.0,1.0,1.0,0.2,0.0 },
                   { 0.0,0.2,1.0,1.0,1.0,1.0,1.0,1.0,1.0,0.2,0.0 },
                   { 0.0,0.2,1.0,1.0,1.0,1.0,1.0,1.0,1.0,0.2,0.0 },
                   { 0.0,0.2,1.0,1.0,1.0,1.0,1.0,1.0,1.0,0.2,0.0 },
                   { 0.0,0.2,1.0,1.0,1.0,1.0,1.0,1.0,1.0,0.2,0.0 },
                   { 0.0,0.2,1.0,1.0,1.0,1.0,1.0,1.0,1.0,0.2,0.0 },
                   { 0.0,0.2,1.0,1.0,1.0,1.0,1.0,1.0,1.0,0.2,0.0 },
                   { 0.0,0.2,0.2,0.2,0.2,0.2,0.2,0.2,0.2,0.2,0.0 },
                   { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 },
                   };

static const float C1[11][11] = 
                 { { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 }, 
                   { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 },
                   { 0.0,0.0,0.2,0.2,0.2,0.2,0.2,0.2,0.2,0.0,0.0 },
                   { 0.0,0.0,0.2,1.0,1.0,1.0,1.0,1.0,0.2,0.0,0.0 },
                   { 0.0,0.0,0.2,1.0,1.0,1.0,1.0,1.0,0.2,0.0,0.0 },
                   { 0.0,0.0,0.2,1.0,1.0,1.0,1.0,1.0,0.2,0.0,0.0 },
                   { 0.0,0.0,0.2,1.0,1.0,1.0,1.0,1.0,0.2,0.0,0.0 },
                   { 0.0,0.0,0.2,1.0,1.0,1.0,1.0,1.0,0.2,0.0,0.0 },
                   { 0.0,0.0,0.2,0.2,0.2,0.2,0.2,0.2,0.2,0.0,0.0 },
                   { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 },
                   { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 },
                   };

static const float C0[11][11] = 
                 { { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 }, 
                   { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 },
                   { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 },
                   { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 },
                   { 0.0,0.0,0.0,0.0,0.8,0.8,0.8,0.0,0.0,0.0,0.0 },
                   { 0.0,0.0,0.0,0.0,0.8,1.0,0.8,0.0,0.0,0.0,0.0 },
                   { 0.0,0.0,0.0,0.0,0.8,0.8,0.8,0.0,0.0,0.0,0.0 },
                   { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 },
                   { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 },
                   { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 },
                   { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 },
                   };

// compute dynamic weight at a certain row, column of the matrix
float Fw( int r, int c, float fL )
{
    return (1.0-fL)*(1.0-fL)*(1.0-fL) * C0[r][c] +
           fL*fL*fL * C3[r][c] +
           3.0f * (1.0-fL)*(1.0-fL)*fL * C1[r][c]+
           3.0f * fL*fL*(1.0-fL) * C2[r][c];
} 

#define BLOCKER_FILTER_SIZE    11
#define BFS  BLOCKER_FILTER_SIZE
#define BFS2 ( BLOCKER_FILTER_SIZE / 2 )

#define SUN_WIDTH g_fSunWidth

//======================================================================================
// This shader computes standard transform and lighting
//======================================================================================
PS_RenderSceneInput main( VS_RenderSceneInput I )
{
    PS_RenderSceneInput O;
    float3 f3NormalWorldSpace;
    
    // Transform the position from object space to homogeneous projection space
    O.f4Position = mul( float4( I.f3Position, 1.0f ), g_f4x4WorldViewProjection );
    
    // Transform the normal from object space to world space    
    f3NormalWorldSpace = normalize( I.f3Normal );
            
    // Calc diffuse color
    float3 f3LightDir = normalize( -g_f4LightDir.xyz ); 
    O.f4Diffuse       = float4( saturate( dot( f3NormalWorldSpace, f3LightDir ) ).xxx, 
                                1.0f );   
    
    // pass through tex coords    
    O.f2TexCoord  = I.f2TexCoord;

    // output position in light space
    O.f4SMC = mul( float4( I.f3Position, 1 ), g_f4x4WorldViewProjLight );

    return O;    
}

//====================================================================================== 
// EOF
//====================================================================================== 

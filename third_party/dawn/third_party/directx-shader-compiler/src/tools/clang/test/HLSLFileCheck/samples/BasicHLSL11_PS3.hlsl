// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: ; PS_IN                    0   xyzw        0     NONE   float
// CHECK: ; PS_IN                    2   xyzw        1     NONE   float
// CHECK: ; PS_IN                    1   xyzw        2     NONE   float
// CHECK: ; PS_IN                    3   xyzw        3     NONE   float
// CHECK: ; PS_INI                   0   xyzw        4     NONE   float
// CHECK: ; PS_INI                   1   xyzw        5     NONE   float
// CHECK: ; PS_INI                   2   xyzw        6     NONE   float
// CHECK: ; PS_INI                   3   xyzw        7     NONE   float
// CHECK: ; PS_INII                  0   xyzw        8     NONE   float
// CHECK: ; PS_INII                  1   xyzw        9     NONE   float
// CHECK: ; PS_INII                  4   xyzw       10     NONE   float
// CHECK: ; PS_INII                  5   xyzw       11     NONE   float
// CHECK: ; PS_INII                  2   xyzw       12     NONE   float
// CHECK: ; PS_INII                  3   xyzw       13     NONE   float
// CHECK: ; PS_INII                  6   xyzw       14     NONE   float
// CHECK: ; PS_INII                  7   xyzw       15     NONE   float
// CHECK: ; NORMAL                   0   xyz        16     NONE   float
// CHECK: ; NORMAL                   1   xyz        17     NONE   float
// CHECK: ; TEXCOORD                 0   xy         18     NONE   float

// CHECK: DepthOutput=0
// CHECK: SampleFrequency=1


// CHECK: NORMAL                   0                 sample
// CHECK: TEXCOORD                 0          noperspective

// CHECK: g_txDiffuse_texture_2d
// CHECK: g_samLinear_sampler

//--------------------------------------------------------------------------------------
// File: BasicHLSL11_PS.hlsl
//
// The pixel shader file for the BasicHLSL11 sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
cbuffer cbPerObject : register( b0 )
{
    float4    g_vObjectColor    : packoffset( c0 );
};

cbuffer cbPerFrame : register( b1 )
{
    float3    g_vLightDir    : packoffset( c0 );
    float    g_fAmbient    : packoffset( c0.w );
};

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
Texture2D    g_txDiffuse : register( t0 );
SamplerState    g_samLinear : register( s0 );

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------

struct S {
 float4 m;
 float4 m2;
};

struct S2 {
 float4 m[2];
 float4 m2[2];
};
struct PS_INPUT
{
  S               s[2] : PS_IN;

  S2              s2 : PS_INI;
  S2              s3[2] : PS_INII;
  sample          float3 vNormal[2]   : NORMAL;
  noperspective   float2 vTexcoord  : TEXCOORD0;
};

uint t;

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 main( PS_INPUT Input) : SV_TARGET
{
    float4 vDiffuse = g_txDiffuse.Sample( g_samLinear, Input.vTexcoord );
    
    float fLighting = saturate( dot( g_vLightDir, Input.vNormal[0] ) );
    fLighting = max( fLighting, g_fAmbient );
    
    return vDiffuse * fLighting;
}


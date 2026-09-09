// RUN: %dxilver 1.5 | %dxc -E main -T ps_6_0 %s -Qstrip_reflect | FileCheck %s

// Make sure there are no type annotations
// CHECK-NOT: !dx.typeAnnotations

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
struct PS_INPUT
{
  sample          float3 vNormal    : NORMAL;
  noperspective   float2 vTexcoord  : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 main( PS_INPUT Input) : SV_TARGET
{
    float4 vDiffuse = g_txDiffuse.Sample( g_samLinear, Input.vTexcoord );
    if (g_vObjectColor.x > 0.3)
      return vDiffuse;

    float fLighting = saturate( dot( g_vLightDir, Input.vNormal ) );
    fLighting = max( fLighting, g_fAmbient );
    
    return vDiffuse * fLighting;
}


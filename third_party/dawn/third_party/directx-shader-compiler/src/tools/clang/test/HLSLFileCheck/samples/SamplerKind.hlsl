// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

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
SamplerComparisonState     g_samLinearC : register( s1 );
RWTexture2D<float4>    uav1 : register( u3 );

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
  sample          float3 vNormal    : NORMAL;
  noperspective   float2 vTexcoord  : TEXCOORD0;
};

float cmpVal;

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 main( PS_INPUT Input) : SV_TARGET
{
    float4 vDiffuse = g_txDiffuse.Sample( g_samLinear, Input.vTexcoord );

    vDiffuse  += g_txDiffuse.CalculateLevelOfDetail(g_samLinear, Input.vTexcoord);

    vDiffuse  += g_txDiffuse.Gather(g_samLinear, Input.vTexcoord);

    vDiffuse  += g_txDiffuse.SampleCmp(g_samLinearC, Input.vTexcoord, cmpVal);

    vDiffuse  += g_txDiffuse.GatherCmp(g_samLinearC, Input.vTexcoord, cmpVal);

    float fLighting = saturate( dot( g_vLightDir, Input.vNormal ) );
    fLighting = max( fLighting, g_fAmbient );
    
    return vDiffuse * fLighting * uav1.Load(int2(0,0));
}


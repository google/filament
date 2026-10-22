// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: sample
// CHECK: bufferLoad
// CHECK: sample
// CHECK: storeOutput

//--------------------------------------------------------------------------------------
// File: FinalPass.hlsl
//
// The PSs for doing tone-mapping based on the input luminance, used in CS path of 
// HDRToneMappingCS11 sample
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
struct QuadVS_Input
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD0;
};

struct QuadVS_Output
{
    float4 Pos : SV_POSITION;              
    float2 Tex : TEXCOORD0;
};

QuadVS_Output QuadVS( QuadVS_Input Input )
{
    QuadVS_Output Output;
    Output.Pos = Input.Pos;
    Output.Tex = Input.Tex;
    return Output;
}

Texture2D<float4> tex : register( t0 );
StructuredBuffer<float> lum : register( t1 );
Texture2D<float4> bloom : register( t2 );

SamplerState PointSampler : register (s0);
SamplerState LinearSampler : register (s1);


static const float  MIDDLE_GRAY = 0.72f;
static const float  LUM_WHITE = 1.5f;

cbuffer cbPS : register( b0 )
{
    float4    g_param;   
};

float4 main( QuadVS_Output Input ) : SV_TARGET
{
    float4 vColor = tex.Sample( PointSampler, Input.Tex );
    float fLum = lum[0]*g_param.x;
    float3 vBloom = bloom.Sample( LinearSampler, Input.Tex );

    // Tone mapping
    vColor.rgb *= MIDDLE_GRAY / (fLum + 0.001f);
    vColor.rgb *= (1.0f + vColor/LUM_WHITE);
    vColor.rgb /= (1.0f + vColor);
    
    vColor.rgb += 0.6f * vBloom;
    vColor.a = 1.0f;

    return vColor;
}


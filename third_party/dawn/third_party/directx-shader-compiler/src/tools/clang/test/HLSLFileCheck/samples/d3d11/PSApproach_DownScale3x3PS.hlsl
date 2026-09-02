// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: sample
// CHECK: storeOutput

//--------------------------------------------------------------------------------------
// File: PSApproach.hlsl
//
// The PSs for doing post-processing, used in PS path of 
// HDRToneMappingCS11 sample
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
static const float4 LUM_VECTOR = float4(.299, .587, .114, 0);
static const float  MIDDLE_GRAY = 0.72f;
static const float  LUM_WHITE = 1.5f;
static const float  BRIGHT_THRESHOLD = 0.5f;

SamplerState PointSampler : register (s0);
SamplerState LinearSampler : register (s1);

struct QuadVS_Output
{
    float4 Pos : SV_POSITION;              
    float2 Tex : TEXCOORD0;
};

Texture2D s0 : register(t0);
Texture2D s1 : register(t1);
Texture2D s2 : register(t2);


float4 main( QuadVS_Output Input ) : SV_TARGET
{
    float fAvg = 0.0f; 
    float4 vColor;
    
    for( int y = -1; y <= 1; y++ )
    {
        for( int x = -1; x <= 1; x++ )
        {
            // Compute the sum of color values
            vColor = s0.Sample( PointSampler, Input.Tex, int2(x,y) );
                        
            fAvg += vColor.r; 
        }
    }
    
    // Divide the sum to complete the average
    fAvg /= 9;
    
    return float4(fAvg, fAvg, fAvg, 1.0f);
}


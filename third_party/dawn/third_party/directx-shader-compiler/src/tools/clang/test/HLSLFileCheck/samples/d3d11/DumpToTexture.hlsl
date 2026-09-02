// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: bufferLoad
// CHECK: storeOutput

//--------------------------------------------------------------------------------------
// File: DumpToTexture.hlsl
//
// The PS for converting CS output buffer to a texture, used in CS path of 
// HDRToneMappingCS11 sample
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
StructuredBuffer<float4> buffer : register( t0 );

struct QuadVS_Output
{
    float4 Pos : SV_POSITION;              
    float2 Tex : TEXCOORD0;
};

cbuffer cbPS : register( b0 )
{
    uint4    g_param;   
};

float4 main( QuadVS_Output Input ) : SV_TARGET
{
    // To calculate the buffer offset, it is natural to use the screen space coordinates,
    // Input.Pos is the screen space coordinates of the pixel being written 
    return buffer[ (Input.Pos.x - 0.5) + (Input.Pos.y - 0.5) * g_param.x ];	
}

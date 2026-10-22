// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: groupId
// CHECK: flattenedThreadIdInGroup
// CHECK: IMax
// CHECK: IMin
// CHECK: bufferLoad
// CHECK: addrspace(3)
// CHECK: barrier
// CHECK: bufferStore

//--------------------------------------------------------------------------------------
// File: FilterCS.hlsl
//
// The CSs for doing vertical and horizontal blur, used in CS path of 
// HDRToneMappingCS11 sample
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
StructuredBuffer<float4> InputBuf : register( t0 );
Texture2D InputTex : register( t1 ); 
RWStructuredBuffer<float4> Result : register( u0 );

cbuffer cb0
{
    float4  g_avSampleWeights[15];
    int2    g_outputsize;
    int2    g_inputsize;
}

#define kernelhalf 7
#define groupthreads 128
groupshared float4 temp[groupthreads];

[numthreads( groupthreads, 1, 1 )]
void main( uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex )
{
    int offsety = GI - kernelhalf + (groupthreads - kernelhalf * 2) * Gid.y;
    offsety = clamp( offsety, 0, g_inputsize.y-1 );
    int offset = Gid.x + offsety * g_inputsize.x;
    temp[GI] = InputBuf[offset];

    GroupMemoryBarrierWithGroupSync();

    // Vertical blur
    if ( GI >= kernelhalf && 
         GI < (groupthreads - kernelhalf) && 
         ( (GI - kernelhalf + (groupthreads - kernelhalf * 2) * Gid.y) < g_outputsize.y) )
    {
        float4 vOut = 0;
        
        [unroll]
        for ( int i = -kernelhalf; i <= kernelhalf; ++i )
            vOut += temp[GI + i] * g_avSampleWeights[i + kernelhalf];

        Result[Gid.x + (GI - kernelhalf + (groupthreads - kernelhalf * 2) * Gid.y) * g_outputsize.x] = float4(vOut.rgb, 1.0f);
    }
}


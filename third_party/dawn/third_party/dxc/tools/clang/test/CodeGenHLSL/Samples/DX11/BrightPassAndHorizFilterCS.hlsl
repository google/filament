// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: groupId
// CHECK: flattenedThreadIdInGroup
// CHECK: IMax
// CHECK: IMin
// CHECK: textureLoad
// CHECK: bufferLoad
// CHECK: FMax
// CHECK: barrier
// CHECK: bufferStore

//--------------------------------------------------------------------------------------
// File: BrightPassAndHorizFilterCS.hlsl
//
// The CS for bright pass and horizontal blur, used in CS path of 
// HDRToneMappingCS11 sample
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
static const float  MIDDLE_GRAY = 0.72f;
static const float  LUM_WHITE = 1.5f;
static const float  BRIGHT_THRESHOLD = 0.5f;

Texture2D Input : register( t0 ); 
StructuredBuffer<float> lum : register( t1 );
RWStructuredBuffer<float4> Result : register( u0 );

cbuffer cb0
{
    float4  g_avSampleWeights[15];
    uint    g_outputwidth;
    float   g_inverse;
    int2    g_inputsize;
}

#define kernelhalf 7
#define groupthreads 128
groupshared float4 temp[groupthreads];

[numthreads( groupthreads, 1, 1 )]
void main( uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex )
{
    int2 coord = int2( GI - kernelhalf + (groupthreads - kernelhalf * 2) * Gid.x, Gid.y );
    coord = coord.xy * 8 + int2(4, 3);
    coord = clamp( coord, int2(0, 0), int2(g_inputsize.x-1, g_inputsize.y-1) );
    float4 vColor = Input.Load( int3(coord, 0) );

    float fLum = lum[0]*g_inverse;

    // Bright pass and tone mapping
    vColor = max( 0.0f, vColor - BRIGHT_THRESHOLD );
    vColor *= MIDDLE_GRAY / (fLum + 0.001f);
    vColor *= (1.0f + vColor/LUM_WHITE);
    vColor /= (1.0f + vColor);

    temp[GI] = vColor;

    GroupMemoryBarrierWithGroupSync();

    // Horizontal blur
    if ( GI >= kernelhalf && 
         GI < (groupthreads - kernelhalf) && 
         ( (Gid.x * (groupthreads - 2 * kernelhalf) + GI - kernelhalf) < g_outputwidth) )
    {
        float4 vOut = 0;
        
        [unroll]
        for ( int i = -kernelhalf; i <= kernelhalf; ++i )
            vOut += temp[GI + i] * g_avSampleWeights[i + kernelhalf];

        Result[GI - kernelhalf + (groupthreads - kernelhalf * 2) * Gid.x + Gid.y * g_outputwidth] = float4(vOut.rgb, 1.0f);
    }
}

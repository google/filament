// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: groupId
// CHECK: threadId
// CHECK: flattenedThreadIdInGroup
// CHECK: textureLoad
// CHECK: dot4
// CHECK: addrspace(3)
// CHECK: barrier
// CHECK: addrspace(3)
// CHECK: barrier
// CHECK: addrspace(3)
// CHECK: barrier
// CHECK: addrspace(3)
// CHECK: barrier
// CHECK: bufferStore

//-----------------------------------------------------------------------------
// File: ReduceTo1DCS.hlsl
//
// Desc: Reduce an input Texture2D to a buffer
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
Texture2D Input : register( t0 ); 
RWStructuredBuffer<float> Result : register( u0 );

cbuffer cbCS : register( b0 )
{
    uint4    g_param;   // (g_param.x, g_param.y) is the x and y dimensions of the Dispatch call
                        // (g_param.z, g_param.w) is the size of the above Input Texture2D
};

//#define CS_FULL_PIXEL_REDUCITON // Defining this or not must be the same as in HDRToneMappingCS11.cpp

#define blocksize 8
#define blocksizeY 8
#define groupthreads (blocksize*blocksizeY)
groupshared float accum[groupthreads];

static const float4 LUM_VECTOR = float4(.299, .587, .114, 0);

[numthreads(blocksize,blocksizeY,1)]
void main( uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex )
{    
    float4 s = 
#ifdef CS_FULL_PIXEL_REDUCITON
        Input.Load( uint3(DTid.xy                                                   , 0) )+ 
        Input.Load( uint3(DTid.xy + uint2(blocksize*g_param.x,                    0), 0) ) +
        Input.Load( uint3(DTid.xy + uint2(0,                   blocksizeY*g_param.y), 0) ) + 
        Input.Load( uint3(DTid.xy + uint2(blocksize*g_param.x, blocksizeY*g_param.y), 0) );
#else
        Input.Load( uint3((float)DTid.x/81.0f*g_param.z, (float)DTid.y/81.0f*g_param.w, 0) );
#endif
        
    accum[GI] = dot( s, LUM_VECTOR );

    // Parallel reduction algorithm follows 
    GroupMemoryBarrierWithGroupSync();
    if ( GI < 32 )
        accum[GI] += accum[32+GI];

    GroupMemoryBarrierWithGroupSync();
    if ( GI < 16 )
        accum[GI] += accum[16+GI];

    GroupMemoryBarrierWithGroupSync();
    if ( GI < 8 )
        accum[GI] += accum[8+GI];

    GroupMemoryBarrierWithGroupSync();
    if ( GI < 4 )
        accum[GI] += accum[4+GI];

    GroupMemoryBarrierWithGroupSync();
    if ( GI < 2 )
        accum[GI] += accum[2+GI];

    GroupMemoryBarrierWithGroupSync();
    if ( GI < 1 )
        accum[GI] += accum[1+GI];

    if ( GI == 0 )
    {                
        Result[Gid.y*g_param.x+Gid.x] = accum[0];
    }
}

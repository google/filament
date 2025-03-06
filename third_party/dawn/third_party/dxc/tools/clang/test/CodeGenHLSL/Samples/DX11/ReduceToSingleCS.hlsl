// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: groupId
// CHECK: threadId
// CHECK: flattenedThreadIdInGroup
// CHECK: addrspace(3)
// CHECK: barrier
// CHECK: addrspace(3)
// CHECK: barrier
// CHECK: addrspace(3)
// CHECK: barrier
// CHECK: bufferStore


//-----------------------------------------------------------------------------
// File: ReduceToSingleCS.hlsl
//
// Desc: Reduce an input buffer by a factor of groupthreads
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

StructuredBuffer<float> Input : register( t0 );
RWStructuredBuffer<float> Result : register( u0 );

cbuffer cbCS : register( b0 )
{
    uint4    g_param;   // g_param.x is the actual elements contained in Input
                        // g_param.y is the x dimension of the Dispatch call
};

#define groupthreads 128
groupshared float accum[groupthreads];

[numthreads(groupthreads,1,1)]
void main( uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex )
{
    if ( DTid.x < g_param.x )
        accum[GI] = Input[DTid.x];
    else
        accum[GI] = 0;

    // Parallel reduction algorithm follows 
    GroupMemoryBarrierWithGroupSync();
    if ( GI < 64 )
        accum[GI] += accum[64+GI];  

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
        Result[Gid.x] = accum[0];
    }
}

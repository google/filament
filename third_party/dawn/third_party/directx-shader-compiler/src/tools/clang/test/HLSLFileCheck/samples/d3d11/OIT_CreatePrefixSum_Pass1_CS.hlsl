// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: groupId
// CHECK: bufferLoad
// CHECK: bufferStore


//-----------------------------------------------------------------------------
// File: OIT_CS.hlsl
//
// Desc: Compute shaders for used in the Order Independent Transparency sample.
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
// TODO: use structured buffers
RWBuffer<float>     deepBufferDepth     : register( u0 );
RWBuffer<uint>      deepBufferColorUINT : register( u1 );
RWTexture2D<float4> frameBuffer         : register( u2 );
RWBuffer<uint>      prefixSum           : register( u3 );

Texture2D<uint> fragmentCount : register ( t0 );

cbuffer CB : register( b0 )
{
    uint g_nFrameWidth      : packoffset( c0.x );
    uint g_nFrameHeight     : packoffset( c0.y );
    uint g_nPassSize        : packoffset( c0.z );
    uint g_nReserved        : packoffset( c0.w );
}

#define blocksize 1
#define groupthreads (blocksize*blocksize)
groupshared float accum[groupthreads];


// Second and following passes.  Each pass distributes the sum of the first half of the group
// to the second half of the group.  There are n/groupsize groups in each pass.
// Each pass increases the group size until it is the size of the buffer.
// The resulting buffer holds the prefix sum of all preceding values in each
// position 
[numthreads(1,1,1)]
void main( uint3 nGid : SV_GroupID, uint3 nDTid : SV_DispatchThreadID, uint3 nGTid : SV_GroupThreadID )
{
    int nThreadNum = nGid.x;
    
    int nValue = prefixSum[nThreadNum*g_nPassSize + g_nPassSize/2 - 1];
    for(int i = nThreadNum*g_nPassSize + g_nPassSize/2; i < nThreadNum*g_nPassSize + g_nPassSize && i < g_nFrameWidth*g_nFrameHeight; i++)
    {
        prefixSum[i] = prefixSum[i] + nValue;
    }
}

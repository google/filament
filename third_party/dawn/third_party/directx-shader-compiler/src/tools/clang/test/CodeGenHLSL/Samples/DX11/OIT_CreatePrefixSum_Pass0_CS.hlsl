// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: groupId
// CHECK: textureLoad
// CHECK: bufferStore
// CHECK: bufferLoad
// CHECK: textureLoad
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

// First pass of the prefix sum creation algorithm.  Converts a 2D buffer to a 1D buffer,
// and sums every other value with the previous value.
[numthreads(1,1,1)]
void main( uint3 nGid : SV_GroupID, uint3 nDTid : SV_DispatchThreadID, uint3 nGTid : SV_GroupThreadID )
{
    int nThreadNum = nGid.y*g_nFrameWidth + nGid.x;
    if( nThreadNum%2 == 0 )
    {
        prefixSum[nThreadNum] = fragmentCount[nGid.xy];
        
        // Add the Fragment count to the next bin
        if( (nThreadNum+1) < g_nFrameWidth * g_nFrameHeight )
        {
            int2 nextUV;
            nextUV.x = (nThreadNum+1) % g_nFrameWidth;
            nextUV.y = (nThreadNum+1) / g_nFrameWidth;
            prefixSum[ nThreadNum+1 ] = prefixSum[ nThreadNum ] + fragmentCount[ nextUV ];
        }
    }
}

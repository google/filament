// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: groupId
// CHECK: threadId
// CHECK: threadIdInGroup
// CHECK: textureLoad
// CHECK: Log
// CHECK: Round_pi
// CHECK: bufferLoad
// CHECK: textureStore

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


#if 1

// Sort the fragments using a bitonic sort, then accumulate the fragments into the final result.
groupshared int nIndex[32];
#define NUM_THREADS 8
[numthreads(1,1,1)]
void main( uint3 nGid : SV_GroupID, uint3 nDTid : SV_DispatchThreadID, uint3 nGTid : SV_GroupThreadID )
{
    uint nThreadNum = nGid.y * g_nFrameWidth + nGid.x;
    
//    uint r0, r1, r2;
//    float rd0, rd1, rd2, rd3, rd4, rd5, rd6, rd7;

    uint N = fragmentCount[nDTid.xy];
    
    uint N2 = 1 << (int)(ceil(log2(N)));

    float fDepth[32];
    for(int i = 0; i < N; i++)
    {
        nIndex[i] = i;
        fDepth[i] = deepBufferDepth[ prefixSum[nThreadNum-1] + i ];
    }
    for(int i = N; i < N2; i++)
    {
        nIndex[i] = i;
        fDepth[i] = 1.1f;
    }
    
    uint idx = blocksize*nGTid.y + nGTid.x;

    // Bitonic sort
    for( int k = 2; k <= N2; k = 2*k )
    {
        for( int j = k>>1; j > 0 ; j = j>>1 ) 
        {
            for( int i = 0; i < N2; i++ ) 
            {
//                GroupMemoryBarrierWithGroupSync();
                //i = idx;

                float di = fDepth[ nIndex[ i ] ];
                int ixj = i^j;
                if ( ( ixj ) > i )
                {
                    float dixj = fDepth[ nIndex[ ixj ] ];
                    if ( ( i&k ) == 0 && di > dixj )
                    { 
                        int temp = nIndex[ i ];
                        nIndex[ i ] = nIndex[ ixj ];
                        nIndex[ ixj ] = temp;
                    }
                    if ( ( i&k ) != 0 && di < dixj )
                    {
                        int temp = nIndex[ i ];
                        nIndex[ i ] = nIndex[ ixj ];
                        nIndex[ ixj ] = temp;
                    }
                }
            }
        }
    }

    // Output the final result to the frame buffer
    if( idx == 0 )
    {

     /*   
        // Debug
        uint color[8];
        for(int i = 0; i < 8; i++)
        {
            color[i] = deepBufferColorUINT[prefixSum[nThreadNum-1] + i];
        }

        for(int i = 0; i < 8; i++)
        {
            deepBufferDepth[nThreadNum*8+i] = fDepth[i];//fDepth[nIndex[i]];
            deepBufferColorUINT[nThreadNum*8+i] = color[nIndex[i]];
        }
     */     
   
        // Accumulate fragments into final result
        float4 result = 0.0f;
        for( int x = N-1; x >= 0; x-- )
        {
            uint bufferValue = deepBufferColorUINT[ prefixSum[nThreadNum-1] + nIndex[ x ] ];
            float4 color;
            color.r = ( ( bufferValue >> 0  & 0xFF )) / 255.0f;
            color.g = ( bufferValue >> 8  & 0xFF ) / 255.0f;
            color.b = ( bufferValue >> 16 & 0xFF ) / 255.0f;
            color.a = ( bufferValue >> 24 & 0xFF ) / 255.0f;
            result = lerp( result, color, color.a );
        }
        result.a = 1.0f;
        frameBuffer[ nGid.xy ] = result;
    }
}

#else
[numthreads(1,1,1)]
void main( uint3 nGid : SV_GroupID, uint3 nDTid : SV_DispatchThreadID, uint3 nGTid : SV_GroupThreadID )
{
    uint nThreadNum = nDTid.y * g_nFrameWidth + nDTid.x;
    float d0 = deepBufferDepth[nThreadNum*8];
    float d1 = deepBufferDepth[nThreadNum*8+1];
    float d2 = deepBufferDepth[nThreadNum*8+2];
    
    uint s0 = deepBufferColorUINT[nThreadNum*8 + 0]; 
    uint s1 = deepBufferColorUINT[nThreadNum*8 + 1];
    uint s2 = deepBufferColorUINT[nThreadNum*8 + 2];
    
    uint r0, r1, r2;
    float rd0, rd1, rd2;
    if( d0 < d1 && d0 < d2 )
    {
        r0 = s0;
        rd0 = d0;
        if( d1 < d2 )
        {
           r1 = s1;
           r2 = s2;
           
           rd1 = d1;
           rd2 = d2;
        }
        else
        {
            r1 = s2;
            r2 = s1;
            
            rd1 = d2;
            rd2 = d1;
        } 
    }
    else if( d1 < d2 )
    {
        r0 = s1;
        rd0 = d1;
        if( d0 < d2 )
        {
          r1 = s0;
          r2 = s2;
          
          rd1 = d0;
          rd2 = d2;
        }
        else
        {
          r1 = s2;
          r2 = s0;
          
          rd1 = d2;
          rd2 = d0;
        }
    }
    else
    {
        r0 = s2;
        rd0 = d2;
        if( d1 < d0 )
        {
          r1 = s1;
          r2 = s0;
          
          rd1 = d1;
          rd2 = d0;
        }
        else
        {
          r1 = s0;
          r2 = s1;
          
          rd1 = d0;
          rd2 = d1;
        }
    }
    
    deepBufferDepth[nThreadNum*8] = rd0;
    deepBufferDepth[nThreadNum*8+1] = rd1;
    deepBufferDepth[nThreadNum*8+2] = rd2;

    deepBufferColorUINT[nThreadNum*8] = r0;
    deepBufferColorUINT[nThreadNum*8+1] = r1;
    deepBufferColorUINT[nThreadNum*8+2] = r2;

    // convert the color to floats
    float4 color[3];
    color[0].r = (r0 >> 0  & 0xFF) / 255.0f;
    color[0].g = (r0 >> 8  & 0xFF) / 255.0f;
    color[0].b = (r0 >> 16 & 0xFF) / 255.0f;
    color[0].a = (r0 >> 24 & 0xFF) / 255.0f;
    
    color[1].r = (r1 >> 0  & 0xFF) / 255.0f;
    color[1].g = (r1 >> 8  & 0xFF) / 255.0f;
    color[1].b = (r1 >> 16 & 0xFF) / 255.0f;
    color[1].a = (r1 >> 24 & 0xFF) / 255.0f;
    
    color[2].r = (r2 >> 0  & 0xFF) / 255.0f;
    color[2].g = (r2 >> 8  & 0xFF) / 255.0f;
    color[2].b = (r2 >> 16 & 0xFF) / 255.0f;
    color[2].a = (r2 >> 24 & 0xFF) / 255.0f;
    
    float4 result = lerp(lerp(lerp(0, color[2], color[2].a), color[1], color[1].a), color[0], color[0].a);
    result.a = 1.0f;
    
    frameBuffer[nDTid.xy] = result;
}

#endif
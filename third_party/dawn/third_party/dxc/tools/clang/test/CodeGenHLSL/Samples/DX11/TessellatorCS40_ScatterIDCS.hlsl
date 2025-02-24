// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: threadId
// CHECK: bufferLoad
// CHECK: bufferStore

//--------------------------------------------------------------------------------------
// File: TessellatorCS40_ScatterIDCS.hlsl
//
// The CS to scatter vertex ID and triangle ID
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
StructuredBuffer<uint2> InputScanned : register(t0);
RWStructuredBuffer<uint2> TriIDIndexIDOut : register(u0);

cbuffer cbCS : register(b1)
{
    uint4 g_param;
}

[numthreads(128, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    if (DTid.x < g_param.x)
    {
        uint start = InputScanned[DTid.x-1].x;
        uint end = InputScanned[DTid.x].x;

        for ( uint i = start; i < end; ++i ) 
        {
            TriIDIndexIDOut[i] = uint2(DTid.x, i - start);
        }
    }
}


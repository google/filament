// RUN: %dxc -E main -T cs_6_0 -O0 -HV 2018 %s | FileCheck %s

// CHECK: threadId
// CHECK: bufferLoad
// CHECK: FMax
// CHECK: FMin
// CHECK: Round_pi
// CHECK: Round_ni
// CHECK: UMax
// CHECK: bufferStore

//--------------------------------------------------------------------------------------
// File: TessellatorCS40_NumVerticesIndicesCS.hlsl
//
// The CS to compute number of vertices and triangles to be generated from edge tessellation factor
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "TessellatorCS40_common.hlsli"

StructuredBuffer<float4> InputEdgeFactor : register(t0);
RWStructuredBuffer<uint2> NumVerticesIndicesOut : register(u0);

cbuffer cbCS : register(b1)
{
    uint4 g_param;
}

[numthreads(128, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    if (DTid.x < g_param.x)
    {
        float4 edge_factor = InputEdgeFactor[DTid.x];
        
        PROCESSED_TESS_FACTORS_TRI processedTessFactors;
        int num_points = TriProcessTessFactors(edge_factor, processedTessFactors, g_partitioning);

        int num_index;
        if (0 == num_points)
        {
            num_index = 0;
        }
        else if (3 == num_points)
        {
            num_index = 4;
        }
        else
        {
            int numRings = ((processedTessFactors.numPointsForOutsideInside.w + 1) / 2); // +1 is so even tess includes the center point, which we want to now

            int4 outsideInsideHalfTessFactor = int4(ceil(processedTessFactors.outsideInsideHalfTessFactor));
            uint3 n = NumStitchTransition(outsideInsideHalfTessFactor, processedTessFactors.outsideInsideTessFactorParity);
            num_index = n.x + n.y + n.z;
            num_index += TotalNumStitchRegular(true, DIAGONALS_MIRRORED, processedTessFactors.numPointsForOutsideInside.w, numRings - 1) * 3;
            if( processedTessFactors.outsideInsideTessFactorParity.w == TESSELLATOR_PARITY_ODD )
            {
                num_index += 4;
            }
        }

        NumVerticesIndicesOut[DTid.x] = uint2(num_points, num_index);
    }
}

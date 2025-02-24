// RUN: %dxc -E main -T cs_6_0 -O0 -HV 2018 %s | FileCheck %s

// CHECK: threadId
// CHECK: bufferLoad
// CHECK: FMax
// CHECK: FMin
// CHECK: Round_pi
// CHECK: Round_ni
// CHECK: UMax
// CHECK: Sqrt
// CHECK: Frc
// CHECK: bufferStore


//--------------------------------------------------------------------------------------
// File: TessellatorCS40_TessellateVerticesCS.hlsl
//
// The CS to tessellate vertices
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "TessellatorCS40_common.hlsli"

StructuredBuffer<uint2> InputTriIDIndexID : register(t0);
StructuredBuffer<float4> InputEdgeFactor : register(t1);

struct TessedVertex
{
    uint BaseTriID;
    float2 bc;
};
RWStructuredBuffer<TessedVertex> TessedVerticesOut : register(u0);

cbuffer cbCS : register(b1)
{
    uint4 g_param;
}

void PlacePointIn1D(PROCESSED_TESS_FACTORS_TRI processedTessFactors, int ctx_index, int pt, out float location, int parity)
{
    int numHalfTessFactorPoints = int(ceil(processedTessFactors.outsideInsideHalfTessFactor[ctx_index]));

    bool bFlip;
    if( pt >= numHalfTessFactorPoints )
    {
        pt = (numHalfTessFactorPoints << 1) - pt;
        if( TESSELLATOR_PARITY_ODD == parity )
        {
            pt -= 1;
        }
        bFlip = true;
    }
    else
    {
        bFlip = false;
    }

    if( pt == numHalfTessFactorPoints ) 
    {
        location = 0.5f;
    }    
    else
    {
        unsigned int indexOnCeilHalfTessFactor = pt;
        unsigned int indexOnFloorHalfTessFactor = indexOnCeilHalfTessFactor;
        if( pt > processedTessFactors.outsideInsideSplitPointOnFloorHalfTessFactor[ctx_index] )
        {
            indexOnFloorHalfTessFactor -= 1;
        }
        float locationOnFloorHalfTessFactor = indexOnFloorHalfTessFactor * processedTessFactors.outsideInsideInvNumSegmentsOnFloorTessFactor[ctx_index];
        float locationOnCeilHalfTessFactor = indexOnCeilHalfTessFactor * processedTessFactors.outsideInsideInvNumSegmentsOnCeilTessFactor[ctx_index];

        location = lerp(locationOnFloorHalfTessFactor, locationOnCeilHalfTessFactor, frac(processedTessFactors.outsideInsideHalfTessFactor[ctx_index]));

        if( bFlip )
        {
            location = 1.0f - location;
        }
    }
}

[numthreads(128, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID, uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex  )
{
    uint id = DTid.x;
    //uint id = Gid.x * 128 + GI; // Workaround for some CS4x preview drivers
    
    if ( id < g_param.x )
    {
        uint tri_id = InputTriIDIndexID[id].x;
        uint vert_id = InputTriIDIndexID[id].y;
        
        float4 outside_inside_factor = InputEdgeFactor[tri_id];

        PROCESSED_TESS_FACTORS_TRI processedTessFactors;
        int num_points = TriProcessTessFactors(outside_inside_factor, processedTessFactors, g_partitioning);

        float2 uv;
        if (3 == num_points)
        {
            if (0 == vert_id)
            {
                uv = float2(0, 1);
            }
            else if (1 == vert_id)
            {
                uv = float2(0, 0);
            }
            else
            {
                uv = float2(1, 0);
            }
        }
        else
        {
            if (vert_id < processedTessFactors.insideEdgePointBaseOffset)
            {
                // Generate exterior ring edge points, clockwise starting from point V (VW, the U==0 edge)

                int edge;
                if (vert_id < processedTessFactors.numPointsForOutsideInside.x - 1)
                {
                    edge = 0;
                }
                else
                {
                    vert_id -= processedTessFactors.numPointsForOutsideInside.x - 1;
                    if (vert_id < processedTessFactors.numPointsForOutsideInside.y - 1)
                    {
                        edge = 1;
                    }
                    else
                    {
                        vert_id -= processedTessFactors.numPointsForOutsideInside.y - 1;
                        edge = 2;
                    }
                }
                
                int p = vert_id;
                int endPoint = processedTessFactors.numPointsForOutsideInside[edge] - 1;
                float param;
                int q = (edge & 0x1) ? p : endPoint - p; // whether to reverse point order given we are defining V or U (W implicit):
                                                     // edge0, VW, has V decreasing, so reverse 1D points below
                                                     // edge1, WU, has U increasing, so don't reverse 1D points  below
                                                     // edge2, UV, has U decreasing, so reverse 1D points below
                PlacePointIn1D(processedTessFactors, edge,q,param, processedTessFactors.outsideInsideTessFactorParity[edge]);
                if (0 == edge)
                {
                    uv = float2(0, param);
                }
                else if (1 == edge)
                {
                    uv = float2(param, 0);
                }
                else
                {
                    uv = float2(param, 1 - param);
                }
            }
            else
            {
                // Generate interior ring points, clockwise spiralling in

                uint index = vert_id - processedTessFactors.insideEdgePointBaseOffset;
                uint ring = 1 + (((3 * processedTessFactors.numPointsForOutsideInside.w - 6) - sqrt(sqr(3 * processedTessFactors.numPointsForOutsideInside.w - 6) - 4 * 3 * index)) + 0.001f) / 6;
                index -= 3 * (processedTessFactors.numPointsForOutsideInside.w - ring - 1) * (ring - 1);

                uint startPoint = ring;
                uint endPoint = processedTessFactors.numPointsForOutsideInside.w - 1 - startPoint;
                if (index < 3 * (endPoint - startPoint))
                {
                    uint edge = index / (endPoint - startPoint);
                    uint p = index - edge * (endPoint - startPoint) + startPoint;

                    int perpendicularAxisPoint = startPoint;
                    float perpParam;
                    PlacePointIn1D(processedTessFactors, 3, perpendicularAxisPoint, perpParam, processedTessFactors.outsideInsideTessFactorParity.w);
                    perpParam = perpParam * 2 / 3;
                    
                    float param;
                    int q = (edge & 0x1) ? p : endPoint - (p - startPoint); // whether to reverse point given we are defining V or U (W implicit):
                                                             // edge0, VW, has V decreasing, so reverse 1D points below
                                                             // edge1, WU, has U increasing, so don't reverse 1D points  below
                                                             // edge2, UV, has U decreasing, so reverse 1D points below
                    PlacePointIn1D(processedTessFactors, 3, q,param, processedTessFactors.outsideInsideTessFactorParity.w);
                    // edge0 VW, has perpendicular parameter U constant
                    // edge1 WU, has perpendicular parameter V constant
                    // edge2 UV, has perpendicular parameter W constant 
                    const unsigned int deriv = 2; // reciprocal is the rate of change of edge-parallel parameters as they are pushed into the triangle
                    if (0 == edge)
                    {
                        uv = float2(perpParam, param - perpParam / deriv);
                    }
                    else if (1 == edge)
                    {
                        uv = float2(param - perpParam / deriv, perpParam);
                    }
                    else
                    {
                        uv = float2(param - perpParam / deriv, 1 - (param - perpParam / deriv + perpParam));
                    }
                }
                else
                {
                    if( processedTessFactors.outsideInsideTessFactorParity.w != TESSELLATOR_PARITY_ODD )
                    {
                        // Last point is the point at the center.
                        uv = 1 / 3.0f;
                    }
                }
            }
        }
        
        TessedVerticesOut[id].BaseTriID = tri_id;
        TessedVerticesOut[id].bc = uv;
    }    
}

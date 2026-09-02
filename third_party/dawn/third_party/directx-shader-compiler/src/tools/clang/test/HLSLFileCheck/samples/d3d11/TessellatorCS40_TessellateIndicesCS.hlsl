// RUN: %dxc -E main -T cs_6_0 -O0 -HV 2018 %s | FileCheck %s

// CHECK: threadId
// CHECK: bufferLoad
// CHECK: FMax
// CHECK: FMin
// CHECK: Round_pi
// CHECK: Round_ni
// CHECK: UMax
// CHECK: Sqrt
// CHECK: bufferStore

//--------------------------------------------------------------------------------------
// File: TessellatorCS40_TessellateIndicesCS.hlsl
//
// The CS to tessellate indices
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "TessellatorCS40_common.hlsli"

StructuredBuffer<uint2> InputTriIDIndexID : register(t0);
StructuredBuffer<float4> InputEdgeFactor : register(t1);
StructuredBuffer<uint2> InputScanned : register(t2);

RWByteAddressBuffer TessedIndicesOut : register(u0);
RWStructuredBuffer<int4> DebugOutput : register(u1); // HLSL Change

cbuffer cbCS : register(b1)
{
    uint4 g_param;
}


int TransformIndex1(int index, int vertices_base)
{
    return vertices_base + index;
}

int TransformIndex2(int index, int vertices_base, INDEX_PATCH_CONTEXT IndexPatchContext)
{
    if( index >= IndexPatchContext.outsidePointIndexPatchBase ) // assumed remapped outide indices are > remapped inside vertices
    {
        if( index == IndexPatchContext.outsidePointIndexBadValue )
        {
            index = IndexPatchContext.outsidePointIndexReplacementValue;
        }
        else
        {
            index += IndexPatchContext.outsidePointIndexDeltaToRealValue;
        }
    }
    else
    {
        if( index == IndexPatchContext.insidePointIndexBadValue )
        {
            index = IndexPatchContext.insidePointIndexReplacementValue;
        }
        else
        {
            index += IndexPatchContext.insidePointIndexDeltaToRealValue;
        }
    }

    return vertices_base + index;
}


int AStitchRegular(bool bTrapezoid, int diagonals,
                                 uint numInsideEdgePoints,
                                 int2 outsideInsideEdgePointBaseOffset,
                                 int i)
{
    if (bTrapezoid)
    {
        ++ outsideInsideEdgePointBaseOffset.x;
    }

    int pt;

    if ((i < 4) && bTrapezoid)
    {
        if (i < 2)
        {
            pt = outsideInsideEdgePointBaseOffset.x - 1 + i; 
        }
        else if (i == 2)
        {
            pt = outsideInsideEdgePointBaseOffset.y;
        }
        else
        {
            pt = -1;
        }
    }

    int index = i;
    if (bTrapezoid)
    {
        index -= 4;
    }

    if (index >= 0)
    {
        uint uindex = (uint)index;
        
        switch( diagonals )
        {
        case DIAGONALS_INSIDE_TO_OUTSIDE:
            if (uindex < 5 * numInsideEdgePoints - 5)
            {
                uint p = uindex / 5;
                uint r = uindex - p * 5;
                if (r < 2)
                {
                    pt = outsideInsideEdgePointBaseOffset.x + p + r;
                }
                else if (r < 4)
                {
                    pt = outsideInsideEdgePointBaseOffset.y + p + r;
                }
                else
                {
                    pt = -1;
                }
            }
            else
            {
                int r = i - (4 + 5 * numInsideEdgePoints - 5);
                if (r < 2)
                {
                    pt = outsideInsideEdgePointBaseOffset.x + numInsideEdgePoints - 1 + r;
                }
                else if (r == 2)
                {
                    pt = outsideInsideEdgePointBaseOffset.y + numInsideEdgePoints - 1;
                }
                else
                {
                    pt = -1;
                }
            }
            break;

        case DIAGONALS_INSIDE_TO_OUTSIDE_EXCEPT_MIDDLE: // Assumes ODD tessellation
            if (uindex < (numInsideEdgePoints / 2 - 1) * 5)
            {
                // First half
                uint p = uindex / 5;
                uint r = uindex - p * 5;
                if (r < 2)
                {
                    pt = outsideInsideEdgePointBaseOffset.x + p + r;
                }
                else if (r < 4)
                {
                    pt = outsideInsideEdgePointBaseOffset.y + p;
                }
                else
                {
                    pt = -1;
                }
            }
            else if (uindex < (numInsideEdgePoints / 2 - 1) * 5 + 8)
            {
                // Middle
                uint r = uindex - (numInsideEdgePoints / 2 - 1) * 5;
                if (0 == r)
                {
                    pt = outsideInsideEdgePointBaseOffset.x + numInsideEdgePoints / 2 - 1;
                }
                else if (r < 3)
                {
                    pt = outsideInsideEdgePointBaseOffset.y + numInsideEdgePoints / 2 - 1 + (2 - r);
                }
                else if (r == 3)
                {
                    pt = -1;
                }
                else if (r < 6)
                {
                    pt = outsideInsideEdgePointBaseOffset.x + numInsideEdgePoints / 2 - 1 + (r - 4);
                }
                else if (r == 6)
                {
                    pt = outsideInsideEdgePointBaseOffset.y + numInsideEdgePoints / 2 - 1 + 1;
                }
                else if (r == 7)
                {
                    pt = -1;
                }
            }
            //else if (uindex < (numInsideEdgePoints/2-1) * 5 + 8 + (numInsideEdgePoints - numInsideEdgePoints/2 - 1) * 5)
            else if (uindex < numInsideEdgePoints * 5 - 2)
            {
                // Second half
                uint p = (uindex - (numInsideEdgePoints / 2 - 1) * 5 + 8) / 5 + numInsideEdgePoints / 2 + 1;
                uint r = uindex - (numInsideEdgePoints / 2 - 1) * 5 + 8 - (p - (numInsideEdgePoints / 2 + 1)) * 5;
                if (r < 2)
                {
                    pt = outsideInsideEdgePointBaseOffset.x + p - 1 + r;
                }
                else if (r < 4)
                {
                    pt = outsideInsideEdgePointBaseOffset.y + p - 1 + r;
                }
                else
                {
                    pt = -1;
                }
            }
            else
            {
                //int r = i - (4 + (numInsideEdgePoints/2-1) * 5 + 8 + (numInsideEdgePoints - numInsideEdgePoints/2 - 1) * 5);
                int r = i - (numInsideEdgePoints * 5 + 2);
                if (r < 2)
                {
                    pt = outsideInsideEdgePointBaseOffset.x + numInsideEdgePoints - 1 + r;
                }
                else if (r == 2)
                {
                    pt = outsideInsideEdgePointBaseOffset.y + numInsideEdgePoints - 1;
                }
                else
                {
                    pt = -1;
                }
            }
            break;

        case DIAGONALS_MIRRORED:
            if (uindex < (numInsideEdgePoints / 2 + 1) * 2)
            {
                uint p = uindex / 2;
                uint r = uindex - p * 2;
                if (0 == r)
                {
                    pt = outsideInsideEdgePointBaseOffset.y + p;
                }
                else
                {
                    pt = outsideInsideEdgePointBaseOffset.x + p;
                }
            }
            else if (uindex == (numInsideEdgePoints / 2 + 1) * 2)
            {
                pt = -1;
            }
            else if (uindex == (numInsideEdgePoints / 2 + 1) * 2 + 1)
            {
                pt = outsideInsideEdgePointBaseOffset.x + numInsideEdgePoints / 2;
            }
            //else if (uindex < (numInsideEdgePoints / 2 + 1) * 2 + 2 + (numInsideEdgePoints - numInsideEdgePoints / 2) * 2)
            else if (uindex < numInsideEdgePoints * 2 + 4)
            {
                uint p = (uindex - ((numInsideEdgePoints / 2 + 1) * 2 + 2)) / 2 + numInsideEdgePoints / 2;
                uint r = uindex - ((numInsideEdgePoints / 2 + 1) * 2 + 2) - (p - numInsideEdgePoints / 2) * 2;
                if (0 == r)
                {
                    pt = outsideInsideEdgePointBaseOffset.x + p;
                }
                else
                {
                    pt = outsideInsideEdgePointBaseOffset.y + p;
                }
            }
            //else if (uindex == (numInsideEdgePoints / 2 + 1) * 2 + 2 + (numInsideEdgePoints - numInsideEdgePoints / 2) * 2)
            else if (uindex == numInsideEdgePoints * 2 + 4)
            {
                pt = -1;
            }
            else
            {
                //int r = i - (4 + (numInsideEdgePoints / 2 + 1) * 2 + 2 + (numInsideEdgePoints - numInsideEdgePoints / 2) * 2 + 1);
                uint r = i - (numInsideEdgePoints * 2 + 9);
                if (r < 2)
                {
                    pt = outsideInsideEdgePointBaseOffset.x + numInsideEdgePoints - 1 + r;
                }
                else if (r == 2)
                {
                    pt = outsideInsideEdgePointBaseOffset.y + numInsideEdgePoints - 1;
                }
                else
                {
                    pt = -1;
                }
            }
            break;
        }
    }

    return pt;
}

int AStitchTransition(int2 outsideInsideEdgePointBaseOffset, int2 outsideInsideNumHalfTessFactorPoints, 
                                    int2 outsideInsideEdgeTessFactorParity,
                                    uint i)
{
    outsideInsideNumHalfTessFactorPoints -= (TESSELLATOR_PARITY_ODD == outsideInsideEdgeTessFactorParity);
    
    uint2 out_in_first_half = uint2(outsidePointIndex[outsideInsideNumHalfTessFactorPoints.x][MAX_FACTOR / 2 + 1].y, insidePointIndex[outsideInsideNumHalfTessFactorPoints.y][MAX_FACTOR / 2 + 1].y) * 4;

    uint3 out_in_middle = 0;
    if ((outsideInsideEdgeTessFactorParity.y != outsideInsideEdgeTessFactorParity.x) || (outsideInsideEdgeTessFactorParity.y == TESSELLATOR_PARITY_ODD))
    {
        if (outsideInsideEdgeTessFactorParity.y == outsideInsideEdgeTessFactorParity.x)
        {
            // Quad in the middle
            out_in_middle.z = 5;
            out_in_middle.xy = 1;
        }
        else if (TESSELLATOR_PARITY_EVEN == outsideInsideEdgeTessFactorParity.y)
        {
            // Triangle pointing inside
            out_in_middle.z = 4;
            out_in_middle.x = 1;
        }
        else
        {
            // Triangle pointing outside
            out_in_middle.z = 4;
            out_in_middle.y = 1;
        }
    }


    int pt = -1;

    if (i < out_in_first_half.y)
    {
        // Advance inside

        uint p = i / 4;
        uint r = i - p * 4;
        p = insidePointIndex[outsideInsideNumHalfTessFactorPoints.y][p].z;
        if ((0 == r) || (2 == r))
        {
            pt = outsideInsideEdgePointBaseOffset.y + insidePointIndex[outsideInsideNumHalfTessFactorPoints.y][p].y + r / 2;
        }
        else if (1 == r)
        {
            pt = outsideInsideEdgePointBaseOffset.x + outsidePointIndex[outsideInsideNumHalfTessFactorPoints.x][p].y;
        }
    }
    else
    {
        i -= out_in_first_half.y;
        
        if (i < out_in_first_half.x)
        {
            // Advance outside

            uint p = i / 4;
            uint r = i - p * 4;
            p = outsidePointIndex[outsideInsideNumHalfTessFactorPoints.x][p].z;
            if (r < 2)
            {
                pt = outsideInsideEdgePointBaseOffset.x + outsidePointIndex[outsideInsideNumHalfTessFactorPoints.x][p].y + r;
            }
            else if (r == 2)
            {
                pt = outsideInsideEdgePointBaseOffset.y + insidePointIndex[outsideInsideNumHalfTessFactorPoints.y][p].y;
                if (insidePointIndex[outsideInsideNumHalfTessFactorPoints.y][p].x)
                {
                    ++ pt;
                }
            }
        }
        else
        {
            i -= out_in_first_half.x;
            
            if (i < out_in_middle.z)
            {
                uint r = i;
                if (outsideInsideEdgeTessFactorParity.y == outsideInsideEdgeTessFactorParity.x)
                {
                    // Quad in the middle
                    if ((0 == r) || (2 == r))
                    {
                        pt = outsideInsideEdgePointBaseOffset.y + out_in_first_half.y / 4 + (2 == r);//r / 2;
                    }
                    else if ((1 == r) || (3 == r))
                    {
                        pt = outsideInsideEdgePointBaseOffset.x + out_in_first_half.x / 4 + (3 == r);//(r - 1) / 2;
                    }
                }
                else if (TESSELLATOR_PARITY_EVEN == outsideInsideEdgeTessFactorParity.y)
                {
                    // Triangle pointing inside
                    if (r == 0)
                    {
                        pt = outsideInsideEdgePointBaseOffset.y + out_in_first_half.y / 4;
                    }
                    else if (r < 3)
                    {
                        pt = outsideInsideEdgePointBaseOffset.x + out_in_first_half.x / 4 + r - 1;
                    }
                }
                else
                {
                    // Triangle pointing outside
                    if ((0 == r) || (2 == r))
                    {
                        pt = outsideInsideEdgePointBaseOffset.y + out_in_first_half.y / 4 + (2 == r);//r / 2;
                    }
                    else if (1 == r)
                    {
                        pt = outsideInsideEdgePointBaseOffset.x + out_in_first_half.x / 4;
                    }
                }
            }
            else
            {
                i -= out_in_middle.z;
                
                if (i < out_in_first_half.x)
                {
                    // Advance outside

                    uint p = i / 4;
                    uint r = i - p * 4;
                    p = outsidePointIndex[outsideInsideNumHalfTessFactorPoints.x][p].z;
                    if (r < 2)
                    {
                        pt = outsideInsideEdgePointBaseOffset.x + out_in_first_half.x / 4 + out_in_middle.x + (outsidePointIndex[outsideInsideNumHalfTessFactorPoints.x][MAX_FACTOR / 2 + 1].y - outsidePointIndex[outsideInsideNumHalfTessFactorPoints.x][p + 1].y) + r;
                    }
                    else if (r == 2)
                    {
                        pt = outsideInsideEdgePointBaseOffset.y + out_in_first_half.y / 4 + out_in_middle.y + (insidePointIndex[outsideInsideNumHalfTessFactorPoints.y][MAX_FACTOR / 2 + 1].y - insidePointIndex[outsideInsideNumHalfTessFactorPoints.y][p + 1].y);
                    }
                }
                else
                {
                    // Advance inside
                    
                    i -= out_in_first_half.x;

                    uint p = i / 4;
                    uint r = i - p * 4;
                    p = insidePointIndex[outsideInsideNumHalfTessFactorPoints.y][p].w;
                    if ((0 == r) || (2 == r))
                    {
                        pt = outsideInsideEdgePointBaseOffset.y + out_in_first_half.y / 4 + out_in_middle.y
                            + (insidePointIndex[outsideInsideNumHalfTessFactorPoints.y][MAX_FACTOR / 2 + 1].y - insidePointIndex[outsideInsideNumHalfTessFactorPoints.y][p + 1].y) + (2 == r);//r / 2;
                    }
                    else if (1 == r)
                    {
                        pt = outsideInsideEdgePointBaseOffset.x + out_in_first_half.x / 4 + out_in_middle.x
                            + (outsidePointIndex[outsideInsideNumHalfTessFactorPoints.x][MAX_FACTOR / 2 + 1].y - outsidePointIndex[outsideInsideNumHalfTessFactorPoints.x][p + 1].y);
                        if (outsidePointIndex[outsideInsideNumHalfTessFactorPoints.x][p].x)
                        {
                            ++ pt;
                        }
                    }
                }
            }
        }
    }

    return pt;
}

[numthreads(128, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID, uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex )
{
    uint id = DTid.x;
    //uint id = Gid.x * 128 + GI; // Workaround for some CS4x preview drivers
    
    if ( id < g_param.x )
    {
        uint tri_id = InputTriIDIndexID[id].x;
        uint index_id = InputTriIDIndexID[id].y;
        uint base_vertex = InputScanned[tri_id-1].x;
        
        float4 outside_inside_factor = InputEdgeFactor[tri_id];
        
        PROCESSED_TESS_FACTORS_TRI processedTessFactors;
        int num_points = TriProcessTessFactors(outside_inside_factor, processedTessFactors, g_partitioning);

        uint tessed_indices;
        if (3 == num_points)
        {
            if (index_id < 3)
            {
                tessed_indices = TransformIndex1(index_id, base_vertex);
            }
            else
            {
                tessed_indices = -1;
            }
        }
        else
        {
            // Generate primitives for all the concentric rings, one side at a time for each ring
            static const int startRing = 1;
            int numRings = ((processedTessFactors.numPointsForOutsideInside.w + 1) / 2); // +1 is so even tess includes the center point, which we want to now

            int4 outsideInsideHalfTessFactor = int4(ceil(processedTessFactors.outsideInsideHalfTessFactor));
            uint3 num = NumStitchTransition(outsideInsideHalfTessFactor, processedTessFactors.outsideInsideTessFactorParity);
            num.y += num.x;
            num.z += num.y;
            uint num_index = num.z;
            num_index += TotalNumStitchRegular(true, DIAGONALS_MIRRORED, processedTessFactors.numPointsForOutsideInside.w, numRings - 1) * 3;
            if( processedTessFactors.outsideInsideTessFactorParity.w == TESSELLATOR_PARITY_ODD )
            {
                num_index += 4;
            }

            int pt;

            if (index_id < num.x)
            {
                int numPointsForInsideEdge = processedTessFactors.numPointsForOutsideInside.w - 2 * startRing;

                pt = AStitchTransition(int2(0, processedTessFactors.insideEdgePointBaseOffset),
                        outsideInsideHalfTessFactor.xw,
                        processedTessFactors.outsideInsideTessFactorParity.xw,
                        index_id);
                if (pt != -1)
                {
                    pt = TransformIndex1(pt, base_vertex);
                }
            }
            else if (index_id < num.y)
            {
                int numPointsForInsideEdge = processedTessFactors.numPointsForOutsideInside.w - 2 * startRing;

                pt = AStitchTransition(
                        int2(processedTessFactors.numPointsForOutsideInside.x - 1, processedTessFactors.insideEdgePointBaseOffset + numPointsForInsideEdge - 1),
                        outsideInsideHalfTessFactor.yw,
                        processedTessFactors.outsideInsideTessFactorParity.yw,
                        index_id - num.x);
                if (pt != -1)
                {
                    pt = TransformIndex1(pt, base_vertex);
                }
            }
            else if (index_id < num.z)
            {
                int numPointsForInsideEdge = processedTessFactors.numPointsForOutsideInside.w - 2 * startRing;

                INDEX_PATCH_CONTEXT IndexPatchContext;
                IndexPatchContext.insidePointIndexDeltaToRealValue    = processedTessFactors.insideEdgePointBaseOffset + 2 * (numPointsForInsideEdge - 1);
                IndexPatchContext.insidePointIndexBadValue            = numPointsForInsideEdge - 1;
                IndexPatchContext.insidePointIndexReplacementValue    = processedTessFactors.insideEdgePointBaseOffset;
                IndexPatchContext.outsidePointIndexPatchBase          = IndexPatchContext.insidePointIndexBadValue+1; // past inside patched index range
                IndexPatchContext.outsidePointIndexDeltaToRealValue   = processedTessFactors.numPointsForOutsideInside.x + processedTessFactors.numPointsForOutsideInside.y - 2 
                                                                                    - IndexPatchContext.outsidePointIndexPatchBase;
                IndexPatchContext.outsidePointIndexBadValue           = IndexPatchContext.outsidePointIndexPatchBase
                                                                                    + processedTessFactors.numPointsForOutsideInside.z - 1;
                IndexPatchContext.outsidePointIndexReplacementValue   = 0;

                pt = AStitchTransition(int2(numPointsForInsideEdge, 0),
                            outsideInsideHalfTessFactor.zw,
                            processedTessFactors.outsideInsideTessFactorParity.zw,
                            index_id - num.y);
                if (pt != -1)
                {
                    pt = TransformIndex2(pt, base_vertex, IndexPatchContext);
                }
            }
            else
            {
                if ((processedTessFactors.outsideInsideTessFactorParity.w == TESSELLATOR_PARITY_ODD) && (index_id >= num_index - 4))
                {
                    int outsideEdgePointBaseOffset = processedTessFactors.insideEdgePointBaseOffset
                        + ((processedTessFactors.numPointsForOutsideInside.w + 1) - (numRings + startRing)) * (numRings - startRing - 1) * 3;

                    if (index_id - (num_index - 4) != 3)
                    {
                        pt = TransformIndex1(outsideEdgePointBaseOffset + index_id - (num_index - 4), base_vertex);
                    }
                    else
                    {
                        pt = -1;
                    }
                }
                else
                {
                    int ring = GetRingFromIndexStitchRegular(true, DIAGONALS_MIRRORED, processedTessFactors.numPointsForOutsideInside.w, index_id - num.z);

                    int tn = TotalNumStitchRegular(true, DIAGONALS_MIRRORED, processedTessFactors.numPointsForOutsideInside.w, ring - 1) * 3;
                    int n = NumStitchRegular(true, DIAGONALS_MIRRORED, processedTessFactors.numPointsForOutsideInside.w - 2 * ring);

                    int edge = (index_id - num.z - tn) / n;
                    int index = (index_id - num.z - tn) - edge * n;

                    int2 outsideInsideEdgePointBaseOffset = processedTessFactors.insideEdgePointBaseOffset
                        + int2(0, 3 * (processedTessFactors.numPointsForOutsideInside.w - 3))
                        + ((processedTessFactors.numPointsForOutsideInside.w - (ring + startRing)) + int2(1, -1)) * (ring - startRing - 1) * 3;

                    int numPointsForInsideEdge = processedTessFactors.numPointsForOutsideInside.w - 2 * ring;
                    int numLastPointsForInsideEdge = numPointsForInsideEdge + 2;

                    if (edge < 2)
                    {
                        pt = AStitchRegular(true, DIAGONALS_MIRRORED,
                                    numPointsForInsideEdge,
                                    outsideInsideEdgePointBaseOffset + (int2(numLastPointsForInsideEdge, numPointsForInsideEdge) - 1) * edge,
                                    index);
                        if (pt != -1)
                        {
                            pt = TransformIndex1(pt, base_vertex);
                        }
                    }
                    else
                    {
                        INDEX_PATCH_CONTEXT IndexPatchContext;
                        IndexPatchContext.insidePointIndexDeltaToRealValue    = outsideInsideEdgePointBaseOffset.y + (numPointsForInsideEdge - 1) * 2;
                        IndexPatchContext.insidePointIndexBadValue            = numPointsForInsideEdge - 1;
                        IndexPatchContext.insidePointIndexReplacementValue    = outsideInsideEdgePointBaseOffset.y;
                        IndexPatchContext.outsidePointIndexPatchBase          = IndexPatchContext.insidePointIndexBadValue+1; // past inside patched index range
                        IndexPatchContext.outsidePointIndexDeltaToRealValue   = outsideInsideEdgePointBaseOffset.x + (numLastPointsForInsideEdge - 1) * 2 
                                                                                    - IndexPatchContext.outsidePointIndexPatchBase;
                        IndexPatchContext.outsidePointIndexBadValue           = IndexPatchContext.outsidePointIndexPatchBase
                                                                                    + numLastPointsForInsideEdge - 1;
                        IndexPatchContext.outsidePointIndexReplacementValue   = outsideInsideEdgePointBaseOffset.x;

                        pt = AStitchRegular(true, DIAGONALS_MIRRORED,
                                        numPointsForInsideEdge,
                                        int2(numPointsForInsideEdge, 0),
                                        index);
                        if (pt != -1)
                        {
                            pt = TransformIndex2(pt, base_vertex, IndexPatchContext);
                        }
                    }
                }
            }

            tessed_indices = pt;
        }

        DebugOutput[DTid.x] = processedTessFactors.outsideInsideSplitPointOnFloorHalfTessFactor; // HLSL Change: Use that output so the loop that defines it doens't get deleted.
        TessedIndicesOut.Store(id*4, tessed_indices);
    }       
}

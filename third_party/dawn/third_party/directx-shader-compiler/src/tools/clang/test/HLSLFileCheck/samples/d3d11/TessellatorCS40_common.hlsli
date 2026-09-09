//--------------------------------------------------------------------------------------
// File: TessellatorCS40_common.hlsl
//
// The common utils included by other shaders in the sample
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "TessellatorCS40_defines.h"

cbuffer cbNeverChanges : register(b0)
{
    uint4 insidePointIndex[MAX_FACTOR / 2 + 1][MAX_FACTOR / 2 + 2];
    uint4 outsidePointIndex[MAX_FACTOR / 2 + 1][MAX_FACTOR / 2 + 2];
}

#define D3D11_TESSELLATOR_MAX_EVEN_TESSELLATION_FACTOR    ( 64 )
#define D3D11_TESSELLATOR_MAX_ODD_TESSELLATION_FACTOR     ( 63 )
#define D3D11_TESSELLATOR_MIN_EVEN_TESSELLATION_FACTOR     ( 2 )
#define D3D11_TESSELLATOR_MIN_ODD_TESSELLATION_FACTOR      ( 1 )

#define D3D11_TESSELLATOR_PARTITIONING_INTEGER            ( 0 )
#define D3D11_TESSELLATOR_PARTITIONING_POW2               ( 1 )
#define D3D11_TESSELLATOR_PARTITIONING_FRACTIONAL_ODD     ( 2 )
#define D3D11_TESSELLATOR_PARTITIONING_FRACTIONAL_EVEN    ( 3 )
    
#define TESSELLATOR_PARITY_EVEN                           ( 0 )
#define TESSELLATOR_PARITY_ODD                            ( 1 )

#define EPSILON 1e-6f
#define MIN_ODD_TESSFACTOR_PLUS_HALF_EPSILON (D3D11_TESSELLATOR_MIN_ODD_TESSELLATION_FACTOR + EPSILON/2)

#define DIAGONALS_INSIDE_TO_OUTSIDE                       ( 0 )
#define DIAGONALS_INSIDE_TO_OUTSIDE_EXCEPT_MIDDLE         ( 1 )
#define DIAGONALS_MIRRORED                                ( 2 )


// This is moved to macro defines at shader compile time, so that the partitioning mode can be changed during runtime
// #define g_partitioning (D3D11_TESSELLATOR_PARTITIONING_POW2)
uint g_partitioning;

struct PROCESSED_TESS_FACTORS_TRI
{
    float4 outsideInsideTessFactor;
    int4 outsideInsideTessFactorParity;

    float4 outsideInsideInvNumSegmentsOnFloorTessFactor; 
    float4 outsideInsideInvNumSegmentsOnCeilTessFactor;
    float4 outsideInsideHalfTessFactor;
    int4 outsideInsideSplitPointOnFloorHalfTessFactor; 

    // Stuff below is specific to the traversal order 
    uint4 numPointsForOutsideInside;
    uint insideEdgePointBaseOffset;
};

struct INDEX_PATCH_CONTEXT
{
    int insidePointIndexDeltaToRealValue;
    int insidePointIndexBadValue;
    int insidePointIndexReplacementValue;
    int outsidePointIndexPatchBase;
    int outsidePointIndexDeltaToRealValue;
    int outsidePointIndexBadValue;
    int outsidePointIndexReplacementValue;
};

bool4 isEven(float4 input)
{
    return (((uint4)input) & 1) ? false : true;
}

uint RemoveMSB(uint val)
{
    int check;
    if( val <= 0x0000ffff )
    {
        check = ( val <= 0x000000ff ) ? 0x00000080 : 0x00008000;
    }
    else
    {
        check = ( val <= 0x00ffffff ) ? 0x00800000 : 0x80000000;
    }
    for (int i = 0; i < 8; i++, check >>= 1)
    {
        if( val & check )
        {
            return (val & ~check);
        }
    }
    return 0;
}

uint4 NumPointsForTessFactor(float4 tessFactor, int4 parity)
{
    return TESSELLATOR_PARITY_ODD == parity ? uint4(ceil(0.5f + tessFactor / 2)) * 2 : uint4(ceil(tessFactor / 2)) * 2 + 1;
}

void ComputeTessFactorContext(float4 tessFactor, int4 parity,
    out float4 invNumSegmentsOnFloorTessFactor,
    out float4 invNumSegmentsOnCeilTessFactor,
    out float4 halfTessFactor,
    out int4 splitPointOnFloorHalfTessFactor)
{
    halfTessFactor = tessFactor / 2;
    
    halfTessFactor += 0.5 * ((TESSELLATOR_PARITY_ODD == parity) | (0.5f == halfTessFactor));
    
    float4 floorHalfTessFactor = floor(halfTessFactor);
    float4 ceilHalfTessFactor = ceil(halfTessFactor);
    int4 numHalfTessFactorPoints = int4(ceilHalfTessFactor);
    
    for (int index = 0; index < 4; ++ index)
    {
        if( ceilHalfTessFactor[index] == floorHalfTessFactor[index] )
        {
            splitPointOnFloorHalfTessFactor[index] =  /*pick value to cause this to be ignored*/ numHalfTessFactorPoints[index]+1;
        }
        else if( TESSELLATOR_PARITY_ODD == parity[index] )
        {
            if( floorHalfTessFactor[index] == 1 )
            {
                splitPointOnFloorHalfTessFactor[index] = 0;
            }
            else
            {
                splitPointOnFloorHalfTessFactor[index] = (RemoveMSB(int(floorHalfTessFactor[index]) - 1) << 1) + 1;
            }
        }
        else
        {
            splitPointOnFloorHalfTessFactor[index] = (RemoveMSB(int(floorHalfTessFactor[index])) << 1) + 1;
        }
    }
    
    int4 numFloorSegments = int4(floorHalfTessFactor * 2);
    int4 numCeilSegments = int4(ceilHalfTessFactor * 2);
    int4 s = (TESSELLATOR_PARITY_ODD == parity);
    numFloorSegments -= s;
    numCeilSegments -= s;
    invNumSegmentsOnFloorTessFactor = 1.0f / numFloorSegments;
    invNumSegmentsOnCeilTessFactor = 1.0f / numCeilSegments;
}

int TriProcessTessFactors( inout float4 tessFactor,
                           out PROCESSED_TESS_FACTORS_TRI processedTessFactors,
                           int partitioning )
{
    processedTessFactors = (PROCESSED_TESS_FACTORS_TRI)0;
    
    int parity = TESSELLATOR_PARITY_EVEN;
    switch( partitioning )
    {
        case D3D11_TESSELLATOR_PARTITIONING_INTEGER:
        default:
            break;
        case D3D11_TESSELLATOR_PARTITIONING_FRACTIONAL_ODD:
            parity = TESSELLATOR_PARITY_ODD;
            break;
        case D3D11_TESSELLATOR_PARTITIONING_FRACTIONAL_EVEN:
            parity = TESSELLATOR_PARITY_EVEN;
            break;
    }

    // Is the patch culled?
    if( !(tessFactor.x > 0) || // NaN will pass
        !(tessFactor.y > 0) ||
        !(tessFactor.z > 0) )
    {
        return 0;
    }

    // Clamp edge TessFactors
    float lowerBound, upperBound;
    switch(partitioning)
    {
        case D3D11_TESSELLATOR_PARTITIONING_INTEGER:
        case D3D11_TESSELLATOR_PARTITIONING_POW2: // don't care about pow2 distinction for validation, just treat as integer
        default:
            lowerBound = D3D11_TESSELLATOR_MIN_ODD_TESSELLATION_FACTOR;
            upperBound = D3D11_TESSELLATOR_MAX_EVEN_TESSELLATION_FACTOR;
            break;
         
        case D3D11_TESSELLATOR_PARTITIONING_FRACTIONAL_EVEN:
            lowerBound = D3D11_TESSELLATOR_MIN_EVEN_TESSELLATION_FACTOR;
            upperBound = D3D11_TESSELLATOR_MAX_EVEN_TESSELLATION_FACTOR;
            break;

        case D3D11_TESSELLATOR_PARTITIONING_FRACTIONAL_ODD:
            lowerBound = D3D11_TESSELLATOR_MIN_ODD_TESSELLATION_FACTOR;
            upperBound = D3D11_TESSELLATOR_MAX_ODD_TESSELLATION_FACTOR;
            break;
    }

    tessFactor.xyz = min( upperBound, max( lowerBound, tessFactor.xyz ) );

    // Clamp inside TessFactors
    if(D3D11_TESSELLATOR_PARTITIONING_FRACTIONAL_ODD == partitioning)
    {
        if( (tessFactor.x > MIN_ODD_TESSFACTOR_PLUS_HALF_EPSILON) ||
            (tessFactor.y > MIN_ODD_TESSFACTOR_PLUS_HALF_EPSILON) ||
            (tessFactor.z > MIN_ODD_TESSFACTOR_PLUS_HALF_EPSILON)) 
            // Don't need the same check for insideTessFactor for tri patches, 
            // since there is only one insideTessFactor, as opposed to quad 
            // patches which have 2 insideTessFactors.
        {
            // Force picture frame
            lowerBound = D3D11_TESSELLATOR_MIN_ODD_TESSELLATION_FACTOR + EPSILON;
        }
    }

    tessFactor.w = min( upperBound, max( lowerBound, tessFactor.w ) );
    // Note the above clamps map NaN to lowerBound

    if (partitioning == D3D11_TESSELLATOR_PARTITIONING_INTEGER)
    {
        tessFactor = ceil(tessFactor);
    }
    else if (partitioning == D3D11_TESSELLATOR_PARTITIONING_POW2)
    {
        static const int exponentMask = 0x7f800000;
        static const int mantissaMask = 0x007fffff;
        static const int exponentLSB = 0x00800000;

        int4 bits = asint(tessFactor);
        tessFactor = bits & mantissaMask ? asfloat((bits & exponentMask) + exponentLSB) : tessFactor;
    }

    // Process tessFactors
    if ((partitioning == D3D11_TESSELLATOR_PARTITIONING_INTEGER)|| (partitioning == D3D11_TESSELLATOR_PARTITIONING_POW2))
    {
        bool4 e = isEven(tessFactor);
        processedTessFactors.outsideInsideTessFactorParity.xyz = e.xyz ? TESSELLATOR_PARITY_EVEN : TESSELLATOR_PARITY_ODD;
        processedTessFactors.outsideInsideTessFactorParity.w = (e.w || (1 == tessFactor.w)) ? TESSELLATOR_PARITY_EVEN : TESSELLATOR_PARITY_ODD;
    }
    else
    {
        processedTessFactors.outsideInsideTessFactorParity = parity;
    }
    
    processedTessFactors.outsideInsideTessFactor = tessFactor;

    if (((partitioning == D3D11_TESSELLATOR_PARTITIONING_INTEGER)|| (partitioning == D3D11_TESSELLATOR_PARTITIONING_POW2)) || (parity == TESSELLATOR_PARITY_ODD))
    {
        // Special case if all TessFactors are 1 
        if( (1 == processedTessFactors.outsideInsideTessFactor.x) &&
            (1 == processedTessFactors.outsideInsideTessFactor.y) &&
            (1 == processedTessFactors.outsideInsideTessFactor.z) &&
            (1 == processedTessFactors.outsideInsideTessFactor.w) )
        {
            return 3;
        }
    }

    // Compute per-TessFactor metadata
    ComputeTessFactorContext(processedTessFactors.outsideInsideTessFactor, processedTessFactors.outsideInsideTessFactorParity,
                             processedTessFactors.outsideInsideInvNumSegmentsOnFloorTessFactor,
                             processedTessFactors.outsideInsideInvNumSegmentsOnCeilTessFactor,
                             processedTessFactors.outsideInsideHalfTessFactor,
                             processedTessFactors.outsideInsideSplitPointOnFloorHalfTessFactor);

    // Compute some initial data.

    // outside edge offsets and storage
    processedTessFactors.numPointsForOutsideInside = NumPointsForTessFactor(processedTessFactors.outsideInsideTessFactor, processedTessFactors.outsideInsideTessFactorParity);
    int NumPoints = processedTessFactors.numPointsForOutsideInside.x + processedTessFactors.numPointsForOutsideInside.y + processedTessFactors.numPointsForOutsideInside.z - 3;

    // inside edge offsets
    {
        uint pointCountMin = (processedTessFactors.outsideInsideTessFactorParity.w == TESSELLATOR_PARITY_ODD) ? 4 : 3;
        // max() allows degenerate transition regions when inside TessFactor == 1
        processedTessFactors.numPointsForOutsideInside.w = max(pointCountMin, processedTessFactors.numPointsForOutsideInside.w);
    }

    processedTessFactors.insideEdgePointBaseOffset = NumPoints;

    // inside storage, including interior edges above
    {
        int numInteriorRings = (processedTessFactors.numPointsForOutsideInside.w >> 1) - 1; 
        int numInteriorPoints;
        if( processedTessFactors.outsideInsideTessFactorParity.w == TESSELLATOR_PARITY_ODD )
        {
            numInteriorPoints = 3*(numInteriorRings*(numInteriorRings+1) - numInteriorRings);
        }
        else
        {
            numInteriorPoints = 3*(numInteriorRings*(numInteriorRings+1)) + 1;
        }
        NumPoints += numInteriorPoints;
    }
    
    return NumPoints;
}

int NumStitchRegular(bool bTrapezoid, int diagonals, int numInsideEdgePoints)
{
    int num_index = 0;

    if( bTrapezoid )
    {
        num_index += 8;
    }
    switch( diagonals )
    {
        case DIAGONALS_INSIDE_TO_OUTSIDE:
            // Diagonals pointing from inside edge forward towards outside edge
            num_index += 5 * numInsideEdgePoints - 5;
            break;

        case DIAGONALS_INSIDE_TO_OUTSIDE_EXCEPT_MIDDLE: // Assumes ODD tessellation
            // Diagonals pointing from outside edge forward towards inside edge
            num_index += 5 * numInsideEdgePoints - 2;
            break;

        case DIAGONALS_MIRRORED:
            num_index += 2 * numInsideEdgePoints + 5;
            break;
    }

    return num_index;
}

uint TotalNumStitchRegular(bool bTrapezoid, int diagonals,
                                 int numPointsForInsideTessFactor, int ring)
{
    uint num_index = 0;

    if( bTrapezoid )
    {
        num_index += 8 * (ring - 1);
    }
    switch( diagonals )
    {
        case DIAGONALS_INSIDE_TO_OUTSIDE:
            // Diagonals pointing from inside edge forward towards outside edge
            num_index += (5 * numPointsForInsideTessFactor - 35 - 5 * ring) * (ring - 1);
            break;

        case DIAGONALS_INSIDE_TO_OUTSIDE_EXCEPT_MIDDLE: // Assumes ODD tessellation
            // Diagonals pointing from outside edge forward towards inside edge
            num_index += (5 * numPointsForInsideTessFactor - 12 - 5 * ring) * (ring - 1);
            break;

        case DIAGONALS_MIRRORED:
            num_index += (2 * numPointsForInsideTessFactor + 1 - 2 * ring) * (ring - 1);
            break;
    }

    return num_index;
}

int sqr(int x)
{
    return x * x;
}

int GetRingFromIndexStitchRegular(bool bTrapezoid, int diagonals, int numPointsForInsideTessFactor, int index)
{
    int t = 0;
    if (bTrapezoid)
    {
        t = 8;
    }

    switch( diagonals )
    {
        case DIAGONALS_INSIDE_TO_OUTSIDE:
            t = (5 * numPointsForInsideTessFactor - (35 - t)) * 3;
            return 1 + uint((t + 15) - sqrt(sqr(t + 15) - 4 * 15 * (t + index)) + 0.001f) / 30;

        case DIAGONALS_INSIDE_TO_OUTSIDE_EXCEPT_MIDDLE:
            t = (5 * numPointsForInsideTessFactor - (12 - t)) * 3;
            return 1 + uint((t + 15) - sqrt(sqr(t + 15) - 4 * 15 * (t + index)) + 0.001f) / 30;

        case DIAGONALS_MIRRORED:
            t = ((t + 1) + 2 * numPointsForInsideTessFactor) * 3;
            return 1 + uint((t + 6) - sqrt(sqr(t + 6) - 4 * 6 * (t + index)) + 0.001f) / 12;

        default:
            return -1;
    }
}

uint3 NumStitchTransition(int4 outsideInsideNumHalfTessFactorPoints, 
                                    int4 outsideInsideEdgeTessFactorParity)
{
    outsideInsideNumHalfTessFactorPoints -= (TESSELLATOR_PARITY_ODD == outsideInsideEdgeTessFactorParity);

    uint3 num_index = insidePointIndex[outsideInsideNumHalfTessFactorPoints.w][MAX_FACTOR / 2 + 1].y * 8;
    
    [unroll]
    for (int edge = 0; edge < 3; ++ edge)
    {
        num_index[edge] += outsidePointIndex[outsideInsideNumHalfTessFactorPoints[edge]][MAX_FACTOR / 2 + 1].y * 8;

        if( (outsideInsideEdgeTessFactorParity.w != outsideInsideEdgeTessFactorParity[edge]) || (outsideInsideEdgeTessFactorParity.w == TESSELLATOR_PARITY_ODD))
        {
            if( outsideInsideEdgeTessFactorParity.w == outsideInsideEdgeTessFactorParity[edge] )
            {
                num_index[edge] += 5;
            }
            else
            {
                num_index[edge] += 4;
            }
        }
    }

    return num_index;
}

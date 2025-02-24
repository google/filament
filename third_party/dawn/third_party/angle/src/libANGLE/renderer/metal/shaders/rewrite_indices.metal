//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// rewrite_indices.metal:
//    Contains utility methods for rewriting indices for provoking vertex usecases.
//

#include "common.h"
#include "rewrite_indices_shared.h"
using namespace metal;

constant uint fixIndexBufferKey [[ function_constant(2000) ]];
constant bool indexBufferIsUint16 = (((fixIndexBufferKey >> MtlFixIndexBufferKeyInShift) & MtlFixIndexBufferKeyTypeMask) == MtlFixIndexBufferKeyUint16);
constant bool indexBufferIsUint32 = (((fixIndexBufferKey >> MtlFixIndexBufferKeyInShift) & MtlFixIndexBufferKeyTypeMask) == MtlFixIndexBufferKeyUint32);
constant bool outIndexBufferIsUint16 = (((fixIndexBufferKey >> MtlFixIndexBufferKeyOutShift) & MtlFixIndexBufferKeyTypeMask) == MtlFixIndexBufferKeyUint16);
constant bool outIndexBufferIsUint32 = (((fixIndexBufferKey >> MtlFixIndexBufferKeyOutShift) & MtlFixIndexBufferKeyTypeMask) == MtlFixIndexBufferKeyUint32);
constant bool doPrimRestart = (fixIndexBufferKey & MtlFixIndexBufferKeyPrimRestart);
constant uint fixIndexBufferMode = (fixIndexBufferKey >> MtlFixIndexBufferKeyModeShift) & MtlFixIndexBufferKeyModeMask;


static inline uint readIdx(
                           const device ushort *indexBufferUint16,
                           const device uint   *indexBufferUint32,
                           const uint restartIndex,
                           const uint indexCount,
                           uint idx,
                           thread bool &foundRestart,
                           thread uint &indexThatRestartedFirst
                           )
{
    uint inIndex = idx;
    if(inIndex < indexCount)
    {
        if(indexBufferIsUint16)
        {
            inIndex = indexBufferUint16[inIndex];
        }
        else if(indexBufferIsUint32)
        {
            inIndex = indexBufferUint32[inIndex];
        }
    }
    else
    {
        foundRestart = true;
        indexThatRestartedFirst = idx;
    }
    if(doPrimRestart && !foundRestart && inIndex == restartIndex)
    {
        foundRestart = true;
        indexThatRestartedFirst = idx;
    }
    return inIndex;
}

static inline void outputPrimitive(
                                   const device ushort *indexBufferUint16,
                                   const device uint   *indexBufferUint32,
                                   device ushort *outIndexBufferUint16,
                                   device uint   *outIndexBufferUint32,
                                   const uint restartIndex,
                                   const uint indexCount,
                                   thread uint &baseIndex,
                                   uint onIndex,
                                   thread uint &onOutIndex
                                   )
{
    if(baseIndex > onIndex) return; // skipped indices while processing
    bool foundRestart = false;
    uint indexThatRestartedFirst = 0;
#define READ_IDX(_idx) readIdx(indexBufferUint16, indexBufferUint32, restartIndex, indexCount, _idx, foundRestart, indexThatRestartedFirst)
#define WRITE_IDX(_idx, _val) \
({ \
    if(outIndexBufferIsUint16) \
    { \
        outIndexBufferUint16[(_idx)] = _val; \
    } \
    if(outIndexBufferIsUint32) \
    { \
        outIndexBufferUint32[(_idx)] = _val; \
    } \
    _idx++; \
})
    switch(fixIndexBufferMode)
    {
        case MtlFixIndexBufferKeyPoints:
        {
            auto tmpIndex = READ_IDX(onIndex);
            if(foundRestart)
            {
                baseIndex = indexThatRestartedFirst + 1;
                return;
            }

            WRITE_IDX(onOutIndex, tmpIndex);
        }
        break;
        case MtlFixIndexBufferKeyLines:
        {
            auto tmpIndex0 = READ_IDX(onIndex + 0);
            auto tmpIndex1 = READ_IDX(onIndex + 1);
            if(foundRestart)
            {
                baseIndex = indexThatRestartedFirst + 1;
                return;
            }
            if((onIndex - baseIndex) & 1) return; // skip this index...

            if(fixIndexBufferKey & MtlFixIndexBufferKeyProvokingVertexLast)
            {
                WRITE_IDX(onOutIndex, tmpIndex1);
                WRITE_IDX(onOutIndex, tmpIndex0);
            }
            else
            {
                WRITE_IDX(onOutIndex, tmpIndex0);
                WRITE_IDX(onOutIndex, tmpIndex1);
            }
        }
        break;
        case MtlFixIndexBufferKeyLineStrip:
        {
            auto tmpIndex0 = READ_IDX(onIndex + 0);
            auto tmpIndex1 = READ_IDX(onIndex + 1);
            if(foundRestart)
            {
                baseIndex = indexThatRestartedFirst + 1;
                return;
            }

            if(fixIndexBufferKey & MtlFixIndexBufferKeyProvokingVertexLast)
            {
                WRITE_IDX(onOutIndex, tmpIndex1);
                WRITE_IDX(onOutIndex, tmpIndex0);
            }
            else
            {
                WRITE_IDX(onOutIndex, tmpIndex0);
                WRITE_IDX(onOutIndex, tmpIndex1);
            }
        }
        break;
        case MtlFixIndexBufferKeyTriangles:
        {
            auto tmpIndex0 = READ_IDX(onIndex + 0);
            auto tmpIndex1 = READ_IDX(onIndex + 1);
            auto tmpIndex2 = READ_IDX(onIndex + 2);
            if(foundRestart)
            {
                baseIndex = indexThatRestartedFirst + 1;
                return;
            }
            if(((onIndex - baseIndex) % 3) != 0) return; // skip this index...

            if(fixIndexBufferKey & MtlFixIndexBufferKeyProvokingVertexLast)
            {
                WRITE_IDX(onOutIndex, tmpIndex2);
                WRITE_IDX(onOutIndex, tmpIndex0);
                WRITE_IDX(onOutIndex, tmpIndex1);
            }
            else
            {
                WRITE_IDX(onOutIndex, tmpIndex0);
                WRITE_IDX(onOutIndex, tmpIndex1);
                WRITE_IDX(onOutIndex, tmpIndex2);
            }
        }
        break;
        case MtlFixIndexBufferKeyTriangleStrip:
        {
            uint isOdd = ((onIndex - baseIndex) & 1); // fixes winding (but not provoking...)
            auto tmpIndex0 = READ_IDX(onIndex + 0 + isOdd);
            auto tmpIndex1 = READ_IDX(onIndex + 1 - isOdd);
            auto tmpIndex2 = READ_IDX(onIndex + 2);
            if(foundRestart)
            {
                baseIndex = indexThatRestartedFirst + 1;
                return;
            }

            if(fixIndexBufferKey & MtlFixIndexBufferKeyProvokingVertexLast)
            {
                WRITE_IDX(onOutIndex, tmpIndex2); // 2 is always the provoking vertex .: do not need to do anything special with isOdd
                WRITE_IDX(onOutIndex, tmpIndex0);
                WRITE_IDX(onOutIndex, tmpIndex1);
            }
            else
            {
                // NOTE: this case is trivially supported in Metal
                if(isOdd)
                {
                    WRITE_IDX(onOutIndex, tmpIndex1); // in the case of odd this is REALLY (onIndex + 0) // provoking vertex
                    WRITE_IDX(onOutIndex, tmpIndex2);
                    WRITE_IDX(onOutIndex, tmpIndex0);
                }
                else
                {
                    WRITE_IDX(onOutIndex, tmpIndex0); // in the case of even this is (onIndex + 0) // provoking vertex
                    WRITE_IDX(onOutIndex, tmpIndex1);
                    WRITE_IDX(onOutIndex, tmpIndex2);
                }
            }
            // assert never worse that worst-case expansion
            assert(onOutIndex <= (onIndex + 1) * 3);
            assert(onOutIndex <= (indexCount - 2) * 3);
        }
        break;

    }
#undef READ_IDX
#undef WRITE_IDX
}

kernel void fixIndexBuffer(
                           const device ushort *indexBufferUint16 [[ buffer(0), function_constant(indexBufferIsUint16) ]],
                           const device uint   *indexBufferUint32 [[ buffer(0), function_constant(indexBufferIsUint32) ]],
                           device ushort *outIndexBufferUint16 [[ buffer(1), function_constant(outIndexBufferIsUint16) ]],
                           device uint   *outIndexBufferUint32 [[ buffer(1), function_constant(outIndexBufferIsUint32) ]],
                           constant uint &indexCount [[ buffer(2) ]],
                           constant uint &primCount [[ buffer(3) ]],
                           uint prim [[thread_position_in_grid]])
{
    constexpr uint restartIndex = 0xFFFFFFFF; // unused
    uint baseIndex = 0;
    uint onIndex = onIndex;
    uint onOutIndex = onOutIndex;
    if(prim < primCount)
    {
        switch(fixIndexBufferMode)
        {
            case MtlFixIndexBufferKeyPoints:
                onIndex = prim;
                onOutIndex = prim;
                break;
            case MtlFixIndexBufferKeyLines:
                onIndex = prim * 2;
                onOutIndex = prim * 2;
                break;
            case MtlFixIndexBufferKeyLineStrip:
                onIndex = prim;
                onOutIndex = prim * 2;
                break;
            case MtlFixIndexBufferKeyTriangles:
                onIndex = prim * 3;
                onOutIndex = prim * 3;
                break;
            case MtlFixIndexBufferKeyTriangleStrip:
                onIndex = prim;
                onOutIndex = prim * 3;
                break;
        }
        outputPrimitive(indexBufferUint16, indexBufferUint32, outIndexBufferUint16, outIndexBufferUint32, restartIndex, indexCount, baseIndex, onIndex, onOutIndex);
    }
}



static inline void generatePrimitive(
                                   device ushort *outIndexBufferUint16,
                                   device uint   *outIndexBufferUint32,
                                   const uint firstVertex,
                                   const uint indexCount,
                                   thread uint &baseIndex,
                                   uint onIndex,
                                   uint primCount,
                                   thread uint &onOutIndex
                                   )
{
    if(baseIndex > onIndex) return; // skipped indices while processing
#define WRITE_IDX(_idx, _val) \
({ \
    if(outIndexBufferIsUint16) \
    { \
        outIndexBufferUint16[(_idx)] = _val + firstVertex; \
    } \
    if(outIndexBufferIsUint32) \
    { \
        outIndexBufferUint32[(_idx)] = _val + firstVertex; \
    } \
    _idx++; \
})
    switch(fixIndexBufferMode)
    {
        case MtlFixIndexBufferKeyPoints:
        {
            WRITE_IDX(onOutIndex, onIndex);
        }
        break;
        case MtlFixIndexBufferKeyLines:
        {
            auto tmpIndex0 = onIndex + 0;
            auto tmpIndex1 = onIndex + 1;
            if(fixIndexBufferKey & MtlFixIndexBufferKeyProvokingVertexLast)
            {
                WRITE_IDX(onOutIndex, tmpIndex1);
                WRITE_IDX(onOutIndex, tmpIndex0);
            }
            else
            {
                WRITE_IDX(onOutIndex, tmpIndex0);
                WRITE_IDX(onOutIndex, tmpIndex1);
            }
        }
        break;
        case MtlFixIndexBufferKeyLineLoop:
        {
            auto tmpIndex0 = onIndex + 0;
            auto tmpIndex1 = (onIndex + 1) % primCount;
            if(fixIndexBufferKey & MtlFixIndexBufferKeyProvokingVertexLast)
            {
                WRITE_IDX(onOutIndex, tmpIndex1);
                WRITE_IDX(onOutIndex, tmpIndex0);
            }
            else
            {
                WRITE_IDX(onOutIndex, tmpIndex0);
                WRITE_IDX(onOutIndex, tmpIndex1);
            }
        }
        break;
        case MtlFixIndexBufferKeyLineStrip:
        {
            auto tmpIndex0 = onIndex + 0;
            auto tmpIndex1 = onIndex + 1;
            if(fixIndexBufferKey & MtlFixIndexBufferKeyProvokingVertexLast)
            {
                WRITE_IDX(onOutIndex, tmpIndex1);
                WRITE_IDX(onOutIndex, tmpIndex0);
            }
            else
            {
                WRITE_IDX(onOutIndex, tmpIndex0);
                WRITE_IDX(onOutIndex, tmpIndex1);
            }
        }
        break;
        case MtlFixIndexBufferKeyTriangles:
        {
            auto tmpIndex0 = onIndex + 0;
            auto tmpIndex1 = onIndex + 1;
            auto tmpIndex2 = onIndex + 2;
            if(fixIndexBufferKey & MtlFixIndexBufferKeyProvokingVertexLast)
            {
                WRITE_IDX(onOutIndex, tmpIndex2);
                WRITE_IDX(onOutIndex, tmpIndex0);
                WRITE_IDX(onOutIndex, tmpIndex1);
            }
            else
            {
                WRITE_IDX(onOutIndex, tmpIndex0);
                WRITE_IDX(onOutIndex, tmpIndex1);
                WRITE_IDX(onOutIndex, tmpIndex2);
            }
        }
        break;
        case MtlFixIndexBufferKeyTriangleStrip:
        {
            uint isOdd = ((onIndex - baseIndex) & 1); // fixes winding. provoking fixed later.
            auto tmpIndex0 = onIndex + 0 + isOdd;
            auto tmpIndex1 = onIndex + 1 - isOdd;
            auto tmpIndex2 = onIndex + 2;
            if(fixIndexBufferKey & MtlFixIndexBufferKeyProvokingVertexLast)
            {
                WRITE_IDX(onOutIndex, tmpIndex2); // 2 is always the provoking vertex .: do not need to do anything special with isOdd
                WRITE_IDX(onOutIndex, tmpIndex0);
                WRITE_IDX(onOutIndex, tmpIndex1);
            }
            else
            {
                if(isOdd)
                {
                    WRITE_IDX(onOutIndex, tmpIndex1); // in the case of odd this is REALLY (onIndex + 0) // provoking vertex
                    WRITE_IDX(onOutIndex, tmpIndex2);
                    WRITE_IDX(onOutIndex, tmpIndex0);
                }
                else
                {
                    WRITE_IDX(onOutIndex, tmpIndex0); // in the case of even this is (onIndex + 0) // provoking vertex
                    WRITE_IDX(onOutIndex, tmpIndex1);
                    WRITE_IDX(onOutIndex, tmpIndex2);
                }
            }
            // assert never worse that worst-case expansion
            assert(onOutIndex <= (onIndex + 1) * 3);
            assert(onOutIndex <= (indexCount - 2) * 3);
            break;
        }
        case MtlFixIndexBufferKeyTriangleFan:
        {
            auto tmpIndex0 = 0;
            auto tmpIndex1 = onIndex + 1;
            auto tmpIndex2 = onIndex + 2;
            // Provoking Vertex for triangle fans does not use the pivot index for flat shading data.
            if(fixIndexBufferKey & MtlFixIndexBufferKeyProvokingVertexLast)
            {
                WRITE_IDX(onOutIndex, tmpIndex2);
                WRITE_IDX(onOutIndex, tmpIndex0);
                WRITE_IDX(onOutIndex, tmpIndex1);
            }
            else
            {
                WRITE_IDX(onOutIndex, tmpIndex1);
                WRITE_IDX(onOutIndex, tmpIndex2);
                WRITE_IDX(onOutIndex, tmpIndex0);
            }
        }
        break;

    }
#undef WRITE_IDX
}



kernel void genIndexBuffer(
                           device ushort *outIndexBufferUint16 [[ buffer(1), function_constant(outIndexBufferIsUint16) ]],
                           device uint   *outIndexBufferUint32 [[ buffer(1), function_constant(outIndexBufferIsUint32) ]],
                           constant uint &indexCount [[ buffer(2) ]],
                           constant uint &primCount [[ buffer(3) ]],
                           constant uint &firstVertex [[ buffer(4) ]],
                           uint prim [[thread_position_in_grid]])
{
    uint baseIndex = 0;
    uint onIndex = onIndex;
    uint onOutIndex = onOutIndex;
    if(prim < primCount)
    {
        switch(fixIndexBufferMode)
        {
            case MtlFixIndexBufferKeyPoints:
                onIndex = prim;
                onOutIndex = prim;
                break;
            case MtlFixIndexBufferKeyLines:
                onIndex = prim * 2;
                onOutIndex = prim * 2;
                break;
            case MtlFixIndexBufferKeyLineStrip:
                onIndex = prim;
                onOutIndex = prim * 2;
                break;
            case MtlFixIndexBufferKeyLineLoop:
                onIndex = prim;
                onOutIndex = prim * 2;
                break;
            case MtlFixIndexBufferKeyTriangles:
                onIndex = prim * 3;
                onOutIndex = prim * 3;
                break;
            case MtlFixIndexBufferKeyTriangleStrip:
                onIndex = prim;
                onOutIndex = prim * 3;
                break;
            case MtlFixIndexBufferKeyTriangleFan:
                onIndex = prim;
                onOutIndex = prim * 3;
                break;
        }
        generatePrimitive(outIndexBufferUint16, outIndexBufferUint32, firstVertex, indexCount, baseIndex, onIndex, primCount, onOutIndex);
    }
}
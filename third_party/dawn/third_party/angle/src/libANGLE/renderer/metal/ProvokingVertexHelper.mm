//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProvokingVertexHelper.mm:
//    Implements the class methods for ProvokingVertexHelper.
//

#include "libANGLE/renderer/metal/ProvokingVertexHelper.h"
#import <Foundation/Foundation.h>
#include "libANGLE/Display.h"
#include "libANGLE/renderer/metal/ContextMtl.h"
#include "libANGLE/renderer/metal/DisplayMtl.h"
#include "libANGLE/renderer/metal/mtl_common.h"
#include "libANGLE/renderer/metal/shaders/rewrite_indices_shared.h"
namespace rx
{

namespace
{
constexpr size_t kInitialIndexBufferSize = 0xFFFF;  // Initial 64k pool.
}
static inline uint primCountForIndexCount(const uint fixIndexBufferKey, const uint indexCount)
{
    const uint fixIndexBufferMode =
        (fixIndexBufferKey >> MtlFixIndexBufferKeyModeShift) & MtlFixIndexBufferKeyModeMask;

    switch (fixIndexBufferMode)
    {
        case MtlFixIndexBufferKeyPoints:
            return indexCount;
        case MtlFixIndexBufferKeyLines:
            return indexCount / 2;
        case MtlFixIndexBufferKeyLineStrip:
            return (uint)MAX(0, (int)indexCount - 1);
        case MtlFixIndexBufferKeyLineLoop:
            return (uint)MAX(0, (int)indexCount);
        case MtlFixIndexBufferKeyTriangles:
            return indexCount / 3;
        case MtlFixIndexBufferKeyTriangleStrip:
            return (uint)MAX(0, (int)indexCount - 2);
        case MtlFixIndexBufferKeyTriangleFan:
            return (uint)MAX(0, (int)indexCount - 2);
        default:
            ASSERT(false);
            return 0;
    }
}

static inline uint indexCountForPrimCount(const uint fixIndexBufferKey, const uint primCount)
{
    const uint fixIndexBufferMode =
        (fixIndexBufferKey >> MtlFixIndexBufferKeyModeShift) & MtlFixIndexBufferKeyModeMask;
    switch (fixIndexBufferMode)
    {
        case MtlFixIndexBufferKeyPoints:
            return primCount;
        case MtlFixIndexBufferKeyLines:
            return primCount * 2;
        case MtlFixIndexBufferKeyLineStrip:
            return primCount * 2;
        case MtlFixIndexBufferKeyLineLoop:
            return primCount * 2;
        case MtlFixIndexBufferKeyTriangles:
            return primCount * 3;
        case MtlFixIndexBufferKeyTriangleStrip:
            return primCount * 3;
        case MtlFixIndexBufferKeyTriangleFan:
            return primCount * 3;
        default:
            ASSERT(false);
            return 0;
    }
}

static inline gl::PrimitiveMode getNewPrimitiveMode(const uint fixIndexBufferKey)
{
    const uint fixIndexBufferMode =
        (fixIndexBufferKey >> MtlFixIndexBufferKeyModeShift) & MtlFixIndexBufferKeyModeMask;
    switch (fixIndexBufferMode)
    {
        case MtlFixIndexBufferKeyPoints:
            return gl::PrimitiveMode::Points;
        case MtlFixIndexBufferKeyLines:
            return gl::PrimitiveMode::Lines;
        case MtlFixIndexBufferKeyLineStrip:
            return gl::PrimitiveMode::Lines;
        case MtlFixIndexBufferKeyLineLoop:
            return gl::PrimitiveMode::Lines;
        case MtlFixIndexBufferKeyTriangles:
            return gl::PrimitiveMode::Triangles;
        case MtlFixIndexBufferKeyTriangleStrip:
            return gl::PrimitiveMode::Triangles;
        case MtlFixIndexBufferKeyTriangleFan:
            return gl::PrimitiveMode::Triangles;
        default:
            ASSERT(false);
            return gl::PrimitiveMode::InvalidEnum;
    }
}
ProvokingVertexHelper::ProvokingVertexHelper(ContextMtl *context) : mIndexBuffers(false)
{
    mIndexBuffers.initialize(context, kInitialIndexBufferSize, mtl::kIndexBufferOffsetAlignment, 0);
}

void ProvokingVertexHelper::onDestroy(ContextMtl *context)
{
    mIndexBuffers.destroy(context);
}

void ProvokingVertexHelper::releaseInFlightBuffers(ContextMtl *contextMtl)
{
    mIndexBuffers.releaseInFlightBuffers(contextMtl);
}

static uint buildIndexBufferKey(const mtl::ProvokingVertexComputePipelineDesc &pipelineDesc)
{
    uint indexBufferKey              = 0;
    gl::DrawElementsType elementType = (gl::DrawElementsType)pipelineDesc.elementType;
    bool doPrimPrestart              = pipelineDesc.primitiveRestartEnabled;
    gl::PrimitiveMode primMode       = pipelineDesc.primitiveMode;
    switch (elementType)
    {
        case gl::DrawElementsType::UnsignedShort:
            indexBufferKey |= MtlFixIndexBufferKeyUint16 << MtlFixIndexBufferKeyInShift;
            indexBufferKey |= MtlFixIndexBufferKeyUint16 << MtlFixIndexBufferKeyOutShift;
            break;
        case gl::DrawElementsType::UnsignedInt:
            indexBufferKey |= MtlFixIndexBufferKeyUint32 << MtlFixIndexBufferKeyInShift;
            indexBufferKey |= MtlFixIndexBufferKeyUint32 << MtlFixIndexBufferKeyOutShift;
            break;
        default:
            ASSERT(false);  // Index type should only be short or int.
            break;
    }
    indexBufferKey |= (uint)primMode << MtlFixIndexBufferKeyModeShift;
    indexBufferKey |= doPrimPrestart ? MtlFixIndexBufferKeyPrimRestart : 0;
    // We only rewrite indices if we're switching the provoking vertex mode.
    indexBufferKey |= MtlFixIndexBufferKeyProvokingVertexLast;
    return indexBufferKey;
}

angle::Result ProvokingVertexHelper::getComputePipleineState(
    ContextMtl *context,
    const mtl::ProvokingVertexComputePipelineDesc &desc,
    mtl::AutoObjCPtr<id<MTLComputePipelineState>> *outComputePipeline)
{
    auto iter = mComputeFunctions.find(desc);
    if (iter != mComputeFunctions.end())
    {
        return context->getPipelineCache().getComputePipeline(context, iter->second,
                                                              outComputePipeline);
    }

    id<MTLLibrary> provokingVertexLibrary = context->getDisplay()->getDefaultShadersLib();
    uint indexBufferKey                   = buildIndexBufferKey(desc);
    auto fcValues = mtl::adoptObjCPtr([[MTLFunctionConstantValues alloc] init]);
    [fcValues setConstantValue:&indexBufferKey type:MTLDataTypeUInt withName:@"fixIndexBufferKey"];

    mtl::AutoObjCPtr<id<MTLFunction>> computeShader;
    if (desc.generateIndices)
    {
        ANGLE_TRY(CreateMslShader(context, provokingVertexLibrary, @"genIndexBuffer",
                                  fcValues.get(), &computeShader));
    }
    else
    {
        ANGLE_TRY(CreateMslShader(context, provokingVertexLibrary, @"fixIndexBuffer",
                                  fcValues.get(), &computeShader));
    }
    mComputeFunctions[desc] = computeShader;

    return context->getPipelineCache().getComputePipeline(context, computeShader,
                                                          outComputePipeline);
}

angle::Result ProvokingVertexHelper::prepareCommandEncoderForDescriptor(
    ContextMtl *context,
    mtl::ComputeCommandEncoder *encoder,
    mtl::ProvokingVertexComputePipelineDesc desc)
{
    mtl::AutoObjCPtr<id<MTLComputePipelineState>> pipelineState;
    ANGLE_TRY(getComputePipleineState(context, desc, &pipelineState));

    encoder->setComputePipelineState(pipelineState);

    return angle::Result::Continue;
}

angle::Result ProvokingVertexHelper::preconditionIndexBuffer(ContextMtl *context,
                                                             mtl::BufferRef indexBuffer,
                                                             size_t indexCount,
                                                             size_t indexOffset,
                                                             bool primitiveRestartEnabled,
                                                             gl::PrimitiveMode primitiveMode,
                                                             gl::DrawElementsType elementsType,
                                                             size_t &outIndexCount,
                                                             size_t &outIndexOffset,
                                                             gl::PrimitiveMode &outPrimitiveMode,
                                                             mtl::BufferRef &outNewBuffer)
{
    // Get specialized program
    // Upload index buffer
    // dispatch per-primitive?
    mtl::ProvokingVertexComputePipelineDesc pipelineDesc;
    pipelineDesc.elementType             = (uint8_t)elementsType;
    pipelineDesc.primitiveMode           = primitiveMode;
    pipelineDesc.primitiveRestartEnabled = primitiveRestartEnabled;
    pipelineDesc.generateIndices         = false;
    uint indexBufferKey                  = buildIndexBufferKey(pipelineDesc);
    uint primCount     = primCountForIndexCount(indexBufferKey, (uint32_t)indexCount);
    uint newIndexCount = indexCountForPrimCount(indexBufferKey, primCount);
    size_t indexSize   = gl::GetDrawElementsTypeSize(elementsType);
    size_t newOffset   = 0;
    mtl::BufferRef newBuffer;
    ANGLE_TRY(mIndexBuffers.allocate(context, newIndexCount * indexSize + indexOffset, nullptr,
                                     &newBuffer, &newOffset));
    uint indexCountEncoded     = (uint)indexCount;
    auto threadsPerThreadgroup = MTLSizeMake(MIN(primCount, 64u), 1, 1);

    mtl::ComputeCommandEncoder *encoder =
        context->getComputeCommandEncoderWithoutEndingRenderEncoder();
    ANGLE_TRY(prepareCommandEncoderForDescriptor(context, encoder, pipelineDesc));
    encoder->setBuffer(indexBuffer, static_cast<uint32_t>(indexOffset), 0);
    encoder->setBufferForWrite(
        newBuffer, static_cast<uint32_t>(indexOffset) + static_cast<uint32_t>(newOffset), 1);
    encoder->setData(&indexCountEncoded, 2);
    encoder->setData(&primCount, 3);
    encoder->dispatch(
        MTLSizeMake((primCount + threadsPerThreadgroup.width - 1) / threadsPerThreadgroup.width, 1,
                    1),
        threadsPerThreadgroup);
    outIndexCount    = newIndexCount;
    outIndexOffset   = newOffset;
    outPrimitiveMode = getNewPrimitiveMode(indexBufferKey);
    outNewBuffer     = newBuffer;
    return angle::Result::Continue;
}

angle::Result ProvokingVertexHelper::generateIndexBuffer(ContextMtl *context,
                                                         size_t first,
                                                         size_t indexCount,
                                                         gl::PrimitiveMode primitiveMode,
                                                         gl::DrawElementsType elementsType,
                                                         size_t &outIndexCount,
                                                         size_t &outIndexOffset,
                                                         gl::PrimitiveMode &outPrimitiveMode,
                                                         mtl::BufferRef &outNewBuffer)
{
    // Get specialized program
    // Upload index buffer
    // dispatch per-primitive?
    mtl::ProvokingVertexComputePipelineDesc pipelineDesc;
    pipelineDesc.elementType             = (uint8_t)elementsType;
    pipelineDesc.primitiveMode           = primitiveMode;
    pipelineDesc.primitiveRestartEnabled = false;
    pipelineDesc.generateIndices         = true;
    uint indexBufferKey                  = buildIndexBufferKey(pipelineDesc);
    uint primCount        = primCountForIndexCount(indexBufferKey, (uint32_t)indexCount);
    uint newIndexCount    = indexCountForPrimCount(indexBufferKey, primCount);
    size_t indexSize      = gl::GetDrawElementsTypeSize(elementsType);
    size_t newIndexOffset = 0;
    mtl::BufferRef newBuffer;
    ANGLE_TRY(mIndexBuffers.allocate(context, newIndexCount * indexSize, nullptr, &newBuffer,
                                     &newIndexOffset));
    uint indexCountEncoded     = static_cast<uint>(indexCount);
    uint firstVertexEncoded    = static_cast<uint>(first);
    uint indexOffsetEncoded    = static_cast<uint>(newIndexOffset);
    auto threadsPerThreadgroup = MTLSizeMake(MIN(primCount, 64u), 1, 1);

    mtl::ComputeCommandEncoder *encoder =
        context->getComputeCommandEncoderWithoutEndingRenderEncoder();
    ANGLE_TRY(prepareCommandEncoderForDescriptor(context, encoder, pipelineDesc));
    encoder->setBufferForWrite(newBuffer, indexOffsetEncoded, 1);
    encoder->setData(indexCountEncoded, 2);
    encoder->setData(primCount, 3);
    encoder->setData(firstVertexEncoded, 4);
    encoder->dispatch(
        MTLSizeMake((primCount + threadsPerThreadgroup.width - 1) / threadsPerThreadgroup.width, 1,
                    1),
        threadsPerThreadgroup);
    outIndexCount    = newIndexCount;
    outIndexOffset   = newIndexOffset;
    outPrimitiveMode = getNewPrimitiveMode(indexBufferKey);
    outNewBuffer     = newBuffer;
    return angle::Result::Continue;
}

}  // namespace rx

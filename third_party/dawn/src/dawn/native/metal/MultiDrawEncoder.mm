// Copyright 2024 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/native/metal/MultiDrawEncoder.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/native/ToBackend.h"
#include "dawn/native/metal/BufferMTL.h"

const char* kShaderSource = R"(
#include <metal_stdlib>
using namespace metal;

struct DrawCmd {
    uint32_t vertexCount;
    uint32_t instanceCount;
    uint32_t firstVertex;
    uint32_t firstInstance;
};

struct DrawIndexedCmd {
    uint32_t indexCount;
    uint32_t instanceCount;
    uint32_t firstIndex;
    uint32_t baseVertex;
    uint32_t firstInstance;
};

struct Constants {
    uint32_t maxDrawCount;
    uint32_t numIndexBufferElementsAfterOffsetLow;
    uint32_t numIndexBufferElementsAfterOffsetHigh;
    uint8_t flags;
    uint8_t primitiveType;
};

// Function constant
constant bool kValidationEnabled [[function_constant(0)]];

// Primitive types
constant uint8_t kPointList = 0;
constant uint8_t kLineList = 1;
constant uint8_t kLineStrip = 2;
constant uint8_t kTriangleList = 3;
constant uint8_t kTriangleStrip = 4;

// Flags
constant uint8_t kDrawCountBuffer = 4u;
constant uint8_t kIndexedDraw = 2u;
constant uint8_t kIndexType16 = 1u;  // bit indicating 16bit index type, else 32bit

struct MultiDrawData {
    command_buffer commandBuffer [[id(0)]];
    constant uint32_t* drawCmd [[id(1)]];  // Either DrawCmd or DrawIndexedCmd depending on flags
    // This can either be a buffer of uint16_t or uint32_t depending on flags
    constant uint32_t* indexBuffer [[id(2)]];
    constant uint* countBuffer [[id(3)]];
};

primitive_type getPrimitiveType(uint8_t type) {
    switch (type) {
        case kPointList:
            return primitive_type::point;
        case kLineList:
            return primitive_type::line;
        case kLineStrip:
            return primitive_type::line_strip;
        case kTriangleList:
            return primitive_type::triangle;
        case kTriangleStrip:
            return primitive_type::triangle_strip;
    }
    return primitive_type::triangle;
}

bool validateIndexed(constant Constants* constants, thread DrawIndexedCmd* cmd) {
    // Validation copied over from IndirectDrawValidationEncoder.cpp

    if (constants->numIndexBufferElementsAfterOffsetHigh >= 2) {
        return true;
    }
    if (constants->numIndexBufferElementsAfterOffsetHigh == 0 &&
        constants->numIndexBufferElementsAfterOffsetLow < cmd->firstIndex) {
        return false;
    }
    uint32_t maxIndexCount = constants->numIndexBufferElementsAfterOffsetLow - cmd->firstIndex;
    if (cmd->indexCount > maxIndexCount) {
        return false;
    }
    return true;
}

kernel void convertMultiDrawIndirect(uint2 gid [[thread_position_in_grid]],
                                     constant MultiDrawData* drawData [[buffer(0)]],
                                     constant Constants* constants [[buffer(1)]]) {
    uint32_t drawCount = 0;

    if (constants->flags & kDrawCountBuffer) {
        drawCount = min(constants->maxDrawCount, *drawData->countBuffer);
    } else {
        drawCount = constants->maxDrawCount;
    }

    // TODO(356461286): May want to measure the performance of 1 thread per draw vs 64 threads
    // that process multiple draws. 64 threads approach:
    // https://dawn-review.googlesource.com/c/dawn/+/195494/7/src/dawn/native/metal/MultiDrawEncoder.mm#103
    const uint cmdIndex = gid.x;

    if (cmdIndex >= drawCount) {
        return;
    }

    if (!(constants->flags & kIndexedDraw)) {  // Not indexed
        constant DrawCmd* commands = (constant DrawCmd*)(drawData->drawCmd);

        DrawCmd cmd = commands[cmdIndex];
        render_command command(drawData->commandBuffer, cmdIndex);
        primitive_type topology = getPrimitiveType(constants->primitiveType);

        command.draw_primitives(topology, cmd.firstVertex, cmd.vertexCount,
                                cmd.instanceCount, cmd.firstInstance);
    } else {
        constant DrawIndexedCmd* commands = (constant DrawIndexedCmd*)(drawData->drawCmd);

        DrawIndexedCmd cmd = commands[cmdIndex];

        if (kValidationEnabled) {
            if (!validateIndexed(constants, &cmd)) {
                return;
            }
        }

        render_command command(drawData->commandBuffer, cmdIndex);
        primitive_type topology = getPrimitiveType(constants->primitiveType);

        if (constants->flags & kIndexType16) {
            constant uint16_t* offsetteduint16 =
                ((constant uint16_t*)drawData->indexBuffer) + cmd.firstIndex;
            command.draw_indexed_primitives(topology, cmd.indexCount,
                                            offsetteduint16, cmd.instanceCount, cmd.baseVertex,
                                            cmd.firstInstance);
        } else {
            constant uint32_t* offsetteduint32 =
                ((constant uint32_t*)drawData->indexBuffer) + cmd.firstIndex;
            command.draw_indexed_primitives(topology, cmd.indexCount,
                                            offsetteduint32, cmd.instanceCount, cmd.baseVertex,
                                            cmd.firstInstance);
        }
    }
})";

// Direct copy of the Metal shader struct
struct Constants {
    uint32_t maxDrawCount;
    uint32_t numIndexBufferElementsAfterOffsetLow;
    uint32_t numIndexBufferElementsAfterOffsetHigh;
    uint8_t flags;
    uint8_t primitiveType;
};

static constexpr uint8_t kDrawCountBuffer = 4u;
static constexpr uint8_t kIndexedDraw = 2u;
static constexpr uint8_t kIndexType16 = 1u;  // bit indicating 16bit index type, else 32bit

// Primitive types
static constexpr uint8_t kPointList = 0;
static constexpr uint8_t kLineList = 1;
static constexpr uint8_t kLineStrip = 2;
static constexpr uint8_t kTriangleList = 3;
static constexpr uint8_t kTriangleStrip = 4;

namespace dawn::native::metal {

static uint8_t GetPrimitiveType(wgpu::PrimitiveTopology primitiveTopology) {
    switch (primitiveTopology) {
        case wgpu::PrimitiveTopology::PointList:
            return kPointList;
        case wgpu::PrimitiveTopology::LineList:
            return kLineList;
        case wgpu::PrimitiveTopology::LineStrip:
            return kLineStrip;
        case wgpu::PrimitiveTopology::TriangleList:
            return kTriangleList;
        case wgpu::PrimitiveTopology::TriangleStrip:
            return kTriangleStrip;
        case wgpu::PrimitiveTopology::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

ResultOrError<Ref<MultiDrawConverterPipeline>> MultiDrawConverterPipeline::Create(
    DeviceBase* device) {
    Ref<MultiDrawConverterPipeline> pipeline = AcquireRef(new MultiDrawConverterPipeline());

    DAWN_TRY(pipeline->Initialize(device));

    return pipeline;
}

MaybeError MultiDrawConverterPipeline::Initialize(DeviceBase* device) {
    id<MTLDevice> mtlDevice = ToBackend(device)->GetMTLDevice();
    NSError* error = nil;
    NSString* source = [NSString stringWithUTF8String:kShaderSource];
    id<MTLLibrary> lib = [mtlDevice newLibraryWithSource:source options:nullptr error:&error];
    if (error != nullptr) {
        return DAWN_INTERNAL_ERROR("Error creating multi draw MTLLibrary:" +
                                   std::string([error.localizedDescription UTF8String]));
    }
    DAWN_ASSERT(lib != nil);

    // Function constant for validation
    MTLFunctionConstantValues* funcConstants = [MTLFunctionConstantValues new];
    bool isValidationEnabled = device->IsValidationEnabled();
    [funcConstants setConstantValue:&isValidationEnabled type:MTLDataTypeBool atIndex:0];

    id<MTLFunction> convertFunc = [lib newFunctionWithName:@"convertMultiDrawIndirect"
                                            constantValues:funcConstants
                                                     error:&error];
    if (error != nullptr) {
        return DAWN_INTERNAL_ERROR("Error creating multi draw converter compute function:" +
                                   std::string([error.localizedDescription UTF8String]));
    }
    DAWN_ASSERT(convertFunc != nil);

    mArgumentEncoder = [convertFunc newArgumentEncoderWithBufferIndex:0];
    mPipeline = [mtlDevice newComputePipelineStateWithFunction:convertFunc error:&error];

    if (error != nullptr) {
        return DAWN_INTERNAL_ERROR("Error creating multi draw converter compute pipeline:" +
                                   std::string([error.localizedDescription UTF8String]));
    }
    DAWN_ASSERT(mPipeline != nil);

    return {};
}

id<MTLArgumentEncoder> MultiDrawConverterPipeline::GetMTLArgumentEncoder() const {
    DAWN_ASSERT(mArgumentEncoder != nullptr);
    return mArgumentEncoder.Get();
}

id<MTLComputePipelineState> MultiDrawConverterPipeline::GetMTLPipeline() const {
    DAWN_ASSERT(mPipeline != nullptr);
    return mPipeline.Get();
}

ResultOrError<std::vector<MultiDrawExecutionData>> PrepareMultiDraws(
    DeviceBase* device,
    id<MTLComputeCommandEncoder> encoder,
    const std::vector<IndirectDrawMetadata::IndirectMultiDraw>& multiDraws) {
    // Create the class that stores the pipeline and argument encoder if it doesn't exist
    InternalPipelineStore* store = device->GetInternalPipelineStore();

    if (store->multidrawICBConverterPipeline == nullptr) {
        DAWN_TRY_ASSIGN(store->multidrawICBConverterPipeline,
                        MultiDrawConverterPipeline::Create(device));
    }

    MultiDrawConverterPipeline* pipelineStore =
        reinterpret_cast<MultiDrawConverterPipeline*>(store->multidrawICBConverterPipeline.Get());

    id<MTLComputePipelineState> pipeline = pipelineStore->GetMTLPipeline();

    std::vector<MultiDrawExecutionData> outExecutionData;
    outExecutionData.reserve(multiDraws.size());

    id<MTLDevice> mtlDevice = ToBackend(device)->GetMTLDevice();
    for (auto& draw : multiDraws) {
        MultiDrawExecutionData& drawData = outExecutionData.emplace_back();
        drawData.mMaxDrawCount = draw.cmd->maxDrawCount;

        id<MTLBuffer> indirectBuffer = ToBackend(draw.cmd->indirectBuffer.Get())->GetMTLBuffer();
        id<MTLBuffer> drawCountBuffer = nullptr;
        id<MTLBuffer> indexBuffer = nullptr;

        if (draw.cmd->drawCountBuffer != nullptr) {
            drawCountBuffer = ToBackend(draw.cmd->drawCountBuffer.Get())->GetMTLBuffer();
        }
        if (draw.indexBuffer != nullptr) {
            indexBuffer = ToBackend(draw.indexBuffer)->GetMTLBuffer();
        }

        // Create indirect command buffer
        MTLIndirectCommandBufferDescriptor* descriptor = [MTLIndirectCommandBufferDescriptor new];
        descriptor.commandTypes = draw.type == IndirectDrawMetadata::DrawType::Indexed
                                      ? MTLIndirectCommandTypeDrawIndexed
                                      : MTLIndirectCommandTypeDraw;
        descriptor.inheritBuffers = true;
        descriptor.inheritPipelineState = true;

        drawData.mIndirectCommandBuffer =
            [mtlDevice newIndirectCommandBufferWithDescriptor:descriptor
                                              maxCommandCount:draw.cmd->maxDrawCount
                                                      options:MTLResourceStorageModePrivate];
        if (drawData.mIndirectCommandBuffer == nil) {
            return DAWN_INTERNAL_ERROR("Error creating an indirect command buffer for multi draw");
        }

        id<MTLArgumentEncoder> argEnc = pipelineStore->GetMTLArgumentEncoder();
        id<MTLBuffer> argBuffer = [mtlDevice newBufferWithLength:argEnc.encodedLength
                                                         options:MTLStorageModeShared];
        DAWN_ASSERT(argBuffer != nil);

        [argEnc setArgumentBuffer:argBuffer offset:0];
        [argEnc setIndirectCommandBuffer:drawData.mIndirectCommandBuffer.Get() atIndex:0];
        [argEnc setBuffer:indirectBuffer offset:draw.cmd->indirectOffset atIndex:1];
        [argEnc setBuffer:indexBuffer offset:draw.indexBufferOffsetInBytes atIndex:2];
        if (draw.cmd->drawCountBuffer != nullptr) {
            [argEnc setBuffer:drawCountBuffer offset:draw.cmd->drawCountOffset atIndex:3];
        }

        [encoder setComputePipelineState:pipeline];
        [encoder setBuffer:argBuffer offset:0 atIndex:0];
        {
            Constants constants;
            constants.maxDrawCount = draw.cmd->maxDrawCount;
            constants.flags = 0;
            constants.primitiveType = GetPrimitiveType(draw.topology);
            constants.numIndexBufferElementsAfterOffsetLow = 0;
            constants.numIndexBufferElementsAfterOffsetHigh = 0;

            if (draw.cmd->drawCountBuffer != nullptr) {
                constants.flags |= kDrawCountBuffer;
            }

            if (draw.type == IndirectDrawMetadata::DrawType::Indexed) {
                constants.flags |= kIndexedDraw;

                const size_t formatSize = IndexFormatSize(draw.indexFormat);
                uint64_t numIndexElements = draw.indexBufferSize / formatSize;

                if (draw.indexFormat == wgpu::IndexFormat::Uint16) {
                    constants.flags |= kIndexType16;
                }

                constants.numIndexBufferElementsAfterOffsetLow =
                    static_cast<uint32_t>(numIndexElements & 0xFFFFFFFF);
                constants.numIndexBufferElementsAfterOffsetHigh =
                    static_cast<uint32_t>((numIndexElements >> 32) & 0xFFFFFFFF);
            }
            [encoder setBytes:&constants length:sizeof(Constants) atIndex:1];
        }

        // Metal requires that we call useResource makes the resource resident for the pass.
        [encoder useResource:drawData.mIndirectCommandBuffer.Get() usage:MTLResourceUsageRead];
        [encoder useResource:indirectBuffer usage:MTLResourceUsageRead];
        if (indexBuffer != nullptr) {
            [encoder useResource:indexBuffer usage:MTLResourceUsageRead];
        }
        if (draw.cmd->drawCountBuffer != nullptr) {
            [encoder useResource:drawCountBuffer usage:MTLResourceUsageRead];
        }

        id<MTLComputePipelineState> pipelineState = pipeline;
        uint32_t threadgroupCount = draw.cmd->maxDrawCount / pipelineState.threadExecutionWidth;
        threadgroupCount +=
            (draw.cmd->maxDrawCount % pipelineState.threadExecutionWidth) == 0 ? 0 : 1;

        [encoder dispatchThreadgroups:MTLSizeMake(threadgroupCount, 1, 1)
                threadsPerThreadgroup:MTLSizeMake(pipelineState.threadExecutionWidth, 1, 1)];
    }

    return std::move(outExecutionData);
}

void ExecuteMultiDraw(const MultiDrawExecutionData& data, id<MTLRenderCommandEncoder> encoder) {
    [encoder executeCommandsInBuffer:data.mIndirectCommandBuffer.Get()
                           withRange:NSMakeRange(0, data.mMaxDrawCount)];
}
}  // namespace dawn::native::metal

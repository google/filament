/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "MetalBlitter.h"

#include "MetalContext.h"

#include <math/vec4.h>
#include <utils/Panic.h>

#define NSERROR_CHECK(message)                                                                     \
    if (error) {                                                                                   \
        auto description = [error.localizedDescription cStringUsingEncoding:NSUTF8StringEncoding]; \
        utils::slog.e << description << utils::io::endl;                                           \
    }                                                                                              \
    ASSERT_POSTCONDITION(error == nil, message);

namespace filament {
namespace backend {
namespace metal {

static id<MTLFunction> gVertexFunction = nil;
static id<MTLFunction> gFragmentColor = nil;
static id<MTLFunction> gFragmentDepth = nil;
static id<MTLFunction> gFragmentColorDepth = nil;

static std::string functionLibrary (R"(
#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct VertexOut
{
    float4 vertexPosition [[position]];
    float2 uv;
};

struct ColorFragmentOut
{
    float4 color [[color(0)]];
};

struct DepthFragmentOut
{
    float depth [[depth(any)]];
};

struct ColorDepthFragmentOut
{
    float4 color [[color(0)]];
    float depth [[depth(any)]];
};

vertex VertexOut
blitterVertex(uint vid [[vertex_id]],
              constant float4* vertices [[buffer(0)]])
{
    VertexOut out = {};
    out.vertexPosition = float4(vertices[vid].xy, 0.0, 1.0);
    out.uv = vertices[vid].zw;
    return out;
}

fragment ColorFragmentOut
blitterFragColor(VertexOut in [[stage_in]],
                 sampler sourceSampler [[sampler(0)]],
                 texture2d<float, access::sample> sourceColor [[texture(0)]],
                 constant uint8_t* lod [[buffer(0)]])
{
    ColorFragmentOut out = {};
    out.color = sourceColor.sample(sourceSampler, in.uv, level(*lod));
    return out;
}

fragment DepthFragmentOut
blitterFragDepth(VertexOut in [[stage_in]],
                 sampler sourceSampler [[sampler(0)]],
                 texture2d<float, access::sample> sourceDepth [[texture(1)]],
                 constant uint8_t* lod [[buffer(0)]])
{
    DepthFragmentOut out = {};
    out.depth = sourceDepth.sample(sourceSampler, in.uv, level(*lod)).r;
    return out;
}

fragment ColorDepthFragmentOut
blitterFragColorDepth(VertexOut in [[stage_in]],
                      sampler sourceSampler [[sampler(0)]],
                      texture2d<float, access::sample> sourceColor [[texture(0)]],
                      texture2d<float, access::sample> sourceDepth [[texture(1)]],
                      constant uint8_t* lod [[buffer(0)]])
{
    ColorDepthFragmentOut out = {};
    out.color = sourceColor.sample(sourceSampler, in.uv, level(*lod));
    out.depth = sourceDepth.sample(sourceSampler, in.uv, level(*lod)).r;
    return out;
}
)");

MetalBlitter::MetalBlitter(MetalContext& context) noexcept : mContext(context) { }

void MetalBlitter::blit(const BlitArgs& args) {
    const bool blitColor = args.source.color != nil && args.destination.color != nil;
    const bool blitDepth = args.source.depth != nil && args.destination.depth != nil;

    if (!blitColor && !blitDepth) {
        return;
    }

    ensureFunctions();

    MTLRenderPassDescriptor* descriptor = [MTLRenderPassDescriptor renderPassDescriptor];

    if (blitColor) {
        setupColorAttachment(args, descriptor);
    }

    if (blitDepth) {
        setupDepthAttachment(args, descriptor);
    }

    id<MTLRenderCommandEncoder> encoder =
            [mContext.currentCommandBuffer renderCommandEncoderWithDescriptor:descriptor];
    encoder.label = @"Blit";

    id<MTLFunction> fragmentFunction = nil;
    if (blitColor && blitDepth) {
        fragmentFunction = gFragmentColorDepth;
    } else if (blitColor) {
        fragmentFunction = gFragmentColor;
    } else if (blitDepth) {
        fragmentFunction = gFragmentDepth;
    }

    PipelineState pipelineState {
        .vertexFunction = gVertexFunction,
        .fragmentFunction = fragmentFunction,
        .vertexDescription = {},
        .colorAttachmentPixelFormat = args.destination.color.pixelFormat,
        .depthAttachmentPixelFormat =
                blitDepth ? args.destination.depth.pixelFormat : MTLPixelFormatInvalid,
        .sampleCount = 1,
        .blendState = {}
    };
    id<MTLRenderPipelineState> pipeline = mContext.pipelineStateCache.getOrCreateState(pipelineState);
    [encoder setRenderPipelineState:pipeline];

    if (blitColor) {
        [encoder setFragmentTexture:args.source.color atIndex:0];
    }

    if (blitDepth) {
        [encoder setFragmentTexture:args.source.depth atIndex:1];
    }

    SamplerParams samplerState;
    samplerState.filterMin = static_cast<SamplerMinFilter>(args.filter);
    samplerState.filterMag = args.filter;
    id<MTLSamplerState> sampler = mContext.samplerStateCache.getOrCreateState(samplerState);
    [encoder setFragmentSamplerState:sampler atIndex:0];

    MTLViewport viewport;
    viewport.originX = args.destination.region.origin.x;
    viewport.originY = args.destination.region.origin.y;
    viewport.height = args.destination.region.size.height;
    viewport.width = args.destination.region.size.width;
    viewport.znear = 0.0;
    viewport.zfar = 1.0;
    [encoder setViewport:viewport];

    DepthStencilState depthStencilState {
        .compareFunction = MTLCompareFunctionAlways,
        .depthWriteEnabled = blitDepth
    };
    id<MTLDepthStencilState> depthStencil =
            mContext.depthStencilStateCache.getOrCreateState(depthStencilState);
    [encoder setDepthStencilState:depthStencil];

    /*
     *  We blit by rendering a single triangle that covers the entire viewport. The UV coordinates
     *  are chosen so that, when interpolated across the viewport, they correctly sample the
     *  desired region of the source texture:
     *
     *  (the Xs denote the 3 vertices)
     *
     *  (l, 2 * t - b)
     *  X
     *
     *
     *
     *  (l, t)     (r, t)
     *  . . . . . .
     *  . . . . . .
     *  . . . . . .
     *  . . . . . .
     *  X . . . . .           X
     *  (l, b)     (r, b)     (2 * r - l, b)
     *
     */

    if (blitColor && blitDepth) {
        // To keep the math simple, for now we'll assume the color and depth attachments of the
        // source render target have the same dimensions.
        ASSERT_PRECONDITION(args.source.color.width == args.source.depth.width &&
                            args.source.color.height == args.source.depth.height,
                            "Source color and depth attachments must be same dimensions.");
    }

    const NSUInteger sourceWidth = blitColor ? args.source.color.width : args.source.depth.width;
    const NSUInteger sourceHeight = blitColor ? args.source.color.height : args.source.depth.height;

    const float u = 1.0f / sourceWidth;
    const float v = 1.0f / sourceHeight;

    const auto& sourceRegion = args.source.region;
    const float left   = sourceRegion.origin.x * u;
    const float top    = sourceRegion.origin.y * v;
    const float right  = (sourceRegion.origin.x + sourceRegion.size.width) * u;
    const float bottom = (sourceRegion.origin.y + sourceRegion.size.height) * v;

    const math::float4 vertices[3] = {
        { -1.0f, -1.0f,  left,  bottom },
        { -1.0f,  3.0f,  left, 2.0f * top - bottom },
        {  3.0f, -1.0f,  2.0f * right - left,  bottom },
    };

    [encoder setVertexBytes:vertices length:(sizeof(math::float4) * 3) atIndex:0];
    [encoder setFragmentBytes:&args.source.level length:sizeof(uint8_t) atIndex:0];
    [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
    [encoder endEncoding];
}

void MetalBlitter::shutdown() noexcept {
    [gVertexFunction release];
    [gFragmentColor release];
    [gFragmentDepth release];
    [gFragmentColorDepth release];
    gFragmentColor = nil;
    gFragmentColorDepth = nil;
    gVertexFunction = nil;
}

void MetalBlitter::setupColorAttachment(const BlitArgs& args,
        MTLRenderPassDescriptor* descriptor) {
    descriptor.colorAttachments[0].texture = args.destination.color;
    descriptor.colorAttachments[0].level = args.destination.level;

    descriptor.colorAttachments[0].loadAction = MTLLoadActionLoad;
    // We don't need to load the contents of the attachment if we're only blitting to part of it.
    if (args.colorDestinationIsFullAttachment()) {
        descriptor.colorAttachments[0].loadAction = MTLLoadActionDontCare;
    }

    descriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
}

void MetalBlitter::setupDepthAttachment(const BlitArgs& args, MTLRenderPassDescriptor* descriptor) {
    descriptor.depthAttachment.texture = args.destination.depth;
    descriptor.depthAttachment.level = args.destination.level;

    descriptor.depthAttachment.loadAction = MTLLoadActionLoad;
    // We don't need to load the contents of the attachment if we're only blitting to part of it.
    if (args.depthDestinationIsFullAttachment()) {
        descriptor.depthAttachment.loadAction = MTLLoadActionDontCare;
    }

    descriptor.depthAttachment.storeAction = MTLStoreActionStore;
}

void MetalBlitter::ensureFunctions() {
    if (gFragmentColor != nil && gFragmentDepth != nil && gFragmentColorDepth != nil &&
        gVertexFunction != nil) {
        return;
    }

    NSError* error = nil;
    NSString* objcSource = [NSString stringWithCString:functionLibrary.data()
                                              encoding:NSUTF8StringEncoding];
    id<MTLLibrary> library = [mContext.device newLibraryWithSource:objcSource
                                                            options:nil
                                                              error:&error];
    NSERROR_CHECK("Unable to compile shading library for MetalBlitter.");

    gVertexFunction = [library newFunctionWithName:@"blitterVertex"];
    gFragmentColor = [library newFunctionWithName:@"blitterFragColor"];
    gFragmentDepth = [library newFunctionWithName:@"blitterFragDepth"];
    gFragmentColorDepth = [library newFunctionWithName:@"blitterFragColorDepth"];
    [library release];
}

} // namespace metal
} // namespace backend
} // namespace filament

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
#include "MetalUtils.h"

#include <utils/Panic.h>
#include <utils/Log.h>

namespace filament::backend {

static const char* functionLibrary = R"(
#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct VertexOut
{
    float4 position [[position]];
};

struct FragmentOut
{
    float4 color [[color(0)]];
};

vertex VertexOut
blitterVertex(uint vid [[vertex_id]],
              constant float2* vertices [[buffer(0)]])
{
    VertexOut out = {};
    out.position = float4(vertices[vid], 0.0, 1.0);
    return out;
}

struct FragmentArgs {
    uint8_t lod;
    uint32_t depthPlane;
    float2 scale;
    float2 dstOffset;
    float2 srcOffset;
};

fragment FragmentOut
blitterFrag(VertexOut in [[stage_in]],
            sampler sourceSampler [[sampler(0)]],

#ifdef MSAA_COLOR_SOURCE
            texture2d_ms<float, access::read> sourceColor [[texture(0)]],
#elif SOURCES_3D
            texture3d<float, access::sample> sourceColor [[texture(0)]],
#else
            texture2d<float, access::sample> sourceColor [[texture(0)]],
#endif  // MSAA_COLOR_SOURCE

            constant FragmentArgs* args [[buffer(0)]])
{
    FragmentOut out = {};

    // These coordinates match the Vulkan vkCmdBlitImage spec:
    // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCmdBlitImage.html
    float2 uvbase = in.position.xy; // unnormalized coordinates at center of texel: (1.5, 2.5, etc)
    float2 uvoffset = uvbase - args->dstOffset;
    float2 uvscaled = uvoffset * args->scale;
    float2 uv = uvscaled + args->srcOffset;

#ifdef MSAA_COLOR_SOURCE
    out.color = float4(0.0);
    for (uint s = 0; s < sourceColor.get_num_samples(); s++) {
        out.color += sourceColor.read(static_cast<uint2>(uv), s);
    }
    out.color /= sourceColor.get_num_samples();
#elif SOURCES_3D
    float2 uvnorm = uv / float2(sourceColor.get_width(args->lod), sourceColor.get_height(args->lod));
    float3 coords = float3(uvnorm, (static_cast<float>(args->depthPlane) + 0.5) /
            sourceColor.get_depth(args->lod));
    out.color += sourceColor.sample(sourceSampler, coords, level(args->lod));
#else
    float2 uvnorm = uv / float2(sourceColor.get_width(args->lod), sourceColor.get_height(args->lod));
    out.color += sourceColor.sample(sourceSampler, uvnorm, level(args->lod));
#endif  // MSAA_COLOR_SOURCE

    return out;
}
)";


template<typename T>
inline bool MTLSizeEqual(T a, T b) noexcept {
    return (a.width == b.width && a.height == b.height && a.depth == b.depth);
}

MetalBlitter::MetalBlitter(MetalContext& context) noexcept : mContext(context) { }

void MetalBlitter::blit(id<MTLCommandBuffer> cmdBuffer, const BlitArgs& args, const char* label) {
    FILAMENT_CHECK_PRECONDITION(args.source.region.size.depth == args.destination.region.size.depth)
            << "Blitting requires the source and destination regions to have the same depth.";

    // Determine if the blit for color or depth are eligible to use a MTLBlitCommandEncoder.
    // blitFastPath returns true upon success.
    // blitFastPath fails if either the format or sampleCount don't match (i.e. resolve)
    if (blitFastPath(cmdBuffer, args, label)) {
        return;
    }

    // If we end-up here, it means that either:
    // - we're resolving a color texture
    // - src/dest formats didn't match, or we're scaling -- this can only happen with the legacy
    //   blit() path and implies that the format is not depth.
    // note: in the future we will support a fast-path resolve

    UTILS_UNUSED_IN_RELEASE
    const bool destinationIsMultisample =
            args.destination.texture.textureType == MTLTextureType2DMultisample;

    assert_invariant(!destinationIsMultisample);
    assert_invariant((args.destination.texture.usage & MTLTextureUsageRenderTarget));
    blitSlowPath(cmdBuffer, args, label);
}

bool MetalBlitter::blitFastPath(id<MTLCommandBuffer> cmdBuffer,
        const BlitArgs& args, const char* label) {

    if (args.source.texture.sampleCount == args.destination.texture.sampleCount &&
        args.source.texture.pixelFormat == args.destination.texture.pixelFormat &&
        MTLSizeEqual(args.source.region.size, args.destination.region.size)) {

        id<MTLBlitCommandEncoder> const blitEncoder = [cmdBuffer blitCommandEncoder];
        blitEncoder.label = @(label);
        [blitEncoder copyFromTexture:args.source.texture
                         sourceSlice:args.source.slice
                         sourceLevel:args.source.level
                        sourceOrigin:args.source.region.origin
                          sourceSize:args.source.region.size
                           toTexture:args.destination.texture
                    destinationSlice:args.destination.slice
                    destinationLevel:args.destination.level
                   destinationOrigin:args.destination.region.origin];
        [blitEncoder endEncoding];

        return true;
    }
    return false;
}

void MetalBlitter::blitSlowPath(id<MTLCommandBuffer> cmdBuffer,
        const BlitArgs& args, const char* label) {
    // scaling in any dimension is not allowed
    assert_invariant(args.source.region.size.depth == args.destination.region.size.depth);
    // we're always blitting a single plane
    uint32_t depthPlaneSource = args.source.region.origin.z;
    uint32_t depthPlaneDest = args.destination.region.origin.z;
    for (NSUInteger d = 0; d < args.source.region.size.depth; d++) {
        blitDepthPlane(cmdBuffer, args,
                depthPlaneSource++, depthPlaneDest++, label);
    }
}

void MetalBlitter::blitDepthPlane(id<MTLCommandBuffer> cmdBuffer, const BlitArgs& args,
        uint32_t depthPlaneSource, uint32_t depthPlaneDest, const char* label) {

    MTLRenderPassDescriptor* const descriptor = [MTLRenderPassDescriptor renderPassDescriptor];

    setupAttachment(descriptor.colorAttachments[0], args, depthPlaneDest);

    id<MTLRenderCommandEncoder> const encoder =
            [cmdBuffer renderCommandEncoderWithDescriptor:descriptor];
    encoder.label = @(label);

    BlitFunctionKey key;
    key.msaaColorSource = args.source.texture.textureType == MTLTextureType2DMultisample;
    key.sources3D       = args.source.texture.textureType == MTLTextureType3D;
    id<MTLFunction> const fragmentFunction = getBlitFragmentFunction(key);

    MetalPipelineState const pipelineState {
        .vertexFunction = getBlitVertexFunction(),
        .fragmentFunction = fragmentFunction,
        .vertexDescription = {},
        .colorAttachmentPixelFormat = {
            args.destination.texture.pixelFormat,
            MTLPixelFormatInvalid,
            MTLPixelFormatInvalid,
            MTLPixelFormatInvalid,
            MTLPixelFormatInvalid,
            MTLPixelFormatInvalid,
            MTLPixelFormatInvalid,
            MTLPixelFormatInvalid
        },
        .depthAttachmentPixelFormat = MTLPixelFormatInvalid,
        .sampleCount = 1,
        .blendState = {}
    };
    id<MTLRenderPipelineState> const pipeline =
            mContext.pipelineStateCache.getOrCreateState(pipelineState);
    [encoder setRenderPipelineState:pipeline];

    // For texture arrays, create a view of the texture at the given slice (layer).

    id<MTLTexture> srcTextureColor = args.source.texture;
    if (srcTextureColor && srcTextureColor.textureType == MTLTextureType2DArray) {
        srcTextureColor = createTextureViewWithSingleSlice(srcTextureColor, args.source.slice);
    }
    [encoder setFragmentTexture:srcTextureColor atIndex:0];

    SamplerMinFilter filterMin = SamplerMinFilter::LINEAR;
    if (args.filter == SamplerMagFilter::NEAREST) {
        filterMin = SamplerMinFilter::LINEAR;
    } else if (args.filter == SamplerMagFilter::LINEAR) {
        filterMin = SamplerMinFilter::LINEAR;
    }

    SamplerState const s {
        .samplerParams = {
            .filterMag = args.filter,
            .filterMin = filterMin
        }
    };
    id<MTLSamplerState> const sampler = mContext.samplerStateCache.getOrCreateState(s);
    [encoder setFragmentSamplerState:sampler atIndex:0];

    MTLViewport viewport;
    viewport.originX = args.destination.region.origin.x;
    viewport.originY = args.destination.region.origin.y;
    viewport.height = args.destination.region.size.height;
    viewport.width = args.destination.region.size.width;
    viewport.znear = 0.0;
    viewport.zfar = 1.0;
    [encoder setViewport:viewport];

    DepthStencilState const depthStencilState {
        .depthCompare = MTLCompareFunctionAlways,
        .depthWriteEnabled = false
    };
    id<MTLDepthStencilState> const depthStencil =
            mContext.depthStencilStateCache.getOrCreateState(depthStencilState);
    [encoder setDepthStencilState:depthStencil];

    // We blit by rendering a single triangle that covers the entire viewport.
    const math::float2 vertices[3] = {
        { -1.0f, -1.0f },
        { -1.0f,  3.0f },
        {  3.0f, -1.0f },
    };

    struct FragmentArgs {
        uint8_t lod;
        uint32_t depthPlane;
        math::float2 scale;
        math::float2 dstOffset;
        math::float2 srcOffset;
    };

    const auto& sourceRegion = args.source.region;
    const auto& destinationRegion = args.destination.region;
    FragmentArgs fargs = {
            .lod = args.source.level,
            .depthPlane = static_cast<uint32_t>(depthPlaneSource),
            .scale = { static_cast<float>(sourceRegion.size.width) / args.destination.region.size.width,
                       static_cast<float>(sourceRegion.size.height) / args.destination.region.size.height },
            .dstOffset = { destinationRegion.origin.x, destinationRegion.origin.y },
            .srcOffset = { sourceRegion.origin.x, sourceRegion.origin.y }
    };

    [encoder setVertexBytes:vertices length:(sizeof(math::float2) * 3) atIndex:0];
    [encoder setFragmentBytes:&fargs length:sizeof(FragmentArgs) atIndex:0];
    [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
    [encoder endEncoding];
}

void MetalBlitter::shutdown() noexcept {
    mBlitFunctions.clear();
    mVertexFunction = nil;
}

void MetalBlitter::setupAttachment(MTLRenderPassAttachmentDescriptor* descriptor,
        const BlitArgs& args, uint32_t depthPlane) {
    descriptor.texture = args.destination.texture;
    descriptor.level = args.destination.level;
    descriptor.slice = args.destination.slice;
    descriptor.depthPlane = depthPlane;
    descriptor.loadAction = MTLLoadActionLoad;
    // We don't need to load the contents of the attachment if we're blitting over all of it.
    if (args.destinationIsFullAttachment()) {
        descriptor.loadAction = MTLLoadActionDontCare;
    }
    descriptor.storeAction = MTLStoreActionStore;
}

id<MTLFunction> MetalBlitter::compileFragmentFunction(BlitFunctionKey key) const {
    MTLCompileOptions* const options = [MTLCompileOptions new];
    NSMutableDictionary* const macros = [NSMutableDictionary dictionary];
    if (key.msaaColorSource) {
        macros[@"MSAA_COLOR_SOURCE"] = @"1";
    }
    if (key.sources3D) {
        macros[@"SOURCES_3D"] = @"1";
    }
    options.preprocessorMacros = macros;
    NSString* const objcSource = [NSString stringWithCString:functionLibrary
                                                    encoding:NSUTF8StringEncoding];
    NSError* error = nil;
    id <MTLLibrary> const library = [mContext.device newLibraryWithSource:objcSource
                                                                  options:options
                                                                    error:&error];
    id <MTLFunction> const function = [library newFunctionWithName:@"blitterFrag"];

    if (!library || !function) {
        if (error) {
            auto description = [error.localizedDescription cStringUsingEncoding:NSUTF8StringEncoding];
            utils::slog.e << description << utils::io::endl;
        }
    }
    FILAMENT_CHECK_POSTCONDITION(library && function)
            << "Unable to compile fragment shader for MetalBlitter.";

    return function;
}

id<MTLFunction> MetalBlitter::getBlitVertexFunction() {
    if (mVertexFunction != nil) {
        return mVertexFunction;
    }

    NSString* const objcSource = [NSString stringWithCString:functionLibrary
                                                    encoding:NSUTF8StringEncoding];
    NSError* error = nil;
    id <MTLLibrary> const library = [mContext.device newLibraryWithSource:objcSource
                                                                  options:nil
                                                                    error:&error];

    id<MTLFunction> const function = [library newFunctionWithName:@"blitterVertex"];

    if (!library || !function) {
        if (error) {
            auto description = [error.localizedDescription cStringUsingEncoding:NSUTF8StringEncoding];
            utils::slog.e << description << utils::io::endl;
        }
    }
    FILAMENT_CHECK_POSTCONDITION(library && function)
            << "Unable to compile vertex shader for MetalBlitter.";

    mVertexFunction = function;

    return mVertexFunction;
}

id<MTLFunction> MetalBlitter::getBlitFragmentFunction(BlitFunctionKey key) {
    assert_invariant(key.isValid());
    auto iter = mBlitFunctions.find(key);
    if (iter != mBlitFunctions.end()) {
        return iter.value();
    }

    auto function = compileFragmentFunction(key);
    mBlitFunctions.emplace(std::make_pair(key, function));

    return function;
}

} // namespace filament::backend


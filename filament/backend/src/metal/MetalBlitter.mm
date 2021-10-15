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

#include <utils/Panic.h>

namespace filament {
namespace backend {
namespace metal {

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
#ifdef BLIT_COLOR
    float4 color [[color(0)]];
#endif

#ifdef BLIT_DEPTH
    float depth [[depth(any)]];
#endif
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

#ifdef BLIT_COLOR
#ifdef MSAA_COLOR_SOURCE
            texture2d_ms<float, access::read> sourceColor [[texture(0)]],
#elif SOURCES_3D
            texture3d<float, access::sample> sourceColor [[texture(0)]],
#else
            texture2d<float, access::sample> sourceColor [[texture(0)]],
#endif  // MSAA_COLOR_SOURCE
#endif  // BLIT_COLOR

#ifdef BLIT_DEPTH
#ifdef MSAA_DEPTH_SOURCE
            texture2d_ms<float, access::read> sourceDepth [[texture(1)]],
#elif SOURCES_3D
            texture3d<float, access::sample> sourceDepth [[texture(1)]],
#else
            texture2d<float, access::sample> sourceDepth [[texture(1)]],
#endif  // MSAA_DEPTH_SOURCE
#endif  // BLIT_DEPTH

            constant FragmentArgs* args [[buffer(0)]])
{
    FragmentOut out = {};

    // These coordinates match the Vulkan vkCmdBlitImage spec:
    // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCmdBlitImage.html
    float2 uvbase = in.position.xy; // unnormalized coordinates at center of texel: (1.5, 2.5, etc)
    float2 uvoffset = uvbase - args->dstOffset;
    float2 uvscaled = uvoffset * args->scale;
    float2 uv = uvscaled + args->srcOffset;

#ifdef BLIT_COLOR
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
#endif  // BLIT_COLOR

#ifdef BLIT_DEPTH
#ifdef MSAA_DEPTH_SOURCE
    out.depth = 0.0;
    for (uint s = 0; s < sourceDepth.get_num_samples(); s++) {
        out.depth += sourceDepth.read(static_cast<uint2>(uv), s).r;
    }
    out.depth /= sourceDepth.get_num_samples();
#elif SOURCES_3D
    float2 uvnormd = uv / float2(sourceDepth.get_width(args->lod), sourceDepth.get_height(args->lod));
    float3 coords = float3(uvnormd, (static_cast<float>(args->depthPlane) + 0.5) /
            sourceDepth.get_depth(args->lod));
    out.depth = sourceDepth.sample(sourceSampler, coords, level(args->lod)).r;
#else
    float2 uvnormd = uv / float2(sourceDepth.get_width(args->lod), sourceDepth.get_height(args->lod));
    out.depth = sourceDepth.sample(sourceSampler, uvnormd, level(args->lod)).r;
#endif  // MSAA_DEPTH_SOURCE
#endif  // BLIT_DEPTH
    return out;
}
)";

MetalBlitter::MetalBlitter(MetalContext& context) noexcept : mContext(context) { }

#define MTLSizeEqual(a, b) (a.width == b.width && a.height == b.height && a.depth == b.depth)

void MetalBlitter::blit(id<MTLCommandBuffer> cmdBuffer, const BlitArgs& args) {
    bool blitColor = args.blitColor();
    bool blitDepth = args.blitDepth();

    ASSERT_PRECONDITION(args.source.slice == 0u, "Source attachment must have slice of 0.");

    ASSERT_PRECONDITION(args.source.region.size.depth == args.destination.region.size.depth,
            "Blitting requires the source and destination regions to have the same depth.");

    if (args.source.color && args.source.depth) {
        MTLTextureType colorType = args.source.color.textureType;
        MTLTextureType depthType = args.source.depth.textureType;

        if (colorType == MTLTextureType2DMultisample) colorType = MTLTextureType2D;
        if (depthType == MTLTextureType2DMultisample) depthType = MTLTextureType2D;

        ASSERT_PRECONDITION(colorType == depthType,
                "Blitting requires color and depth sources to be the same texture type.");
    }

    // Determine if the blit for color or depth are eligible to use a MTLBlitCommandEncoder.
    // blitColor and / or blitDepth are set to false upon success, to indicate that no more work is
    // necessary for that attachment.
    blitFastPath(cmdBuffer, blitColor, blitDepth, args);

    if (!blitColor && !blitDepth) {
        return;
    }

    // If the destination is MSAA and we weren't able to use the fast path, report an error, as
    // blitting to a MSAA texture isn't supported through the "slow path" yet.
    const bool colorDestinationIsMultisample =
            blitColor && args.destination.color.textureType == MTLTextureType2DMultisample;
    const bool depthDestinationIsMultisample =
            blitDepth && args.destination.depth.textureType == MTLTextureType2DMultisample;
    ASSERT_PRECONDITION(!colorDestinationIsMultisample && !depthDestinationIsMultisample,
        "Blitting between MSAA render targets with differing pixel formats and/or regions is not supported.");

    // If the destination texture doesn't have the MTLTextureUsageRenderTarget flag, we have to blit
    // to an intermediate texture first to perform the format conversion. Then, we can perform a
    // "fast blit" to the final destination texture.

    id<MTLTexture> intermediateColor = nil;
    id<MTLTexture> intermediateDepth = nil;
    BlitArgs slowBlit = args;
    BlitArgs finalBlit = args;

    MTLRegion sourceRegionNoOffset = MTLRegionMake3D(0, 0, 0,
            args.source.region.size.width, args.source.region.size.height,
            args.source.region.size.depth);

    if (blitColor && !(args.destination.color.usage & MTLTextureUsageRenderTarget)) {
        intermediateColor = createIntermediateTexture(args.destination.color, args.source.region.size);
        slowBlit.destination.color = finalBlit.source.color = intermediateColor;
        slowBlit.destination.level = finalBlit.source.level = 0;
        slowBlit.destination.slice = finalBlit.source.slice = 0;
        slowBlit.destination.region = finalBlit.source.region = sourceRegionNoOffset;
    }
    if (blitDepth && !(args.destination.depth.usage & MTLTextureUsageRenderTarget)) {
        intermediateDepth = createIntermediateTexture(args.destination.depth, args.source.region.size);
        slowBlit.destination.depth = finalBlit.source.depth = intermediateDepth;
        slowBlit.destination.level = finalBlit.source.level = 0;
        slowBlit.destination.slice = finalBlit.source.slice = 0;
        slowBlit.destination.region = finalBlit.source.region = sourceRegionNoOffset;
    }

    blitSlowPath(cmdBuffer, blitColor, blitDepth, slowBlit);

    bool finalBlitColor = intermediateColor != nil;
    bool finalBlitDepth = intermediateDepth != nil;
    blitFastPath(cmdBuffer, finalBlitColor, finalBlitDepth, finalBlit);
}

void MetalBlitter::blitFastPath(id<MTLCommandBuffer> cmdBuffer, bool& blitColor, bool& blitDepth,
        const BlitArgs& args) {
    if (blitColor) {
        if (args.source.color.sampleCount == args.destination.color.sampleCount &&
            args.source.color.pixelFormat == args.destination.color.pixelFormat &&
            MTLSizeEqual(args.source.region.size, args.destination.region.size)) {

            id<MTLBlitCommandEncoder> blitEncoder = [cmdBuffer blitCommandEncoder];
            [blitEncoder copyFromTexture:args.source.color
                             sourceSlice:0
                             sourceLevel:args.source.level
                            sourceOrigin:args.source.region.origin
                              sourceSize:args.source.region.size
                               toTexture:args.destination.color
                        destinationSlice:args.destination.slice
                        destinationLevel:args.destination.level
                       destinationOrigin:args.destination.region.origin];
            [blitEncoder endEncoding];

            blitColor = false;
        }
    }

    if (blitDepth) {
        if (args.source.depth.sampleCount == args.destination.depth.sampleCount &&
            args.source.depth.pixelFormat == args.destination.depth.pixelFormat &&
            MTLSizeEqual(args.source.region.size, args.destination.region.size)) {

            id<MTLBlitCommandEncoder> blitEncoder = [cmdBuffer blitCommandEncoder];
            [blitEncoder copyFromTexture:args.source.depth
                             sourceSlice:0
                             sourceLevel:args.source.level
                            sourceOrigin:args.source.region.origin
                              sourceSize:args.source.region.size
                               toTexture:args.destination.depth
                        destinationSlice:args.destination.slice
                        destinationLevel:args.destination.level
                       destinationOrigin:args.destination.region.origin];
            [blitEncoder endEncoding];

            blitDepth = false;
        }
    }
}

void MetalBlitter::blitSlowPath(id<MTLCommandBuffer> cmdBuffer, bool& blitColor, bool& blitDepth,
        const BlitArgs& args) {

    uint32_t depthPlaneSource = args.source.region.origin.z;
    uint32_t depthPlaneDest = args.destination.region.origin.z;

    assert_invariant(args.source.region.size.depth == args.destination.region.size.depth);
    uint32_t depthPlaneCount = args.source.region.size.depth;
    for (NSUInteger d = 0; d < depthPlaneCount; d++) {
        blitDepthPlane(cmdBuffer, blitColor, blitDepth, args, depthPlaneSource++, depthPlaneDest++);
    }

    blitColor = false;
    blitDepth = false;
}

void MetalBlitter::blitDepthPlane(id<MTLCommandBuffer> cmdBuffer, bool blitColor, bool blitDepth,
        const BlitArgs& args, uint32_t depthPlaneSource, uint32_t depthPlaneDest) {
    MTLRenderPassDescriptor* descriptor = [MTLRenderPassDescriptor renderPassDescriptor];

    if (blitColor) {
        setupColorAttachment(args, descriptor, depthPlaneDest);
    }

    if (blitDepth) {
        setupDepthAttachment(args, descriptor, depthPlaneDest);
    }

    id<MTLRenderCommandEncoder> encoder = [cmdBuffer renderCommandEncoderWithDescriptor:descriptor];
    encoder.label = @"Blit";

    BlitFunctionKey key;
    key.blitColor = blitColor;
    key.blitDepth = blitDepth;
    key.msaaColorSource = blitColor && args.source.color.textureType == MTLTextureType2DMultisample;
    key.msaaDepthSource = blitDepth && args.source.depth.textureType == MTLTextureType2DMultisample;
    key.sources3D       = blitColor && args.source.color.textureType == MTLTextureType3D;
    if (key.sources3D && blitDepth) {
        assert_invariant(args.source.depth.textureType == MTLTextureType3D);
    }
    id<MTLFunction> fragmentFunction = getBlitFragmentFunction(key);

    PipelineState pipelineState {
        .vertexFunction = getBlitVertexFunction(),
        .fragmentFunction = fragmentFunction,
        .vertexDescription = {},
        .colorAttachmentPixelFormat = {
            blitColor ? args.destination.color.pixelFormat : MTLPixelFormatInvalid,
            MTLPixelFormatInvalid,
            MTLPixelFormatInvalid,
            MTLPixelFormatInvalid,
            MTLPixelFormatInvalid,
            MTLPixelFormatInvalid,
            MTLPixelFormatInvalid,
            MTLPixelFormatInvalid
        },
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

    SamplerMinFilter filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST;
    if (args.filter == SamplerMagFilter::NEAREST) {
        filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST;
    } else if (args.filter == SamplerMagFilter::LINEAR) {
        filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST;
    }

    SamplerState s {
        .samplerParams = {
            .filterMag = args.filter,
            .filterMin = filterMin
        }
    };
    id<MTLSamplerState> sampler = mContext.samplerStateCache.getOrCreateState(s);
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

    [encoder setVertexBytes:vertices length:(sizeof(math::float4) * 3) atIndex:0];
    [encoder setFragmentBytes:&fargs length:sizeof(FragmentArgs) atIndex:0];
    [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
    [encoder endEncoding];
}

id<MTLTexture> MetalBlitter::createIntermediateTexture(id<MTLTexture> t, MTLSize size) {
    MTLTextureDescriptor* descriptor = [MTLTextureDescriptor new];
    descriptor.textureType = size.depth == 1 ? MTLTextureType2D : MTLTextureType3D;
    descriptor.pixelFormat = t.pixelFormat;
    descriptor.width = size.width;
    descriptor.height = size.height;
    descriptor.depth = size.depth;
    descriptor.usage = t.usage & MTLTextureUsageRenderTarget;
    return [mContext.device newTextureWithDescriptor:descriptor];
}

void MetalBlitter::shutdown() noexcept {
    mBlitFunctions.clear();
    mVertexFunction = nil;
}

void MetalBlitter::setupColorAttachment(const BlitArgs& args,
        MTLRenderPassDescriptor* descriptor, uint32_t depthPlane) {
    descriptor.colorAttachments[0].texture = args.destination.color;
    descriptor.colorAttachments[0].level = args.destination.level;
    descriptor.colorAttachments[0].slice = args.destination.slice;
    descriptor.colorAttachments[0].depthPlane = depthPlane;

    descriptor.colorAttachments[0].loadAction = MTLLoadActionLoad;
    // We don't need to load the contents of the attachment if we're blitting over all of it.
    if (args.colorDestinationIsFullAttachment()) {
        descriptor.colorAttachments[0].loadAction = MTLLoadActionDontCare;
    }

    descriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
}

void MetalBlitter::setupDepthAttachment(const BlitArgs& args, MTLRenderPassDescriptor* descriptor,
        uint32_t depthPlane) {
    descriptor.depthAttachment.texture = args.destination.depth;
    descriptor.depthAttachment.level = args.destination.level;
    descriptor.depthAttachment.slice = args.destination.slice;
    descriptor.depthAttachment.depthPlane = depthPlane;

    descriptor.depthAttachment.loadAction = MTLLoadActionLoad;
    // We don't need to load the contents of the attachment if we're blitting over all of it.
    if (args.depthDestinationIsFullAttachment()) {
        descriptor.depthAttachment.loadAction = MTLLoadActionDontCare;
    }

    descriptor.depthAttachment.storeAction = MTLStoreActionStore;
}

id<MTLFunction> MetalBlitter::compileFragmentFunction(BlitFunctionKey key) {
    MTLCompileOptions* options = [MTLCompileOptions new];
    NSMutableDictionary* macros = [NSMutableDictionary dictionary];
    if (key.blitColor) {
        macros[@"BLIT_COLOR"] = @"1";
    }
    if (key.blitDepth) {
        macros[@"BLIT_DEPTH"] = @"1";
    }
    if (key.msaaColorSource) {
        macros[@"MSAA_COLOR_SOURCE"] = @"1";
    }
    if (key.msaaDepthSource) {
        macros[@"MSAA_DEPTH_SOURCE"] = @"1";
    }
    if (key.sources3D) {
        macros[@"SOURCES_3D"] = @"1";
    }
    options.preprocessorMacros = macros;
    NSString* objcSource = [NSString stringWithCString:functionLibrary
                                              encoding:NSUTF8StringEncoding];
    NSError* error = nil;
    id<MTLLibrary> library = [mContext.device newLibraryWithSource:objcSource
                                                            options:options
                                                              error:&error];
    id<MTLFunction> function = [library newFunctionWithName:@"blitterFrag"];

    if (!library || !function) {
        if (error) {
            auto description = [error.localizedDescription cStringUsingEncoding:NSUTF8StringEncoding];
            utils::slog.e << description << utils::io::endl;
        }
    }
    ASSERT_POSTCONDITION(library && function, "Unable to compile fragment shader for MetalBlitter.");

    return function;
}

id<MTLFunction> MetalBlitter::getBlitVertexFunction() {
    if (mVertexFunction != nil) {
        return mVertexFunction;
    }

    NSString* objcSource = [NSString stringWithCString:functionLibrary
                                              encoding:NSUTF8StringEncoding];
    NSError* error = nil;
    id<MTLLibrary> library = [mContext.device newLibraryWithSource:objcSource
                                                           options:nil
                                                             error:&error];
    id<MTLFunction> function = [library newFunctionWithName:@"blitterVertex"];

    if (!library || !function) {
        if (error) {
            auto description = [error.localizedDescription cStringUsingEncoding:NSUTF8StringEncoding];
            utils::slog.e << description << utils::io::endl;
        }
    }
    ASSERT_POSTCONDITION(library && function, "Unable to compile vertex shader for MetalBlitter.");

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

} // namespace metal
} // namespace backend
} // namespace filament

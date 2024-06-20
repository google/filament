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

#include "MetalExternalImage.h"

#include "MetalContext.h"
#include "MetalEnums.h"
#include "MetalUtils.h"

#include <utils/Panic.h>
#include <utils/Log.h>
#include <utils/trap.h>

#define NSERROR_CHECK(message)                                                                     \
    if (error) {                                                                                   \
        auto description = [error.localizedDescription cStringUsingEncoding:NSUTF8StringEncoding]; \
        utils::slog.e << description << utils::io::endl;                                           \
    }                                                                                              \
    FILAMENT_CHECK_POSTCONDITION(error == nil) << message;

namespace filament {
namespace backend {

static const auto cvBufferDeleter = [](const void* buffer) {
    CVBufferRelease((CVMetalTextureRef) buffer);
};

static const char* kernel = R"(
#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

kernel void
ycbcrToRgb(texture2d<half, access::read>  inYTexture    [[texture(0)]],
           texture2d<half, access::read>  inCbCrTexture [[texture(1)]],
           texture2d<half, access::write> outTexture    [[texture(2)]],
           uint2                           gid          [[thread_position_in_grid]])
{
    if (gid.x >= outTexture.get_width() || gid.y >= outTexture.get_height()) {
        return;
    }

    half luminance = inYTexture.read(gid).r;
    // The color plane is half the size of the luminance plane.
    half2 color = inCbCrTexture.read(gid / 2).rg;

    half4 ycbcr = half4(luminance, color, 1.0);

    const half4x4 ycbcrToRGBTransform = half4x4(
        half4(+1.0000f, +1.0000f, +1.0000f, +0.0000f),
        half4(+0.0000f, -0.3441f, +1.7720f, +0.0000f),
        half4(+1.4020f, -0.7141f, +0.0000f, +0.0000f),
        half4(-0.7010f, +0.5291f, -0.8860f, +1.0000f)
    );

    outTexture.write(ycbcrToRGBTransform * ycbcr, gid);
}
)";

MetalExternalImage::MetalExternalImage(MetalContext& context, TextureSwizzle r, TextureSwizzle g,
        TextureSwizzle b, TextureSwizzle a) noexcept : mContext(context), mSwizzle{r, g, b, a} { }

bool MetalExternalImage::isValid() const noexcept {
    return mRgbTexture != nil || mImage != nullptr;
}

void MetalExternalImage::set(CVPixelBufferRef image) noexcept {
    unset();

    if (!image) {
        return;
    }

    OSType formatType = CVPixelBufferGetPixelFormatType(image);
    FILAMENT_CHECK_POSTCONDITION(formatType == kCVPixelFormatType_32BGRA ||
            formatType == kCVPixelFormatType_420YpCbCr8BiPlanarFullRange)
            << "Metal external images must be in either 32BGRA or 420f format.";

    size_t planeCount = CVPixelBufferGetPlaneCount(image);
    FILAMENT_CHECK_POSTCONDITION(planeCount == 0 || planeCount == 2)
            << "The Metal backend does not support images with plane counts of " << planeCount
            << ".";

    if (planeCount == 0) {
        mImage = image;
        mTexture = createTextureFromImage(image, MTLPixelFormatBGRA8Unorm, 0);
        mTextureView = createSwizzledTextureView(mTexture);
        mWidth = CVPixelBufferGetWidth(image);
        mHeight = CVPixelBufferGetHeight(image);
    }

    if (planeCount == 2) {
        CVMetalTextureRef yPlane = createTextureFromImage(image, MTLPixelFormatR8Unorm, Y_PLANE);
        CVMetalTextureRef cbcrPlane = createTextureFromImage(image, MTLPixelFormatRG8Unorm,
                CBCR_PLANE);

        // Get the size of luminance plane.
        mWidth = CVPixelBufferGetWidthOfPlane(image, Y_PLANE);
        mHeight = CVPixelBufferGetHeightOfPlane(image, Y_PLANE);

        id<MTLTexture> rgbTexture = createRgbTexture(mWidth, mHeight);
        id<MTLCommandBuffer> commandBuffer = encodeColorConversionPass(
                CVMetalTextureGetTexture(yPlane),
                CVMetalTextureGetTexture(cbcrPlane),
                rgbTexture);

        mRgbTexture = createSwizzledTextureView(rgbTexture);

        [commandBuffer addCompletedHandler:^(id <MTLCommandBuffer> o) {
            CVBufferRelease(yPlane);
            CVBufferRelease(cbcrPlane);
            CVPixelBufferRelease(image);
        }];

        [commandBuffer commit];
    }
}

void MetalExternalImage::set(CVPixelBufferRef image, size_t plane) noexcept {
    unset();

    if (!image) {
        return;
    }

    const OSType formatType = CVPixelBufferGetPixelFormatType(image);
    FILAMENT_CHECK_POSTCONDITION(formatType == kCVPixelFormatType_420YpCbCr8BiPlanarFullRange)
            << "Metal planar external images must be in the 420f format.";

    mImage = image;

    auto getPlaneFormat = [] (size_t plane) {
      // Right now Metal only supports kCVPixelFormatType_420YpCbCr8BiPlanarFullRange planar
      // external images, so we can make the following assumptions about the format of each plane.
      if (plane == 0) {
        return MTLPixelFormatR8Unorm; // luminance
      }
      if (plane == 1) {
        // CbCr
        return MTLPixelFormatRG8Unorm; // CbCr
      }
      return MTLPixelFormatInvalid;
    };

    const MTLPixelFormat format = getPlaneFormat(plane);
    assert_invariant(format != MTLPixelFormatInvalid);
    mTexture = createTextureFromImage(image, format, plane);
    mTextureView = createSwizzledTextureView(mTexture);
}

id<MTLTexture> MetalExternalImage::getMetalTextureForDraw() const noexcept {
    if (mRgbTexture) {
        return mRgbTexture;
    }

    // Retain the image and Metal texture until the GPU has finished with this frame. This does
    // not need to be done for the RGB texture, because it is an Objective-C object whose
    // lifetime is automatically managed by Metal.
    auto& tracker = mContext.resourceTracker;
    auto commandBuffer = getPendingCommandBuffer(&mContext);
    if (tracker.trackResource((__bridge void*) commandBuffer, mImage, cvBufferDeleter)) {
        CVPixelBufferRetain(mImage);
    }
    if (tracker.trackResource((__bridge void*) commandBuffer, mTexture, cvBufferDeleter)) {
        CVBufferRetain(mTexture);
    }

    assert_invariant(mTextureView);
    return mTextureView;
}

CVMetalTextureRef MetalExternalImage::createTextureFromImage(CVPixelBufferRef image,
        MTLPixelFormat format, size_t plane) {
    const size_t width = CVPixelBufferGetWidthOfPlane(image, plane);
    const size_t height = CVPixelBufferGetHeightOfPlane(image, plane);

    CVMetalTextureRef texture;
    CVReturn result = CVMetalTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
            mContext.textureCache, image, nullptr, format, width, height, plane, &texture);
    FILAMENT_CHECK_POSTCONDITION(result == kCVReturnSuccess)
            << "Could not create a CVMetalTexture from CVPixelBuffer.";

    return texture;
}

void MetalExternalImage::shutdown(MetalContext& context) noexcept {
    context.externalImageComputePipelineState = nil;
}

void MetalExternalImage::assertWritableImage(CVPixelBufferRef image) {
    OSType formatType = CVPixelBufferGetPixelFormatType(image);
    FILAMENT_CHECK_PRECONDITION(formatType == kCVPixelFormatType_32BGRA)
            << "Metal SwapChain images must be in the 32BGRA format.";
}

void MetalExternalImage::unset() {
    CVPixelBufferRelease(mImage);
    CVBufferRelease(mTexture);

    mImage = nullptr;
    mTexture = nullptr;
    mTextureView = nil;
    mRgbTexture = nil;
    mWidth = 0;
    mHeight = 0;
}

id<MTLTexture> MetalExternalImage::createRgbTexture(size_t width, size_t height) {
    MTLTextureDescriptor *descriptor =
            [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                               width:width
                                                              height:height
                                                           mipmapped:NO];
    descriptor.usage = MTLTextureUsageShaderWrite | MTLTextureUsageShaderRead;
    return [mContext.device newTextureWithDescriptor:descriptor];
}

id<MTLTexture> MetalExternalImage::createSwizzledTextureView(id<MTLTexture> texture) const {
    const bool isDefaultSwizzle =
            mSwizzle.r == TextureSwizzle::CHANNEL_0 &&
            mSwizzle.g == TextureSwizzle::CHANNEL_1 &&
            mSwizzle.b == TextureSwizzle::CHANNEL_2 &&
            mSwizzle.a == TextureSwizzle::CHANNEL_3;
    if (!isDefaultSwizzle && mContext.supportsTextureSwizzling) {
        // Even though we've already checked supportsTextureSwizzling, we still need to guard these
        // calls with @availability, otherwise the API usage will generate compiler warnings.
        if (@available(iOS 13, *)) {
            texture = createTextureViewWithSwizzle(texture,
                    getSwizzleChannels(mSwizzle.r, mSwizzle.g, mSwizzle.b, mSwizzle.a));
        }
    }
    return texture;
}

id<MTLTexture> MetalExternalImage::createSwizzledTextureView(CVMetalTextureRef ref) const {
    id<MTLTexture> texture = CVMetalTextureGetTexture(ref);
    return createSwizzledTextureView(texture);
}

void MetalExternalImage::ensureComputePipelineState() {
    if (mContext.externalImageComputePipelineState != nil) {
        return;
    }

    NSError* error = nil;

    NSString* objcSource = [NSString stringWithCString:kernel
                                              encoding:NSUTF8StringEncoding];
    id<MTLLibrary> library = [mContext.device newLibraryWithSource:objcSource
                                                            options:nil
                                                              error:&error];
    NSERROR_CHECK("Unable to compile Metal shading library.");

    id<MTLFunction> kernelFunction = [library newFunctionWithName:@"ycbcrToRgb"];

    mContext.externalImageComputePipelineState =
            [mContext.device newComputePipelineStateWithFunction:kernelFunction
                                                           error:&error];
    NSERROR_CHECK("Unable to create Metal compute pipeline state.");
}

id<MTLCommandBuffer> MetalExternalImage::encodeColorConversionPass(id<MTLTexture> inYPlane,
        id<MTLTexture> inCbCrTexture, id<MTLTexture> outTexture) {
    ensureComputePipelineState();

    id<MTLCommandBuffer> commandBuffer = [mContext.commandQueue commandBuffer];
    commandBuffer.label = @"YCbCr to RGB conversion";

    id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];

    [computeEncoder setComputePipelineState:mContext.externalImageComputePipelineState];
    [computeEncoder setTexture:inYPlane atIndex:0];
    [computeEncoder setTexture:inCbCrTexture atIndex:1];
    [computeEncoder setTexture:outTexture atIndex:2];

    MTLSize tgSize = MTLSizeMake(16, 16, 1);
    MTLSize tgCount;
    tgCount.width = (outTexture.width  + tgSize.width -  1) / tgSize.width;
    tgCount.height = (outTexture.height + tgSize.height - 1) / tgSize.height;
    tgCount.depth = 1;

    [computeEncoder dispatchThreadgroups:tgCount threadsPerThreadgroup:tgSize];

    [computeEncoder endEncoding];

    return commandBuffer;
}

} // namespace backend
} // namespace filament

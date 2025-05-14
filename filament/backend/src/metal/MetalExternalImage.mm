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
#include <utils/trap.h>

#include <absl/log/log.h>

#define NSERROR_CHECK(message)                                                                     \
    if (error) {                                                                                   \
        auto description = [error.localizedDescription cStringUsingEncoding:NSUTF8StringEncoding]; \
        LOG(ERROR) << description;                                                                 \
    }                                                                                              \
    FILAMENT_CHECK_POSTCONDITION(error == nil) << message;

namespace filament {
namespace backend {

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

NSUInteger MetalExternalImage::getWidth() const noexcept {
    if (mImage) {
        return CVPixelBufferGetWidth(mImage);
    }
    if (mRgbTexture) {
        return mRgbTexture.width;
    }
    return 0;
}

NSUInteger MetalExternalImage::getHeight() const noexcept {
    if (mImage) {
        return CVPixelBufferGetHeight(mImage);
    }
    if (mRgbTexture) {
        return mRgbTexture.height;
    }
    return 0;
}

MetalExternalImage MetalExternalImage::createFromImage(
        MetalContext& context, CVPixelBufferRef image) {
    if (!image) {
        return {};
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
        CVMetalTextureRef texture =
                createTextureFromImage(context.textureCache, image, MTLPixelFormatBGRA8Unorm, 0);
        return { CVPixelBufferRetain(image), texture };
    }

    if (planeCount == 2) {
        CVPixelBufferRetain(image);

        CVMetalTextureRef yPlane =
                createTextureFromImage(context.textureCache, image, MTLPixelFormatR8Unorm, Y_PLANE);
        CVMetalTextureRef cbcrPlane =
                createTextureFromImage(context.textureCache, image, MTLPixelFormatRG8Unorm, CBCR_PLANE);

        // Get the size of luminance plane.
        NSUInteger width = CVPixelBufferGetWidthOfPlane(image, Y_PLANE);
        NSUInteger height = CVPixelBufferGetHeightOfPlane(image, Y_PLANE);

        id<MTLTexture> rgbTexture = createRgbTexture(context.device, width, height);
        id<MTLCommandBuffer> commandBuffer = encodeColorConversionPass(context,
                CVMetalTextureGetTexture(yPlane),
                CVMetalTextureGetTexture(cbcrPlane),
                rgbTexture);

        [commandBuffer addCompletedHandler:^(id <MTLCommandBuffer> o) {
            CVBufferRelease(yPlane);
            CVBufferRelease(cbcrPlane);
            CVPixelBufferRelease(image);
        }];

        [commandBuffer commit];
        return MetalExternalImage { rgbTexture };
    }

    return {};
}

MetalExternalImage MetalExternalImage::createFromImagePlane(
        MetalContext& context, CVPixelBufferRef image, uint32_t plane) {
    if (!image) {
        return {};
    }

    const OSType formatType = CVPixelBufferGetPixelFormatType(image);
    FILAMENT_CHECK_POSTCONDITION(formatType == kCVPixelFormatType_420YpCbCr8BiPlanarFullRange)
            << "Metal planar external images must be in the 420f format.";
    FILAMENT_CHECK_POSTCONDITION(plane == 0 || plane == 1)
            << "Metal planar external images must be created from planes 0 or 1.";

    auto getPlaneFormat = [](size_t plane) {
        // Right now Metal only supports kCVPixelFormatType_420YpCbCr8BiPlanarFullRange planar
        // external images, so we can make the following assumptions about the format of each plane.
        if (plane == 0) {
            return MTLPixelFormatR8Unorm;  // luminance
        }
        if (plane == 1) {
            return MTLPixelFormatRG8Unorm;  // CbCr
        }
        return MTLPixelFormatInvalid;
    };

    const MTLPixelFormat format = getPlaneFormat(plane);
    assert_invariant(format != MTLPixelFormatInvalid);
    CVMetalTextureRef mTexture = createTextureFromImage(context.textureCache, image, format, plane);
    return { CVPixelBufferRetain(image), mTexture };
}

MetalExternalImage::MetalExternalImage(MetalExternalImage&& rhs) {
    std::swap(mImage, rhs.mImage);
    std::swap(mTexture, rhs.mTexture);
    std::swap(mRgbTexture, rhs.mRgbTexture);
}

MetalExternalImage& MetalExternalImage::operator=(MetalExternalImage&& rhs) {
    CVPixelBufferRelease(mImage);
    CVBufferRelease(mTexture);
    mImage = nullptr;
    mTexture = nullptr;
    mRgbTexture = nullptr;
    std::swap(mImage, rhs.mImage);
    std::swap(mTexture, rhs.mTexture);
    std::swap(mRgbTexture, rhs.mRgbTexture);
    return *this;
}

MetalExternalImage::~MetalExternalImage() noexcept {
    CVPixelBufferRelease(mImage);
    CVBufferRelease(mTexture);
}

id<MTLTexture> MetalExternalImage::getMtlTexture() const noexcept {
    if (mRgbTexture) {
        return mRgbTexture;
    }
    if (mTexture) {
        return CVMetalTextureGetTexture(mTexture);
    }
    return nil;
}

CVMetalTextureRef MetalExternalImage::createTextureFromImage(CVMetalTextureCacheRef textureCache,
        CVPixelBufferRef image, MTLPixelFormat format, size_t plane) {
    const size_t width = CVPixelBufferGetWidthOfPlane(image, plane);
    const size_t height = CVPixelBufferGetHeightOfPlane(image, plane);

    CVMetalTextureRef texture;
    CVReturn result = CVMetalTextureCacheCreateTextureFromImage(kCFAllocatorDefault, textureCache,
            image, nullptr, format, width, height, plane, &texture);
    FILAMENT_CHECK_POSTCONDITION(result == kCVReturnSuccess)
            << "Could not create a CVMetalTexture from CVPixelBuffer.";

    return texture;
}

void MetalExternalImage::shutdown(MetalContext& context) noexcept {
    context.externalImageComputePipelineState = nil;
}

id<MTLTexture> MetalExternalImage::createRgbTexture(
        id<MTLDevice> device, size_t width, size_t height) {
    MTLTextureDescriptor *descriptor =
            [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                               width:width
                                                              height:height
                                                           mipmapped:NO];
    descriptor.usage = MTLTextureUsageShaderWrite | MTLTextureUsageShaderRead;
    return [device newTextureWithDescriptor:descriptor];
}

void MetalExternalImage::ensureComputePipelineState(MetalContext& context) {
    if (context.externalImageComputePipelineState != nil) {
        return;
    }

    NSError* error = nil;

    NSString* objcSource = [NSString stringWithCString:kernel
                                              encoding:NSUTF8StringEncoding];
    id<MTLLibrary> library = [context.device newLibraryWithSource:objcSource
                                                          options:nil
                                                            error:&error];
    NSERROR_CHECK("Unable to compile Metal shading library.");

    id<MTLFunction> kernelFunction = [library newFunctionWithName:@"ycbcrToRgb"];

    context.externalImageComputePipelineState =
            [context.device newComputePipelineStateWithFunction:kernelFunction error:&error];
    NSERROR_CHECK("Unable to create Metal compute pipeline state.");
}

id<MTLCommandBuffer> MetalExternalImage::encodeColorConversionPass(MetalContext& context,
        id<MTLTexture> inYPlane, id<MTLTexture> inCbCrTexture, id<MTLTexture> outTexture) {
    ensureComputePipelineState(context);

    id<MTLCommandBuffer> commandBuffer = [context.commandQueue commandBuffer];
    commandBuffer.label = @"YCbCr to RGB conversion";

    id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];

    [computeEncoder setComputePipelineState:context.externalImageComputePipelineState];
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

void MetalExternalImage::assertWritableImage(CVPixelBufferRef image) {
    OSType formatType = CVPixelBufferGetPixelFormatType(image);
    FILAMENT_CHECK_PRECONDITION(formatType == kCVPixelFormatType_32BGRA)
            << "Metal SwapChain images must be in the 32BGRA format.";
}

} // namespace backend
} // namespace filament

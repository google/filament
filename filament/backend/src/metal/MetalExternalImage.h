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

#ifndef TNT_METALEXTERNALIMAGE_H
#define TNT_METALEXTERNALIMAGE_H

#import <backend/DriverEnums.h>

#include <CoreVideo/CoreVideo.h>
#include <Metal/Metal.h>

namespace filament {
namespace backend {

struct MetalContext;

/**
 * MetalExternalImage holds necessary handles for converting a single CVPixelBuffer to a Metal
 * texture.
 */
class MetalExternalImage {
public:
    MetalExternalImage() = default;

    MetalExternalImage(MetalExternalImage&&);
    MetalExternalImage& operator=(MetalExternalImage&&);
    ~MetalExternalImage() noexcept;

    MetalExternalImage(const MetalExternalImage&) = delete;
    MetalExternalImage& operator=(const MetalExternalImage&) = delete;

    /**
     * While the texture is used for rendering, this MetalExternalImage must be kept alive.
     */
    id<MTLTexture> getMtlTexture() const noexcept;

    bool isValid() const noexcept {
        return mImage != nil || mRgbTexture != nullptr;
    }

    NSUInteger getWidth() const noexcept;
    NSUInteger getHeight() const noexcept;

    /**
     * Create an external image with the passed-in CVPixelBuffer.
     *
     * Ownership is taken of the CVPixelBuffer, which will be released when the returned
     * MetalExternalImage is destroyed (or, in the case of a YCbCr image, after the conversion has
     * completed).
     *
     * Calling set with a YCbCr image will encode a compute pass to convert the image from
     * YCbCr to RGB.
     */
    static MetalExternalImage createFromImage(MetalContext& context, CVPixelBufferRef image);

    /**
     * Create an external image with a specific plane of the passed-in CVPixelBuffer.
     *
     * Ownership is taken of the CVPixelBuffer, which will be released when the returned
     * MetalExternalImage is destroyed.
     */
    static MetalExternalImage createFromImagePlane(
            MetalContext& context, CVPixelBufferRef image, uint32_t plane);

    static void assertWritableImage(CVPixelBufferRef image);

    /**
     * Free resources. Should be called at least once when no further calls to set will occur.
     */
    static void shutdown(MetalContext& context) noexcept;

private:
    MetalExternalImage(CVPixelBufferRef image, CVMetalTextureRef texture) noexcept
        : mImage(image), mTexture(texture) {}
    explicit MetalExternalImage(id<MTLTexture> texture) noexcept : mRgbTexture(texture) {}

    static id<MTLTexture> createRgbTexture(id<MTLDevice> device, size_t width, size_t height);
    static CVMetalTextureRef createTextureFromImage(CVMetalTextureCacheRef textureCache,
            CVPixelBufferRef image, MTLPixelFormat format, size_t plane);
    static void ensureComputePipelineState(MetalContext& context);
    static id<MTLCommandBuffer> encodeColorConversionPass(MetalContext& context,
            id<MTLTexture> inYPlane, id<MTLTexture> inCbCrTexture, id<MTLTexture> outTexture);

    static constexpr size_t Y_PLANE = 0;
    static constexpr size_t CBCR_PLANE = 1;

    // TODO: this could probably be a union.
    CVPixelBufferRef mImage = nullptr;
    CVMetalTextureRef mTexture = nullptr;
    id<MTLTexture> mRgbTexture = nil;
};

} // namespace backend
} // namespace filament

#endif //TNT_METALEXTERNALIMAGE_H

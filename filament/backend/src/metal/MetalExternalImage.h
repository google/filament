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

    MetalExternalImage(MetalContext& context,
            TextureSwizzle r = TextureSwizzle::CHANNEL_0,
            TextureSwizzle g = TextureSwizzle::CHANNEL_1,
            TextureSwizzle b = TextureSwizzle::CHANNEL_2,
            TextureSwizzle a = TextureSwizzle::CHANNEL_3) noexcept;

    /**
     * @return true, if this MetalExternalImage is holding a live external image. Returns false
     * until set has been called with a valid CVPixelBuffer. The image can be cleared via
     * set(nullptr), and isValid will return false again.
     */
    bool isValid() const noexcept;

    /**
     * Set this external image to the passed-in CVPixelBuffer. Future calls to
     * getMetalTextureForDraw will return a texture backed by this CVPixelBuffer. Previous
     * CVPixelBuffers and related resources will be released when all GPU work using them has
     * finished.
     *
     * Calling set with a YCbCr image will encode a compute pass to convert the image from YCbCr to
     * RGB.
     */
    void set(CVPixelBufferRef image) noexcept;

    /**
     * Set this external image to a specific plane of the passed-in CVPixelBuffer. Future calls to
     * getMetalTextureForDraw will return a texture backed by a single plane of this CVPixelBuffer.
     * Previous CVPixelBuffers and related resources will be released when all GPU work using them
     * has finished.
     */
    void set(CVPixelBufferRef image, size_t plane) noexcept;

    /**
     * Returns the width of the external image, or 0 if one is not set. For YCbCr images, returns
     * the width of the luminance plane.
     */
    size_t getWidth() const noexcept { return mWidth; }

    /**
     * Returns the height of the external image, or 0 if one is not set. For YCbCr images, returns
     * the height of the luminance plane.
     */
    size_t getHeight() const noexcept { return mHeight; }

    /**
     * Get a Metal texture used to draw this image and denote that it is used for the current frame.
     * For future frames that use this external image, getMetalTextureForDraw must be called again.
     */
    id<MTLTexture> getMetalTextureForDraw() const noexcept;

    /**
     * Free resources. Should be called at least once when no further calls to set will occur.
     */
    static void shutdown(MetalContext& context) noexcept;

    static void assertWritableImage(CVPixelBufferRef image);

private:

    void unset();

    CVMetalTextureRef createTextureFromImage(CVPixelBufferRef image, MTLPixelFormat format,
            size_t plane);
    id<MTLTexture> createRgbTexture(size_t width, size_t height);
    id<MTLTexture> createSwizzledTextureView(id<MTLTexture> texture) const;
    id<MTLTexture> createSwizzledTextureView(CVMetalTextureRef texture) const;
    void ensureComputePipelineState();
    id<MTLCommandBuffer> encodeColorConversionPass(id<MTLTexture> inYPlane, id<MTLTexture>
            inCbCrTexture, id<MTLTexture> outTexture);

    static constexpr size_t Y_PLANE = 0;
    static constexpr size_t CBCR_PLANE = 1;

    MetalContext& mContext;

    // If the external image has a single plane, mImage and mTexture hold references to the image
    // and created Metal texture, respectively.
    // mTextureView is a view of mTexture with any swizzling applied.
    CVPixelBufferRef mImage = nullptr;
    CVMetalTextureRef mTexture = nullptr;
    id<MTLTexture> mTextureView = nullptr;
    size_t mWidth = 0;
    size_t mHeight = 0;

    // If the external image is in the YCbCr format, this holds the result of the converted RGB
    // texture.
    id<MTLTexture> mRgbTexture = nil;

    struct {
        TextureSwizzle r, g, b, a;
    } mSwizzle;
};

} // namespace backend
} // namespace filament

#endif //TNT_METALEXTERNALIMAGE_H

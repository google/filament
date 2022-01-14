/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include "MetalUtils.h"

namespace filament::backend::metal {

id<MTLTexture> createTextureViewWithSwizzle(id<MTLTexture> texture,
        MTLTextureSwizzleChannels swizzle) {
    if (swizzle.red == MTLTextureSwizzleRed && swizzle.green == MTLTextureSwizzleGreen &&
            swizzle.blue == MTLTextureSwizzleBlue && swizzle.alpha == MTLTextureSwizzleAlpha) {
        return texture;
    }
    NSUInteger slices = texture.arrayLength;
    if (texture.textureType == MTLTextureTypeCube ||
        texture.textureType == MTLTextureTypeCubeArray) {
        slices *= 6;
    }
    NSUInteger mips = texture.mipmapLevelCount;
    return [texture newTextureViewWithPixelFormat:texture.pixelFormat
                                      textureType:texture.textureType
                                           levels:NSMakeRange(0, mips)
                                           slices:NSMakeRange(0, slices)
                                          swizzle:swizzle];
}

}

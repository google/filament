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

#ifndef TNT_TEXTURERESHAPER_H
#define TNT_TEXTURERESHAPER_H

#include <backend/DriverEnums.h>
#include <backend/PixelBufferDescriptor.h>

#include <functional>

namespace filament {
namespace backend {

class TextureReshaper {

public:

    /**
     * TextureReshaper reshapes pixels in a given format into a graphics API-native format.
     *
     * @param requestedFormat The format of the texture to be reshaped.
     */
    explicit TextureReshaper(TextureFormat requestedFormat) noexcept;

    /**
     * Returns the graphics API-native TextureFormat that pixels will be reshaped into.
     * If the format does not need reshaping, the original requestedFormat is returned.
     */
    TextureFormat getReshapedFormat() const noexcept;

    /**
     * reshapes the pixel buffer by adding components.
     *
     * @param p The pixel buffer to reshape.
     * @return A new PixelBufferDescriptor containing the reshaped pixels.
     */
    PixelBufferDescriptor reshape(PixelBufferDescriptor&& p) const;

    static bool canReshapeTextureFormat(TextureFormat format) noexcept;

private:

    std::function<PixelBufferDescriptor(PixelBufferDescriptor&& p)> reshapeFunction =
            [](PixelBufferDescriptor&& p){ return std::move(p); };
    TextureFormat reshapedFormat;

};

} // namespace backend
} // namespace filament

#endif //TNT_TEXTURERESHAPER_H

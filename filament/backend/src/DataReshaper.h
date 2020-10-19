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

#ifndef TNT_FILAMENT_DRIVER_DATARESHAPER_H
#define TNT_FILAMENT_DRIVER_DATARESHAPER_H

#include <stddef.h>

#include <math/scalar.h>

namespace filament {
namespace backend {

// This little utility adds padding to multi-channel interleaved data by inserting dummy values, or
// discards trailing channels. This is useful for platforms that only accept 4-component data, since
// users often wish to submit (or receive) 3-component data.
class DataReshaper {
public:
    template<typename componentType, size_t srcChannelCount, size_t dstChannelCount,
            componentType maxValue = std::numeric_limits<componentType>::max()>
    static void reshape(void* dest, const void* src, size_t numSrcBytes) {
        const componentType* in = (const componentType*) src;
        componentType* out = (componentType*) dest;
        const size_t srcWordCount = (numSrcBytes / sizeof(componentType)) / srcChannelCount;
        const int minChannelCount = filament::math::min(srcChannelCount, dstChannelCount);
        for (size_t word = 0; word < srcWordCount; ++word) {
            for (size_t channel = 0; channel < minChannelCount; ++channel) {
                out[channel] = in[channel];
            }
            for (size_t channel = srcChannelCount; channel < dstChannelCount; ++channel) {
                out[channel] = maxValue;
            }
            in += srcChannelCount;
            out += dstChannelCount;
        }
    }

    template<typename componentType, size_t srcChannelCount, size_t dstChannelCount,
            componentType maxValue = std::numeric_limits<componentType>::max()>
    static void reshapeImage(uint8_t* dest, const uint8_t* src, size_t srcBytesPerRow,
            size_t dstBytesPerRow, size_t height, bool swizzle03) {
        const size_t srcWordCount = (srcBytesPerRow / sizeof(componentType)) / srcChannelCount;
        const int minChannelCount = filament::math::min(srcChannelCount, dstChannelCount);
        assert(minChannelCount <= 4);
        int inds[4] = {0, 1, 2, 3};
        if (swizzle03) {
            inds[0] = 2;
            inds[2] = 0;
        }
        for (size_t row = 0; row < height; ++row) {
            const componentType* in = (const componentType*) src;
            componentType* out = (componentType*) dest;
            for (size_t word = 0; word < srcWordCount; ++word) {
                for (size_t channel = 0; channel < minChannelCount; ++channel) {
                    out[channel] = in[inds[channel]];
                }
                for (size_t channel = srcChannelCount; channel < dstChannelCount; ++channel) {
                    out[channel] = maxValue;
                }
                in += srcChannelCount;
                out += dstChannelCount;
            }
            src += srcBytesPerRow;
            dest += dstBytesPerRow;
        }
    }
};

} // namespace backend
} // namespace filament

#endif // TNT_FILAMENT_DRIVER_DATARESHAPER_H

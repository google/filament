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

#include <geometry/Transcoder.h>

#include <math/half.h>

using filament::math::half;

namespace filament {
namespace geometry {

// The internal workhorse function of the Transcoder, which takes arbitrary input but always
// produced packed floats. We expose a more readable interface than this to users, who often have
// untyped blobs of interleaved data.
template<typename SOURCE_TYPE, int NORMALIZATION_FACTOR>
void convert(float* target, void const* source, size_t count, int numComponents,
        int srcStride) {
    constexpr float scale = 1.0f / float(NORMALIZATION_FACTOR);
    uint8_t const* srcBytes = (uint8_t const*) source;
    for (size_t i = 0; i < count; ++i, target += numComponents, srcBytes += srcStride) {
        SOURCE_TYPE const* src = (SOURCE_TYPE const*) srcBytes;
        for (int n = 0; n < numComponents; ++n) {
            target[n] = float(src[n]) * scale;
        }
    }
}

// Similar to "convert" but clamps the result to -1, which is required for normalized signed types.
// For example, -128 can be represented in SBYTE but is outside the permitted range and should
// therefore be clamped. For more information, see the Vulkan spec under the section "Conversion
// from Normalized Fixed-Point to Floating-Point".
template<typename SOURCE_TYPE, int NORMALIZATION_FACTOR>
void convertClamped(float* target, void const* source, size_t count, int numComponents,
        int srcStride) {
    constexpr float scale = 1.0f / float(NORMALIZATION_FACTOR);
    uint8_t const* srcBytes = (uint8_t const*) source;
    for (size_t i = 0; i < count; ++i, target += numComponents, srcBytes += srcStride) {
        SOURCE_TYPE const* src = (SOURCE_TYPE const*) srcBytes;
        for (int n = 0; n < numComponents; ++n) {
            const float value = float(src[n]) * scale;
            target[n] = value < -1.0f ? -1.0f : value;
        }
    }
}

size_t Transcoder::operator()(float* target, void const* source, size_t count) const noexcept {
    const size_t required = count * mConfig.numComponents * sizeof(float);
    if (target == nullptr) {
        return required;
    }
    const int comp = mConfig.numComponents;
    switch (mConfig.componentType) {
        case ComponentType::BYTE: {
            const int stride = mConfig.strideBytes ? mConfig.strideBytes : comp;
            if (mConfig.normalized) {
                convertClamped<int8_t, 127>(target, source, count, comp, stride);
            } else {
                convert<int8_t, 1>(target, source, count, comp, stride);
            }
            return required;
        }
        case ComponentType::UBYTE: {
            int const stride = mConfig.strideBytes ? mConfig.strideBytes : comp;
            if (mConfig.normalized) {
                convert<uint8_t, 255>(target, source, count, comp, stride);
            } else {
                convert<uint8_t, 1>(target, source, count, comp, stride);
            }
            return required;
        }
        case ComponentType::SHORT: {
            const int stride = mConfig.strideBytes ? mConfig.strideBytes : (2 * comp);
            if (mConfig.normalized) {
                convertClamped<int16_t, 32767>(target, source, count, comp, stride);
            } else {
                convert<int16_t, 1>(target, source, count, comp, stride);
            }
            return required;
        }
        case ComponentType::USHORT: {
            const int stride = mConfig.strideBytes ? mConfig.strideBytes : (2 * comp);
            if (mConfig.normalized) {
                convert<uint16_t, 65535>(target, source, count, comp, stride);
            } else {
                convert<uint16_t, 1>(target, source, count, comp, stride);
            }
            return required;
        }
        case ComponentType::HALF: {
            const int stride = mConfig.strideBytes ? mConfig.strideBytes : (2 * comp);
            uint8_t const* srcBytes = (uint8_t const*) source;
            for (size_t i = 0; i < count; ++i, target += comp, srcBytes += stride) {
                half const* src = (half const*) srcBytes;
                for (int n = 0; n < comp; ++n) {
                    target[n] = float(src[n]);
                }
            }
            return required;
        }
    }
    return 0;
}

} // namespace geometry
} // namespace filament

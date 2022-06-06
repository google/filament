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
// untyped blobs of interleaved data. Note that this variant takes an arbitrary number of
// components, we also have a fixed-size variant for better compiler output.
template<typename SOURCE_TYPE, int NORMALIZATION_FACTOR>
void convert(float* UTILS_RESTRICT target, void const* UTILS_RESTRICT source, size_t count,
        int componentCount, int srcStride) noexcept {
    constexpr float scale = 1.0f / float(NORMALIZATION_FACTOR);
    uint8_t const* srcBytes = (uint8_t const*) source;
    for (size_t i = 0; i < count; ++i, target += componentCount, srcBytes += srcStride) {
        SOURCE_TYPE const* src = (SOURCE_TYPE const*) srcBytes;
        for (int n = 0; n < componentCount; ++n) {
            target[n] = float(src[n]) * scale;
        }
    }
}

template<typename SOURCE_TYPE, int NORMALIZATION_FACTOR, int NUM_COMPONENTS>
void convert(float* UTILS_RESTRICT target, void const* UTILS_RESTRICT source, size_t count,
        int srcStride) noexcept {
    constexpr float scale = 1.0f / float(NORMALIZATION_FACTOR);
    uint8_t const* srcBytes = (uint8_t const*) source;
    for (size_t i = 0; i < count; ++i, target += NUM_COMPONENTS, srcBytes += srcStride) {
        SOURCE_TYPE const* src = (SOURCE_TYPE const*) srcBytes;
        for (int n = 0; n < NUM_COMPONENTS; ++n) {
            target[n] = float(src[n]) * scale;
        }
    }
}

// Similar to "convert" but clamps the result to -1, which is required for normalized signed types.
// For example, -128 can be represented in SBYTE but is outside the permitted range and should
// therefore be clamped. For more information, see the Vulkan spec under the section "Conversion
// from Normalized Fixed-Point to Floating-Point".
template<typename SOURCE_TYPE, int NORMALIZATION_FACTOR>
void convertClamped(float* UTILS_RESTRICT target, void const* UTILS_RESTRICT source, size_t count,
        int componentCount, int srcStride) noexcept {
    constexpr float scale = 1.0f / float(NORMALIZATION_FACTOR);
    uint8_t const* srcBytes = (uint8_t const*) source;
    for (size_t i = 0; i < count; ++i, target += componentCount, srcBytes += srcStride) {
        SOURCE_TYPE const* src = (SOURCE_TYPE const*) srcBytes;
        for (int n = 0; n < componentCount; ++n) {
            const float value = float(src[n]) * scale;
            target[n] = value < -1.0f ? -1.0f : value;
        }
    }
}

template<typename SOURCE_TYPE, int NORMALIZATION_FACTOR, int NUM_COMPONENTS>
void convertClamped(float* UTILS_RESTRICT target, void const* UTILS_RESTRICT source, size_t count,
        int srcStride) noexcept {
    constexpr float scale = 1.0f / float(NORMALIZATION_FACTOR);
    uint8_t const* srcBytes = (uint8_t const*) source;
    for (size_t i = 0; i < count; ++i, target += NUM_COMPONENTS, srcBytes += srcStride) {
        SOURCE_TYPE const* src = (SOURCE_TYPE const*) srcBytes;
        for (int n = 0; n < NUM_COMPONENTS; ++n) {
            const float value = float(src[n]) * scale;
            target[n] = value < -1.0f ? -1.0f : value;
        }
    }
}

size_t Transcoder::operator()(float* UTILS_RESTRICT target, void const* UTILS_RESTRICT source,
        size_t count) const noexcept {
    const size_t required = count * mConfig.componentCount * sizeof(float);
    if (target == nullptr) {
        return required;
    }
    const uint32_t comp = mConfig.componentCount;
    switch (mConfig.componentType) {
        case ComponentType::BYTE: {
            const uint32_t stride = mConfig.inputStrideBytes ? mConfig.inputStrideBytes : comp;
            if (mConfig.normalized) {
                if (comp == 2) {
                    convertClamped<int8_t, 127, 2>(target, source, count, stride);
                } else if (comp == 3) {
                    convertClamped<int8_t, 127, 3>(target, source, count, stride);
                } else {
                    convertClamped<int8_t, 127>(target, source, count, comp, stride);
                }
            } else {
                if (comp == 2) {
                    convert<int8_t, 1, 2>(target, source, count, stride);
                } else if (comp == 3) {
                    convert<int8_t, 1, 3>(target, source, count, stride);
                } else {
                    convert<int8_t, 1>(target, source, count, comp, stride);
                }
            }
            return required;
        }
        case ComponentType::UBYTE: {
            const uint32_t stride = mConfig.inputStrideBytes ? mConfig.inputStrideBytes : comp;
            if (mConfig.normalized) {
                if (comp == 2) {
                    convert<uint8_t, 255, 2>(target, source, count, stride);
                } else if (comp == 3) {
                    convert<uint8_t, 255, 3>(target, source, count, stride);
                } else {
                    convert<uint8_t, 255>(target, source, count, comp, stride);
                }
            } else {
                if (comp == 2) {
                    convert<uint8_t, 1, 2>(target, source, count, stride);
                } else if (comp == 3) {
                    convert<uint8_t, 1, 3>(target, source, count, stride);
                } else {
                    convert<uint8_t, 1>(target, source, count, comp, stride);
                }
            }
            return required;
        }
        case ComponentType::SHORT: {
            const uint32_t stride = mConfig.inputStrideBytes ? mConfig.inputStrideBytes : (2 * comp);
            if (mConfig.normalized) {
                if (comp == 2) {
                    convertClamped<int16_t, 32767, 2>(target, source, count, stride);
                } else if (comp == 3) {
                    convertClamped<int16_t, 32767, 3>(target, source, count, stride);
                } else {
                    convertClamped<int16_t, 32767>(target, source, count, comp, stride);
                }
            } else {
                if (comp == 2) {
                    convert<int16_t, 1, 2>(target, source, count, stride);
                } else if (comp == 3) {
                    convert<int16_t, 1, 3>(target, source, count, stride);
                } else {
                    convert<int16_t, 1>(target, source, count, comp, stride);
                }
            }
            return required;
        }
        case ComponentType::USHORT: {
            const uint32_t stride = mConfig.inputStrideBytes ? mConfig.inputStrideBytes : (2 * comp);
            if (mConfig.normalized) {
                if (comp == 2) {
                    convert<uint16_t, 65535, 2>(target, source, count, stride);
                } else if (comp == 3) {
                    convert<uint16_t, 65535, 3>(target, source, count, stride);
                } else {
                    convert<uint16_t, 65535>(target, source, count, comp, stride);
                }
            } else {
                if (comp == 2) {
                    convert<uint16_t, 1, 2>(target, source, count, stride);
                } else if (comp == 3) {
                    convert<uint16_t, 1, 3>(target, source, count, stride);
                } else {
                    convert<uint16_t, 1>(target, source, count, comp, stride);
                }
            }
            return required;
        }
        case ComponentType::HALF: {
            const uint32_t stride = mConfig.inputStrideBytes ? mConfig.inputStrideBytes : (2 * comp);
            uint8_t const* srcBytes = (uint8_t const*) source;
            for (size_t i = 0; i < count; ++i, target += comp, srcBytes += stride) {
                half const* src = (half const*) srcBytes;
                for (int n = 0; n < comp; ++n) {
                    target[n] = float(src[n]);
                }
            }
            return required;
        }
        case ComponentType::FLOAT: {
            const uint32_t srcStride =
                    mConfig.inputStrideBytes ? mConfig.inputStrideBytes : (4 * comp);
            uint8_t const* srcBytes = (uint8_t const*) source;
            for (size_t i = 0; i < count; ++i, target += comp, srcBytes += srcStride) {
                // This will never break alignment rules because the glTF spec stipulates that the
                // stride must be a multiple of the component size.
                float const* src = (float const*) srcBytes;
                for (int n = 0; n < comp; ++n) {
                    target[n] = src[n];
                }
            }
            return required;
        }
    }
    return 0;
}

} // namespace geometry
} // namespace filament

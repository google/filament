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

#include <backend/PixelBufferDescriptor.h>

#include <math/scalar.h>
#include <math/half.h>

#include <utils/debug.h>
#include <utils/Logger.h>

#include <cstdint>
#include <cstring>
#include <stddef.h>
#include <stdint.h>

namespace filament {
namespace backend {

namespace {

// Provides an alpha value when expanding 3-channel images to 4-channel.
// Also used as a normalization scale when converting between numeric types.
template<typename componentType> inline componentType getMaxValue();

template<> inline constexpr float getMaxValue() { return 1.0f; }
template<> inline constexpr int32_t getMaxValue() { return 0x7fffffff; }
template<> inline constexpr uint32_t getMaxValue() { return 0xffffffff; }
template<> inline constexpr uint16_t getMaxValue() { return 0x3c00; } // 0x3c00 is 1.0 in half-float.
template<> inline constexpr uint8_t getMaxValue() { return 0xff; }
template<> inline math::half getMaxValue() { return math::half(1.0f); }

template<typename srcComponentType, typename dstComponentType, uint8_t srcChannelCount,
        uint8_t dstChannelCount>
void reshapeImageKernel(uint8_t* UTILS_RESTRICT dest, const uint8_t* UTILS_RESTRICT src,
        size_t srcBytesPerRow, size_t dstRowOffset, size_t dstColumnOffset, size_t dstBytesPerRow,
        size_t width, size_t height, bool swizzle) {
    constexpr size_t minChannelCount = math::min(srcChannelCount, dstChannelCount);
    const dstComponentType dstMaxValue = getMaxValue<dstComponentType>();
    const srcComponentType srcMaxValue = getMaxValue<srcComponentType>();
    double const mFactor = dstMaxValue / ((double) srcMaxValue);
    assert_invariant(minChannelCount <= 4);
    UTILS_ASSUME(minChannelCount <= 4);
    dest += (dstRowOffset * dstBytesPerRow);

    const int inds[4] = { swizzle ? 2 : 0, 1, swizzle ? 0 : 2, 3 };
    for (size_t row = 0; row < height; ++row) {
        const srcComponentType* in = (const srcComponentType*) src;
        dstComponentType* out = (dstComponentType*) dest + (dstColumnOffset * dstChannelCount);
        for (size_t column = 0; column < width; ++column) {
            for (size_t channel = 0; channel < minChannelCount; ++channel) {
                if constexpr (std::is_same_v<dstComponentType, srcComponentType>) {
                    out[channel] = in[inds[channel]];
                } else {
                    // convert to double then clamp and cast to dst type.
                    out[channel] = static_cast<dstComponentType>(std::clamp(
                            in[inds[channel]] * mFactor, 0.0,
                            static_cast<double>(std::numeric_limits<dstComponentType>::max())));
                }
            }
            if constexpr (srcChannelCount == 1 && (dstChannelCount == 3 || dstChannelCount == 4)) {
                // For the special case of src-channel-count=1, dst-channel-count=3/4, we assume
                // the output should be a grayscale image with alpha=1 and the gray value is set
                // to the src value. This is useful for producing a grayscale image from a depth
                // map.
                for (size_t channel = 1; channel < 3; ++channel) {
                    out[channel] = out[0];
                }
                if constexpr (dstChannelCount == 4) {
                    out[3] = dstMaxValue;
                }
            } else {
                for (size_t channel = srcChannelCount; channel < dstChannelCount; ++channel) {
                    out[channel] = dstMaxValue;
                }
            }
            in += srcChannelCount;
            out += dstChannelCount;
        }
        src += srcBytesPerRow;
        dest += dstBytesPerRow;
    }
}

// Converts a n-channel image of UBYTE, INT, UINT, HALF, or FLOAT to a different type.
template<typename dstComponentType, typename srcComponentType>
static void reshapeImageImpl(uint8_t* UTILS_RESTRICT dest, const uint8_t* UTILS_RESTRICT src,
        size_t srcBytesPerRow, size_t srcChannelCount, size_t dstRowOffset, size_t dstColumnOffset,
        size_t dstBytesPerRow, size_t dstChannelCount, size_t width, size_t height, bool swizzle) {

    static_assert(!std::is_same_v<dstComponentType, math::half>);

    void (*impl)(uint8_t* dest, const uint8_t* src, size_t srcBytesPerRow, size_t srcRowOffset,
            size_t srcColumnOffset, size_t dstBytesPerRow, size_t width, size_t height,
            bool swizzle) = nullptr;

    auto channelCountError = [srcChannelCount, dstChannelCount]() {
        LOG(ERROR) << "DataReshaper: src/dst channel count not supported : " << srcChannelCount
                   << "/" << dstChannelCount;
    };

    if (srcChannelCount == 1) {
        switch (dstChannelCount) {
            case 1: impl = reshapeImageKernel<srcComponentType, dstComponentType, 1, 1>; break;
            case 2: impl = reshapeImageKernel<srcComponentType, dstComponentType, 1, 2>; break;
            case 3: impl = reshapeImageKernel<srcComponentType, dstComponentType, 1, 3>; break;
            case 4: impl = reshapeImageKernel<srcComponentType, dstComponentType, 1, 4>; break;
            default: channelCountError(); break;
        }
    } else if (srcChannelCount == 2) {
        switch (dstChannelCount) {
            case 1: impl = reshapeImageKernel<srcComponentType, dstComponentType, 2, 1>; break;
            case 2: impl = reshapeImageKernel<srcComponentType, dstComponentType, 2, 2>; break;
            case 3: impl = reshapeImageKernel<srcComponentType, dstComponentType, 2, 3>; break;
            case 4: impl = reshapeImageKernel<srcComponentType, dstComponentType, 2, 4>; break;
            default: channelCountError(); break;
        }
    } else if (srcChannelCount == 3) {
        switch (dstChannelCount) {
            case 1: impl = reshapeImageKernel<srcComponentType, dstComponentType, 3, 1>; break;
            case 2: impl = reshapeImageKernel<srcComponentType, dstComponentType, 3, 2>; break;
            case 3: impl = reshapeImageKernel<srcComponentType, dstComponentType, 3, 3>; break;
            case 4: impl = reshapeImageKernel<srcComponentType, dstComponentType, 3, 4>; break;
            default: channelCountError(); break;
        }
    } else if (srcChannelCount == 4) {
        switch (dstChannelCount) {
            case 1: impl = reshapeImageKernel<srcComponentType, dstComponentType, 4, 1>; break;
            case 2: impl = reshapeImageKernel<srcComponentType, dstComponentType, 4, 2>; break;
            case 3: impl = reshapeImageKernel<srcComponentType, dstComponentType, 4, 3>; break;
            case 4: impl = reshapeImageKernel<srcComponentType, dstComponentType, 4, 4>; break;
            default: channelCountError(); break;
        }
    } else {
        channelCountError();
    }

    if (impl) {
        impl(dest, src, srcBytesPerRow, dstRowOffset, dstColumnOffset, dstBytesPerRow, width,
                height, swizzle);
    }
}

} // anonymous namespace

class DataReshaper {
public:

    // Adds padding to multi-channel interleaved data by inserting dummy values, or discards
    // trailing channels. This is useful for platforms that only accept 4-component data, since
    // users often wish to submit (or receive) 3-component data.
    template<typename componentType, size_t srcChannelCount, size_t dstChannelCount>
    static void reshape(void* UTILS_RESTRICT dest, const void* UTILS_RESTRICT src,
            size_t numSrcBytes) {
        const componentType maxValue = getMaxValue<componentType>();
        const componentType* in = (const componentType*) src;
        componentType* out = (componentType*) dest;
        const size_t width = (numSrcBytes / sizeof(componentType)) / srcChannelCount;
        constexpr size_t minChannelCount = math::min(srcChannelCount, dstChannelCount);
        for (size_t column = 0; column < width; ++column) {
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

    static void copyImage(uint8_t* UTILS_RESTRICT dest,
                          const uint8_t* UTILS_RESTRICT src,
                          size_t srcBytesPerRow, size_t /*srcChannelCount*/,
                          size_t /*dstRowOffset */, size_t /*dstColumnOffset */,
                          size_t dstBytesPerRow, size_t /*dstChannelCount*/,
                          size_t /*width*/, size_t height, bool /*swizzle*/) {
        if (srcBytesPerRow == dstBytesPerRow) {
            std::memcpy(dest, src, height * srcBytesPerRow);
            return;
        }
        const size_t minBytesPerRow = std::min(srcBytesPerRow, dstBytesPerRow);
        for (size_t i = 0; i < height; ++i, src += srcBytesPerRow, dest += dstBytesPerRow) {
            std::memcpy(dest, src, minBytesPerRow);
        }
    }

    // Converts a n-channel image of UBYTE, INT, UINT, or FLOAT to a different type.
    static bool reshapeImage(PixelBufferDescriptor* UTILS_RESTRICT dst, PixelDataType srcType,
            uint32_t srcChannelCount, const uint8_t* UTILS_RESTRICT srcBytes, int srcBytesPerRow,
            int width, int height, bool swizzle) {
        size_t dstChannelCount;
        switch (dst->format) {
            case PixelDataFormat::R_INTEGER: dstChannelCount = 1; break;
            case PixelDataFormat::RG_INTEGER: dstChannelCount = 2; break;
            case PixelDataFormat::RGB_INTEGER: dstChannelCount = 3; break;
            case PixelDataFormat::RGBA_INTEGER: dstChannelCount = 4; break;
            case PixelDataFormat::R: dstChannelCount = 1; break;
            case PixelDataFormat::RG: dstChannelCount = 2; break;
            case PixelDataFormat::RGB: dstChannelCount = 3; break;
            case PixelDataFormat::RGBA: dstChannelCount = 4; break;
            default:
                LOG(ERROR) << "DataReshaper: unsupported dst->format: " << (int) dst->format;
                return false;
        }
        void (*reshaper)(uint8_t* dest, const uint8_t* src, size_t srcBytesPerRow,
                size_t srcChannelCount, size_t srcRowOffset, size_t srcColumnOffset,
                size_t dstBytesPerRow, size_t dstChannelCount, size_t width, size_t height,
                bool swizzle) = nullptr;
        constexpr auto UBYTE = PixelDataType::UBYTE;
        constexpr auto FLOAT = PixelDataType::FLOAT;
        constexpr auto UINT = PixelDataType::UINT;
        constexpr auto INT = PixelDataType::INT;
        constexpr auto HALF = PixelDataType::HALF;
        switch (dst->type) {
            case UBYTE:
                switch (srcType) {
                    case UBYTE:
                        reshaper = reshapeImageImpl<uint8_t, uint8_t>;
                        if (dst->format == PixelDataFormat::RGBA &&
                                dstChannelCount == srcChannelCount && !swizzle && dst->top == 0 &&
                                dst->left == 0) {
                            reshaper = copyImage;
                        }
                        break;
                    case FLOAT: reshaper = reshapeImageImpl<uint8_t, float>; break;
                    case INT: reshaper = reshapeImageImpl<uint8_t, int32_t>; break;
                    case UINT: reshaper = reshapeImageImpl<uint8_t, uint32_t>; break;
                    case HALF: reshaper = reshapeImageImpl<uint8_t, math::half>; break;
                    default:
                        LOG(ERROR) << "DataReshaper: UBYTE dst, unsupported srcType: "
                                   << (int) srcType;
                        return false;
                }
                break;
            case FLOAT:
                switch (srcType) {
                    case UBYTE: reshaper = reshapeImageImpl<float, uint8_t>; break;
                    case FLOAT: reshaper = reshapeImageImpl<float, float>; break;
                    case INT: reshaper = reshapeImageImpl<float, int32_t>; break;
                    case UINT: reshaper = reshapeImageImpl<float, uint32_t>; break;
                    default:
                        LOG(ERROR) << "DataReshaper: FLOAT dst, unsupported srcType: "
                                   << (int) srcType;
                        return false;
                }
                break;
            case INT:
                switch (srcType) {
                    case UBYTE: reshaper = reshapeImageImpl<int32_t, uint8_t>; break;
                    case FLOAT: reshaper = reshapeImageImpl<int32_t, float>; break;
                    case INT: reshaper = reshapeImageImpl<int32_t, int32_t>; break;
                    case UINT: reshaper = reshapeImageImpl<int32_t, uint32_t>; break;
                    default:
                        LOG(ERROR)
                                << "DataReshaper: INT dst, unsupported srcType: " << (int) srcType;
                        return false;
                }
                break;
            case UINT:
                switch (srcType) {
                    case UBYTE: reshaper = reshapeImageImpl<uint32_t, uint8_t>; break;
                    case FLOAT: reshaper = reshapeImageImpl<uint32_t, float>; break;
                    case INT: reshaper = reshapeImageImpl<uint32_t, int32_t>; break;
                    case UINT: reshaper = reshapeImageImpl<uint32_t, uint32_t>; break;
                    default:
                        LOG(ERROR)
                                << "DataReshaper: UINT dst, unsupported srcType: " << (int) srcType;
                        return false;
                }
                break;
            case HALF:
                switch (srcType) {
                    case HALF:
                        reshaper = copyImage;
                        break;
                    default:
                        LOG(ERROR)
                                << "DataReshaper: HALF dst, unsupported srcType: " << (int) srcType;
                        return false;
                }
                break;
            default:
                LOG(ERROR) << "DataReshaper: unsupported dst->type: " << (int) dst->type;
                return false;
        }
        uint8_t* dstBytes = (uint8_t*) dst->buffer;
        const int dstBytesPerRow = PixelBufferDescriptor::computeDataSize(dst->format, dst->type,
                dst->stride ? dst->stride : width, 1, dst->alignment);
        reshaper(dstBytes, srcBytes, srcBytesPerRow, srcChannelCount, dst->top, dst->left,
                dstBytesPerRow, dstChannelCount, width, height, swizzle);

        return true;
    }
};

} // namespace backend
} // namespace filament

#endif // TNT_FILAMENT_DRIVER_DATARESHAPER_H

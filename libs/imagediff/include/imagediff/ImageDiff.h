/*
 * Copyright (C) 2026 The Android Open Source Project
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

#ifndef IMAGEDIFF_IMAGEDIFF_H
#define IMAGEDIFF_IMAGEDIFF_H

#include <image/LinearImage.h>
#include <utils/CString.h>

#include <cstdint>
#include <vector>

namespace imagediff {

enum class Channel : uint8_t { R = 0x1, G = 0x2, B = 0x4, A = 0x8, RGB = 0x7, RGBA = 0xF };

struct ImageDiffConfig {
    enum class Mode : uint8_t { LEAF, AND, OR } mode = Mode::LEAF;
    enum class Swizzle : uint8_t { RGBA, BGRA } swizzle = Swizzle::RGBA;

    // For LEAF mode:
    uint8_t channelMask = (uint8_t) Channel::RGBA;
    float maxAbsDiff = 0.0f; // Max absolute difference allowed (e.g. 5/255)

    // For AND/OR mode:
    std::vector<ImageDiffConfig> children;

    // Global Logic (Checked at the root level mainly)
    float maxFailingPixelsFraction = 0.0f; // 0.0 = strict, 0.01 = 1% pixels can fail
};

struct ImageDiffResult {

    enum class Status : uint8_t {
        PASSED = 0,
        SIZE_MISMATCH,
        PIXEL_DIFFERENCE
    } status = Status::PASSED;


    size_t failingPixelCount = 0;

    size_t maskedIgnoredPixelCount = 0; // Pixels that passed only due to masking

    float maxDiffFound[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; // [R, G, B, A]

    image::LinearImage diffImage; // Populated if generateDiffImage is true

    image::LinearImage maskImage; // Populated if generateDiffImage is true and mask is present
};

/**
 * Compares two images.
 * @param reference  The golden image (expected).
 * @param candidate  The image to test (actual).
 * @param config     The tolerance configuration.
 * @param mask       Optional: Grayscale image where 0=Ignore, 1=Strict (or vice versa).
 *                   If null, no masking is applied.
 * @param generateDiffImage If true, populates result.diffImage with visual diff.
 */
ImageDiffResult compare(image::LinearImage const& reference, image::LinearImage const& candidate,
        ImageDiffConfig const& config, image::LinearImage const* mask = nullptr,
        bool generateDiffImage = false);

// 8-bit Support
struct Bitmap {
    uint32_t width;
    uint32_t height;
    size_t stride; // in bytes
    void const* data;
};

/**
 * Overload for raw 8-bit RGBA buffers (packed uint32_t).
 * Internal implementation will unpack to float for threshold consistency.
 */
ImageDiffResult compare(Bitmap const& reference, Bitmap const& candidate,
        ImageDiffConfig const& config, Bitmap const* mask = nullptr,
        bool generateDiffImage = false);

// JSON Serialization
bool parseConfig(char const* json, size_t size, ImageDiffConfig* outConfig);
utils::CString serializeResult(ImageDiffResult const& result);

} // namespace imagediff

#endif // IMAGEDIFF_IMAGEDIFF_H

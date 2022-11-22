/*
 * Copyright 2018 The Android Open Source Project
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

#include <imageio/ImageDiffer.h>

#include <image/ColorTransform.h>
#include <image/ImageOps.h>
#include <imageio/ImageDecoder.h>
#include <imageio/ImageEncoder.h>
#include <utils/Panic.h>

#include <fstream>

namespace image {

// TODO: Remove special treatment of 1-channel data.
void updateOrCompare(LinearImage limgResult, const utils::Path& fnameGolden,
        ComparisonMode mode, float epsilon) {
    if (mode == ComparisonMode::SKIP) {
        return;
    }

    // Regenerate the PNG file at the given path.
    if (mode == ComparisonMode::UPDATE) {
        std::ofstream out(fnameGolden, std::ios::binary | std::ios::trunc);
        auto format = ImageEncoder::Format::PNG_LINEAR;
        if (fnameGolden.getExtension() == "rgbm") {
            format = ImageEncoder::Format::RGBM;
        }
        if (limgResult.getChannels() != 1) {
            ImageEncoder::encode(out, format, limgResult, "", fnameGolden);
        } else {
            auto limg2 = combineChannels({limgResult, limgResult, limgResult});
            ImageEncoder::encode(out, format, limg2, "", fnameGolden);
        }
        return;
    }

    // Load the PNG file at the given path.
    std::ifstream in(fnameGolden, std::ios::binary);
    ASSERT_PRECONDITION(in, "Unable to open: %s", fnameGolden.c_str());
    LinearImage limgGolden = ImageDecoder::decode(in, fnameGolden);

    // Convert 4-channel RGBM into proper RGB.
    if (fnameGolden.getExtension() == "rgbm" && limgGolden.getChannels() == 4) {
        limgGolden = toLinearFromRGBM(
                reinterpret_cast< filament::math::float4 const*>(limgGolden.getPixelRef()),
                limgGolden.getWidth(), limgGolden.getHeight());
    }

    // Expand the result image from L to RGB.
    if (limgResult.getChannels() == 1) {
        limgResult = combineChannels({limgResult, limgResult, limgResult});
    }

    // Perform a simple comparison of the two images.
    ASSERT_PRECONDITION(compare(limgResult, limgGolden, epsilon) == 0, "Image mismatch.");
}

}

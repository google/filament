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

// TODO: (1) Remove usage of the old Image class. (2) Remove special treatment of 1-channel data.
void updateOrCompare(LinearImage limgResult, const utils::Path& fnameGolden,
        ComparisonMode mode, float epsilon) {
    if (mode == ComparisonMode::SKIP) {
        return;
    }

    // Regenerate the PNG file at the given path.
    if (mode == ComparisonMode::UPDATE) {
        std::ofstream out(fnameGolden, std::ios::binary | std::ios::trunc);
        const size_t width = limgResult.getWidth();
        const size_t height = limgResult.getHeight();
        const size_t nchan = limgResult.getChannels();
        const size_t bpp = nchan * sizeof(float), bpr = width * bpp, nbytes = bpr * height;
        std::unique_ptr<uint8_t[]> data(new uint8_t[nbytes]);

        auto format = ImageEncoder::Format::PNG_LINEAR;
        if (fnameGolden.getExtension() == "rgbm" && nchan == 3) {
            format = ImageEncoder::Format::RGBM;
        }

        if (nchan != 1) {
            memcpy(data.get(), limgResult.getPixelRef(), nbytes);
            Image im(std::move(data), width, height, bpr, bpp, nchan);
            ImageEncoder::encode(out, format, im, "", fnameGolden);
        } else {
            auto limg2 = combineChannels({limgResult, limgResult, limgResult});
            memcpy(data.get(), limg2.getPixelRef(), nbytes);
            Image im(std::move(data), width, height, bpr, bpp, nchan);
            ImageEncoder::encode(out, format, im, "", fnameGolden);
        }
        return;
    }

    // Load the PNG file at the given path.
    std::ifstream in(fnameGolden, std::ios::binary);
    ASSERT_PRECONDITION(in, "Unable to open: %s", fnameGolden.c_str());
    Image imgGolden = ImageDecoder::decode(in, fnameGolden, ImageDecoder::ColorSpace::LINEAR);
    const size_t width = imgGolden.getWidth(), height = imgGolden.getHeight();
    const size_t nchan = imgGolden.getChannelsCount();

    // Convert 4-channel RGBM into proper RGB.
    LinearImage limgGolden;
    if (fnameGolden.getExtension() == "rgbm" && nchan == 4) {
        limgGolden = toLinearFromRGBM(
                static_cast<math::float4 const*>(imgGolden.getData()),
                imgGolden.getWidth(), imgGolden.getHeight());
    } else {
        limgGolden = LinearImage(width, height, nchan);
        memcpy(limgGolden.getPixelRef(), imgGolden.getData(),
                width * height * sizeof(float) * nchan);
    }

    // Expand the result image from L to RGB.
    if (limgResult.getChannels() == 1) {
        limgResult = combineChannels({limgResult, limgResult, limgResult});
    }

    // Perform a simple comparison of the two images.
    ASSERT_PRECONDITION(compare(limgResult, limgGolden, epsilon) == 0, "Image mismatch.");
}

}

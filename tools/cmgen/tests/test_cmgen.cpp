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

#include <image/ColorTransform.h>
#include <image/ImageOps.h>
#include <image/ImageSampler.h>
#include <image/LinearImage.h>

#include <imageio/ImageDecoder.h>
#include <imageio/ImageEncoder.h>
#include <imageio/ImageDiffer.h>

#include <gtest/gtest.h>

#include <utils/Panic.h>
#include <utils/Path.h>
#include <math/vec3.h>

#include <cstdlib>
#include <fstream>
#include <string>
#include <sstream>

using std::string;
using utils::Path;

using namespace image;

class CmgenTest : public testing::Test {};

static ComparisonMode g_comparisonMode;

TEST_F(CmgenTest, WhiteFurnace) { // NOLINT

    Path comparisonPath = "../../samples/envs/white_furnace";
    Path inputPath = "../../assets/environments/white_furnace/white_furnace.exr";

    const string resultImageFilename = "white_furnace/nx.rgbm";
    const string goldenImageFilename = comparisonPath + "nx.rgbm";

    std::cerr << "Running cmgen on " << inputPath << "...\n";
    string cmdline = string("tools/cmgen/cmgen -x . ") + inputPath.c_str();
    ASSERT_EQ(std::system(cmdline.c_str()), 0);

    std::cerr << "Reading result image from " << resultImageFilename << "...\n";
    std::ifstream resultStream(resultImageFilename.c_str(), std::ios::binary);
    Image resultImage = ImageDecoder::decode(resultStream, resultImageFilename);
    ASSERT_EQ(resultImage.isValid(), true);
    ASSERT_EQ(resultImage.getChannelsCount(), 4);

    std::cerr << "Golden image is at " << goldenImageFilename << "\n";
    LinearImage resultLImage(resultImage.getWidth(), resultImage.getHeight(), 4);
    memcpy(resultLImage.getPixelRef(), resultImage.getData(),
            resultImage.getWidth() * resultImage.getHeight() * sizeof(float) * 4);

    updateOrCompare(resultLImage, goldenImageFilename, g_comparisonMode, 0.01f);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    if (argc != 2) {
        return 1;
    }
    string command(argv[1]);
    if (command == "update") {
        g_comparisonMode = ComparisonMode::UPDATE;
    } else if (command == "compare") {
        g_comparisonMode = ComparisonMode::COMPARE;
    } else {
        return 1;
    }
    return RUN_ALL_TESTS();
}

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

static void checkFileExistence(string path) {
    std::ifstream s(path.c_str(), std::ios::binary);
    if (!s) {
        std::cerr << "ERROR file does not exist: " << path << std::endl;
        exit(1);
    }
}

// This spawns cmgen, telling it to process the environment map located at "inputPath". It creates
// an output folder in the same location as the test executable, which lets us avoid polluting our
// local source tree with output files. The given "resultPath" points the specific newly-generated
// output image that we'd like to compare or update, and the "goldenPath" points to the golden image
// (which lives in our source tree).
static void spawnTool(string inputPath, string resultPath, string goldenPath) {
    const string executableFolder = Path::getCurrentExecutable().getParent();
    inputPath = Path::getCurrentDirectory() + inputPath;
    resultPath = Path::getCurrentExecutable().getParent() + resultPath;
    goldenPath = Path::getCurrentDirectory() + goldenPath;

    std::cout << "Running cmgen on " << inputPath << std::endl;
    checkFileExistence(inputPath);
    string cmdline = executableFolder + "cmgen -x " + executableFolder + " " + inputPath;
    ASSERT_EQ(std::system(cmdline.c_str()), 0);

    std::cout << "Reading result image from " << resultPath << std::endl;
    checkFileExistence(resultPath);
    std::ifstream resultStream(resultPath.c_str(), std::ios::binary);
    Image resultImage = ImageDecoder::decode(resultStream, resultPath);
    ASSERT_EQ(resultImage.isValid(), true);
    ASSERT_EQ(resultImage.getChannelsCount(), 4);
    LinearImage resultLImage = toLinearFromRGBM(
            static_cast<math::float4 const*>(resultImage.getData()),
            resultImage.getWidth(), resultImage.getHeight());

    std::cout << "Golden image is at " << goldenPath << std::endl;
    updateOrCompare(resultLImage, goldenPath, g_comparisonMode, 0.01f);
}

TEST_F(CmgenTest, HdrLatLong) { // NOLINT
    const string inputPath = "assets/environments/white_furnace/white_furnace.exr";
    const string resultPath = "white_furnace/nx.rgbm";
    const string goldenPath = "samples/envs/white_furnace/nx.rgbm";
    spawnTool(inputPath, resultPath, goldenPath);
}

TEST_F(CmgenTest, LdrCrossCube) { // NOLINT
    const string inputPath = "tools/cmgen/tests/Footballfield/Footballfield.png";
    const string resultPath = "Footballfield/m3_nx.rgbm";
    const string goldenPath = "tools/cmgen/tests/Footballfield/m3_nx.rgbm";
    spawnTool(inputPath, resultPath, goldenPath);
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

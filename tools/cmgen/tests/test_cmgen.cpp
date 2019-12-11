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
#include <regex>
#include <string>
#include <sstream>
#include <streambuf>

using std::string;
using utils::Path;
using namespace filament::math;

using namespace image;

class CmgenTest : public testing::Test {};

static ComparisonMode g_comparisonMode;

static void checkFileExistence(const string& path) {
    std::ifstream s(path.c_str(), std::ios::binary);
    if (!s) {
        std::cerr << "ERROR file does not exist: " << path << std::endl;
        exit(1);
    }
}

// Reads the entire content of the specified file
static string readFile(const Path& inputPath) {
    std::ifstream t(inputPath);
    string s;

    // Pre-allocate the memory
    t.seekg(0, std::ios::end);
    s.reserve((size_t) t.tellg());
    t.seekg(0, std::ios::beg);

    // Copy the file content into the string
    s.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
    return s;
}

// This spawns cmgen, telling it to process the environment map located at "inputPath".
// The supplied parameters are passed to cmgen as is. The commandSuffix is added at
// the end of the command line; it is useful to redirect the output to a file for instance
static void launchTool(string inputPath,
        const string& parameters = "", const string& commandSuffix = "") {
    const string executableFolder = Path::getCurrentExecutable().getParent();
    inputPath = Path::getCurrentDirectory() + inputPath;

    std::cout << "Running cmgen on " << inputPath << std::endl;
    checkFileExistence(inputPath);
    string cmdline = executableFolder + "cmgen " + parameters +
            " " + inputPath + " " + commandSuffix;
    ASSERT_EQ(std::system(cmdline.c_str()), 0);
}

// This spawns cmgen, telling it to process the environment map located at "inputPath". It creates
// an output folder in the same location as the test executable, which lets us avoid polluting our
// local source tree with output files. The given "resultPath" points the specific newly-generated
// output image that we'd like to compare or update, and the "goldenPath" points to the golden image
// (which lives in our source tree).
static void processEnvMap(string inputPath, string resultPath, string goldenPath) {
    const string executableFolder = Path::getCurrentExecutable().getParent();
    resultPath = Path::getCurrentExecutable().getParent() + resultPath;
    goldenPath = Path::getCurrentDirectory() + goldenPath;

    launchTool(std::move(inputPath), "--quiet -f rgbm -x " + executableFolder);

    std::cout << "Reading result image from " << resultPath << std::endl;
    checkFileExistence(resultPath);
    std::ifstream resultStream(resultPath.c_str(), std::ios::binary);
    LinearImage resultImage = ImageDecoder::decode(resultStream, resultPath);
    ASSERT_EQ(resultImage.isValid(), true);
    ASSERT_EQ(resultImage.getChannels(), 4);
    LinearImage resultLImage = toLinearFromRGBM(
            reinterpret_cast<filament::math::float4 const*>(resultImage.getPixelRef()),
            resultImage.getWidth(), resultImage.getHeight());

    std::cout << "Golden image is at " << goldenPath << std::endl;
    updateOrCompare(resultLImage, goldenPath, g_comparisonMode, 0.01f);
}

static void compareSh(const string& content, const string& regex,
        const float3& match, float epsilon = 1e-5f) {
    std::smatch smatch;
    if (std::regex_search(content, smatch, std::regex(regex))) {
        float3 sh(std::stof(smatch[1]), std::stof(smatch[2]), std::stof(smatch[3]));
        ASSERT_TRUE(all(lessThan(abs(sh - match), float3{epsilon})));
    }
}

TEST_F(CmgenTest, SphericalHarmonics) { // NOLINT
    const string inputPath = "assets/environments/white_furnace/white_furnace.exr";
    const string resultPath = Path::getCurrentExecutable().getParent() + "white_furnace_sh.txt";
    launchTool(inputPath, "--quiet --sh=3 --sh-shader", "> " + resultPath);

    string content = readFile(resultPath);
    string vec3(R"(\(\s+([-+0-9.]+),\s+([-+0-9.]+),\s+([-+0-9.]+)\); // )");

    compareSh(content, vec3 + "L00",  float3{1.0f});
    compareSh(content, vec3 + "L1-1", float3{0.0f});
    compareSh(content, vec3 + "L10",  float3{0.0f});
    compareSh(content, vec3 + "L11",  float3{0.0f});
    compareSh(content, vec3 + "L2-2", float3{0.0f});
    compareSh(content, vec3 + "L2-1", float3{0.0f});
    compareSh(content, vec3 + "L20",  float3{0.0f});
    compareSh(content, vec3 + "L21",  float3{0.0f});
    compareSh(content, vec3 + "L22",  float3{0.0f});
}

TEST_F(CmgenTest, HdrLatLong) { // NOLINT
    const string inputPath = "assets/environments/white_furnace/white_furnace.exr";
    const string resultPath = "white_furnace/nx.rgbm";
    const string goldenPath = "tools/cmgen/tests/white_furnace_nx.rgbm";
    processEnvMap(inputPath, resultPath, goldenPath);
}

TEST_F(CmgenTest, LdrCrossCube) { // NOLINT
    const string inputPath = "tools/cmgen/tests/Footballfield/Footballfield.png";
    const string resultPath = "Footballfield/m3_nx.rgbm";
    const string goldenPath = "tools/cmgen/tests/Footballfield/m3_nx.rgbm";
    processEnvMap(inputPath, resultPath, goldenPath);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    if (argc != 2) {
        std::cerr << "test_cmgen [compare|update]" << std::endl;
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

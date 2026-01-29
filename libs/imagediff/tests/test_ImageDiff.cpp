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

#include <gtest/gtest.h>

#include <image/LinearImage.h>
#include <imagediff/ImageDiff.h>

#include <vector>

using namespace imagediff;
using namespace image;

class ImageDiffTest : public testing::Test {
protected:
    LinearImage createImage(uint32_t w, uint32_t h, float val) {
        LinearImage img(w, h, 3);
        float* p = img.getPixelRef();
        for (size_t i = 0; i < w * h * 3; ++i) {
            p[i] = val;
        }
        return img;
    }

    void setPixel(LinearImage& img, uint32_t x, uint32_t y, float r, float g, float b) {
        float* p = img.getPixelRef(x, y);
        p[0] = r;
        p[1] = g;
        p[2] = b;
    }
};

TEST_F(ImageDiffTest, ExactMatch) {
    LinearImage img1 = createImage(10, 10, 0.5f);
    LinearImage img2 = createImage(10, 10, 0.5f);

    ImageDiffConfig config;
    auto result = compare(img1, img2, config);
    EXPECT_EQ(result.status, ImageDiffResult::Status::PASSED);
    EXPECT_EQ(result.failingPixelCount, 0);
}

TEST_F(ImageDiffTest, AbsThreshold) {
    LinearImage img1 = createImage(10, 10, 0.5f);
    LinearImage img2 = createImage(10, 10, 0.5f);
    setPixel(img2, 5, 5, 0.6f, 0.5f, 0.5f); // 0.1 diff on Red

    ImageDiffConfig config;
    config.maxAbsDiff = 0.05f; // Should fail
    auto result = compare(img1, img2, config);
    EXPECT_EQ(result.status, ImageDiffResult::Status::PIXEL_DIFFERENCE);
    EXPECT_EQ(result.failingPixelCount, 1);

    config.maxAbsDiff = 0.15f; // Should pass
    result = compare(img1, img2, config);
    EXPECT_EQ(result.status, ImageDiffResult::Status::PASSED);
}

TEST_F(ImageDiffTest, Masking) {
    LinearImage img1 = createImage(10, 10, 0.5f);
    LinearImage img2 = createImage(10, 10, 0.5f);
    setPixel(img2, 5, 5, 1.0f, 0.5f, 0.5f); // Huge diff

    // Mask with 0 at 5,5
    LinearImage mask(10, 10, 1);
    float* mp = mask.getPixelRef();
    for (int i = 0; i < 100; ++i) mp[i] = 1.0f;
    *mask.getPixelRef(5, 5) = 0.0f;

    ImageDiffConfig config;
    config.maxAbsDiff = 0.1f;

    auto result = compare(img1, img2, config, &mask);
    EXPECT_EQ(result.status, ImageDiffResult::Status::PASSED); // Mask ignored the error
}

TEST_F(ImageDiffTest, LogicAndOr) {
    LinearImage img1 = createImage(1, 1, 0.5f);
    LinearImage img2 = createImage(1, 1, 0.6f); // Diff 0.1

    // OR Mode: Fail child 1, Pass child 2 -> Should PASS
    ImageDiffConfig configOR;
    configOR.mode = ImageDiffConfig::Mode::OR;
    configOR.children.resize(2);

    // Child 1: Strict (Fail)
    configOR.children[0].maxAbsDiff = 0.05f;
    // Child 2: Relaxed (Pass)
    configOR.children[1].maxAbsDiff = 0.15f;

    auto result = compare(img1, img2, configOR);
    EXPECT_EQ(result.status, ImageDiffResult::Status::PASSED);

    // AND Mode: Pass child 1, Fail child 2 -> Should FAIL
    ImageDiffConfig configAND;
    configAND.mode = ImageDiffConfig::Mode::AND;
    configAND.children.resize(2);

    // Child 1: Relaxed (Pass)
    configAND.children[0].maxAbsDiff = 0.15f;
    // Child 2: Strict (Fail)
    configAND.children[1].maxAbsDiff = 0.05f;

    result = compare(img1, img2, configAND);
    EXPECT_EQ(result.status, ImageDiffResult::Status::PIXEL_DIFFERENCE);
}

TEST_F(ImageDiffTest, GlobalFailureFraction) {
    LinearImage img1 = createImage(10, 10, 0.5f);
    LinearImage img2 = createImage(10, 10, 0.5f);

    // Fail 2 pixels (2%)
    setPixel(img2, 0, 0, 1.0f, 0.5f, 0.5f);
    setPixel(img2, 1, 0, 1.0f, 0.5f, 0.5f);

    ImageDiffConfig config;
    config.maxFailingPixelsFraction = 0.01f; // 1% allowed
    auto result = compare(img1, img2, config);
    EXPECT_EQ(result.status, ImageDiffResult::Status::PIXEL_DIFFERENCE);

    config.maxFailingPixelsFraction = 0.03f; // 3% allowed
    result = compare(img1, img2, config);
    EXPECT_EQ(result.status, ImageDiffResult::Status::PASSED);
}

TEST_F(ImageDiffTest, JSONSerialization) {
    ImageDiffConfig config;
    config.mode = ImageDiffConfig::Mode::AND;
    config.maxAbsDiff = 0.5f;
    config.children.resize(1);
    config.children[0].maxAbsDiff = 0.1f;

    // Test parsing
    char const* json = R"({
        "mode": "OR",
        "swizzle": "BGRA",
        "maxAbsDiff": "0.2",
        "children": [
            {"maxAbsDiff": "0.1"}
        ]
    })";

    ImageDiffConfig parsed;
    bool success = parseConfig(json, strlen(json), &parsed);
    EXPECT_TRUE(success);
    EXPECT_EQ(parsed.mode, ImageDiffConfig::Mode::OR);
    EXPECT_EQ(parsed.swizzle, ImageDiffConfig::Swizzle::BGRA);
    EXPECT_FLOAT_EQ(parsed.maxAbsDiff, 0.2f);
    EXPECT_EQ(parsed.children.size(), 1);
    EXPECT_FLOAT_EQ(parsed.children[0].maxAbsDiff, 0.1f);
}
TEST_F(ImageDiffTest, Uint8Test) {
    uint32_t w = 2, h = 2;
    // RGBA (Little Endian uint32 0xAABBGGRR) -> 0xFF0000FF is R=255, G=0, B=0, A=255
    std::vector<uint32_t> b1(w * h, 0xFF0000FF);
    std::vector<uint32_t> b2(w * h, 0xFF0000FF);

    Bitmap bmp1 = { w, h, w * 4, b1.data() };
    Bitmap bmp2 = { w, h, w * 4, b2.data() };

    ImageDiffConfig config;
    config.swizzle = ImageDiffConfig::Swizzle::RGBA;
    auto result = compare(bmp1, bmp2, config);
    EXPECT_EQ(result.status, ImageDiffResult::Status::PASSED);

    // Change one pixel in b2 to slight red change
    // 0xFF0000FE -> R=254. Diff 1/255 ~= 0.0039
    b2[0] = 0xFF0000FE;
    config.maxAbsDiff = 0.001f; // Should fail
    result = compare(bmp1, bmp2, config);
    EXPECT_EQ(result.status, ImageDiffResult::Status::PIXEL_DIFFERENCE);
    EXPECT_EQ(result.failingPixelCount, 1);

    config.maxAbsDiff = 0.005f; // Should pass
    result = compare(bmp1, bmp2, config);
    EXPECT_EQ(result.status, ImageDiffResult::Status::PASSED);
}

TEST_F(ImageDiffTest, BitmapMasking) {
    uint32_t w = 2, h = 2;
    std::vector<uint32_t> b1(w * h, 0xFF0000FF);
    std::vector<uint32_t> b2(w * h, 0xFF0000FF);
    b2[0] = 0xFF0000FE; // Slight diff (1/255 ~= 0.0039)

    std::vector<uint8_t> mask = { 0, 255, 255, 255 }; // Mask out the diff at pixel 0

    Bitmap bmp1 = { w, h, w * 4, b1.data() };
    Bitmap bmp2 = { w, h, w * 4, b2.data() };
    Bitmap bmpMask = { w, h, w, mask.data() };

    ImageDiffConfig config;
    config.maxAbsDiff = 0.001f;

    auto result = compare(bmp1, bmp2, config, &bmpMask);
    EXPECT_EQ(result.status, ImageDiffResult::Status::PASSED);
}

TEST_F(ImageDiffTest, SizeMismatch) {
    LinearImage img1 = createImage(10, 10, 0.5f);
    LinearImage img2 = createImage(11, 10, 0.5f);

    ImageDiffConfig config;
    auto result = compare(img1, img2, config);
    EXPECT_EQ(result.status, ImageDiffResult::Status::SIZE_MISMATCH);
    EXPECT_EQ(result.status, ImageDiffResult::Status::SIZE_MISMATCH);

    // Test 8-bit overload
    uint32_t d1[4] = { 0 };
    uint32_t d2[6] = { 0 };
    Bitmap b1 = { 2, 2, 8, d1 };
    Bitmap b2 = { 3, 2, 12, d2 };
    result = compare(b1, b2, config);
    EXPECT_EQ(result.status, ImageDiffResult::Status::SIZE_MISMATCH);
    EXPECT_EQ(result.status, ImageDiffResult::Status::SIZE_MISMATCH);
}

TEST_F(ImageDiffTest, DiffImageGeneration) {
    // Use 4-channel images to test Alpha diff
    LinearImage img1(2, 1, 4);
    LinearImage img2(2, 1, 4);

    // Clear images (Black Transparent)
    memset(img1.getPixelRef(), 0, 2 * 1 * 4 * sizeof(float));
    memset(img2.getPixelRef(), 0, 2 * 1 * 4 * sizeof(float));

    // Make them opaque
    img1.getPixelRef(0, 0)[3] = 1.0f;
    img1.getPixelRef(1, 0)[3] = 1.0f;
    img2.getPixelRef(0, 0)[3] = 1.0f;
    img2.getPixelRef(1, 0)[3] = 1.0f;

    // Pixel 0: No diff (0,0,0,1 vs 0,0,0,1)
    // Pixel 1: Diff 0.5 in R
    float* p2 = img2.getPixelRef(1, 0);
    p2[0] = 0.5f;

    // Mask for Pixel 1 = 0.5
    LinearImage mask(2, 1, 1);
    *mask.getPixelRef(1, 0) = 0.5f;

    ImageDiffConfig config;
    config.maxAbsDiff = 0.1f;

    // Enable diff generation
    auto result = compare(img1, img2, config, &mask, true);

    // Check Status (Weighted diff = 0.5 * 0.5 = 0.25 > 0.1 -> Fail)
    EXPECT_EQ(result.status, ImageDiffResult::Status::PIXEL_DIFFERENCE);
    EXPECT_EQ(result.failingPixelCount, 1);

    // Verify MaskedIgnored count (Pixel 0: Pass, Pixel 1: Fail)
    // Wait, let's make a pixel that passes ONLY due to mask.
    // Pixel 0: Diff 0.2, Mask 0.2 -> Weighted 0.04 (Pass). Unmasked 0.2 (Fail).
    float* p1_0 = img1.getPixelRef(0, 0);
    p1_0[0] = 0.0f;
    float* p2_0 = img2.getPixelRef(0, 0);
    p2_0[0] = 0.2f;
    *mask.getPixelRef(0, 0) = 0.2f;

    result = compare(img1, img2, config, &mask, true);

    // Pixel 0: Diff 0.2, Mask 0.2 -> 0.04 < 0.1 (Pass). Unmasked 0.2 > 0.1 (Fail).
    // Pixel 1: Diff 0.5, Mask 0.5 -> 0.25 > 0.1 (Fail).

    EXPECT_EQ(result.failingPixelCount, 1);       // Pixel 1 fails
    EXPECT_EQ(result.maskedIgnoredPixelCount, 1); // Pixel 0 ignored

    // Verify Diff Image Content
    // Diff Image should have unmasked diff in RGB, and alpha diff in A.
    // Pixel 0: |0 - 0.2| = 0.2 in R. A should be 0 (no alpha diff).
    float const* diffP0 = result.diffImage.getPixelRef(0, 0);
    EXPECT_FLOAT_EQ(diffP0[0], 0.2f); // R
    EXPECT_FLOAT_EQ(diffP0[3], 0.0f); // A (Mask is NOT here anymore)

    // Pixel 1: |0 - 0.5| = 0.5 in R.
    float const* diffP1 = result.diffImage.getPixelRef(1, 0);
    EXPECT_FLOAT_EQ(diffP1[0], 0.5f);

    // Verify Mask Image Content
    ASSERT_EQ(result.maskImage.getWidth(), 2);
    float const* maskP0 = result.maskImage.getPixelRef(0, 0);
    EXPECT_FLOAT_EQ(maskP0[0], 0.2f);

    float const* maskP1 = result.maskImage.getPixelRef(1, 0);
    EXPECT_FLOAT_EQ(maskP1[0], 0.5f);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

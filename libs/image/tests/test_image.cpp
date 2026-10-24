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

#include <imageio/ImageDecoder.h>
#include <imageio/ImageDiffer.h>
#include <imageio/ImageEncoder.h>

#include <image/ColorTransform.h>
#include <image/ImageOps.h>
#include <image/ImageSampler.h>
#include <image/Ktx1Bundle.h>
#include <image/LinearImage.h>

#include <utils/Panic.h>
#include <utils/Path.h>

#include <math/vec3.h>
#include <math/vec4.h>

#include <gtest/gtest.h>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using std::istringstream;
using std::string;
using std::swap;
using std::vector;

using filament::math::float3;
using filament::math::float4;

using namespace image;

class ImageTest : public testing::Test {};

static ComparisonMode g_comparisonMode;
static utils::Path g_comparisonPath;

// Just for fun, define a tiny Ray-Sphere intersector, which we'll use to generate a reasonable
// normal map for testing purposes.
struct Ray { float3 orig, dir; };
struct Sphere { float3 center; float radius2; };
static bool intersect(Ray ray, Sphere sphere, float* t);

// Creates a "size x size" normal map that looks like a hemisphere embedded in a plane.
static LinearImage createNormalMap(uint32_t size);

// Creates a "size x size" height map that looks like a hemisphere embedded in a plane.
static LinearImage createDepthMap(uint32_t size);

// Creates a tiny monochrome image from a pattern string.
static LinearImage createGrayFromAscii(const string& pattern);

// Creates a tiny RGB image from a pattern string.
static LinearImage createColorFromAscii(const string& pattern);

// Saves an image to disk or does a load-and-compare, depending on g_comparisonMode.
static void updateOrCompare(const LinearImage& limg, const utils::Path& fname);

// Subtracts two images, does an abs(), then normalizes such that min/max transform to 0/1.
static LinearImage diffImages(const LinearImage& a, const LinearImage& b);

TEST_F(ImageTest, LuminanceFilters) { // NOLINT
    auto tiny = createGrayFromAscii("000 010 000");
    ASSERT_EQ(tiny.getWidth(), 3);
    ASSERT_EQ(tiny.getHeight(), 3);
    auto src = transpose(createGrayFromAscii("01 23 45"));
    auto ref = createGrayFromAscii("024 135");
    ASSERT_EQ(src.getWidth(), 3);
    ASSERT_EQ(src.getHeight(), 2);
    for (int i = 0; i < 6; i++) {
        EXPECT_FLOAT_EQ(src.getPixelRef()[i], ref.getPixelRef()[i]);
    }
    auto row = createGrayFromAscii("010");
    auto mag1 = resampleImage(row, 6, 1, Filter::HERMITE);
    ASSERT_EQ(mag1.getWidth(), 6);
    ASSERT_EQ(mag1.getHeight(), 1);
    auto mag2 = resampleImage(row, 7, 2, Filter::HERMITE);
    ASSERT_EQ(mag2.getWidth(), 7);
    ASSERT_EQ(mag2.getHeight(), 2);
    auto box = resampleImage(tiny, 6, 6, Filter::BOX);
    auto nearest = resampleImage(tiny, 6, 6, Filter::NEAREST);
    auto ref3 = createGrayFromAscii("000000 000000 001100 001100 000000 000000");
    for (int i = 0; i < 36; i++) {
        EXPECT_FLOAT_EQ(box.getPixelRef()[i], ref3.getPixelRef()[i]);
        EXPECT_FLOAT_EQ(nearest.getPixelRef()[i], ref3.getPixelRef()[i]);
    }
    auto grays0 = resampleImage(tiny, 100, 100, Filter::GAUSSIAN_SCALARS);
    auto mag3 = transpose(resampleImage(tiny, 32, 8, Filter::GAUSSIAN_SCALARS));
    auto grays1 = resampleImage(mag3, 100, 100, Filter::NEAREST);
    updateOrCompare(horizontalStack({grays0, grays1}), "grays.png");
}

TEST_F(ImageTest, DistanceField) { // NOLINT
    auto tiny = createGrayFromAscii("100000 000000 001100 001100 000000 000000");
    auto src = resampleImage(tiny, 256, 256, Filter::BOX);
    auto presence = [] (const LinearImage& img, uint32_t col, uint32_t row, void*) {
        return img.getPixelRef(col, row)[0] ? true : false;
    };
    auto cf = computeCoordField(src, presence, nullptr);
    auto edt = edtFromCoordField(cf, true);

    float maxdist = 0;
    const uint32_t width = edt.getWidth();
    const uint32_t height = edt.getHeight();
    for (int32_t row = 0; row < height; ++row) {
        float* dst = edt.getPixelRef(0, row);
        for (uint32_t col = 0; col < width; ++col) {
            maxdist = std::max(maxdist, dst[col]);
        }
    }
    for (int32_t row = 0; row < height; ++row) {
        float* dst = edt.getPixelRef(0, row);
        for (uint32_t col = 0; col < width; ++col) {
            dst[col] /= maxdist;
        }
    }
    updateOrCompare(horizontalStack({src, edt}), "edt.png");

    tiny = createColorFromAscii("00000 01020 00400 04000 00000");
    src = resampleImage(tiny, 256, 256, Filter::MITCHELL);
    for (int32_t row = 0; row < src.getHeight(); ++row) {
        for (uint32_t col = 0; col < src.getWidth(); ++col) {
            float& r = src.getPixelRef(col, row)[0];
            float& g = src.getPixelRef(col, row)[1];
            float& b = src.getPixelRef(col, row)[2];
            bool inside = r > 0.4 || g > 0.4 || b > 0.4;
            if (!inside) {
                r = g = b = 0.4f;
            }
        }
    }

    auto isInside = [] (const LinearImage& img, uint32_t col, uint32_t row, void*) {
        float r = img.getPixelRef(col, row)[0];
        float g = img.getPixelRef(col, row)[1];
        float b = img.getPixelRef(col, row)[2];
        return !(r > 0.4 && g > 0.4 && b > 0.4);
    };
    cf = computeCoordField(src, isInside, nullptr);
    auto voronoi = voronoiFromCoordField(cf, src);

    updateOrCompare(horizontalStack({src, voronoi}), "voronoi.png");
}

TEST_F(ImageTest, ColorFilters) { // NOLINT
    // Test color space with a classic RED => GREEN color gradient.
    LinearImage color1 = createColorFromAscii("12");
    auto color2 = resampleImage(color1, 100, 100, Filter::NEAREST);
    auto color3 = resampleImage(color1, 100, 100, Filter::GAUSSIAN_SCALARS);
    auto color4 = resampleImage(color1, 100, 100, Filter::LANCZOS);
    auto color5 = diffImages(color3, color4);

    // Try enlarging a 5x5 image using MITCHELL and LANCZOS filters.
    LinearImage color6 = createColorFromAscii("44444 41014 40704 41014 44444");
    auto color6b = resampleImage(color6, 100, 100, Filter::NEAREST);
    auto color7 = resampleImage(color6, 100, 100, Filter::MITCHELL);
    auto color8 = resampleImage(color6, 100, 100, Filter::LANCZOS);
    auto color9 = resampleImage(color6, 100, 100, Filter::GAUSSIAN_SCALARS);

    // Minification tests. Each of these do a nearest magnification afterwards for visualization
    // purposes.
    auto magnify = [](LinearImage img) {
        return resampleImage(img, 100, 100, Filter::NEAREST);
    };
    auto colora = magnify(resampleImage(color9, 3, 3, Filter::NEAREST));
    auto colorb = magnify(resampleImage(color9, 1, 1, Filter::NEAREST));
    auto colorc = magnify(resampleImage(color9, 3, 3, Filter::BOX));
    auto colord = magnify(resampleImage(color9, 1, 1, Filter::BOX));

    auto colors0 = horizontalStack({color2, color3, color4, color5});
    auto colors1 = horizontalStack({color6b, color7, color8, color9});
    auto colors2 = horizontalStack({colora, colorb, colorc, colord});
    auto colors = verticalStack({colors0, colors1, colors2});

    // Even more minification tests....
    auto colore = magnify(resampleImage(colors,  5,  5, Filter::DEFAULT));
    auto colorf = magnify(resampleImage(colors, 50, 50, Filter::DEFAULT));
    auto colorg = magnify(resampleImage(colors,  5,  5, Filter::HERMITE));
    auto colorh = magnify(resampleImage(colors, 50, 50, Filter::HERMITE));
    auto colori = horizontalStack({colore, colorf, colorg, colorh});
    colors = verticalStack({colors, colori});
    updateOrCompare(colors, "colors.png");
    ASSERT_EQ(colors.getWidth(), 400);
    ASSERT_EQ(colors.getHeight(), 400);

    // Test radius multiplier (blurring).
    ImageSampler sampler;
    sampler.horizontalFilter = sampler.verticalFilter = Filter::GAUSSIAN_SCALARS;
    sampler.filterRadiusMultiplier = 1;
    auto blurred0 = resampleImage(color6b, 100, 100, sampler);
    sampler.filterRadiusMultiplier = 10;
    auto blurred1 = resampleImage(color6b, 100, 100, sampler);
    sampler.filterRadiusMultiplier = 20;
    auto blurred2 = resampleImage(color6b, 100, 100, sampler);
    auto blurred3 = resampleImage(color6b, 101, 100, sampler);
    auto blurred4 = resampleImage(color6b,  99, 100, sampler);
    auto blurred = horizontalStack({blurred0, blurred1, blurred2, blurred3, blurred4});

    // Test extraction via sourceRegion and subsequent blurring.
    sampler.sourceRegion = {0, 0.25f, 0.25f, 0.5f};
    sampler.filterRadiusMultiplier = 1;
    auto region0 = resampleImage(colors, 100, 100, sampler);
    sampler.filterRadiusMultiplier = 10;
    auto region1 = resampleImage(colors, 100, 100, sampler);
    sampler.filterRadiusMultiplier = 20;
    auto region2 = resampleImage(colors, 100, 100, sampler);
    auto region3 = resampleImage(colors, 101, 100, sampler);
    auto region4 = resampleImage(colors,  99, 100, sampler);
    auto region = horizontalStack({region0, region1, region2, region3, region4});
    blurred = verticalStack({blurred, region});
    updateOrCompare(blurred, "blurred.png");

    // Sample the reddish-white pixel in the post-blurred image.
    SingleSample result;
    computeSingleSample(colors, 0.375, 0.375, &result);
    auto red = int(result[0] * 255.0f);
    auto grn = int(result[1] * 255.0f);
    auto blu = int(result[2] * 255.0f);
    ASSERT_EQ(red, 204);
    ASSERT_EQ(grn, 200);
    ASSERT_EQ(blu, 200);
}

TEST_F(ImageTest, VectorFilters) { // NOLINT
    auto toColors = vectorsToColors;
    auto normals = createNormalMap(1024);
    auto wrong = resampleImage(toColors(normals), 16, 16, Filter::GAUSSIAN_SCALARS);
    auto right = toColors(resampleImage(normals, 16, 16, Filter::GAUSSIAN_NORMALS));
    auto diff = diffImages(wrong, right);
    auto atlas = horizontalStack({wrong, right, diff});
    atlas = resampleImage(atlas, 300, 100, Filter::NEAREST);
    updateOrCompare(atlas, "normals.png");
}

TEST_F(ImageTest, DepthFilters) { // NOLINT
    auto depths = createDepthMap(1024);
    auto wrong = resampleImage(depths, 16, 16, Filter::GAUSSIAN_SCALARS);
    auto right = resampleImage(depths, 16, 16, Filter::MINIMUM);
    auto diff = diffImages(wrong, right);
    auto atlas = horizontalStack({wrong, right, diff});
    atlas = resampleImage(atlas, 300, 100, Filter::NEAREST);
    updateOrCompare(atlas, "depths.png");
}

TEST_F(ImageTest, ImageOps) { // NOLINT
    auto finalize = [] (LinearImage image) {
        return resampleImage(image, 100, 100, Filter::NEAREST);
    };
    LinearImage x22 = [finalize] () {
        auto original = createColorFromAscii("12 34");
        auto hflipped = finalize(horizontalFlip(original));
        auto vflipped = finalize(verticalFlip(original));
        return horizontalStack({finalize(original), hflipped, vflipped});
    }();
    LinearImage x23 = [finalize] () {
        auto original = createColorFromAscii("123 456");
        auto hflipped = finalize(horizontalFlip(original));
        auto vflipped = finalize(verticalFlip(original));
        return horizontalStack({finalize(original), hflipped, vflipped});
    }();
    LinearImage x32 = [finalize] () {
        auto original = createColorFromAscii("12 34 56");
        auto hflipped = finalize(horizontalFlip(original));
        auto vflipped = finalize(verticalFlip(original));
        return horizontalStack({finalize(original), hflipped, vflipped});
    }();
    auto atlas = verticalStack({x22, x23, x32});
    updateOrCompare(atlas, "imageops.png");
}

TEST_F(ImageTest, ColorTransformRGB) { // NOLINT
    constexpr size_t w = 2;
    constexpr size_t h = 3;
    constexpr uint16_t texels[] = {
        0, 1, 2,
        3, 4, 5,
        6, 7, 8,
        9, 10, 11,
        12, 13, 14,
        20000, 40000, 60000,
    };
    constexpr size_t bpr = w * sizeof(uint16_t) * 3;
    std::unique_ptr<uint8_t[]> data(new uint8_t[h * bpr]);
    memcpy(data.get(), texels, sizeof(texels));
    LinearImage img = image::toLinear<uint16_t>(w, h, bpr, data.get());
    auto pixels = img.get<float3>();
    ASSERT_NEAR(pixels[0].x, 0.0f, 0.001f);
    ASSERT_NEAR(pixels[0].y, 0.0f, 0.001f);
    ASSERT_NEAR(pixels[0].z, 0.0f, 0.001f);
    ASSERT_NEAR(pixels[5].x, 0.07583023f, 0.001f);
    ASSERT_NEAR(pixels[5].y, 0.33077413f, 0.001f);
    ASSERT_NEAR(pixels[5].z, 0.81851715f, 0.001f);
}

TEST_F(ImageTest, ColorTransformRGBA) { // NOLINT
    constexpr size_t w = 4;
    constexpr size_t h = 1;
    constexpr uint16_t texels[] = {
        10000, 20000, 40000, 60000,
        11000, 21000, 41000, 61000,
        13000, 23000, 43000, 63000,
        15000, 25000, 45000, 65000,
    };
    constexpr size_t bpr = w * sizeof(uint16_t) * 4;
    std::unique_ptr<uint8_t[]> data(new uint8_t[h * bpr]);
    memcpy(data.get(), texels, sizeof(texels));
    LinearImage img = image::toLinearWithAlpha<uint16_t>(w, h, bpr, data.get());
    auto pixels = reinterpret_cast<float4*>(img.getPixelRef());
    ASSERT_NEAR(pixels[3].x, 0.04282892f, 0.001f);
    ASSERT_NEAR(pixels[3].y, 0.12025354f, 0.001f);
    ASSERT_NEAR(pixels[3].z, 0.42922019f, 0.001f);
    ASSERT_NEAR(pixels[3].w, 0.99183642f, 0.001f);
}

TEST_F(ImageTest, ColorTransformRGB_10_11_11_REV) { // NOLINT
    const float inf = std::numeric_limits<float>::infinity();
    const float testValues[] = {
        -inf, -100.0f, -1.0f, -1e-6f, -0.0f, 0.0f,
        1e-8f, 1e-6f, 3e-5f, 6.103515625e-5f, 1e-4f,
        0.001f, 0.1f, 0.3333333f, 0.5f, 1.0f, 2.5f, 16.0f, 100.0f, 1024.0f,
        64511.0f, 64512.0f, 65024.0f, 100000.0f, inf
    };

    constexpr uint32_t w = 7;
    constexpr uint32_t h = 5;
    constexpr size_t numValues = sizeof(testValues) / sizeof(testValues[0]);
    for (uint32_t channels : { 3u, 4u }) {
        LinearImage img(w, h, channels);
        float* p = img.getPixelRef();
        for (size_t i = 0; i < w * h; ++i) {
            p[i * channels + 0] = testValues[(i * 3 + 0) % numValues];
            p[i * channels + 1] = testValues[(i * 5 + 1) % numValues];
            p[i * channels + 2] = testValues[(i * 7 + 2) % numValues];
            if (channels == 4) {
                p[i * channels + 3] = 1.0f;
            }
        }
        auto result = image::fromLinearToRGB_10_11_11_REV(img);
        const uint32_t* packed = reinterpret_cast<const uint32_t*>(result.get());
        for (size_t i = 0; i < w * h; ++i) {
            const float3 c{ p[i * channels + 0], p[i * channels + 1], p[i * channels + 2] };
            EXPECT_EQ(packed[i], image::linearToRGB_10_11_11_REV(c));
        }
    }
}

TEST_F(ImageTest, ColorTransformAsLinear) { // NOLINT
    constexpr size_t w = 13;
    constexpr size_t h = 3;

    // Test uint8_t (exercises 16-element NEON loop + scalar tail, with row padding).
    {
        constexpr float invMax8 = 1.0f / 255.0f;
        const size_t bpr3 = w * 3 + 5;
        vector<uint8_t> src3(h * bpr3, 0xcd);
        for (size_t y = 0; y < h; ++y) {
            for (size_t i = 0; i < w * 3; ++i) {
                src3[y * bpr3 + i] = static_cast<uint8_t>((y * 53 + i * 17) & 0xff);
            }
        }
        LinearImage img3 = image::asLinear<uint8_t>(w, h, bpr3, src3.data());
        const float* dst3 = img3.getPixelRef(0, 0);
        for (size_t y = 0; y < h; ++y) {
            for (size_t i = 0; i < w * 3; ++i) {
                EXPECT_FLOAT_EQ(dst3[y * w * 3 + i], src3[y * bpr3 + i] * invMax8);
            }
        }

        const size_t bpr4 = w * 4 + 7;
        vector<uint8_t> src4(h * bpr4, 0xcd);
        for (size_t y = 0; y < h; ++y) {
            for (size_t i = 0; i < w * 4; ++i) {
                src4[y * bpr4 + i] = static_cast<uint8_t>((y * 71 + i * 29) & 0xff);
            }
        }
        LinearImage img4 = image::asLinearWithAlpha<uint8_t>(w, h, bpr4, src4.data());
        const float* dst4 = img4.getPixelRef(0, 0);
        for (size_t y = 0; y < h; ++y) {
            for (size_t i = 0; i < w * 4; ++i) {
                EXPECT_FLOAT_EQ(dst4[y * w * 4 + i], src4[y * bpr4 + i] * invMax8);
            }
        }
    }

    // Test uint16_t and ntohs variants (exercises 8-element NEON loop, 4-element step, and scalar tail).
    {
        constexpr float invMax16 = 1.0f / 65535.0f;
        const size_t stride3 = w * 3 + 3;
        const size_t bpr3 = stride3 * sizeof(uint16_t);
        vector<uint16_t> src3(h * stride3, 0xbeef);
        for (size_t y = 0; y < h; ++y) {
            for (size_t i = 0; i < w * 3; ++i) {
                src3[y * stride3 + i] = static_cast<uint16_t>((y * 4099 + i * 1319) & 0xffff);
            }
        }
        LinearImage img3 = image::asLinear<uint16_t>(w, h, bpr3,
                reinterpret_cast<const uint8_t*>(src3.data()));
        LinearImage img3Ntohs = image::asLinearNtohs<uint16_t>(w, h, bpr3,
                reinterpret_cast<const uint8_t*>(src3.data()));
        const float* dst3 = img3.getPixelRef(0, 0);
        const float* dst3Ntohs = img3Ntohs.getPixelRef(0, 0);
        for (size_t y = 0; y < h; ++y) {
            for (size_t i = 0; i < w * 3; ++i) {
                const uint16_t v = src3[y * stride3 + i];
                EXPECT_FLOAT_EQ(dst3[y * w * 3 + i], v * invMax16);
                EXPECT_FLOAT_EQ(dst3Ntohs[y * w * 3 + i], ntohs(v) * invMax16);
            }
        }

        const size_t stride4 = w * 4 + 3;
        const size_t bpr4 = stride4 * sizeof(uint16_t);
        vector<uint16_t> src4(h * stride4, 0xbeef);
        for (size_t y = 0; y < h; ++y) {
            for (size_t i = 0; i < w * 4; ++i) {
                src4[y * stride4 + i] = static_cast<uint16_t>((y * 6553 + i * 2713) & 0xffff);
            }
        }
        LinearImage img4 = image::asLinearWithAlpha<uint16_t>(w, h, bpr4,
                reinterpret_cast<const uint8_t*>(src4.data()));
        LinearImage img4Ntohs = image::asLinearWithAlphaNtohs<uint16_t>(w, h, bpr4,
                reinterpret_cast<const uint8_t*>(src4.data()));
        const float* dst4 = img4.getPixelRef(0, 0);
        const float* dst4Ntohs = img4Ntohs.getPixelRef(0, 0);
        for (size_t y = 0; y < h; ++y) {
            for (size_t i = 0; i < w * 4; ++i) {
                const uint16_t v = src4[y * stride4 + i];
                EXPECT_FLOAT_EQ(dst4[y * w * 4 + i], v * invMax16);
                EXPECT_FLOAT_EQ(dst4Ntohs[y * w * 4 + i], ntohs(v) * invMax16);
            }
        }
    }
}

TEST_F(ImageTest, ColorTransformToLinear) { // NOLINT
    constexpr size_t w = 13;
    constexpr size_t h = 3;

    // Test uint8_t (exercises 16-element NEON loop, 4-element NEON loop, and scalar tail).
    {
        constexpr float invMax8 = 1.0f / 255.0f;
        const size_t bpr3 = w * 3 + 5;
        vector<uint8_t> src3(h * bpr3, 0xcd);
        for (size_t y = 0; y < h; ++y) {
            for (size_t i = 0; i < w * 3; ++i) {
                src3[y * bpr3 + i] = static_cast<uint8_t>((y * 53 + i * 17) & 0xff);
            }
        }
        src3[0] = 0;
        src3[1] = 10;
        src3[2] = 11;
        src3[3] = 255;
        LinearImage img3 = image::toLinear<uint8_t>(w, h, bpr3, src3.data());
        const float* dst3 = img3.getPixelRef(0, 0);
        for (size_t y = 0; y < h; ++y) {
            for (size_t i = 0; i < w * 3; ++i) {
                const uint8_t v = src3[y * bpr3 + i];
                EXPECT_NEAR(dst3[y * w * 3 + i], image::sRGBToLinear(v * invMax8), 1e-6f);
            }
        }

        const size_t bpr4 = w * 4 + 7;
        vector<uint8_t> src4(h * bpr4, 0xcd);
        for (size_t y = 0; y < h; ++y) {
            for (size_t i = 0; i < w * 4; ++i) {
                src4[y * bpr4 + i] = static_cast<uint8_t>((y * 71 + i * 29) & 0xff);
            }
        }
        LinearImage img4 = image::toLinearWithAlpha<uint8_t>(w, h, bpr4, src4.data());
        const float* dst4 = img4.getPixelRef(0, 0);
        for (size_t y = 0; y < h; ++y) {
            for (size_t i = 0; i < w * 4; ++i) {
                const uint8_t v = src4[y * bpr4 + i];
                if ((i & 3) == 3) {
                    EXPECT_FLOAT_EQ(dst4[y * w * 4 + i], v * invMax8);
                } else {
                    EXPECT_NEAR(dst4[y * w * 4 + i], image::sRGBToLinear(v * invMax8), 1e-6f);
                }
            }
        }
    }

    // Test uint16_t and ntohs variants (exercises 8-element NEON loop, 4-element step, and scalar tail).
    {
        constexpr float invMax16 = 1.0f / 65535.0f;
        const size_t stride3 = w * 3 + 3;
        const size_t bpr3 = stride3 * sizeof(uint16_t);
        vector<uint16_t> src3(h * stride3, 0xbeef);
        for (size_t y = 0; y < h; ++y) {
            for (size_t i = 0; i < w * 3; ++i) {
                src3[y * stride3 + i] = static_cast<uint16_t>((y * 4099 + i * 1319) & 0xffff);
            }
        }
        src3[0] = 0;
        src3[1] = 2650;
        src3[2] = 2651;
        src3[3] = 65535;
        LinearImage img3 = image::toLinear<uint16_t>(w, h, bpr3,
                reinterpret_cast<const uint8_t*>(src3.data()));
        LinearImage img3Ntohs = image::toLinearNtohs<uint16_t>(w, h, bpr3,
                reinterpret_cast<const uint8_t*>(src3.data()));
        const float* dst3 = img3.getPixelRef(0, 0);
        const float* dst3Ntohs = img3Ntohs.getPixelRef(0, 0);
        for (size_t y = 0; y < h; ++y) {
            for (size_t i = 0; i < w * 3; ++i) {
                const uint16_t v = src3[y * stride3 + i];
                EXPECT_NEAR(dst3[y * w * 3 + i], image::sRGBToLinear(v * invMax16), 1e-6f);
                EXPECT_NEAR(dst3Ntohs[y * w * 3 + i], image::sRGBToLinear(ntohs(v) * invMax16), 1e-6f);
            }
        }

        const size_t stride4 = w * 4 + 3;
        const size_t bpr4 = stride4 * sizeof(uint16_t);
        vector<uint16_t> src4(h * stride4, 0xbeef);
        for (size_t y = 0; y < h; ++y) {
            for (size_t i = 0; i < w * 4; ++i) {
                src4[y * stride4 + i] = static_cast<uint16_t>((y * 6553 + i * 2713) & 0xffff);
            }
        }
        LinearImage img4 = image::toLinearWithAlpha<uint16_t>(w, h, bpr4,
                reinterpret_cast<const uint8_t*>(src4.data()));
        LinearImage img4Ntohs = image::toLinearWithAlphaNtohs<uint16_t>(w, h, bpr4,
                reinterpret_cast<const uint8_t*>(src4.data()));
        const float* dst4 = img4.getPixelRef(0, 0);
        const float* dst4Ntohs = img4Ntohs.getPixelRef(0, 0);
        for (size_t y = 0; y < h; ++y) {
            for (size_t i = 0; i < w * 4; ++i) {
                const uint16_t v = src4[y * stride4 + i];
                if ((i & 3) == 3) {
                    EXPECT_FLOAT_EQ(dst4[y * w * 4 + i], v * invMax16);
                    EXPECT_FLOAT_EQ(dst4Ntohs[y * w * 4 + i], ntohs(v) * invMax16);
                } else {
                    EXPECT_NEAR(dst4[y * w * 4 + i], image::sRGBToLinear(v * invMax16), 1e-6f);
                    EXPECT_NEAR(dst4Ntohs[y * w * 4 + i], image::sRGBToLinear(ntohs(v) * invMax16), 1e-6f);
                }
            }
        }
    }
}

TEST_F(ImageTest, Mipmaps) { // NOLINT
    Filter filter = filterFromString("HERMITE");
    ASSERT_EQ(filter, Filter::HERMITE);

    // Miplevels: 5x10, 2x5, 1x2, 1x1.
    LinearImage src = createColorFromAscii(
            "44444 41014 40704 41014 44444 44444 41014 40704 41014 44444");
    uint32_t count = getMipmapCount(src);
    ASSERT_EQ(count, 3);
    vector<LinearImage> mips(count);
    generateMipmaps(src, filter, mips.data(), count);
    updateOrCompare(src, "mip0_5x10.png");
    for (uint32_t index = 0; index < count; ++index) {
        updateOrCompare(mips[index], "mip" + std::to_string(index + 1) + "_5x10.png");
    }

    // Test color space with a classic RED => GREEN color gradient.
    src = createColorFromAscii("12");
    src = resampleImage(src, 200, 100, Filter::NEAREST);
    count = getMipmapCount(src);
    ASSERT_EQ(count, 7);
    mips.resize(count);
    generateMipmaps(src, filter, mips.data(), count);
    updateOrCompare(src, "mip0_200x100.png");
    for (uint32_t index = 0; index < count; ++index) {
        updateOrCompare(mips[index], "mip" + std::to_string(index + 1) + "_200x100.png");
    }
}

TEST_F(ImageTest, Ktx) { // NOLINT
    uint8_t foo[] = {1, 2, 3};
    uint8_t* data;
    uint32_t size;
    Ktx1Bundle nascent(2, 1, true);
    ASSERT_EQ(nascent.getNumMipLevels(), 2);
    ASSERT_EQ(nascent.getArrayLength(), 1);
    ASSERT_TRUE(nascent.isCubemap());
    ASSERT_FALSE(nascent.getBlob({0, 0, 0}, &data, &size));
    ASSERT_TRUE(nascent.setBlob({0, 0, 0}, foo, sizeof(foo)));
    ASSERT_TRUE(nascent.getBlob({0, 0, 0}, &data, &size));
    ASSERT_EQ(size, sizeof(foo));
    ASSERT_EQ(nascent.getMetadata("foo"), nullptr);

    const uint32_t KTX_HEADER_SIZE = 16 * 4;

    auto getFileSize = [](const char* filename) {
        std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
        return in.tellg();
    };

    if (g_comparisonMode == ComparisonMode::COMPARE) {
        const auto path = g_comparisonPath + "conftestimage_R11_EAC.ktx";
        const auto fileSize = getFileSize(path.c_str());
        ASSERT_GT(fileSize, 0);
        vector<uint8_t> buffer(fileSize);
        std::ifstream in(path, std::ifstream::in | std::ifstream::binary);
        ASSERT_TRUE(in.read((char*) buffer.data(), fileSize));
        Ktx1Bundle deserialized(buffer.data(), buffer.size());

        ASSERT_EQ(deserialized.getNumMipLevels(), 1);
        ASSERT_EQ(deserialized.getArrayLength(), 1);
        ASSERT_EQ(deserialized.isCubemap(), false);
        ASSERT_EQ(deserialized.getInfo().pixelWidth, 64);
        ASSERT_EQ(deserialized.getInfo().pixelHeight, 32);
        ASSERT_EQ(deserialized.getInfo().pixelDepth, 0);

        data = nullptr;
        size = 0;
        ASSERT_TRUE(deserialized.getBlob({0, 0, 0}, &data, &size));
        ASSERT_EQ(size, 1024);
        ASSERT_NE(data, nullptr);

        uint32_t serializedSize = deserialized.getSerializedLength();
        ASSERT_EQ(serializedSize, KTX_HEADER_SIZE + sizeof(uint32_t) + 1024);
        ASSERT_EQ(serializedSize, fileSize);

        vector<uint8_t> reserialized(serializedSize);
        ASSERT_TRUE(deserialized.serialize(reserialized.data(), serializedSize));
        ASSERT_EQ(reserialized, buffer);

        deserialized.setMetadata("foo", "bar");
        string val(deserialized.getMetadata("foo"));
        ASSERT_EQ(val, "bar");

        serializedSize = deserialized.getSerializedLength();
        reserialized.resize(serializedSize);
        ASSERT_TRUE(deserialized.serialize(reserialized.data(), serializedSize));

        Ktx1Bundle bundleWithMetadata(reserialized.data(), reserialized.size());
        val = string(bundleWithMetadata.getMetadata("foo"));
        ASSERT_EQ(val, "bar");
    }
}

TEST_F(ImageTest, getSphericalHarmonics) {
    Ktx1Bundle ktx(2, 1, true);

    const char* sphereHarmonics = R"(0.199599 0.197587 0.208682
    0.0894955 0.126985 0.187462
    0.0921711 0.102497 0.105308
    -0.0322833 -0.053886 -0.0661181
    -0.0734081 -0.0808731 -0.0788446
    0.0620748 0.0851526 0.100914
    0.00763482 0.00564362 -0.000848833
    -0.102654 -0.102815 -0.0930881
    -0.022778 -0.0281883 -0.0377256
    )";

    ktx.setMetadata("sh", sphereHarmonics);

    float3 harmonics[9];
    ktx.getSphericalHarmonics(harmonics);

    ASSERT_FLOAT_EQ(harmonics[0].x, 0.199599);
    ASSERT_FLOAT_EQ(harmonics[0].y, 0.197587);
    ASSERT_FLOAT_EQ(harmonics[0].z, 0.208682);

    ASSERT_FLOAT_EQ(harmonics[1].x, 0.0894955);
    ASSERT_FLOAT_EQ(harmonics[1].y, 0.126985);
    ASSERT_FLOAT_EQ(harmonics[1].z, 0.187462);

    ASSERT_FLOAT_EQ(harmonics[2].x, 0.0921711);
    ASSERT_FLOAT_EQ(harmonics[2].y, 0.102497);
    ASSERT_FLOAT_EQ(harmonics[2].z, 0.105308);

    ASSERT_FLOAT_EQ(harmonics[3].x, -0.0322833);
    ASSERT_FLOAT_EQ(harmonics[3].y, -0.053886);
    ASSERT_FLOAT_EQ(harmonics[3].z, -0.0661181);

    ASSERT_FLOAT_EQ(harmonics[4].x, -0.0734081);
    ASSERT_FLOAT_EQ(harmonics[4].y, -0.0808731);
    ASSERT_FLOAT_EQ(harmonics[4].z, -0.0788446);

    ASSERT_FLOAT_EQ(harmonics[5].x, 0.0620748);
    ASSERT_FLOAT_EQ(harmonics[5].y, 0.0851526);
    ASSERT_FLOAT_EQ(harmonics[5].z, 0.100914);

    ASSERT_FLOAT_EQ(harmonics[6].x, 0.00763482);
    ASSERT_FLOAT_EQ(harmonics[6].y, 0.00564362);
    ASSERT_FLOAT_EQ(harmonics[6].z, -0.000848833);

    ASSERT_FLOAT_EQ(harmonics[7].x, -0.102654);
    ASSERT_FLOAT_EQ(harmonics[7].y, -0.102815);
    ASSERT_FLOAT_EQ(harmonics[7].z, -0.0930881);

    ASSERT_FLOAT_EQ(harmonics[8].x, -0.022778);
    ASSERT_FLOAT_EQ(harmonics[8].y, -0.0281883);
    ASSERT_FLOAT_EQ(harmonics[8].z, -0.0377256);
}

static void printUsage(const char* name) {
    string exec_name(utils::Path(name).getName());
    string usage(
            "TEST is a unit test runner for the Filament image library\n"
            "Usages:\n"
            "    TEST compare <path-to-ref-images> [gtest options]\n"
            "    TEST update  <path-to-ref-images> [gtest options]\n"
            "    TEST [gtest options]\n"
            "\n");
    const string from("TEST");
    for (size_t pos = usage.find(from); pos != string::npos; pos = usage.find(from, pos)) {
         usage.replace(pos, from.length(), exec_name);
    }
    printf("%s", usage.c_str());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    if (argc < 2) {
        std::cerr << "\nWARNING: No path provided, skipping reference image comparison.\n\n";
        g_comparisonMode = ComparisonMode::SKIP;
        return RUN_ALL_TESTS();
    }
    const string cmd = argv[1];
    if (cmd == "help") {
        printUsage(argv[0]);
        return 0;
    }
    if (cmd == "compare" || cmd == "update") {
        if (argc != 3) {
            printUsage(argv[0]);
            return 1;
        }
        g_comparisonPath = argv[2];
    }
    if (cmd == "compare") {
        g_comparisonMode = ComparisonMode::COMPARE;
        return RUN_ALL_TESTS();
    }
    if (cmd == "update") {
        g_comparisonMode = ComparisonMode::UPDATE;
        return RUN_ALL_TESTS();
    }
    printUsage(argv[0]);
    return 1;
}

static LinearImage createNormalMap(uint32_t size) {
    LinearImage result(size, size, 3);
    auto vectors = (float3*) result.getPixelRef();
    const float invsize = 1.0f / size;
    const Sphere sphere {
        .center = float3(0.5, 0.5, 0.0),
        .radius2 = 0.15
    };
    for (uint32_t n = 0; n < size * size; ++n) {
        const uint32_t row = n / size, col = n % size;
        const Ray ray {
            .orig = { (col + 0.5f) * invsize, 1.0f - (row + 0.5f) * invsize, 1 },
            .dir = {0, 0, -1}
        };
        float t;
        bool isect = intersect(ray, sphere, &t);
        if (isect) {
            float3 p = ray.orig + t * ray.dir;
            vectors[n] = normalize(p - sphere.center);
        } else {
            vectors[n] = {0, 0, 1};
        }
    }
    return result;
}

static LinearImage createDepthMap(uint32_t size) {
    LinearImage result(size, size, 1);
    auto depths = result.getPixelRef();
    const float invsize = 1.0f / size;
    const Sphere sphere {
        .center = float3(0.5, 0.5, 0.0),
        .radius2 = 0.15
    };
    for (uint32_t n = 0; n < size * size; ++n) {
        const uint32_t row = n / size, col = n % size;
        const Ray ray {
            .orig = { (col + 0.5f) * invsize, 1.0f - (row + 0.5f) * invsize, 1 },
            .dir = {0, 0, -1}
        };
        float t;
        bool isect = intersect(ray, sphere, &t);
        if (isect) {
            float3 p = ray.orig + t * ray.dir;
            depths[n] = p.z;
        } else {
            depths[n] = 1;
        }
    }
    return result;
}

static LinearImage createGrayFromAscii(const string& pattern) {
    uint32_t width = 0;
    uint32_t height = 0;
    string row;

    // Compute the required size.
    for (istringstream istream(pattern); istream >> row; ++height) {
        width = (uint32_t) row.size();
    }

    // Allocate the sequence of pixels.
    LinearImage result(width, height, 1);

    // Fill in the pixel data.
    istringstream istream(pattern);
    float* seq = result.getPixelRef();
    for (int i = 0; istream >> row;) {
        for (char c : row) {
            seq[i++] = c - '0';
        }
    }
    return result;
}

static LinearImage createColorFromAscii(const string& pattern) {
    uint32_t width = 0;
    uint32_t height = 0;
    string row;

    // Compute the required size.
    for (istringstream istream(pattern); istream >> row; ++height) {
        width = (uint32_t) row.size();
    }

    // Allocate the sequence of pixels.
    LinearImage result(width, height, 3);

    // Fill in the pixel data.
    istringstream istream(pattern);
    float* seq = result.getPixelRef();
    for (int i = 0; istream >> row;) {
        for (char c : row) {
            uint32_t val = c - (uint32_t)('0');
            seq[i++] = (val >> 0u) & 1u;
            seq[i++] = (val >> 1u) & 1u;
            seq[i++] = (val >> 2u) & 1u;
            auto col = (float3*) (seq + i - 3);
            *col = sRGBToLinear(*col);
        }
    }
    return result;
}

static void updateOrCompare(const LinearImage& limg, const utils::Path& fname) {
    image::updateOrCompare(limg, g_comparisonPath + fname, g_comparisonMode, 0.0f);
}

static bool solve(float a, float b, float c, float *x0, float *x1) {
    float discr = b * b - 4 * a * c;
    if (discr < 0) return false;
    if (discr == 0) {
        *x0 = *x1 = -0.5f * b / a;
    } else {
        float q = (b > 0) ?  -0.5f * (b + sqrtf(discr)) :  -0.5f * (b - sqrtf(discr));
        *x0 = q / a;
        *x1 = c / q;
    }
    if (*x0 > *x1) swap(*x0, *x1);
    return true;
}

static bool intersect(Ray ray, Sphere sphere, float* t) {
    float t0, t1;
    float3 L = ray.orig - sphere.center;
    float a = dot(ray.dir, ray.dir);
    float b = 2 * dot(ray.dir, L);
    float c = dot(L, L) - sphere.radius2;
    if (!solve(a, b, c, &t0, &t1)) return false;
    if (t0 > t1) swap(t0, t1);
    if (t0 < 0) {
        t0 = t1;
        if (t0 < 0) return false;
    }
    *t = t0;
    return true;
}

static LinearImage diffImages(const LinearImage& a, const LinearImage& b) {
    const uint32_t width = a.getWidth(), height = a.getHeight(), nchan = a.getChannels();
    FILAMENT_CHECK_PRECONDITION(
            width == b.getWidth() && height == b.getHeight() && nchan == b.getChannels())
            << "Images must have same shape.";
    LinearImage result(width, height, nchan);
    float* dst = result.getPixelRef();
    float const* srca = a.getPixelRef();
    float const* srcb = b.getPixelRef();
    float largest = 0;
    float smallest = std::numeric_limits<float>::max();
    for (uint32_t n = 0; n < width * height * nchan; ++n) {
        float delta = std::abs(srca[n] - srcb[n]);
        largest = std::max(largest, delta);
        smallest = std::min(smallest, delta);
        dst[n] = delta;
    }
    const float scale = (largest == smallest) ? 1.0f : (1.0f / largest - smallest);
    for (uint32_t n = 0; n < width * height * nchan; ++n) {
        dst[n] = (dst[n] - smallest) * scale;
    }
    return result;
}

TEST_F(ImageTest, LinearImageDimensionOverflow) { // NOLINT
    LinearImage img1(std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max(), 3);
    EXPECT_FALSE(img1);

    LinearImage img2(2, 2, 0);
    EXPECT_FALSE(img2);
}

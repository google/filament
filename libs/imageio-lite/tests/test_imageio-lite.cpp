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

#include <imageio-lite/ImageDecoder.h>
#include <imageio-lite/ImageEncoder.h>

#include <image/LinearImage.h>

#include <math/vec3.h>
#include <math/vec4.h>

#include <gtest/gtest.h>

#include <sstream>

using namespace image;
using namespace imageio_lite;
using namespace filament::math;

class ImageIOLiteTest : public testing::Test {};

TEST_F(ImageIOLiteTest, TIFFRoundTrip) {
    // Create a simple 4x4 image manually
    uint32_t w = 4;
    uint32_t h = 4;
    LinearImage src(w, h, 3);

    // Fill with a pattern: R, G, B, White
    // Row 0: Red
    // Row 1: Green
    // Row 2: Blue
    // Row 3: White
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            float* p = src.getPixelRef(x, y);
            if (y == 0) {
                p[0] = 1;
                p[1] = 0;
                p[2] = 0;
            } else if (y == 1) {
                p[0] = 0;
                p[1] = 1;
                p[2] = 0;
            } else if (y == 2) {
                p[0] = 0;
                p[1] = 0;
                p[2] = 1;
            } else {
                p[0] = 1;
                p[1] = 1;
                p[2] = 1;
            }
        }
    }

    std::stringstream stream;

    // Encode to TIFF
    bool success = ImageEncoder::encode(stream, ImageEncoder::Format::TIFF, src, "", "test.tif");
    ASSERT_TRUE(success);

    // Decode from TIFF
    stream.seekg(0);
    LinearImage dst = ImageDecoder::decode(stream, "test.tif");

    ASSERT_EQ(dst.getWidth(), src.getWidth());
    ASSERT_EQ(dst.getHeight(), src.getHeight());

    // Since we go Linear -> sRGB (8-bit) -> Linear, there will be precision loss.
    // 1/255 is approx 0.004. So epsilon should be around that or slightly higher due to
    // conversions.
    float const epsilon = 0.01f;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            // src is 3 channels
            float* s_ptr = src.getPixelRef(x, y);
            float3 s = { s_ptr[0], s_ptr[1], s_ptr[2] };

            // dst is 4 channels (TIFF export forces RGBA)
            float* d_ptr = dst.getPixelRef(x, y);
            float4 d = { d_ptr[0], d_ptr[1], d_ptr[2], d_ptr[3] };

            EXPECT_NEAR(s.r, d.r, epsilon);
            EXPECT_NEAR(s.g, d.g, epsilon);
            EXPECT_NEAR(s.b, d.b, epsilon);
            EXPECT_NEAR(1.0f, d.a, epsilon);
        }
    }
}

TEST_F(ImageIOLiteTest, BigEndianTIFF) {
    std::vector<uint8_t> tiff = {
        0x4D,
        0x4D,
        0x00,
        0x2A,
        0x00,
        0x00,
        0x00,
        0x08, // Header (BE)
    };

    auto write16 = [&](uint16_t v) {
        tiff.push_back(v >> 8);
        tiff.push_back(v & 0xFF);
    };
    auto write32 = [&](uint32_t v) {
        tiff.push_back(v >> 24);
        tiff.push_back((v >> 16) & 0xFF);
        tiff.push_back((v >> 8) & 0xFF);
        tiff.push_back(v & 0xFF);
    };

    // IFD with mandatory tags
    write16(8); // Number of entries

    // ImageWidth (256), LONG (4), 1, 1
    write16(256);
    write16(4);
    write32(1);
    write32(1);
    // ImageLength (257), LONG (4), 1, 1
    write16(257);
    write16(4);
    write32(1);
    write32(1);
    // BitsPerSample (258), SHORT (3), 1, 8 (Left justified: 0x00080000)
    write16(258);
    write16(3);
    write32(1);
    write32(0x00080000);
    // Compression (259), SHORT (3), 1, 1 (Left justified)
    write16(259);
    write16(3);
    write32(1);
    write32(0x00010000);
    // PhotometricInterpretation (262), SHORT (3), 1, 2 (RGB) (Left justified)
    write16(262);
    write16(3);
    write32(1);
    write32(0x00020000);
    // StripOffsets (273), LONG (4), 1, 100 (Offset to data)
    write16(273);
    write16(4);
    write32(1);
    write32(200);
    // SamplesPerPixel (277), SHORT (3), 1, 3 (RGB) (Left justified)
    write16(277);
    write16(3);
    write32(1);
    write32(0x00030000); // 3 channels
    // StripByteCounts (279), LONG (4), 1, 3 (1 pixel * 3 bytes)
    write16(279);
    write16(4);
    write32(1);
    write32(3);

    write32(0); // Next IFD

    // Fill data until offset 200
    while (tiff.size() < 200) tiff.push_back(0);
    // Pixel data (RGB)
    tiff.push_back(0xFF);
    tiff.push_back(0x00);
    tiff.push_back(0x00);

    std::string s(tiff.begin(), tiff.end());
    std::stringstream stream(s);
    LinearImage dst = ImageDecoder::decode(stream, "be.tif");

    // If BE decoding works, this returns a valid image.
    // If it fails (e.g. BitsPerSample read as 0), it returns invalid image.
    ASSERT_TRUE(dst.isValid());
    ASSERT_EQ(dst.getWidth(), 1);
    ASSERT_EQ(dst.getHeight(), 1);
}

TEST_F(ImageIOLiteTest, MaliciousHugeStrips) {
    std::vector<uint8_t> tiff = {
        0x49,
        0x49,
        0x2A,
        0x00,
        0x08,
        0x00,
        0x00,
        0x00, // Header (LE)
    };
    auto write16 = [&](uint16_t v) {
        tiff.push_back(v & 0xFF);
        tiff.push_back(v >> 8);
    };
    auto write32 = [&](uint32_t v) {
        tiff.push_back(v & 0xFF);
        tiff.push_back((v >> 8) & 0xFF);
        tiff.push_back((v >> 16) & 0xFF);
        tiff.push_back(v >> 24);
    };

    write16(1); // 1 entry
    // StripOffsets (273), LONG (4), count=1000001, offset=0
    write16(273);
    write16(4);
    write32(1000001);
    write32(0);
    write32(0);

    std::string s(tiff.begin(), tiff.end());

    // We expect this to fail due to FILAMENT_CHECK_PRECONDITION.
    // In builds with exceptions, it throws. In builds without, it aborts.
    // EXPECT_DEATH expects an abort/crash. So we catch and abort if it throws.
    EXPECT_DEATH(
            {
                try {
                    std::stringstream stream(s);
                    ImageDecoder::decode(stream, "bad.tif");
                } catch (...) {
                    std::abort();
                }
            },
            "");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

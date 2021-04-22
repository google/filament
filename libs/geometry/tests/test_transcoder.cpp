/*
 * Copyright 2021 The Android Open Source Project
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

#include <geometry/Transcoder.h>

#include <math/half.h>

#include <gtest/gtest.h>

using filament::math::half;
using filament::geometry::Transcoder;
using filament::geometry::ComponentType;

class TranscoderTest : public testing::Test {};

struct Vertex {
    uint8_t b0, b1, b2, b3;
    uint16_t s0, s1, s2;
    half h0;
};

static constexpr int count = 2;

static const Vertex vbuffer[count] = {
    {
        0x00, 0xff, 0x7f, 0x81,
        0x0000, 0xffff, 0x7fff,
        half(-0.5f)
    },
    {
        0x00, 0xff, 0x7f, 0x80, // 0x80 is -128 when interpreted as int8_t
        0x0000, 0x8000, 0x7fff,
        half(1.0f)
    }
};

TEST_F(TranscoderTest, Normalized) {
    float result[count * 4];

    // UNSIGNED BYTES

    Transcoder transcodeBytes({
        .componentType = ComponentType::UBYTE,
        .normalized = true,
        .componentCount = 4u,
        .inputStrideBytes = sizeof(Vertex)
    });

    transcodeBytes(result, vbuffer, count);

    ASSERT_EQ(result[0], 0.0f);
    ASSERT_EQ(result[1], 1.0f);
    ASSERT_NEAR(result[2], 0.50f, 0.005f);
    ASSERT_NEAR(result[3], 0.51f, 0.005f);

    ASSERT_EQ(result[4], 0.0f);
    ASSERT_EQ(result[5], 1.0f);
    ASSERT_NEAR(result[6], 0.50f, 0.005f);
    ASSERT_NEAR(result[7], 0.50f, 0.005f);

    // SIGNED BYTES (twos-complement)

    Transcoder transcodeSignedBytes({
        .componentType = ComponentType::BYTE,
        .normalized = true,
        .componentCount = 4u,
        .inputStrideBytes = sizeof(Vertex)
    });

    transcodeSignedBytes(result, vbuffer, count);

    ASSERT_EQ(result[0], 0.0f);
    ASSERT_NEAR(result[1], -0.01f, 0.005f);
    ASSERT_EQ(result[2], +1.0f);
    ASSERT_EQ(result[3], -1.0f);
    ASSERT_EQ(result[7], -1.0f);
}

TEST_F(TranscoderTest, NonNormalized) {
    float result[count * 3];
    char const* srcBytes = (char const*) vbuffer;

    // SIGNED SHORTS (twos-complement)

    Transcoder transcodeShorts({
        .componentType = ComponentType::SHORT,
        .normalized = false,
        .componentCount = 3u,
        .inputStrideBytes = sizeof(Vertex)
    });

    size_t written = transcodeShorts(result, srcBytes + 4, count);
    ASSERT_EQ(written, sizeof(result));

    ASSERT_EQ(result[0], 0.0f);
    ASSERT_EQ(result[1], -1.0f);
    ASSERT_EQ(result[2], 32767.0f);
    ASSERT_EQ(result[4], -32768.0f);

    // HALF

    Transcoder transcodeHalf({
        .componentType = ComponentType::HALF,
        .normalized = false, // <= this field is ignored for HALF
        .componentCount = 1u,
        .inputStrideBytes = sizeof(Vertex)
    });

    transcodeHalf(result, srcBytes + 10, count);
    ASSERT_EQ(result[0], -0.5f);
    ASSERT_EQ(result[1], 1.0f);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

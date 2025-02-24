//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// PVRTCCompressedTextureTest.cpp: Sampling tests for PVRTC texture formats
// Invalid usage errors are covered by CompressedTextureFormatsTest.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

class PVRTCCompressedTextureTestES3 : public ANGLETest<>
{
    static constexpr int kDim = 128;

  protected:
    PVRTCCompressedTextureTestES3()
    {
        setWindowWidth(kDim);
        setWindowHeight(kDim);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void testSetUp() override
    {
        // Prepare some filler data.
        // The test only asserts that direct and PBO texture uploads produce
        // identical results, so the decoded values do not matter here.
        for (size_t i = 0; i < mTextureData.size(); ++i)
        {
            mTextureData[i] = static_cast<uint8_t>(i + i / 8 + i / 2048);
        }
    }

    void test(GLenum format, GLsizei dimension)
    {
        // Placeholder for the decoded color values from a directly uploaded texture.
        std::array<GLColor, kDim * kDim> controlData;

        GLsizei imageSize = 0;
        switch (format)
        {
            case GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG:
            case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:
            case GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT:
            case GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT:
                imageSize = (std::max(dimension, 8) * std::max(dimension, 8) * 4 + 7) / 8;
                break;
            case GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG:
            case GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG:
            case GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT:
            case GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT:
                imageSize = (std::max(dimension, 16) * std::max(dimension, 8) * 2 + 7) / 8;
                break;
        }
        ASSERT_GT(imageSize, 0);

        // Directly upload compressed data and remember the decoded values.
        {
            GLTexture texture;
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glCompressedTexImage2D(GL_TEXTURE_2D, 0, format, dimension, dimension, 0, imageSize,
                                   mTextureData.data());
            ASSERT_GL_NO_ERROR();

            glClear(GL_COLOR_BUFFER_BIT);
            drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.0f);
            ASSERT_GL_NO_ERROR();

            glReadPixels(0, 0, kDim, kDim, GL_RGBA, GL_UNSIGNED_BYTE, controlData.data());
            ASSERT_GL_NO_ERROR();
        }

        // Upload the same compressed data using a PBO with different
        // offsets and check that it is sampled correctly each time.
        for (size_t offset = 0; offset <= 8; ++offset)
        {
            std::vector<GLubyte> bufferData(offset);
            std::copy(mTextureData.begin(), mTextureData.end(), std::back_inserter(bufferData));
            ASSERT_EQ(bufferData.size(), mTextureData.size() + offset);

            GLBuffer buffer;
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer);
            glBufferData(GL_PIXEL_UNPACK_BUFFER, bufferData.size(), bufferData.data(),
                         GL_STATIC_READ);
            ASSERT_GL_NO_ERROR();

            GLTexture texture;
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glCompressedTexImage2D(GL_TEXTURE_2D, 0, format, dimension, dimension, 0, imageSize,
                                   reinterpret_cast<void *>(offset));
            ASSERT_GL_NO_ERROR();

            glClear(GL_COLOR_BUFFER_BIT);
            drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.0f);
            ASSERT_GL_NO_ERROR();

            std::array<GLColor, kDim * kDim> readData;
            glReadPixels(0, 0, kDim, kDim, GL_RGBA, GL_UNSIGNED_BYTE, readData.data());
            ASSERT_GL_NO_ERROR();

            // Check only one screen pixel for each texture pixel.
            for (GLsizei x = 0; x < dimension; ++x)
            {
                for (GLsizei y = 0; y < dimension; ++y)
                {
                    // Tested texture sizes are multiples of the window dimensions.
                    const size_t xScaled  = x * kDim / dimension;
                    const size_t yScaled  = y * kDim / dimension;
                    const size_t position = yScaled * kDim + xScaled;
                    EXPECT_EQ(readData[position], controlData[position])
                        << "(" << x << ", " << y << ")"
                        << " of " << dimension << "x" << dimension << " texture with PBO offset "
                        << offset;
                }
            }
        }
    }

    void run(GLenum format)
    {
        mProgram.makeRaster(essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
        for (auto dimension : {1, 2, 4, 8, 16, 32, 64, 128})
        {
            test(format, dimension);
        }
    }

  private:
    std::array<GLubyte, 8192> mTextureData;
    GLProgram mProgram;
};

// Test uploading texture data from a PBO to an RGB_PVRTC_4BPPV1 texture.
TEST_P(PVRTCCompressedTextureTestES3, RGB_PVRTC_4BPPV1)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_IMG_texture_compression_pvrtc"));
    run(GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG);
}

// Test uploading texture data from a PBO to an RGBA_PVRTC_4BPPV1 texture.
TEST_P(PVRTCCompressedTextureTestES3, RGBA_PVRTC_4BPPV1)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_IMG_texture_compression_pvrtc"));
    run(GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG);
}

// Test uploading texture data from a PBO to an RGB_PVRTC_2BPPV1 texture.
TEST_P(PVRTCCompressedTextureTestES3, RGB_PVRTC_2BPPV1)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_IMG_texture_compression_pvrtc"));
    run(GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG);
}

// Test uploading texture data from a PBO to an RGBA_PVRTC_2BPPV1 texture.
TEST_P(PVRTCCompressedTextureTestES3, RGBA_PVRTC_2BPPV1)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_IMG_texture_compression_pvrtc"));
    run(GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG);
}

// Test uploading texture data from a PBO to an SRGB_PVRTC_4BPPV1 texture.
TEST_P(PVRTCCompressedTextureTestES3, SRGB_PVRTC_4BPPV1)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_IMG_texture_compression_pvrtc"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_pvrtc_sRGB"));
    run(GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT);
}

// Test uploading texture data from a PBO to an SRGB_ALPHA_PVRTC_4BPPV1 texture.
TEST_P(PVRTCCompressedTextureTestES3, SRGB_ALPHA_PVRTC_4BPPV1)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_IMG_texture_compression_pvrtc"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_pvrtc_sRGB"));
    run(GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT);
}

// Test uploading texture data from a PBO to an SRGB_PVRTC_2BPPV1 texture.
TEST_P(PVRTCCompressedTextureTestES3, SRGB_PVRTC_2BPPV1)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_IMG_texture_compression_pvrtc"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_pvrtc_sRGB"));
    run(GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT);
}

// Test uploading texture data from a PBO to an SRGB_ALPHA_PVRTC_2BPPV1 texture.
TEST_P(PVRTCCompressedTextureTestES3, SRGB_ALPHA_PVRTC_2BPPV1)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_IMG_texture_compression_pvrtc"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_pvrtc_sRGB"));
    run(GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(PVRTCCompressedTextureTestES3);
ANGLE_INSTANTIATE_TEST_ES3(PVRTCCompressedTextureTestES3);

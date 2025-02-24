//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// MemorySizeTest.cpp : Tests of the GL_ANGLE_memory_size extension.

#include "test_utils/ANGLETest.h"

#include "test_utils/gl_raii.h"

namespace angle
{

class MemorySizeTest : public ANGLETest<>
{
  protected:
    MemorySizeTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

// GL_ANGLE_memory_size is implemented in the front-end and should always be exposed.
TEST_P(MemorySizeTest, ExtensionStringExposed)
{
    EXPECT_TRUE(EnsureGLExtensionEnabled("GL_ANGLE_memory_size"));
}

// Test basic queries of textures
TEST_P(MemorySizeTest, BasicUsageTexture)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_size"));

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);

    GLint result;
    glGetTexParameteriv(GL_TEXTURE_2D, GL_MEMORY_SIZE_ANGLE, &result);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(0, result);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glGetTexParameteriv(GL_TEXTURE_2D, GL_MEMORY_SIZE_ANGLE, &result);
    EXPECT_GL_NO_ERROR();
    EXPECT_GT(result, 0);

    if (getClientMajorVersion() > 3 ||
        (getClientMajorVersion() == 3 && getClientMinorVersion() >= 1))
    {
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 1, GL_MEMORY_SIZE_ANGLE, &result);
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(0, result);

        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_MEMORY_SIZE_ANGLE, &result);
        EXPECT_GL_NO_ERROR();
        EXPECT_GT(result, 0);
    }
}

// Test basic queries of buffers
TEST_P(MemorySizeTest, BasicUsageBuffer)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_size"));

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    GLint result;
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_MEMORY_SIZE_ANGLE, &result);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(0, result);

    if (getClientMajorVersion() > 3 ||
        (getClientMajorVersion() == 3 && getClientMinorVersion() >= 1))
    {
        GLint64 result64;
        glGetBufferParameteri64v(GL_ARRAY_BUFFER, GL_MEMORY_SIZE_ANGLE, &result64);
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(0, result64);
    }

    constexpr GLsizeiptr kBufSize = 16;
    std::array<uint8_t, kBufSize> buf;
    glBufferData(GL_ARRAY_BUFFER, kBufSize, buf.data(), GL_STATIC_DRAW);
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_MEMORY_SIZE_ANGLE, &result);
    EXPECT_GL_NO_ERROR();

    // This value may have to be reset to 1 if some backend delays allocations or compresses
    // buffers.
    constexpr GLint kExpectedMinBufMemorySize = 15;

    EXPECT_GT(result, kExpectedMinBufMemorySize);

    if (getClientMajorVersion() > 3 ||
        (getClientMajorVersion() == 3 && getClientMinorVersion() >= 1))
    {
        GLint64 result64;
        glGetBufferParameteri64v(GL_ARRAY_BUFFER, GL_MEMORY_SIZE_ANGLE, &result64);
        EXPECT_GL_NO_ERROR();
        EXPECT_GT(result64, static_cast<GLint64>(kExpectedMinBufMemorySize));
    }

    // No way to easily test the GLint64 to GLint64 clamping behaviour of glGetBufferParameteriv
    // without allocating a buffer >2gb.
}

// Test basic queries of renderbuffers
TEST_P(MemorySizeTest, BasicUsageRenderbuffer)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_size"));

    GLRenderbuffer renderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);

    GLint result;
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_MEMORY_SIZE_ANGLE, &result);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(0, result);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, 4, 4);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_MEMORY_SIZE_ANGLE, &result);
    EXPECT_GL_NO_ERROR();
    EXPECT_GT(result, 0);
}

// No errors specific to GL_ANGLE_memory_size to test for.

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(MemorySizeTest);
}  // namespace angle

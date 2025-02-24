//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ExternalWrapTest:
//   Tests EXT_EGL_image_external_wrap_modes
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"

constexpr int kPixelThreshold = 1;

namespace angle
{
class ExternalWrapTest : public ANGLETest<>
{
  protected:
    ExternalWrapTest() : mProgram(0), mSourceTexture(0), mExternalImage(0), mExternalTexture(0)
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    void testSetUp() override
    {
        if (!IsGLExtensionEnabled("GL_OES_EGL_image_external"))
        {
            return;
        }

        const char *vertSrc = R"(precision highp float;
attribute vec4 a_position;
varying vec2 v_texcoord;

uniform vec2 u_offset;

void main()
{
    gl_Position = a_position;
    v_texcoord = (a_position.xy * 0.5) + 0.5 + u_offset;
}
)";
        const char *fragSrc = R"(#extension GL_OES_EGL_image_external : require
precision highp float;
uniform samplerExternalOES s_tex;
varying vec2 v_texcoord;

void main()
{
    gl_FragColor = texture2D(s_tex, v_texcoord);
}
)";

        mProgram = CompileProgram(vertSrc, fragSrc);
        ASSERT_GL_NO_ERROR();
        ASSERT_NE(mProgram, 0u);

        constexpr GLsizei texSize = 32;
        GLubyte data[texSize * texSize * 4];

        for (int y = 0; y < texSize; y++)
        {
            float green = static_cast<float>(y) / texSize;
            for (int x = 0; x < texSize; x++)
            {
                float red = static_cast<float>(x) / texSize;

                data[(y * texSize + x) * 4 + 0] = static_cast<GLubyte>(red * 255);
                data[(y * texSize + x) * 4 + 1] = static_cast<GLubyte>(green * 255);

                data[(y * texSize + x) * 4 + 2] = 0;
                data[(y * texSize + x) * 4 + 3] = 255;
            }
        }

        glGenTextures(1, &mSourceTexture);
        glBindTexture(GL_TEXTURE_2D, mSourceTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSize, texSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        ASSERT_GL_NO_ERROR();

        glGenTextures(1, &mExternalTexture);

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override
    {
        if (mProgram != 0)
        {
            glDeleteProgram(mProgram);
        }
        if (mExternalTexture != 0)
        {
            glDeleteTextures(1, &mExternalTexture);
        }
        if (mSourceTexture != 0)
        {
            glDeleteTextures(1, &mSourceTexture);
        }
    }

    void createExternalTexture()
    {
        ASSERT_TRUE(IsEGLDisplayExtensionEnabled(getEGLWindow()->getDisplay(),
                                                 "EGL_KHR_gl_texture_2D_image"));
        ASSERT_TRUE(IsGLExtensionEnabled("GL_OES_EGL_image_external"));

        EGLWindow *window = getEGLWindow();
        EGLint attribs[]  = {
             EGL_IMAGE_PRESERVED,
             EGL_TRUE,
             EGL_NONE,
        };
        EGLImageKHR image = eglCreateImageKHR(
            window->getDisplay(), window->getContext(), EGL_GL_TEXTURE_2D_KHR,
            reinterpret_cast<EGLClientBuffer>(static_cast<uintptr_t>(mSourceTexture)), attribs);
        ASSERT_EGL_SUCCESS();

        glBindTexture(GL_TEXTURE_EXTERNAL_OES, mExternalTexture);
        glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, image);

        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        ASSERT_GL_NO_ERROR();
    }

    GLuint mProgram;
    GLuint mTextureUniformLocation;
    GLuint mOffsetUniformLocation;

    GLuint mSourceTexture;
    EGLImageKHR mExternalImage;
    GLuint mExternalTexture;
};

// Test the default sampling behavior of an external texture
TEST_P(ExternalWrapTest, NoWrap)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_EGL_image_external"));
    ANGLE_SKIP_TEST_IF(
        !IsEGLDisplayExtensionEnabled(getEGLWindow()->getDisplay(), "EGL_KHR_gl_texture_2D_image"));

    // Ozone only supports external target for images created with EGL_EXT_image_dma_buf_import
    ANGLE_SKIP_TEST_IF(IsOzone());

    createExternalTexture();

    ASSERT_NE(mProgram, 0u);
    glUseProgram(mProgram);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, mExternalTexture);
    glUniform2f(glGetUniformLocation(mProgram, "u_offset"), 0.0, 0.0);
    drawQuad(mProgram, "a_position", 0);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(0, 0, 0, 255), kPixelThreshold);
    EXPECT_PIXEL_COLOR_NEAR(127, 0, GLColor(247, 0, 0, 255), kPixelThreshold);
    EXPECT_PIXEL_COLOR_NEAR(0, 127, GLColor(0, 247, 0, 255), kPixelThreshold);
    EXPECT_PIXEL_COLOR_NEAR(127, 127, GLColor(247, 247, 0, 255), kPixelThreshold);
}

// Test that external textures are sampled correctly when used with GL_CLAMP_TO_EDGE
TEST_P(ExternalWrapTest, ClampToEdge)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_EGL_image_external"));
    ANGLE_SKIP_TEST_IF(
        !IsEGLDisplayExtensionEnabled(getEGLWindow()->getDisplay(), "EGL_KHR_gl_texture_2D_image"));

    // Ozone only supports external target for images created with EGL_EXT_image_dma_buf_import
    ANGLE_SKIP_TEST_IF(IsOzone());

    createExternalTexture();

    ASSERT_NE(mProgram, 0u);
    glUseProgram(mProgram);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, mExternalTexture);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glUniform2f(glGetUniformLocation(mProgram, "u_offset"), 0.5, 0.5);
    drawQuad(mProgram, "a_position", 0);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(127, 127, 0, 255), kPixelThreshold);
    EXPECT_PIXEL_COLOR_NEAR(127, 0, GLColor(247, 127, 0, 255), kPixelThreshold);
    EXPECT_PIXEL_COLOR_NEAR(0, 127, GLColor(127, 247, 0, 255), kPixelThreshold);
    EXPECT_PIXEL_COLOR_NEAR(127, 127, GLColor(247, 247, 0, 255), kPixelThreshold);
}

// Test that external textures are sampled correctly when used with GL_REPEAT
TEST_P(ExternalWrapTest, Repeat)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_EGL_image_external"));
    ANGLE_SKIP_TEST_IF(
        !IsEGLDisplayExtensionEnabled(getEGLWindow()->getDisplay(), "EGL_KHR_gl_texture_2D_image"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_EGL_image_external_wrap_modes"));

    // Ozone only supports external target for images created with EGL_EXT_image_dma_buf_import
    ANGLE_SKIP_TEST_IF(IsOzone());

    createExternalTexture();

    ASSERT_NE(mProgram, 0u);
    glUseProgram(mProgram);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, mExternalTexture);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glUniform2f(glGetUniformLocation(mProgram, "u_offset"), 0.5, 0.5);

    drawQuad(mProgram, "a_position", 0);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(127, 127, 0, 255), kPixelThreshold);
    EXPECT_PIXEL_COLOR_NEAR(127, 0, GLColor(119, 127, 0, 255), kPixelThreshold);
    EXPECT_PIXEL_COLOR_NEAR(0, 127, GLColor(127, 119, 0, 255), kPixelThreshold);
    EXPECT_PIXEL_COLOR_NEAR(127, 127, GLColor(119, 119, 0, 255), kPixelThreshold);

    EXPECT_PIXEL_COLOR_NEAR(63, 63, GLColor(247, 247, 0, 255), kPixelThreshold);
    EXPECT_PIXEL_COLOR_NEAR(64, 63, GLColor(0, 247, 0, 255), kPixelThreshold);
}

// Test that external textures are sampled correctly when used with GL_MIRRORED_REPEAT
TEST_P(ExternalWrapTest, MirroredRepeat)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_EGL_image_external"));
    ANGLE_SKIP_TEST_IF(
        !IsEGLDisplayExtensionEnabled(getEGLWindow()->getDisplay(), "EGL_KHR_gl_texture_2D_image"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_EGL_image_external_wrap_modes"));

    // Ozone only supports external target for images created with EGL_EXT_image_dma_buf_import
    ANGLE_SKIP_TEST_IF(IsOzone());

    createExternalTexture();

    ASSERT_NE(mProgram, 0u);
    glUseProgram(mProgram);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, mExternalTexture);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glUniform2f(glGetUniformLocation(mProgram, "u_offset"), 0.5, 0.5);

    drawQuad(mProgram, "a_position", 0);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(127, 127, 0, 255), kPixelThreshold);
    EXPECT_PIXEL_COLOR_NEAR(127, 0, GLColor(127, 127, 0, 255), kPixelThreshold);
    EXPECT_PIXEL_COLOR_NEAR(0, 127, GLColor(127, 127, 0, 255), kPixelThreshold);
    EXPECT_PIXEL_COLOR_NEAR(127, 127, GLColor(127, 127, 0, 255), kPixelThreshold);

    EXPECT_PIXEL_COLOR_NEAR(63, 63, GLColor(247, 247, 0, 255), kPixelThreshold);
    EXPECT_PIXEL_COLOR_NEAR(64, 63, GLColor(247, 247, 0, 255), kPixelThreshold);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(ExternalWrapTest);
}  // namespace angle

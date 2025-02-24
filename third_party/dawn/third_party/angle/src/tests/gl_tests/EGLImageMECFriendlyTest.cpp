//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLImageMECFriendlyTest:
//   MEC friendly tests are tests that use all of the created resources, so that
//   MEC will have to capture everything, and we can test with capture/replay
//   whether this is done correctly. In this case the focus is on external images

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"

constexpr int kPixelThreshold = 1;

namespace angle
{
class EGLImageMECFriendlyTest : public ANGLETest<>
{
  protected:
    EGLImageMECFriendlyTest()
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

        constexpr GLsizei texSize = 32;
        GLubyte data[texSize * texSize * 4];

        for (int y = 0; y < texSize; y++)
        {
            float green = static_cast<float>(y) / texSize;
            for (int x = 0; x < texSize; x++)
            {
                float blue = static_cast<float>(x) / texSize;

                data[(y * texSize + x) * 4 + 0] = 0;
                data[(y * texSize + x) * 4 + 1] = static_cast<GLubyte>(green * 255);
                data[(y * texSize + x) * 4 + 2] = static_cast<GLubyte>(blue * 255);
                ;
                data[(y * texSize + x) * 4 + 3] = 255;
            }
        }

        glGenTextures(1, &mSourceTexture);
        glBindTexture(GL_TEXTURE_2D, mSourceTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSize, texSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     data);
        glBindTexture(GL_TEXTURE_2D, 0);
        glGenTextures(1, &mExternalTexture);

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
    }

    void testTearDown() override
    {
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
    GLuint mSourceTexture{0};
    EGLImageKHR mExternalImage{nullptr};
    GLuint mExternalTexture{0};
    GLuint mProgram{0};
};

// Test the use of an external texture, make the test friendly for triggering MEC
// that is - use all the created resources in all frames
TEST_P(EGLImageMECFriendlyTest, DrawExternalTextureInLoop)
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

    for (int i = 0; i < 10; ++i)
    {
        glClearColor(i / 10.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        drawQuad(mProgram, "a_position", 0);
        EXPECT_GL_NO_ERROR();
        EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(0, 0, 0, 255), kPixelThreshold);
        EXPECT_PIXEL_COLOR_NEAR(127, 0, GLColor(0, 0, 247, 255), kPixelThreshold);
        EXPECT_PIXEL_COLOR_NEAR(0, 127, GLColor(0, 247, 0, 255), kPixelThreshold);
        EXPECT_PIXEL_COLOR_NEAR(127, 127, GLColor(0, 247, 247, 255), kPixelThreshold);
        swapBuffers();
    }
}

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(EGLImageMECFriendlyTest);
}  // namespace angle

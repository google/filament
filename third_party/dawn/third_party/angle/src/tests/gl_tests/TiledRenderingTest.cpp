//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

namespace angle
{
class TiledRenderingTest : public ANGLETest<>
{
  protected:
    TiledRenderingTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

// Validate that the extension entry points generate errors when the extension is not available.
TEST_P(TiledRenderingTest, ExtensionDisabled)
{
    ANGLE_SKIP_TEST_IF(IsGLExtensionEnabled("GL_QCOM_tiled_rendering"));
    glStartTilingQCOM(0, 0, 1, 1, GL_COLOR_BUFFER_BIT0_QCOM);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    glEndTilingQCOM(GL_COLOR_BUFFER_BIT0_QCOM);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test that tiled rendering can be stared and stopped. Verify that only pixels in bounds are
// written.
TEST_P(TiledRenderingTest, BasicUsage)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_QCOM_tiled_rendering"));

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Blue());

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glStartTilingQCOM(10, 12, 15, 17, GL_COLOR_BUFFER_BIT0_QCOM);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0);

    glEndTilingQCOM(GL_COLOR_BUFFER_BIT0_QCOM);

    const int w = getWindowWidth();
    const int h = getWindowHeight();

    EXPECT_PIXEL_RECT_EQ(10, 12, 15, 17, GLColor::blue);
    EXPECT_PIXEL_RECT_EQ(0, 0, w, 12, GLColor::transparentBlack);
    EXPECT_PIXEL_RECT_EQ(0, 12 + 17, w, h - 12 - 17, GLColor::transparentBlack);
    EXPECT_PIXEL_RECT_EQ(0, 0, 10, h, GLColor::transparentBlack);
    EXPECT_PIXEL_RECT_EQ(10 + 15, 0, w - 10 - 15, h, GLColor::transparentBlack);
}

// Test that changing the framebuffer implicitly ends tiled rendering.
TEST_P(TiledRenderingTest, ImplicitEnd)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_QCOM_tiled_rendering"));

    GLTexture tex1;
    glBindTexture(GL_TEXTURE_2D, tex1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer fbo1;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex1, 0);

    GLTexture tex2;
    glBindTexture(GL_TEXTURE_2D, tex2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer fbo2;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo2);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex2, 0);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Blue());

    glClearColor(0, 0, 0, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo1);
    glClear(GL_COLOR_BUFFER_BIT);
    glStartTilingQCOM(0, 0, 8, 8, GL_COLOR_BUFFER_BIT0_QCOM);
    EXPECT_GL_NO_ERROR();

    drawQuad(program, essl1_shaders::PositionAttrib(), 0);

    // Switch framebuffer bindings. Tiling is implicitly ended and start can be called again without
    // errors.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo2);
    glClear(GL_COLOR_BUFFER_BIT);
    glStartTilingQCOM(8, 8, 8, 8, GL_COLOR_BUFFER_BIT0_QCOM);
    EXPECT_GL_NO_ERROR();

    drawQuad(program, essl1_shaders::PositionAttrib(), 0);

    glEndTilingQCOM(GL_COLOR_BUFFER_BIT0_QCOM);

    // Test that the correct tiling regions were used
    glBindFramebuffer(GL_FRAMEBUFFER, fbo1);
    EXPECT_PIXEL_COLOR_EQ(4, 4, GLColor::blue);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo2);
    EXPECT_PIXEL_COLOR_EQ(12, 12, GLColor::blue);

    // Qualcomm drivers do not implicitly end tiling when changing the texture bound to the
    // framebuffer so ANGLE inherits this behaviour. Validate that tiling is not ended.
    glBindTexture(GL_TEXTURE_2D, tex1);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo1);
    glClear(GL_COLOR_BUFFER_BIT);
    glStartTilingQCOM(4, 4, 4, 4, GL_COLOR_BUFFER_BIT0_QCOM);
    EXPECT_GL_NO_ERROR();

    drawQuad(program, essl1_shaders::PositionAttrib(), 0);

    // Redefine to 8x8 with red data
    std::vector<GLColor> redData(8 * 8, GLColor::red);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, redData.data());

    // If tiling is still continuing from before, should only draw to [4, 4] to (8, 8)
    drawQuad(program, essl1_shaders::PositionAttrib(), 0);

    glStartTilingQCOM(4, 4, 4, 4, GL_COLOR_BUFFER_BIT0_QCOM);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glEndTilingQCOM(GL_COLOR_BUFFER_BIT0_QCOM);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_RECT_EQ(0, 0, 4, 4, GLColor::red);
    EXPECT_PIXEL_RECT_EQ(4, 4, 4, 4, GLColor::blue);
}

// Test that the only written areas are the intersection of scissor and tiled rendering area
TEST_P(TiledRenderingTest, Scissor)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_QCOM_tiled_rendering"));

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Blue());

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, 12, 12);

    glStartTilingQCOM(8, 8, 8, 8, GL_COLOR_BUFFER_BIT0_QCOM);

    // Scissor and tile intersect from [8, 8] to [12, 12]

    drawQuad(program, essl1_shaders::PositionAttrib(), 0);

    glEndTilingQCOM(GL_COLOR_BUFFER_BIT0_QCOM);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::transparentBlack);
    EXPECT_PIXEL_COLOR_EQ(6, 6, GLColor::transparentBlack);
    EXPECT_PIXEL_RECT_EQ(8, 8, 4, 4, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(14, 14, GLColor::transparentBlack);
    EXPECT_PIXEL_COLOR_EQ(18, 18, GLColor::transparentBlack);
}

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(TiledRenderingTest);

}  // namespace angle

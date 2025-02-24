//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// MultisampleTest: Tests of multisampled default framebuffer

#include "test_utils/ANGLETest.h"

#include "test_utils/gl_raii.h"
#include "util/OSWindow.h"
#include "util/shader_utils.h"
#include "util/test_utils.h"

using namespace angle;

namespace
{

class MultisampleTest : public ANGLETest<>
{
  protected:
    MultisampleTest()
    {
        setWindowWidth(kWindowWidth);
        setWindowHeight(kWindowHeight);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
        setConfigStencilBits(8);
        setSamples(4);
        setMultisampleEnabled(true);
    }

    void prepareVertexBuffer(GLBuffer &vertexBuffer,
                             const Vector3 *vertices,
                             size_t vertexCount,
                             GLint positionLocation)
    {
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(*vertices) * vertexCount, vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(positionLocation);
    }

  protected:
    static constexpr int kWindowWidth  = 16;
    static constexpr int kWindowHeight = 8;
};

class MultisampleTestES3 : public MultisampleTest
{};

class MultisampleTestES32 : public MultisampleTest
{};

// Test point rendering on a multisampled surface.  GLES2 section 3.3.1.
TEST_P(MultisampleTest, Point)
{
    // http://anglebug.com/42262135
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsNVIDIAShield() && IsOpenGLES());
    // http://anglebug.com/42264264
    ANGLE_SKIP_TEST_IF(IsOzone());

    constexpr char kPointsVS[] = R"(precision highp float;
attribute vec4 a_position;

void main()
{
    gl_PointSize = 3.0;
    gl_Position = a_position;
})";

    ANGLE_GL_PROGRAM(program, kPointsVS, essl1_shaders::fs::Red());
    glUseProgram(program);
    const GLint positionLocation = glGetAttribLocation(program, "a_position");

    GLBuffer vertexBuffer;
    const Vector3 vertices[1] = {{0.0f, 0.0f, 0.0f}};
    prepareVertexBuffer(vertexBuffer, vertices, 1, positionLocation);

    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_POINTS, 0, 1);

    ASSERT_GL_NO_ERROR();

    // The center pixels should be all red.
    EXPECT_PIXEL_COLOR_EQ(kWindowWidth / 2, kWindowHeight / 2, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kWindowWidth / 2 - 1, kWindowHeight / 2, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kWindowWidth / 2, kWindowHeight / 2 - 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kWindowWidth / 2 - 1, kWindowHeight / 2 - 1, GLColor::red);

    // Border pixels should be between red and black, and not exactly either; corners are darker and
    // sides are brighter.
    const GLColor kSideColor   = {128, 0, 0, 128};
    const GLColor kCornerColor = {64, 0, 0, 64};
    constexpr int kErrorMargin = 16;
    EXPECT_PIXEL_COLOR_NEAR(kWindowWidth / 2 - 2, kWindowHeight / 2 - 2, kCornerColor,
                            kErrorMargin);
    EXPECT_PIXEL_COLOR_NEAR(kWindowWidth / 2 - 2, kWindowHeight / 2 + 1, kCornerColor,
                            kErrorMargin);
    EXPECT_PIXEL_COLOR_NEAR(kWindowWidth / 2 + 1, kWindowHeight / 2 - 2, kCornerColor,
                            kErrorMargin);
    EXPECT_PIXEL_COLOR_NEAR(kWindowWidth / 2 + 1, kWindowHeight / 2 + 1, kCornerColor,
                            kErrorMargin);

    EXPECT_PIXEL_COLOR_NEAR(kWindowWidth / 2 - 2, kWindowHeight / 2 - 1, kSideColor, kErrorMargin);
    EXPECT_PIXEL_COLOR_NEAR(kWindowWidth / 2 - 2, kWindowHeight / 2, kSideColor, kErrorMargin);
    EXPECT_PIXEL_COLOR_NEAR(kWindowWidth / 2 - 1, kWindowHeight / 2 - 2, kSideColor, kErrorMargin);
    EXPECT_PIXEL_COLOR_NEAR(kWindowWidth / 2 - 1, kWindowHeight / 2 + 1, kSideColor, kErrorMargin);
    EXPECT_PIXEL_COLOR_NEAR(kWindowWidth / 2, kWindowHeight / 2 - 2, kSideColor, kErrorMargin);
    EXPECT_PIXEL_COLOR_NEAR(kWindowWidth / 2, kWindowHeight / 2 + 1, kSideColor, kErrorMargin);
    EXPECT_PIXEL_COLOR_NEAR(kWindowWidth / 2 + 1, kWindowHeight / 2 - 1, kSideColor, kErrorMargin);
    EXPECT_PIXEL_COLOR_NEAR(kWindowWidth / 2 + 1, kWindowHeight / 2, kSideColor, kErrorMargin);
}

// Test line rendering on a multisampled surface.  GLES2 section 3.4.4.
TEST_P(MultisampleTest, Line)
{
    ANGLE_SKIP_TEST_IF(IsARM64() && IsWindows() && IsD3D());
    // http://anglebug.com/42264264
    ANGLE_SKIP_TEST_IF(IsOzone());

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    glUseProgram(program);
    const GLint positionLocation = glGetAttribLocation(program, essl1_shaders::PositionAttrib());

    GLBuffer vertexBuffer;
    const Vector3 vertices[2] = {{-1.0f, -0.3f, 0.0f}, {1.0f, 0.3f, 0.0f}};
    prepareVertexBuffer(vertexBuffer, vertices, 2, positionLocation);

    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_LINES, 0, 2);

    ASSERT_GL_NO_ERROR();

    // The line goes from left to right at about -17 degrees slope.  It renders as such (captured
    // with renderdoc):
    //
    // D                    D = Dark Red (0.25) or (0.5)
    //  BRA                 R = Red (1.0)
    //     ARB              M = Middle Red (0.75)
    //        D             B = Bright Red (1.0 or 0.75)
    //                      A = Any red (0.5, 0.75 or 1.0)
    //
    // Verify that rendering is done as above.

    const GLColor kDarkRed     = {128, 0, 0, 128};
    const GLColor kMidRed      = {192, 0, 0, 192};
    constexpr int kErrorMargin = 16;
    constexpr int kLargeMargin = 80;

    static_assert(kWindowWidth == 16, "Verification code written for 16x8 window");
    EXPECT_PIXEL_COLOR_NEAR(0, 2, kDarkRed, kLargeMargin);
    EXPECT_PIXEL_COLOR_NEAR(3, 3, GLColor::red, kLargeMargin);
    EXPECT_PIXEL_COLOR_NEAR(4, 3, GLColor::red, kErrorMargin);
    EXPECT_PIXEL_COLOR_NEAR(6, 3, kMidRed, kLargeMargin);
    EXPECT_PIXEL_COLOR_NEAR(8, 4, kMidRed, kLargeMargin);
    EXPECT_PIXEL_COLOR_NEAR(11, 4, GLColor::red, kErrorMargin);
    EXPECT_PIXEL_COLOR_NEAR(12, 4, GLColor::red, kLargeMargin);
    EXPECT_PIXEL_COLOR_NEAR(15, 5, kDarkRed, kLargeMargin);
}

// Test polygon rendering on a multisampled surface.  GLES2 section 3.5.3.
TEST_P(MultisampleTest, Triangle)
{
    // http://anglebug.com/42262135
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsNVIDIAShield() && IsOpenGLES());
    // http://anglebug.com/42264264
    ANGLE_SKIP_TEST_IF(IsOzone());

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    glUseProgram(program);
    const GLint positionLocation = glGetAttribLocation(program, essl1_shaders::PositionAttrib());

    GLBuffer vertexBuffer;
    const Vector3 vertices[3] = {{-1.0f, -1.0f, 0.0f}, {-1.0f, 1.0f, 0.0f}, {1.0f, -1.0f, 0.0f}};
    prepareVertexBuffer(vertexBuffer, vertices, 3, positionLocation);

    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    ASSERT_GL_NO_ERROR();

    // Top-left pixels should be all red.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kWindowWidth / 4, kWindowHeight / 4, GLColor::red);

    // Diagonal pixels from bottom-left to top-right are between red and black.  Pixels above the
    // diagonal are red and pixels below it are black.
    const GLColor kMidRed = {128, 0, 0, 128};
    // D3D11 is off by 63 for red (191 instead of 128), where other back-ends get 128
    constexpr int kErrorMargin = 64;

    for (int i = 2; i + 2 < kWindowWidth; i += 2)
    {
        int j = kWindowHeight - 1 - (i / 2);
        EXPECT_PIXEL_COLOR_NEAR(i, j, kMidRed, kErrorMargin);
        EXPECT_PIXEL_COLOR_EQ(i, j - 1, GLColor::red);
        EXPECT_PIXEL_COLOR_EQ(i, j + 1, GLColor::transparentBlack);
    }
}

// Test polygon rendering on a multisampled surface. And rendering is interrupted by a compute pass
// that converts the index buffer. Make sure the rendering's multisample result is preserved after
// interruption.
TEST_P(MultisampleTest, ContentPresevedAfterInterruption)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_rgb8_rgba8"));
    // http://anglebug.com/42262135
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsNVIDIAShield() && IsOpenGLES());
    // http://anglebug.com/42263216
    ANGLE_SKIP_TEST_IF(IsD3D11());
    // http://anglebug.com/42264264
    ANGLE_SKIP_TEST_IF(IsOzone());

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    glUseProgram(program);
    const GLint positionLocation = glGetAttribLocation(program, essl1_shaders::PositionAttrib());

    if (IsGLExtensionEnabled("GL_EXT_discard_framebuffer"))
    {
        GLenum attachments[] = {GL_COLOR_EXT, GL_DEPTH_EXT, GL_STENCIL_EXT};
        glDiscardFramebufferEXT(GL_FRAMEBUFFER, 3, attachments);
    }
    // Draw triangle
    GLBuffer vertexBuffer;
    const Vector3 vertices[3] = {{-1.0f, -1.0f, 0.0f}, {-1.0f, 1.0f, 0.0f}, {1.0f, -1.0f, 0.0f}};
    prepareVertexBuffer(vertexBuffer, vertices, 3, positionLocation);

    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    ASSERT_GL_NO_ERROR();

    // Draw a line
    GLBuffer vertexBuffer2;
    GLBuffer indexBuffer2;
    const Vector3 vertices2[2] = {{-1.0f, -0.3f, 0.0f}, {1.0f, 0.3f, 0.0f}};
    const GLubyte indices2[]   = {0, 1};
    prepareVertexBuffer(vertexBuffer2, vertices2, 2, positionLocation);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer2);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices2), indices2, GL_STATIC_DRAW);

    glDrawElements(GL_LINES, 2, GL_UNSIGNED_BYTE, 0);

    ASSERT_GL_NO_ERROR();

    // Top-left pixels should be all red.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kWindowWidth / 4, kWindowHeight / 4, GLColor::red);

    // Triangle edge:
    // Diagonal pixels from bottom-left to top-right are between red and black.  Pixels above the
    // diagonal are red and pixels below it are black.
    {
        const GLColor kMidRed      = {128, 0, 0, 128};
        constexpr int kErrorMargin = 16;

        for (int i = 2; i + 2 < kWindowWidth; i += 2)
        {
            // Exclude the middle pixel where the triangle and line cross each other.
            if (abs(kWindowHeight / 2 - (i / 2)) <= 1)
            {
                continue;
            }
            int j = kWindowHeight - 1 - (i / 2);
            EXPECT_PIXEL_COLOR_NEAR(i, j, kMidRed, kErrorMargin);
            EXPECT_PIXEL_COLOR_EQ(i, j - 1, GLColor::red);
            EXPECT_PIXEL_COLOR_EQ(i, j + 1, GLColor::transparentBlack);
        }
    }

    // Line edge:
    {
        const GLColor kDarkRed     = {128, 0, 0, 128};
        constexpr int kErrorMargin = 16;
        constexpr int kLargeMargin = 80;

        static_assert(kWindowWidth == 16, "Verification code written for 16x8 window");
        // Exclude the triangle region.
        EXPECT_PIXEL_COLOR_NEAR(11, 4, GLColor::red, kErrorMargin);
        EXPECT_PIXEL_COLOR_NEAR(12, 4, GLColor::red, kLargeMargin);
        EXPECT_PIXEL_COLOR_NEAR(15, 5, kDarkRed, kLargeMargin);
    }
}

// Test that alpha to coverage is enabled works properly along with early fragment test.
TEST_P(MultisampleTest, AlphaToSampleCoverage)
{
    // http://anglebug.com/42264264
    ANGLE_SKIP_TEST_IF(IsOzone());

    constexpr char kFS[] =
        "precision highp float;\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = vec4(1.0, 0.0, 0.0, 0.0);\n"
        "}\n";
    ANGLE_GL_PROGRAM(transparentRedProgram, essl1_shaders::vs::Simple(), kFS);
    glUseProgram(transparentRedProgram);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearDepthf(1.0f);
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);  // clear to green
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // This should pass depth test, but because of the alpha to coverage enabled, and alpha is 0,
    // the fragment should be discarded. If early fragment test is disabled, no depth will be
    // written. depth buffer should be 1.0.
    glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    // There was a bug in ANGLE that we are checking sampler coverage enabled or not instead of
    // alpha to sample coverage enabled or not. This is specically try to trick ANGLE so that it
    // will enable early fragment test. When early fragment test is accidentally enabled, then the
    // depth test will occur before fragment shader, and depth buffer maybe written with value
    // (0.0+1.0)/2.0=0.5.
    glEnable(GL_SAMPLE_COVERAGE);
    drawQuad(transparentRedProgram, essl1_shaders::PositionAttrib(), 0.0f);

    // Now draw with blue color but to test against 0.0f. This should fail depth test
    glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    glDisable(GL_SAMPLE_COVERAGE);
    glDepthFunc(GL_GREATER);
    ANGLE_GL_PROGRAM(blueProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
    // Zd = 0.5f means (0.5+1.0)/2.0=0.75. Depends on early fragment on or off this will pass or
    // fail depth test.
    drawQuad(blueProgram, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::green);

    ASSERT_GL_NO_ERROR();
}

// Test that resolve from multisample default framebuffer works.
TEST_P(MultisampleTestES3, ResolveToFBO)
{
    GLTexture resolveTexture;
    glBindTexture(GL_TEXTURE_2D, resolveTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWindowWidth, kWindowHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLFramebuffer resolveFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture, 0);

    // Clear the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.25, 0.5, 0.75, 0.25);
    glClear(GL_COLOR_BUFFER_BIT);

    // Resolve into FBO
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glBlitFramebuffer(0, 0, kWindowWidth, kWindowHeight, 0, 0, kWindowWidth, kWindowHeight,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    const GLColor kResult = GLColor(63, 127, 191, 63);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, kResult, 1);
    EXPECT_PIXEL_COLOR_NEAR(kWindowWidth - 1, 0, kResult, 1);
    EXPECT_PIXEL_COLOR_NEAR(0, kWindowHeight - 1, kResult, 1);
    EXPECT_PIXEL_COLOR_NEAR(kWindowWidth - 1, kWindowHeight - 1, kResult, 1);
    EXPECT_PIXEL_COLOR_NEAR(kWindowWidth / 2, kWindowHeight / 2, kResult, 1);
}

// Test that resolve from multisample default framebuffer after an open render pass works when the
// framebuffer is also immediately implicitly resolved due to swap afterwards.
TEST_P(MultisampleTestES3, RenderPassResolveToFBOThenSwap)
{
    GLTexture resolveTexture;
    glBindTexture(GL_TEXTURE_2D, resolveTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWindowWidth, kWindowHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLFramebuffer resolveFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture, 0);

    auto runTest = [&](bool flipY) {
        // Open a render pass by drawing to the default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        ANGLE_GL_PROGRAM(red, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
        drawQuad(red, essl1_shaders::PositionAttrib(), 0.5f);

        // Resolve into FBO
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
        if (flipY)
        {
            glFramebufferParameteriMESA(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, 1);
        }
        glBlitFramebuffer(0, 0, kWindowWidth, kWindowHeight, 0, 0, kWindowWidth, kWindowHeight,
                          GL_COLOR_BUFFER_BIT, GL_NEAREST);
        ASSERT_GL_NO_ERROR();

        // Immediately swap so that an implicit resolve to the backbuffer happens right away.
        swapBuffers();

        glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO);
        EXPECT_PIXEL_RECT_EQ(0, 0, kWindowWidth, kWindowHeight, GLColor::red);
    };

    runTest(false);
    if (IsGLExtensionEnabled("GL_MESA_framebuffer_flip_y"))
    {
        // With multiple backends, the default framebuffer is flipped w.r.t GL's coordinates.  As a
        // result, the glBlitFramebuffer may need to take a different path from a direct multisample
        // resolve.  This test ensures a direct resolve is also tested where possible.
        runTest(true);
    }
}

// Test that CopyTexImage2D from an MSAA default fbo works
TEST_P(MultisampleTestES3, CopyTexImage2DFromMsaaDefaultFbo)
{
    constexpr char kFS[] = R"(#version 300 es
#extension GL_OES_sample_variables : require
precision highp float;
out vec4 my_FragColor;

void main()
{
    switch (uint(gl_SampleID))
    {
        case 0u:
            my_FragColor = vec4(1.0, 0.9, 0.8, 0.7);
            break;
        case 1u:
            my_FragColor = vec4(0.0, 0.1, 0.2, 0.3);
            break;
        case 2u:
            my_FragColor = vec4(0.5, 0.25, 0.75, 1.0);
            break;
        case 3u:
            my_FragColor = vec4(0.4, 0.6, 0.2, 0.8);
            break;
        default:
            my_FragColor = vec4(0.0);
            break;
    }
}
)";

    ANGLE_GL_PROGRAM(programMs, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(programMs);

    // Clear the default framebuffer and draw
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.25, 0.5, 0.75, 0.25);
    glClear(GL_COLOR_BUFFER_BIT);
    drawQuad(programMs, essl3_shaders::PositionAttrib(), 0.0);

    // Create a texture for copy
    GLTexture copyTexture;
    glBindTexture(GL_TEXTURE_2D, copyTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWindowWidth, kWindowHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    constexpr int kCopyWidth  = 10;
    constexpr int kCopyHeight = 5;
    // Copy MSAA default framebuffer into the texture
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, kCopyWidth, kCopyHeight, 0);
    ASSERT_GL_NO_ERROR();

    GLFramebuffer readFbo;
    glBindFramebuffer(GL_FRAMEBUFFER, readFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, copyTexture, 0);

    const GLColor kResult = GLColor(121, 118, 124, 178);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, kResult, 1);
    EXPECT_PIXEL_COLOR_NEAR(kCopyWidth - 1, 0, kResult, 1);
    EXPECT_PIXEL_COLOR_NEAR(0, kCopyHeight - 1, kResult, 1);
    EXPECT_PIXEL_COLOR_NEAR(kCopyWidth - 1, kCopyHeight - 1, kResult, 1);
    EXPECT_PIXEL_COLOR_NEAR(kCopyWidth / 2, kCopyHeight / 2, kResult, 1);
}

class MultisampleResolveTest : public ANGLETest<>
{
  protected:
    static const GLColor kEXPECTED_R8;
    static const GLColor kEXPECTED_RG8;
    static const GLColor kEXPECTED_RGB8;
    static const GLColor kEXPECTED_RGBA8;
    static const GLColor32F kEXPECTED_RF;
    static const GLColor32F kEXPECTED_RGF;
    static const GLColor32F kEXPECTED_RGBF;
    static const GLColor32F kEXPECTED_RGBAF;
    static constexpr GLint kWidth  = 13;
    static constexpr GLint kHeight = 11;

    MultisampleResolveTest() {}

    struct GLResources
    {
        GLFramebuffer fb;
        GLRenderbuffer rb;
    };

    void resolveToFBO(GLenum format,
                      GLint samples,
                      GLint width,
                      GLint height,
                      GLResources &resources)
    {
        constexpr char kVS[] = R"(#version 300 es
        layout(location = 0) in vec4 position;
        void main() {
           gl_Position = position;
        }
        )";

        constexpr char kFS[] = R"(#version 300 es
        precision highp float;
        out vec4 color;
        void main() {
           color = vec4(0.5, 0.6, 0.7, 0.8);
        }
        )";

        ANGLE_GL_PROGRAM(program, kVS, kFS);

        // Make samples = 4 multi-sample framebuffer.
        GLFramebuffer fb0;
        glBindFramebuffer(GL_FRAMEBUFFER, fb0);

        GLRenderbuffer rb0;
        glBindRenderbuffer(GL_RENDERBUFFER, rb0);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, format, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb0);
        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        // Make samples = 0 multi-sample framebuffer.
        glBindFramebuffer(GL_FRAMEBUFFER, resources.fb);

        glBindRenderbuffer(GL_RENDERBUFFER, resources.rb);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 0, format, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                  resources.rb);
        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        // Draw quad to fb0.
        glBindFramebuffer(GL_FRAMEBUFFER, fb0);
        glViewport(0, 0, width, height);
        glUseProgram(program);
        GLBuffer buf;
        glBindBuffer(GL_ARRAY_BUFFER, buf);

        constexpr float vertices[] = {
            -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Blit fb0 to fb1.
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fb0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resources.fb);
        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT,
                          GL_NEAREST);
        ASSERT_GL_NO_ERROR();

        // Prep for read pixels.
        glBindFramebuffer(GL_READ_FRAMEBUFFER, resources.fb);
    }

    void testResolveToUNormFBO(GLenum format,
                               const GLColor &expected_color,
                               GLint samples,
                               GLint width,
                               GLint height)
    {
        GLResources resources;
        resolveToFBO(format, samples, width, height, resources);

        // Check the results
        for (GLint y = 0; y < kHeight; ++y)
        {
            for (GLint x = 0; x < kWidth; ++x)
            {
                EXPECT_PIXEL_COLOR_NEAR(x, y, expected_color, 2);
            }
        }
        ASSERT_GL_NO_ERROR();
    }

    void testResolveToHalfFBO(GLenum format,
                              const GLColor32F &expected_color,
                              GLint samples,
                              GLint width,
                              GLint height)
    {
        if (!IsGLExtensionEnabled("GL_EXT_color_buffer_half_float"))
        {
            return;
        }

        GLResources resources;
        resolveToFBO(format, samples, width, height, resources);

        // Check the results
        for (GLint y = 0; y < kHeight; ++y)
        {
            for (GLint x = 0; x < kWidth; ++x)
            {
                EXPECT_PIXEL_COLOR32F_NEAR(x, y, expected_color, 2.0f / 255.0f);
            }
        }
        ASSERT_GL_NO_ERROR();
    }

    void testResolveToFloatFBO(GLenum format,
                               const GLColor32F &expected_color,
                               GLint samples,
                               GLint width,
                               GLint height)
    {
        if (!IsGLExtensionEnabled("GL_CHROMIUM_color_buffer_float_rgba"))
        {
            return;
        }

        GLResources resources;
        resolveToFBO(format, samples, width, height, resources);

        // Check the results
        for (GLint y = 0; y < kHeight; ++y)
        {
            for (GLint x = 0; x < kWidth; ++x)
            {
                EXPECT_PIXEL_COLOR32F_NEAR(x, y, expected_color, 2.0f / 255.0f);
            }
        }
        ASSERT_GL_NO_ERROR();
    }

    void testResolveToRGBFloatFBO(GLenum format,
                                  const GLColor32F &expected_color,
                                  GLint samples,
                                  GLint width,
                                  GLint height)
    {
        if (!IsGLExtensionEnabled("GL_CHROMIUM_color_buffer_float_rgb"))
        {
            return;
        }

        GLResources resources;
        resolveToFBO(format, samples, width, height, resources);

        // Check the results
        for (GLint y = 0; y < kHeight; ++y)
        {
            for (GLint x = 0; x < kWidth; ++x)
            {
                EXPECT_PIXEL_COLOR32F_NEAR(x, y, expected_color, 2.0f / 255.0f);
            }
        }
        ASSERT_GL_NO_ERROR();
    }

    void peelDepth(GLint colorLoc)
    {
        // Draw full quads from front to back and increasing depths
        // with depth test = less.
        glDepthMask(GL_FALSE);
        constexpr int steps = 64;
        for (int i = 0; i < steps; ++i)
        {
            float l = float(i) / float(steps);
            float c = l;
            float z = c * 2.0f - 1.0f;
            glVertexAttrib4f(1, 0, 0, z, 0);
            glUniform4f(colorLoc, c, c, c, c);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        glDepthMask(GL_TRUE);
    }

    void testResolveDepthToFBO(GLenum format,
                               GLenum attachment,
                               GLint samples,
                               GLint width,
                               GLint height)
    {
        constexpr char kVS[] = R"(#version 300 es
        layout(location = 0) in vec4 position;
        void main() {
           gl_Position = position;
        }
        )";

        constexpr char kFS[] = R"(#version 300 es
        precision highp float;
        out vec4 color;
        void main() {
           color = vec4(0.5, 0.6, 0.7, 0.8);
        }
        )";

        constexpr char kDepthVS[] = R"(#version 300 es
        layout(location = 0) in vec4 position;
        layout(location = 1) in vec4 offset;
        void main() {
           gl_Position = position + offset;
        }
        )";

        constexpr char kDepthFS[] = R"(#version 300 es
        precision highp float;
        uniform vec4 color;
        out vec4 outColor;
        void main() {
           outColor = color;
        }
        )";

        ANGLE_GL_PROGRAM(program, kVS, kFS);
        ANGLE_GL_PROGRAM(depthProgram, kDepthVS, kDepthFS);

        // Make samples = 4 multi-sample framebuffer.
        GLFramebuffer fb0;
        glBindFramebuffer(GL_FRAMEBUFFER, fb0);

        GLRenderbuffer rb0;
        glBindRenderbuffer(GL_RENDERBUFFER, rb0);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_RGBA8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb0);

        GLRenderbuffer db0;
        glBindRenderbuffer(GL_RENDERBUFFER, db0);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, format, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, db0);

        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        // Make samples = 0 multi-sample framebuffer.
        GLFramebuffer fb1;
        glBindFramebuffer(GL_FRAMEBUFFER, fb1);

        GLRenderbuffer rb1;
        glBindRenderbuffer(GL_RENDERBUFFER, rb1);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb1);
        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        GLRenderbuffer db1;
        glBindRenderbuffer(GL_RENDERBUFFER, db1);
        glRenderbufferStorage(GL_RENDERBUFFER, format, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, db1);

        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        // Draw quad to fb0.
        glBindFramebuffer(GL_FRAMEBUFFER, fb0);
        glViewport(0, 0, width, height);
        glClearColor(1, 1, 1, 1);
        glUseProgram(program);

        GLVertexArray va0;
        glBindVertexArray(va0);

        GLBuffer buf0;
        glBindBuffer(GL_ARRAY_BUFFER, buf0);

        // clang-format off
        constexpr float vertices[] = {
            -1.0f, -1.0f, -1.0,
             1.0f, -1.0f,  0.0,
            -1.0f,  1.0f,  0.0,
            -1.0f,  1.0f,  0.0,
             1.0f, -1.0f,  0.0,
             1.0f,  1.0f,  1.0,
        };
        // clang-format on

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_ALWAYS);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Blit fb0 to fb1.
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fb0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb1);
        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT,
                          GL_NEAREST);
        ASSERT_GL_NO_ERROR();

        GLVertexArray va1;
        glBindVertexArray(va1);

        // clang-format off
        constexpr float depthVertices[] = {
            -1.0f, -1.0f,
             1.0f, -1.0f,
            -1.0f,  1.0f,
            -1.0f,  1.0f,
             1.0f, -1.0f,
             1.0f,  1.0f,
        };
        // clang-format on

        GLBuffer buf1;
        glBindBuffer(GL_ARRAY_BUFFER, buf1);
        glBufferData(GL_ARRAY_BUFFER, sizeof(depthVertices), depthVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        glUseProgram(depthProgram);
        GLint colorLoc = glGetUniformLocation(depthProgram, "color");

        // Extract the depth results.
        glBindFramebuffer(GL_FRAMEBUFFER, fb1);
        glClear(GL_COLOR_BUFFER_BIT);
        glDepthFunc(GL_LESS);
        peelDepth(colorLoc);
        std::vector<GLColor> actual(width * height);
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, actual.data());

        // Render what should be a similar result to the non-multi-sampled fb
        glBindVertexArray(va0);
        glDepthFunc(GL_ALWAYS);
        glUseProgram(program);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Extract the expected depth results.
        glBindVertexArray(va1);
        glUseProgram(depthProgram);
        glClear(GL_COLOR_BUFFER_BIT);
        glDepthFunc(GL_LESS);
        peelDepth(colorLoc);
        std::vector<GLColor> expected(width * height);
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, expected.data());

        for (size_t i = 0; i < expected.size(); ++i)
        {
            EXPECT_NEAR(expected[i].R, actual[i].R, 8)
                << "at " << (i % width) << "," << (i / width);
        }

        // Verify we read the depth buffer.
        const GLint minDimension = std::min(width, height);
        for (GLint i = 1; i < minDimension; ++i)
        {
            const GLColor &c1 = expected[i - 1];
            const GLColor &c2 = expected[i * width + i];
            EXPECT_LT(c1.R, c2.R);
        }
        ASSERT_GL_NO_ERROR();
    }
};

// Test the multisampled optimized resolve subpass
TEST_P(MultisampleResolveTest, DISABLED_ResolveSubpassMSImage)
{
    ANGLE_GL_PROGRAM(greenProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());

    // Draw green.
    drawQuad(greenProgram, essl1_shaders::PositionAttrib(), 0.5f);
    swapBuffers();

    // Wait for visual verification.
    angle::Sleep(2000);
}

// This is a test that must be verified visually.
//
// Tests that clear of the default framebuffer with multisample applies to the window.
TEST_P(MultisampleTestES3, DISABLED_ClearMSAAReachesWindow)
{
    ANGLE_GL_PROGRAM(blueProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());

    // Draw blue.
    drawQuad(blueProgram, essl1_shaders::PositionAttrib(), 0.5f);
    swapBuffers();

    // Use glClear to clear to red.  Regression test for the Vulkan backend where this clear
    // remained "deferred" and didn't make it to the window on swap.
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    swapBuffers();

    // Wait for visual verification.
    angle::Sleep(2000);
}

// According to the spec, the minimum value for the multisample line width range limits is one.
TEST_P(MultisampleTestES32, MultisampleLineWidthRangeCheck)
{
    GLfloat range[2] = {0, 0};
    glGetFloatv(GL_MULTISAMPLE_LINE_WIDTH_RANGE, range);
    EXPECT_GL_NO_ERROR();
    EXPECT_GE(range[0], 1.0f);
    EXPECT_GE(range[1], 1.0f);
}

// The multisample line width granularity should not be negative.
TEST_P(MultisampleTestES32, MultisampleLineWidthGranularityCheck)
{
    GLfloat granularity = -1.0f;
    glGetFloatv(GL_MULTISAMPLE_LINE_WIDTH_GRANULARITY, &granularity);
    EXPECT_GL_NO_ERROR();
    EXPECT_GE(granularity, 0.0f);
}

// These colors match the shader in resolveToFBO which returns (0.5, 0.6, 0.7, 0.8).
const GLColor MultisampleResolveTest::kEXPECTED_R8(128, 0, 0, 255);
const GLColor MultisampleResolveTest::kEXPECTED_RG8(128, 153, 0, 255);
const GLColor MultisampleResolveTest::kEXPECTED_RGB8(128, 153, 178, 255);
const GLColor MultisampleResolveTest::kEXPECTED_RGBA8(128, 153, 178, 204);
const GLColor32F MultisampleResolveTest::kEXPECTED_RF(0.5f, 0.0f, 0.0f, 1.0f);
const GLColor32F MultisampleResolveTest::kEXPECTED_RGF(0.5f, 0.6f, 0.0f, 1.0f);
const GLColor32F MultisampleResolveTest::kEXPECTED_RGBF(0.5f, 0.6f, 0.7f, 1.0f);
const GLColor32F MultisampleResolveTest::kEXPECTED_RGBAF(0.5f, 0.6f, 0.7f, 0.8f);

// Test we can render to and resolve an RGBA8 renderbuffer
TEST_P(MultisampleResolveTest, ResolveRGBA8ToFBO4Samples)
{
    testResolveToUNormFBO(GL_RGBA8, kEXPECTED_RGBA8, 4, kWidth, kHeight);
}

// Test we can render to and resolve an RGB8 renderbuffer
TEST_P(MultisampleResolveTest, ResolveRGB8ToFBO4Samples)
{
    testResolveToUNormFBO(GL_RGB8, kEXPECTED_RGB8, 4, kWidth, kHeight);
}

// Test we can render to and resolve an RG8 renderbuffer
TEST_P(MultisampleResolveTest, ResolveRG8ToFBO4Samples)
{
    testResolveToUNormFBO(GL_RG8, kEXPECTED_RG8, 4, kWidth, kHeight);
}

// Test we can render to and resolve an R8 renderbuffer
TEST_P(MultisampleResolveTest, ResolveR8ToFBO4Samples)
{
    testResolveToUNormFBO(GL_R8, kEXPECTED_R8, 4, kWidth, kHeight);
}

// Test we can render to and resolve an R16F renderbuffer
TEST_P(MultisampleResolveTest, ResolveR16FToFBO4Samples)
{
    testResolveToHalfFBO(GL_R16F, kEXPECTED_RF, 4, kWidth, kHeight);
}

// Test we can render to and resolve an RG16F renderbuffer
TEST_P(MultisampleResolveTest, ResolveRG16FToFBO4Samples)
{
    testResolveToHalfFBO(GL_RG16F, kEXPECTED_RGF, 4, kWidth, kHeight);
}

// Test we can render to and resolve an RGB16F renderbuffer
TEST_P(MultisampleResolveTest, ResolveRGB16FToFBO4Samples)
{
    testResolveToHalfFBO(GL_RGB16F, kEXPECTED_RGBF, 4, kWidth, kHeight);
}

// Test we can render to and resolve an RGBA16F renderbuffer
TEST_P(MultisampleResolveTest, ResolveRGBA16FToFBO4Samples)
{
    testResolveToHalfFBO(GL_RGBA16F, kEXPECTED_RGBAF, 4, kWidth, kHeight);
}

// Test we can render to and resolve an R32F renderbuffer
TEST_P(MultisampleResolveTest, ResolveR32FToFBO4Samples)
{
    testResolveToFloatFBO(GL_R32F, kEXPECTED_RF, 4, kWidth, kHeight);
}

// Test we can render to and resolve an RG32F renderbuffer
TEST_P(MultisampleResolveTest, ResolveRG32FToFBO4Samples)
{
    testResolveToFloatFBO(GL_RG32F, kEXPECTED_RGF, 4, kWidth, kHeight);
}

// Test we can render to and resolve an RGB32F renderbuffer
TEST_P(MultisampleResolveTest, ResolveRGB32FToFBO4Samples)
{
    testResolveToRGBFloatFBO(GL_RGB32F, kEXPECTED_RGBF, 4, kWidth, kHeight);
}

// Test we can render to and resolve an RGBA32F renderbuffer
TEST_P(MultisampleResolveTest, ResolveRGBA32FToFBO4Samples)
{
    testResolveToFloatFBO(GL_RGBA32F, kEXPECTED_RGBAF, 4, kWidth, kHeight);
}

TEST_P(MultisampleResolveTest, ResolveD32FS8F4Samples)
{
    testResolveDepthToFBO(GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, 4, kWidth, kHeight);
}

TEST_P(MultisampleResolveTest, ResolveD24S8Samples)
{
    testResolveDepthToFBO(GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, 4, kWidth, kHeight);
}

TEST_P(MultisampleResolveTest, ResolveD32FSamples)
{
    testResolveDepthToFBO(GL_DEPTH_COMPONENT32F, GL_DEPTH_ATTACHMENT, 4, kWidth, kHeight);
}

TEST_P(MultisampleResolveTest, ResolveD24Samples)
{
    testResolveDepthToFBO(GL_DEPTH_COMPONENT24, GL_DEPTH_ATTACHMENT, 4, kWidth, kHeight);
}

TEST_P(MultisampleResolveTest, ResolveD16Samples)
{
    testResolveDepthToFBO(GL_DEPTH_COMPONENT16, GL_DEPTH_ATTACHMENT, 4, kWidth, kHeight);
}

void drawRectAndBlit(GLuint msFramebuffer,
                     GLuint resolveFramebuffer,
                     GLint width,
                     GLint height,
                     GLint matLoc,
                     GLint colorLoc,
                     float x,
                     float y,
                     float w,
                     float h,
                     const GLColor &color)
{
    glBindFramebuffer(GL_FRAMEBUFFER, msFramebuffer);
    float matrix[16] = {
        w, 0, 0, 0, 0, h, 0, 0, 0, 0, 1, 0, x, y, 0, 1,
    };
    glUniformMatrix4fv(matLoc, 1, false, matrix);
    angle::Vector4 c(color.toNormalizedVector());
    glUniform4f(colorLoc, c[0], c[1], c[2], c[3]);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFramebuffer);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

// Tests if we resolve(blit) a multisample renderbuffer that it
// does not lose its contents.
TEST_P(MultisampleResolveTest, DrawAndResolveMultipleTimes)
{
    constexpr GLint samples = 4;
    constexpr GLenum format = GL_RGBA8;
    constexpr GLint width   = 16;
    constexpr GLint height  = 16;

    constexpr char kVS[] = R"(#version 300 es
    layout(location = 0) in vec4 position;
    uniform mat4 mat;
    void main() {
       gl_Position = mat * position;
    }
    )";

    constexpr char kFS[] = R"(#version 300 es
    precision highp float;
    uniform vec4 color;
    out vec4 outColor;
    void main() {
       outColor = color;
    }
    )";

    glViewport(0, 0, width, height);

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    GLint matLoc   = glGetUniformLocation(program, "mat");
    GLint colorLoc = glGetUniformLocation(program, "color");
    glUseProgram(program);

    GLBuffer buf;
    glBindBuffer(GL_ARRAY_BUFFER, buf);

    constexpr float vertices[] = {
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Make samples = 4 multi-sample framebuffer.
    GLFramebuffer msFB;
    glBindFramebuffer(GL_FRAMEBUFFER, msFB);

    GLRenderbuffer msRB;
    glBindRenderbuffer(GL_RENDERBUFFER, msRB);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, format, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, msRB);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Make non-multi-sample framebuffer.
    GLFramebuffer drawFB;
    glBindFramebuffer(GL_FRAMEBUFFER, drawFB);

    GLRenderbuffer drawRB;
    glBindRenderbuffer(GL_RENDERBUFFER, drawRB);
    glRenderbufferStorage(GL_RENDERBUFFER, format, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, drawRB);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    drawRectAndBlit(msFB, drawFB, width, height, matLoc, colorLoc, -1, -1, 2, 2, GLColor::red);
    drawRectAndBlit(msFB, drawFB, width, height, matLoc, colorLoc, 0, -1, 1, 1, GLColor::green);
    drawRectAndBlit(msFB, drawFB, width, height, matLoc, colorLoc, -1, 0, 1, 1, GLColor::blue);
    drawRectAndBlit(msFB, drawFB, width, height, matLoc, colorLoc, 0, 0, 1, 1, GLColor::yellow);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    drawRectAndBlit(msFB, drawFB, width, height, matLoc, colorLoc, -0.5, -0.5, 1, 1,
                    GLColor(0x80, 0x80, 0x80, 0x80));
    glDisable(GL_BLEND);
    ASSERT_GL_NO_ERROR();

    /*
       expected
       +-------------+--------------+
       | blue        |       yellow |
       |   +---------+----------+   |
       |   |.5,.5,1,1| 1,1,.5,1 |   |
       +---+---------+----------+---+
       |   |1,.5,.5,1| .5,1,.5,1|   |
       |   +---------+----------+   |
       | red         |        green |
       +-------------+--------------+
      0,0
    */

    glBindFramebuffer(GL_FRAMEBUFFER, drawFB);
    EXPECT_PIXEL_RECT_EQ(0, 0, width / 2, height / 4, GLColor::red);
    EXPECT_PIXEL_RECT_EQ(width / 2, 0, width / 2, height / 4, GLColor::green);
    EXPECT_PIXEL_RECT_EQ(0, height * 3 / 4, width / 2, height / 4, GLColor::blue);
    EXPECT_PIXEL_RECT_EQ(width / 2, height * 3 / 4, width / 2, height / 4, GLColor::yellow);

    EXPECT_PIXEL_RECT_EQ(width / 4, height / 4, width / 4, height / 4, GLColor(255, 128, 128, 255));
    EXPECT_PIXEL_RECT_EQ(width / 2, height / 4, width / 4, height / 4, GLColor(128, 255, 128, 255));
    EXPECT_PIXEL_RECT_EQ(width / 4, height / 2, width / 4, height / 4, GLColor(128, 128, 255, 255));
    EXPECT_PIXEL_RECT_EQ(width / 2, height / 2, width / 4, height / 4, GLColor(255, 255, 128, 255));
}

// Tests resolve after the read framebuffer's attachment has been swapped out.
TEST_P(MultisampleResolveTest, SwitchAttachmentsBeforeResolve)
{
    constexpr GLuint kWidth  = 16;
    constexpr GLuint kHeight = 24;

    GLRenderbuffer msaaColor0, msaaColor1;
    glBindRenderbuffer(GL_RENDERBUFFER, msaaColor0);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, kWidth, kHeight);
    glBindRenderbuffer(GL_RENDERBUFFER, msaaColor1);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, kWidth, kHeight);

    {
        // First, make one of the MSAA color buffers green.
        GLFramebuffer clearFbo;
        glBindFramebuffer(GL_FRAMEBUFFER, clearFbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                  msaaColor1);
        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        glClearColor(0, 1, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        // Make sure clear is performed.
        GLTexture resolveTexture;
        glBindTexture(GL_TEXTURE_2D, resolveTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);

        GLFramebuffer verifyFBO;
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, verifyFBO);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               resolveTexture, 0);

        glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT,
                          GL_NEAREST);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, verifyFBO);
        EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::green);
    }

    // Set up the msaa framebuffer with the other MSAA color buffer.
    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, msaaColor0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Draw into it to start a render pass
    ANGLE_GL_PROGRAM(red, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    drawQuad(red, essl1_shaders::PositionAttrib(), 0.5f);

    // Set up the resolve framebuffer
    GLRenderbuffer resolveColor;
    glBindRenderbuffer(GL_RENDERBUFFER, resolveColor);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kWidth, kHeight);

    GLFramebuffer resolveFBO;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                              resolveColor);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);

    // Before resolve the MSAA framebuffer, switch its attachment.  Regression test for a bug where
    // the resolve used the previous attachment.
    glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                              msaaColor1);

    glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);

    // Verify that the resolve happened on the pre-cleared attachment, not the rendered one.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO);
    EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::green);
}

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3_AND_ES31_AND(
    MultisampleTest,
    ES3_VULKAN().enable(Feature::EmulatedPrerotation90),
    ES3_VULKAN().enable(Feature::EmulatedPrerotation180),
    ES3_VULKAN().enable(Feature::EmulatedPrerotation270),
    // Simulate missing msaa auto resolve feature in Metal.
    ES2_METAL().disable(Feature::AllowMultisampleStoreAndResolve));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(MultisampleTestES3);
ANGLE_INSTANTIATE_TEST_ES3_AND_ES31_AND(MultisampleTestES3,
                                        ES3_VULKAN().enable(Feature::EmulatedPrerotation90),
                                        ES3_VULKAN().enable(Feature::EmulatedPrerotation180),
                                        ES3_VULKAN().enable(Feature::EmulatedPrerotation270));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(MultisampleTestES32);
ANGLE_INSTANTIATE_TEST_ES32_AND(MultisampleTestES32,
                                ES32_VULKAN().enable(Feature::EmulatedPrerotation90),
                                ES32_VULKAN().enable(Feature::EmulatedPrerotation180),
                                ES32_VULKAN().enable(Feature::EmulatedPrerotation270));

ANGLE_INSTANTIATE_TEST_ES3_AND(
    MultisampleResolveTest,
    ES3_VULKAN().enable(Feature::EmulatedPrerotation90),
    ES3_VULKAN().enable(Feature::EmulatedPrerotation180),
    ES3_VULKAN().enable(Feature::EmulatedPrerotation270),
    ES3_VULKAN().enable(Feature::PreferDrawClearOverVkCmdClearAttachments));

}  // anonymous namespace

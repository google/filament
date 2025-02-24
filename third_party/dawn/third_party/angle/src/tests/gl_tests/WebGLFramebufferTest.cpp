//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// WebGLFramebufferTest.cpp : Framebuffer tests for GL_ANGLE_webgl_compatibility.
// Based on WebGL 1 test renderbuffers/framebuffer-object-attachment.

#include "test_utils/ANGLETest.h"

#include "test_utils/gl_raii.h"

namespace angle
{

class WebGLFramebufferTest : public ANGLETest<>
{
  protected:
    WebGLFramebufferTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setWebGLCompatibilityEnabled(true);
    }

    void drawUByteColorQuad(GLuint program, GLint uniformLoc, const GLColor &color);
    void testDepthStencilDepthStencil(GLint width, GLint height);
    void testDepthStencilRenderbuffer(GLint width,
                                      GLint height,
                                      GLRenderbuffer *colorBuffer,
                                      GLbitfield allowedStatuses);
    void testRenderingAndReading(GLuint program);
    void testUsingIncompleteFramebuffer(GLenum depthFormat, GLenum depthAttachment);
    void testDrawingMissingAttachment();
};

constexpr GLint ALLOW_COMPLETE              = 0x1;
constexpr GLint ALLOW_UNSUPPORTED           = 0x2;
constexpr GLint ALLOW_INCOMPLETE_ATTACHMENT = 0x4;

void checkFramebufferForAllowedStatuses(GLbitfield allowedStatuses)
{
    // If the framebuffer is in an error state for multiple reasons,
    // we can't guarantee which one will be reported.
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    bool statusAllowed =
        ((allowedStatuses & ALLOW_COMPLETE) && (status == GL_FRAMEBUFFER_COMPLETE)) ||
        ((allowedStatuses & ALLOW_UNSUPPORTED) && (status == GL_FRAMEBUFFER_UNSUPPORTED)) ||
        ((allowedStatuses & ALLOW_INCOMPLETE_ATTACHMENT) &&
         (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT));
    EXPECT_TRUE(statusAllowed);
}

void checkBufferBits(GLenum attachment0, GLenum attachment1)
{
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        return;

    bool haveDepthBuffer =
        attachment0 == GL_DEPTH_ATTACHMENT || attachment0 == GL_DEPTH_STENCIL_ATTACHMENT ||
        attachment1 == GL_DEPTH_ATTACHMENT || attachment1 == GL_DEPTH_STENCIL_ATTACHMENT;
    bool haveStencilBuffer =
        attachment0 == GL_STENCIL_ATTACHMENT || attachment0 == GL_DEPTH_STENCIL_ATTACHMENT ||
        attachment1 == GL_STENCIL_ATTACHMENT || attachment1 == GL_DEPTH_STENCIL_ATTACHMENT;

    GLint redBits     = 0;
    GLint greenBits   = 0;
    GLint blueBits    = 0;
    GLint alphaBits   = 0;
    GLint depthBits   = 0;
    GLint stencilBits = 0;
    glGetIntegerv(GL_RED_BITS, &redBits);
    glGetIntegerv(GL_GREEN_BITS, &greenBits);
    glGetIntegerv(GL_BLUE_BITS, &blueBits);
    glGetIntegerv(GL_ALPHA_BITS, &alphaBits);
    glGetIntegerv(GL_DEPTH_BITS, &depthBits);
    glGetIntegerv(GL_STENCIL_BITS, &stencilBits);

    EXPECT_GE(redBits + greenBits + blueBits + alphaBits, 16);

    if (haveDepthBuffer)
        EXPECT_GE(depthBits, 16);
    else
        EXPECT_EQ(0, depthBits);

    if (haveStencilBuffer)
        EXPECT_GE(stencilBits, 8);
    else
        EXPECT_EQ(0, stencilBits);
}

// Tests that certain required combinations work in WebGL compatiblity.
TEST_P(WebGLFramebufferTest, TestFramebufferRequiredCombinations)
{
    // Per discussion with the OpenGL ES working group, the following framebuffer attachment
    // combinations are required to work in all WebGL implementations:
    // 1. COLOR_ATTACHMENT0 = RGBA/UNSIGNED_BYTE texture
    // 2. COLOR_ATTACHMENT0 = RGBA/UNSIGNED_BYTE texture + DEPTH_ATTACHMENT = DEPTH_COMPONENT16
    // renderbuffer
    // 3. COLOR_ATTACHMENT0 = RGBA/UNSIGNED_BYTE texture + DEPTH_STENCIL_ATTACHMENT = DEPTH_STENCIL
    // renderbuffer

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    constexpr int width  = 64;
    constexpr int height = 64;

    // 1. COLOR_ATTACHMENT0 = RGBA/UNSIGNED_BYTE texture
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_NO_ERROR();
    checkFramebufferForAllowedStatuses(ALLOW_COMPLETE);
    checkBufferBits(GL_NONE, GL_NONE);

    // 2. COLOR_ATTACHMENT0 = RGBA/UNSIGNED_BYTE texture + DEPTH_ATTACHMENT = DEPTH_COMPONENT16
    // renderbuffer
    GLRenderbuffer renderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);
    EXPECT_GL_NO_ERROR();
    checkFramebufferForAllowedStatuses(ALLOW_COMPLETE);
    checkBufferBits(GL_DEPTH_ATTACHMENT, GL_NONE);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);

    if (getClientMajorVersion() == 2)
    {
        // 3. COLOR_ATTACHMENT0 = RGBA/UNSIGNED_BYTE texture + DEPTH_STENCIL_ATTACHMENT =
        // DEPTH_STENCIL renderbuffer
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                  renderbuffer);
        EXPECT_GL_NO_ERROR();
        checkFramebufferForAllowedStatuses(ALLOW_COMPLETE);
        checkBufferBits(GL_DEPTH_STENCIL_ATTACHMENT, GL_NONE);
    }
}

void testAttachment(GLint width,
                    GLint height,
                    GLRenderbuffer *colorBuffer,
                    GLenum attachment,
                    GLRenderbuffer *buffer,
                    GLbitfield allowedStatuses)
{
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, *colorBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, *buffer);
    EXPECT_GL_NO_ERROR();
    checkFramebufferForAllowedStatuses(allowedStatuses);
    if ((allowedStatuses & ALLOW_COMPLETE) == 0)
    {
        std::vector<uint8_t> tempBuffer(width * height * 4);

        glClear(GL_COLOR_BUFFER_BIT);
        EXPECT_GL_ERROR(GL_INVALID_FRAMEBUFFER_OPERATION);
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, tempBuffer.data());
        EXPECT_GL_ERROR(GL_INVALID_FRAMEBUFFER_OPERATION);
    }
    checkBufferBits(attachment, GL_NONE);
}

void testAttachments(GLRenderbuffer &colorBuffer,
                     GLenum attachment0,
                     GLRenderbuffer &buffer0,
                     GLenum attachment1,
                     GLRenderbuffer &buffer1,
                     GLbitfield allowedStatuses)
{
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment0, GL_RENDERBUFFER, buffer0);
    EXPECT_GL_NO_ERROR();
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment1, GL_RENDERBUFFER, buffer1);
    EXPECT_GL_NO_ERROR();
    checkFramebufferForAllowedStatuses(allowedStatuses);
    checkBufferBits(attachment0, attachment1);
}

void testColorRenderbuffer(GLint width,
                           GLint height,
                           GLenum internalformat,
                           GLbitfield allowedStatuses)
{
    GLRenderbuffer colorBuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, internalformat, width, height);
    EXPECT_GL_NO_ERROR();
    testAttachment(width, height, &colorBuffer, GL_COLOR_ATTACHMENT0, &colorBuffer,
                   allowedStatuses);
}

GLint getRenderbufferParameter(GLenum paramName)
{
    GLint paramValue = 0;
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, paramName, &paramValue);
    return paramValue;
}

void WebGLFramebufferTest::drawUByteColorQuad(GLuint program,
                                              GLint uniformLoc,
                                              const GLColor &color)
{
    Vector4 vecColor = color.toNormalizedVector();
    glUseProgram(program);
    glUniform4fv(uniformLoc, 1, vecColor.data());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
}

void WebGLFramebufferTest::testDepthStencilDepthStencil(GLint width, GLint height)
{
    if (width == 0 || height == 0)
    {
        return;
    }

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    GLint uniformLoc = glGetUniformLocation(program, essl1_shaders::ColorUniform());
    ASSERT_NE(-1, uniformLoc);

    struct TestInfo
    {
        GLenum firstFormat;
        GLenum firstAttach;
        GLenum secondFormat;
        GLenum secondAttach;
    };

    TestInfo tests[2] = {
        {GL_DEPTH_COMPONENT16, GL_DEPTH_ATTACHMENT, GL_DEPTH_STENCIL, GL_DEPTH_STENCIL_ATTACHMENT},
        {GL_DEPTH_STENCIL, GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH_COMPONENT16, GL_DEPTH_ATTACHMENT}};

    for (const TestInfo &test : tests)
    {
        for (GLint opIndex = 0; opIndex < 2; ++opIndex)
        {
            GLFramebuffer fbo;
            GLTexture tex;
            GLRenderbuffer firstRb;

            // test: firstFormat vs secondFormat with unbind or delete.

            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            // attach texture as color
            glBindTexture(GL_TEXTURE_2D, tex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                         nullptr);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

            // attach first
            glBindRenderbuffer(GL_RENDERBUFFER, firstRb);
            glRenderbufferStorage(GL_RENDERBUFFER, test.firstFormat, width, height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, test.firstAttach, GL_RENDERBUFFER, firstRb);

            EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

            // TODO(jmadill): Remove clear - this should be implicit in WebGL_
            glClear(GL_DEPTH_BUFFER_BIT);

            glEnable(GL_DEPTH_TEST);
            // Test it works
            drawUByteColorQuad(program, uniformLoc, GLColor::green);
            // should not draw since DEPTH_FUNC == LESS
            drawUByteColorQuad(program, uniformLoc, GLColor::red);
            // should be green
            EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

            GLuint secondRb = 0;
            glGenRenderbuffers(1, &secondRb);

            // attach second
            glBindRenderbuffer(GL_RENDERBUFFER, secondRb);
            glRenderbufferStorage(GL_RENDERBUFFER, test.secondFormat, width, height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, test.secondAttach, GL_RENDERBUFFER, secondRb);

            if (opIndex == 0)
            {
                // now delete it
                glDeleteRenderbuffers(1, &secondRb);
                secondRb = 0;
            }
            else
            {
                // unbind it
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, test.secondAttach, GL_RENDERBUFFER, 0);
            }

            // If the first attachment is not restored this may fail
            EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
            EXPECT_GL_NO_ERROR();

            // If the first attachment is not restored this may fail.
            glClear(GL_DEPTH_BUFFER_BIT);
            drawUByteColorQuad(program, uniformLoc, GLColor::green);
            // should not draw since DEPTH_FUNC == LESS
            drawUByteColorQuad(program, uniformLoc, GLColor::red);
            // should be green
            EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
            glDisable(GL_DEPTH_TEST);

            if (opIndex == 1)
            {
                glDeleteRenderbuffers(1, &secondRb);
                secondRb = 0;
            }
        }
    }
    EXPECT_GL_NO_ERROR();
}

void WebGLFramebufferTest::testDepthStencilRenderbuffer(GLint width,
                                                        GLint height,
                                                        GLRenderbuffer *colorBuffer,
                                                        GLbitfield allowedStatuses)
{
    GLRenderbuffer depthStencilBuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, depthStencilBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, width, height);
    EXPECT_GL_NO_ERROR();

    // OpenGL itself doesn't seem to guarantee that e.g. a 2 x 0
    // renderbuffer will report 2 for its width when queried.
    if (!(height == 0 && width > 0))
    {
        EXPECT_EQ(width, getRenderbufferParameter(GL_RENDERBUFFER_WIDTH));
    }
    if (!(width == 0 && height > 0))
    {
        EXPECT_EQ(height, getRenderbufferParameter(GL_RENDERBUFFER_HEIGHT));
    }
    EXPECT_EQ(GL_DEPTH_STENCIL, getRenderbufferParameter(GL_RENDERBUFFER_INTERNAL_FORMAT));
    EXPECT_EQ(0, getRenderbufferParameter(GL_RENDERBUFFER_RED_SIZE));
    EXPECT_EQ(0, getRenderbufferParameter(GL_RENDERBUFFER_GREEN_SIZE));
    EXPECT_EQ(0, getRenderbufferParameter(GL_RENDERBUFFER_BLUE_SIZE));
    EXPECT_EQ(0, getRenderbufferParameter(GL_RENDERBUFFER_ALPHA_SIZE));

    // Avoid verifying these for zero-sized renderbuffers for the time
    // being since it appears that even OpenGL doesn't guarantee them.
    if (width > 0 && height > 0)
    {
        EXPECT_GT(getRenderbufferParameter(GL_RENDERBUFFER_DEPTH_SIZE), 0);
        EXPECT_GT(getRenderbufferParameter(GL_RENDERBUFFER_STENCIL_SIZE), 0);
    }
    EXPECT_GL_NO_ERROR();
    testAttachment(width, height, colorBuffer, GL_DEPTH_STENCIL_ATTACHMENT, &depthStencilBuffer,
                   allowedStatuses);
    testDepthStencilDepthStencil(width, height);
}

// Test various attachment combinations with WebGL framebuffers.
TEST_P(WebGLFramebufferTest, TestAttachments)
{
    // GL_DEPTH_STENCIL renderbuffer format is only valid for WebGL1
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() != 2);

    for (GLint width = 2; width <= 2; width += 2)
    {
        for (GLint height = 2; height <= 2; height += 2)
        {
            // Dimensions width x height.
            GLRenderbuffer colorBuffer;
            glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, width, height);
            EXPECT_GL_NO_ERROR();

            GLRenderbuffer depthBuffer;
            glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
            EXPECT_GL_NO_ERROR();

            GLRenderbuffer stencilBuffer;
            glBindRenderbuffer(GL_RENDERBUFFER, stencilBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, width, height);
            EXPECT_GL_NO_ERROR();

            GLRenderbuffer depthStencilBuffer;
            glBindRenderbuffer(GL_RENDERBUFFER, depthStencilBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, width, height);
            EXPECT_GL_NO_ERROR();

            GLbitfield allowedStatusForGoodCase =
                (width == 0 || height == 0) ? ALLOW_INCOMPLETE_ATTACHMENT : ALLOW_COMPLETE;

            // some cases involving stencil seem to be implementation-dependent
            GLbitfield allowedStatusForImplDependentCase =
                allowedStatusForGoodCase | ALLOW_UNSUPPORTED;

            // Attach depth using DEPTH_ATTACHMENT.
            testAttachment(width, height, &colorBuffer, GL_DEPTH_ATTACHMENT, &depthBuffer,
                           allowedStatusForGoodCase);

            // Attach depth using STENCIL_ATTACHMENT.
            testAttachment(width, height, &colorBuffer, GL_STENCIL_ATTACHMENT, &depthBuffer,
                           ALLOW_INCOMPLETE_ATTACHMENT);

            // Attach depth using DEPTH_STENCIL_ATTACHMENT.
            testAttachment(width, height, &colorBuffer, GL_DEPTH_STENCIL_ATTACHMENT, &depthBuffer,
                           ALLOW_INCOMPLETE_ATTACHMENT);

            // Attach stencil using STENCIL_ATTACHMENT.
            testAttachment(width, height, &colorBuffer, GL_STENCIL_ATTACHMENT, &stencilBuffer,
                           allowedStatusForImplDependentCase);

            // Attach stencil using DEPTH_ATTACHMENT.
            testAttachment(width, height, &colorBuffer, GL_DEPTH_ATTACHMENT, &stencilBuffer,
                           ALLOW_INCOMPLETE_ATTACHMENT);

            // Attach stencil using DEPTH_STENCIL_ATTACHMENT.
            testAttachment(width, height, &colorBuffer, GL_DEPTH_STENCIL_ATTACHMENT, &stencilBuffer,
                           ALLOW_INCOMPLETE_ATTACHMENT);

            // Attach depthStencil using DEPTH_STENCIL_ATTACHMENT.
            testAttachment(width, height, &colorBuffer, GL_DEPTH_STENCIL_ATTACHMENT,
                           &depthStencilBuffer, allowedStatusForGoodCase);

            // Attach depthStencil using DEPTH_ATTACHMENT.
            testAttachment(width, height, &colorBuffer, GL_DEPTH_ATTACHMENT, &depthStencilBuffer,
                           ALLOW_INCOMPLETE_ATTACHMENT);

            // Attach depthStencil using STENCIL_ATTACHMENT.
            testAttachment(width, height, &colorBuffer, GL_STENCIL_ATTACHMENT, &depthStencilBuffer,
                           ALLOW_INCOMPLETE_ATTACHMENT);

            GLbitfield allowedStatusForConflictedAttachment =
                (width == 0 || height == 0) ? ALLOW_UNSUPPORTED | ALLOW_INCOMPLETE_ATTACHMENT
                                            : ALLOW_UNSUPPORTED;

            // Attach depth, then stencil, causing conflict.
            testAttachments(colorBuffer, GL_DEPTH_ATTACHMENT, depthBuffer, GL_STENCIL_ATTACHMENT,
                            stencilBuffer, allowedStatusForConflictedAttachment);

            // Attach stencil, then depth, causing conflict.
            testAttachments(colorBuffer, GL_STENCIL_ATTACHMENT, stencilBuffer, GL_DEPTH_ATTACHMENT,
                            depthBuffer, allowedStatusForConflictedAttachment);

            // Attach depth, then depthStencil, causing conflict.
            testAttachments(colorBuffer, GL_DEPTH_ATTACHMENT, depthBuffer,
                            GL_DEPTH_STENCIL_ATTACHMENT, depthStencilBuffer,
                            allowedStatusForConflictedAttachment);

            // Attach depthStencil, then depth, causing conflict.
            testAttachments(colorBuffer, GL_DEPTH_STENCIL_ATTACHMENT, depthStencilBuffer,
                            GL_DEPTH_ATTACHMENT, depthBuffer, allowedStatusForConflictedAttachment);

            // Attach stencil, then depthStencil, causing conflict.
            testAttachments(colorBuffer, GL_DEPTH_ATTACHMENT, depthBuffer,
                            GL_DEPTH_STENCIL_ATTACHMENT, depthStencilBuffer,
                            allowedStatusForConflictedAttachment);

            // Attach depthStencil, then stencil, causing conflict.
            testAttachments(colorBuffer, GL_DEPTH_STENCIL_ATTACHMENT, depthStencilBuffer,
                            GL_STENCIL_ATTACHMENT, stencilBuffer,
                            allowedStatusForConflictedAttachment);

            // Attach color renderbuffer with internalformat == RGBA4.
            testColorRenderbuffer(width, height, GL_RGBA4, allowedStatusForGoodCase);

            // Attach color renderbuffer with internalformat == RGB5_A1.
            // This particular format seems to be bugged on NVIDIA Retina. http://crbug.com/635081
            // TODO(jmadill): Figure out if we can add a format workaround.
            if (!(IsNVIDIA() && IsMac() && IsOpenGL()))
            {
                testColorRenderbuffer(width, height, GL_RGB5_A1, allowedStatusForGoodCase);
            }

            // Attach color renderbuffer with internalformat == RGB565.
            testColorRenderbuffer(width, height, GL_RGB565, allowedStatusForGoodCase);

            // Create and attach depthStencil renderbuffer.
            testDepthStencilRenderbuffer(width, height, &colorBuffer, allowedStatusForGoodCase);
        }
    }
}

bool tryDepth(GLRenderbuffer *depthBuffer,
              GLenum *depthFormat,
              GLenum *depthAttachment,
              GLenum try_format,
              GLenum try_attachment)
{
    if (*depthAttachment != GL_NONE)
    {
        // If we've tried once unattach the old one.
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, *depthAttachment, GL_RENDERBUFFER, 0);
    }
    *depthFormat     = try_format;
    *depthAttachment = try_attachment;
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, *depthAttachment, GL_RENDERBUFFER, *depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, *depthFormat, 16, 16);
    EXPECT_GL_NO_ERROR();
    return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

bool checkValidColorDepthCombination(GLenum *depthFormat, GLenum *depthAttachment)
{
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    GLRenderbuffer colorBuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, 16, 16);

    GLRenderbuffer depthBuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);

    return tryDepth(&depthBuffer, depthFormat, depthAttachment, GL_DEPTH_COMPONENT16,
                    GL_DEPTH_ATTACHMENT) ||
           tryDepth(&depthBuffer, depthFormat, depthAttachment, GL_DEPTH_STENCIL,
                    GL_DEPTH_STENCIL_ATTACHMENT);
}

// glCheckFramebufferStatus(GL_FRAMEBUFFER) should be either complete or (unsupported/expected).
void checkFramebuffer(GLenum expected)
{
    GLenum actual = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    EXPECT_TRUE(actual == expected ||
                (expected != GL_FRAMEBUFFER_COMPLETE && actual == GL_FRAMEBUFFER_UNSUPPORTED));
}

void WebGLFramebufferTest::testRenderingAndReading(GLuint program)
{
    EXPECT_GL_NO_ERROR();

    // drawArrays with incomplete framebuffer
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_GL_ERROR(GL_INVALID_FRAMEBUFFER_OPERATION);

    // readPixels from incomplete framebuffer
    std::vector<uint8_t> incompleteBuffer(4);
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, incompleteBuffer.data());
    EXPECT_GL_ERROR(GL_INVALID_FRAMEBUFFER_OPERATION);

    // copyTexImage and copyTexSubImage can be either INVALID_FRAMEBUFFER_OPERATION because
    // the framebuffer is invalid OR INVALID_OPERATION because in the case of no attachments
    // the framebuffer is not of a compatible type.

    // copyTexSubImage2D from incomplete framebuffer
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, 1, 1);
    GLenum error = glGetError();
    EXPECT_TRUE(error == GL_INVALID_FRAMEBUFFER_OPERATION || error == GL_INVALID_OPERATION);

    // copyTexImage2D from incomplete framebuffer
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 1, 1, 0);
    error = glGetError();
    EXPECT_TRUE(error == GL_INVALID_FRAMEBUFFER_OPERATION || error == GL_INVALID_OPERATION);

    // clear with incomplete framebuffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    EXPECT_GL_ERROR(GL_INVALID_FRAMEBUFFER_OPERATION);
}

// Test drawing or reading from an incomplete framebuffer
void WebGLFramebufferTest::testUsingIncompleteFramebuffer(GLenum depthFormat,
                                                          GLenum depthAttachment)
{
    // Simple draw program.
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    GLRenderbuffer colorBuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, 16, 16);

    GLRenderbuffer depthBuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, depthAttachment, GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, depthFormat, 16, 16);
    EXPECT_GL_NO_ERROR();
    checkFramebuffer(GL_FRAMEBUFFER_COMPLETE);

    // We pick this combination because it works on desktop OpenGL but should not work on OpenGL ES
    // 2.0
    glRenderbufferStorage(GL_RENDERBUFFER, depthFormat, 32, 16);
    checkFramebuffer(GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS);

    // Drawing or reading from an incomplete framebuffer should generate
    // INVALID_FRAMEBUFFER_OPERATION.
    testRenderingAndReading(program);

    GLFramebuffer fbo2;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo2);
    checkFramebuffer(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);

    // Drawing or reading from an incomplete framebuffer should generate
    // INVALID_FRAMEBUFFER_OPERATION.
    testRenderingAndReading(program);

    GLRenderbuffer colorBuffer2;
    glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer2);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBuffer2);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, 0, 0);

    // Drawing or reading from an incomplete framebuffer should generate
    // INVALID_FRAMEBUFFER_OPERATION.
    testRenderingAndReading(program);
}

void testFramebufferIncompleteAttachment(GLenum depthFormat)
{
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    GLRenderbuffer colorBuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, 16, 16);
    checkFramebuffer(GL_FRAMEBUFFER_COMPLETE);

    // Wrong storage type for type of attachment be FRAMEBUFFER_INCOMPLETE_ATTACHMENT (OpenGL ES 2.0
    // 4.4.5).
    glRenderbufferStorage(GL_RENDERBUFFER, depthFormat, 16, 16);
    checkFramebuffer(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, 16, 16);
    checkFramebuffer(GL_FRAMEBUFFER_COMPLETE);

    // 0 size attachment should be FRAMEBUFFER_INCOMPLETE_ATTACHMENT (OpenGL ES 2.0 4.4.5).
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, 0, 0);
    checkFramebuffer(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);

    EXPECT_GL_NO_ERROR();
}

// No attachments should be INCOMPLETE_FRAMEBUFFER_MISSING_ATTACHMENT (OpenGL ES 2.0 4.4.5).
void testFramebufferIncompleteMissingAttachment()
{
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    checkFramebuffer(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);

    GLRenderbuffer colorBuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, 16, 16);
    checkFramebuffer(GL_FRAMEBUFFER_COMPLETE);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, 0);
    checkFramebuffer(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);

    EXPECT_GL_NO_ERROR();
}

// Attachments of different sizes should be FRAMEBUFFER_INCOMPLETE_DIMENSIONS (OpenGL ES 2.0 4.4.5).
void testFramebufferIncompleteDimensions(GLenum depthFormat, GLenum depthAttachment)
{
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    GLRenderbuffer colorBuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, 16, 16);

    GLRenderbuffer depthBuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, depthAttachment, GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, depthFormat, 16, 16);
    EXPECT_GL_NO_ERROR();
    checkFramebuffer(GL_FRAMEBUFFER_COMPLETE);

    glRenderbufferStorage(GL_RENDERBUFFER, depthFormat, 32, 16);
    checkFramebuffer(GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS);
    glRenderbufferStorage(GL_RENDERBUFFER, depthFormat, 16, 16);
    checkFramebuffer(GL_FRAMEBUFFER_COMPLETE);
    glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, 16, 32);
    checkFramebuffer(GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, 16, 16);
    checkFramebuffer(GL_FRAMEBUFFER_COMPLETE);
    EXPECT_GL_NO_ERROR();

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    EXPECT_GL_NO_ERROR();
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        return;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    checkFramebuffer(GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    checkFramebuffer(GL_FRAMEBUFFER_COMPLETE);

    EXPECT_GL_NO_ERROR();
}

class NoColorFB final : angle::NonCopyable
{
  public:
    NoColorFB(int size)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

        // The only scenario we can verify is an attempt to read or copy
        // from a missing color attachment while the framebuffer is still
        // complete.
        glBindRenderbuffer(GL_RENDERBUFFER, mDepthBuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                                  mDepthBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, size, size);

        // After depth renderbuffer setup
        EXPECT_GL_NO_ERROR();

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            // Unable to allocate a framebuffer with just a depth attachment; this is legal.
            // Try just a depth/stencil renderbuffer.
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);

            glBindRenderbuffer(GL_RENDERBUFFER, mDepthStencilBuffer);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                      mDepthStencilBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, size, size);

            // After depth+stencil renderbuffer setup
            EXPECT_GL_NO_ERROR();
        }
    }

  private:
    GLRenderbuffer mDepthBuffer;
    GLRenderbuffer mDepthStencilBuffer;
    GLFramebuffer mFBO;
};

// Test reading from a missing framebuffer attachment.
void TestReadingMissingAttachment(int size)
{
    // The FBO has no color attachment. ReadPixels, CopyTexImage2D,
    // and CopyTexSubImage2D should all generate INVALID_OPERATION.

    // Before ReadPixels from missing attachment
    std::vector<uint8_t> incompleteBuffer(4);
    EXPECT_GL_NO_ERROR();
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, incompleteBuffer.data());
    // After ReadPixels from missing attachment
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    // Before CopyTexImage2D from missing attachment
    EXPECT_GL_NO_ERROR();
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, size, size, 0);
    // After CopyTexImage2D from missing attachment
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    // Before CopyTexSubImage2D from missing attachment
    EXPECT_GL_NO_ERROR();
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, size, size);
    // After CopyTexSubImage2D from missing attachment
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test drawing to a missing framebuffer attachment.
void WebGLFramebufferTest::testDrawingMissingAttachment()
{
    // Simple draw program.
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());

    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    // try glDrawArrays
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_GL_NO_ERROR();

    // try glDrawElements
    drawIndexedQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_GL_NO_ERROR();
}

// Determine if we can attach both color and depth or color and depth_stencil
TEST_P(WebGLFramebufferTest, CheckValidColorDepthCombination)
{
    GLenum depthFormat     = GL_NONE;
    GLenum depthAttachment = GL_NONE;

    if (checkValidColorDepthCombination(&depthFormat, &depthAttachment))
    {
        testFramebufferIncompleteDimensions(depthFormat, depthAttachment);
        testFramebufferIncompleteAttachment(depthFormat);
        testFramebufferIncompleteMissingAttachment();
        testUsingIncompleteFramebuffer(depthFormat, depthAttachment);

        constexpr int size = 16;
        NoColorFB fb(size);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
        {
            // The FBO has no color attachment. ReadPixels, CopyTexImage2D,
            // and CopyTexSubImage2D should all generate INVALID_OPERATION.
            TestReadingMissingAttachment(size);

            // The FBO has no color attachment. Clear, DrawArrays,
            // and DrawElements should not generate an error.
            testDrawingMissingAttachment();
        }
    }
}

// Test to cover a bug in preserving the texture image index for WebGL framebuffer attachments
TEST_P(WebGLFramebufferTest, TextureAttachmentCommitBug)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_depth_texture"));

    GLTexture depthTexture;
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1, 1, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT,
                 nullptr);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

    glCheckFramebufferStatus(GL_FRAMEBUFFER);

    EXPECT_GL_NO_ERROR();
}

// Test combinations of ordering in setting the resource format and attaching it to the depth
// stencil attacchment.  Covers http://crbug.com/997702
TEST_P(WebGLFramebufferTest, DepthStencilAttachmentOrdering)
{
    constexpr GLsizei kFramebufferSize = 16;

    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kFramebufferSize, kFramebufferSize, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);

    GLRenderbuffer depthStencil;
    glBindRenderbuffer(GL_RENDERBUFFER, depthStencil);

    // Attach the renderbuffer to the framebuffer when it has no format
    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencil);

    // Framebuffer is incomplete because the depth stencil attachment doesn't a format/size
    EXPECT_GL_NO_ERROR();
    EXPECT_GLENUM_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER),
                     GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);

    // Set depth stencil attachment to a color format
    if (EnsureGLExtensionEnabled("GL_OES_rgb8_rgba8"))
    {
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kFramebufferSize, kFramebufferSize);

        // Non-depth stencil format on the depth stencil attachment
        EXPECT_GL_NO_ERROR();
        EXPECT_GLENUM_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER),
                         GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
    }

    {
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, kFramebufferSize,
                              kFramebufferSize);

        // Depth-stencil attachment only has a depth format, not complete
        EXPECT_GL_NO_ERROR();
        EXPECT_GLENUM_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER),
                         GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
    }

    if (EnsureGLExtensionEnabled("GL_OES_packed_depth_stencil"))
    {
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, kFramebufferSize,
                              kFramebufferSize);

        // Framebuffer should be complete now with a depth-stencil format
        EXPECT_GL_NO_ERROR();
        EXPECT_GLENUM_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER), GL_FRAMEBUFFER_COMPLETE);
    }
}

// Only run against WebGL 1 validation, since much was changed in 2.
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(WebGLFramebufferTest);

}  // namespace angle

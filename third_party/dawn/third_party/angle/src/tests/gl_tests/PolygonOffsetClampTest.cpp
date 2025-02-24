//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PolygonOffsetClampTest.cpp: Test cases for GL_EXT_polygon_offset_clamp
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"
#include "util/test_utils.h"

using namespace angle;

class PolygonOffsetClampTest : public ANGLETest<>
{
  protected:
    PolygonOffsetClampTest()
    {
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
        setExtensionsEnabled(false);
    }
};

// Test state queries and updates
TEST_P(PolygonOffsetClampTest, State)
{
    // New state query fails without the extension
    {
        GLfloat clamp = -1.0f;
        glGetFloatv(GL_POLYGON_OFFSET_CLAMP_EXT, &clamp);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
        EXPECT_EQ(clamp, -1.0f);

        ASSERT_GL_NO_ERROR();
    }

    // New function does nothing without enabling the extension
    {
        glPolygonOffsetClampEXT(1.0f, 2.0f, 3.0f);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);

        GLfloat factor = -1.0f;
        glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &factor);
        EXPECT_EQ(factor, 0.0f);

        GLfloat units = -1.0f;
        glGetFloatv(GL_POLYGON_OFFSET_UNITS, &units);
        EXPECT_EQ(units, 0.0f);

        ASSERT_GL_NO_ERROR();
    }

    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_polygon_offset_clamp"));

    // Default state
    {
        GLfloat factor = -1.0f;
        glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &factor);
        EXPECT_EQ(factor, 0.0f);

        GLfloat units = -1.0f;
        glGetFloatv(GL_POLYGON_OFFSET_UNITS, &units);
        EXPECT_EQ(units, 0.0f);

        GLfloat clamp = -1.0f;
        glGetFloatv(GL_POLYGON_OFFSET_CLAMP_EXT, &clamp);
        EXPECT_EQ(clamp, 0.0f);

        ASSERT_GL_NO_ERROR();
    }

    // Full state update using the new function
    {
        glPolygonOffsetClampEXT(1.0f, 2.0f, 3.0f);

        GLfloat factor = -1.0f;
        glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &factor);
        EXPECT_EQ(factor, 1.0f);

        GLfloat units = -1.0f;
        glGetFloatv(GL_POLYGON_OFFSET_UNITS, &units);
        EXPECT_EQ(units, 2.0f);

        GLfloat clamp = -1.0f;
        glGetFloatv(GL_POLYGON_OFFSET_CLAMP_EXT, &clamp);
        EXPECT_EQ(clamp, 3.0f);

        ASSERT_GL_NO_ERROR();
    }

    // Core function resets the clamp value to zero
    {
        glPolygonOffset(4.0f, 5.0f);

        GLfloat factor = -1.0f;
        glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &factor);
        EXPECT_EQ(factor, 4.0f);

        GLfloat units = -1.0f;
        glGetFloatv(GL_POLYGON_OFFSET_UNITS, &units);
        EXPECT_EQ(units, 5.0f);

        GLfloat clamp = -1.0f;
        glGetFloatv(GL_POLYGON_OFFSET_CLAMP_EXT, &clamp);
        EXPECT_EQ(clamp, 0.0f);

        ASSERT_GL_NO_ERROR();
    }

    // NaNs must be accepted and replaced with zeros
    {
        glPolygonOffsetClampEXT(6.0f, 7.0f, 8.0f);

        float nan = std::numeric_limits<float>::quiet_NaN();
        glPolygonOffsetClampEXT(nan, nan, nan);

        GLfloat factor = -1.0f;
        glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &factor);
        EXPECT_EQ(factor, 0.0f);

        GLfloat units = -1.0f;
        glGetFloatv(GL_POLYGON_OFFSET_UNITS, &units);
        EXPECT_EQ(units, 0.0f);

        GLfloat clamp = -1.0f;
        glGetFloatv(GL_POLYGON_OFFSET_CLAMP_EXT, &clamp);
        EXPECT_EQ(clamp, 0.0f);

        ASSERT_GL_NO_ERROR();
    }
}

// Test polygon offset clamping behavior. Ported from dEQP.
TEST_P(PolygonOffsetClampTest, Operation)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_polygon_offset_clamp"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_depth_texture"));

    GLTexture depthTexture;
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 64, 64, 0, GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_SHORT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

    GLTexture colorReadbackTexture;
    glBindTexture(GL_TEXTURE_2D, colorReadbackTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLFramebuffer fboReadback;
    glBindFramebuffer(GL_FRAMEBUFFER, fboReadback);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           colorReadbackTexture, 0);

    ANGLE_GL_PROGRAM(testProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());

    constexpr char kFS[] = R"(
varying highp vec2 v_texCoord;
uniform highp sampler2D tex;
void main()
{
    // Store as unorm24
    highp float d = floor(texture2D(tex, v_texCoord).r * 16777215.0);
    highp float r = floor(d / 65536.0);
    highp float g = floor(mod(d, 65536.0) / 256.0);
    highp float b = mod(d, 256.0);
    gl_FragColor = vec4(r, g, b, 1.0) / 255.0;
})";

    ANGLE_GL_PROGRAM(readDepthProgram, essl1_shaders::vs::Texture2D(), kFS);

    // Setup depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);

    // Bind framebuffer for drawing
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Offset units and clamp values. Should work with all depth buffer formats.
    const std::vector<std::array<GLfloat, 2>> testValues = {
        {-5000.0f, -0.0001f},
        {+5000.0f, +0.0001f},
        {-5000.0f, 0.0f},
        {+5000.0f, 0.0f},
        {-5000.0f, -std::numeric_limits<float>::infinity()},
        {+5000.0f, +std::numeric_limits<float>::infinity()}};

    auto readDepthValue = [&]() {
        glBindFramebuffer(GL_FRAMEBUFFER, fboReadback);
        glDisable(GL_DEPTH_TEST);
        glUseProgram(readDepthProgram);
        glUniform1i(glGetUniformLocation(readDepthProgram, "tex"), 0);

        glBindTexture(GL_TEXTURE_2D, depthTexture);
        drawQuad(readDepthProgram, "a_position", 0.5);

        GLubyte pixels[4];
        glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

        // Convert read depth value to GLfloat normalized
        GLfloat depthValue = (GLfloat)(pixels[0] * 65536 + pixels[1] * 256 + pixels[2]) / 0xFFFFFF;

        // Restore state
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glEnable(GL_DEPTH_TEST);
        glUseProgram(testProgram);

        return depthValue;
    };

    for (auto testValue : testValues)
    {
        const GLfloat units = testValue[0];
        const GLfloat clamp = testValue[1];

        // Draw reference polygon
        glDisable(GL_POLYGON_OFFSET_FILL);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        drawQuad(testProgram, "a_position", 0.0);

        // Get reference depth value
        const GLfloat depthValue = readDepthValue();

        // Draw polygon with depth offset
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(0.0f, units);
        drawQuad(testProgram, "a_position", 0.0);

        // Get depth value with offset
        const GLfloat depthValueOffset = readDepthValue();

        // Draw reference polygon
        glDisable(GL_POLYGON_OFFSET_FILL);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        drawQuad(testProgram, "a_position", 0.0);

        // Draw polygon with depth offset
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffsetClampEXT(0.0f, units, clamp);
        drawQuad(testProgram, "a_position", 0.0);

        // Get depth value with clamped offset
        const GLfloat depthValueOffsetClamp = readDepthValue();

        // Check depth values
        {
            if (clamp == 0.0f || isinf(clamp))
            {
                ASSERT_NE(units, 0.0f);

                // Ensure that offset works
                if (units > 0.0f)
                {
                    EXPECT_LT(depthValue, depthValueOffset);
                    EXPECT_LT(depthValue, depthValueOffsetClamp);
                }
                else if (units < 0.0f)
                {
                    EXPECT_GT(depthValue, depthValueOffset);
                    EXPECT_GT(depthValue, depthValueOffsetClamp);
                }

                // Clamping must have no effect
                EXPECT_EQ(depthValueOffset, depthValueOffsetClamp);
            }
            else if (clamp < 0.0f)
            {
                ASSERT_LT(units, 0);

                // Negative clamp value sets effective offset to max(offset, clamp)
                EXPECT_GT(depthValue, depthValueOffset);
                EXPECT_GT(depthValue, depthValueOffsetClamp);
                EXPECT_LT(depthValueOffset, depthValueOffsetClamp);
            }
            else if (clamp > 0.0f)
            {
                ASSERT_GT(units, 0);

                // Positive clamp value sets effective offset to min(offset, clamp)
                EXPECT_LT(depthValue, depthValueOffset);
                EXPECT_LT(depthValue, depthValueOffsetClamp);
                EXPECT_GT(depthValueOffset, depthValueOffsetClamp);
            }
        }
    }
}

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(PolygonOffsetClampTest);

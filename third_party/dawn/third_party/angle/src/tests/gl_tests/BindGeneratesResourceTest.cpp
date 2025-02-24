//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BindGeneratesResourceTest.cpp : Tests of the GL_CHROMIUM_bind_generates_resource extension.

#include "test_utils/ANGLETest.h"

namespace angle
{

class BindGeneratesResourceTest : public ANGLETest<>
{
  protected:
    BindGeneratesResourceTest() { setBindGeneratesResource(false); }
};

// Context creation would fail if EGL_CHROMIUM_create_context_bind_generates_resource was not
// available so the GL extension should always be present
TEST_P(BindGeneratesResourceTest, ExtensionStringExposed)
{
    EXPECT_TRUE(IsGLExtensionEnabled("GL_CHROMIUM_bind_generates_resource"));
}

// Verify that GL_BIND_GENERATES_RESOURCE_CHROMIUM can be queried but not changed
TEST_P(BindGeneratesResourceTest, QueryValidation)
{
    GLint intValue = 2;
    glGetIntegerv(GL_BIND_GENERATES_RESOURCE_CHROMIUM, &intValue);
    EXPECT_GL_NO_ERROR();
    EXPECT_GL_FALSE(intValue);

    float floatValue = 2.0f;
    glGetFloatv(GL_BIND_GENERATES_RESOURCE_CHROMIUM, &floatValue);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(floatValue, 0.0f);

    GLboolean boolValue = GL_TRUE;
    glGetBooleanv(GL_BIND_GENERATES_RESOURCE_CHROMIUM, &boolValue);
    EXPECT_GL_NO_ERROR();
    EXPECT_GL_FALSE(boolValue);

    boolValue = glIsEnabled(GL_BIND_GENERATES_RESOURCE_CHROMIUM);
    EXPECT_GL_NO_ERROR();
    EXPECT_GL_FALSE(boolValue);

    glEnable(GL_BIND_GENERATES_RESOURCE_CHROMIUM);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glDisable(GL_BIND_GENERATES_RESOURCE_CHROMIUM);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
}

// Test that buffers cannot be generated on bind
TEST_P(BindGeneratesResourceTest, Buffers)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 2);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    EXPECT_GL_NO_ERROR();

    if (getClientMajorVersion() >= 3)
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, 3);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);
        EXPECT_GL_NO_ERROR();

        glBindBufferRange(GL_UNIFORM_BUFFER, 0, 4, 1, 2);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);

        glBindBufferRange(GL_UNIFORM_BUFFER, 0, 0, 1, 2);
        EXPECT_GL_NO_ERROR();
    }
}

// Test that textures cannot be generated on bind
TEST_P(BindGeneratesResourceTest, Textures)
{
    glBindTexture(GL_TEXTURE_2D, 2);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glBindTexture(GL_TEXTURE_2D, 0);
    EXPECT_GL_NO_ERROR();
}

// Test that framebuffers cannot be generated on bind
TEST_P(BindGeneratesResourceTest, Framebuffers)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 2);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    EXPECT_GL_NO_ERROR();
}

// Test that renderbuffer cannot be generated on bind
TEST_P(BindGeneratesResourceTest, Renderbuffers)
{
    glBindRenderbuffer(GL_RENDERBUFFER, 2);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    EXPECT_GL_NO_ERROR();
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(BindGeneratesResourceTest);

}  // namespace angle

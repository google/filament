//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TextureExternalUpdateTest.cpp: Tests for GL_ANGLE_texture_external_update

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

namespace angle
{

class TextureExternalUpdateTest : public ANGLETest<>
{};

// Test basic usage of glInvalidateTextureANGLE
TEST_P(TextureExternalUpdateTest, Invalidate)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_texture_external_update"));

    glInvalidateTextureANGLE(GL_TEXTURE_2D);
    EXPECT_GL_NO_ERROR();

    glInvalidateTextureANGLE(GL_TEXTURE_EXTERNAL_OES);
    if (EnsureGLExtensionEnabled("GL_OES_EGL_image_external"))
    {
        EXPECT_GL_NO_ERROR();
    }
    else
    {
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }
}

// Test basic usage of glTexImage2DExternalANGLE
TEST_P(TextureExternalUpdateTest, TexImage2DExternal)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_texture_external_update"));

    GLenum bindingPoint = GL_TEXTURE_2D;
    if (EnsureGLExtensionEnabled("GL_OES_EGL_image_external"))
    {
        // If external textures are available, try calling glTexImage2DExternalANGLE on them instead
        // because it would be disallowed for regular glTexImage2D calls.
        bindingPoint = GL_TEXTURE_EXTERNAL_OES;
    }

    GLTexture texture;
    glBindTexture(bindingPoint, texture);

    glTexImage2DExternalANGLE(bindingPoint, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE);
    EXPECT_GL_NO_ERROR();

    // No data to verify because the texture has not actually been modified externally
}

// Test the native ID query
TEST_P(TextureExternalUpdateTest, NativeID)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_texture_external_update"));

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);

    GLint nativeId = 0;
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_NATIVE_ID_ANGLE, &nativeId);
    EXPECT_GL_NO_ERROR();
    EXPECT_NE(0, nativeId);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(TextureExternalUpdateTest);

}  // namespace angle

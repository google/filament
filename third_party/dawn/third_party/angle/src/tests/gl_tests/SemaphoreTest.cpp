//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SemaphoreTest.cpp : Tests of the GL_EXT_semaphore extension.

#include "test_utils/ANGLETest.h"

#include "test_utils/gl_raii.h"

namespace angle
{

class SemaphoreTest : public ANGLETest<>
{
  protected:
    SemaphoreTest()
    {
        setWindowWidth(1);
        setWindowHeight(1);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

// glIsSemaphoreEXT must identify semaphores.
TEST_P(SemaphoreTest, SemaphoreShouldBeSemaphore)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_semaphore"));

    constexpr GLsizei kSemaphoreCount = 2;
    GLuint semaphores[kSemaphoreCount];
    glGenSemaphoresEXT(kSemaphoreCount, semaphores);

    EXPECT_FALSE(glIsSemaphoreEXT(0));

    for (GLsizei i = 0; i < kSemaphoreCount; ++i)
    {
        EXPECT_TRUE(glIsSemaphoreEXT(semaphores[i]));
    }

    glDeleteSemaphoresEXT(kSemaphoreCount, semaphores);

    EXPECT_GL_NO_ERROR();
}

// glImportSemaphoreFdEXT must fail for handle types that are not file descriptors.
TEST_P(SemaphoreTest, ShouldFailValidationOnImportFdUnsupportedHandleType)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_semaphore_fd"));

    {
        GLSemaphore semaphore;
        int fd = -1;
        glImportSemaphoreFdEXT(semaphore, GL_HANDLE_TYPE_OPAQUE_WIN32_EXT, fd);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }

    EXPECT_GL_NO_ERROR();
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(SemaphoreTest);

}  // namespace angle

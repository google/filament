//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// MemoryObjectTest.cpp : Tests of the GL_EXT_memory_object extension.

#include "test_utils/ANGLETest.h"

#include "test_utils/gl_raii.h"

namespace angle
{

class MemoryObjectTest : public ANGLETest<>
{
  protected:
    MemoryObjectTest()
    {
        setWindowWidth(1);
        setWindowHeight(1);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

// glIsMemoryObjectEXT must identify memory objects.
TEST_P(MemoryObjectTest, MemoryObjectShouldBeMemoryObject)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_memory_object"));

    // http://anglebug.com/42263921
    ANGLE_SKIP_TEST_IF(IsLinux() && IsAMD() && IsDesktopOpenGL());

    constexpr GLsizei kMemoryObjectCount = 2;
    GLuint memoryObjects[kMemoryObjectCount];
    glCreateMemoryObjectsEXT(kMemoryObjectCount, memoryObjects);

    EXPECT_FALSE(glIsMemoryObjectEXT(0));

    for (GLsizei i = 0; i < kMemoryObjectCount; ++i)
    {
        EXPECT_TRUE(glIsMemoryObjectEXT(memoryObjects[i]));
    }

    glDeleteMemoryObjectsEXT(kMemoryObjectCount, memoryObjects);

    EXPECT_GL_NO_ERROR();
}

// glImportMemoryFdEXT must fail for handle types that are not file descriptors.
TEST_P(MemoryObjectTest, ShouldFailValidationOnImportFdUnsupportedHandleType)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_memory_object_fd"));

    // http://anglebug.com/42263921
    ANGLE_SKIP_TEST_IF(IsLinux() && IsAMD() && IsDesktopOpenGL());

    {
        GLMemoryObject memoryObject;
        GLsizei deviceMemorySize = 1;
        int fd                   = -1;
        glImportMemoryFdEXT(memoryObject, deviceMemorySize, GL_HANDLE_TYPE_OPAQUE_WIN32_EXT, fd);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }

    EXPECT_GL_NO_ERROR();
}

// Test memory object queries
TEST_P(MemoryObjectTest, MemoryObjectQueries)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_memory_object"));

    // http://anglebug.com/42263921
    ANGLE_SKIP_TEST_IF(IsLinux() && IsAMD() && IsDesktopOpenGL());

    GLMemoryObject memoryObject;

    // Validate that the initial state of GL_DEDICATED_MEMORY_OBJECT_EXT is GL_FALSE
    {
        GLint dedicatedMemory = 0;
        glGetMemoryObjectParameterivEXT(memoryObject, GL_DEDICATED_MEMORY_OBJECT_EXT,
                                        &dedicatedMemory);
        EXPECT_GL_NO_ERROR();
        EXPECT_GL_FALSE(dedicatedMemory);
    }

    // Change GL_DEDICATED_MEMORY_OBJECT_EXT to GL_TRUE
    {
        GLint dedicatedMemory = GL_TRUE;
        glMemoryObjectParameterivEXT(memoryObject, GL_DEDICATED_MEMORY_OBJECT_EXT,
                                     &dedicatedMemory);
        EXPECT_GL_NO_ERROR();
    }

    // Confirm that GL_DEDICATED_MEMORY_OBJECT_EXT is now TRUE
    {
        GLint dedicatedMemory = 0;
        glGetMemoryObjectParameterivEXT(memoryObject, GL_DEDICATED_MEMORY_OBJECT_EXT,
                                        &dedicatedMemory);
        EXPECT_GL_NO_ERROR();
        EXPECT_GL_TRUE(dedicatedMemory);
    }

    EXPECT_GL_NO_ERROR();
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(MemoryObjectTest);

}  // namespace angle

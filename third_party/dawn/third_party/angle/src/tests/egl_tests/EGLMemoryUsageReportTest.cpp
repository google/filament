//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLMemoryUsageReportTest:
//   Tests pertaining to EGL_ANGLE_memory_usage_report extension.
//

#include <gtest/gtest.h>

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"

using namespace angle;

class EGLMemoryUsageReportTest : public ANGLETest<>
{
  protected:
    bool hasEGLDisplayExtension(const char *extname) const
    {
        return IsEGLDisplayExtensionEnabled(getEGLWindow()->getDisplay(), extname);
    }

    bool hasMemoryUsageReportExtension() const
    {
        return hasEGLDisplayExtension("EGL_ANGLE_memory_usage_report");
    }

    uint64_t getMemoryUsage(EGLDisplay display, EGLContext context)
    {
        GLint parts[2];
        EXPECT_EGL_TRUE(eglQueryContext(display, context, EGL_CONTEXT_MEMORY_USAGE_ANGLE, parts));

        return (static_cast<uint64_t>(parts[0]) & 0xffffffff) |
               ((static_cast<uint64_t>(parts[1]) & 0xffffffff) << 32);
    }
};

// Basic memory usage queries
TEST_P(EGLMemoryUsageReportTest, BasicQuery)
{
    ANGLE_SKIP_TEST_IF(!hasMemoryUsageReportExtension());

    constexpr GLint kTextureDim              = 1024;
    constexpr GLuint kTextureSize            = kTextureDim * kTextureDim * 4;
    constexpr GLuint kBufferSize             = 4096;
    constexpr GLint kRenderbufferDim         = 512;
    constexpr GLuint kRenderbufferSize       = kRenderbufferDim * kRenderbufferDim * 4;
    constexpr GLuint kTotalObjectsMemorySize = kBufferSize + kRenderbufferSize + kTextureSize;

    EGLDisplay display = getEGLWindow()->getDisplay();
    EGLContext context = getEGLWindow()->getContext();

    uint64_t memorySize1 = getMemoryUsage(display, context);
    uint64_t memorySize2;

    {
        GLBuffer buffer;
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, kBufferSize, nullptr, GL_STATIC_DRAW);

        GLTexture texture;
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kTextureDim, kTextureDim);

        GLRenderbuffer rbo;
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kRenderbufferDim, kRenderbufferDim);

        memorySize2 = getMemoryUsage(display, context);

        EXPECT_EQ(memorySize2 - memorySize1, kTotalObjectsMemorySize);
    }

    uint64_t memorySize3 = getMemoryUsage(display, context);
    EXPECT_EQ(memorySize2 - memorySize3, kTotalObjectsMemorySize);
}

// Test that querying memory usage of a context that is not current works.
TEST_P(EGLMemoryUsageReportTest, QueryNonCurrentContext)
{
    ANGLE_SKIP_TEST_IF(!hasMemoryUsageReportExtension());

    constexpr GLint kTextureDim   = 1024;
    constexpr GLuint kTextureSize = kTextureDim * kTextureDim * 4;

    EGLWindow *window  = getEGLWindow();
    EGLDisplay display = window->getDisplay();

    // Create a 2nd context.
    EGLContext context2 = window->createContext(EGL_NO_CONTEXT, nullptr);

    uint64_t context2MemorySize1 = getMemoryUsage(display, context2);

    {
        EXPECT_EGL_TRUE(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, context2));
        GLTexture texture;
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kTextureDim, kTextureDim);

        // Make default context current.
        EXPECT_TRUE(window->makeCurrent());
        EXPECT_NE(eglGetCurrentContext(), context2);

        // Query 2nd context's memory size
        uint64_t context2MemorySize2 = getMemoryUsage(display, context2);

        EXPECT_EQ(context2MemorySize2 - context2MemorySize1, kTextureSize);
    }

    EXPECT_EGL_TRUE(eglDestroyContext(display, context2));
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EGLMemoryUsageReportTest);
ANGLE_INSTANTIATE_TEST_ES3(EGLMemoryUsageReportTest);

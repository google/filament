//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BindGeneratesResourceTest.cpp : Tests of the GL_CHROMIUM_bind_generates_resource extension.

#include "test_utils/ANGLETest.h"

namespace angle
{
class ContextLostTest : public ANGLETest<>
{
  protected:
    ContextLostTest() {}

    void testSetUp() override
    {
        if (IsEGLClientExtensionEnabled("EGL_EXT_create_context_robustness"))
        {
            setContextResetStrategy(EGL_LOSE_CONTEXT_ON_RESET_EXT);
        }
        else
        {
            setContextResetStrategy(EGL_NO_RESET_NOTIFICATION_EXT);
        }
    }
};

// GL_CHROMIUM_lose_context is implemented in the frontend
TEST_P(ContextLostTest, ExtensionStringExposed)
{
    EXPECT_TRUE(EnsureGLExtensionEnabled("GL_CHROMIUM_lose_context"));
}

// Use GL_CHROMIUM_lose_context to lose a context and verify using GL_EXT_robustness
TEST_P(ContextLostTest, BasicUsageEXT)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_CHROMIUM_lose_context"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_robustness") ||
                       !IsEGLClientExtensionEnabled("EGL_EXT_create_context_robustness"));

    glLoseContextCHROMIUM(GL_GUILTY_CONTEXT_RESET, GL_INNOCENT_CONTEXT_RESET);
    EXPECT_GL_NO_ERROR();
    EXPECT_GLENUM_EQ(glGetGraphicsResetStatusEXT(), GL_GUILTY_CONTEXT_RESET);

    // Errors should be continually generated
    for (size_t i = 0; i < 10; i++)
    {
        glBindTexture(GL_TEXTURE_2D, 0);
        EXPECT_GL_ERROR(GL_CONTEXT_LOST);
    }
}

// Use GL_CHROMIUM_lose_context to lose a context and verify using GL_KHR_robustness
TEST_P(ContextLostTest, BasicUsageKHR)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_CHROMIUM_lose_context"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_KHR_robustness") ||
                       !IsEGLClientExtensionEnabled("EGL_EXT_create_context_robustness"));

    glLoseContextCHROMIUM(GL_GUILTY_CONTEXT_RESET, GL_INNOCENT_CONTEXT_RESET);
    EXPECT_GL_NO_ERROR();
    EXPECT_GLENUM_EQ(glGetGraphicsResetStatusKHR(), GL_GUILTY_CONTEXT_RESET);

    // Errors should be continually generated
    for (size_t i = 0; i < 10; i++)
    {
        glBindTexture(GL_TEXTURE_2D, 0);
        EXPECT_GL_ERROR(GL_CONTEXT_LOST);
    }
}

// When context is lost, polling queries such as glGetSynciv with GL_SYNC_STATUS should always
// return GL_SIGNALED
TEST_P(ContextLostTest, PollingQuery)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_CHROMIUM_lose_context"));
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);

    GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    EXPECT_GL_NO_ERROR();

    glLoseContextCHROMIUM(GL_GUILTY_CONTEXT_RESET, GL_INNOCENT_CONTEXT_RESET);
    EXPECT_GL_NO_ERROR();

    GLint syncStatus = 0;
    glGetSynciv(sync, GL_SYNC_STATUS, 1, nullptr, &syncStatus);
    EXPECT_GL_ERROR(GL_CONTEXT_LOST);
    EXPECT_GLENUM_EQ(syncStatus, GL_SIGNALED);

    // Check that the query fails and the result is unmodified for other queries
    GLint syncCondition = 0xBADF00D;
    glGetSynciv(sync, GL_SYNC_CONDITION, 1, nullptr, &syncCondition);
    EXPECT_GL_ERROR(GL_CONTEXT_LOST);
    EXPECT_GLENUM_EQ(syncCondition, 0xBADF00D);
}

// When context is lost, polling queries such as glGetSynciv with GL_SYNC_STATUS should always
// return GL_SIGNALED
TEST_P(ContextLostTest, ParallelCompileReadyQuery)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_CHROMIUM_lose_context"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_KHR_parallel_shader_compile"));

    GLuint vs = CompileShader(GL_VERTEX_SHADER, essl1_shaders::vs::Simple());
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, essl1_shaders::fs::UniformColor());

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    EXPECT_GL_NO_ERROR();

    glLoseContextCHROMIUM(GL_GUILTY_CONTEXT_RESET, GL_INNOCENT_CONTEXT_RESET);
    EXPECT_GL_NO_ERROR();

    GLint shaderCompletionStatus = 0;
    glGetShaderiv(vs, GL_COMPLETION_STATUS_KHR, &shaderCompletionStatus);
    EXPECT_GL_ERROR(GL_CONTEXT_LOST);
    EXPECT_GLENUM_EQ(shaderCompletionStatus, GL_TRUE);

    GLint programCompletionStatus = 0;
    glGetProgramiv(program, GL_COMPLETION_STATUS_KHR, &programCompletionStatus);
    EXPECT_GL_ERROR(GL_CONTEXT_LOST);
    EXPECT_GLENUM_EQ(programCompletionStatus, GL_TRUE);

    // Check that the query fails and the result is unmodified for other queries
    GLint shaderType = 0xBADF00D;
    glGetShaderiv(vs, GL_SHADER_TYPE, &shaderType);
    EXPECT_GL_ERROR(GL_CONTEXT_LOST);
    EXPECT_GLENUM_EQ(shaderType, 0xBADF00D);

    GLint linkStatus = 0xBADF00D;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_GL_ERROR(GL_CONTEXT_LOST);
    EXPECT_GLENUM_EQ(linkStatus, 0xBADF00D);
}

class ContextLostTestES32 : public ContextLostTest
{};

// Use GL_CHROMIUM_lose_context to lose a context and verify using GLES 3.2 function
TEST_P(ContextLostTestES32, BasicUsage)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_CHROMIUM_lose_context"));
    ANGLE_SKIP_TEST_IF(!IsEGLClientExtensionEnabled("EGL_EXT_create_context_robustness"));

    glLoseContextCHROMIUM(GL_GUILTY_CONTEXT_RESET, GL_INNOCENT_CONTEXT_RESET);
    EXPECT_GL_NO_ERROR();
    EXPECT_GLENUM_EQ(glGetGraphicsResetStatus(), GL_GUILTY_CONTEXT_RESET);

    // Errors should be continually generated
    for (size_t i = 0; i < 10; i++)
    {
        glBindTexture(GL_TEXTURE_2D, 0);
        EXPECT_GL_ERROR(GL_CONTEXT_LOST);
    }
}

class ContextLostSkipValidationTest : public ANGLETest<>
{
  protected:
    ContextLostSkipValidationTest() {}

    void testSetUp() override
    {
        if (IsEGLClientExtensionEnabled("EGL_EXT_create_context_robustness"))
        {
            setContextResetStrategy(EGL_LOSE_CONTEXT_ON_RESET_EXT);
            setNoErrorEnabled(true);
        }
        else
        {
            setContextResetStrategy(EGL_NO_RESET_NOTIFICATION_EXT);
        }
    }
};

// Use GL_CHROMIUM_lose_context to lose a context and verify
TEST_P(ContextLostSkipValidationTest, LostNoErrorGetProgram)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_CHROMIUM_lose_context"));

    GLuint program = glCreateProgram();

    glLoseContextCHROMIUM(GL_GUILTY_CONTEXT_RESET, GL_INNOCENT_CONTEXT_RESET);

    GLint val = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &val);  // Should not crash.
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST(ContextLostTest,
                       WithRobustness(ES2_NULL()),
                       WithRobustness(ES2_D3D9()),
                       WithRobustness(ES2_D3D11()),
                       WithRobustness(ES3_D3D11()),
                       WithRobustness(ES2_VULKAN()),
                       WithRobustness(ES3_VULKAN()));

ANGLE_INSTANTIATE_TEST(ContextLostSkipValidationTest,
                       WithRobustness(ES2_NULL()),
                       WithRobustness(ES2_D3D9()),
                       WithRobustness(ES2_D3D11()),
                       WithRobustness(ES3_D3D11()),
                       WithRobustness(ES2_VULKAN()),
                       WithRobustness(ES3_VULKAN()));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ContextLostTestES32);
ANGLE_INSTANTIATE_TEST(ContextLostTestES32, WithRobustness(ES32_VULKAN()));

}  // namespace angle

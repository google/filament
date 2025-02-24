//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TimerQueriesTest.cpp
//   Various tests for EXT_disjoint_timer_query functionality and validation
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"
#include "util/random_utils.h"
#include "util/test_utils.h"

using namespace angle;

class TimerQueriesTest : public ANGLETest<>
{
  protected:
    TimerQueriesTest() : mProgram(0), mProgramCostly(0)
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    void testSetUp() override
    {
        constexpr char kCostlyVS[] =
            "attribute highp vec4 position; varying highp vec4 testPos; void main(void)\n"
            "{\n"
            "    testPos     = position;\n"
            "    gl_Position = position;\n"
            "}\n";

        constexpr char kCostlyFS[] =
            "precision highp float; varying highp vec4 testPos; void main(void)\n"
            "{\n"
            "    vec4 test = testPos;\n"
            "    for (int i = 0; i < 500; i++)\n"
            "    {\n"
            "        test = sqrt(test);\n"
            "    }\n"
            "    gl_FragColor = test;\n"
            "}\n";

        mProgram = CompileProgram(essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
        ASSERT_NE(0u, mProgram) << "shader compilation failed.";

        mProgramCostly = CompileProgram(kCostlyVS, kCostlyFS);
        ASSERT_NE(0u, mProgramCostly) << "shader compilation failed.";
    }

    void testTearDown() override
    {
        glDeleteProgram(mProgram);
        glDeleteProgram(mProgramCostly);
    }

    GLuint mProgram;
    GLuint mProgramCostly;
};

// Test that all proc addresses are loadable
TEST_P(TimerQueriesTest, ProcAddresses)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_disjoint_timer_query"));

    ASSERT_NE(nullptr, eglGetProcAddress("glGenQueriesEXT"));
    ASSERT_NE(nullptr, eglGetProcAddress("glDeleteQueriesEXT"));
    ASSERT_NE(nullptr, eglGetProcAddress("glIsQueryEXT"));
    ASSERT_NE(nullptr, eglGetProcAddress("glBeginQueryEXT"));
    ASSERT_NE(nullptr, eglGetProcAddress("glEndQueryEXT"));
    ASSERT_NE(nullptr, eglGetProcAddress("glQueryCounterEXT"));
    ASSERT_NE(nullptr, eglGetProcAddress("glGetQueryivEXT"));
    ASSERT_NE(nullptr, eglGetProcAddress("glGetQueryObjectivEXT"));
    ASSERT_NE(nullptr, eglGetProcAddress("glGetQueryObjectuivEXT"));
    ASSERT_NE(nullptr, eglGetProcAddress("glGetQueryObjecti64vEXT"));
    ASSERT_NE(nullptr, eglGetProcAddress("glGetQueryObjectui64vEXT"));
}

// Tests the time elapsed query
TEST_P(TimerQueriesTest, TimeElapsed)
{
    // TODO(anglebug.com/40096747): Failing on ARM-based Apple DTKs.
    ANGLE_SKIP_TEST_IF(IsMac() && IsARM64() && IsDesktopOpenGL());

    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_disjoint_timer_query"));

    // http://anglebug.com/42263715
    ANGLE_SKIP_TEST_IF(IsMac() && IsOpenGL());

    GLint queryTimeElapsedBits = 0;
    glGetQueryivEXT(GL_TIME_ELAPSED_EXT, GL_QUERY_COUNTER_BITS_EXT, &queryTimeElapsedBits);
    ASSERT_GL_NO_ERROR();

    std::cout << "Time elapsed counter bits: " << queryTimeElapsedBits << std::endl;

    // Skip test if the number of bits is 0
    ANGLE_SKIP_TEST_IF(!queryTimeElapsedBits);

    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    GLuint query1 = 0;
    GLuint query2 = 0;
    glGenQueriesEXT(1, &query1);
    glGenQueriesEXT(1, &query2);

    // Test time elapsed for a single quad
    glBeginQueryEXT(GL_TIME_ELAPSED_EXT, query1);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.8f);
    glEndQueryEXT(GL_TIME_ELAPSED_EXT);
    ASSERT_GL_NO_ERROR();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Test time elapsed for costly quad
    glBeginQueryEXT(GL_TIME_ELAPSED_EXT, query2);
    drawQuad(mProgramCostly, "position", 0.8f);
    glEndQueryEXT(GL_TIME_ELAPSED_EXT);
    ASSERT_GL_NO_ERROR();

    swapBuffers();

    int timeout  = 200000;
    GLuint ready = GL_FALSE;
    while (ready == GL_FALSE && timeout > 0)
    {
        angle::Sleep(0);
        glGetQueryObjectuivEXT(query1, GL_QUERY_RESULT_AVAILABLE_EXT, &ready);
        timeout--;
    }
    ready = GL_FALSE;
    while (ready == GL_FALSE && timeout > 0)
    {
        angle::Sleep(0);
        glGetQueryObjectuivEXT(query2, GL_QUERY_RESULT_AVAILABLE_EXT, &ready);
        timeout--;
    }
    ASSERT_LT(0, timeout) << "Query result available timed out" << std::endl;

    GLuint64 result1 = 0;
    GLuint64 result2 = 0;
    glGetQueryObjectui64vEXT(query1, GL_QUERY_RESULT_EXT, &result1);
    glGetQueryObjectui64vEXT(query2, GL_QUERY_RESULT_EXT, &result2);
    ASSERT_GL_NO_ERROR();

    glDeleteQueriesEXT(1, &query1);
    glDeleteQueriesEXT(1, &query2);
    ASSERT_GL_NO_ERROR();

    std::cout << "Elapsed time: " << result1 << " cheap quad" << std::endl;
    std::cout << "Elapsed time: " << result2 << " costly quad" << std::endl;

    // The time elapsed should be nonzero
    EXPECT_LT(0ul, result1);
    EXPECT_LT(0ul, result2);

    // The time elapsed should be less than a second.  Not an actual
    // requirement, but longer than a second to draw something basic hints at
    // an issue with the queries themselves.
    EXPECT_LT(result1, 1000000000ul);
    EXPECT_LT(result2, 1000000000ul);

    // TODO(geofflang): Re-enable this check when it is non-flaky
    // The costly quad should take longer than the cheap quad
    // EXPECT_LT(result1, result2);
}

// Tests time elapsed for a non draw call (texture upload)
TEST_P(TimerQueriesTest, TimeElapsedTextureTest)
{
    // OSX drivers don't seem to properly time non-draw calls so we skip the test on Mac
    ANGLE_SKIP_TEST_IF(IsMac() && IsOpenGL());

    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_disjoint_timer_query"));

    GLint queryTimeElapsedBits = 0;
    glGetQueryivEXT(GL_TIME_ELAPSED_EXT, GL_QUERY_COUNTER_BITS_EXT, &queryTimeElapsedBits);
    ASSERT_GL_NO_ERROR();

    std::cout << "Time elapsed counter bits: " << queryTimeElapsedBits << std::endl;

    // Skip test if the number of bits is 0
    ANGLE_SKIP_TEST_IF(!queryTimeElapsedBits);

    std::vector<GLColor> texData{GLColor::black, GLColor::white, GLColor::white, GLColor::black};

    // Query and texture initialization
    GLuint texture;
    GLuint query = 0;
    glGenQueriesEXT(1, &query);
    glGenTextures(1, &texture);

    // Upload a texture inside the query
    glBeginQueryEXT(GL_TIME_ELAPSED_EXT, query);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, texData.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    glFinish();
    glEndQueryEXT(GL_TIME_ELAPSED_EXT);
    ASSERT_GL_NO_ERROR();

    int timeout  = 200000;
    GLuint ready = GL_FALSE;
    while (ready == GL_FALSE && timeout > 0)
    {
        angle::Sleep(0);
        glGetQueryObjectuivEXT(query, GL_QUERY_RESULT_AVAILABLE_EXT, &ready);
        timeout--;
    }
    ASSERT_LT(0, timeout) << "Query result available timed out" << std::endl;

    GLuint64 result = 0;
    glGetQueryObjectui64vEXT(query, GL_QUERY_RESULT_EXT, &result);
    ASSERT_GL_NO_ERROR();

    glDeleteTextures(1, &texture);
    glDeleteQueriesEXT(1, &query);

    std::cout << "Elapsed time: " << result << std::endl;
    EXPECT_LT(0ul, result);

    // an issue with the queries themselves.
    EXPECT_LT(result, 1000000000ul);
}

// Tests validation of query functions with respect to elapsed time query
TEST_P(TimerQueriesTest, TimeElapsedValidationTest)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_disjoint_timer_query"));

    GLint queryTimeElapsedBits = 0;
    glGetQueryivEXT(GL_TIME_ELAPSED_EXT, GL_QUERY_COUNTER_BITS_EXT, &queryTimeElapsedBits);
    ASSERT_GL_NO_ERROR();

    std::cout << "Time elapsed counter bits: " << queryTimeElapsedBits << std::endl;

    // Skip test if the number of bits is 0
    ANGLE_SKIP_TEST_IF(!queryTimeElapsedBits);

    GLuint query = 0;
    glGenQueriesEXT(-1, &query);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    glGenQueriesEXT(1, &query);
    EXPECT_GL_NO_ERROR();

    glBeginQueryEXT(GL_TIMESTAMP_EXT, query);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glBeginQueryEXT(GL_TIME_ELAPSED_EXT, 0);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glEndQueryEXT(GL_TIME_ELAPSED_EXT);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glBeginQueryEXT(GL_TIME_ELAPSED_EXT, query);
    EXPECT_GL_NO_ERROR();

    glBeginQueryEXT(GL_TIME_ELAPSED_EXT, query);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glEndQueryEXT(GL_TIME_ELAPSED_EXT);
    EXPECT_GL_NO_ERROR();

    glEndQueryEXT(GL_TIME_ELAPSED_EXT);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Tests timer queries operating under multiple EGL contexts with mid-query switching
TEST_P(TimerQueriesTest, TimeElapsedMulticontextTest)
{
    // TODO(jmadill): Figure out why this test is flaky on AMD/OpenGL.
    // http://anglebug.com/42260520
    ANGLE_SKIP_TEST_IF(IsAMD() && IsOpenGL());

    // TODO(anglebug.com/40096747): Failing on ARM-based Apple DTKs.
    ANGLE_SKIP_TEST_IF(IsMac() && IsARM64() && IsDesktopOpenGL());

    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_disjoint_timer_query"));

    // Test skipped because the Vulkan backend doesn't account for (and remove) time spent in other
    // contexts.
    ANGLE_SKIP_TEST_IF(IsVulkan());

    GLint queryTimeElapsedBits = 0;
    glGetQueryivEXT(GL_TIME_ELAPSED_EXT, GL_QUERY_COUNTER_BITS_EXT, &queryTimeElapsedBits);
    ASSERT_GL_NO_ERROR();

    std::cout << "Time elapsed counter bits: " << queryTimeElapsedBits << std::endl;

    // Skip test if the number of bits is 0
    ANGLE_SKIP_TEST_IF(!queryTimeElapsedBits);

    // Without a glClear, the first draw call on GL takes a huge amount of time when run after the
    // D3D test on certain NVIDIA drivers
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    EGLint contextAttributes[] = {
        EGL_CONTEXT_MAJOR_VERSION_KHR,
        GetParam().majorVersion,
        EGL_CONTEXT_MINOR_VERSION_KHR,
        GetParam().minorVersion,
        EGL_NONE,
    };

    EGLWindow *window = getEGLWindow();

    EGLDisplay display = window->getDisplay();
    EGLConfig config   = window->getConfig();
    EGLSurface surface = window->getSurface();

    struct ContextInfo
    {
        EGLContext context;
        GLuint program;
        GLuint query;
        EGLDisplay display;

        ContextInfo() : context(EGL_NO_CONTEXT), program(0), query(0), display(EGL_NO_DISPLAY) {}

        ~ContextInfo()
        {
            if (context != EGL_NO_CONTEXT && display != EGL_NO_DISPLAY)
            {
                eglDestroyContext(display, context);
            }
        }
    };
    ContextInfo contexts[2];

    constexpr char kCostlyVS[] =
        "attribute highp vec4 position; varying highp vec4 testPos; void main(void)\n"
        "{\n"
        "    testPos     = position;\n"
        "    gl_Position = position;\n"
        "}\n";

    constexpr char kCostlyFS[] =
        "precision highp float; varying highp vec4 testPos; void main(void)\n"
        "{\n"
        "    vec4 test = testPos;\n"
        "    for (int i = 0; i < 500; i++)\n"
        "    {\n"
        "        test = sqrt(test);\n"
        "    }\n"
        "    gl_FragColor = test;\n"
        "}\n";

    // Setup the first context with a cheap shader
    contexts[0].context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttributes);
    contexts[0].display = display;
    ASSERT_NE(contexts[0].context, EGL_NO_CONTEXT);
    eglMakeCurrent(display, surface, surface, contexts[0].context);
    contexts[0].program = CompileProgram(essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    glGenQueriesEXT(1, &contexts[0].query);
    ASSERT_GL_NO_ERROR();

    // Setup the second context with an expensive shader
    contexts[1].context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttributes);
    contexts[1].display = display;
    ASSERT_NE(contexts[1].context, EGL_NO_CONTEXT);
    eglMakeCurrent(display, surface, surface, contexts[1].context);
    contexts[1].program = CompileProgram(kCostlyVS, kCostlyFS);
    glGenQueriesEXT(1, &contexts[1].query);
    ASSERT_GL_NO_ERROR();

    // Start the query and draw a quad on the first context without ending the query
    eglMakeCurrent(display, surface, surface, contexts[0].context);
    glBeginQueryEXT(GL_TIME_ELAPSED_EXT, contexts[0].query);
    drawQuad(contexts[0].program, essl1_shaders::PositionAttrib(), 0.8f);
    ASSERT_GL_NO_ERROR();

    // Switch contexts, draw the expensive quad and end its query
    eglMakeCurrent(display, surface, surface, contexts[1].context);
    glBeginQueryEXT(GL_TIME_ELAPSED_EXT, contexts[1].query);
    drawQuad(contexts[1].program, "position", 0.8f);
    glEndQueryEXT(GL_TIME_ELAPSED_EXT);
    ASSERT_GL_NO_ERROR();

    // Go back to the first context, end its query, and get the result
    eglMakeCurrent(display, surface, surface, contexts[0].context);
    glEndQueryEXT(GL_TIME_ELAPSED_EXT);

    GLuint64 result1 = 0;
    GLuint64 result2 = 0;
    glGetQueryObjectui64vEXT(contexts[0].query, GL_QUERY_RESULT_EXT, &result1);
    glDeleteQueriesEXT(1, &contexts[0].query);
    glDeleteProgram(contexts[0].program);
    ASSERT_GL_NO_ERROR();

    // Get the 2nd context's results
    eglMakeCurrent(display, surface, surface, contexts[1].context);
    glGetQueryObjectui64vEXT(contexts[1].query, GL_QUERY_RESULT_EXT, &result2);
    glDeleteQueriesEXT(1, &contexts[1].query);
    glDeleteProgram(contexts[1].program);
    ASSERT_GL_NO_ERROR();

    // Switch back to main context
    eglMakeCurrent(display, surface, surface, window->getContext());

    // Compare the results. The cheap quad should be smaller than the expensive one if
    // virtualization is working correctly
    std::cout << "Elapsed time: " << result1 << " cheap quad" << std::endl;
    std::cout << "Elapsed time: " << result2 << " costly quad" << std::endl;
    EXPECT_LT(0ul, result1);
    EXPECT_LT(0ul, result2);
    EXPECT_LT(result1, 1000000000ul);
    EXPECT_LT(result2, 1000000000ul);

    // This check can never really be non-flaky. http://anglebug.com/42263737
    // EXPECT_LT(result1, result2);
}

// Tests GPU timestamp functionality
TEST_P(TimerQueriesTest, Timestamp)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_disjoint_timer_query"));

    GLint queryTimestampBits = 0;
    glGetQueryivEXT(GL_TIMESTAMP_EXT, GL_QUERY_COUNTER_BITS_EXT, &queryTimestampBits);
    ASSERT_GL_NO_ERROR();

    std::cout << "Timestamp counter bits: " << queryTimestampBits << std::endl;

    // Macs for some reason return 0 bits so skip the test for now if either are 0
    ANGLE_SKIP_TEST_IF(!queryTimestampBits);

    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    GLuint query1 = 0;
    GLuint query2 = 0;
    glGenQueriesEXT(1, &query1);
    glGenQueriesEXT(1, &query2);
    glQueryCounterEXT(query1, GL_TIMESTAMP_EXT);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.8f);
    glQueryCounterEXT(query2, GL_TIMESTAMP_EXT);

    ASSERT_GL_NO_ERROR();

    swapBuffers();

    int timeout  = 200000;
    GLuint ready = GL_FALSE;
    while (ready == GL_FALSE && timeout > 0)
    {
        angle::Sleep(0);
        glGetQueryObjectuivEXT(query1, GL_QUERY_RESULT_AVAILABLE_EXT, &ready);
        timeout--;
    }
    ready = GL_FALSE;
    while (ready == GL_FALSE && timeout > 0)
    {
        angle::Sleep(0);
        glGetQueryObjectuivEXT(query2, GL_QUERY_RESULT_AVAILABLE_EXT, &ready);
        timeout--;
    }
    ASSERT_LT(0, timeout) << "Query result available timed out" << std::endl;

    GLuint64 result1 = 0;
    GLuint64 result2 = 0;
    glGetQueryObjectui64vEXT(query1, GL_QUERY_RESULT_EXT, &result1);
    glGetQueryObjectui64vEXT(query2, GL_QUERY_RESULT_EXT, &result2);

    ASSERT_GL_NO_ERROR();

    glDeleteQueriesEXT(1, &query1);
    glDeleteQueriesEXT(1, &query2);

    std::cout << "Timestamps: " << result1 << " " << result2 << std::endl;
    EXPECT_LT(0ul, result1);
    EXPECT_LT(0ul, result2);
    EXPECT_LT(result1, result2);
}

void getQueryResult(GLuint queryObjectName, GLuint64 *result)
{
    GLuint queryResult = GL_FALSE;
    while (queryResult != GL_TRUE)
    {
        glGetQueryObjectuivEXT(queryObjectName, GL_QUERY_RESULT_AVAILABLE, &queryResult);
        ASSERT_GL_NO_ERROR();
        angle::Sleep(50);
    }
    glGetQueryObjectui64vEXT(queryObjectName, GL_QUERY_RESULT_EXT, result);
}

class TimerstampQueriesTest : public TimerQueriesTest
{};

// Tests getting timestamps via glGetQueryObjectui64vEXT
TEST_P(TimerstampQueriesTest, TimestampBasic)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_disjoint_timer_query"));
    ANGLE_SKIP_TEST_IF(!IsD3D11() || !IsWindows());
    GLint queryTimestampBits = 0;
    glGetQueryivEXT(GL_TIMESTAMP_EXT, GL_QUERY_COUNTER_BITS_EXT, &queryTimestampBits);
    ASSERT_GL_NO_ERROR();
    std::cout << "Timestamp counter bits: " << queryTimestampBits << std::endl;

    GLQuery queryObject1, queryObject2;
    glQueryCounterEXT(queryObject1, GL_TIMESTAMP_EXT);
    ASSERT_GL_NO_ERROR();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    drawQuad(mProgramCostly, "position", 0.8f);
    glQueryCounterEXT(queryObject2, GL_TIMESTAMP_EXT);
    ASSERT_GL_NO_ERROR();

    GLuint64 result1 = 0;
    getQueryResult(queryObject1, &result1);

    GLuint64 result2 = 0;
    getQueryResult(queryObject2, &result2);
    std::cout << "Timestamps: " << result1 << " " << result2 << std::endl;
    if (queryTimestampBits != 0)
    {
        ASSERT_TRUE(result1 != 0 && result2 > result1);
    }
}

class TimerQueriesTestES3 : public TimerQueriesTest
{};

// Tests getting timestamps via glGetInteger64v
TEST_P(TimerQueriesTestES3, TimestampGetInteger64)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_disjoint_timer_query"));
    // http://anglebug.com/40096654
    ANGLE_SKIP_TEST_IF(IsAndroid());

    GLint queryTimestampBits = 0;
    glGetQueryivEXT(GL_TIMESTAMP_EXT, GL_QUERY_COUNTER_BITS_EXT, &queryTimestampBits);
    ASSERT_GL_NO_ERROR();

    std::cout << "Timestamp counter bits: " << queryTimestampBits << std::endl;

    ANGLE_SKIP_TEST_IF(!queryTimestampBits);

    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    GLint64 result1 = 0;
    GLint64 result2 = 0;
    glGetInteger64v(GL_TIMESTAMP_EXT, &result1);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.8f);
    glGetInteger64v(GL_TIMESTAMP_EXT, &result2);
    ASSERT_GL_NO_ERROR();
    std::cout << "Timestamps (getInteger64v): " << result1 << " " << result2 << std::endl;
    EXPECT_LT(0l, result1);
    EXPECT_LT(0l, result2);
    EXPECT_LT(result1, result2);
}

ANGLE_INSTANTIATE_TEST_ES2_AND(TimerstampQueriesTest,
                               ES3_D3D11().disable(Feature::EnableTimestampQueries),
                               ES3_D3D11().enable(Feature::EnableTimestampQueries));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(TimestampQueriesTest);

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(TimerQueriesTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(TimerQueriesTestES3);
ANGLE_INSTANTIATE_TEST_ES3(TimerQueriesTestES3);

//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// EGLRobustnessTest.cpp: tests for EGL_EXT_create_context_robustness
//
// Tests causing GPU resets are disabled, use the following args to run them:
// --gtest_also_run_disabled_tests --gtest_filter=EGLRobustnessTest\*

#include <gtest/gtest.h>

#include "common/debug.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/OSWindow.h"

using namespace angle;

class EGLRobustnessTest : public ANGLETest<>
{
  public:
    enum class eglContextOpenglRobustAccess : bool
    {
        enable  = true,
        disable = false,
    };

    void testSetUp() override
    {
        mOSWindow = OSWindow::New();
        mOSWindow->initialize("EGLRobustnessTest", 500, 500);
        setWindowVisible(mOSWindow, true);

        const auto &platform = GetParam().eglParameters;

        std::vector<EGLint> displayAttributes;
        displayAttributes.push_back(EGL_PLATFORM_ANGLE_TYPE_ANGLE);
        displayAttributes.push_back(platform.renderer);
        displayAttributes.push_back(EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE);
        displayAttributes.push_back(platform.majorVersion);
        displayAttributes.push_back(EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE);
        displayAttributes.push_back(platform.minorVersion);

        if (platform.deviceType != EGL_DONT_CARE)
        {
            displayAttributes.push_back(EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE);
            displayAttributes.push_back(platform.deviceType);
        }

        displayAttributes.push_back(EGL_NONE);

        mDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE,
                                            reinterpret_cast<void *>(mOSWindow->getNativeDisplay()),
                                            &displayAttributes[0]);
        ASSERT_NE(EGL_NO_DISPLAY, mDisplay);

        ASSERT_TRUE(eglInitialize(mDisplay, nullptr, nullptr) == EGL_TRUE);

        int nConfigs = 0;
        ASSERT_TRUE(eglGetConfigs(mDisplay, nullptr, 0, &nConfigs) == EGL_TRUE);
        ASSERT_LE(1, nConfigs);

        std::vector<EGLConfig> allConfigs(nConfigs);
        int nReturnedConfigs = 0;
        ASSERT_TRUE(eglGetConfigs(mDisplay, allConfigs.data(), nConfigs, &nReturnedConfigs) ==
                    EGL_TRUE);
        ASSERT_EQ(nConfigs, nReturnedConfigs);

        for (const EGLConfig &config : allConfigs)
        {
            EGLint surfaceType;
            eglGetConfigAttrib(mDisplay, config, EGL_SURFACE_TYPE, &surfaceType);

            if ((surfaceType & EGL_WINDOW_BIT) != 0)
            {
                mConfig      = config;
                mInitialized = true;
                break;
            }
        }

        if (mInitialized)
        {
            mWindow =
                eglCreateWindowSurface(mDisplay, mConfig, mOSWindow->getNativeWindow(), nullptr);
            ASSERT_EGL_SUCCESS();
        }
    }

    void testTearDown() override
    {
        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroySurface(mDisplay, mWindow);
        destroyContext();
        eglTerminate(mDisplay);
        EXPECT_EGL_SUCCESS();

        OSWindow::Delete(&mOSWindow);
    }

    void createContext(EGLint resetStrategy)
    {
        std::vector<EGLint> contextAttribs = {
            EGL_CONTEXT_CLIENT_VERSION,
            2,
        };

        if (IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_create_context_robustness"))
        {
            contextAttribs.push_back(EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_EXT);
            contextAttribs.push_back(resetStrategy);
        }
        else
        {
            ASSERT_EQ(EGL_NO_RESET_NOTIFICATION_EXT, resetStrategy);
        }
        contextAttribs.push_back(EGL_NONE);
        mContext = eglCreateContext(mDisplay, mConfig, EGL_NO_CONTEXT, contextAttribs.data());
        ASSERT_NE(EGL_NO_CONTEXT, mContext);

        eglMakeCurrent(mDisplay, mWindow, mWindow, mContext);
        ASSERT_EGL_SUCCESS();

        const char *extensionString = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));
        ASSERT_NE(nullptr, strstr(extensionString, "GL_ANGLE_instanced_arrays"));
    }

    void createClientVersion3NonRobustContext(EGLint resetStrategy)
    {
        std::vector<EGLint> contextAttribs = {
            EGL_CONTEXT_CLIENT_VERSION,           3,         EGL_CONTEXT_MINOR_VERSION_KHR, 0,
            EGL_CONTEXT_OPENGL_ROBUST_ACCESS_EXT, EGL_FALSE,
        };
        if (IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_create_context_robustness"))
        {
            contextAttribs.push_back(EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_EXT);
            contextAttribs.push_back(resetStrategy);
        }
        else
        {
            ASSERT_EQ(EGL_NO_RESET_NOTIFICATION_EXT, resetStrategy);
        }
        contextAttribs.push_back(EGL_NONE);
        mContext = eglCreateContext(mDisplay, mConfig, EGL_NO_CONTEXT, contextAttribs.data());
        ASSERT_NE(EGL_NO_CONTEXT, mContext);

        eglMakeCurrent(mDisplay, mWindow, mWindow, mContext);
        ASSERT_EGL_SUCCESS();

        const char *extensionString = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));
        ASSERT_NE(nullptr, strstr(extensionString, "GL_ANGLE_instanced_arrays"));
    }

    void createRobustContext(EGLint resetStrategy, EGLContext shareContext)
    {
        const EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION,
                                         3,
                                         EGL_CONTEXT_MINOR_VERSION_KHR,
                                         0,
                                         EGL_CONTEXT_OPENGL_ROBUST_ACCESS,
                                         EGL_TRUE,
                                         EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_EXT,
                                         resetStrategy,
                                         EGL_NONE};

        mContext = eglCreateContext(mDisplay, mConfig, shareContext, contextAttribs);
        ASSERT_NE(EGL_NO_CONTEXT, mContext);

        eglMakeCurrent(mDisplay, mWindow, mWindow, mContext);
        ASSERT_EGL_SUCCESS();
    }

    void destroyContext()
    {
        if (mContext != EGL_NO_CONTEXT)
        {
            eglDestroyContext(mDisplay, mContext);
            mContext = EGL_NO_CONTEXT;
        }
    }

    void submitLongRunningTask()
    {
        // Cause a GPU reset by drawing 100,000,000 fullscreen quads
        GLuint program = CompileProgram(
            "attribute vec4 pos;\n"
            "varying vec2 texcoord;\n"
            "void main() {gl_Position = pos; texcoord = (pos.xy * 0.5) + 0.5;}\n",
            "precision mediump float;\n"
            "uniform sampler2D tex;\n"
            "varying vec2 texcoord;\n"
            "void main() {gl_FragColor = gl_FragColor = texture2D(tex, texcoord);}\n");
        ASSERT_NE(0u, program);
        glUseProgram(program);

        GLfloat vertices[] = {
            -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f,
            1.0f,  -1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 0.0f, 1.0f,
        };

        const int kNumQuads = 10000;
        std::vector<GLushort> indices(6 * kNumQuads);

        for (size_t i = 0; i < kNumQuads; i++)
        {
            indices[i * 6 + 0] = 0;
            indices[i * 6 + 1] = 1;
            indices[i * 6 + 2] = 2;
            indices[i * 6 + 3] = 1;
            indices[i * 6 + 4] = 2;
            indices[i * 6 + 5] = 3;
        }

        glBindAttribLocation(program, 0, "pos");
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, vertices);
        glEnableVertexAttribArray(0);

        GLTexture texture;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        GLint textureUniformLocation = glGetUniformLocation(program, "tex");
        glUniform1i(textureUniformLocation, 0);

        glViewport(0, 0, mOSWindow->getWidth(), mOSWindow->getHeight());
        glClearColor(1.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawElementsInstancedANGLE(GL_TRIANGLES, kNumQuads * 6, GL_UNSIGNED_SHORT, indices.data(),
                                     10000);
    }

    const char *getInvalidShaderLocalVariableAccessFS()
    {
        static constexpr char kFS[] = R"(#version 300 es
            layout(location = 0) out highp vec4 fragColor;
            uniform highp int u_index;
            uniform mediump float u_color;

            void main (void)
            {
                highp vec4 color = vec4(0.0f);
                color[u_index] = u_color;
                fragColor = color;
            })";

        return kFS;
    }

    void testInvalidShaderLocalVariableAccess(GLuint program,
                                              const eglContextOpenglRobustAccess robustAccessAttrib)
    {
        glUseProgram(program);
        EXPECT_GL_NO_ERROR();

        GLint indexLocation = glGetUniformLocation(program, "u_index");
        ASSERT_NE(-1, indexLocation);
        GLint colorLocation = glGetUniformLocation(program, "u_color");
        ASSERT_NE(-1, colorLocation);

        // delibrately pass in -1 to u_index to test robustness extension protects write out of
        // bound
        constexpr GLint kInvalidIndex = -1;
        glUniform1i(indexLocation, kInvalidIndex);
        EXPECT_GL_NO_ERROR();

        glUniform1f(colorLocation, 1.0f);

        drawQuad(program, essl31_shaders::PositionAttrib(), 0);
        EXPECT_GL_NO_ERROR();

        // When command buffers are submitted to GPU, if robustness is working properly, the
        // fragment shader will not suffer from write out-of-bounds issue, which resulted in context
        // reset and context loss.
        glFinish();

        GLint errorCode = glGetError();

        if (robustAccessAttrib == eglContextOpenglRobustAccess::enable)
        {
            ASSERT(errorCode == GL_NO_ERROR);
        }
        else
        {
            ASSERT(errorCode == GL_NO_ERROR || errorCode == GL_CONTEXT_LOST);
        }
    }

  protected:
    EGLDisplay mDisplay = EGL_NO_DISPLAY;
    EGLSurface mWindow  = EGL_NO_SURFACE;
    EGLContext mContext = EGL_NO_CONTEXT;
    bool mInitialized   = false;

  private:
    EGLConfig mConfig   = 0;
    OSWindow *mOSWindow = nullptr;
};

class EGLRobustnessTestES3 : public EGLRobustnessTest
{};

class EGLRobustnessTestES31 : public EGLRobustnessTest
{};

// Check glGetGraphicsResetStatusEXT returns GL_NO_ERROR if we did nothing
TEST_P(EGLRobustnessTest, NoErrorByDefault)
{
    ANGLE_SKIP_TEST_IF(!mInitialized);
    ASSERT_TRUE(glGetGraphicsResetStatusEXT() == GL_NO_ERROR);
}

// Checks that the application gets no loss with NO_RESET_NOTIFICATION
TEST_P(EGLRobustnessTest, DISABLED_NoResetNotification)
{
    ANGLE_SKIP_TEST_IF(
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_create_context_robustness"));
    ANGLE_SKIP_TEST_IF(!mInitialized);
    createContext(EGL_NO_RESET_NOTIFICATION_EXT);

    if (!IsWindows())
    {
        std::cout << "Test disabled on non Windows platforms because drivers can't recover. "
                  << "See " << __FILE__ << ":" << __LINE__ << std::endl;
        return;
    }
    std::cout << "Causing a GPU reset, brace for impact." << std::endl;

    submitLongRunningTask();
    glFinish();
    ASSERT_TRUE(glGetGraphicsResetStatusEXT() == GL_NO_ERROR);
}

// Checks that resetting the ANGLE display allows to get rid of the context loss.
// Also checks that the application gets notified of the loss of the display.
// We coalesce both tests to reduce the number of TDRs done on Windows: by default
// having more than 5 TDRs in a minute will cause Windows to disable the GPU until
// the computer is rebooted.
TEST_P(EGLRobustnessTest, DISABLED_ResettingDisplayWorks)
{
    ANGLE_SKIP_TEST_IF(
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_create_context_robustness"));

    // Note that on Windows the OpenGL driver fails hard (popup that closes the application)
    // on a TDR caused by D3D. Don't run D3D tests at the same time as the OpenGL tests.
    ANGLE_SKIP_TEST_IF(IsWindows() && isGLRenderer());
    ANGLE_SKIP_TEST_IF(!mInitialized);

    createContext(EGL_LOSE_CONTEXT_ON_RESET_EXT);

    if (!IsWindows())
    {
        std::cout << "Test disabled on non Windows platforms because drivers can't recover. "
                  << "See " << __FILE__ << ":" << __LINE__ << std::endl;
        return;
    }
    std::cout << "Causing a GPU reset, brace for impact." << std::endl;

    submitLongRunningTask();
    glFinish();
    ASSERT_TRUE(glGetGraphicsResetStatusEXT() != GL_NO_ERROR);

    recreateTestFixture();
    ASSERT_TRUE(glGetGraphicsResetStatusEXT() == GL_NO_ERROR);
}

// Test to reproduce the crash when running
// dEQP-EGL.functional.robustness.reset_context.shaders.out_of_bounds.reset_status.writes.uniform_block.fragment
// on Pixel 6
TEST_P(EGLRobustnessTestES3, ContextResetOnInvalidLocalShaderVariableAccess)
{
    ANGLE_SKIP_TEST_IF(!mInitialized);

    ANGLE_SKIP_TEST_IF(
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_KHR_create_context") ||
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_create_context_robustness"));

    createRobustContext(EGL_LOSE_CONTEXT_ON_RESET, EGL_NO_CONTEXT);

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), getInvalidShaderLocalVariableAccessFS());
    testInvalidShaderLocalVariableAccess(program, eglContextOpenglRobustAccess::enable);
}

// Similar to ContextResetOnInvalidLocalShaderVariableAccess, but the program is created on a
// context that's not robust, but used on one that is.
TEST_P(EGLRobustnessTestES3,
       ContextResetOnInvalidLocalShaderVariableAccess_ShareGroupBeforeProgramCreation)
{
    ANGLE_SKIP_TEST_IF(!mInitialized);

    ANGLE_SKIP_TEST_IF(
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_KHR_create_context") ||
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_create_context_robustness"));

    createContext(EGL_LOSE_CONTEXT_ON_RESET);
    EGLContext shareContext = mContext;
    createRobustContext(EGL_LOSE_CONTEXT_ON_RESET, shareContext);

    eglMakeCurrent(mDisplay, mWindow, mWindow, shareContext);
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), getInvalidShaderLocalVariableAccessFS());
    eglMakeCurrent(mDisplay, mWindow, mWindow, mContext);

    testInvalidShaderLocalVariableAccess(program, eglContextOpenglRobustAccess::enable);

    eglDestroyContext(mDisplay, shareContext);
}

// Similar to ContextResetOnInvalidLocalShaderVariableAccess, but the program is created on a
// context that's not robust, but used on one that is.
TEST_P(EGLRobustnessTestES3,
       ContextResetOnInvalidLocalShaderVariableAccess_ShareGroupAfterProgramCreation)
{
    ANGLE_SKIP_TEST_IF(!mInitialized);

    ANGLE_SKIP_TEST_IF(
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_KHR_create_context") ||
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_create_context_robustness"));

    createContext(EGL_LOSE_CONTEXT_ON_RESET);
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), getInvalidShaderLocalVariableAccessFS());

    EGLContext shareContext = mContext;

    createRobustContext(EGL_LOSE_CONTEXT_ON_RESET, shareContext);
    testInvalidShaderLocalVariableAccess(program, eglContextOpenglRobustAccess::enable);

    eglDestroyContext(mDisplay, shareContext);
}

// Test to ensure shader local variable write out of bound won't crash
// when the context has robustness enabled, and EGL_NO_RESET_NOTIFICATION_EXT
// is set as the value for attribute EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEFY_EXT
TEST_P(EGLRobustnessTestES3, ContextNoResetOnInvalidLocalShaderVariableAccess)
{
    ANGLE_SKIP_TEST_IF(!mInitialized);
    ANGLE_SKIP_TEST_IF(
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_KHR_create_context") ||
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_create_context_robustness"));

    createRobustContext(EGL_NO_RESET_NOTIFICATION_EXT, EGL_NO_CONTEXT);

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), getInvalidShaderLocalVariableAccessFS());
    testInvalidShaderLocalVariableAccess(program, eglContextOpenglRobustAccess::enable);
}

// Similar to ContextNoResetOnInvalidLocalShaderVariableAccess, but the program is created on a
// context that's not robust, but used on one that is.
TEST_P(EGLRobustnessTestES3,
       ContextNoResetOnInvalidLocalShaderVariableAccess_ShareGroupBeforeProgramCreation)
{
    ANGLE_SKIP_TEST_IF(!mInitialized);
    ANGLE_SKIP_TEST_IF(
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_KHR_create_context") ||
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_create_context_robustness"));

    createContext(EGL_NO_RESET_NOTIFICATION_EXT);
    EGLContext shareContext = mContext;
    createRobustContext(EGL_NO_RESET_NOTIFICATION_EXT, shareContext);

    eglMakeCurrent(mDisplay, mWindow, mWindow, shareContext);
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), getInvalidShaderLocalVariableAccessFS());
    eglMakeCurrent(mDisplay, mWindow, mWindow, mContext);

    testInvalidShaderLocalVariableAccess(program, eglContextOpenglRobustAccess::enable);

    eglDestroyContext(mDisplay, shareContext);
}

// Similar to ContextNoResetOnInvalidLocalShaderVariableAccess, but the program is created on a
// context that's not robust, but used on one that is.
TEST_P(EGLRobustnessTestES3,
       ContextNoResetOnInvalidLocalShaderVariableAccess_ShareGroupAfterProgramCreation)
{
    ANGLE_SKIP_TEST_IF(!mInitialized);
    ANGLE_SKIP_TEST_IF(
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_KHR_create_context") ||
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_create_context_robustness"));

    createContext(EGL_NO_RESET_NOTIFICATION_EXT);
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), getInvalidShaderLocalVariableAccessFS());

    EGLContext shareContext = mContext;

    createRobustContext(EGL_NO_RESET_NOTIFICATION_EXT, shareContext);
    testInvalidShaderLocalVariableAccess(program, eglContextOpenglRobustAccess::enable);

    eglDestroyContext(mDisplay, shareContext);
}

// Replicate test
// dEQP-EGL.functional.robustness.reset_context.shaders.out_of_bounds_non_robust.reset_status.writes
// .local_array.fragment
// Test that when writing out-of-bounds in fragment shader:
// 1) After draw command, test receives GL_CONTEXT_LOST error or GL_NO_ERROR.
// 2) eglMakeCurrent(EGL_NO_CONTEXT) on lost context should return EGL_SUCCESS.
TEST_P(EGLRobustnessTestES3, NonRobustContextOnInvalidLocalShaderVariableAccessShouldNotCrash)
{

    ANGLE_SKIP_TEST_IF(!mInitialized);
    ANGLE_SKIP_TEST_IF(
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_KHR_create_context") ||
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_create_context_robustness"));

    createClientVersion3NonRobustContext(EGL_LOSE_CONTEXT_ON_RESET_KHR);

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), getInvalidShaderLocalVariableAccessFS());
    testInvalidShaderLocalVariableAccess(program, eglContextOpenglRobustAccess::disable);

    eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    GLint errorCode = eglGetError();
    ASSERT(errorCode == EGL_SUCCESS);
}

// Test that using a program in a non-robust context, then sharing it with a robust context and
// using it with the same state (but with an OOB access) works.
TEST_P(EGLRobustnessTestES31, NonRobustContextThenOOBInSharedRobustContext)
{
    ANGLE_SKIP_TEST_IF(!mInitialized);
    ANGLE_SKIP_TEST_IF(
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_KHR_create_context") ||
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_create_context_robustness"));

    createContext(EGL_NO_RESET_NOTIFICATION_EXT);

    GLint maxFragmentShaderStorageBlocks = 0;
    glGetIntegerv(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS, &maxFragmentShaderStorageBlocks);
    ANGLE_SKIP_TEST_IF(maxFragmentShaderStorageBlocks == 0);

    constexpr char kFS[] = R"(#version 310 es
layout(location = 0) out highp vec4 fragColor;
layout(std140, binding = 0) buffer Block
{
    mediump vec4 data[];
};
uniform mediump uint index;

void main (void)
{
    fragColor = data[index];
})";

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint indexLocation = glGetUniformLocation(program, "index");
    ASSERT_NE(-1, indexLocation);

    constexpr std::array<float, 4> kBufferData = {1, 0, 0, 1};

    GLBuffer buffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(kBufferData), kBufferData.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer);

    // Use the program once before the robust context is created.
    glUniform1ui(indexLocation, 0);
    drawQuad(program, essl31_shaders::PositionAttrib(), 0);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    ASSERT_GL_NO_ERROR();

    // Create the share context as robust, and draw identically except accessing OOB.
    EGLContext shareContext = mContext;

    createRobustContext(EGL_NO_RESET_NOTIFICATION_EXT, shareContext);

    glClearColor(0, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer);
    glUniform1ui(indexLocation, 1'000'000'000u);
    drawQuad(program, essl31_shaders::PositionAttrib(), 0);
    // Expect 0, 0, 0, 0/1 returned from buffer
    GLColor actualColor = angle::ReadColor(0, 0);
    EXPECT_TRUE(actualColor.A == 0 || actualColor.A == 255);
    actualColor.A = 0;
    EXPECT_EQ(actualColor, GLColor::transparentBlack);
    ASSERT_GL_NO_ERROR();

    eglDestroyContext(mDisplay, shareContext);
}

// Similar to NonRobustContextThenOOBInSharedRobustContext, but access is in vertex shader.
TEST_P(EGLRobustnessTestES31, NonRobustContextThenOOBInSharedRobustContext_VertexShader)
{
    ANGLE_SKIP_TEST_IF(!mInitialized);
    ANGLE_SKIP_TEST_IF(
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_KHR_create_context") ||
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_create_context_robustness"));

    createContext(EGL_NO_RESET_NOTIFICATION_EXT);

    GLint maxVertexShaderStorageBlocks = 0;
    glGetIntegerv(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS, &maxVertexShaderStorageBlocks);
    ANGLE_SKIP_TEST_IF(maxVertexShaderStorageBlocks == 0);

    constexpr char kVS[] = R"(#version 310 es
layout(std140, binding = 0) buffer Block
{
    mediump vec4 data[];
};
uniform mediump uint index;
in vec4 position;
out mediump vec4 color;

void main (void)
{
    color = data[index];
    gl_Position = position;
})";

    constexpr char kFS[] = R"(#version 310 es
layout(location = 0) out highp vec4 fragColor;
in mediump vec4 color;

void main (void)
{
    fragColor = color;
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);

    GLint indexLocation = glGetUniformLocation(program, "index");
    ASSERT_NE(-1, indexLocation);

    constexpr std::array<float, 4> kBufferData = {1, 0, 0, 1};

    GLBuffer buffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(kBufferData), kBufferData.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer);

    // Use the program once before the robust context is created.
    glUniform1ui(indexLocation, 0);
    drawQuad(program, "position", 0);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    ASSERT_GL_NO_ERROR();

    // Create the share context as robust, and draw identically except accessing OOB.
    EGLContext shareContext = mContext;

    createRobustContext(EGL_NO_RESET_NOTIFICATION_EXT, shareContext);

    glClearColor(0, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer);
    glUniform1ui(indexLocation, 1'000'000'000u);
    drawQuad(program, "position", 0);
    // Expect 0, 0, 0, 0/1 returned from buffer
    GLColor actualColor = angle::ReadColor(0, 0);
    EXPECT_TRUE(actualColor.A == 0 || actualColor.A == 255);
    actualColor.A = 0;
    EXPECT_EQ(actualColor, GLColor::transparentBlack);
    ASSERT_GL_NO_ERROR();

    eglDestroyContext(mDisplay, shareContext);
}

// Similar to NonRobustContextThenOOBInSharedRobustContext, but access is in compute shader.
TEST_P(EGLRobustnessTestES31, NonRobustContextThenOOBInSharedRobustContext_ComputeShader)
{
    ANGLE_SKIP_TEST_IF(!mInitialized);
    ANGLE_SKIP_TEST_IF(
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_KHR_create_context") ||
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_create_context_robustness"));

    createContext(EGL_NO_RESET_NOTIFICATION_EXT);

    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;

layout(std140, binding = 0) buffer BlockIn
{
    mediump vec4 dataIn[];
};
layout(std140, binding = 1) buffer BlockOut
{
    mediump vec4 dataOut;
};
uniform mediump uint index;

void main (void)
{
    dataOut = dataIn[index];
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);

    GLint indexLocation = glGetUniformLocation(program, "index");
    ASSERT_NE(-1, indexLocation);

    constexpr std::array<float, 4> kBufferData        = {1, 0, 0, 1};
    constexpr std::array<float, 4> kInvalidBufferData = {0, 0, 1, 1};

    GLBuffer bufferIn;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferIn);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(kBufferData), kBufferData.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufferIn);

    GLBuffer bufferOut;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferOut);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(kInvalidBufferData), kInvalidBufferData.data(),
                 GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufferOut);

    // Use the program once before the robust context is created.
    glUniform1ui(indexLocation, 0);
    glDispatchCompute(1, 1, 1);
    ASSERT_GL_NO_ERROR();

    std::array<float, 4> readbackData = {};

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    void *mappedBuffer =
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(readbackData), GL_MAP_READ_BIT);
    memcpy(readbackData.data(), mappedBuffer, sizeof(readbackData));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    EXPECT_EQ(readbackData, kBufferData);

    // Create the share context as robust, and draw identically except accessing OOB.
    EGLContext shareContext = mContext;

    createRobustContext(EGL_NO_RESET_NOTIFICATION_EXT, shareContext);

    glUseProgram(program);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferIn);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufferIn);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferOut);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufferOut);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(kInvalidBufferData), kInvalidBufferData.data(),
                 GL_STATIC_DRAW);

    glUniform1ui(indexLocation, 1'000'000'000u);
    glDispatchCompute(1, 1, 1);

    // Expect 0, 0, 0, 0/1 returned from bufferIn
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    mappedBuffer =
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(readbackData), GL_MAP_READ_BIT);
    memcpy(readbackData.data(), mappedBuffer, sizeof(readbackData));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    ASSERT_GL_NO_ERROR();

    EXPECT_EQ(readbackData[0], 0);
    EXPECT_EQ(readbackData[1], 0);
    EXPECT_EQ(readbackData[2], 0);
    EXPECT_TRUE(readbackData[3] == 0 || readbackData[3] == 1);

    eglDestroyContext(mDisplay, shareContext);
}

// Test that indirect indices on unsized storage buffer arrays work.  Regression test for the
// ClampIndirectIndices AST transformation.
TEST_P(EGLRobustnessTestES31, IndirectIndexOnUnsizedStorageBufferArray)
{
    ANGLE_SKIP_TEST_IF(!mInitialized);
    ANGLE_SKIP_TEST_IF(
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_KHR_create_context") ||
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_create_context_robustness"));

    createRobustContext(EGL_NO_RESET_NOTIFICATION_EXT, EGL_NO_CONTEXT);

    const char kCS[] = R"(#version 310 es

layout(binding = 0, std430) buffer B {
  uint data[];
} b;

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main() {
  b.data[gl_GlobalInvocationID.x] = gl_GlobalInvocationID.x;
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    EXPECT_GL_NO_ERROR();

    constexpr uint32_t kBufferSize              = 2;
    constexpr uint32_t kBufferData[kBufferSize] = {10, 20};

    GLBuffer buffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(kBufferData), kBufferData, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer);

    // Run the compute shader with a large workload.  Only the first two invocations should write to
    // the buffer, the rest should be dropped out due to robust access.
    glUseProgram(program);
    glDispatchCompute(8192, 1, 1);

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    uint32_t bufferDataOut[kBufferSize] = {};
    const uint32_t *ptr                 = reinterpret_cast<uint32_t *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(kBufferData), GL_MAP_READ_BIT));
    memcpy(bufferDataOut, ptr, sizeof(kBufferData));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    for (uint32_t index = 0; index < kBufferSize; ++index)
    {
        EXPECT_EQ(bufferDataOut[index], index) << " index " << index;
    }
}

// Similar to IndirectIndexOnUnsizedStorageBufferArray, but without a block instance name.
TEST_P(EGLRobustnessTestES31, IndirectIndexOnUnsizedStorageBufferArray_NoBlockInstanceName)
{
    ANGLE_SKIP_TEST_IF(!mInitialized);
    ANGLE_SKIP_TEST_IF(
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_KHR_create_context") ||
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_create_context_robustness"));

    createRobustContext(EGL_NO_RESET_NOTIFICATION_EXT, EGL_NO_CONTEXT);

    const char kCS[] = R"(#version 310 es

layout(binding = 0, std430) buffer B {
  uint data[];
};

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main() {
  data[gl_GlobalInvocationID.x] = gl_GlobalInvocationID.x;
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    EXPECT_GL_NO_ERROR();

    constexpr uint32_t kBufferSize              = 2;
    constexpr uint32_t kBufferData[kBufferSize] = {10, 20};

    GLBuffer buffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(kBufferData), kBufferData, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer);

    // Run the compute shader with a large workload.  Only the first two invocations should write to
    // the buffer, the rest should be dropped out due to robust access.
    glUseProgram(program);
    glDispatchCompute(8192, 1, 1);

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    uint32_t bufferDataOut[kBufferSize] = {};
    const uint32_t *ptr                 = reinterpret_cast<uint32_t *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(kBufferData), GL_MAP_READ_BIT));
    memcpy(bufferDataOut, ptr, sizeof(kBufferData));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    for (uint32_t index = 0; index < kBufferSize; ++index)
    {
        EXPECT_EQ(bufferDataOut[index], index) << " index " << index;
    }
}

// Test context destruction after recovering from a long running task.
TEST_P(EGLRobustnessTest, DISABLED_LongRunningTaskVulkanShutdown)
{
    ANGLE_SKIP_TEST_IF(!mInitialized);

    createContext(EGL_NO_RESET_NOTIFICATION_EXT);
    submitLongRunningTask();
    destroyContext();
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EGLRobustnessTest);
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EGLRobustnessTestES3);
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EGLRobustnessTestES31);
ANGLE_INSTANTIATE_TEST(EGLRobustnessTest,
                       WithNoFixture(ES2_VULKAN()),
                       WithNoFixture(ES2_D3D9()),
                       WithNoFixture(ES2_D3D11()),
                       WithNoFixture(ES2_OPENGL()),
                       WithNoFixture(ES2_OPENGLES()),
                       WithNoFixture(ES2_VULKAN_SWIFTSHADER()));
ANGLE_INSTANTIATE_TEST(EGLRobustnessTestES3,
                       WithNoFixture(ES3_VULKAN()),
                       WithNoFixture(ES3_D3D11()),
                       WithNoFixture(ES3_OPENGL()),
                       WithNoFixture(ES3_OPENGLES()),
                       WithNoFixture(ES3_VULKAN_SWIFTSHADER()));
ANGLE_INSTANTIATE_TEST(EGLRobustnessTestES31,
                       WithNoFixture(ES31_VULKAN()),
                       WithNoFixture(ES31_VULKAN_SWIFTSHADER()));

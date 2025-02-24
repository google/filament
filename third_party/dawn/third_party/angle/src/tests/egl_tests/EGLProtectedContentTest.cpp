//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLProtectedContentTest.cpp:
//   EGL extension EGL_EXT_protected_content
//

#include <gtest/gtest.h>

#include <chrono>
#include <iostream>
#include <thread>
#include "test_utils/ANGLETest.h"
#include "util/EGLWindow.h"
#include "util/OSWindow.h"

constexpr bool kSleepForVisualVerification = false;

using namespace std::chrono_literals;

using namespace angle;

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EGLProtectedContentTest);

class EGLProtectedContentTest : public ANGLETest<>
{
  public:
    EGLProtectedContentTest() : mDisplay(EGL_NO_DISPLAY) {}

    void testSetUp() override
    {
        EGLint dispattrs[] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE, GetParam().getRenderer(), EGL_NONE};
        mDisplay           = eglGetPlatformDisplayEXT(
            EGL_PLATFORM_ANGLE_ANGLE, reinterpret_cast<void *>(EGL_DEFAULT_DISPLAY), dispattrs);
        EXPECT_TRUE(mDisplay != EGL_NO_DISPLAY);
        EXPECT_EGL_TRUE(eglInitialize(mDisplay, nullptr, nullptr));
        mMajorVersion = GetParam().majorVersion;
    }

    void testTearDown() override
    {
        if (mDisplay != EGL_NO_DISPLAY)
        {
            eglTerminate(mDisplay);
            eglReleaseThread();
            mDisplay = EGL_NO_DISPLAY;
        }
        ASSERT_EGL_SUCCESS() << "Error during test TearDown";
    }

    bool chooseConfig(EGLConfig *config, bool mutableRenderBuffer = false)
    {
        EGLint clientVersion = mMajorVersion == 3 ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT;
        EGLint attribs[]     = {
            EGL_RED_SIZE,
            8,
            EGL_GREEN_SIZE,
            8,
            EGL_BLUE_SIZE,
            8,
            EGL_ALPHA_SIZE,
            8,
            EGL_RENDERABLE_TYPE,
            clientVersion,
            EGL_SURFACE_TYPE,
            EGL_WINDOW_BIT |
                (mutableRenderBuffer ? EGL_MUTABLE_RENDER_BUFFER_BIT_KHR : EGL_PBUFFER_BIT),
            EGL_BIND_TO_TEXTURE_RGBA,
            EGL_TRUE,
            EGL_NONE};

        EGLint count = 0;
        bool result  = eglChooseConfig(mDisplay, attribs, config, 1, &count);
        EXPECT_EGL_TRUE(result);
        EXPECT_EGL_TRUE(count > 0);
        return result;
    }

    bool createContext(EGLBoolean isProtected, EGLConfig config, EGLContext *context)
    {
        bool result                 = false;
        EGLint attribsProtected[]   = {EGL_CONTEXT_MAJOR_VERSION, mMajorVersion,
                                       EGL_PROTECTED_CONTENT_EXT, EGL_TRUE, EGL_NONE};
        EGLint attribsUnProtected[] = {EGL_CONTEXT_MAJOR_VERSION, mMajorVersion, EGL_NONE};

        *context = eglCreateContext(mDisplay, config, nullptr,
                                    (isProtected ? attribsProtected : attribsUnProtected));
        result   = (*context != EGL_NO_CONTEXT);
        EXPECT_TRUE(result);
        return result;
    }

    bool createPbufferSurface(EGLBoolean isProtected, EGLConfig config, EGLSurface *surface)
    {
        bool result                 = false;
        EGLint attribsProtected[]   = {EGL_WIDTH,
                                       kWidth,
                                       EGL_HEIGHT,
                                       kHeight,
                                       EGL_TEXTURE_FORMAT,
                                       EGL_TEXTURE_RGBA,
                                       EGL_TEXTURE_TARGET,
                                       EGL_TEXTURE_2D,
                                       EGL_PROTECTED_CONTENT_EXT,
                                       EGL_TRUE,
                                       EGL_NONE};
        EGLint attribsUnProtected[] = {EGL_WIDTH,
                                       kWidth,
                                       EGL_HEIGHT,
                                       kHeight,
                                       EGL_TEXTURE_FORMAT,
                                       EGL_TEXTURE_RGBA,
                                       EGL_TEXTURE_TARGET,
                                       EGL_TEXTURE_2D,
                                       EGL_NONE};

        *surface = eglCreatePbufferSurface(mDisplay, config,
                                           (isProtected ? attribsProtected : attribsUnProtected));
        result   = (*surface != EGL_NO_SURFACE);
        EXPECT_TRUE(result);
        return result;
    }

    bool createWindowSurface(EGLBoolean isProtected,
                             EGLConfig config,
                             EGLNativeWindowType win,
                             EGLSurface *surface)
    {
        bool result                 = false;
        EGLint attribsProtected[]   = {EGL_PROTECTED_CONTENT_EXT, EGL_TRUE, EGL_NONE};
        EGLint attribsUnProtected[] = {EGL_NONE};

        *surface = eglCreateWindowSurface(mDisplay, config, win,
                                          (isProtected ? attribsProtected : attribsUnProtected));
        result   = (*surface != EGL_NO_SURFACE);
        EXPECT_TRUE(result);
        return result;
    }

    bool createImage(EGLBoolean isProtected,
                     EGLContext context,
                     EGLenum target,
                     EGLClientBuffer buffer,
                     EGLImage *image)
    {
        bool result                  = false;
        EGLAttrib attribsProtected[] = {EGL_PROTECTED_CONTENT_EXT, EGL_TRUE, EGL_NONE};

        *image = eglCreateImage(mDisplay, context, target, buffer,
                                (isProtected ? attribsProtected : nullptr));
        EXPECT_EGL_SUCCESS();
        result = (*image != EGL_NO_SURFACE);
        EXPECT_TRUE(result);
        return result;
    }

    bool createTexture(EGLBoolean isProtected, GLuint *textureId)
    {
        bool result    = false;
        GLuint texture = 0;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);
        EXPECT_GL_NO_ERROR();
        if (isProtected)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_PROTECTED_EXT, GL_TRUE);
            // GL_INVALID_OPERATION expected when context is not protected too.
            GLenum error = glGetError();
            if (error == GL_INVALID_OPERATION)
            {
                return false;
            }
        }
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kWidth, kHeight);
        EXPECT_GL_NO_ERROR();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        result = (texture != 0);
        EXPECT_TRUE(result);
        *textureId = texture;
        return result;
    }

    bool createTextureFromImage(EGLImage image, GLuint *textureId)
    {
        bool result    = false;
        GLuint texture = 0;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);
        EXPECT_GL_NO_ERROR();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        result = (texture != 0);
        EXPECT_TRUE(result);
        *textureId = texture;
        return result;
    }

    bool createTextureFromPbuffer(EGLSurface pBuffer, GLuint *textureId)
    {
        bool result    = false;
        GLuint texture = 0;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        EXPECT_GL_NO_ERROR();
        EXPECT_TRUE(texture != 0);
        result = eglBindTexImage(mDisplay, pBuffer, EGL_BACK_BUFFER);
        glViewport(0, 0, kWidth, kHeight);
        *textureId = texture;
        return result;
    }

    bool fillTexture(GLuint textureId, GLColor color)
    {
        GLuint pixels[kWidth * kHeight];
        for (uint32_t i = 0; i < (kWidth * kHeight); i++)
        {
            pixels[i] = *(GLuint *)(color.data());
        }
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth, kHeight, GL_RGBA, GL_UNSIGNED_BYTE,
                        (void *)pixels);
        EXPECT_GL_NO_ERROR();
        return true;
    }

    bool renderTexture(GLuint textureId)
    {
        const char *kVertexShader   = R"(
            precision highp float;
            attribute vec4 position;
            varying vec2 texcoord;

            void main()
            {
                gl_Position = vec4(position.xy, 0.0, 1.0);
                texcoord = (position.xy * 0.5) + 0.5;
            }
        )";
        const char *kFragmentShader = R"(
            precision highp float;
            uniform sampler2D tex;
            varying vec2 texcoord;

            void main()
            {
                gl_FragColor = texture2D(tex, texcoord);
            }
        )";

        GLuint program = CompileProgram(kVertexShader, kFragmentShader);
        glUseProgram(program);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glActiveTexture(GL_TEXTURE0);
        GLint texture2DUniformLocation = glGetUniformLocation(program, "tex");
        glUniform1i(texture2DUniformLocation, 0);
        drawQuad(program, "position", 0.5f);
        glDeleteProgram(program);
        EXPECT_GL_NO_ERROR();
        return true;
    }

    bool createRenderbuffer(GLuint *renderbuffer)
    {
        bool result   = false;
        *renderbuffer = 0;
        glGenRenderbuffers(1, renderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, *renderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, kWidth, kHeight);
        EXPECT_GL_NO_ERROR();
        result = (*renderbuffer != 0);
        EXPECT_TRUE(result);
        return result;
    }

    bool createRenderbufferFromImage(EGLImage image, GLuint *renderbuffer)
    {
        bool result   = false;
        *renderbuffer = 0;
        glGenRenderbuffers(1, renderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, *renderbuffer);
        glEGLImageTargetRenderbufferStorageOES(GL_RENDERBUFFER, image);
        EXPECT_GL_NO_ERROR();
        result = (*renderbuffer != 0);
        EXPECT_TRUE(result);
        return result;
    }

    bool createAndroidClientBuffer(bool useProtected,
                                   bool useRenderbuffer,
                                   bool useTexture,
                                   EGLClientBuffer *clientBuffer)
    {
        bool result = false;
        EGLint nativeBufferUsage =
            0 | (useProtected ? EGL_NATIVE_BUFFER_USAGE_PROTECTED_BIT_ANDROID : 0) |
            (useRenderbuffer ? EGL_NATIVE_BUFFER_USAGE_RENDERBUFFER_BIT_ANDROID : 0) |
            (useTexture ? EGL_NATIVE_BUFFER_USAGE_TEXTURE_BIT_ANDROID : 0);

        EGLint attribs[] = {EGL_WIDTH,
                            kWidth,
                            EGL_HEIGHT,
                            kHeight,
                            EGL_RED_SIZE,
                            8,
                            EGL_GREEN_SIZE,
                            8,
                            EGL_BLUE_SIZE,
                            8,
                            EGL_ALPHA_SIZE,
                            8,
                            EGL_NATIVE_BUFFER_USAGE_ANDROID,
                            nativeBufferUsage,
                            EGL_NONE};

        *clientBuffer = eglCreateNativeClientBufferANDROID(attribs);
        EXPECT_EGL_SUCCESS();
        result = (*clientBuffer != nullptr);
        EXPECT_TRUE(result);
        return result;
    }

    void pbufferTest(bool isProtectedContext, bool isProtectedSurface);
    void windowTest(bool isProtectedContext,
                    bool isProtectedSurface,
                    bool mutableRenderBuffer = false);
    void textureTest(bool isProtectedContext, bool isProtectedTexture);
    void textureFromImageTest(bool isProtectedContext, bool isProtectedTexture);
    void textureFromPbufferTest(bool isProtectedContext, bool isProtectedTexture);
    void textureFromAndroidNativeBufferTest(bool isProtectedContext, bool isProtectedTexture);

    void checkSwapBuffersResult(const std::string color,
                                bool isProtectedContext,
                                bool isProtectedSurface)
    {
        if (kSleepForVisualVerification)
        {
            std::this_thread::sleep_for(1s);
        }
        if (isProtectedContext)
        {
            if (isProtectedSurface)
            {
                std::cout << "Operator should see color: " << color << std::endl;
            }
            else
            {
                std::cout << "Operator should see color: BLACK" << std::endl;
            }
        }
        else
        {
            if (isProtectedSurface)
            {
                std::cout << "Operator should see color: BLACK" << std::endl;
            }
            else
            {
                std::cout << "Operator should see color: " << color << std::endl;
            }
        }
    }

    EGLDisplay mDisplay         = EGL_NO_DISPLAY;
    EGLint mMajorVersion        = 0;
    static const EGLint kWidth  = 16;
    static const EGLint kHeight = 16;
};

void EGLProtectedContentTest::pbufferTest(bool isProtectedContext, bool isProtectedSurface)
{
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_protected_content"));

    EGLConfig config = EGL_NO_CONFIG_KHR;
    EXPECT_TRUE(chooseConfig(&config));
    ANGLE_SKIP_TEST_IF(config == EGL_NO_CONFIG_KHR);

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_TRUE(createContext(isProtectedContext, config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    EGLSurface pBufferSurface = EGL_NO_SURFACE;
    EXPECT_TRUE(createPbufferSurface(isProtectedSurface, config, &pBufferSurface));
    ASSERT_EGL_SUCCESS() << "eglCreatePbufferSurface failed.";

    EXPECT_TRUE(eglMakeCurrent(mDisplay, pBufferSurface, pBufferSurface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glFinish();
    ASSERT_GL_NO_ERROR() << "glFinish failed";
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    EXPECT_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent - uncurrent failed.";

    eglDestroySurface(mDisplay, pBufferSurface);
    pBufferSurface = EGL_NO_SURFACE;

    eglDestroyContext(mDisplay, context);
    context = EGL_NO_CONTEXT;
}

// Unprotected context with Unprotected PbufferSurface
TEST_P(EGLProtectedContentTest, UnprotectedContextWithUnprotectedPbufferSurface)
{
    pbufferTest(false, false);
}

void EGLProtectedContentTest::windowTest(bool isProtectedContext,
                                         bool isProtectedSurface,
                                         bool mutableRenderBuffer)
{
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_protected_content"));
    ANGLE_SKIP_TEST_IF(mutableRenderBuffer &&
                       !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_KHR_mutable_render_buffer"));

    EGLConfig config = EGL_NO_CONFIG_KHR;
    EXPECT_TRUE(chooseConfig(&config, mutableRenderBuffer));
    ANGLE_SKIP_TEST_IF(config == EGL_NO_CONFIG_KHR);

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_TRUE(createContext(isProtectedContext, config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    OSWindow *osWindow = OSWindow::New();
    osWindow->initialize("ProtectedContentTest", kWidth, kHeight);
    EGLSurface windowSurface          = EGL_NO_SURFACE;
    EGLBoolean createWinSurfaceResult = createWindowSurface(
        isProtectedSurface, config, osWindow->getNativeWindow(), &windowSurface);
    EXPECT_TRUE(createWinSurfaceResult);
    ASSERT_EGL_SUCCESS() << "eglCreateWindowSurface failed.";

    EXPECT_TRUE(eglMakeCurrent(mDisplay, windowSurface, windowSurface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    if (mutableRenderBuffer)
    {
        EXPECT_TRUE(
            eglSurfaceAttrib(mDisplay, windowSurface, EGL_RENDER_BUFFER, EGL_SINGLE_BUFFER));
        ASSERT_EGL_SUCCESS() << "eglSurfaceAttrib failed.";
        eglSwapBuffers(mDisplay, windowSurface);
        ASSERT_EGL_SUCCESS() << "eglSwapBuffers failed.";
    }

    // Red
    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR() << "glClear failed";
    eglSwapBuffers(mDisplay, windowSurface);
    ASSERT_EGL_SUCCESS() << "eglSwapBuffers failed.";
    checkSwapBuffersResult("RED", isProtectedContext, isProtectedSurface);

    // Green
    glClearColor(0.0, 1.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR() << "glClear failed";
    eglSwapBuffers(mDisplay, windowSurface);
    ASSERT_EGL_SUCCESS() << "eglSwapBuffers failed.";
    checkSwapBuffersResult("GREEN", isProtectedContext, isProtectedSurface);

    // Blue
    glClearColor(0.0, 0.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR() << "glClear failed";
    eglSwapBuffers(mDisplay, windowSurface);
    ASSERT_EGL_SUCCESS() << "eglSwapBuffers failed.";
    checkSwapBuffersResult("BLUE", isProtectedContext, isProtectedSurface);

    EXPECT_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent - uncurrent failed.";

    eglDestroySurface(mDisplay, windowSurface);
    windowSurface = EGL_NO_SURFACE;
    osWindow->destroy();
    OSWindow::Delete(&osWindow);

    eglDestroyContext(mDisplay, context);
    context = EGL_NO_CONTEXT;
}

// Unprotected context with Unprotected WindowSurface
TEST_P(EGLProtectedContentTest, UnprotectedContextWithUnprotectedWindowSurface)
{
    windowTest(false, false);
}

// Protected context with Protected WindowSurface
TEST_P(EGLProtectedContentTest, ProtectedContextWithProtectedWindowSurface)
{
    windowTest(true, true);
}

// Protected context with Protected Mutable Render Buffer WindowSurface.
TEST_P(EGLProtectedContentTest, ProtectedContextWithProtectedMutableRenderBufferWindowSurface)
{
    windowTest(true, true, true);
}

void EGLProtectedContentTest::textureTest(bool isProtectedContext, bool isProtectedTexture)
{
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_protected_content"));

    bool isProtectedSurface = isProtectedTexture;

    EGLConfig config = EGL_NO_CONFIG_KHR;
    EXPECT_TRUE(chooseConfig(&config));
    ANGLE_SKIP_TEST_IF(config == EGL_NO_CONFIG_KHR);

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_TRUE(createContext(isProtectedContext, config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    OSWindow *osWindow = OSWindow::New();
    osWindow->initialize("ProtectedContentTest", kWidth, kHeight);
    EGLSurface windowSurface          = EGL_NO_SURFACE;
    EGLBoolean createWinSurfaceResult = createWindowSurface(
        isProtectedSurface, config, osWindow->getNativeWindow(), &windowSurface);
    EXPECT_TRUE(createWinSurfaceResult);
    ASSERT_EGL_SUCCESS() << "eglCreateWindowSurface failed.";
    glViewport(0, 0, kWidth, kHeight);

    EXPECT_TRUE(eglMakeCurrent(mDisplay, windowSurface, windowSurface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    if (IsGLExtensionEnabled("GL_EXT_protected_textures"))
    {
        GLuint texture = 0;
        bool result    = createTexture(isProtectedTexture, &texture);
        if (isProtectedTexture && !isProtectedContext)
        {
            std::cout << "Can't create protected Texture for Unprotected Context" << std::endl;
            ASSERT_FALSE(result);
        }
        else
        {
            ASSERT_TRUE(result);

            EXPECT_TRUE(fillTexture(texture, GLColor::red));
            EXPECT_TRUE(renderTexture(texture));

            eglSwapBuffers(mDisplay, windowSurface);
            ASSERT_EGL_SUCCESS() << "eglSwapBuffers failed.";
            checkSwapBuffersResult("RED", isProtectedContext, isProtectedSurface);

            glBindTexture(GL_TEXTURE_2D, 0);
            glDeleteTextures(1, &texture);
        }
    }
    else
    {
        std::cout << "Skipping tests, GL_EXT_protected_textures not supported" << std::endl;
    }

    EXPECT_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent - uncurrent failed.";

    eglDestroySurface(mDisplay, windowSurface);
    windowSurface = EGL_NO_SURFACE;
    osWindow->destroy();
    OSWindow::Delete(&osWindow);

    eglDestroyContext(mDisplay, context);
    context = EGL_NO_CONTEXT;
}

// Unprotected context with unprotected texture
TEST_P(EGLProtectedContentTest, UnprotectedContextWithUnprotectedTexture)
{
    textureTest(false, false);
}

// Protected context with protected texture
TEST_P(EGLProtectedContentTest, ProtectedContextWithProtectedTexture)
{
    textureTest(true, true);
}

void EGLProtectedContentTest::textureFromImageTest(bool isProtectedContext, bool isProtectedTexture)
{
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_protected_content"));

    bool isProtectedSurface = isProtectedTexture;

    EGLConfig config = EGL_NO_CONFIG_KHR;
    EXPECT_TRUE(chooseConfig(&config));
    ANGLE_SKIP_TEST_IF(config == EGL_NO_CONFIG_KHR);

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_TRUE(createContext(isProtectedContext, config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    OSWindow *osWindow = OSWindow::New();
    osWindow->initialize("ProtectedContentTest", kWidth, kHeight);
    EGLSurface windowSurface          = EGL_NO_SURFACE;
    EGLBoolean createWinSurfaceResult = createWindowSurface(
        isProtectedSurface, config, osWindow->getNativeWindow(), &windowSurface);
    EXPECT_TRUE(createWinSurfaceResult);
    ASSERT_EGL_SUCCESS() << "eglCreateWindowSurface failed.";

    EXPECT_TRUE(eglMakeCurrent(mDisplay, windowSurface, windowSurface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";
    glViewport(0, 0, kWidth, kHeight);

    if (IsGLExtensionEnabled("GL_OES_EGL_image") &&
        IsGLExtensionEnabled("GL_EXT_protected_textures"))
    {
        GLuint srcTexture = 0;
        if (isProtectedTexture && !isProtectedContext)
        {
            std::cout << "Can't create protected Texture for Unprotected Context, Skipping"
                      << std::endl;
            ASSERT_FALSE(createTexture(isProtectedTexture, &srcTexture));
        }
        else
        {
            ASSERT_TRUE(createTexture(isProtectedTexture, &srcTexture));
            EXPECT_TRUE(fillTexture(srcTexture, GLColor::red));

            EGLImage image = EGL_NO_IMAGE;
            EXPECT_TRUE(createImage(isProtectedTexture, context, EGL_GL_TEXTURE_2D,
                                    (void *)(static_cast<intptr_t>(srcTexture)), &image));

            GLuint dstTexture = 0;
            EXPECT_TRUE(createTextureFromImage(image, &dstTexture));
            EXPECT_TRUE(renderTexture(dstTexture));

            eglSwapBuffers(mDisplay, windowSurface);
            ASSERT_EGL_SUCCESS() << "eglSwapBuffers failed.";
            checkSwapBuffersResult("RED", isProtectedContext, isProtectedSurface);

            glBindTexture(GL_TEXTURE_2D, 0);
            glDeleteTextures(1, &dstTexture);
            glDeleteTextures(1, &srcTexture);

            eglDestroyImage(mDisplay, image);
            image = EGL_NO_IMAGE;
        }
    }
    else
    {
        std::cout << "Skipping tests, GL_OES_EGL_image or GL_EXT_protected_textures not supported"
                  << std::endl;
    }

    EXPECT_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent - uncurrent failed.";

    eglDestroySurface(mDisplay, windowSurface);
    windowSurface = EGL_NO_SURFACE;
    osWindow->destroy();
    OSWindow::Delete(&osWindow);

    eglDestroyContext(mDisplay, context);
    context = EGL_NO_CONTEXT;
}

// Unprotected context with unprotected texture from EGL image
TEST_P(EGLProtectedContentTest, UnprotectedContextWithUnprotectedTextureFromImage)
{
    textureFromImageTest(false, false);
}

// Protected context with protected texture from EGL image
TEST_P(EGLProtectedContentTest, ProtectedContextWithProtectedTextureFromImage)
{
    textureFromImageTest(true, true);
}

void EGLProtectedContentTest::textureFromPbufferTest(bool isProtectedContext,
                                                     bool isProtectedTexture)
{
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_protected_content"));

    bool isProtectedSurface = isProtectedTexture;

    EGLConfig config = EGL_NO_CONFIG_KHR;
    EXPECT_TRUE(chooseConfig(&config));
    ANGLE_SKIP_TEST_IF(config == EGL_NO_CONFIG_KHR);

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_TRUE(createContext(isProtectedContext, config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    EGLSurface pBufferSurface = EGL_NO_SURFACE;
    EXPECT_TRUE(createPbufferSurface(isProtectedSurface, config, &pBufferSurface));
    ASSERT_EGL_SUCCESS() << "eglCreatePbufferSurface failed.";

    EXPECT_TRUE(eglMakeCurrent(mDisplay, pBufferSurface, pBufferSurface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    glViewport(0, 0, kWidth, kHeight);
    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glFinish();
    ASSERT_GL_NO_ERROR() << "glFinish failed";

    OSWindow *osWindow = OSWindow::New();
    osWindow->initialize("ProtectedContentTest", kWidth, kHeight);
    EGLSurface windowSurface          = EGL_NO_SURFACE;
    EGLBoolean createWinSurfaceResult = createWindowSurface(
        isProtectedSurface, config, osWindow->getNativeWindow(), &windowSurface);
    EXPECT_TRUE(createWinSurfaceResult);
    ASSERT_EGL_SUCCESS() << "eglCreateWindowSurface failed.";

    EXPECT_TRUE(eglMakeCurrent(mDisplay, windowSurface, windowSurface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";
    glViewport(0, 0, kWidth, kHeight);

    if (IsGLExtensionEnabled("GL_EXT_protected_textures"))
    {
        GLuint texture = 0;
        EXPECT_TRUE(createTextureFromPbuffer(pBufferSurface, &texture));
        EXPECT_TRUE(renderTexture(texture));

        eglSwapBuffers(mDisplay, windowSurface);
        ASSERT_EGL_SUCCESS() << "eglSwapBuffers failed.";
        checkSwapBuffersResult("RED", isProtectedContext, isProtectedTexture);

        eglReleaseTexImage(mDisplay, pBufferSurface, EGL_BACK_BUFFER);
        glDeleteTextures(1, &texture);
    }
    else
    {
        std::cout << "Skipping tests, GL_EXT_protected_textures not supported" << std::endl;
    }

    EXPECT_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent - uncurrent failed.";

    eglDestroySurface(mDisplay, windowSurface);
    windowSurface = EGL_NO_SURFACE;
    osWindow->destroy();
    OSWindow::Delete(&osWindow);

    eglDestroySurface(mDisplay, pBufferSurface);
    pBufferSurface = EGL_NO_SURFACE;

    eglDestroyContext(mDisplay, context);
    context = EGL_NO_CONTEXT;
}

// Unprotected context with unprotected texture from BindTex of PBufferSurface
TEST_P(EGLProtectedContentTest, UnprotectedContextWithUnprotectedTextureFromPBuffer)
{
    textureFromPbufferTest(false, false);
}

// Protected context with protected texture from BindTex of PBufferSurface
TEST_P(EGLProtectedContentTest, ProtectedContextWithProtectedTextureFromPbuffer)
{
    textureFromPbufferTest(true, true);
}

void EGLProtectedContentTest::textureFromAndroidNativeBufferTest(bool isProtectedContext,
                                                                 bool isProtectedTexture)
{
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_protected_content"));
    ANGLE_SKIP_TEST_IF(
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_ANDROID_get_native_client_buffer"));
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_ANDROID_image_native_buffer"));

    bool isProtectedSurface = isProtectedTexture;

    EGLConfig config = EGL_NO_CONFIG_KHR;
    EXPECT_TRUE(chooseConfig(&config));
    ANGLE_SKIP_TEST_IF(config == EGL_NO_CONFIG_KHR);

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_TRUE(createContext(isProtectedContext, config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    OSWindow *osWindow = OSWindow::New();
    osWindow->initialize("ProtectedContentTest", kWidth, kHeight);
    EGLSurface windowSurface          = EGL_NO_SURFACE;
    EGLBoolean createWinSurfaceResult = createWindowSurface(
        isProtectedSurface, config, osWindow->getNativeWindow(), &windowSurface);
    EXPECT_TRUE(createWinSurfaceResult);
    ASSERT_EGL_SUCCESS() << "eglCreateWindowSurface failed.";

    EXPECT_TRUE(eglMakeCurrent(mDisplay, windowSurface, windowSurface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";
    glViewport(0, 0, kWidth, kHeight);

    if (IsGLExtensionEnabled("GL_EXT_protected_textures"))
    {
        EGLClientBuffer clientBuffer = nullptr;
        EXPECT_TRUE(createAndroidClientBuffer(isProtectedTexture, false, true, &clientBuffer));

        EGLImage image = EGL_NO_IMAGE;
        EXPECT_TRUE(createImage(isProtectedTexture, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID,
                                clientBuffer, &image));

        GLuint texture = 0;
        if (isProtectedTexture && !isProtectedContext)
        {
            std::cout << "Can't create protected Texture for Unprotected Context, Skipping"
                      << std::endl;
            ASSERT_FALSE(createTextureFromImage(image, &texture));
        }
        else
        {
            EXPECT_TRUE(createTextureFromImage(image, &texture));
            EXPECT_TRUE(fillTexture(texture, GLColor::red));
            EXPECT_TRUE(renderTexture(texture));

            eglSwapBuffers(mDisplay, windowSurface);
            ASSERT_EGL_SUCCESS() << "eglSwapBuffers failed.";
            checkSwapBuffersResult("RED", isProtectedContext, isProtectedTexture);

            glBindTexture(GL_TEXTURE_2D, 0);
            glDeleteTextures(1, &texture);

            eglDestroyImage(mDisplay, image);
            image = EGL_NO_IMAGE;
        }
    }
    else
    {
        std::cout << "Skipping tests, GL_EXT_protected_textures not supported" << std::endl;
    }

    EXPECT_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent - uncurrent failed.";

    eglDestroySurface(mDisplay, windowSurface);
    windowSurface = EGL_NO_SURFACE;
    osWindow->destroy();
    OSWindow::Delete(&osWindow);

    eglDestroyContext(mDisplay, context);
    context = EGL_NO_CONTEXT;
}

// Unprotected context with unprotected texture from EGL image from Android native buffer
TEST_P(EGLProtectedContentTest, UnprotectedContextWithUnprotectedTextureFromAndroidNativeBuffer)
{
    textureFromAndroidNativeBufferTest(false, false);
}

// Protected context with protected texture from EGL image from Android native buffer
TEST_P(EGLProtectedContentTest, ProtectedContextWithProtectedTextureFromAndroidNativeBuffer)
{
    textureFromAndroidNativeBufferTest(true, true);
}

// Retrieve the EGL_PROTECTED_CONTENT_EXT attribute via eglQueryContext
TEST_P(EGLProtectedContentTest, QueryContext)
{
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_protected_content"));

    EGLConfig config = EGL_NO_CONFIG_KHR;
    EXPECT_TRUE(chooseConfig(&config));
    ANGLE_SKIP_TEST_IF(config == EGL_NO_CONFIG_KHR);

    bool isProtectedContext = true;
    EGLContext context      = EGL_NO_CONTEXT;
    EXPECT_TRUE(createContext(isProtectedContext, config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    EGLint value = 0;
    EXPECT_EGL_TRUE(eglQueryContext(mDisplay, context, EGL_PROTECTED_CONTENT_EXT, &value));
    ASSERT_EGL_SUCCESS() << "eglQueryContext failed.";

    EXPECT_EQ(value, 1);
}

ANGLE_INSTANTIATE_TEST(EGLProtectedContentTest,
                       WithNoFixture(ES2_OPENGLES()),
                       WithNoFixture(ES3_OPENGLES()),
                       WithNoFixture(ES2_VULKAN()),
                       WithNoFixture(ES3_VULKAN()));

//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

#include <d3d11.h>
#include <cstdint>

#include "util/OSWindow.h"
#include "util/com_utils.h"

using namespace angle;

class EGLPresentPathD3D11 : public ANGLETest<>
{
  protected:
    EGLPresentPathD3D11()
        : mDisplay(EGL_NO_DISPLAY),
          mContext(EGL_NO_CONTEXT),
          mSurface(EGL_NO_SURFACE),
          mOffscreenSurfaceD3D11Texture(nullptr),
          mConfig(0),
          mOSWindow(nullptr),
          mWindowWidth(0)
    {}

    void testSetUp() override
    {
        mOSWindow    = OSWindow::New();
        mWindowWidth = 64;
        mOSWindow->initialize("EGLPresentPathD3D11", mWindowWidth, mWindowWidth);
    }

    void initializeEGL(bool usePresentPathFast)
    {
        int clientVersion = GetParam().majorVersion;

        // Set up EGL Display
        EGLint displayAttribs[] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE,
                                   GetParam().getRenderer(),
                                   EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE,
                                   GetParam().eglParameters.majorVersion,
                                   EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE,
                                   GetParam().eglParameters.majorVersion,
                                   EGL_EXPERIMENTAL_PRESENT_PATH_ANGLE,
                                   usePresentPathFast ? EGL_EXPERIMENTAL_PRESENT_PATH_FAST_ANGLE
                                                      : EGL_EXPERIMENTAL_PRESENT_PATH_COPY_ANGLE,
                                   EGL_NONE};
        mDisplay =
            eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, EGL_DEFAULT_DISPLAY, displayAttribs);
        ASSERT_TRUE(EGL_NO_DISPLAY != mDisplay);
        ASSERT_EGL_TRUE(eglInitialize(mDisplay, nullptr, nullptr));

        // Choose the EGL config
        EGLint numConfigs;
        EGLint configAttribs[] = {EGL_RED_SIZE,
                                  8,
                                  EGL_GREEN_SIZE,
                                  8,
                                  EGL_BLUE_SIZE,
                                  8,
                                  EGL_ALPHA_SIZE,
                                  8,
                                  EGL_RENDERABLE_TYPE,
                                  clientVersion == 3 ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT,
                                  EGL_SURFACE_TYPE,
                                  EGL_PBUFFER_BIT,
                                  EGL_NONE};
        ASSERT_EGL_TRUE(eglChooseConfig(mDisplay, configAttribs, &mConfig, 1, &numConfigs));
        ASSERT_EQ(1, numConfigs);

        // Set up the EGL context
        EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, clientVersion, EGL_NONE};
        mContext                = eglCreateContext(mDisplay, mConfig, nullptr, contextAttribs);
        ASSERT_TRUE(EGL_NO_CONTEXT != mContext);
    }

    void createWindowSurface()
    {
        mSurface = eglCreateWindowSurface(mDisplay, mConfig, mOSWindow->getNativeWindow(), nullptr);
    }

    void createPbufferFromClientBufferSurface()
    {
        EGLAttrib device      = 0;
        EGLAttrib angleDevice = 0;

        EXPECT_TRUE(IsEGLClientExtensionEnabled("EGL_EXT_device_query"));

        ASSERT_EGL_TRUE(eglQueryDisplayAttribEXT(mDisplay, EGL_DEVICE_EXT, &angleDevice));
        ASSERT_EGL_TRUE(eglQueryDeviceAttribEXT(reinterpret_cast<EGLDeviceEXT>(angleDevice),
                                                EGL_D3D11_DEVICE_ANGLE, &device));
        ID3D11Device *d3d11Device = reinterpret_cast<ID3D11Device *>(device);

        D3D11_TEXTURE2D_DESC textureDesc = {0};
        textureDesc.Width                = mWindowWidth;
        textureDesc.Height               = mWindowWidth;
        textureDesc.Format               = DXGI_FORMAT_B8G8R8A8_UNORM;
        textureDesc.MipLevels            = 1;
        textureDesc.ArraySize            = 1;
        textureDesc.SampleDesc.Count     = 1;
        textureDesc.SampleDesc.Quality   = 0;
        textureDesc.Usage                = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags            = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        textureDesc.CPUAccessFlags       = 0;
        textureDesc.MiscFlags            = D3D11_RESOURCE_MISC_SHARED;

        ASSERT_TRUE(SUCCEEDED(
            d3d11Device->CreateTexture2D(&textureDesc, nullptr, &mOffscreenSurfaceD3D11Texture)));

        IDXGIResource *dxgiResource =
            DynamicCastComObject<IDXGIResource>(mOffscreenSurfaceD3D11Texture);
        ASSERT_NE(nullptr, dxgiResource);

        HANDLE sharedHandle = 0;
        ASSERT_TRUE(SUCCEEDED(dxgiResource->GetSharedHandle(&sharedHandle)));
        SafeRelease(dxgiResource);

        EGLint pBufferAttributes[] = {EGL_WIDTH,          mWindowWidth,       EGL_HEIGHT,
                                      mWindowWidth,       EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
                                      EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGBA,   EGL_NONE};

        mSurface = eglCreatePbufferFromClientBuffer(mDisplay, EGL_D3D_TEXTURE_2D_SHARE_HANDLE_ANGLE,
                                                    sharedHandle, mConfig, pBufferAttributes);
        ASSERT_TRUE(EGL_NO_SURFACE != mSurface);
    }

    void makeCurrent() { ASSERT_EGL_TRUE(eglMakeCurrent(mDisplay, mSurface, mSurface, mContext)); }

    void testTearDown() override
    {
        SafeRelease(mOffscreenSurfaceD3D11Texture);

        if (mDisplay != EGL_NO_DISPLAY)
        {
            eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

            if (mSurface != EGL_NO_SURFACE)
            {
                eglDestroySurface(mDisplay, mSurface);
                mSurface = EGL_NO_SURFACE;
            }

            if (mContext != EGL_NO_CONTEXT)
            {
                ASSERT_EGL_TRUE(
                    eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
                eglDestroyContext(mDisplay, mContext);
                mContext = EGL_NO_CONTEXT;
            }

            eglTerminate(mDisplay);
            mDisplay = EGL_NO_DISPLAY;
        }

        mOSWindow->destroy();
        OSWindow::Delete(&mOSWindow);
    }

    void drawQuadUsingGL()
    {
        GLuint m2DProgram;
        GLint mTexture2DUniformLocation;

        constexpr char kVS[] =
            R"(precision highp float;
            attribute vec4 position;
            varying vec2 texcoord;

            void main()
            {
                gl_Position = vec4(position.xy, 0.0, 1.0);
                texcoord = (position.xy * 0.5) + 0.5;
            })";

        constexpr char kFS[] =
            R"(precision highp float;
            uniform sampler2D tex;
            varying vec2 texcoord;

            void main()
            {
                gl_FragColor = texture2D(tex, texcoord);
            })";

        m2DProgram                = CompileProgram(kVS, kFS);
        mTexture2DUniformLocation = glGetUniformLocation(m2DProgram, "tex");

        uint8_t textureInitData[16] = {
            255, 0,   0,   255,  // Red
            0,   255, 0,   255,  // Green
            0,   0,   255, 255,  // Blue
            255, 255, 0,   255   // Red + Green
        };

        // Create a simple RGBA texture
        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     textureInitData);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        ASSERT_GL_NO_ERROR();

        // Draw a quad using the texture
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(m2DProgram);
        glUniform1i(mTexture2DUniformLocation, 0);

        GLint positionLocation = glGetAttribLocation(m2DProgram, "position");
        glUseProgram(m2DProgram);
        const GLfloat vertices[] = {
            -1.0f, 1.0f, 0.5f, -1.0f, -1.0f, 0.5f, 1.0f, -1.0f, 0.5f,
            -1.0f, 1.0f, 0.5f, 1.0f,  -1.0f, 0.5f, 1.0f, 1.0f,  0.5f,
        };

        glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, vertices);
        glEnableVertexAttribArray(positionLocation);

        glDrawArrays(GL_TRIANGLES, 0, 6);
        ASSERT_GL_NO_ERROR();

        glDeleteProgram(m2DProgram);
    }

    void checkPixelsUsingGL()
    {
        // Note that the texture is in BGRA format
        EXPECT_PIXEL_EQ(0, 0, 255, 0, 0, 255);                                  // Red
        EXPECT_PIXEL_EQ(mWindowWidth - 1, 0, 0, 255, 0, 255);                   // Green
        EXPECT_PIXEL_EQ(0, mWindowWidth - 1, 0, 0, 255, 255);                   // Blue
        EXPECT_PIXEL_EQ(mWindowWidth - 1, mWindowWidth - 1, 255, 255, 0, 255);  // Red + green
    }

    void checkPixelsUsingD3D(bool usingPresentPathFast)
    {
        ASSERT_NE(nullptr, mOffscreenSurfaceD3D11Texture);

        D3D11_TEXTURE2D_DESC textureDesc = {0};
        ID3D11Device *device;
        ID3D11DeviceContext *context;
        mOffscreenSurfaceD3D11Texture->GetDesc(&textureDesc);
        mOffscreenSurfaceD3D11Texture->GetDevice(&device);
        device->GetImmediateContext(&context);
        ASSERT_NE(nullptr, device);
        ASSERT_NE(nullptr, context);

        textureDesc.CPUAccessFlags  = D3D11_CPU_ACCESS_READ;
        textureDesc.Usage           = D3D11_USAGE_STAGING;
        textureDesc.BindFlags       = 0;
        textureDesc.MiscFlags       = 0;
        ID3D11Texture2D *cpuTexture = nullptr;
        ASSERT_TRUE(SUCCEEDED(device->CreateTexture2D(&textureDesc, nullptr, &cpuTexture)));

        context->CopyResource(cpuTexture, mOffscreenSurfaceD3D11Texture);

        D3D11_MAPPED_SUBRESOURCE mappedSubresource;
        context->Map(cpuTexture, 0, D3D11_MAP_READ, 0, &mappedSubresource);
        ASSERT_EQ(static_cast<UINT>(mWindowWidth * 4), mappedSubresource.RowPitch);
        ASSERT_EQ(static_cast<UINT>(mWindowWidth * mWindowWidth * 4), mappedSubresource.DepthPitch);

        angle::GLColor *byteData = reinterpret_cast<angle::GLColor *>(mappedSubresource.pData);

        // Note that the texture is in BGRA format, although the GLColor struct is RGBA
        GLColor expectedTopLeftPixel     = GLColor(0, 0, 255, 255);    // Red
        GLColor expectedTopRightPixel    = GLColor(0, 255, 0, 255);    // Green
        GLColor expectedBottomLeftPixel  = GLColor(255, 0, 0, 255);    // Blue
        GLColor expectedBottomRightPixel = GLColor(0, 255, 255, 255);  // Red + Green

        if (usingPresentPathFast)
        {
            // Invert the expected values
            GLColor tempTopLeft      = expectedTopLeftPixel;
            GLColor tempTopRight     = expectedTopRightPixel;
            expectedTopLeftPixel     = expectedBottomLeftPixel;
            expectedTopRightPixel    = expectedBottomRightPixel;
            expectedBottomLeftPixel  = tempTopLeft;
            expectedBottomRightPixel = tempTopRight;
        }

        EXPECT_EQ(expectedTopLeftPixel, byteData[0]);
        EXPECT_EQ(expectedTopRightPixel, byteData[(mWindowWidth - 1)]);
        EXPECT_EQ(expectedBottomLeftPixel, byteData[(mWindowWidth - 1) * mWindowWidth]);
        EXPECT_EQ(expectedBottomRightPixel,
                  byteData[(mWindowWidth - 1) * mWindowWidth + (mWindowWidth - 1)]);

        context->Unmap(cpuTexture, 0);
        SafeRelease(cpuTexture);
        SafeRelease(device);
        SafeRelease(context);
    }

    EGLDisplay mDisplay;
    EGLContext mContext;
    EGLSurface mSurface;
    ID3D11Texture2D *mOffscreenSurfaceD3D11Texture;
    EGLConfig mConfig;
    OSWindow *mOSWindow;
    GLint mWindowWidth;
};

// Test that rendering basic content onto a window surface when present path fast
// is enabled works as expected
TEST_P(EGLPresentPathD3D11, WindowPresentPathFast)
{
    initializeEGL(true);
    createWindowSurface();
    makeCurrent();

    drawQuadUsingGL();

    checkPixelsUsingGL();
}

// Test that rendering basic content onto a client buffer surface when present path fast
// works as expected, and is also oriented the correct way around
TEST_P(EGLPresentPathD3D11, ClientBufferPresentPathFast)
{
    initializeEGL(true);
    createPbufferFromClientBufferSurface();
    makeCurrent();

    drawQuadUsingGL();

    checkPixelsUsingGL();
    checkPixelsUsingD3D(true);
}

// Test that rendering basic content onto a window surface when present path fast
// is disabled works as expected
TEST_P(EGLPresentPathD3D11, WindowPresentPathCopy)
{
    initializeEGL(false);
    createWindowSurface();
    makeCurrent();

    drawQuadUsingGL();

    checkPixelsUsingGL();
}

// Test that rendering basic content onto a client buffer surface when present path
// fast is disabled works as expected, and is also oriented the correct way around
TEST_P(EGLPresentPathD3D11, ClientBufferPresentPathCopy)
{
    initializeEGL(false);
    createPbufferFromClientBufferSurface();
    makeCurrent();

    drawQuadUsingGL();

    checkPixelsUsingGL();
    checkPixelsUsingD3D(false);
}

ANGLE_INSTANTIATE_TEST(EGLPresentPathD3D11, WithNoFixture(ES2_D3D11()));

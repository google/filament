//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// D3DTextureTest:
//   Tests of the EGL_ANGLE_d3d_texture_client_buffer extension

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include <d3d11.h>
#include <d3d9.h>
#include <dxgiformat.h>
#include <windows.h>
#include <wrl/client.h>

#include "util/EGLWindow.h"
#include "util/com_utils.h"

namespace angle
{

class D3DTextureTest : public ANGLETest<>
{
  protected:
    D3DTextureTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
        setConfigStencilBits(8);
    }

    void testSetUp() override
    {
        constexpr char kVS[] =
            R"(precision highp float;
            attribute vec4 position;
            varying vec2 texcoord;

            void main()
            {
                gl_Position = position;
                texcoord = (position.xy * 0.5) + 0.5;
                texcoord.y = 1.0 - texcoord.y;
            })";

        constexpr char kTextureFS[] =
            R"(precision highp float;
            uniform sampler2D tex;
            varying vec2 texcoord;

            void main()
            {
                gl_FragColor = texture2D(tex, texcoord);
            })";

        constexpr char kTextureFSNoSampling[] =
            R"(precision highp float;

            void main()
            {
                gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
            })";

        mTextureProgram = CompileProgram(kVS, kTextureFS);
        ASSERT_NE(0u, mTextureProgram) << "shader compilation failed.";

        mTextureUniformLocation = glGetUniformLocation(mTextureProgram, "tex");
        ASSERT_NE(-1, mTextureUniformLocation);

        mTextureProgramNoSampling = CompileProgram(kVS, kTextureFSNoSampling);
        ASSERT_NE(0u, mTextureProgramNoSampling) << "shader compilation failed.";

        mD3D11Module = LoadLibrary(TEXT("d3d11.dll"));
        ASSERT_NE(nullptr, mD3D11Module);

        PFN_D3D11_CREATE_DEVICE createDeviceFunc = reinterpret_cast<PFN_D3D11_CREATE_DEVICE>(
            GetProcAddress(mD3D11Module, "D3D11CreateDevice"));

        EGLWindow *window   = getEGLWindow();
        EGLDisplay display  = window->getDisplay();
        EGLDeviceEXT device = EGL_NO_DEVICE_EXT;
        if (IsEGLClientExtensionEnabled("EGL_EXT_device_query"))
        {
            EGLAttrib result = 0;
            EXPECT_EGL_TRUE(eglQueryDisplayAttribEXT(display, EGL_DEVICE_EXT, &result));
            device = reinterpret_cast<EGLDeviceEXT>(result);
        }

        ASSERT_NE(EGL_NO_DEVICE_EXT, device);

        if (IsEGLDeviceExtensionEnabled(device, "EGL_ANGLE_device_d3d"))
        {
            EGLAttrib result = 0;
            if (eglQueryDeviceAttribEXT(device, EGL_D3D11_DEVICE_ANGLE, &result))
            {
                mD3D11Device = reinterpret_cast<ID3D11Device *>(result);
                mD3D11Device->AddRef();
            }
            else if (eglQueryDeviceAttribEXT(device, EGL_D3D9_DEVICE_ANGLE, &result))
            {
                mD3D9Device = reinterpret_cast<IDirect3DDevice9 *>(result);
                mD3D9Device->AddRef();
            }
        }
        else
        {
            ASSERT_TRUE(
                SUCCEEDED(createDeviceFunc(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr,
                                           0, D3D11_SDK_VERSION, &mD3D11Device, nullptr, nullptr)));
        }
    }

    void testTearDown() override
    {
        glDeleteProgram(mTextureProgram);
        glDeleteProgram(mTextureProgramNoSampling);

        if (mD3D11Device)
        {
            mD3D11Device->Release();
            mD3D11Device = nullptr;
        }

        FreeLibrary(mD3D11Module);
        mD3D11Module = nullptr;

        if (mD3D9Device)
        {
            mD3D9Device->Release();
            mD3D9Device = nullptr;
        }
    }

    EGLSurface createD3D11PBuffer(size_t width,
                                  size_t height,
                                  UINT sampleCount,
                                  UINT sampleQuality,
                                  UINT bindFlags,
                                  DXGI_FORMAT format,
                                  const EGLint *attribs)
    {
        EGLWindow *window  = getEGLWindow();
        EGLDisplay display = window->getDisplay();
        EGLConfig config   = window->getConfig();

        EXPECT_TRUE(mD3D11Device != nullptr);
        ID3D11Texture2D *texture = nullptr;
        CD3D11_TEXTURE2D_DESC desc(format, static_cast<UINT>(width), static_cast<UINT>(height), 1,
                                   1, bindFlags);
        desc.SampleDesc.Count   = sampleCount;
        desc.SampleDesc.Quality = sampleQuality;
        EXPECT_TRUE(SUCCEEDED(mD3D11Device->CreateTexture2D(&desc, nullptr, &texture)));

        EGLSurface pbuffer = eglCreatePbufferFromClientBuffer(display, EGL_D3D_TEXTURE_ANGLE,
                                                              texture, config, attribs);

        texture->Release();

        return pbuffer;
    }

    EGLSurface createD3D11PBuffer(size_t width,
                                  size_t height,
                                  EGLint eglTextureFormat,
                                  EGLint eglTextureTarget,
                                  UINT sampleCount,
                                  UINT sampleQuality,
                                  UINT bindFlags,
                                  DXGI_FORMAT format)
    {
        EGLint attribs[] = {
            EGL_TEXTURE_FORMAT, eglTextureFormat, EGL_TEXTURE_TARGET,
            eglTextureTarget,   EGL_NONE,         EGL_NONE,
        };
        return createD3D11PBuffer(width, height, sampleCount, sampleQuality, bindFlags, format,
                                  attribs);
    }

    EGLSurface createPBuffer(size_t width,
                             size_t height,
                             EGLint eglTextureFormat,
                             EGLint eglTextureTarget,
                             UINT sampleCount,
                             UINT sampleQuality)
    {
        if (mD3D11Device)
        {
            return createD3D11PBuffer(
                width, height, eglTextureFormat, eglTextureTarget, sampleCount, sampleQuality,
                D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, DXGI_FORMAT_R8G8B8A8_UNORM);
        }

        if (mD3D9Device)
        {
            EGLWindow *window  = getEGLWindow();
            EGLDisplay display = window->getDisplay();
            EGLConfig config   = window->getConfig();

            EGLint attribs[] = {
                EGL_TEXTURE_FORMAT, eglTextureFormat, EGL_TEXTURE_TARGET,
                eglTextureTarget,   EGL_NONE,         EGL_NONE,
            };

            // Multisampled textures are not supported on D3D9.
            EXPECT_TRUE(sampleCount <= 1);
            EXPECT_TRUE(sampleQuality == 0);

            IDirect3DTexture9 *texture = nullptr;
            EXPECT_TRUE(SUCCEEDED(mD3D9Device->CreateTexture(
                static_cast<UINT>(width), static_cast<UINT>(height), 1, D3DUSAGE_RENDERTARGET,
                D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture, nullptr)));

            EGLSurface pbuffer = eglCreatePbufferFromClientBuffer(display, EGL_D3D_TEXTURE_ANGLE,
                                                                  texture, config, attribs);

            texture->Release();

            return pbuffer;
        }
        else
        {
            return EGL_NO_SURFACE;
        }
    }

    bool valid() const
    {
        EGLWindow *window  = getEGLWindow();
        EGLDisplay display = window->getDisplay();
        if (!IsEGLDisplayExtensionEnabled(display, "EGL_ANGLE_d3d_texture_client_buffer"))
        {
            std::cout << "Test skipped due to missing EGL_ANGLE_d3d_texture_client_buffer"
                      << std::endl;
            return false;
        }

        if (!mD3D11Device && !mD3D9Device)
        {
            std::cout << "Test skipped due to no D3D devices being available." << std::endl;
            return false;
        }

        if (IsWindows() && IsAMD() && IsOpenGL())
        {
            std::cout << "Test skipped on Windows AMD OpenGL." << std::endl;
            return false;
        }

        if (IsWindows() && IsIntel() && IsOpenGL())
        {
            std::cout << "Test skipped on Windows Intel OpenGL." << std::endl;
            return false;
        }
        return true;
    }

    void testTextureSamplesAs50PercentGreen(GLuint texture)
    {
        GLFramebuffer scratchFbo;
        glBindFramebuffer(GL_FRAMEBUFFER, scratchFbo);
        GLTexture scratchTexture;
        glBindTexture(GL_TEXTURE_2D, scratchTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, scratchTexture,
                               0);

        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(mTextureProgram);
        glUniform1i(mTextureUniformLocation, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        drawQuad(mTextureProgram, "position", 0.5f);
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(0u, 127u, 0u, 255u), 2);
    }

    GLuint mTextureProgram;
    GLuint mTextureProgramNoSampling;
    GLint mTextureUniformLocation;

    HMODULE mD3D11Module       = nullptr;
    ID3D11Device *mD3D11Device = nullptr;

    IDirect3DDevice9 *mD3D9Device = nullptr;
};

// Test creating pbuffer from textures with several different DXGI formats.
TEST_P(D3DTextureTest, TestD3D11SupportedFormatsSurface)
{
    bool srgbSupported = IsGLExtensionEnabled("GL_EXT_sRGB") || getClientMajorVersion() == 3;
    ANGLE_SKIP_TEST_IF(!valid() || !mD3D11Device || !srgbSupported);

    const DXGI_FORMAT formats[] = {DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                                   DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB};
    for (size_t i = 0; i < 4; ++i)
    {
        if (formats[i] == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB)
        {
            if (IsOpenGL())
            {
                // This generates an invalid format error when calling wglDXRegisterObjectNV().
                // Reproducible at least on NVIDIA driver 390.65 on Windows 10.
                std::cout << "DXGI_FORMAT_B8G8R8A8_UNORM_SRGB subtest skipped: IsOpenGL().\n";
                continue;
            }
        }

        EGLSurface pbuffer = createD3D11PBuffer(32, 32, EGL_TEXTURE_RGBA, EGL_TEXTURE_2D, 1, 0,
                                                D3D11_BIND_RENDER_TARGET, formats[i]);
        ASSERT_EGL_SUCCESS();
        ASSERT_NE(EGL_NO_SURFACE, pbuffer);

        EGLWindow *window  = getEGLWindow();
        EGLDisplay display = window->getDisplay();

        EGLint colorspace = EGL_NONE;
        eglQuerySurface(display, pbuffer, EGL_GL_COLORSPACE, &colorspace);

        if (formats[i] == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB ||
            formats[i] == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB)
        {
            EXPECT_EQ(EGL_GL_COLORSPACE_SRGB, colorspace);
        }
        else
        {
            EXPECT_EQ(EGL_GL_COLORSPACE_LINEAR, colorspace);
        }

        eglMakeCurrent(display, pbuffer, pbuffer, window->getContext());
        ASSERT_EGL_SUCCESS();
        window->makeCurrent();
        eglDestroySurface(display, pbuffer);
    }
}

// Test binding a pbuffer created from a D3D texture as a texture image with several different DXGI
// formats. The test renders to and samples from the pbuffer.
TEST_P(D3DTextureTest, TestD3D11SupportedFormatsTexture)
{
    bool srgb8alpha8TextureAttachmentSupported = getClientMajorVersion() >= 3;
    ANGLE_SKIP_TEST_IF(!valid() || !mD3D11Device || !srgb8alpha8TextureAttachmentSupported);

    bool srgbWriteControlSupported =
        IsGLExtensionEnabled("GL_EXT_sRGB_write_control") && !IsOpenGL();

    const DXGI_FORMAT formats[] = {DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM,
                                   DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                                   DXGI_FORMAT_B8G8R8A8_UNORM_SRGB};
    for (size_t i = 0; i < 4; ++i)
    {
        if (formats[i] == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB)
        {
            if (IsOpenGL())
            {
                // This generates an invalid format error when calling wglDXRegisterObjectNV().
                // Reproducible at least on NVIDIA driver 390.65 on Windows 10.
                std::cout << "DXGI_FORMAT_B8G8R8A8_UNORM_SRGB subtest skipped: IsOpenGL().\n";
                continue;
            }
        }

        SCOPED_TRACE(std::string("Test case:") + std::to_string(i));
        EGLWindow *window  = getEGLWindow();
        EGLDisplay display = window->getDisplay();

        EGLSurface pbuffer =
            createD3D11PBuffer(32, 32, EGL_TEXTURE_RGBA, EGL_TEXTURE_2D, 1, 0,
                               D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, formats[i]);
        ASSERT_EGL_SUCCESS();
        ASSERT_NE(EGL_NO_SURFACE, pbuffer);

        EGLint colorspace = EGL_NONE;
        eglQuerySurface(display, pbuffer, EGL_GL_COLORSPACE, &colorspace);

        GLuint texture = 0u;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        EGLBoolean result = eglBindTexImage(display, pbuffer, EGL_BACK_BUFFER);
        ASSERT_EGL_SUCCESS();
        ASSERT_EGL_TRUE(result);

        GLuint fbo = 0u;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        glViewport(0, 0, 32, 32);

        GLint colorEncoding = 0;
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                              GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING_EXT,
                                              &colorEncoding);

        if (formats[i] == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB ||
            formats[i] == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB)
        {
            EXPECT_EQ(EGL_GL_COLORSPACE_SRGB, colorspace);
            EXPECT_EQ(GL_SRGB_EXT, colorEncoding);
        }
        else
        {
            EXPECT_EQ(EGL_GL_COLORSPACE_LINEAR, colorspace);
            EXPECT_EQ(GL_LINEAR, colorEncoding);
        }

        // Clear the texture with 50% green and check that the color value written is correct.
        glClearColor(0.0f, 0.5f, 0.0f, 1.0f);

        if (colorEncoding == GL_SRGB_EXT)
        {
            glClear(GL_COLOR_BUFFER_BIT);
            EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(0u, 188u, 0u, 255u), 2);
            // Disable SRGB and run the non-sRGB test case.
            if (srgbWriteControlSupported)
                glDisable(GL_FRAMEBUFFER_SRGB_EXT);
        }

        if (colorEncoding == GL_LINEAR || srgbWriteControlSupported)
        {
            glClear(GL_COLOR_BUFFER_BIT);
            EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(0u, 127u, 0u, 255u), 2);
        }

        // Draw with the texture to a linear framebuffer and check that the color value written is
        // correct.
        testTextureSamplesAs50PercentGreen(texture);

        glBindFramebuffer(GL_FRAMEBUFFER, 0u);
        glBindTexture(GL_TEXTURE_2D, 0u);
        glDeleteTextures(1, &texture);
        glDeleteFramebuffers(1, &fbo);
        eglDestroySurface(display, pbuffer);
    }
}

// Test binding a pbuffer created from a D3D texture as a texture image with typeless texture
// formats.
TEST_P(D3DTextureTest, TestD3D11TypelessTexture)
{
    EGLWindow *window  = getEGLWindow();
    EGLDisplay display = window->getDisplay();

    ANGLE_SKIP_TEST_IF(!valid());

    // Typeless formats are optional in the spec and currently only supported on D3D11 backend.
    ANGLE_SKIP_TEST_IF(!IsD3D11());

    // GL_SRGB8_ALPHA8 texture attachment support is required.
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);

    const std::array<EGLint, 2> eglGlColorspaces = {EGL_GL_COLORSPACE_LINEAR,
                                                    EGL_GL_COLORSPACE_SRGB};
    const std::array<DXGI_FORMAT, 2> dxgiFormats = {DXGI_FORMAT_R8G8B8A8_TYPELESS,
                                                    DXGI_FORMAT_B8G8R8A8_TYPELESS};
    for (auto eglGlColorspace : eglGlColorspaces)
    {
        for (auto dxgiFormat : dxgiFormats)
        {
            SCOPED_TRACE(std::string("Test case:") + std::to_string(eglGlColorspace) + " / " +
                         std::to_string(dxgiFormat));

            EGLint attribs[] = {
                EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGBA, EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
                EGL_GL_COLORSPACE,  eglGlColorspace,  EGL_NONE,           EGL_NONE,
            };

            EGLSurface pbuffer = createD3D11PBuffer(
                32, 32, 1, 0, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, dxgiFormat,
                attribs);

            ASSERT_EGL_SUCCESS();
            ASSERT_NE(EGL_NO_SURFACE, pbuffer);

            EGLint colorspace = EGL_NONE;
            eglQuerySurface(display, pbuffer, EGL_GL_COLORSPACE, &colorspace);

            GLuint texture = 0u;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            EGLBoolean result = eglBindTexImage(display, pbuffer, EGL_BACK_BUFFER);
            ASSERT_EGL_SUCCESS();
            ASSERT_EGL_TRUE(result);

            GLuint fbo = 0u;
            glGenFramebuffers(1, &fbo);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
            glViewport(0, 0, 32, 32);

            GLint colorEncoding = 0;
            glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                                  GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING_EXT,
                                                  &colorEncoding);

            if (eglGlColorspace == EGL_GL_COLORSPACE_LINEAR)
            {
                EXPECT_EQ(EGL_GL_COLORSPACE_LINEAR, colorspace);
                EXPECT_EQ(GL_LINEAR, colorEncoding);
            }
            else
            {
                EXPECT_EQ(EGL_GL_COLORSPACE_SRGB, colorspace);
                EXPECT_EQ(GL_SRGB_EXT, colorEncoding);
            }

            // Clear the texture with 50% green and check that the color value written is correct.
            glClearColor(0.0f, 0.5f, 0.0f, 1.0f);

            if (colorEncoding == GL_SRGB_EXT)
            {
                glClear(GL_COLOR_BUFFER_BIT);
                EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(0u, 188u, 0u, 255u), 2);
            }
            if (colorEncoding == GL_LINEAR)
            {
                glClear(GL_COLOR_BUFFER_BIT);
                EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(0u, 127u, 0u, 255u), 2);
            }

            // Draw with the texture to a linear framebuffer and check that the color value written
            // is correct.
            testTextureSamplesAs50PercentGreen(texture);

            glBindFramebuffer(GL_FRAMEBUFFER, 0u);
            glBindTexture(GL_TEXTURE_2D, 0u);
            glDeleteTextures(1, &texture);
            glDeleteFramebuffers(1, &fbo);
            eglDestroySurface(display, pbuffer);
        }
    }
}

class D3DTextureTestES3 : public D3DTextureTest
{
  protected:
    D3DTextureTestES3() : D3DTextureTest() {}
};

// Test swizzling a pbuffer created from a D3D texture as a texture image with typeless texture
// formats.
TEST_P(D3DTextureTestES3, TestD3D11TypelessTextureSwizzle)
{
    EGLWindow *window  = getEGLWindow();
    EGLDisplay display = window->getDisplay();

    ANGLE_SKIP_TEST_IF(!valid());

    // Typeless formats are optional in the spec and currently only supported on D3D11 backend.
    ANGLE_SKIP_TEST_IF(!IsD3D11());

    const std::array<EGLint, 2> eglGlColorspaces = {EGL_GL_COLORSPACE_LINEAR,
                                                    EGL_GL_COLORSPACE_SRGB};
    const std::array<DXGI_FORMAT, 2> dxgiFormats = {DXGI_FORMAT_R8G8B8A8_TYPELESS,
                                                    DXGI_FORMAT_B8G8R8A8_TYPELESS};
    for (auto eglGlColorspace : eglGlColorspaces)
    {
        for (auto dxgiFormat : dxgiFormats)
        {
            SCOPED_TRACE(std::string("Test case:") + std::to_string(eglGlColorspace) + " / " +
                         std::to_string(dxgiFormat));

            EGLint attribs[] = {
                EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGBA, EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
                EGL_GL_COLORSPACE,  eglGlColorspace,  EGL_NONE,           EGL_NONE,
            };

            EGLSurface pbuffer = createD3D11PBuffer(
                32, 32, 1, 0, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, dxgiFormat,
                attribs);

            ASSERT_EGL_SUCCESS();
            ASSERT_NE(EGL_NO_SURFACE, pbuffer);

            EGLint colorspace = EGL_NONE;
            eglQuerySurface(display, pbuffer, EGL_GL_COLORSPACE, &colorspace);

            GLuint texture = 0u;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            EGLBoolean result = eglBindTexImage(display, pbuffer, EGL_BACK_BUFFER);
            ASSERT_EGL_SUCCESS();
            ASSERT_EGL_TRUE(result);

            GLuint fbo = 0u;
            glGenFramebuffers(1, &fbo);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
            glViewport(0, 0, 32, 32);

            GLint colorEncoding = 0;
            glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                                  GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING_EXT,
                                                  &colorEncoding);

            // Clear the texture with 50% blue and check that the color value written is correct.
            glClearColor(0.0f, 0.0f, 0.5f, 1.0f);

            if (colorEncoding == GL_SRGB_EXT)
            {
                glClear(GL_COLOR_BUFFER_BIT);
                EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(0u, 0u, 188u, 255u), 2);
            }
            if (colorEncoding == GL_LINEAR)
            {
                glClear(GL_COLOR_BUFFER_BIT);
                EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(0u, 0u, 127u, 255u), 2);
            }

            // Swizzle the green channel to be sampled from the blue channel of the texture and vice
            // versa.
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_BLUE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_GREEN);
            ASSERT_GL_NO_ERROR();

            // Draw with the texture to a linear framebuffer and check that the color value written
            // is correct.
            testTextureSamplesAs50PercentGreen(texture);

            glBindFramebuffer(GL_FRAMEBUFFER, 0u);
            glBindTexture(GL_TEXTURE_2D, 0u);
            glDeleteTextures(1, &texture);
            glDeleteFramebuffers(1, &fbo);
            eglDestroySurface(display, pbuffer);
        }
    }
}

// Test that EGL_GL_COLORSPACE attrib is not allowed for typed D3D textures.
TEST_P(D3DTextureTest, GlColorspaceNotAllowedForTypedD3DTexture)
{
    ANGLE_SKIP_TEST_IF(!valid());

    // D3D11 device is required to be able to create the texture.
    ANGLE_SKIP_TEST_IF(!mD3D11Device);

    // SRGB support is required.
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_sRGB") && getClientMajorVersion() < 3);

    EGLint attribsExplicitColorspace[] = {
        EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGBA,       EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
        EGL_GL_COLORSPACE,  EGL_GL_COLORSPACE_SRGB, EGL_NONE,           EGL_NONE,
    };
    EGLSurface pbuffer =
        createD3D11PBuffer(32, 32, 1, 0, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
                           DXGI_FORMAT_R8G8B8A8_UNORM, attribsExplicitColorspace);

    ASSERT_EGL_ERROR(EGL_BAD_MATCH);
    ASSERT_EQ(EGL_NO_SURFACE, pbuffer);
}

// Test that trying to create a pbuffer from a typeless texture fails as expected on the backends
// where they are known not to be supported.
TEST_P(D3DTextureTest, TypelessD3DTextureNotSupported)
{
    ANGLE_SKIP_TEST_IF(!valid());

    // D3D11 device is required to be able to create the texture.
    ANGLE_SKIP_TEST_IF(!mD3D11Device);

    // Currently typeless textures are supported on the D3D11 backend. We're testing the backends
    // where there is no support.
    ANGLE_SKIP_TEST_IF(IsD3D11());

    // SRGB support is required.
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_sRGB") && getClientMajorVersion() < 3);

    EGLint attribs[] = {
        EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGBA, EGL_TEXTURE_TARGET,
        EGL_TEXTURE_2D,     EGL_NONE,         EGL_NONE,
    };
    EGLSurface pbuffer =
        createD3D11PBuffer(32, 32, 1, 0, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
                           DXGI_FORMAT_R8G8B8A8_TYPELESS, attribs);
    ASSERT_EGL_ERROR(EGL_BAD_PARAMETER);
    ASSERT_EQ(EGL_NO_SURFACE, pbuffer);
}

// Test creating a pbuffer with unnecessary EGL_WIDTH and EGL_HEIGHT attributes because that's what
// Chromium does. This is a regression test for crbug.com/794086
TEST_P(D3DTextureTest, UnnecessaryWidthHeightAttributes)
{
    ANGLE_SKIP_TEST_IF(!valid() || !IsD3D11());
    ASSERT_TRUE(mD3D11Device != nullptr);
    ID3D11Texture2D *texture = nullptr;
    CD3D11_TEXTURE2D_DESC desc(DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 1, 1, D3D11_BIND_RENDER_TARGET);
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    EXPECT_TRUE(SUCCEEDED(mD3D11Device->CreateTexture2D(&desc, nullptr, &texture)));

    EGLint attribs[] = {
        EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGBA,
        EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
        EGL_WIDTH,          1,
        EGL_HEIGHT,         1,
        EGL_NONE,           EGL_NONE,
    };

    EGLWindow *window  = getEGLWindow();
    EGLDisplay display = window->getDisplay();
    EGLConfig config   = window->getConfig();

    EGLSurface pbuffer =
        eglCreatePbufferFromClientBuffer(display, EGL_D3D_TEXTURE_ANGLE, texture, config, attribs);

    ASSERT_EGL_SUCCESS();
    ASSERT_NE(pbuffer, EGL_NO_SURFACE);

    texture->Release();

    // Make current with fixture EGL to ensure the Surface can be released immediately.
    getEGLWindow()->makeCurrent();
    eglDestroySurface(display, pbuffer);
}

// Test creating a pbuffer from a d3d surface and clearing it
TEST_P(D3DTextureTest, Clear)
{
    if (!valid())
    {
        return;
    }

    EGLWindow *window  = getEGLWindow();
    EGLDisplay display = window->getDisplay();

    const size_t bufferSize = 32;

    EGLSurface pbuffer =
        createPBuffer(bufferSize, bufferSize, EGL_NO_TEXTURE, EGL_NO_TEXTURE, 1, 0);
    ASSERT_EGL_SUCCESS();
    ASSERT_NE(pbuffer, EGL_NO_SURFACE);

    // Apply the Pbuffer and clear it to purple and verify
    eglMakeCurrent(display, pbuffer, pbuffer, window->getContext());
    ASSERT_EGL_SUCCESS();

    glViewport(0, 0, static_cast<GLsizei>(bufferSize), static_cast<GLsizei>(bufferSize));
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_EQ(static_cast<GLint>(bufferSize) / 2, static_cast<GLint>(bufferSize) / 2, 255, 0,
                    255, 255);

    // Make current with fixture EGL to ensure the Surface can be released immediately.
    getEGLWindow()->makeCurrent();
    eglDestroySurface(display, pbuffer);
}

// Test creating a pbuffer with a D3D texture and depth stencil bits in the EGL config creates keeps
// its depth stencil buffer
TEST_P(D3DTextureTest, DepthStencil)
{
    if (!valid())
    {
        return;
    }

    EGLWindow *window  = getEGLWindow();
    EGLDisplay display = window->getDisplay();

    const size_t bufferSize = 32;

    EGLSurface pbuffer =
        createPBuffer(bufferSize, bufferSize, EGL_NO_TEXTURE, EGL_NO_TEXTURE, 1, 0);
    ASSERT_EGL_SUCCESS();
    ASSERT_NE(pbuffer, EGL_NO_SURFACE);

    // Apply the Pbuffer and clear it to purple and verify
    eglMakeCurrent(display, pbuffer, pbuffer, window->getContext());
    ASSERT_EGL_SUCCESS();

    glViewport(0, 0, static_cast<GLsizei>(bufferSize), static_cast<GLsizei>(bufferSize));
    glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
    glClearDepthf(0.5f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glUseProgram(mTextureProgram);
    glUniform1i(mTextureUniformLocation, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::green);

    // Draw a quad that will fail the depth test and verify that the buffer is unchanged
    drawQuad(mTextureProgram, "position", 1.0f);
    EXPECT_PIXEL_COLOR_EQ(static_cast<GLint>(bufferSize) / 2, static_cast<GLint>(bufferSize) / 2,
                          GLColor::cyan);

    // Draw a quad that will pass the depth test and verify that the buffer is green
    drawQuad(mTextureProgram, "position", -1.0f);
    EXPECT_PIXEL_COLOR_EQ(static_cast<GLint>(bufferSize) / 2, static_cast<GLint>(bufferSize) / 2,
                          GLColor::green);

    // Make current with fixture EGL to ensure the Surface can be released immediately.
    getEGLWindow()->makeCurrent();
    eglDestroySurface(display, pbuffer);
}

// Test creating a pbuffer from a d3d surface and binding it to a texture
TEST_P(D3DTextureTest, BindTexImage)
{
    if (!valid())
    {
        return;
    }

    EGLWindow *window  = getEGLWindow();
    EGLDisplay display = window->getDisplay();

    const size_t bufferSize = 32;

    EGLSurface pbuffer =
        createPBuffer(bufferSize, bufferSize, EGL_TEXTURE_RGBA, EGL_TEXTURE_2D, 1, 0);
    ASSERT_EGL_SUCCESS();
    ASSERT_NE(pbuffer, EGL_NO_SURFACE);

    // Apply the Pbuffer and clear it to purple
    eglMakeCurrent(display, pbuffer, pbuffer, window->getContext());
    ASSERT_EGL_SUCCESS();

    glViewport(0, 0, static_cast<GLsizei>(bufferSize), static_cast<GLsizei>(bufferSize));
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_EQ(static_cast<GLint>(bufferSize) / 2, static_cast<GLint>(bufferSize) / 2, 255, 0,
                    255, 255);

    // Apply the window surface
    eglMakeCurrent(display, window->getSurface(), window->getSurface(), window->getContext());

    // Create a texture and bind the Pbuffer to it
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    EXPECT_GL_NO_ERROR();

    eglBindTexImage(display, pbuffer, EGL_BACK_BUFFER);
    glViewport(0, 0, getWindowWidth(), getWindowHeight());
    ASSERT_EGL_SUCCESS();

    // Draw a quad and verify that it is purple
    glUseProgram(mTextureProgram);
    glUniform1i(mTextureUniformLocation, 0);

    drawQuad(mTextureProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();

    // Unbind the texture
    eglReleaseTexImage(display, pbuffer, EGL_BACK_BUFFER);
    ASSERT_EGL_SUCCESS();

    // Verify that purple was drawn
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 255, 0, 255, 255);

    glDeleteTextures(1, &texture);

    // Make current with fixture EGL to ensure the Surface can be released immediately.
    getEGLWindow()->makeCurrent();
    eglDestroySurface(display, pbuffer);
}

// Verify that creating a pbuffer with a multisampled texture will fail on a non-multisampled
// window.
TEST_P(D3DTextureTest, CheckSampleMismatch)
{
    if (!valid())
    {
        return;
    }

    // Multisampling is not supported on D3D9 or OpenGL.
    ANGLE_SKIP_TEST_IF(IsD3D9() || IsOpenGL());

    constexpr size_t bufferSize = 32;

    EGLSurface pbuffer = createPBuffer(bufferSize, bufferSize, EGL_NO_TEXTURE, EGL_NO_TEXTURE, 2,
                                       static_cast<UINT>(D3D11_STANDARD_MULTISAMPLE_PATTERN));
    EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
    EXPECT_EQ(pbuffer, nullptr);
}

// Tests what happens when we make a PBuffer that isn't shader-readable.
TEST_P(D3DTextureTest, NonReadablePBuffer)
{
    ANGLE_SKIP_TEST_IF(!valid() || !IsD3D11());

    constexpr size_t bufferSize = 32;

    EGLSurface pbuffer =
        createD3D11PBuffer(bufferSize, bufferSize, EGL_TEXTURE_RGBA, EGL_TEXTURE_2D, 1, 0,
                           D3D11_BIND_RENDER_TARGET, DXGI_FORMAT_R8G8B8A8_UNORM);

    ASSERT_EGL_SUCCESS();
    ASSERT_NE(pbuffer, EGL_NO_SURFACE);

    EGLWindow *window  = getEGLWindow();
    EGLDisplay display = window->getDisplay();

    eglMakeCurrent(display, pbuffer, pbuffer, window->getContext());
    ASSERT_EGL_SUCCESS();

    glViewport(0, 0, static_cast<GLsizei>(bufferSize), static_cast<GLsizei>(bufferSize));

    // Clear to green.
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Copy the green color to a texture.
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, bufferSize, bufferSize, 0);
    ASSERT_GL_NO_ERROR();

    // Clear to red.
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Draw with the texture and expect green.
    draw2DTexturedQuad(0.5f, 1.0f, false);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Make current with fixture EGL to ensure the Surface can be released immediately.
    getEGLWindow()->makeCurrent();
    eglDestroySurface(display, pbuffer);
}

class D3DTextureTestMS : public D3DTextureTest
{
  protected:
    D3DTextureTestMS() : D3DTextureTest()
    {
        setSamples(4);
        setMultisampleEnabled(true);
    }
};

// Test creating a pbuffer from a multisampled d3d surface and clearing it.
TEST_P(D3DTextureTestMS, Clear)
{
    EGLWindow *window  = getEGLWindow();
    EGLDisplay display = window->getDisplay();

    constexpr size_t bufferSize = 32;
    constexpr UINT testpoint    = bufferSize / 2;

    EGLSurface pbuffer = createPBuffer(bufferSize, bufferSize, EGL_NO_TEXTURE, EGL_NO_TEXTURE, 4,
                                       static_cast<UINT>(D3D11_STANDARD_MULTISAMPLE_PATTERN));
    ASSERT_EGL_SUCCESS();
    ASSERT_NE(pbuffer, EGL_NO_SURFACE);

    // Apply the Pbuffer and clear it to magenta and verify
    eglMakeCurrent(display, pbuffer, pbuffer, window->getContext());
    ASSERT_EGL_SUCCESS();

    glViewport(0, 0, static_cast<GLsizei>(bufferSize), static_cast<GLsizei>(bufferSize));
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(testpoint, testpoint, GLColor::magenta);

    // Make current with fixture EGL to ensure the Surface can be released immediately.
    getEGLWindow()->makeCurrent();
    eglDestroySurface(display, pbuffer);
}

// Test creating a pbuffer from a multisampled d3d surface and drawing with a program.
TEST_P(D3DTextureTestMS, DrawProgram)
{
    EGLWindow *window  = getEGLWindow();
    EGLDisplay display = window->getDisplay();

    constexpr size_t bufferSize = 32;

    EGLSurface pbuffer = createPBuffer(bufferSize, bufferSize, EGL_NO_TEXTURE, EGL_NO_TEXTURE, 4,
                                       static_cast<UINT>(D3D11_STANDARD_MULTISAMPLE_PATTERN));
    ASSERT_EGL_SUCCESS();
    ASSERT_NE(pbuffer, EGL_NO_SURFACE);

    // Apply the Pbuffer and clear it to magenta
    eglMakeCurrent(display, pbuffer, pbuffer, window->getContext());
    ASSERT_EGL_SUCCESS();

    glViewport(0, 0, static_cast<GLsizei>(bufferSize), static_cast<GLsizei>(bufferSize));
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    constexpr GLint testPoint = bufferSize / 2;
    EXPECT_PIXEL_COLOR_EQ(testPoint, testPoint, GLColor::magenta);

    // Apply the window surface
    eglMakeCurrent(display, window->getSurface(), window->getSurface(), window->getContext());
    ASSERT_EGL_SUCCESS();

    glViewport(0, 0, getWindowWidth(), getWindowHeight());
    ASSERT_EGL_SUCCESS();

    // Draw a quad and verify that it is magenta
    glUseProgram(mTextureProgramNoSampling);
    EXPECT_GL_NO_ERROR();

    drawQuad(mTextureProgramNoSampling, "position", 0.5f);
    EXPECT_GL_NO_ERROR();

    // Verify that magenta was drawn
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::magenta);

    // Make current with fixture EGL to ensure the Surface can be released immediately.
    getEGLWindow()->makeCurrent();
    eglDestroySurface(display, pbuffer);
}

// Test for failure when creating a pbuffer from a multisampled d3d surface to bind to a texture.
TEST_P(D3DTextureTestMS, BindTexture)
{
    constexpr size_t bufferSize = 32;

    EGLSurface pbuffer = createPBuffer(bufferSize, bufferSize, EGL_TEXTURE_RGBA, EGL_TEXTURE_2D, 4,
                                       static_cast<UINT>(D3D11_STANDARD_MULTISAMPLE_PATTERN));

    EXPECT_EGL_ERROR(EGL_BAD_ATTRIBUTE);
    EXPECT_EQ(pbuffer, nullptr);
}

// Verify that creating a pbuffer from a multisampled texture with a multisampled window will fail
// when the sample counts do not match.
TEST_P(D3DTextureTestMS, CheckSampleMismatch)
{
    constexpr size_t bufferSize = 32;

    EGLSurface pbuffer = createPBuffer(bufferSize, bufferSize, EGL_NO_TEXTURE, EGL_NO_TEXTURE, 2,
                                       static_cast<UINT>(D3D11_STANDARD_MULTISAMPLE_PATTERN));

    EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
    EXPECT_EQ(pbuffer, nullptr);
}

// Test creating a pbuffer with a D3D texture and depth stencil bits in the EGL config creates keeps
// its depth stencil buffer
TEST_P(D3DTextureTestMS, DepthStencil)
{
    EGLWindow *window  = getEGLWindow();
    EGLDisplay display = window->getDisplay();

    const size_t bufferSize = 32;

    EGLSurface pbuffer = createPBuffer(bufferSize, bufferSize, EGL_NO_TEXTURE, EGL_NO_TEXTURE, 4,
                                       static_cast<UINT>(D3D11_STANDARD_MULTISAMPLE_PATTERN));
    ASSERT_EGL_SUCCESS();
    ASSERT_NE(EGL_NO_SURFACE, pbuffer);

    // Apply the Pbuffer and clear it to purple and verify
    eglMakeCurrent(display, pbuffer, pbuffer, window->getContext());
    ASSERT_EGL_SUCCESS();

    glViewport(0, 0, static_cast<GLsizei>(bufferSize), static_cast<GLsizei>(bufferSize));
    glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
    glClearDepthf(0.5f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glUseProgram(mTextureProgram);
    glUniform1i(mTextureUniformLocation, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::green);

    // Draw a quad that will fail the depth test and verify that the buffer is unchanged
    drawQuad(mTextureProgram, "position", 1.0f);
    EXPECT_PIXEL_COLOR_EQ(static_cast<GLint>(bufferSize) / 2, static_cast<GLint>(bufferSize) / 2,
                          GLColor::cyan);

    // Draw a quad that will pass the depth test and verify that the buffer is green
    drawQuad(mTextureProgram, "position", -1.0f);
    EXPECT_PIXEL_COLOR_EQ(static_cast<GLint>(bufferSize) / 2, static_cast<GLint>(bufferSize) / 2,
                          GLColor::green);

    // Make current with fixture EGL to ensure the Surface can be released immediately.
    getEGLWindow()->makeCurrent();
    eglDestroySurface(display, pbuffer);
}

// Test copyTexImage2D with a multisampled resource
TEST_P(D3DTextureTestMS, CopyTexImage2DTest)
{
    EGLWindow *window  = getEGLWindow();
    EGLDisplay display = window->getDisplay();

    constexpr size_t bufferSize = 32;

    EGLSurface pbuffer = createPBuffer(bufferSize, bufferSize, EGL_NO_TEXTURE, EGL_NO_TEXTURE, 4,
                                       static_cast<UINT>(D3D11_STANDARD_MULTISAMPLE_PATTERN));
    ASSERT_EGL_SUCCESS();
    ASSERT_NE(EGL_NO_SURFACE, pbuffer);

    // Apply the Pbuffer and clear it to magenta and verify
    eglMakeCurrent(display, pbuffer, pbuffer, window->getContext());
    ASSERT_EGL_SUCCESS();

    glViewport(0, 0, static_cast<GLsizei>(bufferSize), static_cast<GLsizei>(bufferSize));
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    glUseProgram(mTextureProgram);
    glUniform1i(mTextureUniformLocation, 0);

    // Specify a 2D texture and set it to green
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::green);

    // Copy from the multisampled framebuffer to the 2D texture
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 1, 1, 0);

    // Draw a quad and verify the color is magenta, not green
    drawQuad(mTextureProgram, "position", 1.0f);
    EXPECT_PIXEL_COLOR_EQ(static_cast<GLint>(bufferSize) / 2, static_cast<GLint>(bufferSize) / 2,
                          GLColor::magenta);
    ASSERT_GL_NO_ERROR();

    // Make current with fixture EGL to ensure the Surface can be released immediately.
    getEGLWindow()->makeCurrent();
    eglDestroySurface(display, pbuffer);
}

// Test copyTexSubImage2D with a multisampled resource
TEST_P(D3DTextureTestMS, CopyTexSubImage2DTest)
{
    EGLWindow *window  = getEGLWindow();
    EGLDisplay display = window->getDisplay();

    constexpr size_t bufferSize = 32;

    EGLSurface pbuffer = createPBuffer(bufferSize, bufferSize, EGL_NO_TEXTURE, EGL_NO_TEXTURE, 4,
                                       static_cast<UINT>(D3D11_STANDARD_MULTISAMPLE_PATTERN));
    ASSERT_EGL_SUCCESS();
    ASSERT_NE(EGL_NO_SURFACE, pbuffer);

    // Apply the Pbuffer and clear it to magenta and verify
    eglMakeCurrent(display, pbuffer, pbuffer, window->getContext());
    ASSERT_EGL_SUCCESS();

    glViewport(0, 0, static_cast<GLsizei>(bufferSize), static_cast<GLsizei>(bufferSize));
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    glUseProgram(mTextureProgram);
    glUniform1i(mTextureUniformLocation, 0);

    // Specify a 2D texture and set it to green
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::green);

    // Copy from the multisampled framebuffer to the 2D texture
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, 1, 1);

    // Draw a quad and verify the color is magenta, not green
    drawQuad(mTextureProgram, "position", 1.0f);
    EXPECT_PIXEL_COLOR_EQ(static_cast<GLint>(bufferSize) / 2, static_cast<GLint>(bufferSize) / 2,
                          GLColor::magenta);
    ASSERT_GL_NO_ERROR();

    // Make current with fixture EGL to ensure the Surface can be released immediately.
    getEGLWindow()->makeCurrent();
    eglDestroySurface(display, pbuffer);
}

class D3DTextureClearTest : public D3DTextureTest
{
  protected:
    D3DTextureClearTest() : D3DTextureTest() {}

    void RunClearTest(DXGI_FORMAT format)
    {
        ANGLE_SKIP_TEST_IF(!valid() || !IsD3D11());

        EGLWindow *window  = getEGLWindow();
        EGLDisplay display = window->getDisplay();

        window->makeCurrent();

        const UINT bufferSize = 32;
        EXPECT_TRUE(mD3D11Device != nullptr);
        ID3D11Texture2D *d3d11Texture = nullptr;
        CD3D11_TEXTURE2D_DESC desc(format, bufferSize, bufferSize, 1, 1,
                                   D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET);
        EXPECT_TRUE(SUCCEEDED(mD3D11Device->CreateTexture2D(&desc, nullptr, &d3d11Texture)));

        // Can use unsized formats for all cases, but use sized ones to match Chromium.
        EGLint internalFormat = GL_NONE;
        switch (format)
        {
            case DXGI_FORMAT_R8G8B8A8_UNORM:
            case DXGI_FORMAT_R16G16B16A16_FLOAT:
                internalFormat = GL_RGBA;
                break;
            case DXGI_FORMAT_B8G8R8A8_UNORM:
                internalFormat = GL_BGRA_EXT;
                break;
            case DXGI_FORMAT_R8_UNORM:
                internalFormat = GL_RED_EXT;
                break;
            case DXGI_FORMAT_R8G8_UNORM:
                internalFormat = GL_RG_EXT;
                break;
            case DXGI_FORMAT_R10G10B10A2_UNORM:
                internalFormat = GL_RGB10_A2_EXT;
                break;
            case DXGI_FORMAT_R16_UNORM:
                internalFormat = GL_R16_EXT;
                break;
            case DXGI_FORMAT_R16G16_UNORM:
                internalFormat = GL_RG16_EXT;
                break;
            default:
                ASSERT_TRUE(false);
                break;
        }

        const EGLint attribs[] = {EGL_TEXTURE_INTERNAL_FORMAT_ANGLE, internalFormat, EGL_NONE};

        EGLImage image = eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_D3D11_TEXTURE_ANGLE,
                                           static_cast<EGLClientBuffer>(d3d11Texture), attribs);
        ASSERT_EGL_SUCCESS();
        ASSERT_NE(image, EGL_NO_IMAGE_KHR);

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        ASSERT_GL_NO_ERROR();

        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);
        ASSERT_GL_NO_ERROR();

        GLuint fbo;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        EXPECT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER),
                  static_cast<unsigned>(GL_FRAMEBUFFER_COMPLETE));
        ASSERT_GL_NO_ERROR();

        glViewport(0, 0, static_cast<GLsizei>(bufferSize), static_cast<GLsizei>(bufferSize));
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ASSERT_GL_NO_ERROR();

        if (format == DXGI_FORMAT_R16G16B16A16_FLOAT)
        {
            EXPECT_PIXEL_32F_EQ(static_cast<GLint>(bufferSize) / 2,
                                static_cast<GLint>(bufferSize) / 2, 1.0f, 1.0f, 1.0f, 1.0f);
        }
        else if (format == DXGI_FORMAT_R16_UNORM)
        {
            EXPECT_PIXEL_16_NEAR(static_cast<GLint>(bufferSize) / 2,
                                 static_cast<GLint>(bufferSize) / 2, 65535, 0, 0, 65535, 0);
        }
        else if (format == DXGI_FORMAT_R16G16_UNORM)
        {
            EXPECT_PIXEL_16_NEAR(static_cast<GLint>(bufferSize) / 2,
                                 static_cast<GLint>(bufferSize) / 2, 65535, 65535, 0, 65535, 0);
        }
        else
        {
            GLuint readColor[4] = {0, 0, 0, 255};
            switch (internalFormat)
            {
                case GL_RGBA:
                case GL_BGRA_EXT:
                case GL_RGB10_A2_EXT:
                    readColor[0] = readColor[1] = readColor[2] = 255;
                    break;
                case GL_RG_EXT:
                    readColor[0] = readColor[1] = 255;
                    break;
                case GL_RED_EXT:
                    readColor[0] = 255;
                    break;
            }
            // Read back as GL_UNSIGNED_BYTE even though the texture might have more than 8bpc.
            EXPECT_PIXEL_EQ(static_cast<GLint>(bufferSize) / 2, static_cast<GLint>(bufferSize) / 2,
                            readColor[0], readColor[1], readColor[2], readColor[3]);
        }

        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(1, &texture);
        eglDestroyImageKHR(display, image);

        d3d11Texture->Release();
    }
};

TEST_P(D3DTextureClearTest, ClearRGBA8)
{
    RunClearTest(DXGI_FORMAT_R8G8B8A8_UNORM);
}

TEST_P(D3DTextureClearTest, ClearBGRA8)
{
    RunClearTest(DXGI_FORMAT_B8G8R8A8_UNORM);
}

TEST_P(D3DTextureClearTest, ClearR8)
{
    RunClearTest(DXGI_FORMAT_R8_UNORM);
}

TEST_P(D3DTextureClearTest, ClearRG8)
{
    RunClearTest(DXGI_FORMAT_R8G8_UNORM);
}

TEST_P(D3DTextureClearTest, ClearRGB10A2)
{
    RunClearTest(DXGI_FORMAT_R10G10B10A2_UNORM);
}

TEST_P(D3DTextureClearTest, ClearRGBAF16)
{
    RunClearTest(DXGI_FORMAT_R16G16B16A16_FLOAT);
}

TEST_P(D3DTextureClearTest, ClearR16)
{
    RunClearTest(DXGI_FORMAT_R16_UNORM);
}

TEST_P(D3DTextureClearTest, ClearRG16)
{
    RunClearTest(DXGI_FORMAT_R16G16_UNORM);
}

TEST_P(D3DTextureTest, NonRenderableTextureImage)
{
    ANGLE_SKIP_TEST_IF(!valid() || !IsD3D11());

    EGLWindow *window  = getEGLWindow();
    EGLDisplay display = window->getDisplay();

    window->makeCurrent();

    const UINT bufferSize = 32;
    EXPECT_TRUE(mD3D11Device != nullptr);
    ID3D11Texture2D *d3d11Texture = nullptr;
    CD3D11_TEXTURE2D_DESC desc(DXGI_FORMAT_R8G8B8A8_UNORM, bufferSize, bufferSize, 1, 1,
                               D3D11_BIND_SHADER_RESOURCE);
    EXPECT_TRUE(SUCCEEDED(mD3D11Device->CreateTexture2D(&desc, nullptr, &d3d11Texture)));

    const EGLint attribs[] = {EGL_NONE};

    EGLImage image = eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_D3D11_TEXTURE_ANGLE,
                                       static_cast<EGLClientBuffer>(d3d11Texture), attribs);
    ASSERT_EGL_SUCCESS();
    ASSERT_NE(image, EGL_NO_IMAGE_KHR);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    ASSERT_GL_NO_ERROR();

    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);
    ASSERT_GL_NO_ERROR();

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER),
              static_cast<unsigned>(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT));
    ASSERT_GL_NO_ERROR();

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &texture);
    eglDestroyImageKHR(display, image);

    d3d11Texture->Release();
}

TEST_P(D3DTextureTest, RGBEmulationTextureImage)
{
    ANGLE_SKIP_TEST_IF(!valid() || !IsD3D11());

    EGLWindow *window  = getEGLWindow();
    EGLDisplay display = window->getDisplay();

    window->makeCurrent();

    const UINT bufferSize = 32;
    EXPECT_TRUE(mD3D11Device != nullptr);
    ID3D11Texture2D *d3d11Texture = nullptr;
    CD3D11_TEXTURE2D_DESC desc(DXGI_FORMAT_R8G8B8A8_UNORM, bufferSize, bufferSize, 1, 1,
                               D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET);
    EXPECT_TRUE(SUCCEEDED(mD3D11Device->CreateTexture2D(&desc, nullptr, &d3d11Texture)));

    const EGLint attribs[] = {EGL_TEXTURE_INTERNAL_FORMAT_ANGLE, GL_RGB, EGL_NONE};

    EGLImage image = eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_D3D11_TEXTURE_ANGLE,
                                       static_cast<EGLClientBuffer>(d3d11Texture), attribs);
    ASSERT_EGL_SUCCESS();
    ASSERT_NE(image, EGL_NO_IMAGE_KHR);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    ASSERT_GL_NO_ERROR();

    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);
    ASSERT_GL_NO_ERROR();

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER),
              static_cast<unsigned>(GL_FRAMEBUFFER_COMPLETE));
    ASSERT_GL_NO_ERROR();

    // Although we are writing 0.5 to the alpha channel it should have the same
    // side effects as if alpha were 1.0.
    glViewport(0, 0, static_cast<GLsizei>(bufferSize), static_cast<GLsizei>(bufferSize));
    glClearColor(1.0f, 0.0f, 1.0f, 0.5f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(static_cast<GLint>(bufferSize) / 2, static_cast<GLint>(bufferSize) / 2, 255, 0,
                    255, 255);

    GLuint rgbaRbo;
    glGenRenderbuffers(1, &rgbaRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rgbaRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8_OES, bufferSize, bufferSize);

    GLuint rgbaFbo;
    glGenFramebuffers(1, &rgbaFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, rgbaFbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rgbaRbo);
    EXPECT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER),
              static_cast<unsigned>(GL_FRAMEBUFFER_COMPLETE));
    ASSERT_GL_NO_ERROR();

    // BlitFramebuffer from/to RGBA framebuffer fails.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, rgbaFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glBlitFramebufferANGLE(0, 0, bufferSize, bufferSize, 0, 0, bufferSize, bufferSize,
                           GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rgbaFbo);
    glBlitFramebufferANGLE(0, 0, bufferSize, bufferSize, 0, 0, bufferSize, bufferSize,
                           GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);
    ASSERT_GL_NO_ERROR();

    GLuint rgbRbo;
    glGenRenderbuffers(1, &rgbRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rgbRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB8_OES, bufferSize, bufferSize);

    GLuint rgbFbo;
    glGenFramebuffers(1, &rgbFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, rgbFbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rgbRbo);
    EXPECT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER),
              static_cast<unsigned>(GL_FRAMEBUFFER_COMPLETE));
    ASSERT_GL_NO_ERROR();
    glClearColor(1.0f, 0.0f, 1.0f, 0.5f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    // Clear texture framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(static_cast<GLint>(bufferSize) / 2, static_cast<GLint>(bufferSize) / 2, 0, 0, 0,
                    255);

    // BlitFramebuffer from/to RGB framebuffer succeeds.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, rgbFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glBlitFramebufferANGLE(0, 0, bufferSize, bufferSize, 0, 0, bufferSize, bufferSize,
                           GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(static_cast<GLint>(bufferSize) / 2, static_cast<GLint>(bufferSize) / 2, 255, 0,
                    255, 255);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rgbFbo);
    glBlitFramebufferANGLE(0, 0, bufferSize, bufferSize, 0, 0, bufferSize, bufferSize,
                           GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();
    glBindFramebuffer(GL_FRAMEBUFFER, rgbFbo);
    EXPECT_PIXEL_EQ(static_cast<GLint>(bufferSize) / 2, static_cast<GLint>(bufferSize) / 2, 0, 0, 0,
                    255);

    glDeleteFramebuffers(1, &rgbFbo);
    glDeleteRenderbuffers(1, &rgbRbo);
    glDeleteFramebuffers(1, &rgbaFbo);
    glDeleteRenderbuffers(1, &rgbaRbo);
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &texture);
    eglDestroyImageKHR(display, image);

    d3d11Texture->Release();
}

TEST_P(D3DTextureTest, TextureArray)
{
    ANGLE_SKIP_TEST_IF(!valid() || !IsD3D11());

    EGLWindow *window  = getEGLWindow();
    EGLDisplay display = window->getDisplay();

    window->makeCurrent();

    const UINT bufferSize = 32;
    const UINT arraySize  = 4;

    ID3D11Texture2D *d3d11Texture = nullptr;
    CD3D11_TEXTURE2D_DESC desc(DXGI_FORMAT_R8G8B8A8_UNORM, bufferSize, bufferSize, arraySize, 1,
                               D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET);
    EXPECT_TRUE(SUCCEEDED(mD3D11Device->CreateTexture2D(&desc, nullptr, &d3d11Texture)));

    const unsigned char kRFill = 0x12;
    const unsigned char kGFill = 0x23;
    const unsigned char kBFill = 0x34;
    const unsigned char kAFill = 0x45;

    std::vector<unsigned char> imageData(bufferSize * bufferSize * 4, 0);
    for (size_t i = 0; i < imageData.size(); i += 4)
    {
        imageData[i]     = kRFill;
        imageData[i + 1] = kGFill;
        imageData[i + 2] = kBFill;
        imageData[i + 3] = kAFill;
    }

    ID3D11DeviceContext *context = nullptr;
    mD3D11Device->GetImmediateContext(&context);
    ASSERT_NE(context, nullptr);

    D3D11_BOX dstBox = {0, 0, 0, bufferSize, bufferSize, 1};
    context->UpdateSubresource(d3d11Texture, arraySize - 1, &dstBox, imageData.data(),
                               bufferSize * 4, imageData.size());

    const EGLint attribs[] = {EGL_D3D11_TEXTURE_ARRAY_SLICE_ANGLE, arraySize - 1, EGL_NONE};
    EGLImage image         = eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_D3D11_TEXTURE_ANGLE,
                                               static_cast<EGLClientBuffer>(d3d11Texture), attribs);
    ASSERT_EGL_SUCCESS();
    ASSERT_NE(image, EGL_NO_IMAGE_KHR);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    ASSERT_GL_NO_ERROR();

    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);
    ASSERT_GL_NO_ERROR();

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER),
              static_cast<unsigned>(GL_FRAMEBUFFER_COMPLETE));
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_EQ(static_cast<GLint>(bufferSize) / 2, static_cast<GLint>(bufferSize) / 2, kRFill,
                    kGFill, kBFill, kAFill);

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &texture);
    eglDestroyImageKHR(display, image);

    d3d11Texture->Release();
}

class D3DTextureYUVTest : public D3DTextureTest
{
  protected:
    void CreateAndBindImageToTexture(EGLDisplay display,
                                     ID3D11Texture2D *d3d11Texture,
                                     EGLint plane,
                                     GLenum internalFormat,
                                     GLenum target,
                                     EGLImage *image,
                                     GLuint *texture)
    {
        const EGLint attribs[] = {EGL_TEXTURE_INTERNAL_FORMAT_ANGLE,
                                  static_cast<EGLint>(internalFormat),
                                  EGL_D3D11_TEXTURE_PLANE_ANGLE, plane, EGL_NONE};
        *image                 = eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_D3D11_TEXTURE_ANGLE,
                                                   static_cast<EGLClientBuffer>(d3d11Texture), attribs);
        ASSERT_EGL_SUCCESS();
        ASSERT_NE(*image, EGL_NO_IMAGE_KHR);

        // Create and bind Y plane texture to image.
        glGenTextures(1, texture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(target, *texture);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        ASSERT_GL_NO_ERROR();

        glEGLImageTargetTexture2DOES(target, *image);
        ASSERT_GL_NO_ERROR();
    }

    void RunYUVSamplerTest(DXGI_FORMAT format)
    {
        ASSERT_TRUE(format == DXGI_FORMAT_NV12 || format == DXGI_FORMAT_P010 ||
                    format == DXGI_FORMAT_P016);
        UINT formatSupport;
        ANGLE_SKIP_TEST_IF(!valid() || !IsD3D11() ||
                           FAILED(mD3D11Device->CheckFormatSupport(format, &formatSupport)));
        ASSERT_TRUE(formatSupport &
                    (D3D11_FORMAT_SUPPORT_TEXTURE2D | D3D11_FORMAT_SUPPORT_SHADER_SAMPLE));

        const bool isNV12          = (format == DXGI_FORMAT_NV12);
        const unsigned kYFillValue = isNV12 ? 0x12 : 0x1234;
        const unsigned kUFillValue = isNV12 ? 0x23 : 0x2345;
        const unsigned kVFillValue = isNV12 ? 0x34 : 0x3456;

        constexpr char kVS[] =
            R"(precision highp float;
            attribute vec4 position;
            varying vec2 texcoord;

            void main()
            {
                gl_Position = position;
                texcoord = (position.xy * 0.5) + 0.5;
                texcoord.y = 1.0 - texcoord.y;
            })";

        constexpr char kFS[] =
            R"(#extension GL_OES_EGL_image_external : require
            precision highp float;
            uniform samplerExternalOES tex;
            varying vec2 texcoord;

            void main()
            {
                gl_FragColor = texture2D(tex, texcoord);
            })";

        GLuint program = CompileProgram(kVS, kFS);
        ASSERT_NE(0u, program) << "shader compilation failed.";

        GLint textureLocation = glGetUniformLocation(program, "tex");
        ASSERT_NE(-1, textureLocation);

        EGLWindow *window  = getEGLWindow();
        EGLDisplay display = window->getDisplay();

        window->makeCurrent();

        const UINT bufferSize = 32;
        EXPECT_TRUE(mD3D11Device != nullptr);
        Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d11Texture;
        CD3D11_TEXTURE2D_DESC desc(format, bufferSize, bufferSize, 1, 1,
                                   D3D11_BIND_SHADER_RESOURCE);

        std::vector<unsigned char> imageData;

        if (isNV12)
        {
            imageData.resize(bufferSize * bufferSize * 3 / 2);
            memset(imageData.data(), kYFillValue, bufferSize * bufferSize);
            const size_t kUVOffset = bufferSize * bufferSize;
            for (size_t i = 0; i < bufferSize * bufferSize / 2; i += 2)
            {
                imageData[kUVOffset + i]     = kUFillValue;
                imageData[kUVOffset + i + 1] = kVFillValue;
            }
        }
        else
        {
            imageData.resize(bufferSize * bufferSize * 3);
            const size_t kUVOffset = bufferSize * bufferSize * 2;
            for (size_t i = 0; i < kUVOffset; i += 2)
            {
                imageData[i]     = kYFillValue & 0xff;
                imageData[i + 1] = (kYFillValue >> 8) & 0xff;
                if (kUVOffset + i < imageData.size())
                {
                    // Interleave U & V samples.
                    const unsigned fill          = (i % 4 == 0) ? kUFillValue : kVFillValue;
                    imageData[kUVOffset + i]     = fill & 0xff;
                    imageData[kUVOffset + i + 1] = (fill >> 8) & 0xff;
                }
            }
        }

        D3D11_SUBRESOURCE_DATA data = {};
        data.pSysMem                = static_cast<const void *>(imageData.data());
        data.SysMemPitch            = isNV12 ? bufferSize : bufferSize * 2;

        EXPECT_TRUE(SUCCEEDED(mD3D11Device->CreateTexture2D(&desc, &data, &d3d11Texture)));

        // Create and bind Y plane texture to image.
        EGLImage yImage;
        GLuint yTexture;

        GLenum internalFormat = format == DXGI_FORMAT_NV12 ? GL_RED_EXT : GL_R16_EXT;
        CreateAndBindImageToTexture(display, d3d11Texture.Get(), 0, internalFormat,
                                    GL_TEXTURE_EXTERNAL_OES, &yImage, &yTexture);

        GLuint rbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, isNV12 ? GL_RGBA8_OES : GL_RGBA16_EXT, bufferSize,
                              bufferSize);
        ASSERT_GL_NO_ERROR();

        GLuint fbo;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
        EXPECT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER),
                  static_cast<unsigned>(GL_FRAMEBUFFER_COMPLETE));
        ASSERT_GL_NO_ERROR();

        // Draw the Y plane using a shader.
        glUseProgram(program);
        glUniform1i(textureLocation, 0);
        ASSERT_GL_NO_ERROR();

        glViewport(0, 0, bufferSize, bufferSize);
        drawQuad(program, "position", 1.0f);
        ASSERT_GL_NO_ERROR();

        if (isNV12)
        {

            EXPECT_PIXEL_EQ(static_cast<GLint>(bufferSize) / 2, static_cast<GLint>(bufferSize) / 2,
                            kYFillValue, 0, 0, 0xff);
        }
        else
        {
            EXPECT_PIXEL_16_NEAR(static_cast<GLint>(bufferSize) / 2,
                                 static_cast<GLint>(bufferSize) / 2, kYFillValue, 0, 0, 0xffff, 0);
        }
        ASSERT_GL_NO_ERROR();

        // Create and bind UV plane texture to image.
        EGLImage uvImage;
        GLuint uvTexture;

        internalFormat = format == DXGI_FORMAT_NV12 ? GL_RG_EXT : GL_RG16_EXT;
        CreateAndBindImageToTexture(display, d3d11Texture.Get(), 1, internalFormat,
                                    GL_TEXTURE_EXTERNAL_OES, &uvImage, &uvTexture);

        // Draw the UV plane using a shader.
        glUseProgram(program);
        glUniform1i(textureLocation, 0);
        ASSERT_GL_NO_ERROR();

        // Use only half of the framebuffer to match UV plane dimensions.
        glViewport(0, 0, bufferSize / 2, bufferSize / 2);
        drawQuad(program, "position", 1.0f);
        ASSERT_GL_NO_ERROR();

        if (isNV12)
        {

            EXPECT_PIXEL_EQ(static_cast<GLint>(bufferSize) / 4, static_cast<GLint>(bufferSize) / 4,
                            kUFillValue, kVFillValue, 0, 0xff);
        }
        else
        {
            EXPECT_PIXEL_16_NEAR(static_cast<GLint>(bufferSize) / 4,
                                 static_cast<GLint>(bufferSize) / 4, kUFillValue, kVFillValue, 0,
                                 0xffff, 0);
        }
        ASSERT_GL_NO_ERROR();

        glDeleteProgram(program);
        glDeleteTextures(1, &yTexture);
        glDeleteTextures(1, &uvTexture);
        glDeleteFramebuffers(1, &fbo);
        glDeleteRenderbuffers(1, &rbo);
        eglDestroyImageKHR(display, yImage);
        eglDestroyImageKHR(display, uvImage);
    }

    void RunYUVRenderTest(DXGI_FORMAT format)
    {
        ASSERT(format == DXGI_FORMAT_NV12 || format == DXGI_FORMAT_P010 ||
               format == DXGI_FORMAT_P016);
        UINT formatSupport;
        ANGLE_SKIP_TEST_IF(!valid() || !IsD3D11() ||
                           FAILED(mD3D11Device->CheckFormatSupport(format, &formatSupport)));
        ASSERT_TRUE(formatSupport &
                    (D3D11_FORMAT_SUPPORT_TEXTURE2D | D3D11_FORMAT_SUPPORT_RENDER_TARGET));

        const bool isNV12          = (format == DXGI_FORMAT_NV12);
        const unsigned kYFillValue = isNV12 ? 0x12 : 0x1234;
        const unsigned kUFillValue = isNV12 ? 0x23 : 0x2345;
        const unsigned kVFillValue = isNV12 ? 0x34 : 0x3456;

        constexpr char kVS[] =
            R"(precision highp float;
            attribute vec4 position;
            varying vec2 texcoord;

            void main()
            {
                gl_Position = position;
                texcoord = (position.xy * 0.5) + 0.5;
                texcoord.y = 1.0 - texcoord.y;
            })";

        constexpr char kFS[] =
            R"(precision highp float;
            uniform vec4 color;
            varying vec2 texcoord;

            void main()
            {
                gl_FragColor = color;
            })";

        GLuint program = CompileProgram(kVS, kFS);
        ASSERT_NE(0u, program) << "shader compilation failed.";

        GLint colorLocation = glGetUniformLocation(program, "color");
        ASSERT_NE(-1, colorLocation);

        EGLWindow *window  = getEGLWindow();
        EGLDisplay display = window->getDisplay();

        window->makeCurrent();

        const UINT bufferSize = 32;
        EXPECT_TRUE(mD3D11Device != nullptr);
        Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d11Texture;
        CD3D11_TEXTURE2D_DESC desc(format, bufferSize, bufferSize, 1, 1,
                                   D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET);

        EXPECT_TRUE(SUCCEEDED(mD3D11Device->CreateTexture2D(&desc, nullptr, &d3d11Texture)));

        // Create and bind Y plane texture to image.
        EGLImage yImage;
        GLuint yTexture;

        GLenum internalFormat = format == DXGI_FORMAT_NV12 ? GL_RED_EXT : GL_R16_EXT;
        CreateAndBindImageToTexture(display, d3d11Texture.Get(), 0, internalFormat, GL_TEXTURE_2D,
                                    &yImage, &yTexture);

        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, yImage);
        ASSERT_GL_NO_ERROR();

        GLuint fbo;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, yTexture, 0);
        EXPECT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER),
                  static_cast<unsigned>(GL_FRAMEBUFFER_COMPLETE));
        ASSERT_GL_NO_ERROR();

        // Draw the Y plane using a shader.
        glUseProgram(program);
        glUniform4f(colorLocation, kYFillValue * 1.0f / (isNV12 ? 0xff : 0xffff), 0, 0, 0);
        ASSERT_GL_NO_ERROR();

        drawQuad(program, "position", 1.0f);
        ASSERT_GL_NO_ERROR();

        // Create and bind UV plane texture to image.
        EGLImage uvImage;
        GLuint uvTexture;

        internalFormat = format == DXGI_FORMAT_NV12 ? GL_RG_EXT : GL_RG16_EXT;
        CreateAndBindImageToTexture(display, d3d11Texture.Get(), 1, internalFormat, GL_TEXTURE_2D,
                                    &uvImage, &uvTexture);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, uvTexture, 0);
        EXPECT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER),
                  static_cast<unsigned>(GL_FRAMEBUFFER_COMPLETE));
        ASSERT_GL_NO_ERROR();

        // Draw the UV plane using a shader.
        glUseProgram(program);
        glUniform4f(colorLocation, kUFillValue * 1.0f / (isNV12 ? 0xff : 0xffff),
                    kVFillValue * 1.0f / (isNV12 ? 0xff : 0xffff), 0, 0);
        ASSERT_GL_NO_ERROR();

        drawQuad(program, "position", 1.0f);
        ASSERT_GL_NO_ERROR();

        Microsoft::WRL::ComPtr<ID3D11Texture2D> stagingTexture;
        CD3D11_TEXTURE2D_DESC stagingDesc = desc;
        stagingDesc.BindFlags             = 0;
        stagingDesc.Usage                 = D3D11_USAGE_STAGING;
        stagingDesc.CPUAccessFlags        = D3D11_CPU_ACCESS_READ;

        EXPECT_TRUE(
            SUCCEEDED(mD3D11Device->CreateTexture2D(&stagingDesc, nullptr, &stagingTexture)));

        Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
        mD3D11Device->GetImmediateContext(&context);

        context->CopyResource(stagingTexture.Get(), d3d11Texture.Get());
        ASSERT_GL_NO_ERROR();

        D3D11_MAPPED_SUBRESOURCE mapped = {};
        EXPECT_TRUE(SUCCEEDED(context->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mapped)));

        uint8_t *yPlane  = reinterpret_cast<uint8_t *>(mapped.pData);
        uint8_t *uvPlane = yPlane + bufferSize * mapped.RowPitch;
        if (isNV12)
        {
            EXPECT_EQ(yPlane[mapped.RowPitch * bufferSize / 2], kYFillValue);
            EXPECT_EQ(uvPlane[mapped.RowPitch * bufferSize / 4], kUFillValue);
            EXPECT_EQ(uvPlane[mapped.RowPitch * bufferSize / 4 + 1], kVFillValue);
        }
        else
        {
            EXPECT_EQ(yPlane[mapped.RowPitch * bufferSize / 2], kYFillValue & 0xff);
            EXPECT_EQ(yPlane[mapped.RowPitch * bufferSize / 2 + 1], (kYFillValue >> 8) & 0xff);
            EXPECT_EQ(uvPlane[mapped.RowPitch * bufferSize / 4], kUFillValue & 0xff);
            EXPECT_EQ(uvPlane[mapped.RowPitch * bufferSize / 4 + 1], (kUFillValue >> 8) & 0xff);
            EXPECT_EQ(uvPlane[mapped.RowPitch * bufferSize / 4 + 2], kVFillValue & 0xff);
            EXPECT_EQ(uvPlane[mapped.RowPitch * bufferSize / 4 + 3], (kVFillValue >> 8) & 0xff);
        }

        context->Unmap(stagingTexture.Get(), 0);

        glDeleteProgram(program);
        glDeleteTextures(1, &yTexture);
        glDeleteTextures(1, &uvTexture);
        glDeleteFramebuffers(1, &fbo);
        eglDestroyImageKHR(display, yImage);
        eglDestroyImageKHR(display, uvImage);
    }

    void RunYUVReadPixelTest(DXGI_FORMAT format)
    {
        ASSERT(format == DXGI_FORMAT_NV12 || format == DXGI_FORMAT_P010 ||
               format == DXGI_FORMAT_P016);
        UINT formatSupport;
        ANGLE_SKIP_TEST_IF(!valid() || !IsD3D11() ||
                           FAILED(mD3D11Device->CheckFormatSupport(format, &formatSupport)));
        ASSERT_TRUE(formatSupport &
                    (D3D11_FORMAT_SUPPORT_TEXTURE2D | D3D11_FORMAT_SUPPORT_RENDER_TARGET));

        const bool isNV12          = (format == DXGI_FORMAT_NV12);
        const unsigned kYFillValue = isNV12 ? 0x12 : 0x1234;
        const unsigned kUFillValue = isNV12 ? 0x23 : 0x2345;
        const unsigned kVFillValue = isNV12 ? 0x34 : 0x3456;

        constexpr char kVS[] =
            R"(precision highp float;
            attribute vec4 position;
            varying vec2 texcoord;

            void main()
            {
                gl_Position = position;
                texcoord = (position.xy * 0.5) + 0.5;
                texcoord.y = 1.0 - texcoord.y;
            })";

        constexpr char kFS[] =
            R"(precision highp float;
            uniform vec4 color;
            varying vec2 texcoord;

            void main()
            {
                gl_FragColor = color;
            })";

        GLuint program = CompileProgram(kVS, kFS);
        ASSERT_NE(0u, program) << "shader compilation failed.";

        GLint colorLocation = glGetUniformLocation(program, "color");
        ASSERT_NE(-1, colorLocation);

        EGLWindow *window  = getEGLWindow();
        EGLDisplay display = window->getDisplay();

        window->makeCurrent();

        const UINT bufferSize = 32;
        EXPECT_TRUE(mD3D11Device != nullptr);
        Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d11Texture;
        CD3D11_TEXTURE2D_DESC desc(format, bufferSize, bufferSize, 1, 1,
                                   D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET);

        EXPECT_TRUE(SUCCEEDED(mD3D11Device->CreateTexture2D(&desc, nullptr, &d3d11Texture)));

        // Create and bind Y plane texture to image.
        EGLImage yImage;
        GLuint yTexture;

        GLenum internalFormat = format == DXGI_FORMAT_NV12 ? GL_RED_EXT : GL_R16_EXT;
        CreateAndBindImageToTexture(display, d3d11Texture.Get(), 0, internalFormat, GL_TEXTURE_2D,
                                    &yImage, &yTexture);

        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, yImage);
        ASSERT_GL_NO_ERROR();

        GLuint fbo;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, yTexture, 0);
        EXPECT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER),
                  static_cast<unsigned>(GL_FRAMEBUFFER_COMPLETE));
        ASSERT_GL_NO_ERROR();

        // Draw the Y plane using a shader.
        glUseProgram(program);
        glUniform4f(colorLocation, kYFillValue * 1.0f / (isNV12 ? 0xff : 0xffff), 0, 0, 0);
        ASSERT_GL_NO_ERROR();

        drawQuad(program, "position", 1.0f);
        ASSERT_GL_NO_ERROR();

        // Read the Y plane pixels.
        if (isNV12)
        {
            GLubyte yPixels[4] = {};
            glReadPixels(0, bufferSize / 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, yPixels);
            EXPECT_EQ(yPixels[0], kYFillValue);
        }
        else
        {
            GLushort yPixels[4] = {};
            glReadPixels(0, bufferSize / 2, 1, 1, GL_RGBA, GL_UNSIGNED_SHORT, yPixels);
            EXPECT_EQ(yPixels[0], kYFillValue);
        }

        // Create and bind UV plane texture to image.
        EGLImage uvImage;
        GLuint uvTexture;

        internalFormat = format == DXGI_FORMAT_NV12 ? GL_RG_EXT : GL_RG16_EXT;
        CreateAndBindImageToTexture(display, d3d11Texture.Get(), 1, internalFormat, GL_TEXTURE_2D,
                                    &uvImage, &uvTexture);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, uvTexture, 0);
        EXPECT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER),
                  static_cast<unsigned>(GL_FRAMEBUFFER_COMPLETE));
        ASSERT_GL_NO_ERROR();

        // Draw the UV plane using a shader.
        glUseProgram(program);
        glUniform4f(colorLocation, kUFillValue * 1.0f / (isNV12 ? 0xff : 0xffff),
                    kVFillValue * 1.0f / (isNV12 ? 0xff : 0xffff), 0, 0);
        ASSERT_GL_NO_ERROR();

        drawQuad(program, "position", 1.0f);
        ASSERT_GL_NO_ERROR();

        // Read the UV plane pixels.
        if (isNV12)
        {
            GLubyte uvPixels[4] = {};
            glReadPixels(0, bufferSize / 4, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, uvPixels);
            EXPECT_EQ(uvPixels[0], kUFillValue);
            EXPECT_EQ(uvPixels[1], kVFillValue);
        }
        else
        {
            GLushort uvPixels[4] = {};
            glReadPixels(0, bufferSize / 4, 1, 1, GL_RGBA, GL_UNSIGNED_SHORT, uvPixels);
            EXPECT_EQ(uvPixels[0], kUFillValue);
            EXPECT_EQ(uvPixels[1], kVFillValue);
        }

        Microsoft::WRL::ComPtr<ID3D11Texture2D> stagingTexture;
        CD3D11_TEXTURE2D_DESC stagingDesc = desc;
        stagingDesc.BindFlags             = 0;
        stagingDesc.Usage                 = D3D11_USAGE_STAGING;
        stagingDesc.CPUAccessFlags        = D3D11_CPU_ACCESS_READ;

        EXPECT_TRUE(
            SUCCEEDED(mD3D11Device->CreateTexture2D(&stagingDesc, nullptr, &stagingTexture)));

        Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
        mD3D11Device->GetImmediateContext(&context);

        context->CopyResource(stagingTexture.Get(), d3d11Texture.Get());
        ASSERT_GL_NO_ERROR();

        D3D11_MAPPED_SUBRESOURCE mapped = {};
        EXPECT_TRUE(SUCCEEDED(context->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mapped)));

        uint8_t *yPlane  = reinterpret_cast<uint8_t *>(mapped.pData);
        uint8_t *uvPlane = yPlane + bufferSize * mapped.RowPitch;
        if (isNV12)
        {
            EXPECT_EQ(yPlane[mapped.RowPitch * bufferSize / 2], kYFillValue);
            EXPECT_EQ(uvPlane[mapped.RowPitch * bufferSize / 4], kUFillValue);
            EXPECT_EQ(uvPlane[mapped.RowPitch * bufferSize / 4 + 1], kVFillValue);
        }
        else
        {
            EXPECT_EQ(yPlane[mapped.RowPitch * bufferSize / 2], kYFillValue & 0xff);
            EXPECT_EQ(yPlane[mapped.RowPitch * bufferSize / 2 + 1], (kYFillValue >> 8) & 0xff);
            EXPECT_EQ(uvPlane[mapped.RowPitch * bufferSize / 4], kUFillValue & 0xff);
            EXPECT_EQ(uvPlane[mapped.RowPitch * bufferSize / 4 + 1], (kUFillValue >> 8) & 0xff);
            EXPECT_EQ(uvPlane[mapped.RowPitch * bufferSize / 4 + 2], kVFillValue & 0xff);
            EXPECT_EQ(uvPlane[mapped.RowPitch * bufferSize / 4 + 3], (kVFillValue >> 8) & 0xff);
        }

        context->Unmap(stagingTexture.Get(), 0);

        glDeleteProgram(program);
        glDeleteTextures(1, &yTexture);
        glDeleteTextures(1, &uvTexture);
        glDeleteFramebuffers(1, &fbo);
        eglDestroyImageKHR(display, yImage);
        eglDestroyImageKHR(display, uvImage);
    }

    template <typename T>
    void RunYUVWritePixelTest(DXGI_FORMAT format)
    {
        ASSERT(format == DXGI_FORMAT_NV12 || format == DXGI_FORMAT_P010 ||
               format == DXGI_FORMAT_P016);
        UINT formatSupport;
        ANGLE_SKIP_TEST_IF(!valid() || !IsD3D11() ||
                           FAILED(mD3D11Device->CheckFormatSupport(format, &formatSupport)));
        ASSERT_TRUE(formatSupport &
                    (D3D11_FORMAT_SUPPORT_TEXTURE2D | D3D11_FORMAT_SUPPORT_RENDER_TARGET));

        const bool isNV12                = (format == DXGI_FORMAT_NV12);
        const unsigned kYFillValueFull   = isNV12 ? 0x12 : 0x1234;
        const unsigned kUFillValueFull   = isNV12 ? 0x23 : 0x2345;
        const unsigned kVFillValueFull   = isNV12 ? 0x34 : 0x3456;
        const unsigned kYFillValueOffset = isNV12 ? 0x56 : 0x5678;
        const unsigned kUFillValueOffset = isNV12 ? 0x67 : 0x6789;
        const unsigned kVFillValueOffset = isNV12 ? 0x78 : 0x7890;

        EGLWindow *window  = getEGLWindow();
        EGLDisplay display = window->getDisplay();
        window->makeCurrent();

        const UINT bufferSize = 32;
        EXPECT_TRUE(mD3D11Device != nullptr);
        Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d11Texture;
        CD3D11_TEXTURE2D_DESC desc(format, bufferSize, bufferSize, 1, 1,
                                   D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET);

        EXPECT_TRUE(SUCCEEDED(mD3D11Device->CreateTexture2D(&desc, nullptr, &d3d11Texture)));

        // Create and bind Y plane texture to image.
        EGLImage yImage;
        GLuint yTexture;
        GLenum internalFormat = isNV12 ? GL_RED_EXT : GL_R16_EXT;
        CreateAndBindImageToTexture(display, d3d11Texture.Get(), 0, internalFormat, GL_TEXTURE_2D,
                                    &yImage, &yTexture);
        ASSERT_GL_NO_ERROR();

        // Write the Y plane data to full texture (0, 0) to (32, 32).
        GLenum type = isNV12 ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT;
        std::vector<T> yData(bufferSize * bufferSize, kYFillValueFull);
        glTexSubImage2D(GL_TEXTURE_2D, /*level=*/0, /*xoffset=*/0, /*yoffset=*/0,
                        /*width=*/bufferSize,
                        /*height=*/bufferSize, GL_RED_EXT, type, yData.data());
        ASSERT_GL_NO_ERROR();

        GLuint fbo;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, yTexture, 0);
        EXPECT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER),
                  static_cast<unsigned>(GL_FRAMEBUFFER_COMPLETE));
        ASSERT_GL_NO_ERROR();

        // Read the Y plane pixels for a region starting at 0, 0 offsets.
        // yPixels of size (4*4) region (*4) bytes per RGBA pixel
        T yPixels[4 * 4 * 4] = {};
        glReadPixels(/*x=*/0, /*y=*/0, /*width*/ 4, /*height=*/4, GL_RGBA, type, yPixels);
        EXPECT_EQ(yPixels[0], kYFillValueFull);
        EXPECT_EQ(yPixels[4], kYFillValueFull);
        EXPECT_EQ(yPixels[16], kYFillValueFull);

        // Write the Y plane data with offseted values for subregion (16, 16) - (32, 32).
        std::vector<T> yDataOffset(bufferSize * bufferSize, kYFillValueOffset);
        glTexSubImage2D(GL_TEXTURE_2D, /*level=*/0, /*xoffset=*/bufferSize / 2,
                        /*yoffset=*/bufferSize / 2, /*width=*/bufferSize / 2,
                        /*height=*/bufferSize / 2, GL_RED_EXT, type, yDataOffset.data());
        ASSERT_GL_NO_ERROR();

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, yTexture, 0);
        EXPECT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER),
                  static_cast<unsigned>(GL_FRAMEBUFFER_COMPLETE));
        ASSERT_GL_NO_ERROR();

        // Read the Y plane pixels for a region starting at some offsets.
        // yPixels of size (16*16) region (*4) bytes per RGBA pixel
        T yPixelsOffset[16 * 16 * 4] = {};
        glReadPixels(/*x=*/bufferSize / 2, /*y=*/bufferSize / 2, /*width=*/bufferSize / 2,
                     /*height=*/bufferSize / 2, GL_RGBA, type, yPixelsOffset);
        EXPECT_EQ(yPixelsOffset[0], kYFillValueOffset);
        EXPECT_EQ(yPixelsOffset[12], kYFillValueOffset);

        // Create and bind UV plane texture to image.
        EGLImage uvImage;
        GLuint uvTexture;
        internalFormat = format == DXGI_FORMAT_NV12 ? GL_RG_EXT : GL_RG16_EXT;
        CreateAndBindImageToTexture(display, d3d11Texture.Get(), 1, internalFormat, GL_TEXTURE_2D,
                                    &uvImage, &uvTexture);
        ASSERT_GL_NO_ERROR();

        // Write the UV plane data to texture's full uv plane (0, 0,) - (16, 16).
        std::vector<T> uvData((bufferSize * bufferSize) / 2);
        for (UINT i = 0; i < (bufferSize * bufferSize) / 2; i++)
        {
            uvData[i] = i % 2 == 0 ? kUFillValueFull : kVFillValueFull;
        }
        glTexSubImage2D(GL_TEXTURE_2D, /*level=*/0, /*xoffset=*/0, /*yoffset=*/0,
                        /*width=*/bufferSize / 2,
                        /*height=*/bufferSize / 2, GL_RG_EXT, type, uvData.data());
        ASSERT_GL_NO_ERROR();

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, uvTexture, 0);
        EXPECT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER),
                  static_cast<unsigned>(GL_FRAMEBUFFER_COMPLETE));
        ASSERT_GL_NO_ERROR();

        // Read the UV plane pixels for a region starting at 0, 0 offsets.
        // uvPixels of size (4*4) region (*4) bytes per RGBA pixel
        T uvPixels[4 * 4 * 4] = {};
        glReadPixels(/*x=*/0, /*y=*/0, /*width=*/4, /*height=*/4, GL_RGBA, type, uvPixels);
        EXPECT_EQ(uvPixels[0], kUFillValueFull);
        EXPECT_EQ(uvPixels[1], kVFillValueFull);
        EXPECT_EQ(uvPixels[4], kUFillValueFull);
        EXPECT_EQ(uvPixels[5], kVFillValueFull);
        EXPECT_EQ(uvPixels[16], kUFillValueFull);
        EXPECT_EQ(uvPixels[17], kVFillValueFull);

        // Write the UV plane data with offset values for subregion (8, 8) - (16, 16).
        std::vector<T> uvDataOffset((bufferSize * bufferSize) / 2);
        for (UINT i = 0; i < (bufferSize * bufferSize) / 2; i++)
        {
            uvDataOffset[i] = i % 2 == 0 ? kUFillValueOffset : kVFillValueOffset;
        }
        glTexSubImage2D(GL_TEXTURE_2D, /*level=*/0, /*xoffset=*/bufferSize / 4,
                        /*yoffset=*/bufferSize / 4, /*width=*/bufferSize / 4,
                        /*height=*/bufferSize / 4, GL_RG_EXT, type, uvDataOffset.data());
        ASSERT_GL_NO_ERROR();

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, uvTexture, 0);
        EXPECT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER),
                  static_cast<unsigned>(GL_FRAMEBUFFER_COMPLETE));
        ASSERT_GL_NO_ERROR();

        // Read the UV plane pixels for a region starting at some offsets.
        // uvPixels of size (8*8) region (*4) bytes per RGBA pixel
        T uvPixelsOffset[8 * 8 * 4] = {};
        glReadPixels(/*x=*/bufferSize / 4, /*y=*/bufferSize / 4, /*width=*/bufferSize / 4,
                     /*height=*/bufferSize / 4, GL_RGBA, type, uvPixelsOffset);
        EXPECT_EQ(uvPixelsOffset[0], kUFillValueOffset);
        EXPECT_EQ(uvPixelsOffset[1], kVFillValueOffset);
        EXPECT_EQ(uvPixelsOffset[12], kUFillValueOffset);
        EXPECT_EQ(uvPixelsOffset[13], kVFillValueOffset);

        Microsoft::WRL::ComPtr<ID3D11Texture2D> stagingTexture;
        CD3D11_TEXTURE2D_DESC stagingDesc = desc;
        stagingDesc.BindFlags             = 0;
        stagingDesc.Usage                 = D3D11_USAGE_STAGING;
        stagingDesc.CPUAccessFlags        = D3D11_CPU_ACCESS_READ;

        EXPECT_TRUE(
            SUCCEEDED(mD3D11Device->CreateTexture2D(&stagingDesc, nullptr, &stagingTexture)));

        Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
        mD3D11Device->GetImmediateContext(&context);

        context->CopyResource(stagingTexture.Get(), d3d11Texture.Get());
        ASSERT_GL_NO_ERROR();

        D3D11_MAPPED_SUBRESOURCE mapped = {};
        EXPECT_TRUE(SUCCEEDED(context->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mapped)));

        uint8_t *yPlane  = reinterpret_cast<uint8_t *>(mapped.pData);
        uint8_t *uvPlane = yPlane + bufferSize * mapped.RowPitch;
        auto getYValue   = [&](int x, int y) {
            const T *lineStart = reinterpret_cast<const T *>(yPlane + y * mapped.RowPitch);
            return lineStart[x];
        };

        auto getUValue = [&](int x, int y) {
            const T *lineStart = reinterpret_cast<const T *>(uvPlane + y * mapped.RowPitch);
            return lineStart[x * 2 + 0];
        };

        auto getVValue = [&](int x, int y) {
            const T *lineStart = reinterpret_cast<const T *>(uvPlane + y * mapped.RowPitch);
            return lineStart[x * 2 + 1];
        };
        // Compare first y pixel with full write values.
        EXPECT_EQ(getYValue(0, 0), kYFillValueFull);
        // Compare last y pixel with overwritten subregion values.
        EXPECT_EQ(getYValue(bufferSize - 1, bufferSize - 1), kYFillValueOffset);
        // Compare first uv pixel with full write values.
        EXPECT_EQ(getUValue(0, 0), kUFillValueFull);
        EXPECT_EQ(getVValue(0, 0), kVFillValueFull);
        // Compare last uv pixel with overwritten subregion values.
        EXPECT_EQ(getUValue(bufferSize / 2 - 1, bufferSize / 2 - 1), kUFillValueOffset);
        EXPECT_EQ(getVValue(bufferSize / 2 - 1, bufferSize / 2 - 1), kVFillValueOffset);

        context->Unmap(stagingTexture.Get(), 0);

        glDeleteTextures(1, &yTexture);
        glDeleteTextures(1, &uvTexture);
        glDeleteFramebuffers(1, &fbo);
        eglDestroyImageKHR(display, yImage);
        eglDestroyImageKHR(display, uvImage);
    }
};

// Test that an NV12 D3D11 texture can be imported as two R8 and RG8 EGLImages and the resulting GL
// textures can be sampled from.
TEST_P(D3DTextureYUVTest, NV12TextureImageSampler)
{
    RunYUVSamplerTest(DXGI_FORMAT_NV12);
}

// ANGLE ES2/D3D11 supports GL_EXT_texture_norm16 even though the extension spec says it's ES3 only.
// Test P010 on ES2 since Chromium's Skia context is ES2 and it uses P010 for HDR video playback.
TEST_P(D3DTextureYUVTest, P010TextureImageSampler)
{
    RunYUVSamplerTest(DXGI_FORMAT_P010);
}

// Same as above, but for P016. P016 doesn't seem to be supported on all GPUs so it might be skipped
// more often than P010 and NV12 e.g. on the NVIDIA GTX 1050 Ti.
TEST_P(D3DTextureYUVTest, P016TextureImageSampler)
{
    RunYUVSamplerTest(DXGI_FORMAT_P016);
}

// Test that an NV12 D3D11 texture can be imported as two R8 and RG8 EGLImages and rendered to as
// framebuffer attachments.
TEST_P(D3DTextureYUVTest, NV12TextureImageRender)
{
    RunYUVRenderTest(DXGI_FORMAT_NV12);
}

// ANGLE ES2/D3D11 supports GL_EXT_texture_norm16 even though the extension spec says it's ES3 only.
// Test P010 on ES2 since Chromium's Skia context is ES2 and it uses P010 for HDR video playback.
TEST_P(D3DTextureYUVTest, P010TextureImageRender)
{
    RunYUVRenderTest(DXGI_FORMAT_P010);
}

// Same as above, but for P016. P016 doesn't seem to be supported on all GPUs so it might be skipped
// more often than P010 and NV12 e.g. on the NVIDIA GTX 1050 Ti.
TEST_P(D3DTextureYUVTest, P016TextureImageRender)
{
    RunYUVRenderTest(DXGI_FORMAT_P016);
}

// Test that an NV12 D3D11 texture can be imported as two R8 and RG8 EGLImages and rendered to as
// framebuffer attachments and then read from as individual planes.
TEST_P(D3DTextureYUVTest, NV12TextureImageReadPixel)
{
    RunYUVReadPixelTest(DXGI_FORMAT_NV12);
}

// ANGLE ES2/D3D11 supports GL_EXT_texture_norm16 even though the extension spec says it's ES3 only.
// Test P010 on ES2 since Chromium's Skia context is ES2 and it uses P010 for HDR video playback.
TEST_P(D3DTextureYUVTest, P010TextureImageReadPixel)
{
    RunYUVReadPixelTest(DXGI_FORMAT_P010);
}

// Same as above, but for P016. P016 doesn't seem to be supported on all GPUs so it might be skipped
// more often than P010 and NV12 e.g. on the NVIDIA GTX 1050 Ti.
TEST_P(D3DTextureYUVTest, P016TextureImageReadPixel)
{
    RunYUVReadPixelTest(DXGI_FORMAT_P016);
}

// Test that an NV12 D3D11 texture can be imported as two R8 and RG8 EGLImages and write data to
// them through glTexSubImage2D and then rendered to as framebuffer attachments and then read from
// as individual planes.
TEST_P(D3DTextureYUVTest, NV12TextureImageWritePixel)
{
    RunYUVWritePixelTest<uint8_t>(DXGI_FORMAT_NV12);
}

// Test that an P010 D3D11 texture can be imported as two R16 and RG16 EGLImages and write data to
// them through glTexSubImage2D and then rendered to as framebuffer attachments and then read from
// as individual planes.
TEST_P(D3DTextureYUVTest, P010TextureImageWritePixel)
{
    RunYUVWritePixelTest<uint16_t>(DXGI_FORMAT_P010);
}

// Test that an P016 D3D11 texture can be imported as two R16 and RG16 EGLImages and write data to
// them through glTexSubImage2D and then rendered to as framebuffer attachments and then read from
// as individual planes.
TEST_P(D3DTextureYUVTest, P016TextureImageWritePixel)
{
    RunYUVWritePixelTest<uint16_t>(DXGI_FORMAT_P016);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES2(D3DTextureTest);
ANGLE_INSTANTIATE_TEST_ES2(D3DTextureClearTest);
ANGLE_INSTANTIATE_TEST_ES2(D3DTextureYUVTest);
ANGLE_INSTANTIATE_TEST_ES3(D3DTextureTestES3);
// D3D Debug device reports an error. http://anglebug.com/40096593
// ANGLE_INSTANTIATE_TEST(D3DTextureTestMS, ES2_D3D11());
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(D3DTextureTestMS);
}  // namespace angle

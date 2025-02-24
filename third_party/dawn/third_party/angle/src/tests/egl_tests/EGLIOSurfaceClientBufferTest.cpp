//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
//    EGLIOSurfaceClientBufferTest.cpp: tests for the EGL_ANGLE_iosurface_client_buffer extension.
//

#include "test_utils/ANGLETest.h"

#include "common/mathutil.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"

#include <CoreFoundation/CoreFoundation.h>
#if TARGET_OS_OSX
#    include <IOSurface/IOSurface.h>
#else
#    include <IOSurface/IOSurfaceRef.h>
#endif
using namespace angle;

namespace
{

constexpr char kIOSurfaceExt[] = "EGL_ANGLE_iosurface_client_buffer";

void AddIntegerValue(CFMutableDictionaryRef dictionary, const CFStringRef key, int32_t value)
{
    CFNumberRef number = CFNumberCreate(nullptr, kCFNumberSInt32Type, &value);
    CFDictionaryAddValue(dictionary, key, number);
    CFRelease(number);
}

class [[nodiscard]] ScopedIOSurfaceRef : angle::NonCopyable
{
  public:
    explicit ScopedIOSurfaceRef(IOSurfaceRef surface) : mSurface(surface) {}

    ~ScopedIOSurfaceRef()
    {
        if (mSurface != nullptr)
        {
            CFRelease(mSurface);
            mSurface = nullptr;
        }
    }

    IOSurfaceRef get() const { return mSurface; }

    ScopedIOSurfaceRef(ScopedIOSurfaceRef &&other)
    {
        if (mSurface != nullptr)
        {
            CFRelease(mSurface);
        }
        mSurface       = other.mSurface;
        other.mSurface = nullptr;
    }

    ScopedIOSurfaceRef &operator=(ScopedIOSurfaceRef &&other)
    {
        if (mSurface != nullptr)
        {
            CFRelease(mSurface);
        }
        mSurface       = other.mSurface;
        other.mSurface = nullptr;

        return *this;
    }

  private:
    IOSurfaceRef mSurface = nullptr;
};

struct IOSurfacePlaneInfo
{
    int width;
    int height;
    int bytesPerElement;
};

ScopedIOSurfaceRef CreateIOSurface(int32_t format, const std::vector<IOSurfacePlaneInfo> &planes)
{
    EXPECT_GT(planes.size(), 0u);

    CFMutableDictionaryRef dict = CFDictionaryCreateMutable(
        kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    AddIntegerValue(dict, kIOSurfaceWidth, planes[0].width);
    AddIntegerValue(dict, kIOSurfaceHeight, planes[0].height);
    AddIntegerValue(dict, kIOSurfacePixelFormat, format);

    if (planes.size() > 1)
    {
        CFMutableArrayRef planesInfo =
            CFArrayCreateMutable(kCFAllocatorDefault, planes.size(), &kCFTypeArrayCallBacks);
        for (const IOSurfacePlaneInfo &plane : planes)
        {
            CFMutableDictionaryRef planeInfo =
                CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
                                          &kCFTypeDictionaryValueCallBacks);
            AddIntegerValue(planeInfo, kIOSurfacePlaneWidth, plane.width);
            AddIntegerValue(planeInfo, kIOSurfacePlaneHeight, plane.height);
            AddIntegerValue(planeInfo, kIOSurfacePlaneBytesPerElement, plane.bytesPerElement);

            CFArrayAppendValue(planesInfo, planeInfo);
            CFRelease(planeInfo);
        }

        CFDictionaryAddValue(dict, kIOSurfacePlaneInfo, planesInfo);
        CFRelease(planesInfo);
    }
    else
    {
        AddIntegerValue(dict, kIOSurfaceBytesPerElement, planes[0].bytesPerElement);
    }

    IOSurfaceRef ioSurface = IOSurfaceCreate(dict);
    EXPECT_NE(nullptr, ioSurface);
    CFRelease(dict);

    return ScopedIOSurfaceRef(ioSurface);
}

ScopedIOSurfaceRef CreateSinglePlaneIOSurface(int width,
                                              int height,
                                              int32_t format,
                                              int bytesPerElement)
{
    std::vector<IOSurfacePlaneInfo> planes{{width, height, bytesPerElement}};
    return CreateIOSurface(format, planes);
}

}  // anonymous namespace

class IOSurfaceClientBufferTest : public ANGLETest<>
{
  protected:
    EGLint getTextureTarget() const
    {
        EGLint target = 0;
        eglGetConfigAttrib(mDisplay, mConfig, EGL_BIND_TO_TEXTURE_TARGET_ANGLE, &target);
        return target;
    }

    GLint getGLTextureTarget() const
    {
        EGLint targetEGL = getTextureTarget();
        GLenum targetGL  = 0;
        switch (targetEGL)
        {
            case EGL_TEXTURE_2D:
                targetGL = GL_TEXTURE_2D;
                break;
            case EGL_TEXTURE_RECTANGLE_ANGLE:
                targetGL = GL_TEXTURE_RECTANGLE_ANGLE;
                break;
            default:
                break;
        }
        return targetGL;
    }

    IOSurfaceClientBufferTest() : mConfig(0), mDisplay(nullptr) {}

    void testSetUp() override
    {
        mConfig  = getEGLWindow()->getConfig();
        mDisplay = getEGLWindow()->getDisplay();
    }

    void createIOSurfacePbuffer(const ScopedIOSurfaceRef &ioSurface,
                                EGLint width,
                                EGLint height,
                                EGLint plane,
                                GLenum internalFormat,
                                GLenum type,
                                EGLSurface *pbuffer) const
    {
        // clang-format off
        const EGLint attribs[] = {
            EGL_WIDTH,                         width,
            EGL_HEIGHT,                        height,
            EGL_IOSURFACE_PLANE_ANGLE,         plane,
            EGL_TEXTURE_TARGET,                getTextureTarget(),
            EGL_TEXTURE_INTERNAL_FORMAT_ANGLE,
                static_cast<EGLint>(internalFormat),
            EGL_TEXTURE_FORMAT,                EGL_TEXTURE_RGBA,
            EGL_TEXTURE_TYPE_ANGLE,            static_cast<EGLint>(type),
            EGL_NONE,                          EGL_NONE,
        };
        // clang-format on

        *pbuffer = eglCreatePbufferFromClientBuffer(mDisplay, EGL_IOSURFACE_ANGLE, ioSurface.get(),
                                                    mConfig, attribs);
        EXPECT_NE(EGL_NO_SURFACE, *pbuffer);
    }

    void bindIOSurfaceToTexture(const ScopedIOSurfaceRef &ioSurface,
                                EGLint width,
                                EGLint height,
                                EGLint plane,
                                GLenum internalFormat,
                                GLenum type,
                                EGLSurface *pbuffer,
                                GLTexture *texture) const
    {
        createIOSurfacePbuffer(ioSurface, width, height, plane, internalFormat, type, pbuffer);

        // Bind the pbuffer
        glBindTexture(getGLTextureTarget(), *texture);
        EGLBoolean result = eglBindTexImage(mDisplay, *pbuffer, EGL_BACK_BUFFER);
        EXPECT_EGL_TRUE(result);
        EXPECT_EGL_SUCCESS();
    }

    void doClearTest(const ScopedIOSurfaceRef &ioSurface,
                     EGLint width,
                     EGLint height,
                     EGLint plane,
                     GLenum internalFormat,
                     GLenum type,
                     const GLColor &data)
    {
        std::array<uint8_t, 4> dataArray{data.R, data.G, data.B, data.A};
        doClearTest(ioSurface, width, height, plane, internalFormat, type, dataArray);
    }

    template <typename T, size_t dataSize>
    void doClearTest(const ScopedIOSurfaceRef &ioSurface,
                     EGLint width,
                     EGLint height,
                     EGLint plane,
                     GLenum internalFormat,
                     GLenum type,
                     const std::array<T, dataSize> &data)
    {
        // Bind the IOSurface to a texture and clear it.
        EGLSurface pbuffer;
        GLTexture texture;
        bindIOSurfaceToTexture(ioSurface, width, height, plane, internalFormat, type, &pbuffer,
                               &texture);

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        EXPECT_GL_NO_ERROR();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, getGLTextureTarget(), texture,
                               0);
        EXPECT_GL_NO_ERROR();
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
        EXPECT_GL_NO_ERROR();

        glClearColor(1.0f / 255.0f, 2.0f / 255.0f, 3.0f / 255.0f, 4.0f / 255.0f);
        EXPECT_GL_NO_ERROR();
        glClear(GL_COLOR_BUFFER_BIT);
        EXPECT_GL_NO_ERROR();

        // Unbind pbuffer and check content.
        EGLBoolean result = eglReleaseTexImage(mDisplay, pbuffer, EGL_BACK_BUFFER);
        EXPECT_EGL_TRUE(result);
        EXPECT_EGL_SUCCESS();

        // IOSurface client buffer's rendering doesn't automatically finish after
        // eglReleaseTexImage(). Need to explicitly call glFinish().
        glFinish();

        IOSurfaceLock(ioSurface.get(), kIOSurfaceLockReadOnly, nullptr);
        std::array<T, dataSize> iosurfaceData;
        memcpy(iosurfaceData.data(), IOSurfaceGetBaseAddressOfPlane(ioSurface.get(), plane),
               sizeof(T) * data.size());
        IOSurfaceUnlock(ioSurface.get(), kIOSurfaceLockReadOnly, nullptr);

        if (internalFormat == GL_RGB && IsMac() && IsOpenGL())
        {
            // Ignore alpha component for BGRX, the alpha value is undefined
            for (int i = 0; i < 3; i++)
            {
                ASSERT_EQ(data[i], iosurfaceData[i]);
            }
        }
        else
        {
            ASSERT_EQ(data, iosurfaceData);
        }

        result = eglDestroySurface(mDisplay, pbuffer);
        EXPECT_EGL_TRUE(result);
        EXPECT_EGL_SUCCESS();
    }

    enum ColorMask
    {
        R = 1,
        G = 2,
        B = 4,
        A = 8,
    };
    void doSampleTestWithExtraSteps(const ScopedIOSurfaceRef &ioSurface,
                                    EGLint width,
                                    EGLint height,
                                    EGLint plane,
                                    GLenum internalFormat,
                                    GLenum type,
                                    void *data,
                                    size_t dataSize,
                                    int mask,
                                    const std::function<void()> &extraStepsBeforeSample)
    {
        // Write the data to the IOSurface
        IOSurfaceLock(ioSurface.get(), 0, nullptr);
        memcpy(IOSurfaceGetBaseAddressOfPlane(ioSurface.get(), plane), data, dataSize);
        IOSurfaceUnlock(ioSurface.get(), 0, nullptr);

        GLTexture texture;
        glBindTexture(getGLTextureTarget(), texture);
        glTexParameteri(getGLTextureTarget(), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(getGLTextureTarget(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // Bind the IOSurface to a texture and clear it.
        EGLSurface pbuffer;
        bindIOSurfaceToTexture(ioSurface, width, height, plane, internalFormat, type, &pbuffer,
                               &texture);

        if (extraStepsBeforeSample)
        {
            extraStepsBeforeSample();
        }

        doSampleTestWithTexture(texture, mask);

        EGLBoolean result = eglDestroySurface(mDisplay, pbuffer);
        EXPECT_EGL_TRUE(result);
        EXPECT_EGL_SUCCESS();
    }

    void doSampleTest(const ScopedIOSurfaceRef &ioSurface,
                      EGLint width,
                      EGLint height,
                      EGLint plane,
                      GLenum internalFormat,
                      GLenum type,
                      void *data,
                      size_t dataSize,
                      int mask)
    {
        doSampleTestWithExtraSteps(ioSurface, width, height, plane, internalFormat, type, data,
                                   dataSize, mask, nullptr);
    }

    void doSampleTestWithTexture(const GLTexture &texture, int mask)
    {
        constexpr char kVS[] =
            "attribute vec4 position;\n"
            "void main()\n"
            "{\n"
            "    gl_Position = vec4(position.xy, 0.0, 1.0);\n"
            "}\n";
        constexpr char kFS_rect[] =
            "#extension GL_ARB_texture_rectangle : require\n"
            "precision mediump float;\n"
            "uniform sampler2DRect tex;\n"
            "void main()\n"
            "{\n"
            "    gl_FragColor = texture2DRect(tex, vec2(0, 0));\n"
            "}\n";
        constexpr char kFS_2D[] =
            "precision mediump float;\n"
            "uniform sampler2D tex;\n"
            "void main()\n"
            "{\n"
            "    gl_FragColor = texture2D(tex, vec2(0, 0));\n"
            "}\n";

        ANGLE_GL_PROGRAM(program, kVS,
                         (getTextureTarget() == EGL_TEXTURE_RECTANGLE_ANGLE ? kFS_rect : kFS_2D));
        glUseProgram(program);

        GLint location = glGetUniformLocation(program, "tex");
        ASSERT_NE(-1, location);
        glUniform1i(location, 0);

        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);
        drawQuad(program, "position", 0.5f, 1.0f, false);

        GLColor expectedColor((mask & R) ? 1 : 0, (mask & G) ? 2 : 0, (mask & B) ? 3 : 0,
                              (mask & A) ? 4 : 255);
        EXPECT_PIXEL_COLOR_EQ(0, 0, expectedColor);
        ASSERT_GL_NO_ERROR();
    }

    void doBlitTest(bool ioSurfaceIsSource, int width, int height)
    {
        if (!hasBlitExt())
        {
            return;
        }

        // Create IOSurface and bind it to a texture.
        ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(width, height, 'BGRA', 4);
        EGLSurface pbuffer;
        GLTexture texture;
        bindIOSurfaceToTexture(ioSurface, width, height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, &pbuffer,
                               &texture);

        GLFramebuffer iosurfaceFbo;
        glBindFramebuffer(GL_FRAMEBUFFER, iosurfaceFbo);
        EXPECT_GL_NO_ERROR();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, getGLTextureTarget(), texture,
                               0);
        EXPECT_GL_NO_ERROR();
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
        EXPECT_GL_NO_ERROR();

        // Create another framebuffer with a regular renderbuffer.
        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        EXPECT_GL_NO_ERROR();
        GLRenderbuffer rbo;
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        EXPECT_GL_NO_ERROR();
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, width, height);
        EXPECT_GL_NO_ERROR();
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
        EXPECT_GL_NO_ERROR();
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
        EXPECT_GL_NO_ERROR();

        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        EXPECT_GL_NO_ERROR();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        EXPECT_GL_NO_ERROR();

        // Choose which is going to be the source and destination.
        GLFramebuffer &src = ioSurfaceIsSource ? iosurfaceFbo : fbo;
        GLFramebuffer &dst = ioSurfaceIsSource ? fbo : iosurfaceFbo;

        // Clear source to known color.
        glBindFramebuffer(GL_FRAMEBUFFER, src);
        glClearColor(1.0f / 255.0f, 2.0f / 255.0f, 3.0f / 255.0f, 4.0f / 255.0f);
        EXPECT_GL_NO_ERROR();
        glClear(GL_COLOR_BUFFER_BIT);
        EXPECT_GL_NO_ERROR();

        // Blit to destination.
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, dst);
        glBlitFramebufferANGLE(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT,
                               GL_NEAREST);

        // Read back from destination.
        glBindFramebuffer(GL_FRAMEBUFFER, dst);
        GLColor expectedColor(1, 2, 3, 4);
        EXPECT_PIXEL_COLOR_EQ(0, 0, expectedColor);

        // Unbind pbuffer and check content.
        EGLBoolean result = eglReleaseTexImage(mDisplay, pbuffer, EGL_BACK_BUFFER);
        EXPECT_EGL_TRUE(result);
        EXPECT_EGL_SUCCESS();

        result = eglDestroySurface(mDisplay, pbuffer);
        EXPECT_EGL_TRUE(result);
        EXPECT_EGL_SUCCESS();
    }

    bool hasIOSurfaceExt() const { return IsEGLDisplayExtensionEnabled(mDisplay, kIOSurfaceExt); }
    bool hasBlitExt() const
    {
        return IsEGLDisplayExtensionEnabled(mDisplay, "ANGLE_framebuffer_blit");
    }

    EGLConfig mConfig;
    EGLDisplay mDisplay;
};

// Test using RGBA8888 IOSurfaces for rendering
TEST_P(IOSurfaceClientBufferTest, RenderToRGBA8888IOSurface)
{
    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());

    // DesktopOpenGL doesn't support RGBA IOSurface.
    ANGLE_SKIP_TEST_IF(IsDesktopOpenGL());

    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(1, 1, 'RGBA', 4);

    GLColor color(1, 2, 3, 4);
    doClearTest(ioSurface, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, color);
}

// Test reading from RGBA8888 IOSurfaces
TEST_P(IOSurfaceClientBufferTest, ReadFromRGBA8888IOSurface)
{
    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());

    // DesktopOpenGL doesn't support RGBA IOSurface.
    ANGLE_SKIP_TEST_IF(IsDesktopOpenGL());

    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(1, 1, 'RGBA', 4);

    GLColor color(1, 2, 3, 4);
    doSampleTest(ioSurface, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &color, sizeof(color),
                 R | G | B | A);
}

// Test using BGRA8888 IOSurfaces for rendering
TEST_P(IOSurfaceClientBufferTest, RenderToBGRA8888IOSurface)
{
    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());

    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(1, 1, 'BGRA', 4);

    GLColor color(3, 2, 1, 4);
    doClearTest(ioSurface, 1, 1, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, color);
}

// Test reading from BGRA8888 IOSurfaces
TEST_P(IOSurfaceClientBufferTest, ReadFromBGRA8888IOSurface)
{
    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());

    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(1, 1, 'BGRA', 4);

    GLColor color(3, 2, 1, 4);
    doSampleTest(ioSurface, 1, 1, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, &color, sizeof(color),
                 R | G | B | A);
}

// Test using RGBX8888 IOSurfaces for rendering
TEST_P(IOSurfaceClientBufferTest, RenderToRGBX8888IOSurface)
{
    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());

    // DesktopOpenGL doesn't support RGBA IOSurface.
    ANGLE_SKIP_TEST_IF(IsDesktopOpenGL());

    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(1, 1, 'RGBA', 4);

    GLColor color(1, 2, 3, 255);
    doClearTest(ioSurface, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, color);
}

// Test reading from RGBX8888 IOSurfaces
TEST_P(IOSurfaceClientBufferTest, ReadFromRGBX8888IOSurface)
{
    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());

    // DesktopOpenGL doesn't support RGBA IOSurface.
    ANGLE_SKIP_TEST_IF(IsDesktopOpenGL());

    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(1, 1, 'RGBA', 4);

    GLColor color(1, 2, 3, 255);
    doSampleTest(ioSurface, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &color, sizeof(color), R | G | B);
}

// Test using BGRX8888 IOSurfaces for rendering
TEST_P(IOSurfaceClientBufferTest, RenderToBGRX8888IOSurface)
{
    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());

    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(1, 1, 'BGRA', 4);

    GLColor color(3, 2, 1, 255);
    doClearTest(ioSurface, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, color);
}

// Test reading from BGRX8888 IOSurfaces
TEST_P(IOSurfaceClientBufferTest, ReadFromBGRX8888IOSurface)
{
    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());

    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(1, 1, 'BGRA', 4);

    GLColor color(3, 2, 1, 4);
    doSampleTest(ioSurface, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &color, sizeof(color), R | G | B);
}

// Test using RG88 IOSurfaces for rendering
TEST_P(IOSurfaceClientBufferTest, RenderToRG88IOSurface)
{
    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());

    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(1, 1, '2C08', 2);

    std::array<uint8_t, 2> color{1, 2};
    doClearTest(ioSurface, 1, 1, 0, GL_RG, GL_UNSIGNED_BYTE, color);
}

// Test reading from RG88 IOSurfaces
TEST_P(IOSurfaceClientBufferTest, ReadFromRG88IOSurface)
{
    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());

    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(1, 1, '2C08', 2);

    uint8_t color[2] = {1, 2};
    doSampleTest(ioSurface, 1, 1, 0, GL_RG, GL_UNSIGNED_BYTE, &color, sizeof(color), R | G);
}

// Test using R8 IOSurfaces for rendering
TEST_P(IOSurfaceClientBufferTest, RenderToR8IOSurface)
{
    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());

    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(1, 1, 'L008', 1);

    std::array<uint8_t, 1> color{1};
    doClearTest(ioSurface, 1, 1, 0, GL_RED, GL_UNSIGNED_BYTE, color);
}

// Test reading from R8 IOSurfaces
TEST_P(IOSurfaceClientBufferTest, ReadFromR8IOSurface)
{
    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());

    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(1, 1, 'L008', 1);

    uint8_t color = 1;
    doSampleTest(ioSurface, 1, 1, 0, GL_RED, GL_UNSIGNED_BYTE, &color, sizeof(color), R);
}

// Test using RG1616 IOSurfaces for rendering
TEST_P(IOSurfaceClientBufferTest, RenderToRG1616IOSurface)
{
    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_norm16"));

    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(1, 1, '2C16', 4);

    std::array<uint16_t, 2> color{257, 514};
    doClearTest(ioSurface, 1, 1, 0, GL_RG, GL_UNSIGNED_SHORT, color);
}

// Test reading from RG1616 IOSurfaces
TEST_P(IOSurfaceClientBufferTest, ReadFromRG1616IOSurface)
{
    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_norm16"));

    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(1, 1, '2C16', 4);

    uint16_t color[2] = {257, 514};
    doSampleTest(ioSurface, 1, 1, 0, GL_RG, GL_UNSIGNED_SHORT, &color, sizeof(color), R | G);
}

// Test using R16 IOSurfaces for rendering
TEST_P(IOSurfaceClientBufferTest, RenderToR16IOSurface)
{
    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_norm16"));

    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(1, 1, 'L016', 2);

    std::array<uint16_t, 1> color{257};
    doClearTest(ioSurface, 1, 1, 0, GL_RED, GL_UNSIGNED_SHORT, color);
}

// Test reading from R16 IOSurfaces
TEST_P(IOSurfaceClientBufferTest, ReadFromR16IOSurface)
{
    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_norm16"));

    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(1, 1, 'L016', 2);

    uint16_t color = 257;
    doSampleTest(ioSurface, 1, 1, 0, GL_RED, GL_UNSIGNED_SHORT, &color, sizeof(color), R);
}

// Test using BGRA_1010102 IOSurfaces for rendering
TEST_P(IOSurfaceClientBufferTest, RenderToBGRA1010102IOSurface)
{
    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());

    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(1, 1, 'l10r', 4);

    std::array<uint32_t, 1> color{(0 << 30) | (1 << 22) | (2 << 12) | (3 << 2)};
    doClearTest(ioSurface, 1, 1, 0, GL_RGB10_A2, GL_UNSIGNED_INT_2_10_10_10_REV, color);
}

// Test reading from BGRA_1010102 IOSurfaces
TEST_P(IOSurfaceClientBufferTest, ReadFromBGRA1010102IOSurface)
{
    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());

    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(1, 1, 'l10r', 4);

    uint32_t color = (3 << 30) | (1 << 22) | (2 << 12) | (3 << 2);
    doSampleTest(ioSurface, 1, 1, 0, GL_RGB10_A2, GL_UNSIGNED_INT_2_10_10_10_REV, &color,
                 sizeof(color),
                 R | G | B);  // Don't test alpha, unorm '4' can't be represented with 2 bits.
}

// Test using RGBA_16F IOSurfaces for rendering
TEST_P(IOSurfaceClientBufferTest, RenderToRGBA16FIOSurface)
{
    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());

    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(1, 1, 'RGhA', 8);

    std::array<GLushort, 4> color{
        gl::float32ToFloat16(1.0f / 255.0f), gl::float32ToFloat16(2.0f / 255.0f),
        gl::float32ToFloat16(3.0f / 255.0f), gl::float32ToFloat16(4.0f / 255.0f)};
    doClearTest(ioSurface, 1, 1, 0, GL_RGBA, GL_HALF_FLOAT, color);
}

// Test reading from RGBA_16F IOSurfaces
TEST_P(IOSurfaceClientBufferTest, ReadFromToRGBA16FIOSurface)
{
    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());

    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(1, 1, 'RGhA', 8);

    std::array<GLushort, 4> color{
        gl::float32ToFloat16(1.0f / 255.0f), gl::float32ToFloat16(2.0f / 255.0f),
        gl::float32ToFloat16(3.0f / 255.0f), gl::float32ToFloat16(4.0f / 255.0f)};
    doSampleTest(ioSurface, 1, 1, 0, GL_RGBA, GL_HALF_FLOAT, color.data(), sizeof(GLushort) * 4,
                 R | G | B | A);
}

// Test using YUV420 IOSurfaces for rendering
TEST_P(IOSurfaceClientBufferTest, RenderToYUV420IOSurface)
{
    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());

    std::vector<IOSurfacePlaneInfo> planes{{2, 2, 1}, {1, 1, 2}};
    ScopedIOSurfaceRef ioSurface = CreateIOSurface('420v', planes);

    {
        std::array<GLubyte, 1> colors{1};
        doClearTest(ioSurface, planes[0].width, planes[0].height, 0, GL_RED, GL_UNSIGNED_BYTE,
                    colors);
    }

    {
        std::array<GLubyte, 2> colors{1, 2};
        doClearTest(ioSurface, planes[1].width, planes[1].height, 1, GL_RG, GL_UNSIGNED_BYTE,
                    colors);
    }
}

// Test reading from YUV420 IOSurfaces
TEST_P(IOSurfaceClientBufferTest, ReadFromToYUV420IOSurface)
{
    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());

    std::vector<IOSurfacePlaneInfo> planes{{2, 2, 1}, {1, 1, 2}};
    ScopedIOSurfaceRef ioSurface = CreateIOSurface('420v', planes);

    {
        std::array<GLubyte, 1> colors{1};
        doSampleTest(ioSurface, planes[0].width, planes[0].height, 0, GL_RED, GL_UNSIGNED_BYTE,
                     colors.data(), sizeof(GLubyte) * colors.size(), R);
    }

    {
        std::array<GLubyte, 2> colors{1, 2};
        doSampleTest(ioSurface, planes[1].width, planes[1].height, 1, GL_RG, GL_UNSIGNED_BYTE,
                     colors.data(), sizeof(GLubyte) * colors.size(), R | G);
    }
}

// Test using P010 IOSurfaces for rendering
TEST_P(IOSurfaceClientBufferTest, RenderToP010IOSurface)
{
    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());

    std::vector<IOSurfacePlaneInfo> planes{{2, 2, 2}, {1, 1, 4}};
    ScopedIOSurfaceRef ioSurface = CreateIOSurface('x420', planes);

    {
        std::array<GLushort, 1> colors{257};
        doClearTest(ioSurface, planes[0].width, planes[0].height, 0, GL_RED, GL_UNSIGNED_SHORT,
                    colors);
    }

    {
        std::array<GLushort, 2> colors{257, 514};
        doClearTest(ioSurface, planes[1].width, planes[1].height, 1, GL_RG, GL_UNSIGNED_SHORT,
                    colors);
    }
}

// Test reading from P010 IOSurfaces
TEST_P(IOSurfaceClientBufferTest, ReadFromToP010IOSurface)
{
    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());

    std::vector<IOSurfacePlaneInfo> planes{{2, 2, 2}, {1, 1, 4}};
    ScopedIOSurfaceRef ioSurface = CreateIOSurface('x420', planes);

    {
        std::array<GLushort, 1> colors{257};
        doSampleTest(ioSurface, planes[0].width, planes[0].height, 0, GL_RED, GL_UNSIGNED_SHORT,
                     colors.data(), sizeof(GLushort) * colors.size(), R);
    }

    {
        std::array<GLushort, 2> colors{257, 514};
        doSampleTest(ioSurface, planes[1].width, planes[1].height, 1, GL_RG, GL_UNSIGNED_SHORT,
                     colors.data(), sizeof(GLushort) * colors.size(), R | G);
    }
}

// Test blitting from IOSurface
TEST_P(IOSurfaceClientBufferTest, BlitFromIOSurface)
{
    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());

    doBlitTest(true, 2, 2);
}

// Test blitting to IOSurface
TEST_P(IOSurfaceClientBufferTest, BlitToIOSurface)
{
    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());

    doBlitTest(false, 2, 2);
}

// Test using glCopyTexSubImage to copy to BGRX8888 IOSurfaces works.
TEST_P(IOSurfaceClientBufferTest, CopySubImageToBGRX8888IOSurface)
{
    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());

    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(1, 1, 'BGRA', 4);

    GLTexture texture;
    glBindTexture(getGLTextureTarget(), texture);
    glTexParameteri(getGLTextureTarget(), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(getGLTextureTarget(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Bind the IOSurface to a texture and clear it.
    EGLSurface pbuffer;
    bindIOSurfaceToTexture(ioSurface, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &pbuffer, &texture);

    // 1. Clear default framebuffer with desired color.
    GLColor color(1, 2, 3, 4);
    glClearColor(color.R / 255.f, color.G / 255.f, color.B / 255.f, color.A / 255.f);
    glClear(GL_COLOR_BUFFER_BIT);

    // 2. Copy color from default framebuffer to iosurface's bound texture.
    glCopyTexSubImage2D(getGLTextureTarget(), 0, 0, 0, 0, 0, 1, 1);
    EXPECT_GL_NO_ERROR();

    // 3. Do texture sampling verification.
    doSampleTestWithTexture(texture, R | G | B);
}

// Test the validation errors for missing attributes for eglCreatePbufferFromClientBuffer with
// IOSurface
TEST_P(IOSurfaceClientBufferTest, NegativeValidationMissingAttributes)
{
    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());

    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(10, 10, 'BGRA', 4);

    // Success case
    {
        // clang-format off
        const EGLint attribs[] = {
            EGL_WIDTH,                         10,
            EGL_HEIGHT,                        10,
            EGL_IOSURFACE_PLANE_ANGLE,         0,
            EGL_TEXTURE_TARGET,                getTextureTarget(),
            EGL_TEXTURE_INTERNAL_FORMAT_ANGLE, GL_BGRA_EXT,
            EGL_TEXTURE_FORMAT,                EGL_TEXTURE_RGBA,
            EGL_TEXTURE_TYPE_ANGLE,            GL_UNSIGNED_BYTE,
            EGL_NONE,                          EGL_NONE,
        };
        // clang-format off

        EGLSurface pbuffer = eglCreatePbufferFromClientBuffer(mDisplay, EGL_IOSURFACE_ANGLE, ioSurface.get(), mConfig, attribs);
        EXPECT_NE(EGL_NO_SURFACE, pbuffer);

        EGLBoolean result = eglDestroySurface(mDisplay, pbuffer);
        EXPECT_EGL_TRUE(result);
        EXPECT_EGL_SUCCESS();
    }

    // Missing EGL_WIDTH
    {
        // clang-format off
        const EGLint attribs[] = {
            EGL_HEIGHT,                        10,
            EGL_IOSURFACE_PLANE_ANGLE,         0,
            EGL_TEXTURE_TARGET,                getTextureTarget(),
            EGL_TEXTURE_INTERNAL_FORMAT_ANGLE, GL_BGRA_EXT,
            EGL_TEXTURE_FORMAT,                EGL_TEXTURE_RGBA,
            EGL_TEXTURE_TYPE_ANGLE,            GL_UNSIGNED_BYTE,
            EGL_NONE,                          EGL_NONE,
        };
        // clang-format on

        EGLSurface pbuffer = eglCreatePbufferFromClientBuffer(mDisplay, EGL_IOSURFACE_ANGLE,
                                                              ioSurface.get(), mConfig, attribs);
        EXPECT_EQ(EGL_NO_SURFACE, pbuffer);
        EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
    }

    // Missing EGL_HEIGHT
    {
        // clang-format off
        const EGLint attribs[] = {
            EGL_WIDTH,                         10,
            EGL_IOSURFACE_PLANE_ANGLE,         0,
            EGL_TEXTURE_TARGET,                getTextureTarget(),
            EGL_TEXTURE_INTERNAL_FORMAT_ANGLE, GL_BGRA_EXT,
            EGL_TEXTURE_FORMAT,                EGL_TEXTURE_RGBA,
            EGL_TEXTURE_TYPE_ANGLE,            GL_UNSIGNED_BYTE,
            EGL_NONE,                          EGL_NONE,
        };
        // clang-format on

        EGLSurface pbuffer = eglCreatePbufferFromClientBuffer(mDisplay, EGL_IOSURFACE_ANGLE,
                                                              ioSurface.get(), mConfig, attribs);
        EXPECT_EQ(EGL_NO_SURFACE, pbuffer);
        EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
    }

    // Missing EGL_IOSURFACE_PLANE_ANGLE
    {
        // clang-format off
        const EGLint attribs[] = {
            EGL_WIDTH,                         10,
            EGL_HEIGHT,                        10,
            EGL_TEXTURE_TARGET,                getTextureTarget(),
            EGL_TEXTURE_INTERNAL_FORMAT_ANGLE, GL_BGRA_EXT,
            EGL_TEXTURE_FORMAT,                EGL_TEXTURE_RGBA,
            EGL_TEXTURE_TYPE_ANGLE,            GL_UNSIGNED_BYTE,
            EGL_NONE,                          EGL_NONE,
        };
        // clang-format on

        EGLSurface pbuffer = eglCreatePbufferFromClientBuffer(mDisplay, EGL_IOSURFACE_ANGLE,
                                                              ioSurface.get(), mConfig, attribs);
        EXPECT_EQ(EGL_NO_SURFACE, pbuffer);
        EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
    }

    // Missing EGL_TEXTURE_TARGET - EGL_BAD_MATCH from the base spec of
    // eglCreatePbufferFromClientBuffer
    {
        // clang-format off
        const EGLint attribs[] = {
            EGL_WIDTH,                         10,
            EGL_HEIGHT,                        10,
            EGL_IOSURFACE_PLANE_ANGLE,         0,
            EGL_TEXTURE_INTERNAL_FORMAT_ANGLE, GL_BGRA_EXT,
            EGL_TEXTURE_FORMAT,                EGL_TEXTURE_RGBA,
            EGL_TEXTURE_TYPE_ANGLE,            GL_UNSIGNED_BYTE,
            EGL_NONE,                          EGL_NONE,
        };
        // clang-format on

        EGLSurface pbuffer = eglCreatePbufferFromClientBuffer(mDisplay, EGL_IOSURFACE_ANGLE,
                                                              ioSurface.get(), mConfig, attribs);
        EXPECT_EQ(EGL_NO_SURFACE, pbuffer);
        EXPECT_EGL_ERROR(EGL_BAD_MATCH);
    }

    // Missing EGL_TEXTURE_INTERNAL_FORMAT_ANGLE
    {
        // clang-format off
        const EGLint attribs[] = {
            EGL_WIDTH,                         10,
            EGL_HEIGHT,                        10,
            EGL_IOSURFACE_PLANE_ANGLE,         0,
            EGL_TEXTURE_TARGET,                getTextureTarget(),
            EGL_TEXTURE_FORMAT,                EGL_TEXTURE_RGBA,
            EGL_TEXTURE_TYPE_ANGLE,            GL_UNSIGNED_BYTE,
            EGL_NONE,                          EGL_NONE,
        };
        // clang-format on

        EGLSurface pbuffer = eglCreatePbufferFromClientBuffer(mDisplay, EGL_IOSURFACE_ANGLE,
                                                              ioSurface.get(), mConfig, attribs);
        EXPECT_EQ(EGL_NO_SURFACE, pbuffer);
        EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
    }

    // Missing EGL_TEXTURE_FORMAT - EGL_BAD_MATCH from the base spec of
    // eglCreatePbufferFromClientBuffer
    {
        // clang-format off
        const EGLint attribs[] = {
            EGL_WIDTH,                         10,
            EGL_HEIGHT,                        10,
            EGL_IOSURFACE_PLANE_ANGLE,         0,
            EGL_TEXTURE_TARGET,                getTextureTarget(),
            EGL_TEXTURE_INTERNAL_FORMAT_ANGLE, GL_BGRA_EXT,
            EGL_TEXTURE_TYPE_ANGLE,            GL_UNSIGNED_BYTE,
            EGL_NONE,                          EGL_NONE,
        };
        // clang-format on

        EGLSurface pbuffer = eglCreatePbufferFromClientBuffer(mDisplay, EGL_IOSURFACE_ANGLE,
                                                              ioSurface.get(), mConfig, attribs);
        EXPECT_EQ(EGL_NO_SURFACE, pbuffer);
        EXPECT_EGL_ERROR(EGL_BAD_MATCH);
    }

    // Missing EGL_TEXTURE_TYPE_ANGLE
    {
        // clang-format off
        const EGLint attribs[] = {
            EGL_WIDTH,                         10,
            EGL_HEIGHT,                        10,
            EGL_IOSURFACE_PLANE_ANGLE,         0,
            EGL_TEXTURE_TARGET,                getTextureTarget(),
            EGL_TEXTURE_INTERNAL_FORMAT_ANGLE, GL_BGRA_EXT,
            EGL_TEXTURE_FORMAT,                EGL_TEXTURE_RGBA,
            EGL_NONE,                          EGL_NONE,
        };
        // clang-format on

        EGLSurface pbuffer = eglCreatePbufferFromClientBuffer(mDisplay, EGL_IOSURFACE_ANGLE,
                                                              ioSurface.get(), mConfig, attribs);
        EXPECT_EQ(EGL_NO_SURFACE, pbuffer);
        EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
    }
}

// Test the validation errors for bad parameters for eglCreatePbufferFromClientBuffer with IOSurface
TEST_P(IOSurfaceClientBufferTest, NegativeValidationBadAttributes)
{
    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());

    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(10, 10, 'BGRA', 4);

    // Success case
    {
        // clang-format off
        const EGLint attribs[] = {
            EGL_WIDTH,                         10,
            EGL_HEIGHT,                        10,
            EGL_IOSURFACE_PLANE_ANGLE,         0,
            EGL_TEXTURE_TARGET,                getTextureTarget(),
            EGL_TEXTURE_INTERNAL_FORMAT_ANGLE, GL_BGRA_EXT,
            EGL_TEXTURE_FORMAT,                EGL_TEXTURE_RGBA,
            EGL_TEXTURE_TYPE_ANGLE,            GL_UNSIGNED_BYTE,
            EGL_NONE,                          EGL_NONE,
        };
        // clang-format off

        EGLSurface pbuffer = eglCreatePbufferFromClientBuffer(mDisplay, EGL_IOSURFACE_ANGLE, ioSurface.get(), mConfig, attribs);
        EXPECT_NE(EGL_NO_SURFACE, pbuffer);

        EGLBoolean result = eglDestroySurface(mDisplay, pbuffer);
        EXPECT_EGL_TRUE(result);
        EXPECT_EGL_SUCCESS();
    }

    // EGL_TEXTURE_FORMAT must be EGL_TEXTURE_RGBA
    {
        // clang-format off
        const EGLint attribs[] = {
            EGL_WIDTH,                         10,
            EGL_HEIGHT,                        10,
            EGL_IOSURFACE_PLANE_ANGLE,         0,
            EGL_TEXTURE_TARGET,                getTextureTarget(),
            EGL_TEXTURE_INTERNAL_FORMAT_ANGLE, GL_BGRA_EXT,
            EGL_TEXTURE_FORMAT,                EGL_TEXTURE_RGB,
            EGL_TEXTURE_TYPE_ANGLE,            GL_UNSIGNED_BYTE,
            EGL_NONE,                          EGL_NONE,
        };
        // clang-format on

        EGLSurface pbuffer = eglCreatePbufferFromClientBuffer(mDisplay, EGL_IOSURFACE_ANGLE,
                                                              ioSurface.get(), mConfig, attribs);
        EXPECT_EQ(EGL_NO_SURFACE, pbuffer);
        EXPECT_EGL_ERROR(EGL_BAD_ATTRIBUTE);
    }

    // EGL_WIDTH must be at least 1
    {
        // clang-format off
        const EGLint attribs[] = {
            EGL_WIDTH,                         0,
            EGL_HEIGHT,                        10,
            EGL_IOSURFACE_PLANE_ANGLE,         0,
            EGL_TEXTURE_TARGET,                getTextureTarget(),
            EGL_TEXTURE_INTERNAL_FORMAT_ANGLE, GL_BGRA_EXT,
            EGL_TEXTURE_FORMAT,                EGL_TEXTURE_RGBA,
            EGL_TEXTURE_TYPE_ANGLE,            GL_UNSIGNED_BYTE,
            EGL_NONE,                          EGL_NONE,
        };
        // clang-format on

        EGLSurface pbuffer = eglCreatePbufferFromClientBuffer(mDisplay, EGL_IOSURFACE_ANGLE,
                                                              ioSurface.get(), mConfig, attribs);
        EXPECT_EQ(EGL_NO_SURFACE, pbuffer);
        EXPECT_EGL_ERROR(EGL_BAD_ATTRIBUTE);
    }

    // EGL_WIDTH must be at most the width of the IOSurface
    {
        // clang-format off
        const EGLint attribs[] = {
            EGL_WIDTH,                         11,
            EGL_HEIGHT,                        10,
            EGL_IOSURFACE_PLANE_ANGLE,         0,
            EGL_TEXTURE_TARGET,                getTextureTarget(),
            EGL_TEXTURE_INTERNAL_FORMAT_ANGLE, GL_BGRA_EXT,
            EGL_TEXTURE_FORMAT,                EGL_TEXTURE_RGBA,
            EGL_TEXTURE_TYPE_ANGLE,            GL_UNSIGNED_BYTE,
            EGL_NONE,                          EGL_NONE,
        };
        // clang-format on

        EGLSurface pbuffer = eglCreatePbufferFromClientBuffer(mDisplay, EGL_IOSURFACE_ANGLE,
                                                              ioSurface.get(), mConfig, attribs);
        EXPECT_EQ(EGL_NO_SURFACE, pbuffer);
        EXPECT_EGL_ERROR(EGL_BAD_ATTRIBUTE);
    }

    // EGL_HEIGHT must be at least 1
    {
        // clang-format off
        const EGLint attribs[] = {
            EGL_WIDTH,                         10,
            EGL_HEIGHT,                        0,
            EGL_IOSURFACE_PLANE_ANGLE,         0,
            EGL_TEXTURE_TARGET,                getTextureTarget(),
            EGL_TEXTURE_INTERNAL_FORMAT_ANGLE, GL_BGRA_EXT,
            EGL_TEXTURE_FORMAT,                EGL_TEXTURE_RGBA,
            EGL_TEXTURE_TYPE_ANGLE,            GL_UNSIGNED_BYTE,
            EGL_NONE,                          EGL_NONE,
        };
        // clang-format on

        EGLSurface pbuffer = eglCreatePbufferFromClientBuffer(mDisplay, EGL_IOSURFACE_ANGLE,
                                                              ioSurface.get(), mConfig, attribs);
        EXPECT_EQ(EGL_NO_SURFACE, pbuffer);
        EXPECT_EGL_ERROR(EGL_BAD_ATTRIBUTE);
    }

    // EGL_HEIGHT must be at most the height of the IOSurface
    {
        // clang-format off
        const EGLint attribs[] = {
            EGL_WIDTH,                         10,
            EGL_HEIGHT,                        11,
            EGL_IOSURFACE_PLANE_ANGLE,         0,
            EGL_TEXTURE_TARGET,                getTextureTarget(),
            EGL_TEXTURE_INTERNAL_FORMAT_ANGLE, GL_BGRA_EXT,
            EGL_TEXTURE_FORMAT,                EGL_TEXTURE_RGBA,
            EGL_TEXTURE_TYPE_ANGLE,            GL_UNSIGNED_BYTE,
            EGL_NONE,                          EGL_NONE,
        };
        // clang-format on

        EGLSurface pbuffer = eglCreatePbufferFromClientBuffer(mDisplay, EGL_IOSURFACE_ANGLE,
                                                              ioSurface.get(), mConfig, attribs);
        EXPECT_EQ(EGL_NO_SURFACE, pbuffer);
        EXPECT_EGL_ERROR(EGL_BAD_ATTRIBUTE);
    }

    // EGL_TEXTURE_FORMAT must be equal to the config's texture target
    {
        EGLint target      = getTextureTarget();
        EGLint wrongTarget = 0;
        switch (target)
        {
            case EGL_TEXTURE_RECTANGLE_ANGLE:
                wrongTarget = EGL_TEXTURE_2D;
                break;
            case EGL_TEXTURE_2D:
                wrongTarget = EGL_TEXTURE_RECTANGLE_ANGLE;
                break;
            default:
                break;
        }
        // clang-format off
        const EGLint attribs[] = {
            EGL_WIDTH,                         10,
            EGL_HEIGHT,                        10,
            EGL_IOSURFACE_PLANE_ANGLE,         0,
            EGL_TEXTURE_TARGET,                wrongTarget,
            EGL_TEXTURE_INTERNAL_FORMAT_ANGLE, GL_BGRA_EXT,
            EGL_TEXTURE_FORMAT,                EGL_TEXTURE_RGBA,
            EGL_TEXTURE_TYPE_ANGLE,            GL_UNSIGNED_BYTE,
            EGL_NONE,                          EGL_NONE,
        };
        // clang-format on

        EGLSurface pbuffer = eglCreatePbufferFromClientBuffer(mDisplay, EGL_IOSURFACE_ANGLE,
                                                              ioSurface.get(), mConfig, attribs);
        EXPECT_EQ(EGL_NO_SURFACE, pbuffer);
        EXPECT_EGL_ERROR(EGL_BAD_ATTRIBUTE);
    }

    // EGL_IOSURFACE_PLANE_ANGLE must be at least 0
    {
        // clang-format off
        const EGLint attribs[] = {
            EGL_WIDTH,                         10,
            EGL_HEIGHT,                        10,
            EGL_IOSURFACE_PLANE_ANGLE,         -1,
            EGL_TEXTURE_TARGET,                getTextureTarget(),
            EGL_TEXTURE_INTERNAL_FORMAT_ANGLE, GL_BGRA_EXT,
            EGL_TEXTURE_FORMAT,                EGL_TEXTURE_RGBA,
            EGL_TEXTURE_TYPE_ANGLE,            GL_UNSIGNED_BYTE,
            EGL_NONE,                          EGL_NONE,
        };
        // clang-format on

        EGLSurface pbuffer = eglCreatePbufferFromClientBuffer(mDisplay, EGL_IOSURFACE_ANGLE,
                                                              ioSurface.get(), mConfig, attribs);
        EXPECT_EQ(EGL_NO_SURFACE, pbuffer);
        EXPECT_EGL_ERROR(EGL_BAD_ATTRIBUTE);
    }

    // EGL_IOSURFACE_PLANE_ANGLE must less than the number of planes of the IOSurface
    {
        // clang-format off
        const EGLint attribs[] = {
            EGL_WIDTH,                         10,
            EGL_HEIGHT,                        10,
            EGL_IOSURFACE_PLANE_ANGLE,         1,
            EGL_TEXTURE_TARGET,                getTextureTarget(),
            EGL_TEXTURE_INTERNAL_FORMAT_ANGLE, GL_BGRA_EXT,
            EGL_TEXTURE_FORMAT,                EGL_TEXTURE_RGBA,
            EGL_TEXTURE_TYPE_ANGLE,            GL_UNSIGNED_BYTE,
            EGL_NONE,                          EGL_NONE,
        };
        // clang-format on

        EGLSurface pbuffer = eglCreatePbufferFromClientBuffer(mDisplay, EGL_IOSURFACE_ANGLE,
                                                              ioSurface.get(), mConfig, attribs);
        EXPECT_EQ(EGL_NO_SURFACE, pbuffer);
        EXPECT_EGL_ERROR(EGL_BAD_ATTRIBUTE);
    }

    // The internal format / type most be listed in the table
    {
        // clang-format off
        const EGLint attribs[] = {
            EGL_WIDTH,                         10,
            EGL_HEIGHT,                        10,
            EGL_IOSURFACE_PLANE_ANGLE,         0,
            EGL_TEXTURE_TARGET,                getTextureTarget(),
            EGL_TEXTURE_INTERNAL_FORMAT_ANGLE, GL_RGBA,
            EGL_TEXTURE_FORMAT,                EGL_TEXTURE_RGBA,
            EGL_TEXTURE_TYPE_ANGLE,            GL_FLOAT,
            EGL_NONE,                          EGL_NONE,
        };
        // clang-format on

        EGLSurface pbuffer = eglCreatePbufferFromClientBuffer(mDisplay, EGL_IOSURFACE_ANGLE,
                                                              ioSurface.get(), mConfig, attribs);
        EXPECT_EQ(EGL_NO_SURFACE, pbuffer);
        EXPECT_EGL_ERROR(EGL_BAD_ATTRIBUTE);
    }
}

// Test IOSurface pbuffers can be made current
TEST_P(IOSurfaceClientBufferTest, MakeCurrent)
{
    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());

    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(10, 10, 'BGRA', 4);

    EGLSurface pbuffer;
    createIOSurfacePbuffer(ioSurface, 10, 10, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, &pbuffer);

    EGLContext context = getEGLWindow()->getContext();
    EGLBoolean result  = eglMakeCurrent(mDisplay, pbuffer, pbuffer, context);
    EXPECT_EGL_TRUE(result);
    EXPECT_EGL_SUCCESS();
    // The test harness expects the EGL state to be restored before the test exits.
    result = eglMakeCurrent(mDisplay, getEGLWindow()->getSurface(), getEGLWindow()->getSurface(),
                            context);
    EXPECT_EGL_TRUE(result);
    EXPECT_EGL_SUCCESS();
}

// Test reading from BGRX8888 IOSurfaces with bound texture's base/max level set to zero.
// This to verify that changing base/level shouldn't delete the binding.
// bug: https://bugs.chromium.org/p/chromium/issues/detail?id=1337324
TEST_P(IOSurfaceClientBufferTest, ReadFromBGRX8888IOSurfaceWithTexBaseMaxLevelSetToZero)
{
    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);

    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(1, 1, 'BGRA', 4);

    GLColor color(3, 2, 1, 4);
    doSampleTestWithExtraSteps(ioSurface, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &color, sizeof(color),
                               R | G | B, /* extra steps */ [this] {
                                   glTexParameteri(getGLTextureTarget(), GL_TEXTURE_BASE_LEVEL, 0);
                                   glTexParameteri(getGLTextureTarget(), GL_TEXTURE_MAX_LEVEL, 0);
                               });
}

// Test that the following scenario works:
// - change IOSurface bound texture's max level to 0.
// - attach IOSurface bound texture to a FBO 1.
// - bind FBO 1
// - clear FBO 1 -> this should trigger render targets initialization in backends.
// - bind FBO 0.
// - draw IOSurface bound texture to FBO 0.
//   -> In the past, this could trigger the texture's render targets invalidation in metal backend.
//   See https://issues.chromium.org/issues/335353385
// - bind FBO 1
// - blit FBO 0 to FBO 1.
//   -> this will reconstruct render pass descriptor in metal backend.
// - flush to restart render encoder with new render pass descriptor.
// - draw.
TEST_P(IOSurfaceClientBufferTest, SetMaxLevelWouldInvalidateRenderTargetBug)
{
    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);

    ANGLE_GL_PROGRAM(blueProgram, angle::essl1_shaders::vs::Simple(),
                     angle::essl1_shaders::fs::Blue());

    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(1, 1, 'BGRA', 4);

    GLTexture texture;
    glBindTexture(getGLTextureTarget(), texture);

    // Bind the IOSurface to a texture.
    EGLSurface pbuffer;
    bindIOSurfaceToTexture(ioSurface, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &pbuffer, &texture);

    // 1. Change the texture's max level to 0.
    glTexParameteri(getGLTextureTarget(), GL_TEXTURE_MAX_LEVEL, 0);
    EXPECT_GL_NO_ERROR();

    // 2. Attach IOSurface bound texture to a FBO and clear it.
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    EXPECT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, getGLTextureTarget(), texture, 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glClearColor(1.0f / 255.0f, 2.0f / 255.0f, 3.0f / 255.0f, 4.0f / 255.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // 3. Draw IOSurface bound texture to default FBO.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    doSampleTestWithTexture(texture, R | G | B);

    // 3. Draw to custom FBO again
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glClear(GL_COLOR_BUFFER_BIT);
    // Using a blit is important: it will trigger render pass reconstruction in
    // metal backend due to DIRTY_BIT_COLOR_BUFFER_CONTENTS_0 dirty bit.
    glBlitFramebuffer(0, 0, 1, 1, 0, 0, 1, 1, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glFlush();  // flush so that render encoder will be recreated again in metal backend.
    glUseProgram(blueProgram);
    drawQuad(blueProgram, angle::essl1_shaders::PositionAttrib(), 0.5f);
    glFlush();

    // Expect the final color to be accumulated color
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor(1, 2, 255, 255));
}

// Test using GL_EXT_multisampled_render_to_texture to render to BGRX IOSurface.
TEST_P(IOSurfaceClientBufferTest, MultisampledRenderToTextureBGRX)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_multisampled_render_to_texture"));

    ANGLE_SKIP_TEST_IF(!hasIOSurfaceExt());

    ANGLE_GL_PROGRAM(colorProgram, angle::essl1_shaders::vs::Simple(),
                     angle::essl1_shaders::fs::UniformColor());

    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(1, 1, 'BGRA', 4);

    GLTexture texture;
    glBindTexture(getGLTextureTarget(), texture);

    // Bind the IOSurface to a texture.
    EGLSurface pbuffer;
    bindIOSurfaceToTexture(ioSurface, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &pbuffer, &texture);

    // Attach IOSurface bound texture to a single sampled FBO and clear it.
    GLFramebuffer singleSampledFbo;
    glBindFramebuffer(GL_FRAMEBUFFER, singleSampledFbo);
    EXPECT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, getGLTextureTarget(), texture, 0);
    glClearColor(1.0f / 255.0f, 2.0f / 255.0f, 3.0f / 255.0f, 4.0f / 255.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Attach IOSurface to a multisampled FBO and draw translucent blue color
    GLFramebuffer multisampledSampledFbo;
    glBindFramebuffer(GL_FRAMEBUFFER, multisampledSampledFbo);
    EXPECT_GL_NO_ERROR();
    glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, getGLTextureTarget(),
                                         texture, 0, 4);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glUseProgram(colorProgram);
    const GLint colorUniformLocation =
        glGetUniformLocation(colorProgram, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);
    glUniform4f(colorUniformLocation, 0, 0, 1, 0.5f);
    drawQuad(colorProgram, angle::essl1_shaders::PositionAttrib(), 0.5f);

    // Expect the final color to be accumulated color
    glBindFramebuffer(GL_FRAMEBUFFER, singleSampledFbo);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor(1, 2, 255, 255));
}

// TODO(cwallez@chromium.org): Test setting width and height to less than the IOSurface's work as
// expected.

ANGLE_INSTANTIATE_TEST(IOSurfaceClientBufferTest,
                       ES2_OPENGL(),
                       ES3_OPENGL(),
                       ES2_VULKAN_SWIFTSHADER(),
                       ES3_VULKAN_SWIFTSHADER(),
                       ES2_METAL(),
                       ES3_METAL(),
                       ES3_METAL()
                           .enable(Feature::EnableMultisampledRenderToTextureOnNonTilers)
                           .enable(Feature::EmulateDontCareLoadWithRandomClear));

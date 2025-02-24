//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ImageTest:
//   Tests the correctness of eglImage.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/MultiThreadSteps.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"
#include "util/test_utils.h"

#include "common/android_util.h"

#if defined(ANGLE_PLATFORM_ANDROID) && __ANDROID_API__ >= 29
#    define ANGLE_AHARDWARE_BUFFER_SUPPORT
// NDK header file for access to Android Hardware Buffers
#    include <android/hardware_buffer.h>
#    define ANGLE_AHARDWARE_BUFFER_LOCK_PLANES_SUPPORT
#endif

#if defined(ANGLE_PLATFORM_ANDROID) && __ANDROID_API__ >= 33
constexpr bool kHasAHBFrontBufferUsageSupport = 1;
#else
[[maybe_unused]] constexpr bool kHasAHBFrontBufferUsageSupport = 0;
#endif

namespace angle
{
namespace
{
constexpr char kOESExt[]                         = "GL_OES_EGL_image";
constexpr char kExternalExt[]                    = "GL_OES_EGL_image_external";
constexpr char kExternalESSL3Ext[]               = "GL_OES_EGL_image_external_essl3";
constexpr char kYUVInternalFormatExt[]           = "GL_ANGLE_yuv_internal_format";
constexpr char kYUVTargetExt[]                   = "GL_EXT_YUV_target";
constexpr char kRGBXInternalFormatExt[]          = "GL_ANGLE_rgbx_internal_format";
constexpr char kBaseExt[]                        = "EGL_KHR_image_base";
constexpr char k2DTextureExt[]                   = "EGL_KHR_gl_texture_2D_image";
constexpr char k3DTextureExt[]                   = "EGL_KHR_gl_texture_3D_image";
constexpr char kPixmapExt[]                      = "EGL_KHR_image_pixmap";
constexpr char kRenderbufferExt[]                = "EGL_KHR_gl_renderbuffer_image";
constexpr char kCubemapExt[]                     = "EGL_KHR_gl_texture_cubemap_image";
constexpr char kImageGLColorspaceExt[]           = "EGL_EXT_image_gl_colorspace";
constexpr char kEGLImageArrayExt[]               = "GL_EXT_EGL_image_array";
constexpr char kEGLAndroidImageNativeBufferExt[] = "EGL_ANDROID_image_native_buffer";
constexpr char kEGLImageStorageExt[]             = "GL_EXT_EGL_image_storage";
constexpr char kEGLImageStorageCompressionExt[]  = "GL_EXT_EGL_image_storage_compression";
constexpr char kTextureStorageCompressionExt[]   = "GL_EXT_texture_storage_compression";
constexpr EGLint kDefaultAttribs[]               = {
    EGL_IMAGE_PRESERVED,
    EGL_TRUE,
    EGL_NONE,
};
constexpr EGLint kColorspaceAttribs[] = {
    EGL_IMAGE_PRESERVED, EGL_TRUE, EGL_GL_COLORSPACE, EGL_GL_COLORSPACE_SRGB_KHR, EGL_NONE,
};
constexpr EGLint kNativeClientBufferAttribs_RGBA8_Texture[] = {
    EGL_WIDTH,
    1,
    EGL_HEIGHT,
    1,
    EGL_RED_SIZE,
    8,
    EGL_GREEN_SIZE,
    8,
    EGL_BLUE_SIZE,
    8,
    EGL_ALPHA_SIZE,
    8,
    EGL_NATIVE_BUFFER_USAGE_ANDROID,
    EGL_NATIVE_BUFFER_USAGE_TEXTURE_BIT_ANDROID,
    EGL_NONE};
constexpr EGLint kNativeClientBufferAttribs_RGBA8_Renderbuffer[] = {
    EGL_WIDTH,
    1,
    EGL_HEIGHT,
    1,
    EGL_RED_SIZE,
    8,
    EGL_GREEN_SIZE,
    8,
    EGL_BLUE_SIZE,
    8,
    EGL_ALPHA_SIZE,
    8,
    EGL_NATIVE_BUFFER_USAGE_ANDROID,
    EGL_NATIVE_BUFFER_USAGE_RENDERBUFFER_BIT_ANDROID,
    EGL_NONE};
// Color data in linear and sRGB colorspace
// 2D texture data
GLubyte kLinearColor[] = {132, 55, 219, 255};
GLubyte kSrgbColor[]   = {190, 128, 238, 255};
// 3D texture data
GLubyte kLinearColor3D[] = {131, 242, 100, 255, 201, 89, 133, 255};
GLubyte kSrgbColor3D[]   = {190, 249, 168, 255, 230, 159, 191, 255};
// Cubemap texture data
GLubyte kLinearColorCube[] = {75, 135, 205, 255, 201, 89,  133, 255, 111, 201, 108, 255,
                              30, 90,  230, 255, 180, 210, 70,  255, 77,  111, 99,  255};
GLubyte kSrgbColorCube[]   = {148, 192, 232, 255, 230, 159, 191, 255, 176, 230, 174, 255,
                              96,  160, 244, 255, 219, 234, 143, 255, 149, 176, 167, 255};
GLfloat kCubeFaceX[]       = {1.0, -1.0, 0.0, 0.0, 0.0, 0.0};
GLfloat kCubeFaceY[]       = {0.0, 0.0, 1.0, -1.0, 0.0, 0.0};
GLfloat kCubeFaceZ[]       = {0.0, 0.0, 0.0, 0.0, 1.0, -1.0};

constexpr int kColorspaceAttributeIndex     = 2;
constexpr int k3DColorspaceAttributeIndex   = 4;
constexpr int kTextureZOffsetAttributeIndex = 1;
constexpr size_t kCubeFaceCount             = 6;

constexpr int AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM  = 1;
constexpr int AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM  = 2;
constexpr int AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM    = 3;
constexpr int AHARDWAREBUFFER_FORMAT_D24_UNORM       = 0x31;
constexpr int AHARDWAREBUFFER_FORMAT_Y8Cr8Cb8_420_SP = 0x11;
constexpr int AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420    = 0x23;
constexpr int AHARDWAREBUFFER_FORMAT_YV12            = 0x32315659;

[[maybe_unused]] constexpr uint64_t ANGLE_AHARDWAREBUFFER_USAGE_FRONT_BUFFER = (1ULL << 32);

}  // anonymous namespace

class ImageTest : public ANGLETest<>
{
  protected:
    ImageTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    const char *getVS() const
    {
        return R"(precision highp float;
attribute vec4 position;
varying vec2 texcoord;

void main()
{
    gl_Position = position;
    texcoord = (position.xy * 0.5) + 0.5;
    texcoord.y = 1.0 - texcoord.y;
})";
    }

    const char *getVS2DArray() const
    {
        return R"(#version 300 es
out vec2 texcoord;
in vec4 position;
void main()
{
    gl_Position = vec4(position.xy, 0.0, 1.0);
    texcoord = (position.xy * 0.5) + 0.5;
})";
    }

    const char *getVSCube() const
    {
        return R"(#version 300 es
in vec4 position;
void main()
{
    gl_Position = vec4(position.xy, 0.0, 1.0);
})";
    }

    const char *getVSCubeArray() const
    {
        return R"(#version 310 es
in vec4 position;
void main()
{
    gl_Position = vec4(position.xy, 0.0, 1.0);
})";
    }

    const char *getVSESSL3() const
    {
        return R"(#version 300 es
precision highp float;
in vec4 position;
out vec2 texcoord;

void main()
{
    gl_Position = position;
    texcoord = (position.xy * 0.5) + 0.5;
    texcoord.y = 1.0 - texcoord.y;
})";
    }

    const char *getTextureFS() const
    {
        return R"(precision highp float;
uniform sampler2D tex;
varying vec2 texcoord;

void main()
{
    gl_FragColor = texture2D(tex, texcoord);
})";
    }

    const char *getTexture2DArrayFS() const
    {
        return R"(#version 300 es
precision highp float;
uniform highp sampler2DArray tex2DArray;
uniform uint layer;
in vec2 texcoord;
out vec4 fragColor;
void main()
{
    fragColor = texture(tex2DArray, vec3(texcoord.x, texcoord.y, float(layer)));
})";
    }

    const char *getTextureCubeFS() const
    {
        return R"(#version 300 es
precision highp float;
uniform highp samplerCube texCube;
uniform vec3 faceCoord;
out vec4 fragColor;
void main()
{
    fragColor = texture(texCube, faceCoord);
})";
    }

    const char *getTextureCubeArrayFS() const
    {
        return R"(#version 310 es
#extension GL_OES_texture_cube_map_array : require
precision highp float;
uniform highp samplerCubeArray texCubeArray;
uniform vec3 faceCoord;
uniform uint layer;
out vec4 fragColor;
void main()
{
    fragColor = texture(texCubeArray, vec4(faceCoord, float(layer)));
})";
    }

    const char *getTextureExternalFS() const
    {
        return R"(#extension GL_OES_EGL_image_external : require
precision highp float;
uniform samplerExternalOES tex;
varying vec2 texcoord;

void main()
{
    gl_FragColor = texture2D(tex, texcoord);
})";
    }

    const char *getTextureExternalESSL3FS() const
    {
        return R"(#version 300 es
#extension GL_OES_EGL_image_external_essl3 : require
precision highp float;
uniform samplerExternalOES tex;
in vec2 texcoord;
out vec4 color;

void main()
{
    color = texture(tex, texcoord);
})";
    }

    const char *getTextureYUVFS() const
    {
        return R"(#version 300 es
#extension GL_EXT_YUV_target : require
precision highp float;
uniform __samplerExternal2DY2YEXT tex;
in vec2 texcoord;
out vec4 color;

void main()
{
    color = texture(tex, texcoord);
})";
    }

    const char *getRenderYUVFS() const
    {
        return R"(#version 300 es
#extension GL_EXT_YUV_target : require
precision highp float;
uniform vec4 u_color;
layout (yuv) out vec4 color;

void main()
{
    color = u_color;
})";
    }

    void testSetUp() override
    {
        mTextureProgram = CompileProgram(getVS(), getTextureFS());
        if (mTextureProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        mTextureUniformLocation = glGetUniformLocation(mTextureProgram, "tex");

        if (getClientMajorVersion() >= 3)
        {
            m2DArrayTextureProgram = CompileProgram(getVS2DArray(), getTexture2DArrayFS());
            if (m2DArrayTextureProgram == 0)
            {
                FAIL() << "shader compilation failed.";
            }

            m2DArrayTextureUniformLocation =
                glGetUniformLocation(m2DArrayTextureProgram, "tex2DArray");
            m2DArrayTextureLayerUniformLocation =
                glGetUniformLocation(m2DArrayTextureProgram, "layer");
        }

        if (IsGLExtensionEnabled("GL_OES_EGL_image_external"))
        {
            mTextureExternalProgram = CompileProgram(getVS(), getTextureExternalFS());
            ASSERT_NE(0u, mTextureExternalProgram) << "shader compilation failed.";

            mTextureExternalUniformLocation = glGetUniformLocation(mTextureExternalProgram, "tex");
        }

        if (IsGLExtensionEnabled("GL_OES_EGL_image_external_essl3"))
        {
            mTextureExternalESSL3Program =
                CompileProgram(getVSESSL3(), getTextureExternalESSL3FS());
            ASSERT_NE(0u, mTextureExternalESSL3Program) << "shader compilation failed.";

            mTextureExternalESSL3UniformLocation =
                glGetUniformLocation(mTextureExternalESSL3Program, "tex");
        }

        if (IsGLExtensionEnabled(kYUVTargetExt))
        {
            mTextureYUVProgram = CompileProgram(getVSESSL3(), getTextureYUVFS());
            ASSERT_NE(0u, mTextureYUVProgram) << "shader compilation failed.";

            mTextureYUVUniformLocation = glGetUniformLocation(mTextureYUVProgram, "tex");

            mRenderYUVProgram = CompileProgram(getVSESSL3(), getRenderYUVFS());
            ASSERT_NE(0u, mRenderYUVProgram) << "shader compilation failed.";

            mRenderYUVUniformLocation = glGetUniformLocation(mRenderYUVProgram, "u_color");
        }

        if (IsGLExtensionEnabled(kEGLImageStorageExt))
        {
            mCubeTextureProgram = CompileProgram(getVSCube(), getTextureCubeFS());
            if (mCubeTextureProgram == 0)
            {
                FAIL() << "shader compilation failed.";
            }
            mCubeTextureUniformLocation = glGetUniformLocation(mCubeTextureProgram, "texCube");
            mCubeTextureFaceCoordUniformLocation =
                glGetUniformLocation(mCubeTextureProgram, "faceCoord");

            if ((getClientMajorVersion() >= 3 && getClientMinorVersion() >= 1) &&
                IsGLExtensionEnabled("GL_EXT_texture_cube_map_array"))
            {
                mCubeArrayTextureProgram =
                    CompileProgram(getVSCubeArray(), getTextureCubeArrayFS());
                if (mCubeArrayTextureProgram == 0)
                {
                    FAIL() << "shader compilation failed.";
                }
                mCubeArrayTextureUniformLocation =
                    glGetUniformLocation(mCubeArrayTextureProgram, "texCubeArray");
                mCubeArrayTextureFaceCoordUniformLocation =
                    glGetUniformLocation(mCubeArrayTextureProgram, "faceCoord");
                mCubeArrayTextureLayerUniformLocation =
                    glGetUniformLocation(mCubeArrayTextureProgram, "layer");
            }
        }

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override
    {
        glDeleteProgram(mTextureProgram);
        glDeleteProgram(mTextureExternalProgram);
        glDeleteProgram(mTextureExternalESSL3Program);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        ASSERT_GL_NO_ERROR();
    }

    // 1) For tests that sample from EGLImages with colorspace override -
    //    1) Always upload color values in sRGB colorspace
    //    2) The image should be treated as if it was an sRGB image resulting in
    //       the sampled value to be to decoded to linear colorspace
    //
    // 2) For tests that render to EGLImages with colorspace override -
    //    1) Always upload color values in linear colorspace
    //    2) The image should be treated as if it was an sRGB image resulting in
    //       the rendered color to be encoded in sRGB colorspace

    enum class EglImageUsage
    {
        Sampling,
        Rendering
    };

    bool attribListHasSrgbColorspace(const EGLint *attribs,
                                     const int colorspaceAttributeIndex) const
    {
        return (attribs[colorspaceAttributeIndex] == EGL_GL_COLORSPACE &&
                attribs[colorspaceAttributeIndex + 1] == EGL_GL_COLORSPACE_SRGB_KHR);
    }

    GLubyte *getExpected2DColorForAttribList(const EGLint *attribs,
                                             EglImageUsage usage = EglImageUsage::Sampling) const
    {
        const bool srgbColorspace = attribListHasSrgbColorspace(attribs, kColorspaceAttributeIndex);
        return (usage == EglImageUsage::Sampling) ? (srgbColorspace ? kLinearColor : kSrgbColor)
                                                  : (srgbColorspace ? kSrgbColor : kLinearColor);
    }

    GLubyte *getExpected3DColorForAttribList(const EGLint *attribs,
                                             EglImageUsage usage = EglImageUsage::Sampling) const
    {
        const bool srgbColorspace =
            attribListHasSrgbColorspace(attribs, k3DColorspaceAttributeIndex);
        return (usage == EglImageUsage::Sampling)
                   ? (srgbColorspace ? kLinearColor3D : kSrgbColor3D)
                   : (srgbColorspace ? kSrgbColor3D : kLinearColor3D);
    }

    GLubyte *getExpectedCubeColorForAttribList(const EGLint *attribs,
                                               EglImageUsage usage = EglImageUsage::Sampling) const
    {
        const bool srgbColorspace = attribListHasSrgbColorspace(attribs, kColorspaceAttributeIndex);
        return (usage == EglImageUsage::Sampling)
                   ? (srgbColorspace ? kLinearColorCube : kSrgbColorCube)
                   : (srgbColorspace ? kSrgbColorCube : kLinearColorCube);
    }

    void createEGLImage2DTextureStorage(size_t width,
                                        size_t height,
                                        GLenum format,
                                        const GLint *attribs,
                                        GLTexture &sourceTexture,
                                        EGLImageKHR *outSourceImage)
    {

        glBindTexture(GL_TEXTURE_2D, sourceTexture);
        glTexStorageAttribs2DEXT(GL_TEXTURE_2D, 1, format, static_cast<GLsizei>(width),
                                 static_cast<GLsizei>(height), attribs);

        ASSERT_GL_NO_ERROR();
        // Disable mipmapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        ASSERT_GL_NO_ERROR();

        // Create an image from the source texture
        EGLWindow *window = getEGLWindow();

        EGLImageKHR image =
            eglCreateImageKHR(window->getDisplay(), window->getContext(), EGL_GL_TEXTURE_2D_KHR,
                              reinterpretHelper<EGLClientBuffer>(sourceTexture), nullptr);

        ASSERT_EGL_SUCCESS();

        *outSourceImage = image;
    }

    void createEGLImage2DTextureSource(size_t width,
                                       size_t height,
                                       GLenum format,
                                       GLenum type,
                                       const EGLint *attribs,
                                       void *data,
                                       GLTexture &sourceTexture,
                                       EGLImageKHR *outSourceImage)
    {
        // Create a source 2D texture
        glBindTexture(GL_TEXTURE_2D, sourceTexture);

        glTexImage2D(GL_TEXTURE_2D, 0, format, static_cast<GLsizei>(width),
                     static_cast<GLsizei>(height), 0, format, type, data);

        // Disable mipmapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        ASSERT_GL_NO_ERROR();

        // Create an image from the source texture
        EGLWindow *window = getEGLWindow();

        EGLImageKHR image =
            eglCreateImageKHR(window->getDisplay(), window->getContext(), EGL_GL_TEXTURE_2D_KHR,
                              reinterpretHelper<EGLClientBuffer>(sourceTexture), attribs);

        ASSERT_EGL_SUCCESS();

        *outSourceImage = image;
    }

    void createEGLImageCubemapTextureSource(size_t width,
                                            size_t height,
                                            GLenum format,
                                            GLenum type,
                                            const EGLint *attribs,
                                            uint8_t *data,
                                            size_t dataStride,
                                            EGLenum imageTarget,
                                            GLTexture &sourceTexture,
                                            EGLImageKHR *outSourceImage)
    {
        // Create a source cube map texture
        glBindTexture(GL_TEXTURE_CUBE_MAP, sourceTexture);

        for (GLenum faceIdx = 0; faceIdx < 6; faceIdx++)
        {
            glTexImage2D(faceIdx + GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, format,
                         static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, format, type,
                         data + (faceIdx * dataStride));
        }

        // Disable mipmapping
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        ASSERT_GL_NO_ERROR();

        // Create an image from the source texture
        EGLWindow *window = getEGLWindow();

        EGLImageKHR image =
            eglCreateImageKHR(window->getDisplay(), window->getContext(), imageTarget,
                              reinterpretHelper<EGLClientBuffer>(sourceTexture), attribs);

        ASSERT_EGL_SUCCESS();

        *outSourceImage = image;
    }

    void createEGLImage3DTextureSource(size_t width,
                                       size_t height,
                                       size_t depth,
                                       GLenum format,
                                       GLenum type,
                                       const EGLint *attribs,
                                       void *data,
                                       GLTexture &sourceTexture,
                                       EGLImageKHR *outSourceImage)
    {
        // Create a source 3D texture
        glBindTexture(GL_TEXTURE_3D, sourceTexture);

        glTexImage3D(GL_TEXTURE_3D, 0, format, static_cast<GLsizei>(width),
                     static_cast<GLsizei>(height), static_cast<GLsizei>(depth), 0, format, type,
                     data);

        // Disable mipmapping
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        ASSERT_GL_NO_ERROR();

        // Create an image from the source texture
        EGLWindow *window = getEGLWindow();

        EGLImageKHR image =
            eglCreateImageKHR(window->getDisplay(), window->getContext(), EGL_GL_TEXTURE_3D_KHR,
                              reinterpretHelper<EGLClientBuffer>(sourceTexture), attribs);

        ASSERT_EGL_SUCCESS();

        *outSourceImage = image;
    }

    void createEGLImageRenderbufferSource(size_t width,
                                          size_t height,
                                          GLenum internalFormat,
                                          const EGLint *attribs,
                                          GLRenderbuffer &sourceRenderbuffer,
                                          EGLImageKHR *outSourceImage)
    {
        // Create a source renderbuffer
        glBindRenderbuffer(GL_RENDERBUFFER, sourceRenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, static_cast<GLsizei>(width),
                              static_cast<GLsizei>(height));

        // Create an image from the source renderbuffer
        EGLWindow *window = getEGLWindow();

        EGLImageKHR image =
            eglCreateImageKHR(window->getDisplay(), window->getContext(), EGL_GL_RENDERBUFFER_KHR,
                              reinterpretHelper<EGLClientBuffer>(sourceRenderbuffer), attribs);

        ASSERT_EGL_SUCCESS();

        *outSourceImage = image;
    }

    void createEGLImageTargetTexture2D(EGLImageKHR image, GLTexture &targetTexture)
    {
        // Create a target texture from the image
        glBindTexture(GL_TEXTURE_2D, targetTexture);
        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);

        // Disable mipmapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        ASSERT_GL_NO_ERROR();
    }

    void createEGLImageTargetTexture2DArray(EGLImageKHR image, GLTexture &targetTexture)
    {
        // Create a target texture from the image
        glBindTexture(GL_TEXTURE_2D_ARRAY, targetTexture);
        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D_ARRAY, image);

        // Disable mipmapping
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        ASSERT_GL_NO_ERROR();
    }

    void createEGLImageTargetTextureExternal(EGLImageKHR image, GLuint targetTexture)
    {
        // Create a target texture from the image
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, targetTexture);
        glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, image);

        // Disable mipmapping
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        ASSERT_GL_NO_ERROR();
    }

    void createEGLImageTargetTextureStorage(EGLImageKHR image,
                                            GLenum targetType,
                                            GLuint targetTexture,
                                            const GLint *attribs)
    {
        // Create a target texture from the image
        glBindTexture(targetType, targetTexture);
        glEGLImageTargetTexStorageEXT(targetType, image, attribs);

        ASSERT_GL_NO_ERROR();

        // Disable mipmapping
        glTexParameteri(targetType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(targetType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        ASSERT_GL_NO_ERROR();
    }

    size_t getLayerPitch(size_t height, size_t rowStride)
    {
        // Undocumented alignment of layer stride.  This is potentially platform dependent, but
        // allows functionality to be tested.
        constexpr size_t kLayerAlignment = 4096;

        const size_t layerSize = height * rowStride;
        return (layerSize + kLayerAlignment - 1) & ~(kLayerAlignment - 1);
    }

    struct AHBPlaneData
    {
        const GLubyte *data;
        size_t bytesPerPixel;
    };

    bool writeAHBData(AHardwareBuffer *aHardwareBuffer,
                      size_t width,
                      size_t height,
                      size_t depth,
                      bool isYUV,
                      const std::vector<AHBPlaneData> &data)
    {
#if defined(ANGLE_AHARDWARE_BUFFER_SUPPORT)
        ASSERT(!data.empty());
#    if defined(ANGLE_AHARDWARE_BUFFER_LOCK_PLANES_SUPPORT)
        AHardwareBuffer_Planes planeInfo;
        int res = AHardwareBuffer_lockPlanes(
            aHardwareBuffer, AHARDWAREBUFFER_USAGE_CPU_WRITE_RARELY, -1, nullptr, &planeInfo);
        if (res != 0)
        {
            WARN() << "AHardwareBuffer_lockPlanes failed";
            return false;
        }

        EXPECT_EQ(data.size(), planeInfo.planeCount);

        for (size_t planeIdx = 0; planeIdx < data.size(); planeIdx++)
        {
            const AHBPlaneData &planeData      = data[planeIdx];
            const AHardwareBuffer_Plane &plane = planeInfo.planes[planeIdx];

            size_t planeHeight = (isYUV && planeIdx > 0) ? (height / 2) : height;
            size_t planeWidth  = (isYUV && planeIdx > 0) ? (width / 2) : width;
            size_t layerPitch  = getLayerPitch(planeHeight, plane.rowStride);

            for (size_t z = 0; z < depth; z++)
            {
                const uint8_t *srcDepthSlice =
                    reinterpret_cast<const uint8_t *>(planeData.data) +
                    z * planeHeight * planeWidth * planeData.bytesPerPixel;

                for (size_t y = 0; y < planeHeight; y++)
                {
                    const uint8_t *srcRow =
                        srcDepthSlice + y * planeWidth * planeData.bytesPerPixel;

                    for (size_t x = 0; x < planeWidth; x++)
                    {
                        const uint8_t *src = srcRow + x * planeData.bytesPerPixel;
                        uint8_t *dst = reinterpret_cast<uint8_t *>(plane.data) + z * layerPitch +
                                       y * plane.rowStride + x * plane.pixelStride;
                        memcpy(dst, src, planeData.bytesPerPixel);
                    }
                }
            }
        }

        res = AHardwareBuffer_unlock(aHardwareBuffer, nullptr);
        EXPECT_EQ(res, 0);
#    else
        EXPECT_EQ(1u, data.size());
        void *mappedMemory = nullptr;
        int res = AHardwareBuffer_lock(aHardwareBuffer, AHARDWAREBUFFER_USAGE_CPU_WRITE_RARELY, -1,
                                       nullptr, &mappedMemory);
        EXPECT_EQ(res, 0);

        // Need to grab the stride the implementation might have enforced
        AHardwareBuffer_Desc aHardwareBufferDescription = {};
        AHardwareBuffer_describe(aHardwareBuffer, &aHardwareBufferDescription);
        const size_t stride = aHardwareBufferDescription.stride * data[0].bytesPerPixel;
        size_t layerPitch   = getLayerPitch(height, stride);

        uint32_t rowSize = stride * height;
        for (size_t z = 0; z < depth; z++)
        {
            for (uint32_t y = 0; y < height; y++)
            {
                size_t dstPtrOffset = z * layerPitch + y * stride;
                size_t srcPtrOffset = (z * height + y) * width * data[0].bytesPerPixel;

                uint8_t *dst = reinterpret_cast<uint8_t *>(mappedMemory) + dstPtrOffset;
                memcpy(dst, data[0].data + srcPtrOffset, rowSize);
            }
        }

        res = AHardwareBuffer_unlock(aHardwareBuffer, nullptr);
        EXPECT_EQ(res, 0);
#    endif
        return true;
#else
        return false;
#endif
    }

    enum AHBUsage
    {
        kAHBUsageGPUSampledImage   = 1 << 0,
        kAHBUsageGPUFramebuffer    = 1 << 1,
        kAHBUsageGPUCubeMap        = 1 << 2,
        kAHBUsageGPUMipMapComplete = 1 << 3,
        kAHBUsageFrontBuffer       = 1 << 4,
    };

    constexpr static uint32_t kDefaultAHBUsage = kAHBUsageGPUSampledImage | kAHBUsageGPUFramebuffer;
    constexpr static uint32_t kDefaultAHBYUVUsage = kAHBUsageGPUSampledImage;

#if defined(ANGLE_AHARDWARE_BUFFER_SUPPORT)
    AHardwareBuffer_Desc createAndroidHardwareBufferDesc(size_t width,
                                                         size_t height,
                                                         size_t depth,
                                                         int androidFormat,
                                                         uint32_t usage)
    {
        // The height and width are number of pixels of size format
        AHardwareBuffer_Desc aHardwareBufferDescription = {};
        aHardwareBufferDescription.width                = width;
        aHardwareBufferDescription.height               = height;
        aHardwareBufferDescription.layers               = depth;
        aHardwareBufferDescription.format               = androidFormat;
        aHardwareBufferDescription.usage                = AHARDWAREBUFFER_USAGE_CPU_WRITE_RARELY;
        if ((usage & kAHBUsageGPUSampledImage) != 0)
        {
            aHardwareBufferDescription.usage |= AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
        }
        if ((usage & kAHBUsageGPUFramebuffer) != 0)
        {
            aHardwareBufferDescription.usage |= AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER;
        }
        if ((usage & kAHBUsageGPUCubeMap) != 0)
        {
            aHardwareBufferDescription.usage |= AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP;
        }
        if ((usage & kAHBUsageGPUMipMapComplete) != 0)
        {
            aHardwareBufferDescription.usage |= AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE;
        }
        if ((usage & kAHBUsageFrontBuffer) != 0)
        {
            aHardwareBufferDescription.usage |= ANGLE_AHARDWAREBUFFER_USAGE_FRONT_BUFFER;
        }
        aHardwareBufferDescription.stride = 0;
        aHardwareBufferDescription.rfu0   = 0;
        aHardwareBufferDescription.rfu1   = 0;
        return aHardwareBufferDescription;
    }
#endif

    bool isAndroidHardwareBufferConfigurationSupported(size_t width,
                                                       size_t height,
                                                       size_t depth,
                                                       int androidFormat,
                                                       uint32_t usage)
    {
#if defined(ANGLE_AHARDWARE_BUFFER_SUPPORT)
        const AHardwareBuffer_Desc aHardwareBufferDescription =
            createAndroidHardwareBufferDesc(width, height, depth, androidFormat, usage);
        return AHardwareBuffer_isSupported(&aHardwareBufferDescription);
#else
        return false;
#endif
    }

    AHardwareBuffer *createAndroidHardwareBuffer(size_t width,
                                                 size_t height,
                                                 size_t depth,
                                                 int androidFormat,
                                                 uint32_t usage,
                                                 const std::vector<AHBPlaneData> &data)
    {
#if defined(ANGLE_AHARDWARE_BUFFER_SUPPORT)
        const AHardwareBuffer_Desc aHardwareBufferDescription =
            createAndroidHardwareBufferDesc(width, height, depth, androidFormat, usage);

        // Allocate memory from Android Hardware Buffer
        AHardwareBuffer *aHardwareBuffer = nullptr;
        EXPECT_EQ(0, AHardwareBuffer_allocate(&aHardwareBufferDescription, &aHardwareBuffer));

        if (!data.empty())
        {
            const bool isYUV = androidFormat == AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420 ||
                               androidFormat == AHARDWAREBUFFER_FORMAT_YV12;
            writeAHBData(aHardwareBuffer, width, height, depth, isYUV, data);
        }

        return aHardwareBuffer;
#else
        return nullptr;
#endif  // ANGLE_PLATFORM_ANDROID
    }

    void destroyAndroidHardwareBuffer(AHardwareBuffer *aHardwarebuffer)
    {
#if defined(ANGLE_AHARDWARE_BUFFER_SUPPORT)
        AHardwareBuffer_release(aHardwarebuffer);
#endif
    }

    void createEGLImageAndroidHardwareBufferSource(size_t width,
                                                   size_t height,
                                                   size_t depth,
                                                   int androidPixelFormat,
                                                   uint32_t usage,
                                                   const EGLint *attribs,
                                                   const std::vector<AHBPlaneData> &data,
                                                   AHardwareBuffer **outSourceAHB,
                                                   EGLImageKHR *outSourceImage)
    {
        // Set Android Memory
        AHardwareBuffer *aHardwareBuffer =
            createAndroidHardwareBuffer(width, height, depth, androidPixelFormat, usage, data);
        EXPECT_NE(aHardwareBuffer, nullptr);

        // Create an image from the source AHB
        EGLWindow *window = getEGLWindow();

        EGLImageKHR image = eglCreateImageKHR(
            window->getDisplay(), EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID,
            angle::android::AHardwareBufferToClientBuffer(aHardwareBuffer), attribs);
        ASSERT_EGL_SUCCESS();

        *outSourceAHB   = aHardwareBuffer;
        *outSourceImage = image;
    }

    void createEGLImageANWBClientBufferSource(size_t width,
                                              size_t height,
                                              size_t depth,
                                              const EGLint *attribsANWB,
                                              const EGLint *attribsImage,
                                              const std::vector<AHBPlaneData> &data,
                                              EGLImageKHR *outSourceImage)
    {
        // Set Android Memory

        EGLClientBuffer eglClientBuffer = eglCreateNativeClientBufferANDROID(attribsANWB);
        EXPECT_NE(eglClientBuffer, nullptr);

        // allocate AHB memory
#if defined(ANGLE_AHARDWARE_BUFFER_SUPPORT)
        AHardwareBuffer *pAHardwareBuffer = angle::android::ANativeWindowBufferToAHardwareBuffer(
            angle::android::ClientBufferToANativeWindowBuffer(eglClientBuffer));
        if (!data.empty())
        {
            bool success = writeAHBData(pAHardwareBuffer, width, height, depth, false, data);
            if (!success)
            {
                return;
            }
        }
#endif  // ANGLE_AHARDWARE_BUFFER_SUPPORT

        // Create an image from the source eglClientBuffer
        EGLWindow *window = getEGLWindow();

        EGLImageKHR image =
            eglCreateImageKHR(window->getDisplay(), EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID,
                              eglClientBuffer, attribsImage);
        ASSERT_EGL_SUCCESS();

        *outSourceImage = image;
    }

    void createEGLImageTargetRenderbuffer(EGLImageKHR image, GLuint targetRenderbuffer)
    {
        // Create a target texture from the image
        glBindRenderbuffer(GL_RENDERBUFFER, targetRenderbuffer);
        glEGLImageTargetRenderbufferStorageOES(GL_RENDERBUFFER, image);

        ASSERT_GL_NO_ERROR();
    }

    void ValidationGLEGLImage_helper(const EGLint *attribs);
    void SourceAHBTarget2D_helper(const EGLint *attribs);
    void SourceAHBTarget2DImageStorageGenerateMipmap_helper(const EGLint *attribs);
    void SourceAHBTarget2DArray_helper(const EGLint *attribs);
    void SourceAHBTargetExternal_helper(const EGLint *attribs);
    void SourceAHBTargetExternalESSL3_helper(const EGLint *attribs);
    void SourceNativeClientBufferTargetExternal_helper(const EGLint *attribs);
    void SourceNativeClientBufferTargetRenderbuffer_helper(const EGLint *attribs);
    void Source2DTarget2D_helper(const EGLint *attribs);
    void Source2DTarget2DArray_helper(const EGLint *attribs);
    void Source2DTargetRenderbuffer_helper(const EGLint *attribs);
    void Source2DTargetExternal_helper(const EGLint *attribs);
    void Source2DTargetExternalESSL3_helper(const EGLint *attribs);
    void SourceCubeTarget2D_helper(const EGLint *attribs);
    void SourceCubeTargetRenderbuffer_helper(const EGLint *attribs);
    void SourceCubeTargetExternal_helper(const EGLint *attribs);
    void SourceCubeTargetExternalESSL3_helper(const EGLint *attribs);
    void Source3DTargetTexture_helper(EGLint *attribs);
    void Source3DTargetRenderbuffer_helper(EGLint *attribs);
    void Source3DTargetExternal_helper(EGLint *attribs);
    void Source3DTargetExternalESSL3_helper(EGLint *attribs);
    void SourceRenderbufferTargetTexture_helper(const EGLint *attribs);
    void SourceRenderbufferTargetTextureExternal_helper(const EGLint *attribs);
    void SourceRenderbufferTargetRenderbuffer_helper(const EGLint *attribs);
    void FixedRatedCompressionBasicHelper(const GLint *attribs);
    void FixedRatedCompressionImageAttribCheck(EGLImageKHR image,
                                               const GLint *attribs,
                                               const GLint expectResult);
    void SourceRenderbufferTargetTextureExternalESSL3_helper(const EGLint *attribs);
    void ImageStorageGenerateMipmap_helper(const EGLint *attribs,
                                           const GLsizei width,
                                           const GLsizei height,
                                           AHardwareBuffer *srcAhb,
                                           GLuint srcTexture,
                                           EGLImageKHR *imageOut);
    void ImageCheckingTextureAccessHelper(GLenum target, bool mipmap);
    void verifyImageStorageMipmap(const EGLint *attribs,
                                  EGLImageKHR image,
                                  const GLsizei mipLevelCount);
    void verifyImageStorageMipmapWithBlend(const EGLint *attribs,
                                           EGLImageKHR image,
                                           const GLsizei mipLevelCount);
    void verifyResultsTexture(GLuint texture,
                              const GLubyte referenceColor[4],
                              GLenum textureTarget,
                              GLuint program,
                              GLuint textureUniform)
    {
        // Draw a quad with the target texture
        glUseProgram(program);
        glBindTexture(textureTarget, texture);
        glUniform1i(textureUniform, 0);

        drawQuad(program, "position", 0.5f);

        // Expect that the rendered quad's color is the same as the reference color with a tolerance
        // of 2
        EXPECT_PIXEL_NEAR(0, 0, referenceColor[0], referenceColor[1], referenceColor[2],
                          referenceColor[3], 2);
    }

    void verifyResultsTextureLeftAndRight(GLuint texture,
                                          const GLubyte leftColor[4],
                                          const GLubyte rightColor[4],
                                          GLenum textureTarget,
                                          GLuint program,
                                          GLuint textureUniform)
    {
        verifyResultsTexture(texture, leftColor, textureTarget, program, textureUniform);

        // verifyResultsTexture only verifies top-left. Here also verifies top-right.
        EXPECT_PIXEL_NEAR(getWindowWidth() - 1, 0, rightColor[0], rightColor[1], rightColor[2],
                          rightColor[3], 1);
    }

    void verifyResults2D(GLuint texture, const GLubyte data[4])
    {
        verifyResultsTexture(texture, data, GL_TEXTURE_2D, mTextureProgram,
                             mTextureUniformLocation);
    }

    void verifyResults2DLeftAndRight(GLuint texture, const GLubyte left[4], const GLubyte right[4])
    {
        verifyResultsTextureLeftAndRight(texture, left, right, GL_TEXTURE_2D, mTextureProgram,
                                         mTextureUniformLocation);
    }

    void verifyResults2DArray(GLuint texture, const GLubyte data[4], uint32_t layerIndex = 0)
    {
        glUseProgram(m2DArrayTextureProgram);
        glUniform1ui(m2DArrayTextureLayerUniformLocation, layerIndex);

        verifyResultsTexture(texture, data, GL_TEXTURE_2D_ARRAY, m2DArrayTextureProgram,
                             m2DArrayTextureUniformLocation);
    }

    void verifyResultsCube(GLuint texture, const GLubyte data[4], uint32_t faceIndex = 0)
    {
        glUseProgram(mCubeTextureProgram);
        glUniform3f(mCubeTextureFaceCoordUniformLocation, kCubeFaceX[faceIndex],
                    kCubeFaceY[faceIndex], kCubeFaceZ[faceIndex]);

        verifyResultsTexture(texture, data, GL_TEXTURE_CUBE_MAP, mCubeTextureProgram,
                             mCubeTextureUniformLocation);
    }

    void verifyResultsCubeArray(GLuint texture,
                                const GLubyte data[4],
                                uint32_t faceIndex  = 0,
                                uint32_t layerIndex = 0)
    {
        glUseProgram(mCubeArrayTextureProgram);
        glUniform1ui(mCubeArrayTextureLayerUniformLocation, layerIndex);
        glUniform3f(mCubeArrayTextureFaceCoordUniformLocation, kCubeFaceX[faceIndex],
                    kCubeFaceY[faceIndex], kCubeFaceZ[faceIndex]);

        verifyResultsTexture(texture, data, GL_TEXTURE_CUBE_MAP_ARRAY, mCubeArrayTextureProgram,
                             mCubeArrayTextureUniformLocation);
    }

    void verifyResultsExternal(GLuint texture, const GLubyte data[4])
    {
        verifyResultsTexture(texture, data, GL_TEXTURE_EXTERNAL_OES, mTextureExternalProgram,
                             mTextureExternalUniformLocation);
    }

    void verifyResultsExternalESSL3(GLuint texture, const GLubyte data[4])
    {
        verifyResultsTexture(texture, data, GL_TEXTURE_EXTERNAL_OES, mTextureExternalESSL3Program,
                             mTextureExternalESSL3UniformLocation);
    }

    void verifyResultsExternalYUV(GLuint texture, const GLubyte data[4])
    {
        verifyResultsTexture(texture, data, GL_TEXTURE_EXTERNAL_OES, mTextureYUVProgram,
                             mTextureYUVUniformLocation);
    }

    void verifyResultsRenderbuffer(GLuint renderbuffer, GLubyte referenceColor[4])
    {
        // Bind the renderbuffer to a framebuffer
        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                  renderbuffer);

        // Expect renderbuffer to match referenceColor with a tolerance of 1.
        EXPECT_PIXEL_NEAR(0, 0, referenceColor[0], referenceColor[1], referenceColor[2],
                          referenceColor[3], 1);
    }

    void verifyResultsRenderbufferWithClearAndDraw(GLuint texture,
                                                   GLuint renderbuffer,
                                                   GLubyte clearColor[4],
                                                   GLubyte referenceColor[4])
    {
        // Bind the renderbuffer to a framebuffer
        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                  renderbuffer);

        // Clear the renderbuffer with the clear color
        glClearColor(clearColor[0] / 255.0f, clearColor[1] / 255.0f, clearColor[2] / 255.0f,
                     clearColor[3] / 255.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Expect renderbuffer to match referenceColor with a tolerance of 1.
        EXPECT_PIXEL_NEAR(0, 0, referenceColor[0], referenceColor[1], referenceColor[2],
                          referenceColor[3], 1);

        // Sample source texture and draw onto renderbuffer and expect rendered quad's color
        // is the same as the reference color with a tolerance of 1
        verifyResultsTexture(texture, referenceColor, GL_TEXTURE_2D, mTextureProgram,
                             mTextureUniformLocation);
    }

    enum class AHBVerifyRegion
    {
        Entire,
        LeftHalf,
        RightHalf,
    };

    void verifyResultAHB(AHardwareBuffer *source,
                         const std::vector<AHBPlaneData> &data,
                         AHBVerifyRegion verifyRegion = AHBVerifyRegion::Entire)
    {
#if defined(ANGLE_AHARDWARE_BUFFER_SUPPORT)
        AHardwareBuffer_Desc aHardwareBufferDescription;
        AHardwareBuffer_describe(source, &aHardwareBufferDescription);
        bool isYUV = (aHardwareBufferDescription.format == AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420);
        const uint32_t width  = aHardwareBufferDescription.width;
        const uint32_t height = aHardwareBufferDescription.height;
        const uint32_t depth  = aHardwareBufferDescription.layers;

#    if defined(ANGLE_AHARDWARE_BUFFER_LOCK_PLANES_SUPPORT)
        AHardwareBuffer_Planes planeInfo;
        ASSERT_EQ(0, AHardwareBuffer_lockPlanes(source, AHARDWAREBUFFER_USAGE_CPU_WRITE_RARELY, -1,
                                                nullptr, &planeInfo));
        ASSERT_EQ(data.size(), planeInfo.planeCount);

        for (size_t planeIdx = 0; planeIdx < data.size(); planeIdx++)
        {
            const AHBPlaneData &planeData      = data[planeIdx];
            const AHardwareBuffer_Plane &plane = planeInfo.planes[planeIdx];

            const size_t planeHeight = (isYUV && planeIdx > 0) ? (height / 2) : height;
            const size_t planeWidth  = (isYUV && planeIdx > 0) ? (width / 2) : width;
            size_t layerPitch        = getLayerPitch(planeHeight, plane.rowStride);

            uint32_t xStart = 0;
            uint32_t xEnd   = planeWidth;

            switch (verifyRegion)
            {
                case AHBVerifyRegion::Entire:
                    break;
                case AHBVerifyRegion::LeftHalf:
                    xEnd = planeWidth / 2;
                    break;
                case AHBVerifyRegion::RightHalf:
                    xStart = planeWidth / 2;
                    break;
            }

            for (size_t z = 0; z < depth; z++)
            {
                const uint8_t *referenceDepthSlice =
                    reinterpret_cast<const uint8_t *>(planeData.data) +
                    z * planeHeight * (xEnd - xStart) * planeData.bytesPerPixel;
                for (size_t y = 0; y < planeHeight; y++)
                {
                    const uint8_t *referenceRow =
                        referenceDepthSlice + y * (xEnd - xStart) * planeData.bytesPerPixel;
                    for (size_t x = xStart; x < xEnd; x++)
                    {
                        const uint8_t *referenceData =
                            referenceRow + (x - xStart) * planeData.bytesPerPixel;
                        std::vector<uint8_t> reference(referenceData,
                                                       referenceData + planeData.bytesPerPixel);

                        const uint8_t *ahbData = reinterpret_cast<uint8_t *>(plane.data) +
                                                 z * layerPitch + y * plane.rowStride +
                                                 x * plane.pixelStride;
                        std::vector<uint8_t> ahb(ahbData, ahbData + planeData.bytesPerPixel);

                        EXPECT_EQ(reference, ahb)
                            << "at (" << x << ", " << y << ") on plane " << planeIdx;
                    }
                }
            }
        }
        ASSERT_EQ(0, AHardwareBuffer_unlock(source, nullptr));
#    else
        ASSERT_EQ(1u, data.size());
        ASSERT_FALSE(isYUV);

        const uint32_t rowStride = aHardwareBufferDescription.stride * data[0].bytesPerPixel;
        size_t layerPitch        = getLayerPitch(height, rowStride);

        void *mappedMemory = nullptr;
        ASSERT_EQ(0, AHardwareBuffer_lock(source, AHARDWAREBUFFER_USAGE_CPU_WRITE_RARELY, -1,
                                          nullptr, &mappedMemory));

        uint32_t xStart = 0;
        uint32_t xEnd   = width;

        switch (verifyRegion)
        {
            case AHBVerifyRegion::Entire:
                break;
            case AHBVerifyRegion::LeftHalf:
                xEnd = width / 2;
                break;
            case AHBVerifyRegion::RightHalf:
                xStart = width / 2;
                break;
        }
        for (size_t z = 0; z < depth; z++)
        {
            const uint8_t *referenceDepthSlice =
                reinterpret_cast<const uint8_t *>(data[0].data) +
                z * height * (xEnd - xStart) * data[0].bytesPerPixel;
            for (size_t y = 0; y < height; y++)
            {
                const uint8_t *referenceRow =
                    referenceDepthSlice + y * (xEnd - xStart) * data[0].bytesPerPixel;
                for (size_t x = xStart; x < xEnd; x++)
                {
                    const uint8_t *referenceData =
                        referenceRow + (x - xStart) * data[0].bytesPerPixel;
                    std::vector<uint8_t> reference(referenceData,
                                                   referenceData + data[0].bytesPerPixel);

                    const uint8_t *ahbData = reinterpret_cast<uint8_t *>(mappedMemory) +
                                             z * layerPitch + y * rowStride +
                                             x * data[0].bytesPerPixel;
                    std::vector<uint8_t> ahb(ahbData, ahbData + data[0].bytesPerPixel);

                    EXPECT_EQ(reference, ahb) << "at (" << x << ", " << y << ")";
                }
            }
        }
        ASSERT_EQ(0, AHardwareBuffer_unlock(source, nullptr));
#    endif
#endif
    }

    template <typename destType, typename sourcetype>
    destType reinterpretHelper(const sourcetype &source)
    {
        static_assert(sizeof(destType) == sizeof(size_t),
                      "destType should be the same size as a size_t");
        size_t sourceSizeT = static_cast<size_t>(source.get());
        return reinterpret_cast<destType>(sourceSizeT);
    }

    bool hasImageGLColorspaceExt() const
    {
        // Possible GLES driver bug on Pixel2 devices: http://anglebug.com/42263865
        if (IsPixel2() && IsOpenGLES())
        {
            return false;
        }
        return IsEGLDisplayExtensionEnabled(getEGLWindow()->getDisplay(), kImageGLColorspaceExt);
    }

    bool hasAndroidImageNativeBufferExt() const
    {
        return IsEGLDisplayExtensionEnabled(getEGLWindow()->getDisplay(),
                                            kEGLAndroidImageNativeBufferExt);
    }

    bool hasEglImageStorageExt() const { return IsGLExtensionEnabled(kEGLImageStorageExt); }

    bool hasEglImageStorageCompressionExt() const
    {
        return IsGLExtensionEnabled(kEGLImageStorageCompressionExt);
    }

    bool hasTextureStorageCompressionExt() const
    {
        return IsGLExtensionEnabled(kTextureStorageCompressionExt);
    }

    bool hasAndroidHardwareBufferSupport() const
    {
#if defined(ANGLE_AHARDWARE_BUFFER_SUPPORT)
        return true;
#else
        return false;
#endif
    }

    bool hasAhbLockPlanesSupport() const
    {
#if defined(ANGLE_AHARDWARE_BUFFER_LOCK_PLANES_SUPPORT)
        return true;
#else
        return false;
#endif
    }

    bool hasEglImageArrayExt() const { return IsGLExtensionEnabled(kEGLImageArrayExt); }

    bool hasOESExt() const { return IsGLExtensionEnabled(kOESExt); }

    bool hasExternalExt() const { return IsGLExtensionEnabled(kExternalExt); }

    bool hasExternalESSL3Ext() const { return IsGLExtensionEnabled(kExternalESSL3Ext); }

    bool hasYUVInternalFormatExt() const { return IsGLExtensionEnabled(kYUVInternalFormatExt); }

    bool hasYUVTargetExt() const { return IsGLExtensionEnabled(kYUVTargetExt); }

    bool hasRGBXInternalFormatExt() const { return IsGLExtensionEnabled(kRGBXInternalFormatExt); }

    bool hasBaseExt() const
    {
        return IsEGLDisplayExtensionEnabled(getEGLWindow()->getDisplay(), kBaseExt);
    }

    bool has2DTextureExt() const
    {
        return IsEGLDisplayExtensionEnabled(getEGLWindow()->getDisplay(), k2DTextureExt);
    }

    bool has3DTextureExt() const
    {
        return IsEGLDisplayExtensionEnabled(getEGLWindow()->getDisplay(), k3DTextureExt);
    }

    bool hasPixmapExt() const
    {
        return IsEGLDisplayExtensionEnabled(getEGLWindow()->getDisplay(), kPixmapExt);
    }

    bool hasRenderbufferExt() const
    {
        return IsEGLDisplayExtensionEnabled(getEGLWindow()->getDisplay(), kRenderbufferExt);
    }

    bool hasCubemapExt() const
    {
        return IsEGLDisplayExtensionEnabled(getEGLWindow()->getDisplay(), kCubemapExt);
    }

    angle::VulkanPerfCounters getPerfCounters()
    {
        ASSERT(IsVulkan());

        if (mCounterNameToIndexMap.empty())
        {
            mCounterNameToIndexMap = BuildCounterNameToIndexMap();
        }

        return GetPerfCounters(mCounterNameToIndexMap);
    }

    void externalTextureTracerTestHelper(const EGLint *attribsToRecoverInMEC)
    {
        const EGLWindow *eglWindow = getEGLWindow();
        // Frame 1 begins
        // Create the Image
        GLTexture sourceTexture1;
        EGLImageKHR image1;

        GLubyte data[] = {132, 55, 219, 255};
        // Create a source 2D texture
        glBindTexture(GL_TEXTURE_2D, sourceTexture1);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(1), static_cast<GLsizei>(1), 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, data);

        // Disable mipmapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        ASSERT_GL_NO_ERROR();

        image1 = eglCreateImageKHR(
            eglWindow->getDisplay(), eglWindow->getContext(), EGL_GL_TEXTURE_2D_KHR,
            reinterpretHelper<EGLClientBuffer>(sourceTexture1), attribsToRecoverInMEC);

        ASSERT_EGL_SUCCESS();

        // Create the target
        GLTexture targetTexture1;
        // Create a target texture from the image
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, targetTexture1);
        glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, image1);

        // Disable mipmapping
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        ASSERT_GL_NO_ERROR();

        // Calls On EndFrame(), with MidExecutionSetup to restore external texture targetTexture1
        // above
        EGLDisplay display = eglWindow->getDisplay();
        EGLSurface surface = eglWindow->getSurface();
        eglSwapBuffers(display, surface);
        // Frame 1 ends

        // Frame 2 begins
        // Create another eglImage with another associated texture
        // Draw using the eglImage texture targetTexture1 created in frame 1
        GLTexture sourceTexture2;
        EGLImageKHR image2;

        // Create a source 2D texture
        glBindTexture(GL_TEXTURE_2D, sourceTexture2);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(1), static_cast<GLsizei>(1), 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, data);

        // Disable mipmapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        ASSERT_GL_NO_ERROR();

        constexpr EGLint defaultAttribs[] = {
            EGL_IMAGE_PRESERVED,
            EGL_TRUE,
            EGL_NONE,
        };
        image2 = eglCreateImageKHR(
            eglWindow->getDisplay(), eglWindow->getContext(), EGL_GL_TEXTURE_2D_KHR,
            reinterpretHelper<EGLClientBuffer>(sourceTexture2), defaultAttribs);

        ASSERT_EGL_SUCCESS();

        // Create the target
        GLTexture targetTexture2;
        // Create a target texture from the image
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, targetTexture2);
        glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, image2);

        // Disable mipmapping
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        ASSERT_GL_NO_ERROR();
        glUseProgram(mTextureExternalProgram);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, targetTexture1);
        glUniform1i(mTextureExternalUniformLocation, 0);

        drawQuad(mTextureExternalProgram, "position", 0.5f);

        // Calls On EndFrame() to save the gl calls creating external texture targetTexture2;
        // We use this as a reference to check the gl calls we restore for targetTexture1
        // in MidExecutionSetup
        eglSwapBuffers(display, surface);
        // Frame 2 ends

        // Frame 3 begins
        // Draw a quad with the targetTexture2
        glUseProgram(mTextureExternalProgram);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, targetTexture2);
        glUniform1i(mTextureExternalUniformLocation, 0);

        drawQuad(mTextureExternalProgram, "position", 0.5f);

        eglSwapBuffers(display, surface);
        // Frame 3 ends

        // Clean up
        eglDestroyImageKHR(eglWindow->getDisplay(), image1);
        eglDestroyImageKHR(eglWindow->getDisplay(), image2);
    }

    void framebufferAttachmentDeletedWhileInUseHelper(bool useTextureAttachment,
                                                      bool deleteSourceTextureLast);
    void framebufferResolveAttachmentDeletedWhileInUseHelper(bool useTextureAttachment,
                                                             bool deleteSourceTextureLast);
    void useAHBByGLThenForeignThenGLHelper(
        std::function<void(const GLTexture &, uint32_t, uint32_t)> firstUse,
        std::function<void(const GLTexture &, uint32_t, uint32_t)> secondUse);

    EGLint default3DAttribs[5] = {
        EGL_GL_TEXTURE_ZOFFSET_KHR, static_cast<EGLint>(0), EGL_IMAGE_PRESERVED, EGL_TRUE, EGL_NONE,
    };
    EGLint colorspace3DAttribs[7] = {
        EGL_GL_TEXTURE_ZOFFSET_KHR,
        static_cast<EGLint>(0),
        EGL_IMAGE_PRESERVED,
        EGL_TRUE,
        EGL_GL_COLORSPACE,
        EGL_GL_COLORSPACE_SRGB_KHR,
        EGL_NONE,
    };
    GLuint mTextureProgram;
    GLuint m2DArrayTextureProgram;
    GLuint mCubeTextureProgram;
    GLuint mCubeArrayTextureProgram;
    GLint mTextureUniformLocation;
    GLuint m2DArrayTextureUniformLocation;
    GLuint m2DArrayTextureLayerUniformLocation;
    GLuint mCubeTextureUniformLocation;
    GLuint mCubeTextureFaceCoordUniformLocation;
    GLuint mCubeArrayTextureUniformLocation;
    GLuint mCubeArrayTextureFaceCoordUniformLocation;
    GLuint mCubeArrayTextureLayerUniformLocation;

    GLuint mTextureExternalProgram        = 0;
    GLint mTextureExternalUniformLocation = -1;

    GLuint mTextureExternalESSL3Program        = 0;
    GLint mTextureExternalESSL3UniformLocation = -1;

    GLuint mTextureYUVProgram        = 0;
    GLint mTextureYUVUniformLocation = -1;

    GLuint mRenderYUVProgram        = 0;
    GLint mRenderYUVUniformLocation = -1;

    CounterNameToIndexMap mCounterNameToIndexMap;
};

class ImageTestES3 : public ImageTest
{};

class ImageTestES31 : public ImageTest
{};

// Tests that the extension is exposed on the platforms we think it should be. Please modify this as
// you change extension availability.
TEST_P(ImageTest, ANGLEExtensionAvailability)
{
    // EGL support is based on driver extension availability.
    ANGLE_SKIP_TEST_IF(IsOpenGLES() && IsAndroid());
    ANGLE_SKIP_TEST_IF(IsOpenGLES() && IsOzone());

    if (IsD3D11() || IsD3D9())
    {
        EXPECT_TRUE(hasOESExt());
        EXPECT_TRUE(hasExternalExt());
        EXPECT_TRUE(hasBaseExt());
        EXPECT_TRUE(has2DTextureExt());
        EXPECT_TRUE(hasRenderbufferExt());
        EXPECT_FALSE(has3DTextureExt());

        if (IsD3D11())
        {
            EXPECT_TRUE(hasCubemapExt());

            if (getClientMajorVersion() >= 3)
            {
                EXPECT_TRUE(hasExternalESSL3Ext());
            }
            else
            {
                EXPECT_FALSE(hasExternalESSL3Ext());
            }
        }
        else
        {
            EXPECT_FALSE(hasCubemapExt());
            EXPECT_FALSE(hasExternalESSL3Ext());
        }
    }
    else if (IsVulkan())
    {
        EXPECT_TRUE(hasOESExt());
        EXPECT_TRUE(hasExternalExt());
        EXPECT_TRUE(hasBaseExt());
        EXPECT_TRUE(has2DTextureExt());
        EXPECT_TRUE(hasCubemapExt());
        EXPECT_TRUE(hasRenderbufferExt());
        if (getClientMajorVersion() >= 3)
        {
            EXPECT_TRUE(hasExternalESSL3Ext());
        }
        else
        {
            EXPECT_FALSE(hasExternalESSL3Ext());
        }
    }
    else if (IsMetal())
    {
        // NOTE(hqle): Metal currently doesn't implement any image extensions besides
        // EGL_ANGLE_metal_texture_client_buffer
        EXPECT_TRUE(hasOESExt());
        EXPECT_TRUE(hasBaseExt());
        EXPECT_FALSE(hasExternalExt());
        EXPECT_FALSE(hasExternalESSL3Ext());
        EXPECT_FALSE(has2DTextureExt());
        EXPECT_FALSE(has3DTextureExt());
        EXPECT_FALSE(hasRenderbufferExt());
    }
    else
    {
        EXPECT_FALSE(hasOESExt());
        EXPECT_FALSE(hasExternalExt());
        EXPECT_FALSE(hasExternalESSL3Ext());
        EXPECT_FALSE(hasBaseExt());
        EXPECT_FALSE(has2DTextureExt());
        EXPECT_FALSE(has3DTextureExt());
        EXPECT_FALSE(hasRenderbufferExt());
    }

    // These extensions are not yet available on any platform.
    EXPECT_FALSE(hasPixmapExt());
}

// Check validation from the EGL_KHR_image_base extension
TEST_P(ImageTest, ValidationImageBase)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());

    GLTexture glTexture2D;
    glBindTexture(GL_TEXTURE_2D, glTexture2D);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    EGLDisplay display        = window->getDisplay();
    EGLContext context        = window->getContext();
    EGLConfig config          = window->getConfig();
    EGLImageKHR image         = EGL_NO_IMAGE_KHR;
    EGLClientBuffer texture2D = reinterpretHelper<EGLClientBuffer>(glTexture2D);

    // Test validation of eglCreateImageKHR

    // If <dpy> is not the handle of a valid EGLDisplay object, the error EGL_BAD_DISPLAY is
    // generated.
    image = eglCreateImageKHR(reinterpret_cast<EGLDisplay>(0xBAADF00D), context,
                              EGL_GL_TEXTURE_2D_KHR, texture2D, nullptr);
    EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
    EXPECT_EGL_ERROR(EGL_BAD_DISPLAY);

    // If <ctx> is neither the handle of a valid EGLContext object on <dpy> nor EGL_NO_CONTEXT, the
    // error EGL_BAD_CONTEXT is generated.
    image = eglCreateImageKHR(display, reinterpret_cast<EGLContext>(0xBAADF00D),
                              EGL_GL_TEXTURE_2D_KHR, texture2D, nullptr);
    EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
    EXPECT_EGL_ERROR(EGL_BAD_CONTEXT);

    // Test EGL_NO_CONTEXT with a 2D texture target which does require a context.
    image = eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_GL_TEXTURE_2D_KHR, texture2D, nullptr);
    EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
    EXPECT_EGL_ERROR(EGL_BAD_CONTEXT);

    // If an attribute specified in <attrib_list> is not one of the attributes listed in Table bbb,
    // the error EGL_BAD_PARAMETER is generated.
    EGLint badAttributes[] = {
        static_cast<EGLint>(0xDEADBEEF),
        0,
        EGL_NONE,
    };

    image = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_2D_KHR, texture2D, badAttributes);
    EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
    EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);

    // If the resource specified by <dpy>, <ctx>, <target>, <buffer> and <attrib_list> has an off -
    // screen buffer bound to it(e.g., by a
    // previous call to eglBindTexImage), the error EGL_BAD_ACCESS is generated.
    EGLint surfaceType = 0;
    eglGetConfigAttrib(display, config, EGL_SURFACE_TYPE, &surfaceType);

    EGLint bindToTextureRGBA = 0;
    eglGetConfigAttrib(display, config, EGL_BIND_TO_TEXTURE_RGBA, &bindToTextureRGBA);
    if ((surfaceType & EGL_PBUFFER_BIT) != 0 && bindToTextureRGBA == EGL_TRUE)
    {
        EGLint pbufferAttributes[] = {
            EGL_WIDTH,          1,
            EGL_HEIGHT,         1,
            EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGBA,
            EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
            EGL_NONE,           EGL_NONE,
        };
        EGLSurface pbuffer = eglCreatePbufferSurface(display, config, pbufferAttributes);
        ASSERT_NE(pbuffer, EGL_NO_SURFACE);
        EXPECT_EGL_SUCCESS();

        eglBindTexImage(display, pbuffer, EGL_BACK_BUFFER);
        EXPECT_EGL_SUCCESS();

        image = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_2D_KHR, texture2D, nullptr);
        EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
        EXPECT_EGL_ERROR(EGL_BAD_ACCESS);

        eglReleaseTexImage(display, pbuffer, EGL_BACK_BUFFER);
        eglDestroySurface(display, pbuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        EXPECT_EGL_SUCCESS();
        EXPECT_GL_NO_ERROR();
    }

    // If the resource specified by <dpy>, <ctx>, <target>, <buffer> and
    // <attrib_list> is itself an EGLImage sibling, the error EGL_BAD_ACCESS is generated.
    image = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_2D_KHR, texture2D, nullptr);
    EXPECT_NE(image, EGL_NO_IMAGE_KHR);
    EXPECT_EGL_SUCCESS();

    /* TODO(geofflang): Enable this validation when it passes.
    EGLImageKHR image2 = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_2D_KHR,
    reinterpret_cast<EGLClientBuffer>(texture2D), nullptr);
    EXPECT_EQ(image2, EGL_NO_IMAGE_KHR);
    EXPECT_EGL_ERROR(EGL_BAD_ACCESS);
    */

    // Test validation of eglDestroyImageKHR
    // Note: image is now a valid EGL image
    EGLBoolean result = EGL_FALSE;

    // If <dpy> is not the handle of a valid EGLDisplay object, the error EGL_BAD_DISPLAY is
    // generated.
    result = eglDestroyImageKHR(reinterpret_cast<EGLDisplay>(0xBAADF00D), image);
    EXPECT_EQ(result, static_cast<EGLBoolean>(EGL_FALSE));
    EXPECT_EGL_ERROR(EGL_BAD_DISPLAY);

    // If <image> is not a valid EGLImageKHR object created with respect to <dpy>, the error
    // EGL_BAD_PARAMETER is generated.
    result = eglDestroyImageKHR(display, reinterpret_cast<EGLImageKHR>(0xBAADF00D));
    EXPECT_EQ(result, static_cast<EGLBoolean>(EGL_FALSE));
    EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);

    // Clean up and validate image is destroyed
    result = eglDestroyImageKHR(display, image);
    EXPECT_EQ(result, static_cast<EGLBoolean>(EGL_TRUE));
    EXPECT_EGL_SUCCESS();

    EXPECT_GL_NO_ERROR();
}

// Check validation from the EGL_KHR_gl_texture_2D_image, EGL_KHR_gl_texture_cubemap_image,
// EGL_KHR_gl_texture_3D_image and EGL_KHR_gl_renderbuffer_image extensions
TEST_P(ImageTest, ValidationGLImage)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt());

    EGLDisplay display = window->getDisplay();
    EGLContext context = window->getContext();
    EGLImageKHR image  = EGL_NO_IMAGE_KHR;

    if (has2DTextureExt())
    {
        // If <target> is EGL_GL_TEXTURE_2D_KHR, EGL_GL_TEXTURE_CUBE_MAP_*_KHR or
        // EGL_GL_TEXTURE_3D_KHR and <buffer> is not the name of a texture object of type <target>,
        // the error EGL_BAD_PARAMETER is generated.
        GLTexture textureCube;
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureCube);
        for (GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
             face++)
        {
            glTexImage2D(face, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        }

        image = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_2D_KHR,
                                  reinterpretHelper<EGLClientBuffer>(textureCube), nullptr);
        EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
        EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);

        // If EGL_GL_TEXTURE_LEVEL_KHR is 0, <target> is EGL_GL_TEXTURE_2D_KHR,
        // EGL_GL_TEXTURE_CUBE_MAP_*_KHR or EGL_GL_TEXTURE_3D_KHR, <buffer> is the name of an
        // incomplete GL texture object, and any mipmap levels other than mipmap level 0 are
        // specified, the error EGL_BAD_PARAMETER is generated.
        GLTexture incompleteTexture;
        glBindTexture(GL_TEXTURE_2D, incompleteTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        EGLint level0Attribute[] = {
            EGL_GL_TEXTURE_LEVEL_KHR,
            0,
            EGL_NONE,
        };
        image = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_2D_KHR,
                                  reinterpretHelper<EGLClientBuffer>(incompleteTexture),
                                  level0Attribute);
        EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
        EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);

        // If EGL_GL_TEXTURE_LEVEL_KHR is 0, <target> is EGL_GL_TEXTURE_2D_KHR or
        // EGL_GL_TEXTURE_3D_KHR, <buffer> is not the name of a complete GL texture object, and
        // mipmap level 0 is not specified, the error EGL_BAD_PARAMETER is generated.
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        image = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_2D_KHR,
                                  reinterpretHelper<EGLClientBuffer>(incompleteTexture), nullptr);
        EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
        EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);

        // If <target> is EGL_GL_TEXTURE_2D_KHR, EGL_GL_TEXTURE_CUBE_MAP_*_KHR,
        // EGL_GL_RENDERBUFFER_KHR or EGL_GL_TEXTURE_3D_KHR and <buffer> refers to the default GL
        // texture object(0) for the corresponding GL target, the error EGL_BAD_PARAMETER is
        // generated.
        image = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_2D_KHR, 0, nullptr);
        EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
        EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);

        // If <target> is EGL_GL_TEXTURE_2D_KHR, EGL_GL_TEXTURE_CUBE_MAP_*_KHR, or
        // EGL_GL_TEXTURE_3D_KHR, and the value specified in <attr_list> for
        // EGL_GL_TEXTURE_LEVEL_KHR is not a valid mipmap level for the specified GL texture object
        // <buffer>, the error EGL_BAD_MATCH is generated.
        EGLint level2Attribute[] = {
            EGL_GL_TEXTURE_LEVEL_KHR,
            2,
            EGL_NONE,
        };
        image = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_2D_KHR,
                                  reinterpretHelper<EGLClientBuffer>(incompleteTexture),
                                  level2Attribute);
        EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
        EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
    }
    else
    {
        GLTexture texture2D;
        glBindTexture(GL_TEXTURE_2D, texture2D);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        // From EGL_KHR_image_base:
        // If <target> is not one of the values in Table aaa, the error EGL_BAD_PARAMETER is
        // generated.
        image = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_2D_KHR,
                                  reinterpretHelper<EGLClientBuffer>(texture2D), nullptr);
        EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
        EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
    }

    if (hasCubemapExt())
    {
        // If EGL_GL_TEXTURE_LEVEL_KHR is 0, <target> is EGL_GL_TEXTURE_CUBE_MAP_*_KHR, <buffer> is
        // not the name of a complete GL texture object, and one or more faces do not have mipmap
        // level 0 specified, the error EGL_BAD_PARAMETER is generated.
        GLTexture incompleteTextureCube;
        glBindTexture(GL_TEXTURE_CUBE_MAP, incompleteTextureCube);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);

        EGLint level0Attribute[] = {
            EGL_GL_TEXTURE_LEVEL_KHR,
            0,
            EGL_NONE,
        };
        image = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR,
                                  reinterpretHelper<EGLClientBuffer>(incompleteTextureCube),
                                  level0Attribute);
        EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
        EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
    }
    else
    {
        GLTexture textureCube;
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureCube);
        for (GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
             face++)
        {
            glTexImage2D(face, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        }

        // From EGL_KHR_image_base:
        // If <target> is not one of the values in Table aaa, the error EGL_BAD_PARAMETER is
        // generated.
        image = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR,
                                  reinterpretHelper<EGLClientBuffer>(textureCube), nullptr);
        EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
        EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
    }

    if (has3DTextureExt() && getClientMajorVersion() >= 3)
    {
        // If <target> is EGL_GL_TEXTURE_3D_KHR, and the value specified in <attr_list> for
        // EGL_GL_TEXTURE_ZOFFSET_KHR exceeds the depth of the specified mipmap level - of - detail
        // in <buffer>, the error EGL_BAD_PARAMETER is generated.
        GLTexture texture3D;
        glBindTexture(GL_TEXTURE_3D, texture3D);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, 2, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        EGLint zOffset3Parameter[] = {
            EGL_GL_TEXTURE_ZOFFSET_KHR,
            3,
            EGL_NONE,
        };
        image = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_3D_KHR,
                                  reinterpretHelper<EGLClientBuffer>(texture3D), zOffset3Parameter);
        EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
        EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);

        EGLint zOffsetNegative1Parameter[] = {
            EGL_GL_TEXTURE_ZOFFSET_KHR,
            -1,
            EGL_NONE,
        };
        image = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_3D_KHR,
                                  reinterpretHelper<EGLClientBuffer>(texture3D),
                                  zOffsetNegative1Parameter);
        EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
        EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
    }
    else
    {
        if (has2DTextureExt())
        {
            GLTexture texture2D;
            glBindTexture(GL_TEXTURE_2D, texture2D);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

            // Verify EGL_GL_TEXTURE_ZOFFSET_KHR is not a valid parameter
            EGLint zOffset0Parameter[] = {
                EGL_GL_TEXTURE_ZOFFSET_KHR,
                0,
                EGL_NONE,
            };

            image =
                eglCreateImageKHR(display, context, EGL_GL_TEXTURE_2D_KHR,
                                  reinterpretHelper<EGLClientBuffer>(texture2D), zOffset0Parameter);
            EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
            EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
        }

        if (getClientMajorVersion() >= 3)
        {
            GLTexture texture3D;
            glBindTexture(GL_TEXTURE_3D, texture3D);
            glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, 1, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

            // From EGL_KHR_image_base:
            // If <target> is not one of the values in Table aaa, the error EGL_BAD_PARAMETER is
            // generated.
            image = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_3D_KHR,
                                      reinterpretHelper<EGLClientBuffer>(texture3D), nullptr);
            EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
            EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
        }
    }

    if (hasRenderbufferExt())
    {
        // If <target> is EGL_GL_RENDERBUFFER_KHR and <buffer> is not the name of a renderbuffer
        // object, or if <buffer> is the name of a multisampled renderbuffer object, the error
        // EGL_BAD_PARAMETER is generated.
        image = eglCreateImageKHR(display, context, EGL_GL_RENDERBUFFER_KHR,
                                  reinterpret_cast<EGLClientBuffer>(0), nullptr);
        EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
        EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);

        if (IsGLExtensionEnabled("GL_ANGLE_framebuffer_multisample"))
        {
            GLRenderbuffer renderbuffer;
            glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
            glRenderbufferStorageMultisampleANGLE(GL_RENDERBUFFER, 1, GL_RGBA8, 1, 1);
            EXPECT_GL_NO_ERROR();

            image = eglCreateImageKHR(display, context, EGL_GL_RENDERBUFFER_KHR,
                                      reinterpret_cast<EGLClientBuffer>(0), nullptr);
            EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
            EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
        }
    }
    else
    {
        GLRenderbuffer renderbuffer;
        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, 1, 1);

        // From EGL_KHR_image_base:
        // If <target> is not one of the values in Table aaa, the error EGL_BAD_PARAMETER is
        // generated.
        image = eglCreateImageKHR(display, context, EGL_GL_RENDERBUFFER_KHR,
                                  reinterpretHelper<EGLClientBuffer>(renderbuffer), nullptr);
        EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
        EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
    }
}

// Check validation from the GL_OES_EGL_image extension
TEST_P(ImageTest, ValidationGLEGLImage)
{
    ValidationGLEGLImage_helper(kDefaultAttribs);
}

TEST_P(ImageTest, ValidationGLEGLImage_Colorspace)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 && !IsGLExtensionEnabled("GL_EXT_sRGB"));
    ANGLE_SKIP_TEST_IF(!hasImageGLColorspaceExt());
    ValidationGLEGLImage_helper(kColorspaceAttribs);
}

void ImageTest::ValidationGLEGLImage_helper(const EGLint *attribs)
{
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());

    // Create the Image
    GLTexture source;
    EGLImageKHR image;
    createEGLImage2DTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, attribs, kLinearColor, source,
                                  &image);

    // If <target> is not TEXTURE_2D, the error INVALID_ENUM is generated.
    glEGLImageTargetTexture2DOES(GL_TEXTURE_CUBE_MAP_POSITIVE_X, image);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // If <image> does not refer to a valid eglImageOES object, the error INVALID_VALUE is
    // generated.
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, reinterpret_cast<GLeglImageOES>(0xBAADF00D));
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // <target> must be RENDERBUFFER_OES, and <image> must be the handle of a valid EGLImage
    // resource, cast into the type
    // eglImageOES.
    glEGLImageTargetRenderbufferStorageOES(GL_TEXTURE_2D, image);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // If a renderbuffer is not bound, the error INVALID_OPERATION is generated.
    // (Not in specification.)
    glEGLImageTargetRenderbufferStorageOES(GL_RENDERBUFFER, image);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // If the GL is unable to create a renderbuffer using the specified eglImageOES, the error
    // INVALID_OPERATION is generated.If <image>
    // does not refer to a valid eglImageOES object, the error INVALID_VALUE is generated.
    GLRenderbuffer renderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glEGLImageTargetRenderbufferStorageOES(GL_RENDERBUFFER,
                                           reinterpret_cast<GLeglImageOES>(0xBAADF00D));
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // Clean up
    eglDestroyImageKHR(getEGLWindow()->getDisplay(), image);
}

// Check validation from the GL_OES_EGL_image_external extension
TEST_P(ImageTest, ValidationGLEGLImageExternal)
{
    ANGLE_SKIP_TEST_IF(!hasExternalExt());

    GLTexture texture;
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture);

    // In the initial state of a TEXTURE_EXTERNAL_OES texture object, the value assigned to
    // TEXTURE_MIN_FILTER and TEXTURE_MAG_FILTER is LINEAR, and the s and t wrap modes are both set
    // to CLAMP_TO_EDGE
    auto getTexParam = [](GLenum target, GLenum pname) {
        GLint value = 0;
        glGetTexParameteriv(target, pname, &value);
        EXPECT_GL_NO_ERROR();
        return value;
    };
    EXPECT_GLENUM_EQ(GL_LINEAR, getTexParam(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER));
    EXPECT_GLENUM_EQ(GL_LINEAR, getTexParam(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER));
    EXPECT_GLENUM_EQ(GL_CLAMP_TO_EDGE, getTexParam(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S));
    EXPECT_GLENUM_EQ(GL_CLAMP_TO_EDGE, getTexParam(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T));

    // "When <target> is TEXTURE_EXTERNAL_OES only NEAREST and LINEAR are accepted as
    // TEXTURE_MIN_FILTER, only CLAMP_TO_EDGE is accepted as TEXTURE_WRAP_S and TEXTURE_WRAP_T, and
    // only FALSE is accepted as GENERATE_MIPMAP. Attempting to set other values for
    // TEXTURE_MIN_FILTER, TEXTURE_WRAP_S, TEXTURE_WRAP_T, or GENERATE_MIPMAP will result in an
    // INVALID_ENUM error.
    GLenum validMinFilters[]{
        GL_NEAREST,
        GL_LINEAR,
    };
    for (auto minFilter : validMinFilters)
    {
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, minFilter);
        EXPECT_GL_NO_ERROR();
    }

    GLenum invalidMinFilters[]{
        GL_NEAREST_MIPMAP_LINEAR,
        GL_NEAREST_MIPMAP_NEAREST,
        GL_LINEAR_MIPMAP_LINEAR,
        GL_LINEAR_MIPMAP_NEAREST,
    };
    for (auto minFilter : invalidMinFilters)
    {
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, minFilter);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }

    GLenum validWrapModes[]{
        GL_CLAMP_TO_EDGE,
    };
    for (auto wrapMode : validWrapModes)
    {
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, wrapMode);
        EXPECT_GL_NO_ERROR();
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, wrapMode);
        EXPECT_GL_NO_ERROR();
    }

    if (IsGLExtensionEnabled("GL_EXT_EGL_image_external_wrap_modes"))
    {
        GLenum validWrapModesEXT[]{GL_REPEAT, GL_MIRRORED_REPEAT};
        for (auto wrapMode : validWrapModesEXT)
        {
            glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, wrapMode);
            EXPECT_GL_NO_ERROR();
            glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, wrapMode);
            EXPECT_GL_NO_ERROR();
        }
    }
    else
    {
        GLenum invalidWrapModes[]{
            GL_REPEAT,
            GL_MIRRORED_REPEAT,
        };
        for (auto wrapMode : invalidWrapModes)
        {
            glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, wrapMode);
            EXPECT_GL_ERROR(GL_INVALID_ENUM);
            glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, wrapMode);
            EXPECT_GL_ERROR(GL_INVALID_ENUM);
        }
    }

    // When <target> is set to TEXTURE_EXTERNAL_OES, GenerateMipmap always fails and generates an
    // INVALID_ENUM error.
    glGenerateMipmap(GL_TEXTURE_EXTERNAL_OES);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
}

// Check validation from the GL_OES_EGL_image_external_essl3 extension
TEST_P(ImageTest, ValidationGLEGLImageExternalESSL3)
{
    ANGLE_SKIP_TEST_IF(!hasExternalESSL3Ext());

    // Make sure this extension is not exposed without ES3.
    ASSERT_GE(getClientMajorVersion(), 3);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture);

    // It is an INVALID_OPERATION error to set the TEXTURE_BASE_LEVEL to a value other than zero.
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_BASE_LEVEL, 1);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_BASE_LEVEL, 10);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_BASE_LEVEL, 0);
    EXPECT_GL_NO_ERROR();
}

// Check validation from the GL_EXT_EGL_image_storage extension
TEST_P(ImageTest, ValidationGLEGLImageStorage)
{
    ANGLE_SKIP_TEST_IF(!hasEglImageStorageExt());

    // Make sure this extension is not exposed without ES3.
    ASSERT_GE(getClientMajorVersion(), 3);

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());

    // Create the Image
    GLTexture source2D;
    EGLImageKHR image2D;
    createEGLImage2DTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, kDefaultAttribs, kLinearColor,
                                  source2D, &image2D);

    // <target> must be one of GL_TEXTURE_2D, GL_TEXTURE_2D_ARRAY, GL_TEXTURE_3D,
    // GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_ARRAY.  On OpenGL implementations
    // (non-ES), <target> can also be GL_TEXTURE_1D or GL_TEXTURE_1D_ARRAY.
    // If the implementation supports OES_EGL_image_external, <target> can be
    // GL_TEXTURE_EXTERNAL_OES
    glEGLImageTargetTexStorageEXT(GL_TEXTURE_CUBE_MAP_POSITIVE_X, image2D, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // If <image> is NULL, the error INVALID_VALUE is generated.  If <image> is
    // neither NULL nor a valid value, the behavior is undefined, up to and
    // including program termination.
    glEGLImageTargetTexStorageEXT(GL_TEXTURE_2D, nullptr, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // If the GL is unable to specify a texture object using the supplied
    // eglImageOES <image> the error INVALID_OPERATION is generated.
    glEGLImageTargetTexStorageEXT(GL_TEXTURE_3D, image2D, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    GLint nonNullAttrib[1] = {GL_TEXTURE_2D};

    // If <attrib_list> is neither NULL nor a pointer to the value GL_NONE, the
    // error INVALID_VALUE is generated.
    glEGLImageTargetTexStorageEXT(GL_TEXTURE_2D, image2D, nonNullAttrib);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // Clean up
    eglDestroyImageKHR(getEGLWindow()->getDisplay(), image2D);
}

TEST_P(ImageTest, Source2DTarget2D)
{
    Source2DTarget2D_helper(kDefaultAttribs);
}

TEST_P(ImageTest, Source2DTarget2D_Colorspace)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 && !IsGLExtensionEnabled("GL_EXT_sRGB"));
    ANGLE_SKIP_TEST_IF(!hasImageGLColorspaceExt());
    Source2DTarget2D_helper(kColorspaceAttribs);
}

void ImageTest::Source2DTarget2D_helper(const EGLint *attribs)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());

    // Create the Image
    GLTexture source;
    EGLImageKHR image;
    createEGLImage2DTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, attribs,
                                  static_cast<void *>(kSrgbColor), source, &image);

    // Create the target
    GLTexture target;
    createEGLImageTargetTexture2D(image, target);

    // Verify that the target texture has the expected color
    verifyResults2D(target, getExpected2DColorForAttribList(attribs));

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
}

void ImageTest::ImageStorageGenerateMipmap_helper(const EGLint *attribs,
                                                  const GLsizei width,
                                                  const GLsizei height,
                                                  AHardwareBuffer *srcAhb,
                                                  GLuint srcTexture,
                                                  EGLImageKHR *imageOut)
{
    ASSERT(srcAhb != nullptr || glIsTexture(srcTexture));

    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 && !IsGLExtensionEnabled("GL_EXT_sRGB"));
    ANGLE_SKIP_TEST_IF(!hasImageGLColorspaceExt());

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasEglImageStorageExt());

    constexpr int kNumTiles = 8;
    const int tileWidth     = width / kNumTiles;
    const int tileHeight    = height / kNumTiles;

    // Create EGLImage and then a target texture from that image
    EGLWindow *window = getEGLWindow();
    if (srcAhb != nullptr)
    {
        *imageOut =
            eglCreateImageKHR(window->getDisplay(), EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID,
                              angle::android::AHardwareBufferToClientBuffer(srcAhb), attribs);
    }
    else
    {
        *imageOut =
            eglCreateImageKHR(window->getDisplay(), window->getContext(), EGL_GL_TEXTURE_2D_KHR,
                              reinterpret_cast<EGLClientBuffer>(srcTexture), attribs);
    }
    ASSERT_EGL_SUCCESS();

    GLTexture dstTexture;
    glBindTexture(GL_TEXTURE_2D, dstTexture);
    // Setup for mipmapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    ASSERT_GL_NO_ERROR();

    glEGLImageTargetTexStorageEXT(GL_TEXTURE_2D, *imageOut, nullptr);
    ASSERT_GL_NO_ERROR();

    // Create framebuffer, attach level 0 of target texture and render pattern
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dstTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    glEnable(GL_SCISSOR_TEST);
    for (int i = 0; i < kNumTiles; ++i)
    {
        for (int j = 0; j < kNumTiles; ++j)
        {
            const float v = (i & 1) ^ (j & 1) ? 0.5f : 0.f;
            glClearColor(v, 0.f, v, v);
            glScissor(i * tileWidth, j * tileHeight, tileWidth, tileHeight);
            glClear(GL_COLOR_BUFFER_BIT);
        }
    }
    glDisable(GL_SCISSOR_TEST);

    // Generate mipmap for target texture
    glGenerateMipmap(GL_TEXTURE_2D);
}

void ImageTest::verifyImageStorageMipmap(const EGLint *attribs,
                                         EGLImageKHR image,
                                         const GLsizei mipLevelCount)
{
    if (image == EGL_NO_IMAGE_KHR)
    {
        // Early return if image isn't valid
        return;
    }

    GLubyte linearColor[] = {64, 0, 64, 64};
    GLubyte srgbColor[]   = {137, 0, 137, 64};
    GLubyte *expectedColor =
        attribListHasSrgbColorspace(attribs, kColorspaceAttributeIndex) ? srgbColor : linearColor;

    GLTexture targetTexture;
    glBindTexture(GL_TEXTURE_2D, targetTexture);
    // Setup for mipmapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    ASSERT_GL_NO_ERROR();

    glEGLImageTargetTexStorageEXT(GL_TEXTURE_2D, image, nullptr);
    ASSERT_GL_NO_ERROR();

    // Create target framebuffer, attach "(mipLevelCount - 1)" level of target texture and verify
    // data
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, targetTexture,
                           mipLevelCount - 1);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_PIXEL_NEAR(0, 0, expectedColor[0], expectedColor[1], expectedColor[2], expectedColor[3],
                      1);

    // Verify that the target texture generates linear color when sampled
    glActiveTexture(GL_TEXTURE0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, targetTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, mipLevelCount - 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipLevelCount - 1);
    ASSERT_GL_NO_ERROR();

    verifyResults2D(targetTexture, linearColor);
}

void ImageTest::verifyImageStorageMipmapWithBlend(const EGLint *attribs,
                                                  EGLImageKHR image,
                                                  const GLsizei mipLevelCount)
{
    if (image == EGL_NO_IMAGE_KHR)
    {
        // Early return if image isn't valid
        return;
    }

    // Need to have at least miplevel 1
    ASSERT(mipLevelCount >= 1);
    // Verification used by only those tests with colorspace overrides
    ASSERT(attribListHasSrgbColorspace(attribs, kColorspaceAttributeIndex));

    GLTexture targetTexture;
    glBindTexture(GL_TEXTURE_2D, targetTexture);
    // Setup for mipmapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    ASSERT_GL_NO_ERROR();

    glEGLImageTargetTexStorageEXT(GL_TEXTURE_2D, image, nullptr);
    ASSERT_GL_NO_ERROR();

    // Create target framebuffer, attach mipLevel == 1 of target texture and verify
    // data with blending enabled.
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, targetTexture, 1);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Blend green color with contents of mipLevel 1
    // source color at (7, 11) of mipLevel 1 = [137, 0, 137, 64]
    GLubyte blendedColor[] = {137, 255, 137, 255};
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    glUseProgram(program);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);

    EXPECT_PIXEL_NEAR(7, 11, blendedColor[0], blendedColor[1], blendedColor[2], blendedColor[3], 1);
}

void ImageTest::SourceAHBTarget2DImageStorageGenerateMipmap_helper(const EGLint *attribs)
{
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());

    constexpr GLsizei kWidth    = 40;
    constexpr GLsizei kHeight   = 32;
    constexpr GLsizei kDepth    = 1;
    const GLsizei mipLevelCount = static_cast<GLsizei>(std::log2(std::max(kWidth, kHeight)) + 1);

    ANGLE_SKIP_TEST_IF(!isAndroidHardwareBufferConfigurationSupported(
        kWidth, kHeight, kDepth, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
        kDefaultAHBUsage | kAHBUsageGPUMipMapComplete));

    // Create source AHB
    AHardwareBuffer *aHardwareBuffer =
        createAndroidHardwareBuffer(kWidth, kHeight, kDepth, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                    kDefaultAHBUsage | kAHBUsageGPUMipMapComplete, {});
    EXPECT_NE(aHardwareBuffer, nullptr);

    EGLImageKHR image = EGL_NO_IMAGE_KHR;
    ImageStorageGenerateMipmap_helper(attribs, kWidth, kHeight, aHardwareBuffer, 0, &image);
    verifyImageStorageMipmap(attribs, image, mipLevelCount);

    // Clean up image
    eglDestroyImageKHR(getEGLWindow()->getDisplay(), image);

    // Clean up AHB
    destroyAndroidHardwareBuffer(aHardwareBuffer);
}

// Test interaction between AHB, GL_EXT_EGL_image_storage and glGenerateMipmap
TEST_P(ImageTestES3, SourceAHBTarget2DGenerateMipmap)
{
    SourceAHBTarget2DImageStorageGenerateMipmap_helper(kDefaultAttribs);
}

// Test interaction between AHB, GL_EXT_EGL_image_storage and glGenerateMipmap with colorspace
// overrides This mirrors the SingleLayer_ColorTest_MipmapComplete_R8G8B8A8_UNORM_sRGB Android CTS
// test
TEST_P(ImageTestES3, SourceAHBTarget2DGenerateMipmap_Colorspace)
{
    SourceAHBTarget2DImageStorageGenerateMipmap_helper(kColorspaceAttribs);
}

// Test to ensure that Vulkan backend's LOAD_OP is correct for non-0 miplevels. A bug in
// content tracking of mip levels will cause rendering artifacts and result in test failure.
TEST_P(ImageTestES3, SourceAHBTarget2DGenerateMipmapColorspaceBlend)
{
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());

    constexpr GLsizei kWidth    = 40;
    constexpr GLsizei kHeight   = 32;
    constexpr GLsizei kDepth    = 1;
    const GLsizei mipLevelCount = static_cast<GLsizei>(std::log2(std::max(kWidth, kHeight)) + 1);

    ANGLE_SKIP_TEST_IF(!isAndroidHardwareBufferConfigurationSupported(
        kWidth, kHeight, kDepth, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
        kDefaultAHBUsage | kAHBUsageGPUMipMapComplete));

    // Create source AHB
    AHardwareBuffer *aHardwareBuffer =
        createAndroidHardwareBuffer(kWidth, kHeight, kDepth, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                    kDefaultAHBUsage | kAHBUsageGPUMipMapComplete, {});
    EXPECT_NE(aHardwareBuffer, nullptr);

    EGLImageKHR image = EGL_NO_IMAGE_KHR;
    ImageStorageGenerateMipmap_helper(kColorspaceAttribs, kWidth, kHeight, aHardwareBuffer, 0,
                                      &image);
    verifyImageStorageMipmapWithBlend(kColorspaceAttribs, image, mipLevelCount);

    // Clean up image
    eglDestroyImageKHR(getEGLWindow()->getDisplay(), image);

    // Clean up AHB
    destroyAndroidHardwareBuffer(aHardwareBuffer);
}

// Try to orphan image created with the GL_EXT_EGL_image_storage extension
TEST_P(ImageTestES3, Source2DTarget2DStorageOrphan)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasEglImageStorageExt());

    // Create the Image
    GLTexture source;
    EGLImageKHR image;
    createEGLImage2DTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, kDefaultAttribs,
                                  static_cast<void *>(&kLinearColor), source, &image);

    // Create the target
    GLTexture target;
    createEGLImageTargetTextureStorage(image, GL_TEXTURE_2D, target, nullptr);

    // Expect that the target texture has the same color as the source texture
    verifyResults2D(target, kLinearColor);

    // Try to orphan this target texture
    glBindTexture(GL_TEXTURE_2D, target);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, kLinearColor);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
}

// Create target texture from EGL image and then trigger texture respecification.
TEST_P(ImageTest, Source2DTarget2DTargetTextureRespecifyColorspace)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_format_sRGB_override"));

    // Create the Image
    GLTexture source;
    EGLImageKHR image;
    createEGLImage2DTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, kDefaultAttribs,
                                  static_cast<void *>(&kSrgbColor), source, &image);

    // Create the target
    GLTexture target;
    createEGLImageTargetTexture2D(image, target);

    // Expect that the target texture has the same color as the source texture
    verifyResults2D(target, kSrgbColor);

    // Respecify texture colorspace and verify results
    glBindTexture(GL_TEXTURE_2D, target);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_FORMAT_SRGB_OVERRIDE_EXT, GL_SRGB);
    ASSERT_GL_NO_ERROR();
    // Expect that the target texture decodes the sRGB color to linear when sampling
    verifyResults2D(target, kLinearColor);

    // Reset texture parameter and verify results again
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_FORMAT_SRGB_OVERRIDE_EXT, GL_NONE);
    ASSERT_GL_NO_ERROR();
    // Expect that the target texture has the same color as the source texture
    verifyResults2D(target, kSrgbColor);

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
}

// Create target texture from EGL image and then trigger texture respecification.
TEST_P(ImageTest, Source2DTarget2DTargetTextureRespecifySize)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());

    // Create the Image
    GLTexture source;
    EGLImageKHR image;
    createEGLImage2DTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, kDefaultAttribs,
                                  static_cast<void *>(&kLinearColor), source, &image);

    // Create the target
    GLTexture target;
    createEGLImageTargetTexture2D(image, target);

    // Expect that the target texture has the same color as the source texture
    verifyResults2D(target, kLinearColor);

    // Respecify texture size and verify results
    std::array<GLubyte, 16> referenceColor;
    referenceColor.fill(127);
    glBindTexture(GL_TEXTURE_2D, target);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 referenceColor.data());
    ASSERT_GL_NO_ERROR();

    // Expect that the target texture has the reference color values
    verifyResults2D(target, referenceColor.data());

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
}

// Create target texture from EGL image and then trigger texture respecification.
TEST_P(ImageTestES3, Source2DTarget2DTargetTextureRespecifyLevel)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());

    // Create the Image
    GLTexture source;
    EGLImageKHR image;
    createEGLImage2DTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, kDefaultAttribs,
                                  static_cast<void *>(&kLinearColor), source, &image);

    // Create the target
    GLTexture target;
    createEGLImageTargetTexture2D(image, target);

    // Expect that the target texture has the same color as the source texture
    verifyResults2D(target, kLinearColor);

    // Respecify texture levels and verify results
    glBindTexture(GL_TEXTURE_2D, target);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
    ASSERT_GL_NO_ERROR();

    // Expect that the target texture has the reference color values
    verifyResults2D(target, kLinearColor);

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
}

// Create target texture from EGL image and then trigger texture respecification which releases the
// last image ref.
TEST_P(ImageTest, ImageOrphanRefCountingBug)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());

    // Create the first Image
    GLTexture source1;
    EGLImageKHR image1;
    createEGLImage2DTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, kDefaultAttribs,
                                  static_cast<void *>(&kLinearColor), source1, &image1);

    // Create the target
    GLTexture target;
    createEGLImageTargetTexture2D(image1, target);

    // Delete the source and image. A ref is still held by the target texture
    source1.reset();
    eglDestroyImageKHR(window->getDisplay(), image1);

    // Create the second Image
    GLTexture source2;
    EGLImageKHR image2;
    createEGLImage2DTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, kDefaultAttribs,
                                  static_cast<void *>(&kLinearColor), source2, &image2);

    // Respecify the target with the second image.
    glBindTexture(GL_TEXTURE_2D, target);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image2);

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image2);
}

// Testing source 2D texture, target 2D array texture
TEST_P(ImageTest, Source2DTarget2DArray)
{
    Source2DTarget2DArray_helper(kDefaultAttribs);
}

// Testing source 2D texture with colorspace, target 2D array texture
TEST_P(ImageTest, Source2DTarget2DArray_Colorspace)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 && !IsGLExtensionEnabled("GL_EXT_sRGB"));
    ANGLE_SKIP_TEST_IF(!hasImageGLColorspaceExt());
    Source2DTarget2DArray_helper(kColorspaceAttribs);
}

void ImageTest::Source2DTarget2DArray_helper(const EGLint *attribs)
{
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt() ||
                       !hasEglImageArrayExt());

    // Create the Image
    GLTexture source;
    EGLImageKHR image;
    createEGLImage2DTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, attribs, kSrgbColor, source,
                                  &image);

    // Create the target
    GLTexture target;
    createEGLImageTargetTexture2DArray(image, target);

    // Verify that the target texture has the expected color
    verifyResults2DArray(target, getExpected2DColorForAttribList(attribs));

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
}

// Testing source AHB EGL image, if the client buffer is null, the test will not crash
TEST_P(ImageTest, SourceAHBInvalid)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !IsVulkan());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());

    // Create the Image
    EGLImageKHR image = eglCreateImageKHR(window->getDisplay(), EGL_NO_CONTEXT,
                                          EGL_NATIVE_BUFFER_ANDROID, nullptr, nullptr);
    ASSERT_EGL_ERROR(EGL_BAD_PARAMETER);
    EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
}

// Testing source AHB EGL image, if the client buffer is not a  ANativeWindowBuffer,
// eglCreateImageKHR should return NO_IMAGE and generate error EGL_BAD_PARAMETER.
TEST_P(ImageTest, SourceAHBCorrupt)
{
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !IsVulkan());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());

#if defined(ANGLE_AHARDWARE_BUFFER_SUPPORT)
    EGLWindow *window = getEGLWindow();

    const AHardwareBuffer_Desc aHardwareBufferDescription = createAndroidHardwareBufferDesc(
        16, 16, 1, AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM, kAHBUsageGPUSampledImage);

    // Allocate memory from Android Hardware Buffer
    AHardwareBuffer *aHardwareBuffer = nullptr;
    EXPECT_EQ(0, AHardwareBuffer_allocate(&aHardwareBufferDescription, &aHardwareBuffer));

    std::memset(
        reinterpret_cast<void *>(angle::android::AHardwareBufferToClientBuffer(aHardwareBuffer)), 0,
        sizeof(int));
    EGLImageKHR ahbImage = eglCreateImageKHR(
        window->getDisplay(), EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID,
        angle::android::AHardwareBufferToClientBuffer(aHardwareBuffer), kDefaultAttribs);

    ASSERT_EGL_ERROR(EGL_BAD_PARAMETER);
    EXPECT_EQ(ahbImage, EGL_NO_IMAGE_KHR);

    AHardwareBuffer_release(aHardwareBuffer);
#endif
}

// Helper function to check if it is reasonable to access texture resource
void ImageTest::ImageCheckingTextureAccessHelper(GLenum target, bool mipmap)
{
    constexpr GLsizei width = 2, height = 2, depth = 2;
    GLTexture source;
    EGLenum eglTarget;
    EGLWindow *window = getEGLWindow();

    glBindTexture(target, source);
    switch (target)
    {
        case GL_TEXTURE_2D:
            eglTarget = EGL_GL_TEXTURE_2D_KHR;
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                         nullptr);
            break;
        case GL_TEXTURE_3D:
            eglTarget = EGL_GL_TEXTURE_3D_KHR;
            glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, width, height, depth, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, nullptr);
            break;
        case GL_TEXTURE_CUBE_MAP:
            eglTarget = EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR;
            for (GLenum faceIdx = 0; faceIdx < 6; faceIdx++)
            {
                glTexImage2D(faceIdx + GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, width, height, 0,
                             GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            }
            break;
        default:
            return;
    }

    if (mipmap)
    {
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmap(target);
    }

    EGLImageKHR image = eglCreateImageKHR(window->getDisplay(), window->getContext(), eglTarget,
                                          reinterpretHelper<EGLClientBuffer>(source), nullptr);
    ASSERT_EGL_SUCCESS();
    EXPECT_NE(image, EGL_NO_IMAGE_KHR);

    // If the texture is bound to egl image, EGL_BAD_ACCESS should be returned.
    EGLImageKHR invalidImage1 =
        eglCreateImageKHR(window->getDisplay(), window->getContext(), eglTarget,
                          reinterpretHelper<EGLClientBuffer>(source), nullptr);
    ASSERT_EGL_ERROR(EGL_BAD_ACCESS);
    EXPECT_EQ(invalidImage1, EGL_NO_IMAGE_KHR);

    // If the image is destroyed, the texture could be bound to egl image here.
    eglDestroyImageKHR(window->getDisplay(), image);

    EGLImageKHR validImage1 =
        eglCreateImageKHR(window->getDisplay(), window->getContext(), eglTarget,
                          reinterpretHelper<EGLClientBuffer>(source), nullptr);
    ASSERT_EGL_SUCCESS();
    EXPECT_NE(validImage1, EGL_NO_IMAGE_KHR);

    if (target == GL_TEXTURE_3D)
    {
        constexpr EGLint zOffsetAttribs[] = {
            EGL_GL_TEXTURE_ZOFFSET,
            1,
            EGL_NONE,
        };
        EGLImageKHR validImage2 =
            eglCreateImageKHR(window->getDisplay(), window->getContext(), eglTarget,
                              reinterpretHelper<EGLClientBuffer>(source), zOffsetAttribs);
        ASSERT_EGL_SUCCESS();
        EXPECT_NE(validImage2, EGL_NO_IMAGE_KHR);
        eglDestroyImageKHR(window->getDisplay(), validImage2);
    }

    if (target == GL_TEXTURE_CUBE_MAP)
    {
        for (GLenum faceIdx = 1; faceIdx < 6; faceIdx++)
        {
            EGLImageKHR validImage2 =
                eglCreateImageKHR(window->getDisplay(), window->getContext(), eglTarget + faceIdx,
                                  reinterpretHelper<EGLClientBuffer>(source), nullptr);
            ASSERT_EGL_SUCCESS();
            EXPECT_NE(validImage2, EGL_NO_IMAGE_KHR);
            eglDestroyImageKHR(window->getDisplay(), validImage2);
        }
    }

    if (mipmap)
    {
        constexpr EGLint mipmapAttribs[] = {
            EGL_GL_TEXTURE_LEVEL,
            1,
            EGL_NONE,
        };
        EGLImageKHR validImage3 =
            eglCreateImageKHR(window->getDisplay(), window->getContext(), eglTarget,
                              reinterpretHelper<EGLClientBuffer>(source), mipmapAttribs);
        ASSERT_EGL_SUCCESS();
        EXPECT_NE(validImage3, EGL_NO_IMAGE_KHR);

        EGLImageKHR invalidImage2 =
            eglCreateImageKHR(window->getDisplay(), window->getContext(), eglTarget,
                              reinterpretHelper<EGLClientBuffer>(source), mipmapAttribs);
        ASSERT_EGL_ERROR(EGL_BAD_ACCESS);
        EXPECT_EQ(invalidImage2, EGL_NO_IMAGE_KHR);

        eglDestroyImageKHR(window->getDisplay(), validImage3);
        EGLImageKHR validImage4 =
            eglCreateImageKHR(window->getDisplay(), window->getContext(), eglTarget,
                              reinterpretHelper<EGLClientBuffer>(source), mipmapAttribs);
        ASSERT_EGL_SUCCESS();
        EXPECT_NE(validImage4, EGL_NO_IMAGE_KHR);
        eglDestroyImageKHR(window->getDisplay(), validImage4);
    }

    eglDestroyImageKHR(window->getDisplay(), validImage1);
}

// Testing GLES resources when creating EGL image, if the client buffer itself is an EGL sibling,
// eglCreateImageKHR should return NO_IMAGE and generate error EGL_BAD_ACCESS.
TEST_P(ImageTest, SourceBadAccess)
{
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !IsVulkan());

    // Validate gles 2D texture
    if (has2DTextureExt())
    {
        ImageCheckingTextureAccessHelper(GL_TEXTURE_2D, false);
        ImageCheckingTextureAccessHelper(GL_TEXTURE_2D, true);
    }

    // Validate gles 3D texture
    if (has3DTextureExt() && getClientMajorVersion() >= 3)
    {
        ImageCheckingTextureAccessHelper(GL_TEXTURE_3D, false);
        ImageCheckingTextureAccessHelper(GL_TEXTURE_3D, true);
    }

    // Validate gles cube map texture
    if (hasCubemapExt())
    {
        ImageCheckingTextureAccessHelper(GL_TEXTURE_CUBE_MAP, false);
        ImageCheckingTextureAccessHelper(GL_TEXTURE_CUBE_MAP, true);
    }

    // Validate gles renderbuffer
    if (hasRenderbufferExt())
    {
        EGLWindow *window = getEGLWindow();
        GLRenderbuffer source;
        EGLImageKHR image;
        createEGLImageRenderbufferSource(1, 1, GL_RGBA8_OES, kDefaultAttribs, source, &image);

        EGLImageKHR invalidImage =
            eglCreateImageKHR(window->getDisplay(), window->getContext(), EGL_GL_RENDERBUFFER_KHR,
                              reinterpretHelper<EGLClientBuffer>(source), kDefaultAttribs);
        ASSERT_EGL_ERROR(EGL_BAD_ACCESS);
        EXPECT_EQ(invalidImage, EGL_NO_IMAGE_KHR);
        eglDestroyImageKHR(window->getDisplay(), image);

        EGLImageKHR validImage =
            eglCreateImageKHR(window->getDisplay(), window->getContext(), EGL_GL_RENDERBUFFER_KHR,
                              reinterpretHelper<EGLClientBuffer>(source), kDefaultAttribs);
        ASSERT_EGL_SUCCESS();
        EXPECT_NE(validImage, EGL_NO_IMAGE_KHR);
        eglDestroyImageKHR(window->getDisplay(), validImage);
    }
}

// Testing source AHB EGL image, target 2D texture and delete when in use
// If refcounted correctly, the test should pass without issues
TEST_P(ImageTest, SourceAHBTarget2DEarlyDelete)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());

    GLubyte data[4] = {7, 51, 197, 231};

    // Create the Image
    AHardwareBuffer *source;
    EGLImageKHR image;
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                              kDefaultAHBUsage, kDefaultAttribs, {{data, 4}},
                                              &source, &image);

    // Create a texture target to bind the egl image
    GLTexture target;
    createEGLImageTargetTexture2D(image, target);

    // Delete the source AHB when in use
    destroyAndroidHardwareBuffer(source);

    // Use texture target bound to egl image as source and render to framebuffer
    // Verify that data in framebuffer matches that in the egl image
    verifyResults2D(target, data);

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
}

// Testing source AHB EGL image, target 2D texture
TEST_P(ImageTest, SourceAHBTarget2D)
{
    SourceAHBTarget2D_helper(kDefaultAttribs);
}

// Testing source AHB EGL image with colorspace, target 2D texture
TEST_P(ImageTest, SourceAHBTarget2D_Colorspace)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 && !IsGLExtensionEnabled("GL_EXT_sRGB"));
    ANGLE_SKIP_TEST_IF(!hasImageGLColorspaceExt());
    SourceAHBTarget2D_helper(kColorspaceAttribs);
}

void ImageTest::SourceAHBTarget2D_helper(const EGLint *attribs)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    // Create the Image
    AHardwareBuffer *source;
    EGLImageKHR image;
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                              kDefaultAHBUsage, attribs, {{kSrgbColor, 4}}, &source,
                                              &image);

    // Create a texture target to bind the egl image
    GLTexture target;
    createEGLImageTargetTexture2D(image, target);

    // Use texture target bound to egl image as source and render to framebuffer
    // Verify that the target texture has the expected color
    verifyResults2D(target, getExpected2DColorForAttribList(attribs));

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
    destroyAndroidHardwareBuffer(source);
}

// Testing source AHB EGL images, target 2D external texture, cycling through YUV sources.
TEST_P(ImageTest, SourceAHBTarget2DExternalCycleThroughYuvSourcesNoData)
{
    // http://issuetracker.google.com/175021871
    ANGLE_SKIP_TEST_IF(IsPixel2() || IsPixel2XL());

    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());

    // Create YCbCr source and image but without initial data
    AHardwareBuffer *ycbcrSource;
    EGLImageKHR ycbcrImage;
    createEGLImageAndroidHardwareBufferSource(2, 2, 1, AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420,
                                              kDefaultAHBUsage, kDefaultAttribs, {}, &ycbcrSource,
                                              &ycbcrImage);
    EXPECT_NE(ycbcrSource, nullptr);
    EXPECT_NE(ycbcrImage, EGL_NO_IMAGE_KHR);

    // Create YCrCb source and image but without initial data
    AHardwareBuffer *ycrcbSource;
    EGLImageKHR ycrcbImage;
    createEGLImageAndroidHardwareBufferSource(2, 2, 1, AHARDWAREBUFFER_FORMAT_Y8Cr8Cb8_420_SP,
                                              kDefaultAHBUsage, kDefaultAttribs, {}, &ycrcbSource,
                                              &ycrcbImage);
    EXPECT_NE(ycrcbSource, nullptr);
    EXPECT_NE(ycrcbImage, EGL_NO_IMAGE_KHR);

    // Create YV12 source and image but without initial data
    AHardwareBuffer *yv12Source;
    EGLImageKHR yv12Image;
    createEGLImageAndroidHardwareBufferSource(2, 2, 1, AHARDWAREBUFFER_FORMAT_YV12,
                                              kDefaultAHBUsage, kDefaultAttribs, {}, &yv12Source,
                                              &yv12Image);
    EXPECT_NE(yv12Source, nullptr);
    EXPECT_NE(yv12Image, EGL_NO_IMAGE_KHR);

    // Create a texture target to bind the egl image
    GLTexture target;
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, target);
    // Disable mipmapping
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Bind YCbCr image
    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, ycbcrImage);
    // Draw while sampling should result in no EGL/GL errors
    glUseProgram(mTextureExternalProgram);
    glUniform1i(mTextureExternalUniformLocation, 0);
    drawQuad(mTextureExternalProgram, "position", 0.5f);
    ASSERT_GL_NO_ERROR();

    // Bind YCrCb image
    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, ycrcbImage);
    // Draw while sampling should result in no EGL/GL errors
    glUseProgram(mTextureExternalProgram);
    glUniform1i(mTextureExternalUniformLocation, 0);
    drawQuad(mTextureExternalProgram, "position", 0.5f);
    ASSERT_GL_NO_ERROR();

    // Bind YV12 image
    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, yv12Image);
    // Draw while sampling should result in no EGL/GL errors
    glUseProgram(mTextureExternalProgram);
    glUniform1i(mTextureExternalUniformLocation, 0);
    drawQuad(mTextureExternalProgram, "position", 0.5f);
    ASSERT_GL_NO_ERROR();

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), ycbcrImage);
    destroyAndroidHardwareBuffer(ycbcrSource);
    eglDestroyImageKHR(window->getDisplay(), ycrcbImage);
    destroyAndroidHardwareBuffer(ycrcbSource);
    eglDestroyImageKHR(window->getDisplay(), yv12Image);
    destroyAndroidHardwareBuffer(yv12Source);
}

// Testing source AHB EGL images, target 2D external texture, cycling through RGB and YUV sources.
TEST_P(ImageTest, SourceAHBTarget2DExternalCycleThroughRgbAndYuvSources)
{
    // http://issuetracker.google.com/175021871
    ANGLE_SKIP_TEST_IF(IsPixel2() || IsPixel2XL());

    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    ANGLE_SKIP_TEST_IF(!isAndroidHardwareBufferConfigurationSupported(
        1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM, kDefaultAHBUsage));
    ANGLE_SKIP_TEST_IF(!isAndroidHardwareBufferConfigurationSupported(
        2, 2, 1, AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420, kDefaultAHBUsage));

    // Create RGB Image
    GLubyte rgbColor[4] = {0, 0, 255, 255};

    AHardwareBuffer *rgbSource;
    EGLImageKHR rgbImage;
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                              kDefaultAHBUsage, kDefaultAttribs, {{rgbColor, 4}},
                                              &rgbSource, &rgbImage);

    // Create YUV Image
    // 3 planes of data
    GLubyte dataY[4]  = {40, 40, 40, 40};
    GLubyte dataCb[1] = {
        240,
    };
    GLubyte dataCr[1] = {
        109,
    };

    GLubyte expectedRgbColor[4] = {0, 0, 255, 255};

    AHardwareBuffer *yuvSource;
    EGLImageKHR yuvImage;
    createEGLImageAndroidHardwareBufferSource(
        2, 2, 1, AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420, kDefaultAHBUsage, kDefaultAttribs,
        {{dataY, 1}, {dataCb, 1}, {dataCr, 1}}, &yuvSource, &yuvImage);

    // Create a texture target to bind the egl image
    GLTexture target;
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, target);
    // Disable mipmapping
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Bind YUV image
    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, yuvImage);
    // Expect render target to have the same color as expectedRgbColor
    verifyResultsExternal(target, expectedRgbColor);

    // Bind RGB image
    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, rgbImage);
    // Expect render target to have the same color as rgbColor
    verifyResultsExternal(target, rgbColor);

    // Bind YUV image
    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, yuvImage);
    // Expect render target to have the same color as expectedRgbColor
    verifyResultsExternal(target, expectedRgbColor);

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), yuvImage);
    destroyAndroidHardwareBuffer(yuvSource);
    eglDestroyImageKHR(window->getDisplay(), rgbImage);
    destroyAndroidHardwareBuffer(rgbSource);
}

// Testing source AHB EGL images, target 2D external textures, cycling through RGB and YUV targets.
TEST_P(ImageTest, SourceAHBTarget2DExternalCycleThroughRgbAndYuvTargets)
{
    // http://issuetracker.google.com/175021871
    ANGLE_SKIP_TEST_IF(IsPixel2() || IsPixel2XL());

    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    // Create RGBA Image
    GLubyte rgbaColor[4] = {0, 0, 255, 255};

    AHardwareBuffer *rgbaSource;
    EGLImageKHR rgbaImage;
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                              kDefaultAHBUsage, kDefaultAttribs, {{rgbaColor, 4}},
                                              &rgbaSource, &rgbaImage);

    // Create YUV Image
    // 3 planes of data
    GLubyte dataY[4]  = {40, 40, 40, 40};
    GLubyte dataCb[1] = {
        240,
    };
    GLubyte dataCr[1] = {
        109,
    };

    GLubyte expectedRgbColor[4] = {0, 0, 255, 255};

    AHardwareBuffer *yuvSource;
    EGLImageKHR yuvImage;
    createEGLImageAndroidHardwareBufferSource(
        2, 2, 1, AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420, kDefaultAHBUsage, kDefaultAttribs,
        {{dataY, 1}, {dataCb, 1}, {dataCr, 1}}, &yuvSource, &yuvImage);

    // Create texture target siblings to bind the egl images
    // Create YUV target and bind the image
    GLTexture yuvTarget;
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, yuvTarget);
    // Disable mipmapping
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();
    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, yuvImage);
    ASSERT_GL_NO_ERROR();

    // Create RGBA target and bind the image
    GLTexture rgbaTarget;
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, rgbaTarget);
    // Disable mipmapping
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();
    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, rgbaImage);
    ASSERT_GL_NO_ERROR();

    // Cycle through targets
    // YUV target
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, yuvTarget);
    // Expect render target to have the same color as expectedRgbColor
    verifyResultsExternal(yuvTarget, expectedRgbColor);

    // RGBA target
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, rgbaTarget);
    // Expect render target to have the same color as rgbColor
    verifyResultsExternal(rgbaTarget, rgbaColor);

    // YUV target
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, yuvTarget);
    // Expect render target to have the same color as expectedRgbColor
    verifyResultsExternal(yuvTarget, expectedRgbColor);

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), yuvImage);
    destroyAndroidHardwareBuffer(yuvSource);
    eglDestroyImageKHR(window->getDisplay(), rgbaImage);
    destroyAndroidHardwareBuffer(rgbaSource);
}

// Testing source AHB EGL images, target 2D external textures, cycling through YUV targets.
TEST_P(ImageTest, SourceAHBTarget2DExternalCycleThroughYuvTargetsNoData)
{
    // http://issuetracker.google.com/175021871
    ANGLE_SKIP_TEST_IF(IsPixel2() || IsPixel2XL());

    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());

    // Create YCbCr source and image but without initial data
    AHardwareBuffer *ycbcrSource;
    EGLImageKHR ycbcrImage;
    createEGLImageAndroidHardwareBufferSource(2, 2, 1, AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420,
                                              kDefaultAHBUsage, kDefaultAttribs, {}, &ycbcrSource,
                                              &ycbcrImage);
    EXPECT_NE(ycbcrSource, nullptr);
    EXPECT_NE(ycbcrImage, EGL_NO_IMAGE_KHR);

    // Create YV12 source and image but without initial data
    AHardwareBuffer *yv12Source;
    EGLImageKHR yv12Image;
    createEGLImageAndroidHardwareBufferSource(2, 2, 1, AHARDWAREBUFFER_FORMAT_YV12,
                                              kDefaultAHBUsage, kDefaultAttribs, {}, &yv12Source,
                                              &yv12Image);
    EXPECT_NE(yv12Source, nullptr);
    EXPECT_NE(yv12Image, EGL_NO_IMAGE_KHR);

    // Create texture target siblings to bind the egl images
    // Create YCbCr target and bind the image
    GLTexture ycbcrTarget;
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, ycbcrTarget);
    // Disable mipmapping
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();
    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, ycbcrImage);
    ASSERT_GL_NO_ERROR();

    // Create YV12 target and bind the image
    GLTexture yv12Target;
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, yv12Target);
    // Disable mipmapping
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();
    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, yv12Image);
    ASSERT_GL_NO_ERROR();

    // Cycle through targets
    glUseProgram(mTextureExternalProgram);
    glUniform1i(mTextureExternalUniformLocation, 0);

    // Bind YCbCr image
    // YCbCr target
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, ycbcrTarget);
    // Draw while sampling should result in no EGL/GL errors
    drawQuad(mTextureExternalProgram, "position", 0.5f);
    ASSERT_GL_NO_ERROR();

    // YV12 target
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, yv12Target);
    // Draw while sampling should result in no EGL/GL errors
    drawQuad(mTextureExternalProgram, "position", 0.5f);
    ASSERT_GL_NO_ERROR();

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), ycbcrImage);
    destroyAndroidHardwareBuffer(ycbcrSource);
    eglDestroyImageKHR(window->getDisplay(), yv12Image);
    destroyAndroidHardwareBuffer(yv12Source);
}

// Testing source AHB EGL image, target 2D texture retaining initial data.
TEST_P(ImageTest, SourceAHBTarget2DRetainInitialData)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    GLubyte data[4] = {0, 255, 0, 255};

    // Create the Image
    AHardwareBuffer *source;
    EGLImageKHR image;
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                              kDefaultAHBUsage, kDefaultAttribs, {{data, 4}},
                                              &source, &image);

    // Create a texture target to bind the egl image
    GLTexture target;
    createEGLImageTargetTexture2D(image, target);

    // Create a framebuffer, and blend into the texture.
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Blend into the framebuffer.
    ANGLE_GL_PROGRAM(drawRed, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    drawQuad(drawRed, essl1_shaders::PositionAttrib(), 0.0f);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_BLEND);

    // Use texture target bound to egl image as source and render to framebuffer
    // Verify that data in framebuffer matches that in the egl image
    GLubyte expect[4] = {255, 255, 0, 255};
    verifyResults2D(target, expect);

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
    destroyAndroidHardwareBuffer(source);
}

// Test interaction between AHBs and GL_EXT_multisampled_render_to_texture
TEST_P(ImageTest, SourceAHBTarget2DMSRTTInteraction)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_multisampled_render_to_texture"));

    // Create the Image
    AHardwareBuffer *source;
    EGLImageKHR image;
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                              kDefaultAHBUsage, kDefaultAttribs, {}, &source,
                                              &image);

    // Create the target
    GLTexture target;
    createEGLImageTargetTexture2D(image, target);

    // Bind target texture to mulisampled framebuffer
    GLFramebuffer fboMS;
    glBindFramebuffer(GL_FRAMEBUFFER, fboMS);
    glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                         target, 0, 4);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear framebuffer
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    // Check clear result
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);

    // Check the AHB is updated
    verifyResultAHB(source, {{GLColor::blue.data(), 4}});

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
}
// Testing source AHB EGL image, target 2D array texture
TEST_P(ImageTest, SourceAHBTarget2DArray)
{
    SourceAHBTarget2DArray_helper(kDefaultAttribs);
}

// Testing source AHB EGL image with colorspace, target 2D array texture
TEST_P(ImageTest, SourceAHBTarget2DArray_Colorspace)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 && !IsGLExtensionEnabled("GL_EXT_sRGB"));
    ANGLE_SKIP_TEST_IF(!hasImageGLColorspaceExt());
    SourceAHBTarget2DArray_helper(kColorspaceAttribs);
}

void ImageTest::SourceAHBTarget2DArray_helper(const EGLint *attribs)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt() ||
                       !hasEglImageArrayExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    // Create the Image
    AHardwareBuffer *source;
    EGLImageKHR image;
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                              kDefaultAHBUsage, attribs, {{kSrgbColor, 4}}, &source,
                                              &image);

    // Create a texture target to bind the egl image
    GLTexture target;
    createEGLImageTargetTexture2DArray(image, target);

    // Use texture target bound to egl image as source and render to framebuffer
    // Verify that the target texture has the expected color
    verifyResults2DArray(target, getExpected2DColorForAttribList(attribs));

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
    destroyAndroidHardwareBuffer(source);
}

// Testing source AHB EGL image, target external texture
TEST_P(ImageTest, SourceAHBTargetExternal)
{
    SourceAHBTargetExternal_helper(kDefaultAttribs);
}

// Testing source AHB EGL image with colorspace, target external texture
TEST_P(ImageTest, SourceAHBTargetExternal_Colorspace)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 && !IsGLExtensionEnabled("GL_EXT_sRGB"));
    ANGLE_SKIP_TEST_IF(!hasImageGLColorspaceExt());
    SourceAHBTargetExternal_helper(kColorspaceAttribs);
}

void ImageTest::SourceAHBTargetExternal_helper(const EGLint *attribs)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt() || !hasExternalExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    // Ozone only supports external target for images created with EGL_EXT_image_dma_buf_import
    ANGLE_SKIP_TEST_IF(IsOzone());

    // Create the Image
    AHardwareBuffer *source;
    EGLImageKHR image;
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                              kDefaultAHBUsage, attribs, {{kSrgbColor, 4}}, &source,
                                              &image);

    // Create a texture target to bind the egl image
    GLTexture target;
    createEGLImageTargetTextureExternal(image, target);

    // Use texture target bound to egl image as source and render to framebuffer
    // Verify that the target texture has the expected color
    verifyResultsExternal(target, getExpected2DColorForAttribList(attribs));

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
    destroyAndroidHardwareBuffer(source);
}

// Testing source AHB EGL image, target external ESSL3 texture
TEST_P(ImageTestES3, SourceAHBTargetExternalESSL3)
{
    SourceAHBTargetExternalESSL3_helper(kDefaultAttribs);
}

// Test sampling from a YUV texture using GL_ANGLE_yuv_internal_format as external texture and then
// switching to raw YUV sampling using EXT_yuv_target
TEST_P(ImageTestES3, SourceYUVTextureTargetExternalRGBSampleYUVSample)
{
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt() ||
                       !hasYUVInternalFormatExt() || !hasYUVTargetExt());

    // Create source YUV texture
    GLTexture yuvTexture;
    GLubyte yuvColor[6]         = {7, 51, 197, 231, 128, 192};
    GLubyte expectedRgbColor[4] = {255, 159, 211, 255};
    constexpr size_t kWidth     = 2;
    constexpr size_t kHeight    = 2;

    glBindTexture(GL_TEXTURE_2D, yuvTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_G8_B8R8_2PLANE_420_UNORM_ANGLE, kWidth, kHeight);
    ASSERT_GL_NO_ERROR();
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth, kHeight, GL_G8_B8R8_2PLANE_420_UNORM_ANGLE,
                    GL_UNSIGNED_BYTE, yuvColor);
    ASSERT_GL_NO_ERROR();
    // Disable mipmapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Create the Image
    EGLWindow *window = getEGLWindow();
    EGLImageKHR image =
        eglCreateImageKHR(window->getDisplay(), window->getContext(), EGL_GL_TEXTURE_2D_KHR,
                          reinterpretHelper<EGLClientBuffer>(yuvTexture), kDefaultAttribs);
    ASSERT_EGL_SUCCESS();

    // Create a texture target to bind the egl image
    GLTexture target;
    createEGLImageTargetTextureExternal(image, target);

    // Draw quad with program that samples YUV data with implicit RGB conversion
    glUseProgram(mTextureExternalProgram);
    glUniform1i(mTextureExternalUniformLocation, 0);
    drawQuad(mTextureExternalProgram, "position", 0.5f);
    // Expect that the rendered quad's color is converted to RGB
    EXPECT_PIXEL_NEAR(0, 0, expectedRgbColor[0], expectedRgbColor[1], expectedRgbColor[2],
                      expectedRgbColor[3], 1);

    // Draw quad with program that samples raw YUV data
    glUseProgram(mTextureYUVProgram);
    glUniform1i(mTextureYUVUniformLocation, 0);
    drawQuad(mTextureYUVProgram, "position", 0.5f);
    // Expect that the rendered quad's color matches the raw YUV data
    EXPECT_PIXEL_NEAR(0, 0, yuvColor[2], yuvColor[4], yuvColor[5], 255, 1);

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
}

// Similar to SourceYUVTextureTargetExternalRGBSampleYUVSample, but added swizzle after
// __samplerExternal2DY2YEXT from texture.
TEST_P(ImageTestES3, SourceYUVTextureTargetExternalRGBSampleYUVSampleWithSwizzle)
{
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt() ||
                       !hasYUVInternalFormatExt() || !hasYUVTargetExt());

    // Create source YUV texture
    GLTexture yuvTexture;
    GLubyte yuvColor[6]      = {7, 51, 197, 231, 128, 192};
    constexpr size_t kWidth  = 2;
    constexpr size_t kHeight = 2;

    glBindTexture(GL_TEXTURE_2D, yuvTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_G8_B8R8_2PLANE_420_UNORM_ANGLE, kWidth, kHeight);
    ASSERT_GL_NO_ERROR();
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth, kHeight, GL_G8_B8R8_2PLANE_420_UNORM_ANGLE,
                    GL_UNSIGNED_BYTE, yuvColor);
    ASSERT_GL_NO_ERROR();
    // Disable mipmapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Create the Image
    EGLWindow *window = getEGLWindow();
    EGLImageKHR image =
        eglCreateImageKHR(window->getDisplay(), window->getContext(), EGL_GL_TEXTURE_2D_KHR,
                          reinterpretHelper<EGLClientBuffer>(yuvTexture), kDefaultAttribs);
    ASSERT_EGL_SUCCESS();

    // Create a texture target to bind the egl image
    GLTexture target;
    createEGLImageTargetTextureExternal(image, target);

    // Draw quad with program that samples raw YUV data and then swizzle
    const char *fragmentShaderSource = R"(#version 300 es
#extension GL_EXT_YUV_target : require
precision highp float;
uniform __samplerExternal2DY2YEXT tex;
in vec2 texcoord;
out vec4 color;

void main()
{
    color = vec4(texture(tex, texcoord).zyx, 1.0);
})";
    ANGLE_GL_PROGRAM(textureYUVProgram, getVSESSL3(), fragmentShaderSource);
    ASSERT_NE(0u, textureYUVProgram) << "shader compilation failed.";
    glUseProgram(textureYUVProgram);
    GLint uniformLocation = glGetUniformLocation(textureYUVProgram, "tex");
    ASSERT_NE(-1, uniformLocation);
    glUniform1i(uniformLocation, 0);
    drawQuad(textureYUVProgram, "position", 0.5f);
    // Expect that the rendered quad's color matches the raw YUV data after component swizzle
    EXPECT_PIXEL_NEAR(0, 0, yuvColor[5], yuvColor[4], yuvColor[2], 255, 1);

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
}

// Test interaction between GL_ANGLE_yuv_internal_format and EXT_yuv_target when a program has
// both __samplerExternal2DY2YEXT and samplerExternalOES samplers.
TEST_P(ImageTestES3, ProgramWithBothExternalY2YAndExternalOESSampler)
{
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt() ||
                       !hasYUVInternalFormatExt() || !hasYUVTargetExt());

    GLubyte yuvColor[6]         = {40, 40, 40, 40, 240, 109};
    GLubyte expectedRgbColor[4] = {0, 0, 255, 255};
    constexpr size_t kWidth     = 2;
    constexpr size_t kHeight    = 2;

    // Create 2 plane YUV texture source
    GLTexture yuvTexture0;
    glBindTexture(GL_TEXTURE_2D, yuvTexture0);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_G8_B8R8_2PLANE_420_UNORM_ANGLE, kWidth, kHeight);
    ASSERT_GL_NO_ERROR();
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth, kHeight, GL_G8_B8R8_2PLANE_420_UNORM_ANGLE,
                    GL_UNSIGNED_BYTE, yuvColor);
    ASSERT_GL_NO_ERROR();
    // Disable mipmapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Create 2 plane YUV texture source
    GLTexture yuvTexture1;
    glBindTexture(GL_TEXTURE_2D, yuvTexture1);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_G8_B8R8_2PLANE_420_UNORM_ANGLE, kWidth, kHeight);
    ASSERT_GL_NO_ERROR();
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth, kHeight, GL_G8_B8R8_2PLANE_420_UNORM_ANGLE,
                    GL_UNSIGNED_BYTE, yuvColor);
    ASSERT_GL_NO_ERROR();
    // Disable mipmapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Create the Images
    EGLWindow *window = getEGLWindow();
    EGLImageKHR image0 =
        eglCreateImageKHR(window->getDisplay(), window->getContext(), EGL_GL_TEXTURE_2D_KHR,
                          reinterpretHelper<EGLClientBuffer>(yuvTexture0), kDefaultAttribs);
    ASSERT_EGL_SUCCESS();

    EGLImageKHR image1 =
        eglCreateImageKHR(window->getDisplay(), window->getContext(), EGL_GL_TEXTURE_2D_KHR,
                          reinterpretHelper<EGLClientBuffer>(yuvTexture1), kDefaultAttribs);
    ASSERT_EGL_SUCCESS();

    // Create texture targets for EGLImages
    GLTexture target0;
    createEGLImageTargetTextureExternal(image0, target0);

    GLTexture target1;
    createEGLImageTargetTextureExternal(image1, target1);

    // Create program with 2 samplers
    const char *vertexShaderSource   = R"(#version 300 es
out vec2 texcoord;
in vec4 position;
void main()
{
    gl_Position = vec4(position.xy, 0.0, 1.0);
    texcoord = (position.xy * 0.5) + 0.5;
})";
    const char *fragmentShaderSource = R"(#version 300 es
#extension GL_EXT_YUV_target : require
#extension GL_OES_EGL_image_external_essl3 : require
precision highp float;
uniform __samplerExternal2DY2YEXT tex0;
uniform samplerExternalOES tex1;
uniform uint samplerSelector;
in vec2 texcoord;
out vec4 fragColor;

void main()
{
    vec4 color0 = texture(tex0, texcoord);
    vec4 color1 = texture(tex1, texcoord);
    if (samplerSelector == 0u)
    {
        fragColor = color0;
    }
    else if (samplerSelector == 1u)
    {
        fragColor = color1;
    }
    else
    {
        fragColor = vec4(1.0);
    }
})";

    ANGLE_GL_PROGRAM(twoSamplersProgram, vertexShaderSource, fragmentShaderSource);
    glUseProgram(twoSamplersProgram);
    GLint tex0Location = glGetUniformLocation(twoSamplersProgram, "tex0");
    ASSERT_NE(-1, tex0Location);
    GLint tex1Location = glGetUniformLocation(twoSamplersProgram, "tex1");
    ASSERT_NE(-1, tex1Location);
    GLint samplerSelectorLocation = glGetUniformLocation(twoSamplersProgram, "samplerSelector");
    ASSERT_NE(-1, samplerSelectorLocation);

    // Bind texture target to GL_TEXTURE_EXTERNAL_OES
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, target0);
    ASSERT_GL_NO_ERROR();

    // Bind texture target to GL_TEXTURE_EXTERNAL_OES
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, target1);
    ASSERT_GL_NO_ERROR();

    // Set sampler uniform values
    glUniform1i(tex0Location, 0);
    glUniform1i(tex1Location, 1);

    // Set sampler selector uniform value and draw
    glUniform1ui(samplerSelectorLocation, 0);
    drawQuad(twoSamplersProgram, "position", 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(0, 0, yuvColor[3], yuvColor[4], yuvColor[5], 255, 1);

    // Switch sampler selector uniform value and draw
    glUniform1ui(samplerSelectorLocation, 1);
    drawQuad(twoSamplersProgram, "position", 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(0, 0, expectedRgbColor[0], expectedRgbColor[1], expectedRgbColor[2],
                      expectedRgbColor[3], 1);

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image0);
    eglDestroyImageKHR(window->getDisplay(), image1);
}

// Test sampling from a YUV AHB with a regular external sampler and pre-initialized data
TEST_P(ImageTest, SourceYUVAHBTargetExternalRGBSampleInitData)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    // http://issuetracker.google.com/175021871
    ANGLE_SKIP_TEST_IF(IsPixel2() || IsPixel2XL());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    // 3 planes of data
    GLubyte dataY[4]  = {7, 51, 197, 231};
    GLubyte dataCb[1] = {
        128,
    };
    GLubyte dataCr[1] = {
        192,
    };

    // Create the Image
    AHardwareBuffer *source;
    EGLImageKHR image;
    createEGLImageAndroidHardwareBufferSource(
        2, 2, 1, AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420, kDefaultAHBUsage, kDefaultAttribs,
        {{dataY, 1}, {dataCb, 1}, {dataCr, 1}}, &source, &image);

    // Create a texture target to bind the egl image
    GLTexture target;
    createEGLImageTargetTextureExternal(image, target);

    GLubyte pixelColor[4] = {255, 159, 211, 255};
    verifyResultsExternal(target, pixelColor);

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
    destroyAndroidHardwareBuffer(source);
}

// Test sampling from a YUV AHB with a regular external sampler without data. This gives coverage of
// sampling even if we can't verify the results.
TEST_P(ImageTest, SourceYUVAHBTargetExternalRGBSampleNoData)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());

    // Create the Image without data so we don't need ANGLE_AHARDWARE_BUFFER_LOCK_PLANES_SUPPORT
    AHardwareBuffer *source;
    EGLImageKHR image;
    createEGLImageAndroidHardwareBufferSource(2, 2, 1, AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420,
                                              kDefaultAHBUsage, kDefaultAttribs, {}, &source,
                                              &image);

    // Create a texture target to bind the egl image
    GLTexture target;
    createEGLImageTargetTextureExternal(image, target);

    glUseProgram(mTextureExternalProgram);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, target);
    glUniform1i(mTextureExternalUniformLocation, 0);

    // Sample from the YUV texture with a nearest sampler
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    drawQuad(mTextureExternalProgram, "position", 0.5f);

    // Sample from the YUV texture with a linear sampler
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    drawQuad(mTextureExternalProgram, "position", 0.5f);

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
    destroyAndroidHardwareBuffer(source);
}

// Test sampling from a YUV AHB using EXT_yuv_target
TEST_P(ImageTestES3, SourceYUVAHBTargetExternalYUVSample)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt() || !hasYUVTargetExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    // 3 planes of data
    GLubyte dataY[4]  = {7, 51, 197, 231};
    GLubyte dataCb[1] = {
        128,
    };
    GLubyte dataCr[1] = {
        192,
    };

    // Create the Image
    AHardwareBuffer *source;
    EGLImageKHR image;
    createEGLImageAndroidHardwareBufferSource(
        2, 2, 1, AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420, kDefaultAHBUsage, kDefaultAttribs,
        {{dataY, 1}, {dataCb, 1}, {dataCr, 1}}, &source, &image);

    // Create a texture target to bind the egl image
    GLTexture target;
    createEGLImageTargetTextureExternal(image, target);

    GLubyte pixelColor[4] = {197, 128, 192, 255};
    verifyResultsExternalYUV(target, pixelColor);

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
    destroyAndroidHardwareBuffer(source);
}

TEST_P(ImageTestES3, SourceYUVAHBTargetExternalYUVSampleLinearFiltering)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    ANGLE_SKIP_TEST_IF(!isAndroidHardwareBufferConfigurationSupported(
        2, 4, 1, AHARDWAREBUFFER_FORMAT_YV12, kDefaultAHBYUVUsage));

    // [  Red,   Red]
    // [  Red,   Red]
    // [Black, Black]
    // [Black, Black]

    // clang-format off
    GLubyte dataY[]  = {
        81, 81,
        81, 81,
        16, 16,
        16, 16,
    };
    GLubyte dataCb[] = {
        90,
        128,
    };
    GLubyte dataCr[] = {
        240,
        128,
    };
    // clang-format on

    // Create the Image
    AHardwareBuffer *ahbSource;
    EGLImageKHR ahbImage;
    createEGLImageAndroidHardwareBufferSource(
        2, 4, 1, AHARDWAREBUFFER_FORMAT_YV12, kDefaultAHBYUVUsage, kDefaultAttribs,
        {{dataY, 1}, {dataCb, 1}, {dataCr, 1}}, &ahbSource, &ahbImage);

    ASSERT_GL_NO_ERROR();

    // Create a texture target to bind the egl image
    GLTexture ahbTexture;
    createEGLImageTargetTextureExternal(ahbImage, ahbTexture);

    // Configure linear filtering
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, ahbTexture);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Draw fullscreen sampling from ahbTexture.
    glUseProgram(mTextureExternalProgram);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, ahbTexture);
    glUniform1i(mTextureExternalUniformLocation, 0);
    drawQuad(mTextureExternalProgram, "position", 0.5f);

    // Framebuffer needs to be bigger than the AHB so there is an area in between that will result
    // in half-red.
    const int windowHeight = getWindowHeight();
    ASSERT_GE(windowHeight, 8);

    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor::black, 1);
    EXPECT_PIXEL_COLOR_NEAR(0, windowHeight - 1, GLColor::red, 1);

    // Approximately half-red:
    EXPECT_PIXEL_COLOR_NEAR(0, windowHeight / 2, GLColor(127, 0, 0, 255), 15.0);

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), ahbImage);
    destroyAndroidHardwareBuffer(ahbSource);
}

// Test rendering to a YUV AHB using EXT_yuv_target
TEST_P(ImageTestES3, RenderToYUVAHB)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt() || !hasYUVTargetExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    // 3 planes of data, initialize to all zeroes
    GLubyte dataY[4]  = {0, 0, 0, 0};
    GLubyte dataCb[1] = {
        0,
    };
    GLubyte dataCr[1] = {
        0,
    };

    // Create the Image
    AHardwareBuffer *source;
    EGLImageKHR image;
    createEGLImageAndroidHardwareBufferSource(
        2, 2, 1, AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420, kDefaultAHBUsage, kDefaultAttribs,
        {{dataY, 1}, {dataCb, 1}, {dataCr, 1}}, &source, &image);

    // Create a texture target to bind the egl image
    GLTexture target;
    createEGLImageTargetTextureExternal(image, target);

    // Set up a framebuffer to render into the AHB
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_EXTERNAL_OES, target,
                           0);
    ASSERT_GL_NO_ERROR();
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLubyte drawColor[4] = {197, 128, 192, 255};

    glUseProgram(mRenderYUVProgram);
    glUniform4f(mRenderYUVUniformLocation, drawColor[0] / 255.0f, drawColor[1] / 255.0f,
                drawColor[2] / 255.0f, drawColor[3] / 255.0f);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
    drawQuad(mRenderYUVProgram, "position", 0.0f);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    drawQuad(mRenderYUVProgram, "position", 0.0f);
    ASSERT_GL_NO_ERROR();

    // ReadPixels returns the RGB converted color
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(255, 159, 212, 255), 1.0);

    // Finish before reading back AHB data
    glFinish();

    GLubyte expectedDataY[4]  = {drawColor[0], drawColor[0], drawColor[0], drawColor[0]};
    GLubyte expectedDataCb[1] = {
        drawColor[1],
    };
    GLubyte expectedDataCr[1] = {
        drawColor[2],
    };
    verifyResultAHB(source, {{expectedDataY, 1}, {expectedDataCb, 1}, {expectedDataCr, 1}});

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
    destroyAndroidHardwareBuffer(source);
}

// Test clearing to a YUV AHB using EXT_yuv_target
TEST_P(ImageTestES3, ClearYUVAHB)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt() || !hasYUVTargetExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());

    // Create the Image without data so we don't need ANGLE_AHARDWARE_BUFFER_LOCK_PLANES_SUPPORT
    AHardwareBuffer *source;
    EGLImageKHR image;
    createEGLImageAndroidHardwareBufferSource(2, 2, 1, AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420,
                                              kDefaultAHBUsage, kDefaultAttribs, {}, &source,
                                              &image);

    // Create a texture target to bind the egl image
    GLTexture target;
    createEGLImageTargetTextureExternal(image, target);

    // Set up a framebuffer to render into the AHB
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_EXTERNAL_OES, target,
                           0);
    ASSERT_GL_NO_ERROR();
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clearing a YUV framebuffer reinterprets the rgba clear color as YUV values and writes them
    // directly to the buffer
    GLubyte clearColor[4] = {197, 128, 192, 255};
    glClearColor(clearColor[0] / 255.0f, clearColor[1] / 255.0f, clearColor[2] / 255.0f,
                 clearColor[3] / 255.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    // ReadPixels returns the RGB converted color
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(255, 159, 212, 255), 1.0);

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
    destroyAndroidHardwareBuffer(source);
}

// Test glClear on FBO with AHB attachment is applied to the AHB image before we read back
TEST_P(ImageTestES3, AHBClearAppliedBeforeReadBack)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    const GLubyte kRed[]   = {255, 0, 0, 255};
    const GLubyte kBlack[] = {0, 0, 0, 0};

    // Create one image backed by the AHB.
    AHardwareBuffer *ahb;
    EGLImageKHR ahbImage;
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                              kDefaultAHBUsage, kDefaultAttribs, {{kBlack, 4}},
                                              &ahb, &ahbImage);
    GLTexture ahbTexture;
    createEGLImageTargetTexture2D(ahbImage, ahbTexture);

    // Create one framebuffer backed by the AHB.
    {
        GLFramebuffer ahbFbo;
        glBindFramebuffer(GL_FRAMEBUFFER, ahbFbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ahbTexture, 0);
        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        // Clear to red
        glClearColor(1, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        glFinish();
    }

    verifyResultAHB(ahb, {{kRed, 4}});

    eglDestroyImageKHR(window->getDisplay(), ahbImage);
    destroyAndroidHardwareBuffer(ahb);
}

// Similar to AHBClearAppliedBeforeReadBack, but clear is applied twice.
TEST_P(ImageTestES3, AHBTwiceClearAppliedBeforeReadBack)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    const GLubyte kRed[]   = {255, 0, 0, 255};
    const GLubyte kBlack[] = {0, 0, 0, 0};

    // Create one image backed by the AHB.
    AHardwareBuffer *ahb;
    EGLImageKHR ahbImage;
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                              kDefaultAHBUsage, kDefaultAttribs, {{kBlack, 4}},
                                              &ahb, &ahbImage);
    GLTexture ahbTexture;
    createEGLImageTargetTexture2D(ahbImage, ahbTexture);

    // Create one framebuffer backed by the AHB.
    {
        GLFramebuffer ahbFbo;
        glBindFramebuffer(GL_FRAMEBUFFER, ahbFbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ahbTexture, 0);
        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        // Clear to green, then to red
        glClearColor(0, 1, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(1, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        glFinish();
    }

    verifyResultAHB(ahb, {{kRed, 4}});

    eglDestroyImageKHR(window->getDisplay(), ahbImage);
    destroyAndroidHardwareBuffer(ahb);
}

// Test that glClear on FBO with AHB attachment is applied to the AHB image before detaching the AHB
// image from FBO
TEST_P(ImageTestES3, AHBClearAndDetachBeforeReadback)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    const GLubyte kRed[]   = {255, 0, 0, 255};
    const GLubyte kBlack[] = {0, 0, 0, 0};

    // Create one image backed by the AHB.
    AHardwareBuffer *ahb;
    EGLImageKHR ahbImage;
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                              kDefaultAHBUsage, kDefaultAttribs, {{kBlack, 4}},
                                              &ahb, &ahbImage);
    GLTexture ahbTexture;
    createEGLImageTargetTexture2D(ahbImage, ahbTexture);

    // Create one framebuffer backed by the AHB.
    {
        GLFramebuffer ahbFbo;
        glBindFramebuffer(GL_FRAMEBUFFER, ahbFbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ahbTexture, 0);
        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        // Clear to red
        glClearColor(1, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        // Detach the AHB image from the FBO color attachment
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
        glFinish();
    }

    verifyResultAHB(ahb, {{kRed, 4}});

    eglDestroyImageKHR(window->getDisplay(), ahbImage);
    destroyAndroidHardwareBuffer(ahb);
}

// Test that glClear on FBO with AHB color attachment is applied to the AHB image before implicity
// unbinding the AHB image from FBO
TEST_P(ImageTestES3, AHBClearAndAttachAnotherTextureBeforeReadback)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    const GLubyte kRed[]   = {255, 0, 0, 255};
    const GLubyte kBlack[] = {0, 0, 0, 0};

    // Create one image backed by the AHB.
    AHardwareBuffer *ahb;
    EGLImageKHR ahbImage;
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                              kDefaultAHBUsage, kDefaultAttribs, {{kBlack, 4}},
                                              &ahb, &ahbImage);
    GLTexture ahbTexture;
    createEGLImageTargetTexture2D(ahbImage, ahbTexture);

    // Create one framebuffer backed by the AHB.
    {
        GLFramebuffer ahbFbo;
        glBindFramebuffer(GL_FRAMEBUFFER, ahbFbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ahbTexture, 0);
        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        // Clear to red
        glClearColor(1, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        // Attach a random texture to the same FBO color attachment slot that AHB image was attached
        // to, this should implicity detach the AHB image from the FBO.
        GLTexture newTexture;
        glBindTexture(GL_TEXTURE_2D, newTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, newTexture, 0);
        glFinish();
    }

    verifyResultAHB(ahb, {{kRed, 4}});

    eglDestroyImageKHR(window->getDisplay(), ahbImage);
    destroyAndroidHardwareBuffer(ahb);
}

// Test glClear to FBO with AHB color attachment is applied to the AHB image before we switch back
// to the default FBO
TEST_P(ImageTestES3, AHBClearAndSwitchToDefaultFBOBeforeReadBack)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    const GLubyte kRed[]   = {255, 0, 0, 255};
    const GLubyte kBlack[] = {0, 0, 0, 0};

    // Create one image backed by the AHB.
    AHardwareBuffer *ahb;
    EGLImageKHR ahbImage;
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                              kDefaultAHBUsage, kDefaultAttribs, {{kBlack, 4}},
                                              &ahb, &ahbImage);
    GLTexture ahbTexture;
    createEGLImageTargetTexture2D(ahbImage, ahbTexture);

    // Create one framebuffer backed by the AHB.
    {
        GLFramebuffer ahbFbo;
        glBindFramebuffer(GL_FRAMEBUFFER, ahbFbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ahbTexture, 0);
        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        // Clear to red
        glClearColor(1, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        // Switch to default FBO
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glFinish();
    }

    verifyResultAHB(ahb, {{kRed, 4}});

    eglDestroyImageKHR(window->getDisplay(), ahbImage);
    destroyAndroidHardwareBuffer(ahb);
}

// Test glClear on FBO with AHB color attachment is applied to the AHB image with glClientWaitSync
TEST_P(ImageTestES3, AHBClearWithGLClientWaitSyncBeforeReadBack)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    const GLubyte kRed[]   = {255, 0, 0, 255};
    const GLubyte kBlack[] = {0, 0, 0, 0};

    // Create one image backed by the AHB.
    AHardwareBuffer *ahb;
    EGLImageKHR ahbImage;
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                              kDefaultAHBUsage, kDefaultAttribs, {{kBlack, 4}},
                                              &ahb, &ahbImage);
    GLTexture ahbTexture;
    createEGLImageTargetTexture2D(ahbImage, ahbTexture);

    // Create one framebuffer backed by the AHB.
    {
        GLFramebuffer ahbFbo;
        glBindFramebuffer(GL_FRAMEBUFFER, ahbFbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ahbTexture, 0);
        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        // Clear to red
        glClearColor(1, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        // Create a GLSync object and immediately wait on it
        GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
    }

    verifyResultAHB(ahb, {{kRed, 4}});

    eglDestroyImageKHR(window->getDisplay(), ahbImage);
    destroyAndroidHardwareBuffer(ahb);
}

// Test glDraw + glFlush on FBO with AHB attachment are applied to the AHB image before we read back
TEST_P(ImageTestES3, AHBDrawFlushAppliedBeforeReadBack)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport() || !kHasAHBFrontBufferUsageSupport);

    // Create a GLTexture backed by the AHB.
    AHardwareBuffer *ahb;
    EGLImageKHR ahbImage;
    const GLubyte kBlack[] = {0, 0, 0, 0};
    const GLubyte kRed[]   = {255, 0, 0, 255};
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                              kDefaultAHBUsage | kAHBUsageFrontBuffer,
                                              kDefaultAttribs, {{kBlack, 4}}, &ahb, &ahbImage);
    GLTexture ahbTexture;
    createEGLImageTargetTexture2D(ahbImage, ahbTexture);

    // Create one framebuffer backed by the AHB-based GLTexture
    GLFramebuffer ahbFbo;
    glBindFramebuffer(GL_FRAMEBUFFER, ahbFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ahbTexture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Draw to the FBO and call glFlush()
    ANGLE_GL_PROGRAM(drawColor, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(drawColor);
    GLint colorUniformLocation =
        glGetUniformLocation(drawColor, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    glUniform4f(colorUniformLocation, kRed[0] / 255.0f, kRed[1] / 255.0f, kRed[2] / 255.0f,
                kRed[3] / 255.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.5f);
    glFlush();
    // unlike glFinish(), glFlush() does not wait for commands execution to complete.
    // sleep for 1 second before reading back from AHB.
    angle::Sleep(1000);

    // Verify the result
    verifyResultAHB(ahb, {{kRed, 4}});

    eglDestroyImageKHR(window->getDisplay(), ahbImage);
    destroyAndroidHardwareBuffer(ahb);
}

// Test that glDraw + glFlush on FBO with AHB attachment are applied to the AHB
// image before detaching the AHB image from FBO
TEST_P(ImageTestES3, AHBDrawFlushAndDetachBeforeReadBack)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport() || !kHasAHBFrontBufferUsageSupport);

    // Create a GLTexture backed by the AHB.
    AHardwareBuffer *ahb;
    EGLImageKHR ahbImage;
    const GLubyte kBlack[] = {0, 0, 0, 0};
    const GLubyte kRed[]   = {255, 0, 0, 255};
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                              kDefaultAHBUsage | kAHBUsageFrontBuffer,
                                              kDefaultAttribs, {{kBlack, 4}}, &ahb, &ahbImage);
    GLTexture ahbTexture;
    createEGLImageTargetTexture2D(ahbImage, ahbTexture);

    // Create one framebuffer backed by the AHB-based GLTexture
    GLFramebuffer ahbFbo;
    glBindFramebuffer(GL_FRAMEBUFFER, ahbFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ahbTexture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Draw to the FBO and call glFlush()
    ANGLE_GL_PROGRAM(drawColor, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(drawColor);
    GLint colorUniformLocation =
        glGetUniformLocation(drawColor, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    glUniform4f(colorUniformLocation, kRed[0] / 255.0f, kRed[1] / 255.0f, kRed[2] / 255.0f,
                kRed[3] / 255.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.5f);

    glFlush();
    // unlike glFinish(), glFlush() does not wait for commands execution to complete.
    // sleep for 1 second before reading back from AHB.
    angle::Sleep(1000);

    // Detach the AHB image from the FBO color attachment
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);

    // Verify the result
    verifyResultAHB(ahb, {{kRed, 4}});

    eglDestroyImageKHR(window->getDisplay(), ahbImage);
    destroyAndroidHardwareBuffer(ahb);
}

// Test that glDraw + glFlush on FBO with AHB attachment are applied to the AHB
// image before implicitly unbinding the AHB image from FBO
TEST_P(ImageTestES3, AHBDrawFlushAndAttachAnotherTextureBeforeReadBack)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport() || !kHasAHBFrontBufferUsageSupport);

    // Create a GLTexture backed by the AHB.
    AHardwareBuffer *ahb;
    EGLImageKHR ahbImage;
    const GLubyte kBlack[] = {0, 0, 0, 0};
    const GLubyte kRed[]   = {255, 0, 0, 255};
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                              kDefaultAHBUsage | kAHBUsageFrontBuffer,
                                              kDefaultAttribs, {{kBlack, 4}}, &ahb, &ahbImage);
    GLTexture ahbTexture;
    createEGLImageTargetTexture2D(ahbImage, ahbTexture);

    // Create one framebuffer backed by the AHB-based GLTexture
    GLFramebuffer ahbFbo;
    glBindFramebuffer(GL_FRAMEBUFFER, ahbFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ahbTexture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Draw to the FBO and call glFlush()
    ANGLE_GL_PROGRAM(drawColor, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(drawColor);
    GLint colorUniformLocation =
        glGetUniformLocation(drawColor, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    glUniform4f(colorUniformLocation, kRed[0] / 255.0f, kRed[1] / 255.0f, kRed[2] / 255.0f,
                kRed[3] / 255.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.5f);

    glFlush();
    // unlike glFinish(), glFlush() does not wait for commands execution to complete.
    // sleep for 1 second before reading back from AHB.
    angle::Sleep(1000);

    // Attach a random texture to the same FBO color attachment slot that AHB image was attached
    // to, this should implicity detach the AHB image from the FBO.
    GLTexture newTexture;
    glBindTexture(GL_TEXTURE_2D, newTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, newTexture, 0);

    // Verify the result
    verifyResultAHB(ahb, {{kRed, 4}});

    eglDestroyImageKHR(window->getDisplay(), ahbImage);
    destroyAndroidHardwareBuffer(ahb);
}

// Test that glDraw + glFlush on FBO with AHB attachment are applied to the AHB
// image before switching to the default FBO
TEST_P(ImageTestES3, AHBDrawFlushAndSwitchToDefaultFBOBeforeReadBack)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport() || !kHasAHBFrontBufferUsageSupport);

    // Create a GLTexture backed by the AHB.
    AHardwareBuffer *ahb;
    EGLImageKHR ahbImage;
    const GLubyte kBlack[] = {0, 0, 0, 0};
    const GLubyte kRed[]   = {255, 0, 0, 255};
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                              kDefaultAHBUsage | kAHBUsageFrontBuffer,
                                              kDefaultAttribs, {{kBlack, 4}}, &ahb, &ahbImage);
    GLTexture ahbTexture;
    createEGLImageTargetTexture2D(ahbImage, ahbTexture);

    // Create one framebuffer backed by the AHB-based GLTexture
    GLFramebuffer ahbFbo;
    glBindFramebuffer(GL_FRAMEBUFFER, ahbFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ahbTexture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Draw to the FBO and call glFlush()
    ANGLE_GL_PROGRAM(drawColor, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(drawColor);
    GLint colorUniformLocation =
        glGetUniformLocation(drawColor, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    glUniform4f(colorUniformLocation, kRed[0] / 255.0f, kRed[1] / 255.0f, kRed[2] / 255.0f,
                kRed[3] / 255.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.5f);

    glFlush();
    // unlike glFinish(), glFlush() does not wait for commands execution to complete.
    // sleep for 1 second before reading back from AHB.
    angle::Sleep(1000);

    // Switch to default FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Verify the result
    verifyResultAHB(ahb, {{kRed, 4}});

    eglDestroyImageKHR(window->getDisplay(), ahbImage);
    destroyAndroidHardwareBuffer(ahb);
}

// Test that texture swizzle parameters work with EGL image-backed textures
TEST_P(ImageTestES3, AHBTextureSwizzleParameters)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    // Create the Image
    AHardwareBuffer *ahb;
    EGLImageKHR ahbImage;
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM,
                                              kDefaultAHBUsage, kDefaultAttribs,
                                              {{kLinearColor, 4}}, &ahb, &ahbImage);

    GLTexture ahbTexture;
    createEGLImageTargetTexture2D(ahbImage, ahbTexture);

    glBindTexture(GL_TEXTURE_2D, ahbTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_GREEN);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_BLUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);

    GLubyte expectedColor[] = {kLinearColor[1], kLinearColor[2], kLinearColor[0], kLinearColor[3]};
    verifyResults2D(ahbTexture, expectedColor);

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), ahbImage);
    destroyAndroidHardwareBuffer(ahb);
}

// Test that RGBX data are preserved when importing from AHB.  Regression test for a bug in the
// Vulkan backend where the image was cleared due to format emulation.
TEST_P(ImageTestES3, RGBXAHBImportPreservesData)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    // Create the Image
    AHardwareBuffer *ahb;
    EGLImageKHR ahbImage;
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM,
                                              kDefaultAHBUsage, kDefaultAttribs,
                                              {{kLinearColor, 4}}, &ahb, &ahbImage);

    GLTexture ahbTexture;
    createEGLImageTargetTexture2D(ahbImage, ahbTexture);

    verifyResults2D(ahbTexture, kLinearColor);
    verifyResultAHB(ahb, {{kLinearColor, 4}});

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), ahbImage);
    destroyAndroidHardwareBuffer(ahb);
}

// Test that RGBX data are preserved when importing from AHB created with sRGB color space.
TEST_P(ImageTestES3, RGBXAHBImportPreservesData_Colorspace)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    const GLubyte kRed50SRGB[]   = {188, 0, 0, 255};
    const GLubyte kRed50Linear[] = {128, 0, 0, 255};

    // Create the Image
    AHardwareBuffer *ahb;
    EGLImageKHR ahbImage;
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM,
                                              kDefaultAHBUsage, kColorspaceAttribs,
                                              {{kRed50SRGB, 4}}, &ahb, &ahbImage);

    GLTexture ahbTexture;
    createEGLImageTargetTexture2D(ahbImage, ahbTexture);

    verifyResults2D(ahbTexture, kRed50Linear);
    verifyResultAHB(ahb, {{kRed50SRGB, 4}});

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), ahbImage);
    destroyAndroidHardwareBuffer(ahb);
}

// Tests that RGBX can be successfully loaded with 3-channel data and read back as 4-channel data.
TEST_P(ImageTestES3, RGBXAHBUploadDownload)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport() || !hasRGBXInternalFormatExt());

    const size_t kWidth  = 32;
    const size_t kHeight = 32;

    const GLubyte kBlack[] = {0, 0, 0, 255};
    const GLubyte kCyan[]  = {0, 255, 255};

    std::vector<GLubyte> pixelsRGBABlack;
    for (size_t h = 0; h < kHeight; h++)
    {
        for (size_t w = 0; w < kWidth; w++)
        {
            pixelsRGBABlack.push_back(kBlack[0]);
            pixelsRGBABlack.push_back(kBlack[1]);
            pixelsRGBABlack.push_back(kBlack[2]);
            pixelsRGBABlack.push_back(kBlack[3]);
        }
    }

    std::vector<GLubyte> pixelsRGBCyan;
    for (size_t h = 0; h < kHeight; h++)
    {
        for (size_t w = 0; w < kWidth; w++)
        {
            pixelsRGBCyan.push_back(kCyan[0]);
            pixelsRGBCyan.push_back(kCyan[1]);
            pixelsRGBCyan.push_back(kCyan[2]);
        }
    }

    // Create the Image
    AHardwareBuffer *ahb;
    EGLImageKHR ahbImage;
    createEGLImageAndroidHardwareBufferSource(
        kWidth, kHeight, 1, AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM, kDefaultAHBUsage,
        kDefaultAttribs, {{pixelsRGBABlack.data(), 4}}, &ahb, &ahbImage);

    GLTexture ahbTexture;
    createEGLImageTargetTexture2D(ahbImage, ahbTexture);

    verifyResults2D(ahbTexture, kBlack);
    verifyResultAHB(ahb, {{pixelsRGBABlack.data(), 4}});

    glBindTexture(GL_TEXTURE_2D, ahbTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth, kHeight, GL_RGB, GL_UNSIGNED_BYTE,
                    pixelsRGBCyan.data());
    ASSERT_GL_NO_ERROR();

    GLFramebuffer ahbFramebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, ahbFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ahbTexture, 0);
    ASSERT_GL_NO_ERROR();
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    std::vector<GLubyte> readback;
    readback.resize(kWidth * kHeight * 4);
    glReadPixels(0, 0, kWidth, kHeight, GL_RGBX8_ANGLE, GL_UNSIGNED_BYTE, readback.data());

    for (size_t y = 0; y < kHeight; y++)
    {
        const GLubyte *actualRowData = readback.data() + (y * kWidth * 4);
        for (size_t x = 0; x < kWidth; x++)
        {
            const GLubyte *actualPixelData = actualRowData + (x * 4);
            EXPECT_EQ(actualPixelData[0], kCyan[0]) << "at (" << x << ", " << y << ")";
            EXPECT_EQ(actualPixelData[1], kCyan[1]) << "at (" << x << ", " << y << ")";
            EXPECT_EQ(actualPixelData[2], kCyan[2]) << "at (" << x << ", " << y << ")";
        }
    }

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), ahbImage);
    destroyAndroidHardwareBuffer(ahb);
}

// Test that RGBA data are preserved when importing from AHB and glTexSubImage is able to update
// data.
TEST_P(ImageTestES3, RGBAAHBUploadData)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    const GLubyte kGarbage[]     = {123, 123, 123, 123};
    const GLubyte kRed50Linear[] = {128, 0, 0, 127};

    // Create the Image
    AHardwareBuffer *ahb;
    EGLImageKHR ahbImage;
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                              kDefaultAHBUsage, kDefaultAttribs, {{kGarbage, 4}},
                                              &ahb, &ahbImage);

    GLTexture ahbTexture;
    createEGLImageTargetTexture2D(ahbImage, ahbTexture);

    glBindTexture(GL_TEXTURE_2D, ahbTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, kRed50Linear);
    glFinish();

    verifyResults2D(ahbTexture, kRed50Linear);
    verifyResultAHB(ahb, {{kRed50Linear, 4}});

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), ahbImage);
    destroyAndroidHardwareBuffer(ahb);
}

// Test that RGBA data are preserved when importing from AHB with sRGB color space and glTexSubImage
// is able to update data.
TEST_P(ImageTestES3, RGBAAHBUploadDataColorspace)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    const GLubyte kGarbage[]     = {123, 123, 123, 123};
    const GLubyte kRed50SRGB[]   = {188, 0, 0, 128};
    const GLubyte kRed50Linear[] = {128, 0, 0, 127};

    // Create the Image
    AHardwareBuffer *ahb;
    EGLImageKHR ahbImage;
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                              kDefaultAHBUsage, kColorspaceAttribs, {{kGarbage, 4}},
                                              &ahb, &ahbImage);

    GLTexture ahbTexture;
    createEGLImageTargetTexture2D(ahbImage, ahbTexture);

    glBindTexture(GL_TEXTURE_2D, ahbTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, kRed50SRGB);
    glFinish();

    verifyResults2D(ahbTexture, kRed50Linear);
    verifyResultAHB(ahb, {{kRed50SRGB, 4}});

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), ahbImage);
    destroyAndroidHardwareBuffer(ahb);
}

// Test that RGBX data are preserved when importing from AHB and glTexSubImage is able to update
// data.
TEST_P(ImageTestES3, RGBXAHBUploadData)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    const GLubyte kGarbage[]     = {123, 123, 123, 123};
    const GLubyte kRed50Linear[] = {128, 0, 0, 255};

    // Create the Image
    AHardwareBuffer *ahb;
    EGLImageKHR ahbImage;
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM,
                                              kDefaultAHBUsage, kDefaultAttribs, {{kGarbage, 4}},
                                              &ahb, &ahbImage);

    GLTexture ahbTexture;
    createEGLImageTargetTexture2D(ahbImage, ahbTexture);

    glBindTexture(GL_TEXTURE_2D, ahbTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, kRed50Linear);
    glFinish();

    verifyResults2D(ahbTexture, kRed50Linear);
    verifyResultAHB(ahb, {{kRed50Linear, 4}});

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), ahbImage);
    destroyAndroidHardwareBuffer(ahb);
}

// Test that RGBX data are preserved when importing from AHB created with sRGB color space and
// glTexSubImage is able to update data.
TEST_P(ImageTestES3, RGBXAHBUploadDataColorspace)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    const GLubyte kGarbage[]     = {123, 123, 123, 123};
    const GLubyte kRed50SRGB[]   = {188, 0, 0, 255};
    const GLubyte kRed50Linear[] = {128, 0, 0, 255};

    // Create the Image
    AHardwareBuffer *ahb;
    EGLImageKHR ahbImage;
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM,
                                              kDefaultAHBUsage, kColorspaceAttribs,
                                              {{kGarbage, sizeof(kGarbage)}}, &ahb, &ahbImage);

    GLTexture ahbTexture;
    createEGLImageTargetTexture2D(ahbImage, ahbTexture);

    glBindTexture(GL_TEXTURE_2D, ahbTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, kRed50SRGB);
    glFinish();

    verifyResults2D(ahbTexture, kRed50Linear);
    verifyResultAHB(ahb, {{kRed50SRGB, sizeof(kRed50SRGB)}});

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), ahbImage);
    destroyAndroidHardwareBuffer(ahb);
}

// Test that RGB data are preserved when importing from AHB created with sRGB color space and
// glTexSubImage is able to update data.
TEST_P(ImageTestES3, RGBAHBUploadDataColorspace)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());
    ANGLE_SKIP_TEST_IF(!isAndroidHardwareBufferConfigurationSupported(
        1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM, kDefaultAHBUsage));

    const GLubyte kGarbage[]     = {123, 123, 123};
    const GLubyte kRed50SRGB[]   = {188, 0, 0};
    const GLubyte kRed50Linear[] = {128, 0, 0, 255};

    // Create the Image
    AHardwareBuffer *ahb;
    EGLImageKHR ahbImage;
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM,
                                              kDefaultAHBUsage, kColorspaceAttribs,
                                              {{kGarbage, sizeof(kGarbage)}}, &ahb, &ahbImage);

    GLTexture ahbTexture;
    createEGLImageTargetTexture2D(ahbImage, ahbTexture);

    glBindTexture(GL_TEXTURE_2D, ahbTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, kRed50SRGB);
    glFinish();

    verifyResults2D(ahbTexture, kRed50Linear);
    verifyResultAHB(ahb, {{kRed50SRGB, sizeof(kRed50SRGB)}});

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), ahbImage);
    destroyAndroidHardwareBuffer(ahb);
}

// Test that RGBX data are preserved when importing from AHB.  Tests interaction of emulated channel
// being cleared with no GPU_FRAMEBUFFER usage specified.
TEST_P(ImageTestES3, RGBXAHBImportNoFramebufferUsage)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    // Create the Image
    AHardwareBuffer *ahb;
    EGLImageKHR ahbImage;
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM,
                                              kAHBUsageGPUSampledImage, kDefaultAttribs,
                                              {{kLinearColor, 4}}, &ahb, &ahbImage);

    GLTexture ahbTexture;
    createEGLImageTargetTexture2D(ahbImage, ahbTexture);

    verifyResults2D(ahbTexture, kLinearColor);
    verifyResultAHB(ahb, {{kLinearColor, 4}});

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), ahbImage);
    destroyAndroidHardwareBuffer(ahb);
}

// Test that RGBX data are preserved when importing from AHB.  Tests interaction of emulated channel
// being cleared with multiple layers.
TEST_P(ImageTestES3, RGBXAHBImportMultipleLayers)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    // Limit the test to single layer for now. writeAHBData is assuming alignment between layers
    // being 4096 which may not true on some GPUs. There is no API to retrieve such alignment from
    // driver. For now just limit to single layer so that we can still test single layer behavior
    // here.
    constexpr size_t kLayerCount = 1;

    ANGLE_SKIP_TEST_IF(!isAndroidHardwareBufferConfigurationSupported(
        1, 1, kLayerCount, AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM, kDefaultAHBUsage));

    const GLubyte kInitColor[] = {132, 55, 219, 12, 77, 23, 190, 101, 231, 44, 143, 99};

    // Create the Image
    AHardwareBuffer *ahb;
    EGLImageKHR ahbImage;
    createEGLImageAndroidHardwareBufferSource(
        1, 1, kLayerCount, AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM, kDefaultAHBUsage, kDefaultAttribs,
        {{kInitColor, 4}}, &ahb, &ahbImage);

    GLTexture ahbTexture;
    createEGLImageTargetTexture2DArray(ahbImage, ahbTexture);

    // RGBX doesn't have alpha, so readback should return 255.
    const GLubyte kExpectedColor[] = {
        kInitColor[0], kInitColor[1], kInitColor[2], 255,           kInitColor[4],  kInitColor[5],
        kInitColor[6], 255,           kInitColor[8], kInitColor[9], kInitColor[10], 255,
    };
    for (uint32_t layerIndex = 0; layerIndex < kLayerCount; ++layerIndex)
    {
        verifyResults2DArray(ahbTexture, kExpectedColor + 4 * layerIndex, layerIndex);
    }
    verifyResultAHB(ahb, {{kExpectedColor, 4}});

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), ahbImage);
    destroyAndroidHardwareBuffer(ahb);
}

// Test that RGBX data are preserved when importing from AHB.  Tests interaction of emulated channel
// being cleared with glReadPixels.
TEST_P(ImageTestES3, RGBXAHBImportThenReadPixels)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    // Create the Image
    AHardwareBuffer *ahb;
    EGLImageKHR ahbImage;
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM,
                                              kDefaultAHBUsage, kDefaultAttribs,
                                              {{kLinearColor, 4}}, &ahb, &ahbImage);

    GLTexture ahbTexture;
    createEGLImageTargetTexture2D(ahbImage, ahbTexture);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ahbTexture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // RGBX doesn't have alpha, so readback should return 255.  kLinearColor[3] is already 255.
    EXPECT_PIXEL_NEAR(0, 0, kLinearColor[0], kLinearColor[1], kLinearColor[2], kLinearColor[3], 1);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    verifyResults2D(ahbTexture, kLinearColor);
    verifyResultAHB(ahb, {{kLinearColor, 4}});

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), ahbImage);
    destroyAndroidHardwareBuffer(ahb);
}

// Test that RGBX data are preserved when importing from AHB.  Tests interaction of emulated channel
// being cleared with a following clear.
TEST_P(ImageTestES3, RGBXAHBImportThenClear)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    // Create the Image
    AHardwareBuffer *ahb;
    EGLImageKHR ahbImage;
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM,
                                              kDefaultAHBUsage, kDefaultAttribs,
                                              {{kLinearColor, 4}}, &ahb, &ahbImage);

    GLTexture ahbTexture;
    createEGLImageTargetTexture2D(ahbImage, ahbTexture);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ahbTexture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear
    const GLubyte kClearColor[] = {63, 127, 191, 55};
    glClearColor(kClearColor[0] / 255.0f, kClearColor[1] / 255.0f, kClearColor[2] / 255.0f,
                 kClearColor[3] / 255.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // RGBX doesn't have alpha, so readback should return 255.
    const GLubyte kExpectedColor[] = {kClearColor[0], kClearColor[1], kClearColor[2], 255};
    verifyResults2D(ahbTexture, kExpectedColor);
    verifyResultAHB(ahb, {{kExpectedColor, 4}});

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), ahbImage);
    destroyAndroidHardwareBuffer(ahb);
}

// Test that RGBX data are preserved when importing from AHB.  Tests interaction of emulated channel
// being cleared with a following clear and a draw call.
TEST_P(ImageTestES3, RGBXAHBImportThenClearThenDraw)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    // Create the Image
    AHardwareBuffer *ahb;
    EGLImageKHR ahbImage;
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM,
                                              kDefaultAHBUsage, kDefaultAttribs,
                                              {{kLinearColor, 4}}, &ahb, &ahbImage);

    GLTexture ahbTexture;
    createEGLImageTargetTexture2D(ahbImage, ahbTexture);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ahbTexture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear
    const GLubyte kClearColor[] = {63, 127, 191, 55};
    glClearColor(kClearColor[0] / 255.0f, kClearColor[1] / 255.0f, kClearColor[2] / 255.0f,
                 kClearColor[3] / 255.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw with blend
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    ANGLE_GL_PROGRAM(drawColor, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(drawColor);
    GLint colorUniformLocation =
        glGetUniformLocation(drawColor, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    glUniform4f(colorUniformLocation, 0.25f, 0.25f, 0.25f, 0.25f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.5f);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_BLEND);

    // RGBX doesn't have alpha, so readback should return 255.
    const GLubyte kExpectedColor[] = {static_cast<GLubyte>(kClearColor[0] + 64),
                                      static_cast<GLubyte>(kClearColor[1] + 64),
                                      static_cast<GLubyte>(kClearColor[2] + 64), 255};
    verifyResults2D(ahbTexture, kExpectedColor);
    verifyResultAHB(ahb, {{kExpectedColor, 4}});

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), ahbImage);
    destroyAndroidHardwareBuffer(ahb);
}

// Test that RGBX data are preserved when importing from AHB.  Tests interaction of emulated channel
// being cleared with a following data upload.
TEST_P(ImageTestES3, RGBXAHBImportThenUpload)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    const GLubyte kInitColor[] = {132, 55, 219, 12, 132, 55, 219, 12};

    // Create the Image
    AHardwareBuffer *ahb;
    EGLImageKHR ahbImage;
    createEGLImageAndroidHardwareBufferSource(2, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM,
                                              kDefaultAHBUsage, kDefaultAttribs, {{kInitColor, 4}},
                                              &ahb, &ahbImage);

    GLTexture ahbTexture;
    createEGLImageTargetTexture2D(ahbImage, ahbTexture);

    // Upload data
    const GLubyte kUploadColor[] = {63, 127, 191, 55};
    glTexSubImage2D(GL_TEXTURE_2D, 0, 1, 0, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, kUploadColor);
    ASSERT_GL_NO_ERROR();

    // RGBX doesn't have alpha, so readback should return 255.
    const GLubyte kExpectedColorRight[] = {kUploadColor[0], kUploadColor[1], kUploadColor[2], 255};
    const GLubyte kExpectedColorLeft[]  = {kInitColor[0], kInitColor[1], kInitColor[2], 255};
    verifyResults2DLeftAndRight(ahbTexture, kExpectedColorLeft, kExpectedColorRight);
    verifyResultAHB(ahb, {{kExpectedColorLeft, 4}}, AHBVerifyRegion::LeftHalf);
    verifyResultAHB(ahb, {{kExpectedColorRight, 4}}, AHBVerifyRegion::RightHalf);

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), ahbImage);
    destroyAndroidHardwareBuffer(ahb);
}

// Tests interaction of emulated channel being cleared with a following data upload and immediately
// ends to check that the image updates are processed and flushed without errors. It is similar to
// RGBXAHBImportThenUpload, but there is no pixel reading or destroying the image to avoid extra
// staged update flushes.
TEST_P(ImageTestES3, IncompleteRGBXAHBImportThenUploadThenEnd)
{
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    const GLubyte kInitColor[] = {132, 55, 219, 12, 132, 55, 219, 12};

    // Create the Image
    AHardwareBuffer *ahb;
    EGLImageKHR ahbImage;
    createEGLImageAndroidHardwareBufferSource(2, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM,
                                              kDefaultAHBUsage, kDefaultAttribs, {{kInitColor, 4}},
                                              &ahb, &ahbImage);

    GLTexture ahbTexture;
    createEGLImageTargetTexture2D(ahbImage, ahbTexture);

    // Upload data
    const GLubyte kUploadColor[] = {63, 127, 191, 55};
    glTexSubImage2D(GL_TEXTURE_2D, 0, 1, 0, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, kUploadColor);
    ASSERT_GL_NO_ERROR();

    // Clean up
    destroyAndroidHardwareBuffer(ahb);
    // This test relies on internal assertions to catch the issue regarding unflushed updates after
    // clearing emulated channels.
}

// Test that RGBX data are preserved when importing from AHB.  Tests interaction of emulated channel
// being cleared with occlusion queries.
TEST_P(ImageTestES3, RGBXAHBImportOcclusionQueryNotCounted)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    GLQueryEXT query;
    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query);

    // Create the Image
    AHardwareBuffer *ahb;
    EGLImageKHR ahbImage;
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM,
                                              kDefaultAHBUsage, kDefaultAttribs,
                                              {{kLinearColor, 4}}, &ahb, &ahbImage);

    GLTexture ahbTexture;
    createEGLImageTargetTexture2D(ahbImage, ahbTexture);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ahbTexture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Perform a masked clear.  Both the emulated clear and the masked clear should be performed,
    // neither of which should contribute to the occlusion query.
    const GLubyte kClearColor[] = {63, 127, 191, 55};
    glColorMask(GL_TRUE, GL_FALSE, GL_TRUE, GL_TRUE);
    glClearColor(kClearColor[0] / 255.0f, kClearColor[1] / 255.0f, kClearColor[2] / 255.0f,
                 kClearColor[3] / 255.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    // RGBX doesn't have alpha, so readback should return 255.
    const GLubyte kExpectedColor[] = {kClearColor[0], kLinearColor[1], kClearColor[2], 255};
    verifyResults2D(ahbTexture, kExpectedColor);
    verifyResultAHB(ahb, {{kExpectedColor, 4}});

    GLuint result = GL_TRUE;
    glGetQueryObjectuivEXT(query, GL_QUERY_RESULT_EXT, &result);
    EXPECT_GL_NO_ERROR();

    EXPECT_GL_FALSE(result);

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), ahbImage);
    destroyAndroidHardwareBuffer(ahb);
}

// Test repeatedly importing and releasing AHBs into textures to replicate behavior where
// SurfaceFlinger optimistically imports AHBs but never actually ends up using them. Regression
// test to check that AHB releases are not left pending and kept alive to avoid running out of
// memory.
TEST_P(ImageTestES3, AHBImportReleaseStress)
{
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    // Counters only available on Vulkan.
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    const GLubyte kBlack[] = {0, 0, 0, 0};

    glFinish();

    GLPerfMonitor monitor;
    glBeginPerfMonitorAMD(monitor);

    const uint64_t initialPendingSubmissionGarbageObjects =
        getPerfCounters().pendingSubmissionGarbageObjects;

    for (int i = 0; i < 20; i++)
    {
        AHardwareBuffer *ahb;
        EGLImageKHR ahbImage;
        createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                                  kDefaultAHBUsage, kDefaultAttribs, {{kBlack, 4}},
                                                  &ahb, &ahbImage);

        {
            GLTexture ahbTexture;
            glBindTexture(GL_TEXTURE_2D, ahbTexture);
            glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, ahbImage);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            ASSERT_GL_NO_ERROR();

            // Intentionally not doing anything which may explicitly flush operations on the AHB.
        }

        eglDestroyImageKHR(getEGLWindow()->getDisplay(), ahbImage);
        destroyAndroidHardwareBuffer(ahb);
    }

    glEndPerfMonitorAMD(monitor);

    EXPECT_LE(getPerfCounters().pendingSubmissionGarbageObjects,
              initialPendingSubmissionGarbageObjects + 10);
}

// Test validation of using EXT_yuv_target
TEST_P(ImageTestES3, YUVValidation)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt() || !hasYUVTargetExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());

    // Create the Image without data so we don't need ANGLE_AHARDWARE_BUFFER_LOCK_PLANES_SUPPORT
    AHardwareBuffer *yuvSource;
    EGLImageKHR yuvImage;
    createEGLImageAndroidHardwareBufferSource(2, 2, 1, AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420,
                                              kDefaultAHBUsage, kDefaultAttribs, {}, &yuvSource,
                                              &yuvImage);

    GLTexture yuvTexture;
    createEGLImageTargetTextureExternal(yuvImage, yuvTexture);

    GLFramebuffer yuvFbo;
    glBindFramebuffer(GL_FRAMEBUFFER, yuvFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_EXTERNAL_OES,
                           yuvTexture, 0);
    ASSERT_GL_NO_ERROR();
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Create an rgba image
    AHardwareBuffer *rgbaSource;
    EGLImageKHR rgbaImage;
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                              kDefaultAHBUsage, kDefaultAttribs, {}, &rgbaSource,
                                              &rgbaImage);

    GLTexture rgbaExternalTexture;
    createEGLImageTargetTextureExternal(rgbaImage, rgbaExternalTexture);

    GLFramebuffer rgbaExternalFbo;
    glBindFramebuffer(GL_FRAMEBUFFER, rgbaExternalFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_EXTERNAL_OES,
                           rgbaExternalTexture, 0);
    ASSERT_GL_NO_ERROR();
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Create a 2d rgb texture/framebuffer
    GLTexture rgb2DTexture;
    glBindTexture(GL_TEXTURE_2D, rgb2DTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer rgb2DFbo;
    glBindFramebuffer(GL_FRAMEBUFFER, rgb2DFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rgb2DTexture, 0);
    ASSERT_GL_NO_ERROR();
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // It's an error to sample from a non-yuv external texture with a yuv sampler
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(mTextureYUVProgram);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, rgbaExternalTexture);
    glUniform1i(mTextureYUVUniformLocation, 0);

    drawQuad(mTextureYUVProgram, "position", 0.5f);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // It's an error to render into a YUV framebuffer without a YUV writing program
    glBindFramebuffer(GL_FRAMEBUFFER, yuvFbo);
    glUseProgram(mTextureExternalESSL3Program);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, rgbaExternalTexture);
    glUniform1i(mTextureExternalESSL3UniformLocation, 0);

    drawQuad(mTextureExternalESSL3Program, "position", 0.5f);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // It's an error to render to a RGBA framebuffer with a YUV writing program
    glBindFramebuffer(GL_FRAMEBUFFER, rgb2DFbo);
    glUseProgram(mRenderYUVProgram);

    drawQuad(mRenderYUVProgram, "position", 0.5f);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // It's an error to set disable r, g, or b color writes when rendering to a yuv framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, yuvFbo);
    glUseProgram(mRenderYUVProgram);

    glColorMask(false, true, true, true);
    drawQuad(mRenderYUVProgram, "position", 0.5f);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glColorMask(true, false, true, true);
    drawQuad(mRenderYUVProgram, "position", 0.5f);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glColorMask(true, true, false, true);
    drawQuad(mRenderYUVProgram, "position", 0.5f);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // It's an error to enable blending when rendering to a yuv framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, yuvFbo);
    glUseProgram(mRenderYUVProgram);

    glDisable(GL_BLEND);
    drawQuad(mRenderYUVProgram, "position", 0.5f);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // It's an error to blit to/from a yuv framebuffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, yuvFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rgb2DFbo);
    glBlitFramebuffer(0, 0, 1, 1, 0, 0, 1, 1, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, rgb2DFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, yuvFbo);
    glBlitFramebuffer(0, 0, 1, 1, 0, 0, 1, 1, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // It's an error to glCopyTexImage/glCopyTexSubImage from a YUV framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, yuvFbo);
    glBindTexture(GL_TEXTURE_2D, rgb2DTexture);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, 1, 1, 0);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, 1, 1);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), yuvImage);
    destroyAndroidHardwareBuffer(yuvSource);

    eglDestroyImageKHR(window->getDisplay(), rgbaImage);
    destroyAndroidHardwareBuffer(rgbaSource);
}

// Testing source AHB EGL image with colorspace, target external ESSL3 texture
TEST_P(ImageTestES3, SourceAHBTargetExternalESSL3_Colorspace)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 && !IsGLExtensionEnabled("GL_EXT_sRGB"));
    ANGLE_SKIP_TEST_IF(!hasImageGLColorspaceExt());
    SourceAHBTargetExternalESSL3_helper(kColorspaceAttribs);
}

void ImageTest::SourceAHBTargetExternalESSL3_helper(const EGLint *attribs)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt() ||
                       !hasExternalESSL3Ext());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    // Create the Image
    AHardwareBuffer *source;
    EGLImageKHR image;
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                              kDefaultAHBUsage, attribs, {{kSrgbColor, 4}}, &source,
                                              &image);

    // Create a texture target to bind the egl image
    GLTexture target;
    createEGLImageTargetTextureExternal(image, target);

    // Use texture target bound to egl image as source and render to framebuffer
    // Verify that the target texture has the expected color
    verifyResultsExternalESSL3(target, getExpected2DColorForAttribList(attribs));

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
    destroyAndroidHardwareBuffer(source);
}

// Testing source multi-layered AHB EGL image, target 2D array texture
TEST_P(ImageTestES3, SourceAHBArrayTarget2DArray)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt() ||
                       !hasEglImageArrayExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());

    constexpr size_t kDepth = 2;

    ANGLE_SKIP_TEST_IF(!isAndroidHardwareBufferConfigurationSupported(
        1, 1, kDepth, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM, kDefaultAHBUsage));

    // Create the Image without data so we don't need ANGLE_AHARDWARE_BUFFER_LOCK_PLANES_SUPPORT
    AHardwareBuffer *source;
    EGLImageKHR image;
    createEGLImageAndroidHardwareBufferSource(1, 1, kDepth, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                              kDefaultAHBUsage, kDefaultAttribs, {}, &source,
                                              &image);

    // Create a texture target to bind the egl image
    GLTexture target;
    createEGLImageTargetTexture2DArray(image, target);

    // Upload texture data
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, 1, 1, kDepth, GL_RGBA, GL_UNSIGNED_BYTE,
                    kLinearColor3D);

    // Use texture target bound to egl image as source and render to framebuffer
    for (size_t layer = 0; layer < kDepth; layer++)
    {
        // Expect that the target texture has the same color as the source texture
        verifyResults2DArray(target, &kLinearColor3D[layer * 4], layer);
    }

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
    destroyAndroidHardwareBuffer(source);
}

// Testing source cubemap AHB EGL image, target cubemap texture
TEST_P(ImageTestES3, SourceAHBCubeTargetCube)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasEglImageStorageExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());

    ANGLE_SKIP_TEST_IF(!isAndroidHardwareBufferConfigurationSupported(
        1, 1, kCubeFaceCount, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
        kDefaultAHBUsage | kAHBUsageGPUCubeMap));

    // Create the Image without data so we don't need ANGLE_AHARDWARE_BUFFER_LOCK_PLANES_SUPPORT
    AHardwareBuffer *source;
    EGLImageKHR image;
    createEGLImageAndroidHardwareBufferSource(
        1, 1, kCubeFaceCount, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
        kDefaultAHBUsage | kAHBUsageGPUCubeMap, kDefaultAttribs, {}, &source, &image);

    // Create a texture target to bind the egl image
    GLTexture target;
    createEGLImageTargetTextureStorage(image, GL_TEXTURE_CUBE_MAP, target, nullptr);

    // Upload texture data
    for (size_t faceIdx = 0; faceIdx < kCubeFaceCount; faceIdx++)
    {
        glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIdx, 0, 0, 0, 1, 1, GL_RGBA,
                        GL_UNSIGNED_BYTE, &kLinearColorCube[faceIdx * 4]);
        ASSERT_GL_NO_ERROR();
    }

    // Use texture target bound to egl image as source and render to framebuffer
    for (size_t faceIdx = 0; faceIdx < kCubeFaceCount; faceIdx++)
    {
        // Expect that the target texture has the same color as the source texture
        verifyResultsCube(target, &kLinearColorCube[faceIdx * 4], faceIdx);
    }

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
    destroyAndroidHardwareBuffer(source);
}

// Testing source cubemap array AHB EGL image, target cubemap array texture
TEST_P(ImageTestES31, SourceAHBCubeArrayTargetCubeArray)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!(getClientMajorVersion() >= 3 && getClientMinorVersion() >= 1));
    ANGLE_SKIP_TEST_IF(!hasEglImageStorageExt() ||
                       !IsGLExtensionEnabled("GL_EXT_texture_cube_map_array"));
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());

    constexpr size_t kDepth = kCubeFaceCount * 2;
    ANGLE_SKIP_TEST_IF(!isAndroidHardwareBufferConfigurationSupported(
        1, 1, kDepth, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
        kDefaultAHBUsage | kAHBUsageGPUCubeMap));

    // Create the Image without data so we don't need ANGLE_AHARDWARE_BUFFER_LOCK_PLANES_SUPPORT
    AHardwareBuffer *source;
    EGLImageKHR image;
    createEGLImageAndroidHardwareBufferSource(1, 1, kDepth, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                              kDefaultAHBUsage | kAHBUsageGPUCubeMap,
                                              kDefaultAttribs, {}, &source, &image);

    // Create a texture target to bind the egl image
    GLTexture target;
    createEGLImageTargetTextureStorage(image, GL_TEXTURE_CUBE_MAP_ARRAY, target, nullptr);

    // Upload texture data
    for (size_t faceIdx = 0; faceIdx < kCubeFaceCount; faceIdx++)
    {
        glTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, 0, 0, faceIdx, 1, 1, 1, GL_RGBA,
                        GL_UNSIGNED_BYTE, &kLinearColorCube[faceIdx * 4]);
        glTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, 0, 0, 11 - faceIdx, 1, 1, 1, GL_RGBA,
                        GL_UNSIGNED_BYTE, &kLinearColorCube[faceIdx * 4]);
        ASSERT_GL_NO_ERROR();
    }

    // Use texture target bound to egl image as source and render to framebuffer
    for (size_t faceIdx = 0; faceIdx < kCubeFaceCount; faceIdx++)
    {
        // Expect that the target texture has the same color as the source texture
        verifyResultsCubeArray(target, &kLinearColorCube[faceIdx * 4], faceIdx, 0);
        verifyResultsCubeArray(target, &kLinearColorCube[(5 - faceIdx) * 4], faceIdx, 1);
    }

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
    destroyAndroidHardwareBuffer(source);
}

// Testing source 2D AHB with mipmap EGL image, target 2D texture with mipmap
TEST_P(ImageTestES3, SourceAHBMipTarget2DMip)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasEglImageStorageExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());

    ANGLE_SKIP_TEST_IF(!isAndroidHardwareBufferConfigurationSupported(
        2, 2, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
        kDefaultAHBUsage | kAHBUsageGPUMipMapComplete));

    // Create the Image without data so we don't need ANGLE_AHARDWARE_BUFFER_LOCK_PLANES_SUPPORT
    AHardwareBuffer *source;
    EGLImageKHR image;
    createEGLImageAndroidHardwareBufferSource(2, 2, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                              kDefaultAHBUsage | kAHBUsageGPUMipMapComplete,
                                              kDefaultAttribs, {}, &source, &image);

    // Create a texture target to bind the egl image
    GLTexture target;
    createEGLImageTargetTextureStorage(image, GL_TEXTURE_2D, target, nullptr);

    // Upload texture data
    // Set Mip level 0 to one color
    const std::vector<GLColor> kRedData(4, GLColor::red);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, kRedData.data());

    // Set Mip level 1 to a different color
    const std::vector<GLColor> kGreenData(1, GLColor::green);
    glTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, kGreenData.data());

    // Use texture target bound to egl image as source and render to framebuffer
    // Expect that the target texture has the same color as the corresponding mip level in the
    // source texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    verifyResults2D(target, GLColor::red.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);
    verifyResults2D(target, GLColor::green.data());

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
    destroyAndroidHardwareBuffer(source);
}

// Test glGenerateMipmap and GL_EXT_EGL_image_storage interaction
TEST_P(ImageTestES3, SourceAHBMipTarget2DMipGenerateMipmap)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasEglImageStorageExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());

    ANGLE_SKIP_TEST_IF(!isAndroidHardwareBufferConfigurationSupported(
        2, 2, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
        kDefaultAHBUsage | kAHBUsageGPUMipMapComplete));

    // Create the Image without data so we don't need ANGLE_AHARDWARE_BUFFER_LOCK_PLANES_SUPPORT
    AHardwareBuffer *source;
    EGLImageKHR image;
    createEGLImageAndroidHardwareBufferSource(2, 2, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                              kDefaultAHBUsage | kAHBUsageGPUMipMapComplete,
                                              kDefaultAttribs, {}, &source, &image);

    // Create a texture target to bind the egl image
    GLTexture target;
    createEGLImageTargetTextureStorage(image, GL_TEXTURE_2D, target, nullptr);

    // Upload texture data
    // Set Mip level 0 to one color
    const std::vector<GLColor> kRedData(4, GLColor::red);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, kRedData.data());

    // Set Mip level 1 to a different color
    const std::vector<GLColor> kGreenData(1, GLColor::green);
    glTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, kGreenData.data());

    // Generate mipmap level 1
    glGenerateMipmap(GL_TEXTURE_2D);

    // Use mipmap level 1 of texture target bound to egl image as source and render to framebuffer
    // Expect that the target texture has the same color as the mip level 0 in the source texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);
    verifyResults2D(target, GLColor::red.data());

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
    destroyAndroidHardwareBuffer(source);
}

// Create a depth format AHB backed EGL image and verify that the image's aspect is honored
TEST_P(ImageTest, SourceAHBTarget2DDepth)
{
    // TODO - Support for depth formats in AHB is missing (http://anglebug.com/42263405)
    ANGLE_SKIP_TEST_IF(true);

    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    GLint level             = 0;
    GLsizei width           = 1;
    GLsizei height          = 1;
    GLsizei depth           = 1;
    GLint depthStencilValue = 0;

    // Create the Image
    AHardwareBuffer *source;
    EGLImageKHR image;
    createEGLImageAndroidHardwareBufferSource(
        width, height, depth, AHARDWAREBUFFER_FORMAT_D24_UNORM, kDefaultAHBUsage, kDefaultAttribs,
        {{reinterpret_cast<GLubyte *>(&depthStencilValue), 3}}, &source, &image);

    // Create a texture target to bind the egl image
    GLTexture depthTextureTarget;
    createEGLImageTargetTexture2D(image, depthTextureTarget);

    // Create a color texture and fill it with red
    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 GLColor::red.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    EXPECT_GL_NO_ERROR();

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    EXPECT_GL_NO_ERROR();

    // Attach the color and depth texture to the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    EXPECT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTextureTarget,
                           0);
    EXPECT_GL_NO_ERROR();

    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // Clear the color texture to red
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_EQ(0, 0, 255, 0, 0, 255);

    // Enable Depth test but disable depth writes. The depth function is set to ">".
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_GREATER);

    // Fill any fragment of the color attachment with blue if it passes the depth test.
    ANGLE_GL_PROGRAM(colorFillProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
    drawQuad(colorFillProgram, essl1_shaders::PositionAttrib(), 1.0f, 1.0f);

    // Since 1.0f > 0.0f, all fragments of the color attachment should be blue.
    EXPECT_PIXEL_EQ(0, 0, 0, 0, 255, 255);

    // Clean up
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    eglDestroyImageKHR(window->getDisplay(), image);
    destroyAndroidHardwareBuffer(source);
}

TEST_P(ImageTest, Source2DTargetRenderbuffer)
{
    Source2DTargetRenderbuffer_helper(kDefaultAttribs);
}

TEST_P(ImageTest, Source2DTargetRenderbuffer_Colorspace)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);
    ANGLE_SKIP_TEST_IF(!hasImageGLColorspaceExt());
    Source2DTargetRenderbuffer_helper(kColorspaceAttribs);
}

void ImageTest::Source2DTargetRenderbuffer_helper(const EGLint *attribs)
{

    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());

    // Create sampling texture
    GLTexture sampleTexture;
    glBindTexture(GL_TEXTURE_2D, sampleTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, kLinearColor);
    // Disable mipmapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Create the attachment texture and image
    GLTexture attachmentTexture;
    EGLImageKHR image;
    createEGLImage2DTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, attribs, nullptr,
                                  attachmentTexture, &image);

    // Create the renderbuffer
    GLRenderbuffer renderbuffer;
    createEGLImageTargetRenderbuffer(image, renderbuffer);

    // Verify that the render buffer has the expected color
    verifyResultsRenderbufferWithClearAndDraw(
        sampleTexture, renderbuffer, kLinearColor,
        getExpected2DColorForAttribList(attribs, EglImageUsage::Rendering));

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
}

// Testing source native client buffer EGL image, target external texture
// where source native client buffer is created using EGL_ANDROID_create_native_client_buffer API
TEST_P(ImageTest, SourceNativeClientBufferTargetExternal)
{
    SourceNativeClientBufferTargetExternal_helper(kDefaultAttribs);
}

// Testing source native client buffer EGL image with colorspace, target external texture
// where source native client buffer is created using EGL_ANDROID_create_native_client_buffer API
TEST_P(ImageTest, SourceNativeClientBufferTargetExternal_Colorspace)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);
    ANGLE_SKIP_TEST_IF(!hasImageGLColorspaceExt());
    SourceNativeClientBufferTargetExternal_helper(kColorspaceAttribs);
}

void ImageTest::SourceNativeClientBufferTargetExternal_helper(const EGLint *attribs)
{

    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    // Create an Image backed by a native client buffer allocated using
    // EGL_ANDROID_create_native_client_buffer API
    EGLImageKHR image;
    createEGLImageANWBClientBufferSource(1, 1, 1, kNativeClientBufferAttribs_RGBA8_Texture, attribs,
                                         {{kSrgbColor, 4}}, &image);

    // Create the target
    GLTexture target;
    createEGLImageTargetTextureExternal(image, target);

    // Verify that the target texture has the expected color
    verifyResultsExternal(target, getExpected2DColorForAttribList(attribs));

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
}

// Testing source native client buffer EGL image, target Renderbuffer
// where source native client buffer is created using EGL_ANDROID_create_native_client_buffer API
TEST_P(ImageTest, SourceNativeClientBufferTargetRenderbuffer)
{
    SourceNativeClientBufferTargetRenderbuffer_helper(kDefaultAttribs);
}

// Testing source native client buffer EGL image with colorspace, target Renderbuffer
// where source native client buffer is created using EGL_ANDROID_create_native_client_buffer API
TEST_P(ImageTest, SourceNativeClientBufferTargetRenderbuffer_Colorspace)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);
    ANGLE_SKIP_TEST_IF(!hasImageGLColorspaceExt());
    SourceNativeClientBufferTargetRenderbuffer_helper(kColorspaceAttribs);
}

void ImageTest::SourceNativeClientBufferTargetRenderbuffer_helper(const EGLint *attribs)
{

    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    // Create an Image backed by a native client buffer allocated using
    // EGL_ANDROID_create_native_client_buffer API
    EGLImageKHR image = EGL_NO_IMAGE_KHR;
    createEGLImageANWBClientBufferSource(1, 1, 1, kNativeClientBufferAttribs_RGBA8_Renderbuffer,
                                         attribs, {{kSrgbColor, 4}}, &image);
    // We are locking AHB to initialize AHB with data. The lock is allowed to fail, and may fail if
    // driver decided to allocate with framebuffer compression enabled.
    ANGLE_SKIP_TEST_IF(image == EGL_NO_IMAGE_KHR);

    // Create the target
    GLRenderbuffer target;
    createEGLImageTargetRenderbuffer(image, target);

    // Create a framebuffer with renderbuffer attachment and clear it
    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, target);
    glClearColor(kLinearColor[0] / 255.0f, kLinearColor[1] / 255.0f, kLinearColor[2] / 255.0f,
                 kLinearColor[3] / 255.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    // Verify that the render buffer has the expected color
    verifyResultsRenderbuffer(target,
                              getExpected2DColorForAttribList(attribs, EglImageUsage::Rendering));

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
}

TEST_P(ImageTest, Source2DTargetExternal)
{
    Source2DTargetExternal_helper(kDefaultAttribs);
}

TEST_P(ImageTest, Source2DTargetExternal_Colorspace)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 && !IsGLExtensionEnabled("GL_EXT_sRGB"));
    ANGLE_SKIP_TEST_IF(!hasImageGLColorspaceExt());
    Source2DTargetExternal_helper(kColorspaceAttribs);
}

void ImageTest::Source2DTargetExternal_helper(const EGLint *attribs)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt() || !hasExternalExt());

    // Ozone only supports external target for images created with EGL_EXT_image_dma_buf_import
    ANGLE_SKIP_TEST_IF(IsOzone());

    // Create the Image
    GLTexture source;
    EGLImageKHR image;
    createEGLImage2DTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, attribs, kSrgbColor, source,
                                  &image);

    // Create the target
    GLTexture target;
    createEGLImageTargetTextureExternal(image, target);

    // Verify that the target texture has the expected color
    verifyResultsExternal(target, getExpected2DColorForAttribList(attribs));

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
}

TEST_P(ImageTestES3, Source2DTargetExternalESSL3)
{
    Source2DTargetExternalESSL3_helper(kDefaultAttribs);
}

TEST_P(ImageTestES3, Source2DTargetExternalESSL3_Colorspace)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 && !IsGLExtensionEnabled("GL_EXT_sRGB"));
    ANGLE_SKIP_TEST_IF(!hasImageGLColorspaceExt());
    Source2DTargetExternalESSL3_helper(kColorspaceAttribs);
}

void ImageTest::Source2DTargetExternalESSL3_helper(const EGLint *attribs)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt() ||
                       !hasExternalESSL3Ext());

    // Create the Image
    GLTexture source;
    EGLImageKHR image;
    createEGLImage2DTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, attribs, kSrgbColor, source,
                                  &image);

    // Create the target
    GLTexture target;
    createEGLImageTargetTextureExternal(image, target);

    // Verify that the target texture has the expected color
    verifyResultsExternalESSL3(target, getExpected2DColorForAttribList(attribs));

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
}

TEST_P(ImageTest, SourceCubeTarget2D)
{
    SourceCubeTarget2D_helper(kDefaultAttribs);
}

TEST_P(ImageTest, SourceCubeTarget2D_Colorspace)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 && !IsGLExtensionEnabled("GL_EXT_sRGB"));
    ANGLE_SKIP_TEST_IF(!hasImageGLColorspaceExt());
    SourceCubeTarget2D_helper(kColorspaceAttribs);
}

void ImageTest::SourceCubeTarget2D_helper(const EGLint *attribs)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !hasCubemapExt());

    for (EGLenum faceIdx = 0; faceIdx < 6; faceIdx++)
    {
        // Create the Image
        GLTexture source;
        EGLImageKHR image;
        createEGLImageCubemapTextureSource(
            1, 1, GL_RGBA, GL_UNSIGNED_BYTE, attribs, reinterpret_cast<uint8_t *>(kSrgbColorCube),
            sizeof(GLubyte) * 4, EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR + faceIdx, source, &image);

        // Create the target
        GLTexture target;
        createEGLImageTargetTexture2D(image, target);

        // Verify that the target texture has the expected color
        verifyResults2D(target, &getExpectedCubeColorForAttribList(attribs)[faceIdx * 4]);

        // Clean up
        eglDestroyImageKHR(window->getDisplay(), image);
    }
}

TEST_P(ImageTest, SourceCubeTargetRenderbuffer)
{
    SourceCubeTargetRenderbuffer_helper(kDefaultAttribs);
}

TEST_P(ImageTest, SourceCubeTargetRenderbuffer_Colorspace)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);
    ANGLE_SKIP_TEST_IF(!hasImageGLColorspaceExt());
    SourceCubeTargetRenderbuffer_helper(kColorspaceAttribs);
}

void ImageTest::SourceCubeTargetRenderbuffer_helper(const EGLint *attribs)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !hasCubemapExt());

    // http://anglebug.com/42261821
    ANGLE_SKIP_TEST_IF(IsVulkan() && IsIntel() && IsFuchsia());

    for (EGLenum faceIdx = 0; faceIdx < 6; faceIdx++)
    {
        // Create the Image
        GLTexture source;
        EGLImageKHR image;
        createEGLImageCubemapTextureSource(
            1, 1, GL_RGBA, GL_UNSIGNED_BYTE, attribs, reinterpret_cast<uint8_t *>(kSrgbColorCube),
            sizeof(GLubyte) * 4, EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR + faceIdx, source, &image);

        // Create the target
        GLRenderbuffer target;
        createEGLImageTargetRenderbuffer(image, target);

        // Create a framebuffer with renderbuffer attachment and clear it
        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, target);
        glClearColor(
            kLinearColorCube[faceIdx * 4 + 0] / 255.0f, kLinearColorCube[faceIdx * 4 + 1] / 255.0f,
            kLinearColorCube[faceIdx * 4 + 2] / 255.0f, kLinearColorCube[faceIdx * 4 + 3] / 255.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ASSERT_GL_NO_ERROR();

        // Verify that the render buffer has the expected color
        verifyResultsRenderbuffer(target, &getExpectedCubeColorForAttribList(
                                              attribs, EglImageUsage::Rendering)[faceIdx * 4]);

        // Clean up
        eglDestroyImageKHR(window->getDisplay(), image);
    }
}

// Test cubemap -> external texture EGL images.
TEST_P(ImageTest, SourceCubeTargetExternal)
{
    SourceCubeTargetExternal_helper(kDefaultAttribs);
}

TEST_P(ImageTest, SourceCubeTargetExternal_Colorspace)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 && !IsGLExtensionEnabled("GL_EXT_sRGB"));
    ANGLE_SKIP_TEST_IF(!hasImageGLColorspaceExt());
    SourceCubeTargetExternal_helper(kColorspaceAttribs);
}

void ImageTest::SourceCubeTargetExternal_helper(const EGLint *attribs)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !hasCubemapExt() || !hasExternalExt());

    // Ozone only supports external target for images created with EGL_EXT_image_dma_buf_import
    ANGLE_SKIP_TEST_IF(IsOzone());

    for (EGLenum faceIdx = 0; faceIdx < 6; faceIdx++)
    {
        // Create the Image
        GLTexture source;
        EGLImageKHR image;
        // Upload sRGB color so that it is converted to linear when sampling.
        createEGLImageCubemapTextureSource(
            1, 1, GL_RGBA, GL_UNSIGNED_BYTE, attribs, reinterpret_cast<uint8_t *>(kSrgbColorCube),
            sizeof(GLubyte) * 4, EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR + faceIdx, source, &image);

        // Create the target
        GLTexture target;
        createEGLImageTargetTextureExternal(image, target);

        // Verify that the target texture has the expected color
        verifyResultsExternal(target, &getExpectedCubeColorForAttribList(attribs)[faceIdx * 4]);

        // Clean up
        eglDestroyImageKHR(window->getDisplay(), image);
    }
}

// Test cubemap -> external texture EGL images using ESSL3 shaders.
TEST_P(ImageTestES3, SourceCubeTargetExternalESSL3)
{
    SourceCubeTargetExternalESSL3_helper(kDefaultAttribs);
}

TEST_P(ImageTestES3, SourceCubeTargetExternalESSL3_Colorspace)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 && !IsGLExtensionEnabled("GL_EXT_sRGB"));
    ANGLE_SKIP_TEST_IF(!hasImageGLColorspaceExt());
    SourceCubeTargetExternalESSL3_helper(kColorspaceAttribs);
}

void ImageTest::SourceCubeTargetExternalESSL3_helper(const EGLint *attribs)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasExternalESSL3Ext() || !hasBaseExt() || !hasCubemapExt());

    for (EGLenum faceIdx = 0; faceIdx < 6; faceIdx++)
    {
        // Create the Image
        GLTexture source;
        EGLImageKHR image;
        // Upload sRGB color so that it is converted to linear when sampling.
        createEGLImageCubemapTextureSource(
            1, 1, GL_RGBA, GL_UNSIGNED_BYTE, attribs, reinterpret_cast<uint8_t *>(kSrgbColorCube),
            sizeof(GLubyte) * 4, EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR + faceIdx, source, &image);

        // Create the target
        GLTexture target;
        createEGLImageTargetTextureExternal(image, target);

        // Verify that the target texture has the expected color
        verifyResultsExternalESSL3(target,
                                   &getExpectedCubeColorForAttribList(attribs)[faceIdx * 4]);

        // Clean up
        eglDestroyImageKHR(window->getDisplay(), image);
    }
}

TEST_P(ImageTest, Source3DTargetTexture)
{
    Source3DTargetTexture_helper(default3DAttribs);
}

TEST_P(ImageTest, Source3DTargetTexture_Colorspace)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 && !IsGLExtensionEnabled("GL_EXT_sRGB"));
    ANGLE_SKIP_TEST_IF(!hasImageGLColorspaceExt());
    Source3DTargetTexture_helper(colorspace3DAttribs);
}

void ImageTest::Source3DTargetTexture_helper(EGLint *attribs)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has3DTextureExt());

    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 && !IsGLExtensionEnabled("GL_OES_texture_3D"));

    constexpr size_t depth = 2;

    for (size_t layer = 0; layer < depth; layer++)
    {
        // Create the Image
        GLTexture source;
        EGLImageKHR image;
        // Upload sRGB color so that it is converted to linear when sampling.
        attribs[kTextureZOffsetAttributeIndex] = static_cast<EGLint>(layer);
        createEGLImage3DTextureSource(1, 1, depth, GL_RGBA, GL_UNSIGNED_BYTE, attribs, kSrgbColor3D,
                                      source, &image);

        // Create the target
        GLTexture target;
        createEGLImageTargetTexture2D(image, target);

        // Verify that the target texture has the expected color
        verifyResults2D(target, &getExpected3DColorForAttribList(attribs)[layer * 4]);

        // Clean up
        eglDestroyImageKHR(window->getDisplay(), image);
    }
}

TEST_P(ImageTest, Source3DTargetRenderbuffer)
{
    Source3DTargetRenderbuffer_helper(default3DAttribs);
}

TEST_P(ImageTest, Source3DTargetRenderbuffer_Colorspace)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);
    ANGLE_SKIP_TEST_IF(!hasImageGLColorspaceExt());
    Source3DTargetRenderbuffer_helper(colorspace3DAttribs);
}

void ImageTest::Source3DTargetRenderbuffer_helper(EGLint *attribs)
{
    // Qualcom drivers appear to always bind the 0 layer of the source 3D texture when the
    // target is a renderbuffer. They work correctly when the target is a 2D texture.
    // http://anglebug.com/42261453
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());

    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has3DTextureExt());

    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 && !IsGLExtensionEnabled("GL_OES_texture_3D"));

    constexpr size_t depth = 2;

    for (size_t layer = 0; layer < depth; layer++)
    {
        // Create the Image
        GLTexture source;
        EGLImageKHR image;

        attribs[kTextureZOffsetAttributeIndex] = static_cast<EGLint>(layer);
        createEGLImage3DTextureSource(1, 1, depth, GL_RGBA, GL_UNSIGNED_BYTE, attribs, kSrgbColor3D,
                                      source, &image);

        // Create the target
        GLRenderbuffer target;
        createEGLImageTargetRenderbuffer(image, target);

        // Create a framebuffer with renderbuffer attachment and clear it
        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, target);
        glClearColor(kLinearColor3D[layer * 4 + 0] / 255.0f, kLinearColor3D[layer * 4 + 1] / 255.0f,
                     kLinearColor3D[layer * 4 + 2] / 255.0f,
                     kLinearColor3D[layer * 4 + 3] / 255.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ASSERT_GL_NO_ERROR();

        // Verify that the render buffer has the expected color
        verifyResultsRenderbuffer(
            target, &getExpected3DColorForAttribList(attribs, EglImageUsage::Rendering)[layer * 4]);

        // Clean up
        eglDestroyImageKHR(window->getDisplay(), image);
    }
}

// Test 3D -> external texture EGL images.
TEST_P(ImageTest, Source3DTargetExternal)
{
    Source3DTargetExternal_helper(default3DAttribs);
}

TEST_P(ImageTest, Source3DTargetExternal_Colorspace)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 && !IsGLExtensionEnabled("GL_EXT_sRGB"));
    ANGLE_SKIP_TEST_IF(!hasImageGLColorspaceExt());
    Source3DTargetExternal_helper(colorspace3DAttribs);
}

void ImageTest::Source3DTargetExternal_helper(EGLint *attribs)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasExternalExt() || !hasBaseExt() || !has3DTextureExt());

    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 && !IsGLExtensionEnabled("GL_OES_texture_3D"));

    // Ozone only supports external target for images created with EGL_EXT_image_dma_buf_import
    ANGLE_SKIP_TEST_IF(IsOzone());

    constexpr size_t depth = 2;

    for (size_t layer = 0; layer < depth; layer++)
    {
        // Create the Image
        GLTexture source;
        EGLImageKHR image;
        // Upload sRGB color so that it is converted to linear when sampling.
        attribs[kTextureZOffsetAttributeIndex] = static_cast<EGLint>(layer);
        createEGLImage3DTextureSource(1, 1, depth, GL_RGBA, GL_UNSIGNED_BYTE, attribs, kSrgbColor3D,
                                      source, &image);

        // Create the target
        GLTexture target;
        createEGLImageTargetTextureExternal(image, target);

        // Verify that the target texture has the expected color
        verifyResultsExternal(target, &getExpected3DColorForAttribList(attribs)[layer * 4]);

        // Clean up
        eglDestroyImageKHR(window->getDisplay(), image);
    }
}

// Test 3D -> external texture EGL images using ESSL3 shaders.
TEST_P(ImageTestES3, Source3DTargetExternalESSL3)
{
    Source3DTargetExternalESSL3_helper(default3DAttribs);
}

TEST_P(ImageTestES3, Source3DTargetExternalESSL3_Colorspace)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 && !IsGLExtensionEnabled("GL_EXT_sRGB"));
    ANGLE_SKIP_TEST_IF(!hasImageGLColorspaceExt());
    Source3DTargetExternalESSL3_helper(colorspace3DAttribs);
}

void ImageTest::Source3DTargetExternalESSL3_helper(EGLint *attribs)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasExternalESSL3Ext() || !hasBaseExt() ||
                       !has3DTextureExt());

    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 && !IsGLExtensionEnabled("GL_OES_texture_3D"));

    constexpr size_t depth = 2;

    for (size_t layer = 0; layer < depth; layer++)
    {
        // Create the Image
        GLTexture source;
        EGLImageKHR image;
        // Upload sRGB color so that it is converted to linear when sampling.
        attribs[kTextureZOffsetAttributeIndex] = static_cast<EGLint>(layer);
        createEGLImage3DTextureSource(1, 1, depth, GL_RGBA, GL_UNSIGNED_BYTE, attribs, kSrgbColor3D,
                                      source, &image);

        // Create the target
        GLTexture target;
        createEGLImageTargetTextureExternal(image, target);

        // Verify that the target texture has the expected color
        verifyResultsExternalESSL3(target, &getExpected3DColorForAttribList(attribs)[layer * 4]);

        // Clean up
        eglDestroyImageKHR(window->getDisplay(), image);
    }
}

TEST_P(ImageTest, SourceRenderbufferTargetTexture)
{
    SourceRenderbufferTargetTexture_helper(kDefaultAttribs);
}

TEST_P(ImageTest, SourceRenderbufferTargetTexture_Colorspace)
{
    ANGLE_SKIP_TEST_IF(!hasImageGLColorspaceExt());
    SourceRenderbufferTargetTexture_helper(kColorspaceAttribs);
}

void ImageTest::SourceRenderbufferTargetTexture_helper(const EGLint *attribs)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !hasRenderbufferExt());

    // Create the Image
    GLRenderbuffer source;
    EGLImageKHR image;
    createEGLImageRenderbufferSource(1, 1, GL_RGBA8_OES, attribs, source, &image);

    // Create a framebuffer with renderbuffer attachment and clear it
    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, source);
    glClearColor(kSrgbColor[0] / 255.0f, kSrgbColor[1] / 255.0f, kSrgbColor[2] / 255.0f,
                 kSrgbColor[3] / 255.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    // Create the target
    GLTexture target;
    createEGLImageTargetTexture2D(image, target);

    // Verify that the target texture has the expected color
    verifyResults2D(target, getExpected2DColorForAttribList(attribs));

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
}

// Test renderbuffer -> external texture EGL images.
TEST_P(ImageTest, SourceRenderbufferTargetTextureExternal)
{
    SourceRenderbufferTargetTextureExternal_helper(kDefaultAttribs);
}

TEST_P(ImageTest, SourceRenderbufferTargetTextureExternal_Colorspace)
{
    ANGLE_SKIP_TEST_IF(!hasImageGLColorspaceExt());
    SourceRenderbufferTargetTextureExternal_helper(kColorspaceAttribs);
}

void ImageTest::SourceRenderbufferTargetTextureExternal_helper(const EGLint *attribs)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasExternalExt() || !hasBaseExt() || !hasRenderbufferExt());

    // Ozone only supports external target for images created with EGL_EXT_image_dma_buf_import
    ANGLE_SKIP_TEST_IF(IsOzone());

    // Create the Image
    GLRenderbuffer source;
    EGLImageKHR image;
    createEGLImageRenderbufferSource(1, 1, GL_RGBA8_OES, attribs, source, &image);

    // Create a framebuffer with renderbuffer attachment and clear it
    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, source);
    glClearColor(kSrgbColor[0] / 255.0f, kSrgbColor[1] / 255.0f, kSrgbColor[2] / 255.0f,
                 kSrgbColor[3] / 255.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    // Create the target
    GLTexture target;
    createEGLImageTargetTextureExternal(image, target);

    // Verify that the target texture has the expected color
    verifyResultsExternal(target, getExpected2DColorForAttribList(attribs));

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
}

// Test renderbuffer -> external texture EGL images using ESSL3 shaders.
TEST_P(ImageTestES3, SourceRenderbufferTargetTextureExternalESSL3)
{
    SourceRenderbufferTargetTextureExternalESSL3_helper(kDefaultAttribs);
}

TEST_P(ImageTestES3, SourceRenderbufferTargetTextureExternalESSL3_Colorspace)
{
    ANGLE_SKIP_TEST_IF(!hasImageGLColorspaceExt());
    SourceRenderbufferTargetTextureExternalESSL3_helper(kColorspaceAttribs);
}

void ImageTest::SourceRenderbufferTargetTextureExternalESSL3_helper(const EGLint *attribs)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasExternalESSL3Ext() || !hasBaseExt() ||
                       !hasRenderbufferExt());

    // Create the Image
    GLRenderbuffer source;
    EGLImageKHR image;
    createEGLImageRenderbufferSource(1, 1, GL_RGBA8_OES, attribs, source, &image);

    // Create a framebuffer with renderbuffer attachment and clear it
    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, source);
    glClearColor(kSrgbColor[0] / 255.0f, kSrgbColor[1] / 255.0f, kSrgbColor[2] / 255.0f,
                 kSrgbColor[3] / 255.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    // Create the target
    GLTexture target;
    createEGLImageTargetTextureExternal(image, target);

    // Verify that the target texture has the expected color
    verifyResultsExternalESSL3(target, getExpected2DColorForAttribList(attribs));

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
}

TEST_P(ImageTest, SourceRenderbufferTargetRenderbuffer)
{
    SourceRenderbufferTargetRenderbuffer_helper(kDefaultAttribs);
}

TEST_P(ImageTest, SourceRenderbufferTargetRenderbuffer_Colorspace)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 && !IsGLExtensionEnabled("GL_EXT_sRGB"));
    ANGLE_SKIP_TEST_IF(!hasImageGLColorspaceExt());
    SourceRenderbufferTargetRenderbuffer_helper(kColorspaceAttribs);
}

void ImageTest::SourceRenderbufferTargetRenderbuffer_helper(const EGLint *attribs)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !hasRenderbufferExt());

    // Create the Image
    GLRenderbuffer source;
    EGLImageKHR image;
    createEGLImageRenderbufferSource(1, 1, GL_RGBA8_OES, attribs, source, &image);

    // Create the target
    GLRenderbuffer target;
    createEGLImageTargetRenderbuffer(image, target);

    // Create a framebuffer with renderbuffer attachment and clear it
    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, target);
    glClearColor(kLinearColor[0] / 255.0f, kLinearColor[1] / 255.0f, kLinearColor[2] / 255.0f,
                 kLinearColor[3] / 255.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    // Verify that the render buffer has the expected color
    verifyResultsRenderbuffer(target,
                              getExpected2DColorForAttribList(attribs, EglImageUsage::Rendering));

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
}

void ImageTest::FixedRatedCompressionBasicHelper(const GLint *attribs)
{
    constexpr size_t width  = 16;
    constexpr size_t height = 16;
    GLTexture textureSource;
    EGLImageKHR image;
    EGLWindow *window = getEGLWindow();
    createEGLImage2DTextureStorage(width, height, GL_RGBA8, attribs, textureSource, &image);

    GLTexture textureAttachment;
    createEGLImageTargetTextureStorage(image, GL_TEXTURE_2D, textureAttachment, attribs);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureAttachment,
                           0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    ANGLE_GL_PROGRAM(drawRed, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    drawQuad(drawRed, essl1_shaders::PositionAttrib(), 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, width, height, GLColor::red);
    ASSERT_GL_NO_ERROR();
    eglDestroyImageKHR(window->getDisplay(), image);
}

// Test basic usage of extension GL_EXT_EGL_image_storage_compression
TEST_P(ImageTest, FixedRatedCompressionBasic)
{
    ANGLE_SKIP_TEST_IF(!hasEglImageStorageExt() || !hasEglImageStorageCompressionExt());
    ANGLE_SKIP_TEST_IF(!hasTextureStorageCompressionExt());
    constexpr GLint kAttribList[3][3] = {
        {GL_NONE, GL_NONE, GL_NONE},
        {GL_SURFACE_COMPRESSION_EXT, GL_SURFACE_COMPRESSION_FIXED_RATE_NONE_EXT, GL_NONE},
        {GL_SURFACE_COMPRESSION_EXT, GL_SURFACE_COMPRESSION_FIXED_RATE_DEFAULT_EXT, GL_NONE},
    };
    for (const GLint *attribs : kAttribList)
    {
        FixedRatedCompressionBasicHelper(attribs);
    }
}

void ImageTest::FixedRatedCompressionImageAttribCheck(EGLImageKHR image,
                                                      const GLint *attribs,
                                                      const GLint expectResult)
{
    GLTexture textureAttachment;
    // Create a target texture from the image
    glBindTexture(GL_TEXTURE_2D, textureAttachment);
    glEGLImageTargetTexStorageEXT(GL_TEXTURE_2D, image, attribs);
    ASSERT_GL_ERROR(expectResult);
}

// Test whether the result is expected when the attributes mismatched with source
TEST_P(ImageTest, FixedRatedCompressionMixedAttrib)
{
    ANGLE_SKIP_TEST_IF(!hasEglImageStorageExt() || !hasEglImageStorageCompressionExt());
    ANGLE_SKIP_TEST_IF(!hasTextureStorageCompressionExt());
    constexpr size_t width                 = 16;
    constexpr size_t height                = 16;
    EGLWindow *window                      = getEGLWindow();
    constexpr GLint textureAttribList[][3] = {
        {GL_NONE, GL_NONE, GL_NONE},
        {GL_SURFACE_COMPRESSION_EXT, GL_SURFACE_COMPRESSION_FIXED_RATE_NONE_EXT, GL_NONE},
        {GL_SURFACE_COMPRESSION_EXT, GL_SURFACE_COMPRESSION_FIXED_RATE_DEFAULT_EXT, GL_NONE},
    };
    constexpr GLint imageAttribList[][3] = {
        {GL_NONE, GL_NONE, GL_NONE},
        {GL_SURFACE_COMPRESSION_EXT, GL_SURFACE_COMPRESSION_FIXED_RATE_NONE_EXT, GL_NONE},
    };

    constexpr GLint invalidImageAttribList[][3] = {
        {GL_SURFACE_COMPRESSION_EXT, GL_SURFACE_COMPRESSION_EXT, GL_NONE},
        {GL_SURFACE_COMPRESSION_FIXED_RATE_NONE_EXT, GL_SURFACE_COMPRESSION_FIXED_RATE_NONE_EXT,
         GL_NONE},
    };

    for (const GLint *textureAttribs : textureAttribList)
    {
        GLTexture textureSource;
        EGLImageKHR image;
        bool isFixRatedCompressed;
        createEGLImage2DTextureStorage(width, height, GL_RGBA8, textureAttribs, textureSource,
                                       &image);
        /* Query compression rate */
        GLint compressRate = GL_SURFACE_COMPRESSION_FIXED_RATE_NONE_EXT;
        glGetTexParameteriv(GL_TEXTURE_2D, GL_SURFACE_COMPRESSION_EXT, &compressRate);
        ASSERT_GL_NO_ERROR();
        isFixRatedCompressed = (compressRate == GL_SURFACE_COMPRESSION_FIXED_RATE_DEFAULT_EXT ||
                                (compressRate >= GL_SURFACE_COMPRESSION_FIXED_RATE_1BPC_EXT &&
                                 compressRate <= GL_SURFACE_COMPRESSION_FIXED_RATE_12BPC_EXT));

        for (const GLint *attribs : imageAttribList)
        {
            if (isFixRatedCompressed && attribs[0] == GL_SURFACE_COMPRESSION_EXT &&
                attribs[1] == GL_SURFACE_COMPRESSION_FIXED_RATE_NONE_EXT)
            {
                FixedRatedCompressionImageAttribCheck(image, attribs, GL_INVALID_OPERATION);
            }
            else
            {
                FixedRatedCompressionImageAttribCheck(image, attribs, GL_NO_ERROR);
            }
        }

        for (const GLint *attribs : invalidImageAttribList)
        {
            FixedRatedCompressionImageAttribCheck(image, attribs, GL_INVALID_VALUE);
        }

        eglDestroyImageKHR(window->getDisplay(), image);
    }
}

// Delete the source texture and EGL image.  The image targets should still have the same data
// because
// they hold refs to the image.
TEST_P(ImageTest, Deletion)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());

    GLubyte originalData[4] = {255, 0, 255, 255};
    GLubyte updateData[4]   = {0, 255, 0, 255};

    // Create the Image
    GLTexture source;
    EGLImageKHR image;
    createEGLImage2DTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, kDefaultAttribs, originalData,
                                  source, &image);

    // Create multiple targets
    GLTexture targetTexture;
    createEGLImageTargetTexture2D(image, targetTexture);

    GLRenderbuffer targetRenderbuffer;
    createEGLImageTargetRenderbuffer(image, targetRenderbuffer);

    // Delete the source texture
    source.reset();

    // Expect that both the targets have the original data
    verifyResults2D(targetTexture, originalData);
    verifyResultsRenderbuffer(targetRenderbuffer, originalData);

    // Update the data of the target
    glBindTexture(GL_TEXTURE_2D, targetTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, updateData);

    // Expect that both targets have the updated data
    verifyResults2D(targetTexture, updateData);
    verifyResultsRenderbuffer(targetRenderbuffer, updateData);

    // Delete the EGL image
    eglDestroyImageKHR(window->getDisplay(), image);
    image = EGL_NO_IMAGE_KHR;

    // Update the data of the target back to the original data
    glBindTexture(GL_TEXTURE_2D, targetTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, originalData);

    // Expect that both targets have the original data again
    verifyResults2D(targetTexture, originalData);
    verifyResultsRenderbuffer(targetRenderbuffer, originalData);
}

TEST_P(ImageTest, MipLevels)
{
    // Driver returns OOM in read pixels, some internal error.
    ANGLE_SKIP_TEST_IF(IsOzone() && IsOpenGLES());
    // Also fails on NVIDIA Shield TV bot.
    // http://anglebug.com/42262494
    ANGLE_SKIP_TEST_IF(IsNVIDIAShield() && IsOpenGLES());
    // On Vulkan, the clear operation in the loop is optimized with a render pass loadOp=Clear.  On
    // Linux/Intel, that operation is mistakenly clearing the rest of the mips to 0.
    // http://anglebug.com/42261962
    ANGLE_SKIP_TEST_IF(IsVulkan() && IsLinux() && IsIntel());

    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());

    const size_t mipLevels   = 3;
    const size_t textureSize = 4;
    std::vector<GLColor> mip0Data(textureSize * textureSize, GLColor::red);
    std::vector<GLColor> mip1Data(mip0Data.size() << 1, GLColor::green);
    std::vector<GLColor> mip2Data(mip0Data.size() << 2, GLColor::blue);
    GLubyte *data[mipLevels] = {
        reinterpret_cast<GLubyte *>(&mip0Data[0]),
        reinterpret_cast<GLubyte *>(&mip1Data[0]),
        reinterpret_cast<GLubyte *>(&mip2Data[0]),
    };

    GLTexture source;
    glBindTexture(GL_TEXTURE_2D, source);

    for (size_t level = 0; level < mipLevels; level++)
    {
        glTexImage2D(GL_TEXTURE_2D, static_cast<GLint>(level), GL_RGBA, textureSize >> level,
                     textureSize >> level, 0, GL_RGBA, GL_UNSIGNED_BYTE, data[level]);
    }

    ASSERT_GL_NO_ERROR();

    for (size_t level = 0; level < mipLevels; level++)
    {
        // Create the Image
        EGLint attribs[] = {
            EGL_GL_TEXTURE_LEVEL_KHR,
            static_cast<EGLint>(level),
            EGL_NONE,
        };
        EGLImageKHR image =
            eglCreateImageKHR(window->getDisplay(), window->getContext(), EGL_GL_TEXTURE_2D_KHR,
                              reinterpretHelper<EGLClientBuffer>(source), attribs);
        ASSERT_EGL_SUCCESS();

        // Create a texture and renderbuffer target
        GLTexture textureTarget;
        createEGLImageTargetTexture2D(image, textureTarget);

        // Disable mipmapping
        glBindTexture(GL_TEXTURE_2D, textureTarget);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        GLRenderbuffer renderbufferTarget;
        createEGLImageTargetRenderbuffer(image, renderbufferTarget);

        // Expect that the targets have the same color as the source texture
        verifyResults2D(textureTarget, data[level]);
        verifyResultsRenderbuffer(renderbufferTarget, data[level]);

        // Update the data by uploading data to the texture
        std::vector<GLuint> textureUpdateData(textureSize * textureSize, level);
        glBindTexture(GL_TEXTURE_2D, textureTarget);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureSize >> level, textureSize >> level, GL_RGBA,
                        GL_UNSIGNED_BYTE, textureUpdateData.data());
        ASSERT_GL_NO_ERROR();

        // Expect that both the texture and renderbuffer see the updated texture data
        verifyResults2D(textureTarget, reinterpret_cast<GLubyte *>(textureUpdateData.data()));
        verifyResultsRenderbuffer(renderbufferTarget,
                                  reinterpret_cast<GLubyte *>(textureUpdateData.data()));

        // Update the renderbuffer by clearing it
        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                  renderbufferTarget);

        GLubyte clearValue = static_cast<GLubyte>(level);
        GLubyte renderbufferClearData[4]{clearValue, clearValue, clearValue, clearValue};
        glClearColor(renderbufferClearData[0] / 255.0f, renderbufferClearData[1] / 255.0f,
                     renderbufferClearData[2] / 255.0f, renderbufferClearData[3] / 255.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        ASSERT_GL_NO_ERROR();

        // Expect that both the texture and renderbuffer see the cleared renderbuffer data
        verifyResults2D(textureTarget, renderbufferClearData);
        verifyResultsRenderbuffer(renderbufferTarget, renderbufferClearData);

        // Clean up
        eglDestroyImageKHR(window->getDisplay(), image);
    }
}

// Respecify the source texture, orphaning it.  The target texture should not have updated data.
TEST_P(ImageTest, Respecification)
{
    // Respecification of textures that does not change the size of the level attached to the EGL
    // image does not cause orphaning on Qualcomm devices. http://anglebug.com/42261452
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());
    ANGLE_SKIP_TEST_IF(IsOzone() && IsOpenGLES());

    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());

    GLubyte originalData[4] = {255, 0, 255, 255};
    GLubyte updateData[4]   = {0, 255, 0, 255};

    // Create the Image
    GLTexture source;
    EGLImageKHR image;
    createEGLImage2DTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, kDefaultAttribs, originalData,
                                  source, &image);

    // Create the target
    GLTexture target;
    createEGLImageTargetTexture2D(image, target);

    // Respecify source
    glBindTexture(GL_TEXTURE_2D, source);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, updateData);

    // Expect that the target texture has the original data
    verifyResults2D(target, originalData);

    // Expect that the source texture has the updated data
    verifyResults2D(source, updateData);

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
}

// Respecify the source texture with a different size, orphaning it.  The target texture should not
// have updated data.
TEST_P(ImageTest, RespecificationDifferentSize)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());

    GLubyte originalData[4] = {255, 0, 255, 255};
    GLubyte updateData[16]  = {0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255};

    // Create the Image
    GLTexture source;
    EGLImageKHR image;
    createEGLImage2DTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, kDefaultAttribs, originalData,
                                  source, &image);

    // Create the target
    GLTexture target;
    createEGLImageTargetTexture2D(image, target);

    // Respecify source
    glBindTexture(GL_TEXTURE_2D, source);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, updateData);

    // Expect that the target texture has the original data
    verifyResults2D(target, originalData);

    // Expect that the source texture has the updated data
    verifyResults2D(source, updateData);

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
}

// First render to a target texture, then respecify the source texture, orphaning it.
// The target texture's FBO should be notified of the target texture's orphaning.
TEST_P(ImageTest, RespecificationWithFBO)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());

    GLubyte originalData[4] = {255, 0, 255, 255};
    GLubyte updateData[4]   = {0, 255, 0, 255};

    // Create the Image
    GLTexture source;
    EGLImageKHR image;
    createEGLImage2DTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, kDefaultAttribs, originalData,
                                  source, &image);

    // Create the target
    GLTexture target;
    createEGLImageTargetTexture2D(image, target);

    // Render to the target texture
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target, 0);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);

    // Respecify source with same parameters. This should not change the texture storage in D3D11.
    glBindTexture(GL_TEXTURE_2D, source);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, updateData);

    // Expect that the source texture has the updated data
    verifyResults2D(source, updateData);

    // Render to the target texture again and verify it gets the rendered pixels.
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
}

// Test that respecifying a level of the target texture orphans it and keeps a copy of the EGLimage
// data
TEST_P(ImageTest, RespecificationOfOtherLevel)
{
    // Respecification of textures that does not change the size of the level attached to the EGL
    // image does not cause orphaning on Qualcomm devices. http://anglebug.com/42261452
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());

    // It is undefined what happens to the mip 0 of the dest texture after it is orphaned. Some
    // backends explicitly copy the data but Vulkan does not.
    ANGLE_SKIP_TEST_IF(IsVulkan());

    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());

    GLubyte originalData[2 * 2 * 4] = {
        255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255,
    };

    GLubyte updateData[2 * 2 * 4] = {
        0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255,
    };

    // Create the Image
    GLTexture source;
    EGLImageKHR image;
    createEGLImage2DTextureSource(2, 2, GL_RGBA, GL_UNSIGNED_BYTE, kDefaultAttribs, originalData,
                                  source, &image);

    // Create the target
    GLTexture target;
    createEGLImageTargetTexture2D(image, target);

    // Expect that the target and source textures have the original data
    verifyResults2D(source, originalData);
    verifyResults2D(target, originalData);

    // Add a new mipLevel to the target, orphaning it
    glBindTexture(GL_TEXTURE_2D, target);
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, originalData);
    EXPECT_GL_NO_ERROR();

    // Expect that the target and source textures still have the original data
    verifyResults2D(source, originalData);
    verifyResults2D(target, originalData);

    // Update the source's data
    glBindTexture(GL_TEXTURE_2D, source);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, updateData);

    // Expect that the target still has the original data and source has the updated data
    verifyResults2D(source, updateData);
    verifyResults2D(target, originalData);

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
}

// Update the data of the source and target textures.  All image siblings should have the new data.
TEST_P(ImageTest, UpdatedData)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());

    GLubyte originalData[4] = {255, 0, 255, 255};
    GLubyte updateData[4]   = {0, 255, 0, 255};

    // Create the Image
    GLTexture source;
    EGLImageKHR image;
    createEGLImage2DTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, kDefaultAttribs, originalData,
                                  source, &image);

    // Create multiple targets
    GLTexture targetTexture;
    createEGLImageTargetTexture2D(image, targetTexture);

    GLRenderbuffer targetRenderbuffer;
    createEGLImageTargetRenderbuffer(image, targetRenderbuffer);

    // Expect that both the source and targets have the original data
    verifyResults2D(source, originalData);
    verifyResults2D(targetTexture, originalData);
    verifyResultsRenderbuffer(targetRenderbuffer, originalData);

    // Update the data of the source
    glBindTexture(GL_TEXTURE_2D, source);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, updateData);

    // Expect that both the source and targets have the updated data
    verifyResults2D(source, updateData);
    verifyResults2D(targetTexture, updateData);
    verifyResultsRenderbuffer(targetRenderbuffer, updateData);

    // Update the data of the target back to the original data
    glBindTexture(GL_TEXTURE_2D, targetTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, originalData);

    // Expect that both the source and targets have the original data again
    verifyResults2D(source, originalData);
    verifyResults2D(targetTexture, originalData);
    verifyResultsRenderbuffer(targetRenderbuffer, originalData);

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
}

// Check that the external texture is successfully updated when only glTexSubImage2D is called.
TEST_P(ImageTest, AHBUpdatedExternalTexture)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    GLubyte originalData[4]      = {255, 0, 255, 255};
    GLubyte updateData[4]        = {0, 255, 0, 255};
    const uint32_t bytesPerPixel = 4;

    // Create the Image
    AHardwareBuffer *source;
    EGLImageKHR image;
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                              kDefaultAHBUsage, kDefaultAttribs,
                                              {{originalData, bytesPerPixel}}, &source, &image);

    // Create target
    GLTexture targetTexture;
    createEGLImageTargetTexture2D(image, targetTexture);

    // Expect that both the target have the original data
    verifyResults2D(targetTexture, originalData);

    // Update the data of the source
    glBindTexture(GL_TEXTURE_2D, targetTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, updateData);

    // Set sync object and flush the GL commands
    EGLSyncKHR fence = eglCreateSyncKHR(window->getDisplay(), EGL_SYNC_FENCE_KHR, NULL);
    ASSERT_NE(fence, EGL_NO_SYNC_KHR);
    glFlush();

    // Delete the target texture
    targetTexture.reset();

    // Wait that the flush command is finished
    EGLint result = eglClientWaitSyncKHR(window->getDisplay(), fence, 0, 1000000000);
    ASSERT_EQ(result, EGL_CONDITION_SATISFIED_KHR);
    ASSERT_EGL_TRUE(eglDestroySyncKHR(window->getDisplay(), fence));

    // Delete the EGL image
    eglDestroyImageKHR(window->getDisplay(), image);

    // Access the android hardware buffer directly to check the data is updated
    verifyResultAHB(source, {{updateData, bytesPerPixel}});

    // Create the EGL image again
    image =
        eglCreateImageKHR(window->getDisplay(), EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID,
                          angle::android::AHardwareBufferToClientBuffer(source), kDefaultAttribs);
    ASSERT_EGL_SUCCESS();

    // Create the target texture again
    GLTexture targetTexture2;
    createEGLImageTargetTexture2D(image, targetTexture2);

    // Expect that the target have the update data
    verifyResults2D(targetTexture2, updateData);

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
    destroyAndroidHardwareBuffer(source);
}

// Check that the texture successfully updates when an image is deleted
TEST_P(ImageTest, DeletedImageWithSameSizeAndFormat)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());

    GLubyte originalData[4] = {255, 0, 255, 255};
    GLubyte updateData[4]   = {0, 255, 0, 255};

    // Create the Image
    GLTexture source;
    EGLImageKHR image;
    createEGLImage2DTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, kDefaultAttribs, originalData,
                                  source, &image);

    // Create texture & bind to Image
    GLTexture texture;
    createEGLImageTargetTexture2D(image, texture);

    // Delete Image
    eglDestroyImageKHR(window->getDisplay(), image);

    ASSERT_EGL_SUCCESS();

    // Redefine Texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, updateData);

    ASSERT_GL_NO_ERROR();
}

// Check that create a source cube texture and then redefine the same target texture with each face
// of source cube texture renders correctly
TEST_P(ImageTest, SourceCubeAndSameTargetTextureWithEachCubeFace)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());

    // Create a source cube map texture
    GLTexture sourceTexture;
    glBindTexture(GL_TEXTURE_CUBE_MAP, sourceTexture);
    uint8_t *data     = reinterpret_cast<uint8_t *>(kLinearColorCube);
    size_t dataStride = sizeof(GLubyte) * 4;
    for (GLenum faceIdx = 0; faceIdx < 6; faceIdx++)
    {
        glTexImage2D(faceIdx + GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, 1, 1, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, data + (faceIdx * dataStride));
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    EGLImageKHR images[6];
    GLTexture targetTexture;
    glBindTexture(GL_TEXTURE_2D, targetTexture);

    for (GLenum faceIdx = 0; faceIdx < 6; faceIdx++)
    {
        // Create the Image with EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR
        images[faceIdx] =
            eglCreateImageKHR(window->getDisplay(), window->getContext(),
                              EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR + faceIdx,
                              reinterpretHelper<EGLClientBuffer>(sourceTexture), kDefaultAttribs);

        // Create a target texture from the image
        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, images[faceIdx]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        ASSERT_GL_NO_ERROR();
        // Expect that the target texture has the same color as the source texture
        verifyResults2D(targetTexture, &kLinearColorCube[faceIdx * 4]);
    }

    // Clean up
    for (GLenum faceIdx = 0; faceIdx < 6; faceIdx++)
    {
        eglDestroyImageKHR(window->getDisplay(), images[faceIdx]);
    }
}

// Case for testing External Texture support in MEC.
// To run this test with the right capture setting, make sure to set these environment variables:
//
// For Linux:
//      export ANGLE_CAPTURE_FRAME_START=2
//      export ANGLE_CAPTURE_FRAME_END=2
//      export ANGLE_CAPTURE_LABEL=external_textures
//      export ANGLE_CAPTURE_OUT_DIR=[PATH_TO_ANGLE]/src/tests/restricted_traces/external_textures/
//
// For Android:
//      adb shell setprop debug.angle.capture.frame_start 2
//      adb shell setprop debug.angle.capture.frame_end 2
//      adb shell setprop debug.angle.capture.label external_textures
//      adb shell setprop debug.angle.capture.out_dir /data/data/externaltextures/angle_capture/
TEST_P(ImageTest, AppTraceExternalTextureDefaultAttribs)
{
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt() ||
                       !hasExternalESSL3Ext());

    constexpr EGLint attribs[] = {
        EGL_IMAGE_PRESERVED,
        EGL_TRUE,
        EGL_NONE,
    };

    externalTextureTracerTestHelper(attribs);
}

// Same as AppTraceExternalTextureUseCase, except we will pass additional attrib_list values in
// EGLAttrib* for eglCreateImageKHR calls
TEST_P(ImageTest, AppTraceExternalTextureOtherAttribs)
{
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt() ||
                       !hasExternalESSL3Ext());

    constexpr EGLint attribs[] = {
        EGL_IMAGE_PRESERVED, EGL_TRUE, EGL_GL_TEXTURE_LEVEL, 0, EGL_NONE,
    };

    externalTextureTracerTestHelper(attribs);
}

// Same as AppTraceExternalTextureUseCase, except we will pass nullptr as EGLAttrib* for
// eglCreateImageKHR calls
TEST_P(ImageTest, AppTraceExternalTextureNullAttribs)
{
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt() ||
                       !hasExternalESSL3Ext());

    externalTextureTracerTestHelper(nullptr);
}

// Alternate case for testing External Texture (created with AHB) support in MEC.
// Make sure to use the following environment variables for the right capture setting on Android:
//
// adb shell setprop debug.angle.capture.frame_start 2
// adb shell setprop debug.angle.capture.frame_end 2
// adb shell setprop debug.angle.capture.label AHB_textures
// adb shell setprop debug.angle.capture.out_dir /data/data/AHBtextures/angle_capture/
TEST_P(ImageTest, AppTraceExternalTextureWithAHBUseCase)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());

    GLubyte data[4] = {7, 51, 197, 231};

    // Create the Image
    AHardwareBuffer *source;
    EGLImageKHR image;
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                              kDefaultAHBUsage, kDefaultAttribs, {{data, 4}},
                                              &source, &image);

    // Create a texture target to bind the egl image & disable mipmapping
    GLTexture target;
    createEGLImageTargetTextureExternal(image, target);

    // Calls On EndFrame(), with MidExecutionSetup to restore external target texture above
    EGLDisplay display = getEGLWindow()->getDisplay();
    EGLSurface surface = getEGLWindow()->getSurface();
    eglSwapBuffers(display, surface);

    // Create another eglImage with another associated texture
    // Draw using the eglImage target texture created in frame 1
    AHardwareBuffer *source2;
    EGLImageKHR image2;
    createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                              kDefaultAHBUsage, kDefaultAttribs, {{data, 4}},
                                              &source2, &image2);

    // Create another texture target to bind the egl image & disable mipmapping
    GLTexture target2;
    createEGLImageTargetTextureExternal(image, target2);

    glUseProgram(mTextureExternalProgram);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, target);
    glUniform1i(mTextureExternalUniformLocation, 0);

    drawQuad(mTextureExternalProgram, "position", 0.5f);

    // Calls On EndFrame() to save the gl calls creating external texture target2;
    // We use this as a reference to check the gl calls we restore for GLTexture target
    // in MidExecutionSetup
    eglSwapBuffers(display, surface);

    // Draw a quad with the GLTexture target2
    glUseProgram(mTextureExternalProgram);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, target2);
    glUniform1i(mTextureExternalUniformLocation, 0);

    drawQuad(mTextureExternalProgram, "position", 0.5f);

    eglSwapBuffers(display, surface);

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
    eglDestroyImageKHR(window->getDisplay(), image2);
}

// Thread 0 creates the AHB and binds it to a texture, thread 1 uses it without synchronization.
TEST_P(ImageTest, MultithreadedAHBImportAndUseAsTexture)
{
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    EGLWindow *window = getEGLWindow();

    GLuint sharedTexture = 0;

    // Create the Image
    AHardwareBuffer *source;
    EGLImageKHR image;

    constexpr GLubyte kInitialData[4] = {127, 63, 191, 255};

    std::mutex mutex;
    std::condition_variable condVar;

    enum class Step
    {
        Start,
        Thread0CreatedTexture,
        Thread1UsedTexture,
        Finish,
        Abort,
    };
    Step currentStep = Step::Start;

    auto thread0 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                                  kDefaultAHBUsage, kDefaultAttribs,
                                                  {{kInitialData, 4}}, &source, &image);
        ASSERT_GL_NO_ERROR();

        GLTexture texture;
        sharedTexture = texture;

        createEGLImageTargetTextureExternal(image, sharedTexture);
        ASSERT_GL_NO_ERROR();

        // Wait for the other thread to use it.
        threadSynchronization.nextStep(Step::Thread0CreatedTexture);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread1UsedTexture));

        eglDestroyImageKHR(window->getDisplay(), image);
        texture.reset();

        threadSynchronization.nextStep(Step::Finish);

        // Clean up
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    };

    auto thread1 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        // Wait for thread 0 to set up
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread0CreatedTexture));

        // Sample from the texture
        ANGLE_GL_PROGRAM(drawTexture, getVS(), getTextureExternalFS());
        glUseProgram(drawTexture);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, sharedTexture);
        glUniform1i(mTextureExternalUniformLocation, 0);
        ASSERT_GL_NO_ERROR();

        drawQuad(drawTexture, "position", 0.5f);
        ASSERT_GL_NO_ERROR();

        // Make a submission
        EXPECT_PIXEL_COLOR_NEAR(
            0, 0, GLColor(kInitialData[0], kInitialData[1], kInitialData[2], kInitialData[3]), 1);

        // Notify the other thread that it's finished using the texture.
        threadSynchronization.nextStep(Step::Thread1UsedTexture);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Finish));

        // Clean up
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    };

    std::array<LockStepThreadFunc, 2> threadFuncs = {
        std::move(thread0),
        std::move(thread1),
    };

    RunLockStepThreads(getEGLWindow(), threadFuncs.size(), threadFuncs.data());

    ASSERT_NE(currentStep, Step::Abort);
}

// Thread 0 creates the AHB and binds it to a renderbuffer, thread 1 uses it without
// synchronization.
TEST_P(ImageTest, MultithreadedAHBImportAndUseAsRenderbuffer)
{
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !hasRenderbufferExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!hasAhbLockPlanesSupport());
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    EGLWindow *window = getEGLWindow();

    GLuint sharedRenderbuffer = 0;

    // Create the Image
    AHardwareBuffer *source;
    EGLImageKHR image;

    constexpr GLubyte kInitialData[4] = {127, 63, 191, 255};

    std::mutex mutex;
    std::condition_variable condVar;

    enum class Step
    {
        Start,
        Thread0CreatedRenderbuffer,
        Thread1UsedRenderbuffer,
        Finish,
        Abort,
    };
    Step currentStep = Step::Start;

    auto thread0 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        createEGLImageAndroidHardwareBufferSource(1, 1, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                                                  kDefaultAHBUsage, kDefaultAttribs,
                                                  {{kInitialData, 4}}, &source, &image);
        ASSERT_GL_NO_ERROR();

        GLRenderbuffer renderbuffer;
        sharedRenderbuffer = renderbuffer;

        createEGLImageTargetRenderbuffer(image, sharedRenderbuffer);
        ASSERT_GL_NO_ERROR();

        // Wait for the other thread to use it.
        threadSynchronization.nextStep(Step::Thread0CreatedRenderbuffer);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread1UsedRenderbuffer));

        eglDestroyImageKHR(window->getDisplay(), image);
        renderbuffer.reset();

        threadSynchronization.nextStep(Step::Finish);

        // Clean up
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    };

    auto thread1 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        // Wait for thread 0 to set up
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread0CreatedRenderbuffer));

        // Blend into the renderbuffer
        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                  sharedRenderbuffer);
        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
        ASSERT_GL_NO_ERROR();

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        ANGLE_GL_PROGRAM(drawRed, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
        drawQuad(drawRed, essl1_shaders::PositionAttrib(), 0.5f);
        ASSERT_GL_NO_ERROR();

        // Make a submission
        EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(255, kInitialData[1], kInitialData[2], 255), 1);

        // Notify the other thread that it's finished using the renderbuffer.
        threadSynchronization.nextStep(Step::Thread1UsedRenderbuffer);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Finish));

        // Clean up
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    };

    std::array<LockStepThreadFunc, 2> threadFuncs = {
        std::move(thread0),
        std::move(thread1),
    };

    RunLockStepThreads(getEGLWindow(), threadFuncs.size(), threadFuncs.data());

    ASSERT_NE(currentStep, Step::Abort);
}

void ImageTest::framebufferAttachmentDeletedWhileInUseHelper(bool useTextureAttachment,
                                                             bool deleteSourceTextureLast)
{
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt());
    ANGLE_SKIP_TEST_IF(useTextureAttachment && !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!useTextureAttachment && !hasRenderbufferExt());
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    EGLWindow *window = getEGLWindow();
    EGLImageKHR image = EGL_NO_IMAGE_KHR;

    std::mutex mutex;
    std::condition_variable condVar;
    EGLSyncKHR sync;

    enum class Step
    {
        Start,
        Thread0CreatedImage,
        Thread1UsedImage,
        Finish,
        Abort,
    };
    Step currentStep = Step::Start;

    EXPECT_EGL_TRUE(window->makeCurrent(EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));

    // This thread will use window context
    std::thread thread0([&]() {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        window->makeCurrent();

        // Create the Image
        GLTexture source;
        createEGLImage2DTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, kDefaultAttribs,
                                      kLinearColor, source, &image);

        sync = eglCreateSyncKHR(window->getDisplay(), EGL_SYNC_FENCE_KHR, nullptr);
        ASSERT_GL_NO_ERROR();

        // Wait thread 1 finish using the Image
        threadSynchronization.nextStep(Step::Thread0CreatedImage);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread1UsedImage));

        if (!deleteSourceTextureLast)
        {
            // Delete "source" texture first - image buffer will be deleted with the "image"
            source.reset();
        }

        // Destroy Image
        eglDestroyImageKHR(window->getDisplay(), image);

        if (deleteSourceTextureLast)
        {
            // Delete "source" texture last - this will delete image buffer
            source.reset();
        }

        threadSynchronization.nextStep(Step::Finish);

        EXPECT_EGL_TRUE(window->makeCurrent(EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    });

    // This thread will use non Shared context
    auto thread1 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread0CreatedImage));

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));
        eglWaitSyncKHR(dpy, sync, 0);

        // Create the target and set up a framebuffer to render into the Image
        GLFramebuffer fbo;
        GLTexture targetTexture;
        GLRenderbuffer targetRenderbuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        if (useTextureAttachment)
        {
            createEGLImageTargetTexture2D(image, targetTexture);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                   targetTexture, 0);
        }
        else
        {
            createEGLImageTargetRenderbuffer(image, targetRenderbuffer);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                      targetRenderbuffer);
        }
        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        // Test that framebuffer has source content
        EXPECT_PIXEL_EQ(0, 0, kLinearColor[0], kLinearColor[1], kLinearColor[2], kLinearColor[3]);

        // Create additional target texture
        GLTexture targetTexture2;
        createEGLImageTargetTexture2D(image, targetTexture2);

        // Enable additive blend
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        // Draw Red quad into the framebuffer
        ANGLE_GL_PROGRAM(drawRed, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
        drawQuad(drawRed, essl1_shaders::PositionAttrib(), 0.0f);
        ASSERT_GL_NO_ERROR();

        // Delete "targetTexture2" that may affect RenderPass because it uses same Image
        targetTexture2.reset();

        // Clear previous draw
        glClearColor(128 / 255.0f, 128 / 255.0f, 128 / 255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw Green quad into the framebuffer
        ANGLE_GL_PROGRAM(drawGreen, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
        drawQuad(drawGreen, essl1_shaders::PositionAttrib(), 0.0f);
        ASSERT_GL_NO_ERROR();

        // Test that clear and second draw worked as expected
        EXPECT_PIXEL_EQ(0, 0, 128, 255, 128, 255);

        // Draw again to open RenderPass after the read pixels
        drawQuad(drawGreen, essl1_shaders::PositionAttrib(), 0.0f);
        ASSERT_GL_NO_ERROR();

        // Delete resources
        fbo.reset();
        targetTexture.reset();
        targetRenderbuffer.reset();
        ASSERT_GL_NO_ERROR();

        // Wait thread 0 destroys the Image and source
        threadSynchronization.nextStep(Step::Thread1UsedImage);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Finish));

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    };

    std::array<LockStepThreadFunc, 1> threadFuncs = {
        std::move(thread1),
    };

    RunLockStepThreads(getEGLWindow(), threadFuncs.size(), threadFuncs.data());
    thread0.join();

    window->makeCurrent();

    ASSERT_NE(currentStep, Step::Abort);
}

// Testing Target 2D Texture deleted while still used in the RenderPass (Image destroyed last).
TEST_P(ImageTest, TargetTexture2DDeletedWhileInUse)
{
    framebufferAttachmentDeletedWhileInUseHelper(true, false);
}

// Testing Target 2D Texture deleted while still used in the RenderPass (Source deleted last).
TEST_P(ImageTest, TargetTexture2DDeletedWhileInUse2)
{
    framebufferAttachmentDeletedWhileInUseHelper(true, true);
}

// Testing Target Renderbuffer deleted while still used in the RenderPass (Image destroyed last).
TEST_P(ImageTest, TargetRenderbufferDeletedWhileInUse)
{
    framebufferAttachmentDeletedWhileInUseHelper(false, false);
}

// Testing Target Renderbuffer deleted while still used in the RenderPass (Source deleted last).
TEST_P(ImageTest, TargetRenderbufferDeletedWhileInUse2)
{
    framebufferAttachmentDeletedWhileInUseHelper(false, true);
}

void ImageTest::framebufferResolveAttachmentDeletedWhileInUseHelper(bool useTextureAttachment,
                                                                    bool deleteSourceTextureLast)
{
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt());
    ANGLE_SKIP_TEST_IF(useTextureAttachment && !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!useTextureAttachment && !hasRenderbufferExt());
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    EGLWindow *window = getEGLWindow();
    EGLImageKHR image = EGL_NO_IMAGE_KHR;

    std::mutex mutex;
    std::condition_variable condVar;
    EGLSyncKHR sync;

    enum class Step
    {
        Start,
        Thread0CreatedImage,
        Thread1UsedImage,
        Finish,
        Abort,
    };
    Step currentStep = Step::Start;

    EXPECT_EGL_TRUE(window->makeCurrent(EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));

    // This thread will use window context
    std::thread thread0([&]() {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        window->makeCurrent();

        // Create the Image
        GLTexture source;
        createEGLImage2DTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, kDefaultAttribs,
                                      kLinearColor, source, &image);

        sync = eglCreateSyncKHR(window->getDisplay(), EGL_SYNC_FENCE_KHR, nullptr);
        ASSERT_GL_NO_ERROR();

        // Wait thread 1 finish using the Image
        threadSynchronization.nextStep(Step::Thread0CreatedImage);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread1UsedImage));

        if (!deleteSourceTextureLast)
        {
            // Delete "source" texture first - image buffer will be deleted with the "image"
            source.reset();
        }

        // Destroy Image
        eglDestroyImageKHR(window->getDisplay(), image);

        if (deleteSourceTextureLast)
        {
            // Delete "source" texture last - this will delete image buffer
            source.reset();
        }

        threadSynchronization.nextStep(Step::Finish);

        EXPECT_EGL_TRUE(window->makeCurrent(EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    });

    // This thread will use non Shared context
    auto thread1 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread0CreatedImage));

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));
        eglWaitSyncKHR(dpy, sync, 0);

        // Create the target and set up a framebuffer to render into the Image
        GLFramebuffer fbo;
        GLTexture targetTexture;
        GLRenderbuffer targetRenderbuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        if (useTextureAttachment)
        {
            createEGLImageTargetTexture2D(image, targetTexture);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                   targetTexture, 0);
        }
        else
        {
            createEGLImageTargetRenderbuffer(image, targetRenderbuffer);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                      targetRenderbuffer);
        }
        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        // Test that framebuffer has source content
        EXPECT_PIXEL_EQ(0, 0, kLinearColor[0], kLinearColor[1], kLinearColor[2], kLinearColor[3]);

        // Create additional target texture
        GLTexture targetTexture2;
        createEGLImageTargetTexture2D(image, targetTexture2);

        // Create MSAA framebuffer
        GLTexture msaaColor;
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaaColor);
        glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, 1, 1, false);

        GLFramebuffer msaaFBO;
        glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
                               msaaColor, 0);
        ASSERT_GL_NO_ERROR();
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        // Draw Red quad into the MSAA framebuffer
        ANGLE_GL_PROGRAM(drawRed, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
        drawQuad(drawRed, essl1_shaders::PositionAttrib(), 0.0f);
        ASSERT_GL_NO_ERROR();

        // Resolve into image
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
        glBlitFramebuffer(0, 0, 1, 1, 0, 0, 1, 1, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        ASSERT_GL_NO_ERROR();

        // Delete "targetTexture2" that may affect RenderPass because it uses same Image
        targetTexture2.reset();

        // Start another render pass and blend into the image.
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        ANGLE_GL_PROGRAM(drawGreen, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
        drawQuad(drawGreen, essl1_shaders::PositionAttrib(), 0.0f);
        ASSERT_GL_NO_ERROR();

        // Test that resolve and draw worked
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::yellow);

        // Draw again to open RenderPass after the read pixels
        glDisable(GL_BLEND);
        drawQuad(drawGreen, essl1_shaders::PositionAttrib(), 0.0f);
        ASSERT_GL_NO_ERROR();

        // Delete resources
        fbo.reset();
        targetTexture.reset();
        targetRenderbuffer.reset();
        ASSERT_GL_NO_ERROR();

        // Wait thread 0 destroys the Image and source
        threadSynchronization.nextStep(Step::Thread1UsedImage);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Finish));

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    };

    std::array<LockStepThreadFunc, 1> threadFuncs = {
        std::move(thread1),
    };

    RunLockStepThreads(getEGLWindow(), threadFuncs.size(), threadFuncs.data());
    thread0.join();

    window->makeCurrent();

    ASSERT_NE(currentStep, Step::Abort);
}

// Testing Target 2D Texture deleted while still used in the RenderPass as resolve attachment (Image
// destroyed last).
TEST_P(ImageTestES31, TargetTexture2DDeletedWhileInUseAsResolve)
{
    framebufferResolveAttachmentDeletedWhileInUseHelper(true, false);
}

// Testing Target 2D Texture deleted while still used in the RenderPass as resolve attachment
// (Source deleted last).
TEST_P(ImageTestES31, TargetTexture2DDeletedWhileInUseAsResolve2)
{
    framebufferResolveAttachmentDeletedWhileInUseHelper(true, true);
}

// Testing Target Renderbuffer deleted while still used in the RenderPass as resolve attachment
// (Image destroyed last).
TEST_P(ImageTestES31, TargetRenderbufferDeletedWhileInUseAsResolve)
{
    framebufferResolveAttachmentDeletedWhileInUseHelper(false, false);
}

// Testing Target Renderbuffer deleted while still used in the RenderPass as resolve attachment
// (Source deleted last).
TEST_P(ImageTestES31, TargetRenderbufferDeletedWhileInUseAsResolve2)
{
    framebufferResolveAttachmentDeletedWhileInUseHelper(false, true);
}

// Test that the AHB can be used in some way in GL, then accessed by foreign entity, then used again
// by GL.  This makes sure transitions in and out of the FOREIGN Vulkan queue are correctly
// implemented.
void ImageTest::useAHBByGLThenForeignThenGLHelper(
    std::function<void(const GLTexture &, uint32_t, uint32_t)> firstUse,
    std::function<void(const GLTexture &, uint32_t, uint32_t)> secondUse)
{
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !hasRenderbufferExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());

    constexpr uint32_t kWidth  = 53;
    constexpr uint32_t kHeight = 37;

    // Create the Image
    AHardwareBuffer *source;
    EGLImageKHR image;
    createEGLImageAndroidHardwareBufferSource(
        kWidth, kHeight, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM, kDefaultAHBUsage,
        kDefaultAttribs, {}, &source, &image);

    // Create a texture target to bind the egl image
    GLTexture target;
    createEGLImageTargetTexture2D(image, target);

    glViewport(0, 0, kWidth, kHeight);

    // Use the image in GL once.
    firstUse(target, kWidth, kHeight);

    // In between uses, overwrite parts of the image.
    // First, wait for first use to finish.
    EGLWindow *window = getEGLWindow();
    EGLSyncKHR fence  = eglCreateSyncKHR(window->getDisplay(), EGL_SYNC_FENCE_KHR, NULL);
    glFlush();
    EGLint result = eglClientWaitSyncKHR(window->getDisplay(), fence, 0, 1'000'000'000);
    ASSERT_EQ(result, EGL_CONDITION_SATISFIED_KHR);
    ASSERT_EGL_TRUE(eglDestroySyncKHR(window->getDisplay(), fence));

    // Then overwrite a quarter of the image with some color
    std::vector<GLubyte> data((kWidth / 2) * (kHeight / 2) * 4, 99);
    writeAHBData(source, kWidth / 2, kHeight / 2, 1, false, {{data.data(), 4}});

    // And use the image again in GL.
    secondUse(target, kWidth, kHeight);

    ASSERT_GL_NO_ERROR();
}

// Test draw, use in foreign, then draw again
TEST_P(ImageTest, DrawForeignDraw)
{
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    auto first = [&](const GLTexture &texture, uint32_t width, uint32_t height) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.0f);
    };
    auto second = [&](const GLTexture &texture, uint32_t width, uint32_t height) {
        glBindTexture(GL_TEXTURE_2D, texture);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        glEnable(GL_SCISSOR_TEST);
        glScissor(width / 4, 0, width / 2, height);

        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.0f);

        // Expect the following:
        //
        // +-----+-----+-----+-----+
        // |     |     |     |     |
        // | 99  | 99  | G   | G   |
        // |     | +   | +   |     |
        // |     | R   | R   |     |
        // |     |     |     |     |
        // +-----+-----+-----+     |
        // |     |           |     |
        // | G   | G + R     | G   |
        // |     |           |     |
        // |     |           |     |
        // |     |           |     |
        // +-----+-----------+-----+
        //
        EXPECT_PIXEL_RECT_EQ(0, 0, width / 4, height / 2, GLColor(99, 99, 99, 99));
        EXPECT_PIXEL_RECT_EQ(0, height / 2, width / 4, height - height / 2, GLColor::green);
        EXPECT_PIXEL_RECT_EQ(width / 4, 0, width / 2 - width / 4, height / 2,
                             GLColor(255, 99, 99, 255));
        EXPECT_PIXEL_RECT_EQ(width / 4, height / 2, width / 2, height - height / 2,
                             GLColor::yellow);
        EXPECT_PIXEL_RECT_EQ(width / 2, 0, width / 2 - width / 4, height / 2, GLColor::yellow);
        EXPECT_PIXEL_RECT_EQ(width / 4 + width / 2, 0, width - width / 2 - width / 4, height,
                             GLColor::green);
    };

    useAHBByGLThenForeignThenGLHelper(first, second);
}

// Test draw, use in foreign, then blit
TEST_P(ImageTestES3, DrawForeignBlit)
{
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    auto first = [&](const GLTexture &texture, uint32_t width, uint32_t height) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.0f);
    };
    auto second = [&](const GLTexture &texture, uint32_t width, uint32_t height) {
        GLFramebuffer otherFbo;
        glBindFramebuffer(GL_FRAMEBUFFER, otherFbo);

        GLTexture color;
        glBindTexture(GL_TEXTURE_2D, color);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        glClearColor(0, 0, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindTexture(GL_TEXTURE_2D, texture);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

        glBlitFramebuffer(width / 4, 0, width / 2 + width / 4, height, width / 4, 0,
                          width / 2 + width / 4, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);

        // Expect the following:
        //
        // +-----+-----------+-----+
        // |     |           |     |
        // | 99  | B         | G   |
        // |     |           |     |
        // |     |           |     |
        // |     |           |     |
        // +-----+           |     |
        // |     |           |     |
        // | G   |           |     |
        // |     |           |     |
        // |     |           |     |
        // |     |           |     |
        // +-----+-----------+-----+
        //
        EXPECT_PIXEL_RECT_EQ(0, 0, width / 4, height / 2, GLColor(99, 99, 99, 99));
        EXPECT_PIXEL_RECT_EQ(0, height / 2, width / 4, height - height / 2, GLColor::green);
        EXPECT_PIXEL_RECT_EQ(width / 4, 0, width / 2 - width / 4, height, GLColor::blue);
        EXPECT_PIXEL_RECT_EQ(width / 4 + width / 2, 0, width - width / 2 - width / 4, height,
                             GLColor::green);
    };

    useAHBByGLThenForeignThenGLHelper(first, second);
}

// Test draw, readback, use in foreign, then draw again
TEST_P(ImageTest, DrawReadbackForeignDraw)
{
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    auto first = [&](const GLTexture &texture, uint32_t width, uint32_t height) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.0f);

        // A second usage of the same image
        EXPECT_PIXEL_RECT_EQ(0, 0, width, height, GLColor::green);
    };
    auto second = [&](const GLTexture &texture, uint32_t width, uint32_t height) {
        glBindTexture(GL_TEXTURE_2D, texture);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        glEnable(GL_SCISSOR_TEST);
        glScissor(width / 4, 0, width / 2, height);

        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.0f);

        // Expect the following:
        //
        // +-----+-----+-----+-----+
        // |     |     |     |     |
        // | 99  | 99  | G   | G   |
        // |     | +   | +   |     |
        // |     | R   | R   |     |
        // |     |     |     |     |
        // +-----+-----+-----+     |
        // |     |           |     |
        // | G   | G + R     | G   |
        // |     |           |     |
        // |     |           |     |
        // |     |           |     |
        // +-----+-----------+-----+
        //
        EXPECT_PIXEL_RECT_EQ(0, 0, width / 4, height / 2, GLColor(99, 99, 99, 99));
        EXPECT_PIXEL_RECT_EQ(0, height / 2, width / 4, height - height / 2, GLColor::green);
        EXPECT_PIXEL_RECT_EQ(width / 4, 0, width / 2 - width / 4, height / 2,
                             GLColor(255, 99, 99, 255));
        EXPECT_PIXEL_RECT_EQ(width / 4, height / 2, width / 2, height - height / 2,
                             GLColor::yellow);
        EXPECT_PIXEL_RECT_EQ(width / 2, 0, width / 2 - width / 4, height / 2, GLColor::yellow);
        EXPECT_PIXEL_RECT_EQ(width / 4 + width / 2, 0, width - width / 2 - width / 4, height,
                             GLColor::green);
    };

    useAHBByGLThenForeignThenGLHelper(first, second);
}

// Test draw, use in foreign, then readback
TEST_P(ImageTest, DrawForeignReadback)
{
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    auto first = [&](const GLTexture &texture, uint32_t width, uint32_t height) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.0f);
    };
    auto second = [&](const GLTexture &texture, uint32_t width, uint32_t height) {
        glBindTexture(GL_TEXTURE_2D, texture);

        // Expect the following:
        //
        // +-----------+-----------+
        // |           |           |
        // | 99        |           |
        // |           |           |
        // |           |           |
        // |           |           |
        // +-----------+           |
        // |                       |
        // | G                     |
        // |                       |
        // |                       |
        // |                       |
        // +-----------------------+
        //
        EXPECT_PIXEL_RECT_EQ(0, 0, width / 2, height / 2, GLColor(99, 99, 99, 99));
        EXPECT_PIXEL_RECT_EQ(width / 2, 0, width - width / 2, height / 2, GLColor::green);
        EXPECT_PIXEL_RECT_EQ(0, height / 2, width, height - height / 2, GLColor::green);
    };

    useAHBByGLThenForeignThenGLHelper(first, second);
}

// Test use as resolve attachment, use in foreign, then draw again
TEST_P(ImageTestES3, ResolveForeignDraw)
{
    GLFramebuffer fbo;

    auto first = [&](const GLTexture &texture, uint32_t width, uint32_t height) {
        GLRenderbuffer color;
        glBindRenderbuffer(GL_RENDERBUFFER, color);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, width, height);

        GLFramebuffer msaaFbo;
        glBindFramebuffer(GL_FRAMEBUFFER, msaaFbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.0f);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture,
                               0);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);

        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT,
                          GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    };
    auto second = [&](const GLTexture &texture, uint32_t width, uint32_t height) {
        glBindTexture(GL_TEXTURE_2D, texture);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        glEnable(GL_SCISSOR_TEST);
        glScissor(width / 4, 0, width / 2, height);

        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.0f);

        // Expect the following:
        //
        // +-----+-----+-----+-----+
        // |     |     |     |     |
        // | 99  | 99  | G   | G   |
        // |     | +   | +   |     |
        // |     | R   | R   |     |
        // |     |     |     |     |
        // +-----+-----+-----+     |
        // |     |           |     |
        // | G   | G + R     | G   |
        // |     |           |     |
        // |     |           |     |
        // |     |           |     |
        // +-----+-----------+-----+
        //
        EXPECT_PIXEL_RECT_EQ(0, 0, width / 4, height / 2, GLColor(99, 99, 99, 99));
        EXPECT_PIXEL_RECT_EQ(0, height / 2, width / 4, height - height / 2, GLColor::green);
        EXPECT_PIXEL_RECT_EQ(width / 4, 0, width / 2 - width / 4, height / 2,
                             GLColor(255, 99, 99, 255));
        EXPECT_PIXEL_RECT_EQ(width / 4, height / 2, width / 2, height - height / 2,
                             GLColor::yellow);
        EXPECT_PIXEL_RECT_EQ(width / 2, 0, width / 2 - width / 4, height / 2, GLColor::yellow);
        EXPECT_PIXEL_RECT_EQ(width / 4 + width / 2, 0, width - width / 2 - width / 4, height,
                             GLColor::green);
    };

    useAHBByGLThenForeignThenGLHelper(first, second);
}

// Test upload, use in foreign, then draw
TEST_P(ImageTest, UploadForeignDraw)
{
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    auto first = [&](const GLTexture &texture, uint32_t width, uint32_t height) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        std::vector<GLColor> data(width * height, GLColor::blue);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE,
                        data.data());

        EXPECT_PIXEL_RECT_EQ(0, 0, width, height, GLColor::blue);
    };
    auto second = [&](const GLTexture &texture, uint32_t width, uint32_t height) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        glEnable(GL_SCISSOR_TEST);
        glScissor(width / 4, 0, width / 2, height);

        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.0f);

        // Expect the following:
        //
        // +-----+-----+-----+-----+
        // |     |     |     |     |
        // | 99  | 99  | B   | B   |
        // |     | +   | +   |     |
        // |     | R   | R   |     |
        // |     |     |     |     |
        // +-----+-----+-----+     |
        // |     |           |     |
        // | B   | B + R     | B   |
        // |     |           |     |
        // |     |           |     |
        // |     |           |     |
        // +-----+-----------+-----+
        //
        EXPECT_PIXEL_RECT_EQ(0, 0, width / 4, height / 2, GLColor(99, 99, 99, 99));
        EXPECT_PIXEL_RECT_EQ(0, height / 2, width / 4, height - height / 2, GLColor::blue);
        EXPECT_PIXEL_RECT_EQ(width / 4, 0, width / 2 - width / 4, height / 2,
                             GLColor(255, 99, 99, 255));
        EXPECT_PIXEL_RECT_EQ(width / 4, height / 2, width / 2, height - height / 2,
                             GLColor::magenta);
        EXPECT_PIXEL_RECT_EQ(width / 2, 0, width / 2 - width / 4, height / 2, GLColor::magenta);
        EXPECT_PIXEL_RECT_EQ(width / 4 + width / 2, 0, width - width / 2 - width / 4, height,
                             GLColor::blue);
    };

    useAHBByGLThenForeignThenGLHelper(first, second);
}

// Test sample in FS, use in foreign, then sample again in VS
TEST_P(ImageTestES3, SampleForeignSample)
{
    GLTexture color;

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    auto first = [&](const GLTexture &texture, uint32_t width, uint32_t height) {
        glBindTexture(GL_TEXTURE_2D, color);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        std::vector<GLColor> data(width * height, GLColor::blue);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE,
                        data.data());

        ANGLE_GL_PROGRAM(drawTexture, essl1_shaders::vs::Texture2D(),
                         essl1_shaders::fs::Texture2D());
        glUseProgram(drawTexture);
        GLint texLocation = glGetUniformLocation(drawTexture, essl1_shaders::Texture2DUniform());
        ASSERT_NE(-1, texLocation);
        glUniform1i(texLocation, 0);
        drawQuad(drawTexture, essl1_shaders::PositionAttrib(), 0.5f);

        EXPECT_PIXEL_RECT_EQ(0, 0, width, height, GLColor::blue);
    };
    auto second = [&](const GLTexture &texture, uint32_t width, uint32_t height) {
        glBindTexture(GL_TEXTURE_2D, texture);

        constexpr char kVS[] = R"(precision highp float;
uniform highp sampler2D tex;
attribute vec2 position;
varying vec4 color;
void main()
{
    color = texture2D(tex, position * 0.5 + 0.5);
    gl_Position = vec4(position, 0, 1);
})";

        constexpr char kFS[] = R"(precision highp float;
varying vec4 color;
void main()
{
    gl_FragColor = color;
})";

        ANGLE_GL_PROGRAM(program, kVS, kFS);
        glUseProgram(program);
        GLint texLocation = glGetUniformLocation(program, "tex");
        ASSERT_NE(-1, texLocation);
        glUniform1i(texLocation, 0);

        const std::array<Vector3, 12> kVertices = {{
            Vector3(-1.0f, -1.0f, 0.5f),
            Vector3(-1.0f, -0.1f, 0.5f),
            Vector3(-0.1f, -0.1f, 0.5f),

            Vector3(-1.0f, -1.0f, 0.5f),
            Vector3(-0.1f, -0.1f, 0.5f),
            Vector3(-0.1f, -1.0f, 0.5f),

            Vector3(0.1f, -1.0f, 0.5f),
            Vector3(0.1f, -0.1f, 0.5f),
            Vector3(1.0f, -0.1f, 0.5f),

            Vector3(0.1f, -1.0f, 0.5f),
            Vector3(1.0f, -0.1f, 0.5f),
            Vector3(1.0f, -1.0f, 0.5f),
        }};

        GLint positionLocation = glGetAttribLocation(program, "position");
        ASSERT_NE(-1, positionLocation);
        glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, kVertices.data());
        glEnableVertexAttribArray(positionLocation);

        glClearColor(0, 255, 0, 255);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 12);

        // Expect the following:
        //
        // +---------+---+---------+
        // |         |   |         |
        // | 99      |   | B       |
        // |         |   |         |
        // |         |   |         |
        // |---------+   +---------|
        // |                       |
        // |                       |
        // | G                     |
        // |                       |
        // |                       |
        // |                       |
        // +-----------------------+
        //
        EXPECT_PIXEL_RECT_EQ(0, 0, width / 2 - width / 10 - 1, height / 2 - height / 10 - 1,
                             GLColor(99, 99, 99, 99));
        EXPECT_PIXEL_RECT_EQ(width / 2 + width / 10 + 1, 0, width - width / 2 - width / 10 - 1,
                             height / 2 - 1, GLColor::blue);
        EXPECT_PIXEL_RECT_EQ(0, height / 2, width, height - height / 2, GLColor::green);
    };

    useAHBByGLThenForeignThenGLHelper(first, second);
}

// Test draw, use in foreign, readback in another context, use in foreign, then sample in the
// original context.
TEST_P(ImageTestES3, MultithreadedDrawForeignReadbackForeignSample)
{
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!hasAndroidImageNativeBufferExt() || !hasAndroidHardwareBufferSupport());
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    EGLWindow *window = getEGLWindow();

    constexpr uint32_t kWidth  = 53;
    constexpr uint32_t kHeight = 37;

    // Create the Image
    AHardwareBuffer *source;
    EGLImageKHR image;
    createEGLImageAndroidHardwareBufferSource(
        kWidth, kHeight, 1, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM, kDefaultAHBUsage,
        kDefaultAttribs, {}, &source, &image);

    std::mutex mutex;
    std::condition_variable condVar;

    enum class Step
    {
        Start,
        Thread0DrawDone,
        Thread1ReadbackDone,
        Finish,
        Abort,
    };
    Step currentStep = Step::Start;

    auto thread0 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        // Create a texture target to bind the egl image
        GLTexture target;
        createEGLImageTargetTexture2D(image, target);
        ASSERT_GL_NO_ERROR();

        // Draw into the image in this context.
        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target, 0);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        glViewport(0, 0, kWidth, kHeight);
        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.0f);
        ASSERT_GL_NO_ERROR();

        EGLSyncKHR fence = eglCreateSyncKHR(window->getDisplay(), EGL_SYNC_FENCE_KHR, NULL);
        glFlush();
        EGLint result = eglClientWaitSyncKHR(window->getDisplay(), fence, 0, 1'000'000'000);
        ASSERT_EQ(result, EGL_CONDITION_SATISFIED_KHR);
        ASSERT_EGL_TRUE(eglDestroySyncKHR(window->getDisplay(), fence));

        // Then overwrite a quarter of the image with some color
        std::vector<GLubyte> data((kWidth / 2) * (kHeight / 2) * 4, 99);
        writeAHBData(source, kWidth / 2, kHeight / 2, 1, false, {{data.data(), 4}});

        // Wait for the other thread to read back from it.
        threadSynchronization.nextStep(Step::Thread0DrawDone);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread1ReadbackDone));

        // Create a different framebuffer to render to.
        GLTexture color;
        glBindTexture(GL_TEXTURE_2D, color);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        // Sample from the image.
        glBindTexture(GL_TEXTURE_2D, target);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        ANGLE_GL_PROGRAM(drawTexture, essl1_shaders::vs::Texture2D(),
                         essl1_shaders::fs::Texture2D());
        glUseProgram(drawTexture);
        GLint texLocation = glGetUniformLocation(drawTexture, essl1_shaders::Texture2DUniform());
        ASSERT_NE(-1, texLocation);
        glUniform1i(texLocation, 0);
        drawQuad(drawTexture, essl1_shaders::PositionAttrib(), 0.5f);

        // Expect the following:
        //
        // +-----+-----+-----+-----+
        // |     |     |           |
        // | 123 | 99  |  G        |
        // |     |     |           |
        // |     |     |           |
        // |     |     |           |
        // |     +-----+           |
        // |     |                 |
        // | 123 |                 |
        // |     |                 |
        // |     |                 |
        // |     |                 |
        // +-----+-----------+-----+
        //
        EXPECT_PIXEL_RECT_EQ(0, 0, kWidth / 4, kHeight, GLColor(123, 123, 123, 123));
        EXPECT_PIXEL_RECT_EQ(kWidth / 4, 0, kWidth / 2 - kWidth / 4, kHeight / 2,
                             GLColor(99, 99, 99, 99));
        EXPECT_PIXEL_RECT_EQ(kWidth / 4, kHeight / 2, kWidth / 2 - kWidth / 4,
                             kHeight - kHeight / 2, GLColor::green);
        EXPECT_PIXEL_RECT_EQ(kWidth / 2, 0, kWidth - kWidth / 2, kHeight, GLColor::green);

        eglDestroyImageKHR(window->getDisplay(), image);

        threadSynchronization.nextStep(Step::Finish);

        // Clean up
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    };

    auto thread1 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        // Create a texture target to bind the egl image
        GLTexture target;
        createEGLImageTargetTexture2D(image, target);

        // Wait for thread 0 to set up, and rebind the texture.
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread0DrawDone));
        glBindTexture(GL_TEXTURE_2D, target);

        // Expect the following:
        //
        // +-----------+-----------+
        // |           |           |
        // | 99        |           |
        // |           |           |
        // |           |           |
        // |           |           |
        // +-----------+           |
        // |                       |
        // | G                     |
        // |                       |
        // |                       |
        // |                       |
        // +-----------------------+
        //
        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target, 0);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        EXPECT_PIXEL_RECT_EQ(0, 0, kWidth / 2, kHeight / 2, GLColor(99, 99, 99, 99));
        EXPECT_PIXEL_RECT_EQ(kWidth / 2, 0, kWidth - kWidth / 2, kHeight / 2, GLColor::green);
        EXPECT_PIXEL_RECT_EQ(0, kHeight / 2, kWidth, kHeight - kHeight / 2, GLColor::green);

        // Then overwrite parts of the image with another color
        std::vector<GLubyte> data((kWidth / 4) * kHeight * 4, 123);
        writeAHBData(source, kWidth / 4, kHeight, 1, false, {{data.data(), 4}});

        // Notify the other thread that it's finished reading back the texture.
        threadSynchronization.nextStep(Step::Thread1ReadbackDone);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Finish));

        // Clean up
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    };

    std::array<LockStepThreadFunc, 2> threadFuncs = {
        std::move(thread0),
        std::move(thread1),
    };

    RunLockStepThreads(getEGLWindow(), threadFuncs.size(), threadFuncs.data());

    ASSERT_NE(currentStep, Step::Abort);
}

// Test redefining the same GL texture with different EGLImages
TEST_P(ImageTest, RedefineWithMultipleImages)
{
    EGLWindow *window = getEGLWindow();

    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());

    GLubyte originalData[4] = {255, 0, 255, 255};
    GLubyte updateData[4]   = {0, 255, 0, 255};

    // Create the Images
    GLTexture source1, source2;
    EGLImageKHR image1, image2;
    createEGLImage2DTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, kDefaultAttribs, originalData,
                                  source1, &image1);
    createEGLImage2DTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, kDefaultAttribs, originalData,
                                  source2, &image2);

    // Create texture & bind to Image
    GLTexture texture;
    createEGLImageTargetTexture2D(image1, texture);

    // Upload some data between the redefinition
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, updateData);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    glClear(GL_COLOR_BUFFER_BIT);

    // Bind the second image to this texture
    createEGLImageTargetTexture2D(image2, texture);

    // Delete Image
    eglDestroyImageKHR(window->getDisplay(), image1);
    eglDestroyImageKHR(window->getDisplay(), image2);

    ASSERT_EGL_SUCCESS();

    ASSERT_GL_NO_ERROR();
}

// Regression test to check that sRGB texture can be used to create image in sRGB colorspace.
// Also check that creating image using sRGB texture in linear colorspace wouldn't fail.
TEST_P(ImageTestES3, CreatesRGBImages)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt() || !has2DTextureExt());
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_sRGB"));
    ANGLE_SKIP_TEST_IF(!hasImageGLColorspaceExt());
    ANGLE_SKIP_TEST_IF(
        !IsEGLDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_gl_colorspace"));

    std::vector<EGLint> colorSpaces = {EGL_GL_COLORSPACE_SRGB_KHR, EGL_GL_COLORSPACE_LINEAR_KHR};
    constexpr GLsizei kWidth        = 2;
    constexpr GLsizei kHeight       = 2;

    for (size_t i = 0; i < colorSpaces.size(); i++)
    {
        // Create sRGB texture
        GLTexture sRGBTexture;
        glBindTexture(GL_TEXTURE_2D, sRGBTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, kWidth, kHeight, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
        ASSERT_GL_NO_ERROR();

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        ASSERT_GL_NO_ERROR();

        EGLint createImageAttribs[] = {
            EGL_IMAGE_PRESERVED_KHR, EGL_TRUE, EGL_GL_COLORSPACE_KHR, colorSpaces[i], EGL_NONE,
        };

        // Create the Image using sRGB texture
        EGLImageKHR image =
            eglCreateImageKHR(window->getDisplay(), window->getContext(), EGL_GL_TEXTURE_2D_KHR,
                              reinterpretHelper<EGLClientBuffer>(sRGBTexture), createImageAttribs);
        ASSERT_EGL_SUCCESS();
        ASSERT_NE(image, EGL_NO_IMAGE_KHR);

        // Clean up
        eglDestroyImageKHR(window->getDisplay(), image);
    }
}

// Regression test to check that sRGB texture can be used to create image in sRGB colorspace.
// Also check that creating image using sRGB texture in linear colorspace wouldn't fail.
TEST_P(ImageTestES3, DmaBufNegativeValidation)
{
    EGLWindow *window = getEGLWindow();
    ANGLE_SKIP_TEST_IF(!hasBaseExt());
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(getEGLWindow()->getDisplay(),
                                                     "EGL_EXT_image_dma_buf_import"));

    const EGLint invalidImageAttributeList[][3] = {
        {EGL_YUV_COLOR_SPACE_HINT_EXT, EGL_NONE, EGL_NONE},
        {EGL_SAMPLE_RANGE_HINT_EXT, EGL_NONE, EGL_NONE},
        {EGL_YUV_CHROMA_HORIZONTAL_SITING_HINT_EXT, EGL_NONE, EGL_NONE},
        {EGL_YUV_CHROMA_VERTICAL_SITING_HINT_EXT, EGL_NONE, EGL_NONE},
    };

    EGLImageKHR image;

    for (size_t i = 0; i < 4; i++)
    {
        image = eglCreateImageKHR(window->getDisplay(), EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, NULL,
                                  invalidImageAttributeList[i]);
        ASSERT_EGL_ERROR(EGL_BAD_ATTRIBUTE);
        ASSERT_EQ(image, EGL_NO_IMAGE_KHR);
    }
}

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3_AND(ImageTest,
                                       ES3_VULKAN().enable(Feature::AllocateNonZeroMemory));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ImageTestES3);
ANGLE_INSTANTIATE_TEST_ES3_AND(ImageTestES3, ES3_VULKAN().enable(Feature::AllocateNonZeroMemory));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ImageTestES31);
ANGLE_INSTANTIATE_TEST_ES31_AND(ImageTestES31,
                                ES31_VULKAN().enable(Feature::AllocateNonZeroMemory));
}  // namespace angle

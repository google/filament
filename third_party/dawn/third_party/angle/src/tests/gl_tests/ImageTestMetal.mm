//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ImageTestMetal:
//   Tests the correctness of eglImage with native Metal texture extensions.
//

#include "test_utils/ANGLETest.h"

#include "common/mathutil.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"

#include <CoreFoundation/CoreFoundation.h>
#include <Metal/Metal.h>
#include <gmock/gmock.h>
#include <span>

namespace angle
{
namespace
{
constexpr char kOESExt[]                      = "GL_OES_EGL_image";
constexpr char kBaseExt[]                     = "EGL_KHR_image_base";
constexpr char kDeviceMtlExt[]                = "EGL_ANGLE_device_metal";
constexpr char kEGLMtlImageNativeTextureExt[] = "EGL_ANGLE_metal_texture_client_buffer";
constexpr EGLint kDefaultAttribs[]            = {
    EGL_NONE,
};

template <typename T>
class ScopedMetalObjectRef : angle::NonCopyable
{
  public:
    ScopedMetalObjectRef() = default;

    explicit ScopedMetalObjectRef(T &&surface) : mObject(surface) {}

    ~ScopedMetalObjectRef()
    {
        if (mObject)
        {
            release();
            mObject = nil;
        }
    }

    T get() const { return mObject; }

    operator bool() const { return !!mObject; }

    // auto cast to T
    operator T() const { return mObject; }
    ScopedMetalObjectRef(const ScopedMetalObjectRef &other)
    {
        if (mObject)
        {
            release();
        }
        mObject = other.mObject;
    }

    explicit ScopedMetalObjectRef(ScopedMetalObjectRef &&other)
    {
        if (mObject)
        {
            release();
        }
        mObject       = other.mObject;
        other.mObject = nil;
    }

    ScopedMetalObjectRef &operator=(ScopedMetalObjectRef &&other)
    {
        if (mObject)
        {
            release();
        }
        mObject       = other.mObject;
        other.mObject = nil;

        return *this;
    }

    ScopedMetalObjectRef &operator=(const ScopedMetalObjectRef &other)
    {
        if (mObject)
        {
            release();
        }
        mObject = other.mObject;

        return *this;
    }

  private:
    void release()
    {
#if !__has_feature(objc_arc)
        [mObject release];
#endif
    }

    T mObject = nil;
};

using ScopedMetalTextureRef      = ScopedMetalObjectRef<id<MTLTexture>>;
using ScopedMetalBufferRef       = ScopedMetalObjectRef<id<MTLBuffer>>;
using ScopedMetalCommandQueueRef = ScopedMetalObjectRef<id<MTLCommandQueue>>;

}  // anonymous namespace

bool IsDepthOrStencil(MTLPixelFormat format)
{
    switch (format)
    {
        case MTLPixelFormatDepth16Unorm:
        case MTLPixelFormatDepth32Float:
        case MTLPixelFormatStencil8:
        case MTLPixelFormatDepth24Unorm_Stencil8:
        case MTLPixelFormatDepth32Float_Stencil8:
        case MTLPixelFormatX32_Stencil8:
        case MTLPixelFormatX24_Stencil8:
            return true;

        default:
            return false;
    }
}

ScopedMetalTextureRef CreateMetalTexture2D(id<MTLDevice> deviceMtl,
                                           int width,
                                           int height,
                                           MTLPixelFormat format,
                                           int arrayLength)
{
    @autoreleasepool
    {
        MTLTextureDescriptor *desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:format
                                                                                        width:width
                                                                                       height:width
                                                                                    mipmapped:NO];
        desc.usage                 = MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget;
        if (IsDepthOrStencil(format))
        {
            desc.storageMode = MTLStorageModePrivate;
        }
        if (arrayLength)
        {
            desc.arrayLength = arrayLength;
            desc.textureType = MTLTextureType2DArray;
        }
        ScopedMetalTextureRef re([deviceMtl newTextureWithDescriptor:desc]);
        return re;
    }
}

id<MTLSharedEvent> CreateMetalSharedEvent(id<MTLDevice> deviceMtl)
{
    id<MTLSharedEvent> sharedEvent = [deviceMtl newSharedEvent];
    sharedEvent.label              = @"TestSharedEvent";
    return sharedEvent;
}

EGLSync CreateEGLSyncFromMetalSharedEvent(EGLDisplay display,
                                          id<MTLSharedEvent> sharedEvent,
                                          uint64_t signalValue,
                                          bool signaled)
{
    EGLAttrib signalValueHi            = signalValue >> 32;
    EGLAttrib signalValueLo            = signalValue & 0xffffffff;
    std::vector<EGLAttrib> syncAttribs = {
        EGL_SYNC_METAL_SHARED_EVENT_OBJECT_ANGLE,          reinterpret_cast<EGLAttrib>(sharedEvent),
        EGL_SYNC_METAL_SHARED_EVENT_SIGNAL_VALUE_HI_ANGLE, signalValueHi,
        EGL_SYNC_METAL_SHARED_EVENT_SIGNAL_VALUE_LO_ANGLE, signalValueLo};

    if (signaled)
    {
        syncAttribs.push_back(EGL_SYNC_CONDITION);
        syncAttribs.push_back(EGL_SYNC_METAL_SHARED_EVENT_SIGNALED_ANGLE);
    }

    syncAttribs.push_back(EGL_NONE);

    EGLSync syncWithSharedEvent =
        eglCreateSync(display, EGL_SYNC_METAL_SHARED_EVENT_ANGLE, syncAttribs.data());
    EXPECT_NE(syncWithSharedEvent, EGL_NO_SYNC);

    return syncWithSharedEvent;
}

class ImageTestMetal : public ANGLETest<>
{
  protected:
    ImageTestMetal()
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
        constexpr char kVS[] = "precision highp float;\n"
                               "attribute vec4 position;\n"
                               "varying vec2 texcoord;\n"
                               "\n"
                               "void main()\n"
                               "{\n"
                               "    gl_Position = position;\n"
                               "    texcoord = (position.xy * 0.5) + 0.5;\n"
                               "    texcoord.y = 1.0 - texcoord.y;\n"
                               "}\n";

        constexpr char kTextureFS[] = "precision highp float;\n"
                                      "uniform sampler2D tex;\n"
                                      "varying vec2 texcoord;\n"
                                      "\n"
                                      "void main()\n"
                                      "{\n"
                                      "    gl_FragColor = texture2D(tex, texcoord);\n"
                                      "}\n";

        mTextureProgram = CompileProgram(kVS, kTextureFS);
        if (mTextureProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        mTextureUniformLocation = glGetUniformLocation(mTextureProgram, "tex");

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override { glDeleteProgram(mTextureProgram); }

    id<MTLDevice> getMtlDevice()
    {
        EGLAttrib angleDevice = 0;
        EGLAttrib device      = 0;
        EXPECT_EGL_TRUE(
            eglQueryDisplayAttribEXT(getEGLWindow()->getDisplay(), EGL_DEVICE_EXT, &angleDevice));

        EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(reinterpret_cast<EGLDeviceEXT>(angleDevice),
                                                EGL_METAL_DEVICE_ANGLE, &device));

        return (__bridge id<MTLDevice>)reinterpret_cast<void *>(device);
    }

    ScopedMetalTextureRef createMtlTexture2D(int width, int height, MTLPixelFormat format)
    {
        id<MTLDevice> device = getMtlDevice();

        return CreateMetalTexture2D(device, width, height, format, 0);
    }

    ScopedMetalTextureRef createMtlTexture2DArray(int width,
                                                  int height,
                                                  int arrayLength,
                                                  MTLPixelFormat format)
    {
        id<MTLDevice> device = getMtlDevice();

        return CreateMetalTexture2D(device, width, height, format, arrayLength);
    }

    void getTextureSliceBytes(id<MTLTexture> texture,
                              unsigned bytesPerRow,
                              MTLRegion region,
                              unsigned mipmapLevel,
                              unsigned slice,
                              std::span<uint8_t> sliceImage)
    {
        @autoreleasepool
        {
            id<MTLDevice> device = texture.device;
            ScopedMetalBufferRef readBuffer([device
                newBufferWithLength:sliceImage.size()
                            options:MTLResourceStorageModeShared]);
            ScopedMetalCommandQueueRef commandQueue([device newCommandQueue]);
            id<MTLCommandBuffer> commandBuffer    = [commandQueue commandBuffer];
            id<MTLBlitCommandEncoder> blitEncoder = [commandBuffer blitCommandEncoder];
            [blitEncoder copyFromTexture:texture
                             sourceSlice:slice
                             sourceLevel:mipmapLevel
                            sourceOrigin:region.origin
                              sourceSize:region.size
                                toBuffer:readBuffer
                       destinationOffset:0
                  destinationBytesPerRow:bytesPerRow
                destinationBytesPerImage:sliceImage.size()];
            [blitEncoder endEncoding];
            [commandBuffer commit];
            [commandBuffer waitUntilCompleted];
            memcpy(sliceImage.data(), readBuffer.get().contents, sliceImage.size());
        }
    }
    void sourceMetalTarget2D_helper(GLubyte data[4],
                                    const EGLint *attribs,
                                    EGLImageKHR *imageOut,
                                    GLuint *textureOut);

    void verifyResultsTexture(GLuint texture,
                              const GLubyte data[4],
                              GLenum textureTarget,
                              GLuint program,
                              GLuint textureUniform)
    {
        // Draw a quad with the target texture
        glUseProgram(program);
        glBindTexture(textureTarget, texture);
        glUniform1i(textureUniform, 0);

        drawQuad(program, "position", 0.5f);

        // Expect that the rendered quad has the same color as the source texture
        EXPECT_PIXEL_NEAR(0, 0, data[0], data[1], data[2], data[3], 1.0);
    }

    void verifyResults2D(GLuint texture, const GLubyte data[4])
    {
        verifyResultsTexture(texture, data, GL_TEXTURE_2D, mTextureProgram,
                             mTextureUniformLocation);
    }

    void drawColorQuad(GLColor color)
    {
        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
        glUseProgram(program);
        GLint colorUniformLocation =
            glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
        ASSERT_NE(colorUniformLocation, -1);
        glUniform4fv(colorUniformLocation, 1, color.toNormalizedVector().data());
        drawQuad(program, essl1_shaders::PositionAttrib(), 0);
        glUseProgram(0);
    }

    bool hasDepth24Stencil8PixelFormat()
    {
        id<MTLDevice> device = getMtlDevice();
        return device.depth24Stencil8PixelFormatSupported;
    }

    bool hasImageNativeMetalTextureExt() const
    {
        if (!IsMetal())
        {
            return false;
        }
        EGLAttrib angleDevice = 0;
        eglQueryDisplayAttribEXT(getEGLWindow()->getDisplay(), EGL_DEVICE_EXT, &angleDevice);
        if (!angleDevice)
        {
            return false;
        }
        auto extensionString = static_cast<const char *>(
            eglQueryDeviceStringEXT(reinterpret_cast<EGLDeviceEXT>(angleDevice), EGL_EXTENSIONS));
        if (strstr(extensionString, kDeviceMtlExt) == nullptr)
        {
            return false;
        }
        return IsEGLDisplayExtensionEnabled(getEGLWindow()->getDisplay(),
                                            kEGLMtlImageNativeTextureExt);
    }

    bool hasOESExt() const { return IsGLExtensionEnabled(kOESExt); }

    bool hasBaseExt() const
    {
        return IsEGLDisplayExtensionEnabled(getEGLWindow()->getDisplay(), kBaseExt);
    }

    GLuint mTextureProgram;
    GLint mTextureUniformLocation;
};

void ImageTestMetal::sourceMetalTarget2D_helper(GLubyte data[4],
                                                const EGLint *attribs,
                                                EGLImageKHR *imageOut,
                                                GLuint *textureOut)
{
    EGLWindow *window = getEGLWindow();

    // Create MTLTexture
    ScopedMetalTextureRef textureMtl = createMtlTexture2D(1, 1, MTLPixelFormatRGBA8Unorm);

    // Create image
    EGLImageKHR image =
        eglCreateImageKHR(window->getDisplay(), EGL_NO_CONTEXT, EGL_METAL_TEXTURE_ANGLE,
                          reinterpret_cast<EGLClientBuffer>(textureMtl.get()), attribs);
    ASSERT_EGL_SUCCESS();

    // Write the data to the MTLTexture
    [textureMtl replaceRegion:MTLRegionMake2D(0, 0, 1, 1)
                  mipmapLevel:0
                        slice:0
                    withBytes:data
                  bytesPerRow:4
                bytesPerImage:0];

    // Create a texture target to bind the egl image
    GLuint target;
    glGenTextures(1, &target);
    glBindTexture(GL_TEXTURE_2D, target);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);

    *imageOut   = image;
    *textureOut = target;
}

// Test that trying to set renderbuffer storage without a renderbuffer is an error.
TEST_P(ImageTestMetal, RenderbufferStorageNoRenderbufferIsError)
{
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt());
    ANGLE_SKIP_TEST_IF(!hasImageNativeMetalTextureExt());

    EGLDisplay display = getEGLWindow()->getDisplay();

    const int bufferSize = 32;
    ScopedMetalTextureRef textureMtl =
        createMtlTexture2DArray(bufferSize, bufferSize, 1, MTLPixelFormatDepth32Float_Stencil8);
    const EGLint attribs[] = {EGL_TEXTURE_INTERNAL_FORMAT_ANGLE, GL_DEPTH24_STENCIL8, EGL_NONE};
    EGLImageKHR image =
        eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_METAL_TEXTURE_ANGLE,
                          reinterpret_cast<EGLClientBuffer>(textureMtl.get()), attribs);
    EXPECT_EGL_SUCCESS();
    EXPECT_NE(image, nullptr);

    glEGLImageTargetRenderbufferStorageOES(GL_RENDERBUFFER, image);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    GLRenderbuffer depthStencilBuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, depthStencilBuffer);
    glEGLImageTargetRenderbufferStorageOES(GL_RENDERBUFFER, image);
    EXPECT_GL_NO_ERROR();
}

// Testing source metal EGL image, target 2D texture
TEST_P(ImageTestMetal, SourceMetalTarget2D)
{
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt());
    ANGLE_SKIP_TEST_IF(!hasImageNativeMetalTextureExt());

    EGLWindow *window = getEGLWindow();

    // Create the Image
    EGLImageKHR image;
    GLuint texTarget;
    GLubyte data[4] = {7, 51, 197, 231};
    sourceMetalTarget2D_helper(data, kDefaultAttribs, &image, &texTarget);

    // Use texture target bound to egl image as source and render to framebuffer
    // Verify that data in framebuffer matches that in the egl image
    verifyResults2D(texTarget, data);

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
    glDeleteTextures(1, &texTarget);
}

// Create source metal EGL image, target 2D texture, then trigger texture respecification.
TEST_P(ImageTestMetal, SourceMetal2DTargetTextureRespecifySize)
{
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt());
    ANGLE_SKIP_TEST_IF(!hasImageNativeMetalTextureExt());

    EGLWindow *window = getEGLWindow();

    // Create the Image
    EGLImageKHR image;
    GLuint texTarget;
    GLubyte data[4] = {7, 51, 197, 231};
    sourceMetalTarget2D_helper(data, kDefaultAttribs, &image, &texTarget);

    // Use texture target bound to egl image as source and render to framebuffer
    // Verify that data in framebuffer matches that in the egl image
    verifyResults2D(texTarget, data);

    // Respecify texture size and verify results
    std::array<GLubyte, 16> referenceColor;
    referenceColor.fill(127);
    glBindTexture(GL_TEXTURE_2D, texTarget);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 referenceColor.data());
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 referenceColor.data());
    ASSERT_GL_NO_ERROR();

    // Expect that the target texture has the reference color values
    verifyResults2D(texTarget, referenceColor.data());

    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
    glDeleteTextures(1, &texTarget);
}

// Tests that OpenGL can sample from a texture bound with Metal texture slice.
TEST_P(ImageTestMetal, SourceMetalTarget2DArray)
{
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt());
    ANGLE_SKIP_TEST_IF(!hasImageNativeMetalTextureExt());
    ScopedMetalTextureRef textureMtl = createMtlTexture2DArray(1, 1, 3, MTLPixelFormatRGBA8Unorm);

    GLubyte data0[4] = {93, 83, 75, 128};
    [textureMtl replaceRegion:MTLRegionMake2D(0, 0, 1, 1)
                  mipmapLevel:0
                        slice:0
                    withBytes:data0
                  bytesPerRow:4
                bytesPerImage:4];
    GLubyte data1[4] = {7, 51, 197, 231};
    [textureMtl replaceRegion:MTLRegionMake2D(0, 0, 1, 1)
                  mipmapLevel:0
                        slice:1
                    withBytes:data1
                  bytesPerRow:4
                bytesPerImage:4];
    GLubyte data2[4] = {33, 51, 44, 33};
    [textureMtl replaceRegion:MTLRegionMake2D(0, 0, 1, 1)
                  mipmapLevel:0
                        slice:2
                    withBytes:data2
                  bytesPerRow:4
                bytesPerImage:4];

    EGLDisplay display = getEGLWindow()->getDisplay();
    EGLImageKHR image0 =
        eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_METAL_TEXTURE_ANGLE,
                          reinterpret_cast<EGLClientBuffer>(textureMtl.get()), nullptr);
    ASSERT_EGL_SUCCESS();
    const EGLint attribs1[] = {EGL_METAL_TEXTURE_ARRAY_SLICE_ANGLE, 1, EGL_NONE};
    EGLImageKHR image1 =
        eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_METAL_TEXTURE_ANGLE,
                          reinterpret_cast<EGLClientBuffer>(textureMtl.get()), attribs1);
    ASSERT_EGL_SUCCESS();
    const EGLint attribs2[] = {EGL_METAL_TEXTURE_ARRAY_SLICE_ANGLE, 2, EGL_NONE};
    EGLImageKHR image2 =
        eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_METAL_TEXTURE_ANGLE,
                          reinterpret_cast<EGLClientBuffer>(textureMtl.get()), attribs2);
    ASSERT_EGL_SUCCESS();

    GLTexture targetTexture;
    glBindTexture(GL_TEXTURE_2D, targetTexture);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image0);
    verifyResults2D(targetTexture, data0);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image1);
    verifyResults2D(targetTexture, data1);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image2);
    verifyResults2D(targetTexture, data2);
    eglDestroyImageKHR(display, image0);
    eglDestroyImageKHR(display, image1);
    eglDestroyImageKHR(display, image2);
    EXPECT_GL_NO_ERROR();
    EXPECT_EGL_SUCCESS();
}

// Test that bound slice to EGLImage is not affected by releasing the source texture.
TEST_P(ImageTestMetal, SourceMetalTarget2DArrayReleasedSourceOk)
{
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt());
    ANGLE_SKIP_TEST_IF(!hasImageNativeMetalTextureExt());
    ScopedMetalTextureRef textureMtl = createMtlTexture2DArray(1, 1, 3, MTLPixelFormatRGBA8Unorm);

    GLubyte data1[4] = {7, 51, 197, 231};
    [textureMtl replaceRegion:MTLRegionMake2D(0, 0, 1, 1)
                  mipmapLevel:0
                        slice:1
                    withBytes:data1
                  bytesPerRow:4
                bytesPerImage:4];

    EGLDisplay display      = getEGLWindow()->getDisplay();
    const EGLint attribs1[] = {EGL_METAL_TEXTURE_ARRAY_SLICE_ANGLE, 1, EGL_NONE};
    EGLImageKHR image1 =
        eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_METAL_TEXTURE_ANGLE,
                          reinterpret_cast<EGLClientBuffer>(textureMtl.get()), attribs1);
    ASSERT_EGL_SUCCESS();
    // This is being tested: release the source texture but the slice keeps working.
    textureMtl = {};
    GLTexture targetTexture;
    glBindTexture(GL_TEXTURE_2D, targetTexture);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image1);
    verifyResults2D(targetTexture, data1);
    eglDestroyImageKHR(display, image1);
    EXPECT_GL_NO_ERROR();
    EXPECT_EGL_SUCCESS();
}

// Tests that OpenGL can draw to a texture bound with Metal texture.
TEST_P(ImageTestMetal, DrawMetalTarget2D)
{
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt());
    ANGLE_SKIP_TEST_IF(!hasImageNativeMetalTextureExt());

    EGLDisplay display = getEGLWindow()->getDisplay();

    ScopedMetalTextureRef textureMtl = createMtlTexture2D(1, 1, MTLPixelFormatRGBA8Unorm);
    [textureMtl replaceRegion:MTLRegionMake2D(0, 0, 1, 1)
                  mipmapLevel:0
                        slice:0
                    withBytes:GLColor::red.data()
                  bytesPerRow:4
                bytesPerImage:4];

    EGLImageKHR image =
        eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_METAL_TEXTURE_ANGLE,
                          reinterpret_cast<EGLClientBuffer>(textureMtl.get()), nullptr);
    ASSERT_EGL_SUCCESS();

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);

    GLFramebuffer targetFbo;
    glBindFramebuffer(GL_FRAMEBUFFER, targetFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glViewport(0, 0, 1, 1);
    drawColorQuad(GLColor::magenta);
    EXPECT_GL_NO_ERROR();
    eglDestroyImageKHR(display, image);
    eglWaitUntilWorkScheduledANGLE(display);
    EXPECT_GL_NO_ERROR();
    EXPECT_EGL_SUCCESS();

    GLColor result;
    getTextureSliceBytes(textureMtl, 4, MTLRegionMake2D(0, 0, 1, 1), 0, 0, {result.data(), 4});
    EXPECT_EQ(result, GLColor::magenta);
}

// Tests that OpenGL can draw to a texture bound with Metal texture slice.
TEST_P(ImageTestMetal, DrawMetalTarget2DArray)
{
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt());
    ANGLE_SKIP_TEST_IF(!hasImageNativeMetalTextureExt());

    EGLDisplay display = getEGLWindow()->getDisplay();

    ScopedMetalTextureRef textureMtl = createMtlTexture2DArray(1, 1, 2, MTLPixelFormatRGBA8Unorm);
    [textureMtl replaceRegion:MTLRegionMake2D(0, 0, 1, 1)
                  mipmapLevel:0
                        slice:0
                    withBytes:GLColor::red.data()
                  bytesPerRow:4
                bytesPerImage:4];
    [textureMtl replaceRegion:MTLRegionMake2D(0, 0, 1, 1)
                  mipmapLevel:0
                        slice:1
                    withBytes:GLColor::red.data()
                  bytesPerRow:4
                bytesPerImage:4];

    const EGLint attribs[] = {EGL_METAL_TEXTURE_ARRAY_SLICE_ANGLE, 1, EGL_NONE};
    EGLImageKHR image =
        eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_METAL_TEXTURE_ANGLE,
                          reinterpret_cast<EGLClientBuffer>(textureMtl.get()), attribs);
    ASSERT_EGL_SUCCESS();

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);
    EXPECT_GL_NO_ERROR();
    GLFramebuffer targetFbo;
    glBindFramebuffer(GL_FRAMEBUFFER, targetFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glViewport(0, 0, 1, 1);
    drawColorQuad(GLColor::magenta);
    EXPECT_GL_NO_ERROR();
    eglDestroyImageKHR(display, image);
    eglWaitUntilWorkScheduledANGLE(display);
    EXPECT_GL_NO_ERROR();
    EXPECT_EGL_SUCCESS();

    GLColor result;
    getTextureSliceBytes(textureMtl, 4, MTLRegionMake2D(0, 0, 1, 1), 0, 1, {result.data(), 4});
    EXPECT_EQ(result, GLColor::magenta);
}

// Tests that OpenGL can blit to a texture bound with Metal texture slice.
TEST_P(ImageTestMetal, BlitMetalTarget2DArray)
{
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt());
    ANGLE_SKIP_TEST_IF(!hasImageNativeMetalTextureExt());

    EGLDisplay display = getEGLWindow()->getDisplay();

    GLTexture colorBuffer;
    glBindTexture(GL_TEXTURE_2D, colorBuffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, GLColor::green.data());
    glTexSubImage2D(GL_TEXTURE_2D, 0, 1, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                    GLColor::yellow.data());
    EXPECT_GL_NO_ERROR();

    GLFramebuffer sourceFbo;
    glBindFramebuffer(GL_FRAMEBUFFER, sourceFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    ScopedMetalTextureRef textureMtl = createMtlTexture2DArray(1, 1, 2, MTLPixelFormatRGBA8Unorm);
    [textureMtl replaceRegion:MTLRegionMake2D(0, 0, 1, 1)
                  mipmapLevel:0
                        slice:0
                    withBytes:GLColor::red.data()
                  bytesPerRow:4
                bytesPerImage:4];
    [textureMtl replaceRegion:MTLRegionMake2D(0, 0, 1, 1)
                  mipmapLevel:0
                        slice:1
                    withBytes:GLColor::red.data()
                  bytesPerRow:4
                bytesPerImage:4];

    for (int slice = 0; slice < 2; ++slice)
    {
        const EGLint attribs[] = {EGL_METAL_TEXTURE_ARRAY_SLICE_ANGLE, slice, EGL_NONE};
        EGLImageKHR image =
            eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_METAL_TEXTURE_ANGLE,
                              reinterpret_cast<EGLClientBuffer>(textureMtl.get()), attribs);
        ASSERT_EGL_SUCCESS();

        GLTexture texture;
        glBindTexture(GL_TEXTURE_2D, texture);
        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        verifyResults2D(texture, GLColor::red.data());
        EXPECT_GL_NO_ERROR();

        GLFramebuffer targetFbo;
        glBindFramebuffer(GL_FRAMEBUFFER, targetFbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, sourceFbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, targetFbo);
        glBlitFramebufferANGLE(slice, 0, slice + 1, 1, 0, 0, 1, 1, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        EXPECT_GL_NO_ERROR();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        verifyResults2D(texture, slice == 0 ? GLColor::green.data() : GLColor::yellow.data());
        eglDestroyImageKHR(display, image);
        EXPECT_GL_NO_ERROR();
        EXPECT_EGL_SUCCESS();
    }
    eglWaitUntilWorkScheduledANGLE(display);
    EXPECT_EGL_SUCCESS();

    GLColor result;
    getTextureSliceBytes(textureMtl, 4, MTLRegionMake2D(0, 0, 1, 1), 0, 0, {result.data(), 4});
    EXPECT_EQ(result, GLColor::green);
    getTextureSliceBytes(textureMtl, 4, MTLRegionMake2D(0, 0, 1, 1), 0, 1, {result.data(), 4});
    EXPECT_EQ(result, GLColor::yellow);
}

// Tests that OpenGL can override the internal format for a texture bound with
// Metal texture.
TEST_P(ImageTestMetal, OverrideMetalTextureInternalFormat)
{
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt());
    ANGLE_SKIP_TEST_IF(!hasImageNativeMetalTextureExt());
    ANGLE_SKIP_TEST_IF(hasDepth24Stencil8PixelFormat());

    EGLDisplay display = getEGLWindow()->getDisplay();

    // On iOS devices, GL_DEPTH24_STENCIL8 is unavailable and is interally converted into
    // GL_DEPTH32F_STENCIL8. This tests the ability to attach MTLPixelFormatDepth32Float_Stencil8
    // and have GL treat it as GL_DEPTH24_STENCIL8 instead of GL_DEPTH32F_STENCIL8.
    ScopedMetalTextureRef textureMtl =
        createMtlTexture2DArray(1, 1, 1, MTLPixelFormatDepth32Float_Stencil8);
    const EGLint attribs[] = {EGL_TEXTURE_INTERNAL_FORMAT_ANGLE, GL_DEPTH24_STENCIL8, EGL_NONE};
    EGLImageKHR image =
        eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_METAL_TEXTURE_ANGLE,
                          reinterpret_cast<EGLClientBuffer>(textureMtl.get()), attribs);
    EXPECT_EGL_SUCCESS();
    EXPECT_NE(image, nullptr);
}

// Tests that OpenGL can override the internal format for a texture bound with
// Metal texture and that rendering to the texture is successful.
TEST_P(ImageTestMetal, RenderingTest)
{
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt());
    ANGLE_SKIP_TEST_IF(!hasImageNativeMetalTextureExt());
    ANGLE_SKIP_TEST_IF(hasDepth24Stencil8PixelFormat());

    EGLDisplay display = getEGLWindow()->getDisplay();

    const int bufferSize = 32;
    ScopedMetalTextureRef textureMtl =
        createMtlTexture2DArray(bufferSize, bufferSize, 1, MTLPixelFormatDepth32Float_Stencil8);
    const EGLint attribs[] = {EGL_TEXTURE_INTERNAL_FORMAT_ANGLE, GL_DEPTH24_STENCIL8, EGL_NONE};
    EGLImageKHR image =
        eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_METAL_TEXTURE_ANGLE,
                          reinterpret_cast<EGLClientBuffer>(textureMtl.get()), attribs);
    EXPECT_EGL_SUCCESS();
    EXPECT_NE(image, nullptr);

    GLRenderbuffer colorBuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, bufferSize, bufferSize);
    EXPECT_GL_NO_ERROR();

    GLRenderbuffer depthStencilBuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, depthStencilBuffer);
    glEGLImageTargetRenderbufferStorageOES(GL_RENDERBUFFER, image);
    EXPECT_GL_NO_ERROR();

    GLFramebuffer fb;
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBuffer);
    if (getClientMajorVersion() >= 3)
    {
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                  depthStencilBuffer);
    }
    else
    {
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                                  depthStencilBuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                  depthStencilBuffer);
    }

    EXPECT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glClearColor(1.f, 0.f, 0.f, 1.f);
    glClearDepthf(1.f);
    glClearStencil(0x55);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0x55, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilMask(0xFF);

    // Draw green.
    ANGLE_GL_PROGRAM(drawGreen, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    drawQuad(drawGreen, essl1_shaders::PositionAttrib(), 0.95f);

    // Verify that green was drawn.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, bufferSize - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(bufferSize - 1, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(bufferSize - 1, bufferSize - 1, GLColor::green);

    eglDestroyImageKHR(display, image);
}

// Tests that OpenGL override the with a bad internal format for a texture bound
// with Metal texture fails.
TEST_P(ImageTestMetal, OverrideMetalTextureInternalFormatBadFormat)
{
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt());
    ANGLE_SKIP_TEST_IF(!hasImageNativeMetalTextureExt());

    EGLDisplay display = getEGLWindow()->getDisplay();

    // On iOS devices, GL_DEPTH24_STENCIL8 is unavailable and is interally converted into
    // GL_DEPTH32F_STENCIL8. This tests the ability to attach MTLPixelFormatDepth32Float_Stencil8
    // and have GL treat it as GL_DEPTH24_STENCIL8 instead of GL_DEPTH32F_STENCIL8.
    ScopedMetalTextureRef textureMtl =
        createMtlTexture2DArray(1, 1, 1, MTLPixelFormatDepth32Float_Stencil8);
    const EGLint attribs[] = {EGL_TEXTURE_INTERNAL_FORMAT_ANGLE, GL_TRIANGLES, EGL_NONE};
    EGLImageKHR image =
        eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_METAL_TEXTURE_ANGLE,
                          reinterpret_cast<EGLClientBuffer>(textureMtl.get()), attribs);
    EXPECT_EGL_ERROR(EGL_BAD_ATTRIBUTE);
    EXPECT_EQ(image, nullptr);
}

// Tests that OpenGL override the with an incompatible internal format for a texture bound
// with Metal texture fails.
TEST_P(ImageTestMetal, OverrideMetalTextureInternalFormatIncompatibleFormat)
{
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt());
    ANGLE_SKIP_TEST_IF(!hasImageNativeMetalTextureExt());

    EGLDisplay display = getEGLWindow()->getDisplay();

    // On iOS devices, GL_DEPTH24_STENCIL8 is unavailable and is interally converted into
    // GL_DEPTH32F_STENCIL8. This tests the ability to attach MTLPixelFormatDepth32Float_Stencil8
    // and have GL treat it as GL_DEPTH24_STENCIL8 instead of GL_DEPTH32F_STENCIL8.
    ScopedMetalTextureRef textureMtl =
        createMtlTexture2DArray(1, 1, 1, MTLPixelFormatDepth32Float_Stencil8);
    const EGLint attribs[] = {EGL_TEXTURE_INTERNAL_FORMAT_ANGLE, GL_RGBA8, EGL_NONE};
    EGLImageKHR image =
        eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_METAL_TEXTURE_ANGLE,
                          reinterpret_cast<EGLClientBuffer>(textureMtl.get()), attribs);
    EXPECT_EGL_ERROR(EGL_BAD_ATTRIBUTE);
    EXPECT_EQ(image, nullptr);
}

// Test this scenario:
// Metal and GL share the same MTL texture (called texture1).
// GL Context draws to texture1:
// - draw.
// - upload texture2
// - draw using the texture2 as source.
// - place a sync object.
//
// Metal reads the texture1:
// - wait for the shared event sync object.
// - copy the texture1 to a buffer.
// - The buffer should contain color from texture2 after being uploaded.
//
// Previously this would cause a bug in Metal backend because texture upload would
// create a new blit encoder in a middle of a render pass and the command buffer would mistrack the
// ongoing render encoder. Thus making the metal sync object being placed incorrectly.
TEST_P(ImageTestMetal, SharedEventSyncWhenThereIsTextureUploadBetweenDraws)
{
    ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt());
    ANGLE_SKIP_TEST_IF(!hasImageNativeMetalTextureExt());

    EGLDisplay display1 = getEGLWindow()->getDisplay();

    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);
    ANGLE_SKIP_TEST_IF(
        !IsEGLDisplayExtensionEnabled(display1, "EGL_ANGLE_metal_shared_event_sync"));

    @autoreleasepool
    {
        // Create MTLTexture
        constexpr int kSharedTextureSize = 1024;
        ScopedMetalTextureRef textureMtl =
            createMtlTexture2D(kSharedTextureSize, kSharedTextureSize, MTLPixelFormatR32Sint);

        // Create SharedEvent
        id<MTLDevice> deviceMtl           = getMtlDevice();
        id<MTLSharedEvent> sharedEventMtl = CreateMetalSharedEvent(deviceMtl);

        // -------------------------- Metal ---------------------------
        // Create a buffer on Metal to store the final value.
        ScopedMetalBufferRef dstBuffer(
            [deviceMtl newBufferWithLength:sizeof(float) options:MTLResourceStorageModeShared]);
        ScopedMetalCommandQueueRef commandQueue([deviceMtl newCommandQueue]);
        id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];

        // Wait for drawing on GL context to finish on server side.
        // Note: we issue a wait even before the draw calls are issued on GL context.
        // GL context will issue a signaling later (see below).
        constexpr uint64_t kSignalValue = 0xff;
        [commandBuffer encodeWaitForEvent:sharedEventMtl value:kSignalValue];

        // Copy a pixel from texture1 to dstBuffer
        id<MTLBlitCommandEncoder> blitEncoder = [commandBuffer blitCommandEncoder];
        [blitEncoder copyFromTexture:textureMtl
                         sourceSlice:0
                         sourceLevel:0
                        sourceOrigin:MTLOriginMake(kSharedTextureSize - 1, kSharedTextureSize - 2,
                                                   0)
                          sourceSize:MTLSizeMake(1, 1, 1)
                            toBuffer:dstBuffer
                   destinationOffset:0
              destinationBytesPerRow:sizeof(float) * kSharedTextureSize
            destinationBytesPerImage:0];
        [blitEncoder endEncoding];
        [commandBuffer commit];

        // -------------------------- GL context ---------------------------
        constexpr int kNumValues = 1000;
        // A deliberately slow shader that reads a texture many times then write
        // the sum value to an ouput varible.
        constexpr char kFS[] = R"(#version 300 es
out highp ivec4 outColor;

uniform highp isampler2D u_valuesTex;

void main()
{
    highp int value = 0;
    for (int i = 0; i < 1000; ++i) {
        highp float uCoords = (float(i) + 0.5) / float(1000);
        value += textureLod(u_valuesTex, vec2(uCoords, 0.0), 0.0).r;
    }

    outColor = ivec4(value);
})";

        ANGLE_GL_PROGRAM(complexProgram, essl3_shaders::vs::Simple(), kFS);
        GLint valuesTexLocation = glGetUniformLocation(complexProgram, "u_valuesTex");
        ASSERT_NE(valuesTexLocation, -1);

        // Create the shared texture from MTLTexture.
        EGLImageKHR image =
            eglCreateImageKHR(display1, EGL_NO_CONTEXT, EGL_METAL_TEXTURE_ANGLE,
                              reinterpret_cast<EGLClientBuffer>(textureMtl.get()), kDefaultAttribs);
        EXPECT_EGL_SUCCESS();
        GLTexture texture1;
        glBindTexture(GL_TEXTURE_2D, texture1);
        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);
        EXPECT_GL_ERROR(GL_NO_ERROR);
        glViewport(0, 0, kSharedTextureSize, kSharedTextureSize);

        // Create texture holding multiple values to be accumulated in shader.
        GLTexture texture2;
        glBindTexture(GL_TEXTURE_2D, texture2);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, kNumValues, 1, 0, GL_RED_INTEGER, GL_INT, nullptr);
        glFlush();
        EXPECT_GL_ERROR(GL_NO_ERROR);

        // Using GL context to draw to the texture1
        glBindTexture(GL_TEXTURE_2D, texture1);
        GLFramebuffer framebuffer1;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer1);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture1, 0);

        // First draw with initial color
        {
            constexpr char kSimpleFS[] = R"(#version 300 es
out highp ivec4 outColor;

void main()
{
    outColor = ivec4(1);
})";
            ANGLE_GL_PROGRAM(colorProgram, angle::essl3_shaders::vs::Simple(), kSimpleFS);
            drawQuad(colorProgram, angle::essl3_shaders::PositionAttrib(), 0.5f);
        }

        // Upload the texture2
        std::vector<int32_t> values(kNumValues);
        for (size_t i = 0; i < values.size(); ++i)
        {
            values[i] = i;
        }
        glBindTexture(GL_TEXTURE_2D, texture2);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kNumValues, 1, GL_RED_INTEGER, GL_INT,
                        values.data());

        // 2nd draw call draw the texture2 to texture1.
        glUseProgram(complexProgram);
        drawQuad(complexProgram, angle::essl1_shaders::PositionAttrib(), 0.5f);

        // Place a sync object on GL context's commands stream.
        EGLSync syncGL = CreateEGLSyncFromMetalSharedEvent(display1, sharedEventMtl, kSignalValue,
                                                           /*signaled=*/false);
        glFlush();

        // -------------------------- Metal ---------------------------
        [commandBuffer waitUntilCompleted];

        // Read dstBuffer
        const int32_t kExpectedSum = kNumValues * (kNumValues - 1) / 2;
        int32_t *mappedInts        = static_cast<int32_t *>(dstBuffer.get().contents);
        EXPECT_EQ(mappedInts[0], kExpectedSum);

        eglDestroySync(display1, syncGL);
        eglDestroyImage(display1, image);

    }  // @autoreleasepool
}

class ImageClearTestMetal : public ImageTestMetal
{
  protected:
    ImageClearTestMetal() : ImageTestMetal() {}

    void RunUnsizedClearTest(MTLPixelFormat format)
    {
        ANGLE_SKIP_TEST_IF(!hasOESExt() || !hasBaseExt());
        ANGLE_SKIP_TEST_IF(!hasImageNativeMetalTextureExt());

        EGLWindow *window  = getEGLWindow();
        EGLDisplay display = window->getDisplay();

        window->makeCurrent();

        const GLint bufferSize = 32;
        ScopedMetalTextureRef textureMtl =
            createMtlTexture2DArray(bufferSize, bufferSize, 1, format);
        EXPECT_TRUE(textureMtl);

        EGLint internalFormat = GL_NONE;
        switch (format)
        {
            case MTLPixelFormatR8Unorm:
            case MTLPixelFormatR16Unorm:
                internalFormat = GL_RED_EXT;
                break;
            case MTLPixelFormatRG8Unorm:
            case MTLPixelFormatRG16Unorm:
                internalFormat = GL_RG_EXT;
                break;
            case MTLPixelFormatRGBA8Unorm:
            case MTLPixelFormatRGBA16Float:
            case MTLPixelFormatRGB10A2Unorm:
                internalFormat = GL_RGBA;
                break;
            case MTLPixelFormatRGBA8Unorm_sRGB:
                internalFormat = GL_SRGB_ALPHA_EXT;
                break;
            case MTLPixelFormatBGRA8Unorm:
                internalFormat = GL_BGRA_EXT;
                break;
            default:
                break;
        }

        const EGLint attribs[] = {EGL_TEXTURE_INTERNAL_FORMAT_ANGLE, internalFormat, EGL_NONE};

        EGLImageKHR image =
            eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_METAL_TEXTURE_ANGLE,
                              reinterpret_cast<EGLClientBuffer>(textureMtl.get()), attribs);
        ASSERT_EGL_SUCCESS();
        ASSERT_NE(image, EGL_NO_IMAGE_KHR);

        GLTexture texture;
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        EXPECT_GL_NO_ERROR();

        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);
        EXPECT_GL_NO_ERROR();

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        EXPECT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER),
                  static_cast<unsigned>(GL_FRAMEBUFFER_COMPLETE));
        EXPECT_GL_NO_ERROR();

        glViewport(0, 0, static_cast<GLsizei>(bufferSize), static_cast<GLsizei>(bufferSize));
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ASSERT_GL_NO_ERROR();

        if (format == MTLPixelFormatRGBA16Float)
        {
            EXPECT_PIXEL_32F_EQ(bufferSize / 2, bufferSize / 2, 1.0f, 1.0f, 1.0f, 1.0f);
        }
        else if (format == MTLPixelFormatR16Unorm)
        {
            EXPECT_PIXEL_16_NEAR(bufferSize / 2, bufferSize / 2, 65535, 0, 0, 65535, 0);
        }
        else if (format == MTLPixelFormatRG16Unorm)
        {
            EXPECT_PIXEL_16_NEAR(bufferSize / 2, bufferSize / 2, 65535, 65535, 0, 65535, 0);
        }
        else
        {
            GLuint readColor[4] = {0, 0, 0, 255};
            switch (format)
            {
                case MTLPixelFormatR8Unorm:
                    readColor[0] = 255;
                    break;
                case MTLPixelFormatRG8Unorm:
                    readColor[0] = readColor[1] = 255;
                    break;
                case MTLPixelFormatRGBA8Unorm:
                case MTLPixelFormatRGB10A2Unorm:
                case MTLPixelFormatRGBA16Float:
                case MTLPixelFormatRGBA8Unorm_sRGB:
                case MTLPixelFormatBGRA8Unorm:
                    readColor[0] = readColor[1] = readColor[2] = 255;
                    break;
                default:
                    break;
            }
            // Read back as GL_UNSIGNED_BYTE even though the texture might have more than 8bpc.
            EXPECT_PIXEL_EQ(bufferSize / 2, bufferSize / 2, readColor[0], readColor[1],
                            readColor[2], readColor[3]);
        }
    }
};

TEST_P(ImageClearTestMetal, ClearUnsizedRGBA8)
{
    RunUnsizedClearTest(MTLPixelFormatRGBA8Unorm);
}

TEST_P(ImageClearTestMetal, ClearUnsizedsRGBA8)
{
    RunUnsizedClearTest(MTLPixelFormatRGBA8Unorm_sRGB);
}

TEST_P(ImageClearTestMetal, ClearUnsizedBGRA8)
{
    RunUnsizedClearTest(MTLPixelFormatBGRA8Unorm);
}

TEST_P(ImageClearTestMetal, ClearUnsizedR8)
{
    RunUnsizedClearTest(MTLPixelFormatR8Unorm);
}

TEST_P(ImageClearTestMetal, ClearUnsizedRG8)
{
    RunUnsizedClearTest(MTLPixelFormatRG8Unorm);
}

TEST_P(ImageClearTestMetal, ClearUnsizedRGB10A2)
{
    RunUnsizedClearTest(MTLPixelFormatRGB10A2Unorm);
}

TEST_P(ImageClearTestMetal, ClearUnsizedRGBAF16)
{
    RunUnsizedClearTest(MTLPixelFormatRGBA16Float);
}

TEST_P(ImageClearTestMetal, ClearUnsizedR16)
{
    RunUnsizedClearTest(MTLPixelFormatR16Unorm);
}

TEST_P(ImageClearTestMetal, ClearUnsizedRG16)
{
    RunUnsizedClearTest(MTLPixelFormatRG16Unorm);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST(ImageTestMetal, ES2_METAL(), ES3_METAL());
ANGLE_INSTANTIATE_TEST(ImageClearTestMetal, ES2_METAL(), ES3_METAL());
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ImageTestMetal);
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ImageClearTestMetal);
}  // namespace angle

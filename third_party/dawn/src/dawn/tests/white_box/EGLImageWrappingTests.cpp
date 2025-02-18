// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <string>
#include <utility>
#include <vector>

#include "dawn/common/DynamicLib.h"
#include "dawn/common/egl_platform.h"
#include "dawn/native/OpenGLBackend.h"
#include "dawn/native/opengl/DeviceGL.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class EGLFunctions {
  public:
    EGLFunctions() {
#if DAWN_PLATFORM_IS(WINDOWS)
        const char* eglLib = "libEGL.dll";
#elif DAWN_PLATFORM_IS(MACOS)
        const char* eglLib = "libEGL.dylib";
#else
        const char* eglLib = "libEGL.so";
#endif
        EXPECT_TRUE(mlibEGL.Open(eglLib));
        CreateImage = reinterpret_cast<PFNEGLCREATEIMAGEPROC>(LoadProc("eglCreateImage"));
        DestroyImage = reinterpret_cast<PFNEGLDESTROYIMAGEPROC>(LoadProc("eglDestroyImage"));
        GetCurrentContext =
            reinterpret_cast<PFNEGLGETCURRENTCONTEXTPROC>(LoadProc("eglGetCurrentContext"));
        GetCurrentDisplay =
            reinterpret_cast<PFNEGLGETCURRENTDISPLAYPROC>(LoadProc("eglGetCurrentDisplay"));
        QueryString = reinterpret_cast<PFNEGLQUERYSTRINGPROC>(LoadProc("eglQueryString"));
    }

  private:
    void* LoadProc(const char* name) {
        void* proc = mlibEGL.GetProc(name);
        EXPECT_NE(proc, nullptr);
        return proc;
    }

  public:
    PFNEGLCREATEIMAGEPROC CreateImage;
    PFNEGLDESTROYIMAGEPROC DestroyImage;
    PFNEGLGETCURRENTCONTEXTPROC GetCurrentContext;
    PFNEGLGETCURRENTDISPLAYPROC GetCurrentDisplay;
    PFNEGLQUERYSTRINGPROC QueryString;

  private:
    DynamicLib mlibEGL;
};

class ScopedEGLImage {
  public:
    ScopedEGLImage(PFNEGLDESTROYIMAGEPROC destroyImage,
                   PFNGLDELETETEXTURESPROC deleteTextures,
                   EGLDisplay display,
                   EGLImage image,
                   GLuint texture)
        : mDestroyImage(destroyImage),
          mDeleteTextures(deleteTextures),
          mDisplay(display),
          mImage(image),
          mTexture(texture) {}

    ScopedEGLImage(ScopedEGLImage&& other) {
        if (mImage != nullptr) {
            mDestroyImage(mDisplay, mImage);
        }
        if (mTexture != 0) {
            mDeleteTextures(1, &mTexture);
        }
        mDestroyImage = std::move(other.mDestroyImage);
        mDeleteTextures = std::move(other.mDeleteTextures);
        mDisplay = std::move(other.mDisplay);
        mImage = std::move(other.mImage);
        mTexture = std::move(other.mTexture);
    }

    ~ScopedEGLImage() {
        if (mTexture != 0) {
            mDeleteTextures(1, &mTexture);
        }
        if (mImage != nullptr) {
            mDestroyImage(mDisplay, mImage);
        }
    }

    EGLImage getImage() const { return mImage; }

    GLuint getTexture() const { return mTexture; }

  private:
    PFNEGLDESTROYIMAGEPROC mDestroyImage = nullptr;
    PFNGLDELETETEXTURESPROC mDeleteTextures = nullptr;
    EGLDisplay mDisplay = nullptr;
    EGLImage mImage = nullptr;
    GLuint mTexture = 0;
};

class EGLImageTestBase : public DawnTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::DawnInternalUsages};
    }

    bool HasExtension(const char* string) {
        return strstr(egl.QueryString(egl.GetCurrentDisplay(), EGL_EXTENSIONS), string) != nullptr;
    }

    void SetUp() override {
        DawnTest::SetUp();
        // TODO(crbug.com/dawn/2206): remove this check if possible.
        DAWN_TEST_UNSUPPORTED_IF(!HasExtension("KHR_gl_texture_2D_image"));
        // TODO(crbug.com/388318201): EGL extension not available
        DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode() &&
                                 HasToggleEnabled("gl_force_es_31_and_no_extensions"));
    }

  public:
    ScopedEGLImage CreateEGLImage(uint32_t width,
                                  uint32_t height,
                                  GLenum internalFormat,
                                  GLenum format,
                                  GLenum type,
                                  void* data) {
        native::opengl::Device* openglDevice =
            native::opengl::ToBackend(native::FromAPI(device.Get()));
        const native::opengl::OpenGLFunctions& gl = openglDevice->GetGL();
        GLuint tex;
        gl.GenTextures(1, &tex);
        gl.BindTexture(GL_TEXTURE_2D, tex);
        gl.TexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, data);
        EGLAttrib attribs[1] = {EGL_NONE};
        EGLClientBuffer buffer = reinterpret_cast<EGLClientBuffer>(static_cast<intptr_t>(tex));
        EGLDisplay dpy = egl.GetCurrentDisplay();
        EGLContext ctx = egl.GetCurrentContext();
        EGLImage eglImage = egl.CreateImage(dpy, ctx, EGL_GL_TEXTURE_2D, buffer, attribs);
        EXPECT_NE(nullptr, eglImage);

        return ScopedEGLImage(egl.DestroyImage, gl.DeleteTextures, dpy, eglImage, tex);
    }
    wgpu::Texture WrapEGLImage(const wgpu::TextureDescriptor* descriptor,
                               EGLImage eglImage,
                               bool isInitialized = false) {
        native::opengl::ExternalImageDescriptorEGLImage externDesc;
        externDesc.cTextureDescriptor = reinterpret_cast<const WGPUTextureDescriptor*>(descriptor);
        externDesc.image = eglImage;
        externDesc.isInitialized = isInitialized;
        WGPUTexture texture = native::opengl::WrapExternalEGLImage(device.Get(), &externDesc);
        return wgpu::Texture::Acquire(texture);
    }
    EGLFunctions egl;
};

// A small fixture used to initialize default data for the EGLImage validation tests.
// These tests are skipped if the harness is using the wire.
class EGLImageValidationTests : public EGLImageTestBase {
  public:
    EGLImageValidationTests() {
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
        descriptor.size = {10, 10, 1};
        descriptor.sampleCount = 1;
        descriptor.mipLevelCount = 1;
        descriptor.usage = wgpu::TextureUsage::RenderAttachment;
    }

    ScopedEGLImage CreateDefaultEGLImage() {
        return CreateEGLImage(10, 10, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }

  protected:
    wgpu::TextureDescriptor descriptor;
};

// Test a successful wrapping of an EGLImage in a texture
TEST_P(EGLImageValidationTests, Success) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    ScopedEGLImage image = CreateDefaultEGLImage();
    wgpu::Texture texture = WrapEGLImage(&descriptor, image.getImage());
    ASSERT_NE(texture.Get(), nullptr);
}

// Test a successful wrapping of an EGLImage in a texture with DawnTextureInternalUsageDescriptor
TEST_P(EGLImageValidationTests, SuccessWithInternalUsageDescriptor) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    wgpu::DawnTextureInternalUsageDescriptor internalDesc = {};
    descriptor.nextInChain = &internalDesc;
    internalDesc.internalUsage = wgpu::TextureUsage::CopySrc;
    internalDesc.sType = wgpu::SType::DawnTextureInternalUsageDescriptor;

    ScopedEGLImage image = CreateDefaultEGLImage();
    wgpu::Texture texture = WrapEGLImage(&descriptor, image.getImage());
    ASSERT_NE(texture.Get(), nullptr);
}

// Test an error occurs if an invalid sType is the nextInChain
TEST_P(EGLImageValidationTests, InvalidTextureDescriptor) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());

    wgpu::ChainedStruct chainedDescriptor;
    chainedDescriptor.sType = wgpu::SType::SurfaceDescriptorFromWindowsSwapChainPanel;
    descriptor.nextInChain = &chainedDescriptor;

    ScopedEGLImage image = CreateDefaultEGLImage();
    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapEGLImage(&descriptor, image.getImage()));

    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor dimension isn't 2D
TEST_P(EGLImageValidationTests, InvalidTextureDimension) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    descriptor.dimension = wgpu::TextureDimension::e3D;

    ScopedEGLImage image = CreateDefaultEGLImage();
    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapEGLImage(&descriptor, image.getImage()));

    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor mip level count isn't 1
TEST_P(EGLImageValidationTests, InvalidMipLevelCount) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    descriptor.mipLevelCount = 2;

    ScopedEGLImage image = CreateDefaultEGLImage();
    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapEGLImage(&descriptor, image.getImage()));
    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor depth isn't 1
TEST_P(EGLImageValidationTests, InvalidDepth) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    descriptor.size.depthOrArrayLayers = 2;

    ScopedEGLImage image = CreateDefaultEGLImage();
    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapEGLImage(&descriptor, image.getImage()));
    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor sample count isn't 1
TEST_P(EGLImageValidationTests, InvalidSampleCount) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    descriptor.sampleCount = 4;

    ScopedEGLImage image = CreateDefaultEGLImage();
    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapEGLImage(&descriptor, image.getImage()));
    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor width doesn't match the surface's
TEST_P(EGLImageValidationTests, InvalidWidth) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    descriptor.size.width = 11;

    ScopedEGLImage image = CreateDefaultEGLImage();
    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapEGLImage(&descriptor, image.getImage()));
    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor height doesn't match the surface's
TEST_P(EGLImageValidationTests, InvalidHeight) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    descriptor.size.height = 11;

    ScopedEGLImage image = CreateDefaultEGLImage();
    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapEGLImage(&descriptor, image.getImage()));
    ASSERT_EQ(texture.Get(), nullptr);
}

// Fixture to test using EGLImages through different usages.
// These tests are skipped if the harness is using the wire.
class EGLImageUsageTests : public EGLImageTestBase {
  public:
    // Test that clearing using BeginRenderPass writes correct data in the eglImage.
    void DoClearTest(EGLImage eglImage,
                     GLuint texture,
                     wgpu::TextureFormat format,
                     GLenum glFormat,
                     GLenum glType,
                     void* data,
                     size_t dataSize) {
        native::opengl::Device* openglDevice =
            native::opengl::ToBackend(native::FromAPI(device.Get()));
        const native::opengl::OpenGLFunctions& gl = openglDevice->GetGL();

        // Get a texture view for the eglImage
        wgpu::TextureDescriptor textureDescriptor;
        textureDescriptor.dimension = wgpu::TextureDimension::e2D;
        textureDescriptor.format = format;
        textureDescriptor.size = {1, 1, 1};
        textureDescriptor.sampleCount = 1;
        textureDescriptor.mipLevelCount = 1;
        textureDescriptor.usage = wgpu::TextureUsage::RenderAttachment;

        wgpu::DawnTextureInternalUsageDescriptor internalDesc = {};
        textureDescriptor.nextInChain = &internalDesc;
        internalDesc.internalUsage = wgpu::TextureUsage::CopySrc;
        internalDesc.sType = wgpu::SType::DawnTextureInternalUsageDescriptor;

        wgpu::Texture eglImageTexture = WrapEGLImage(&textureDescriptor, eglImage);
        ASSERT_NE(eglImageTexture, nullptr);

        wgpu::TextureView eglImageView = eglImageTexture.CreateView();

        utils::ComboRenderPassDescriptor renderPassDescriptor({eglImageView}, {});
        renderPassDescriptor.cColorAttachments[0].clearValue = {1 / 255.0f, 2 / 255.0f, 3 / 255.0f,
                                                                4 / 255.0f};

        // Execute commands to clear the eglImage
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDescriptor);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        // Check the correct data was written
        std::vector<uint8_t> result(dataSize);
        GLuint fbo;
        gl.GenFramebuffers(1, &fbo);
        gl.BindFramebuffer(GL_FRAMEBUFFER, fbo);
        gl.FramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture,
                                0);
        gl.ReadPixels(0, 0, 1, 1, glFormat, glType, result.data());
        gl.BindFramebuffer(GL_FRAMEBUFFER, 0);
        gl.DeleteFramebuffers(1, &fbo);
        ASSERT_EQ(0, memcmp(result.data(), data, dataSize));
    }

    template <class T>
    void DoSampleTest(EGLImage image, wgpu::TextureFormat format, T* data) {
        // Get a texture view for the GL texture.
        wgpu::TextureDescriptor textureDescriptor;

        textureDescriptor.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc;
        textureDescriptor.size = {2, 1, 1};
        textureDescriptor.format = format;

        wgpu::Texture wrappedTexture = WrapEGLImage(&textureDescriptor, image, true);

        ASSERT_NE(wrappedTexture, nullptr);

        // Create a color attachment texture.
        wgpu::TextureDescriptor attachmentDescriptor;
        attachmentDescriptor.usage =
            wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
        attachmentDescriptor.size = {1, 1, 1};
        attachmentDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;

        wgpu::Texture attachment = device.CreateTexture(&attachmentDescriptor);

        utils::ComboRenderPassDescriptor renderPassDescriptor({attachment.CreateView()}, {});
        renderPassDescriptor.cColorAttachments[0].clearValue = {1.0f, 0.0f, 0.0f, 1.0f};

        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
            @vertex
            fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
                var pos = array(
                    vec2f(-3.0, -1.0),
                    vec2f( 3.0, -1.0),
                    vec2f( 0.0,  2.0));

                return vec4f(pos[VertexIndex], 0.0, 1.0);
            })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var myTexture : texture_2d<f32>;
            @group(0) @binding(1) var mySampler : sampler;
            struct FragmentOut {
               @location(0) color : vec4<f32>
            }
            @fragment
            fn main(@builtin(position) FragCoord : vec4f) -> FragmentOut {
                var output : FragmentOut;
                output.color = textureSample(myTexture, mySampler, FragCoord.xy);
                return output;
            })");

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;

        auto renderPipeline = device.CreateRenderPipeline(&descriptor);
        auto sampler = device.CreateSampler();
        auto bindGroup = utils::MakeBindGroup(device, renderPipeline.GetBindGroupLayout(0),
                                              {
                                                  {0, wrappedTexture.CreateView()},
                                                  {1, sampler},
                                              });

        // Execute commands to sample the wrapped texture
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDescriptor);
        pass.SetPipeline(renderPipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.Draw(6, 1, 0, 0);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        // Check that the source texture contains the expected data
        EXPECT_TEXTURE_EQ(data, wrappedTexture, {0, 0}, {1, 1});

        // Check that the expected data was sampled from the wrapped texture into the attachment.
        EXPECT_TEXTURE_EQ(data, attachment, {0, 0}, {1, 1});
    }
};

// Test clearing a R8 EGLImage
TEST_P(EGLImageUsageTests, ClearR8EGLImage) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    ScopedEGLImage eglImage = CreateEGLImage(1, 1, GL_R8, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    uint8_t data = 0x01;
    DoClearTest(eglImage.getImage(), eglImage.getTexture(), wgpu::TextureFormat::R8Unorm, GL_RED,
                GL_UNSIGNED_BYTE, &data, sizeof(data));
}

// Test clearing a RG8 EGLImage
TEST_P(EGLImageUsageTests, ClearRG8EGLImage) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    ScopedEGLImage eglImage = CreateEGLImage(1, 1, GL_RG8, GL_RG, GL_UNSIGNED_BYTE, nullptr);

    uint16_t data = 0x0201;
    DoClearTest(eglImage.getImage(), eglImage.getTexture(), wgpu::TextureFormat::RG8Unorm, GL_RG,
                GL_UNSIGNED_BYTE, &data, sizeof(data));
}

// Test clearing an RGBA8 EGLImage
TEST_P(EGLImageUsageTests, ClearRGBA8EGLImage) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    ScopedEGLImage eglImage = CreateEGLImage(1, 1, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    uint32_t data = 0x04030201;
    DoClearTest(eglImage.getImage(), eglImage.getTexture(), wgpu::TextureFormat::RGBA8Unorm,
                GL_RGBA, GL_UNSIGNED_BYTE, &data, sizeof(data));
}

// Test sampling an imported R8 GL texture
TEST_P(EGLImageUsageTests, SampleR8EGLImage) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    uint8_t data[2] = {0x42, 0x42};
    ScopedEGLImage eglImage = CreateEGLImage(2, 1, GL_R8, GL_RED, GL_UNSIGNED_BYTE, data);

    DoSampleTest(eglImage.getImage(), wgpu::TextureFormat::R8Unorm, data);
}

// Test sampling an imported RG8 GL texture
TEST_P(EGLImageUsageTests, SampleRG8EGLImage) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    uint16_t data[2] = {0x4221, 0x4221};
    ScopedEGLImage eglImage = CreateEGLImage(2, 1, GL_RG8, GL_RG, GL_UNSIGNED_BYTE, data);

    DoSampleTest(eglImage.getImage(), wgpu::TextureFormat::RG8Unorm, data);
}

// Test sampling an imported RGBA8 GL texture
TEST_P(EGLImageUsageTests, SampleRGBA8EGLImage) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    uint32_t data[2] = {0x48844221, 0x48844221};
    ScopedEGLImage eglImage = CreateEGLImage(2, 1, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, data);

    DoSampleTest(eglImage.getImage(), wgpu::TextureFormat::RGBA8Unorm, data);
}

DAWN_INSTANTIATE_TEST(EGLImageValidationTests, OpenGLESBackend());
DAWN_INSTANTIATE_TEST(EGLImageUsageTests, OpenGLESBackend());

}  // anonymous namespace
}  // namespace dawn

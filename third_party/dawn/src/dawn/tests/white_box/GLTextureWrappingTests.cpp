// Copyright 2023 The Dawn & Tint Authors
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

#include <utility>
#include <vector>

#include "dawn/common/DynamicLib.h"
#include "dawn/native/OpenGLBackend.h"
#include "dawn/native/opengl/DeviceGL.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn {
namespace {

class ScopedGLTexture {
  public:
    ScopedGLTexture(PFNGLDELETETEXTURESPROC deleteTextures, GLuint texture)
        : mDeleteTextures(deleteTextures), mTexture(texture) {}

    ScopedGLTexture(ScopedGLTexture&& other) {
        if (mTexture != 0) {
            mDeleteTextures(1, &mTexture);
        }
        mDeleteTextures = std::move(other.mDeleteTextures);
        mTexture = std::move(other.mTexture);
    }

    ~ScopedGLTexture() {
        if (mTexture != 0) {
            mDeleteTextures(1, &mTexture);
        }
    }

    GLuint Get() const { return mTexture; }

  private:
    PFNGLDELETETEXTURESPROC mDeleteTextures = nullptr;
    GLuint mTexture = 0;
};

class GLTextureTestBase : public DawnTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        if (SupportsFeatures({wgpu::FeatureName::ANGLETextureSharing})) {
            return {
                wgpu::FeatureName::DawnInternalUsages,
                wgpu::FeatureName::ANGLETextureSharing,
            };
        } else {
            return {
                wgpu::FeatureName::DawnInternalUsages,
            };
        }
    }

    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures({wgpu::FeatureName::ANGLETextureSharing}));
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());

        // Create a second GL device from which we can vend textures.
        mSecondDevice = CreateDevice();
        mSecondDeviceGL = native::opengl::ToBackend(native::FromAPI(mSecondDevice.Get()));
    }

  public:
    ScopedGLTexture CreateGLTexture(uint32_t width,
                                    uint32_t height,
                                    GLenum internalFormat,
                                    GLenum format,
                                    GLenum type,
                                    void* data) {
        const native::opengl::OpenGLFunctions& gl = mSecondDeviceGL->GetGL();
        GLuint tex;
        gl.GenTextures(1, &tex);
        gl.BindTexture(GL_TEXTURE_2D, tex);
        gl.TexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, data);

        return ScopedGLTexture(gl.DeleteTextures, tex);
    }
    wgpu::Texture WrapGLTexture(const wgpu::TextureDescriptor* descriptor,
                                GLuint texture,
                                bool isInitialized = false) {
        native::opengl::ExternalImageDescriptorGLTexture externDesc;
        externDesc.cTextureDescriptor = reinterpret_cast<const WGPUTextureDescriptor*>(descriptor);
        externDesc.isInitialized = isInitialized;
        externDesc.texture = texture;
        return wgpu::Texture::Acquire(
            native::opengl::WrapExternalGLTexture(device.Get(), &externDesc));
    }

  protected:
    wgpu::Device mSecondDevice;
    raw_ptr<native::opengl::Device> mSecondDeviceGL;  // Depends on `mSecondDevice`.
};

// A small fixture used to initialize default data for the GLTexture validation tests.
// These tests are skipped if the harness is using the wire.
class GLTextureValidationTests : public GLTextureTestBase {
  public:
    GLTextureValidationTests() {
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
        descriptor.size = {10, 10, 1};
        descriptor.sampleCount = 1;
        descriptor.mipLevelCount = 1;
        descriptor.usage = wgpu::TextureUsage::RenderAttachment;
    }

    ScopedGLTexture CreateDefaultGLTexture() {
        return CreateGLTexture(10, 10, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }

  protected:
    wgpu::TextureDescriptor descriptor;
};

class GLTextureValidationNoANGLETextureSharingTests : public GLTextureValidationTests {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::DawnInternalUsages};
    }
};

// Test a successful wrapping of a GL texture in a WebGPU texture
TEST_P(GLTextureValidationTests, Success) {
    ScopedGLTexture glTexture = CreateDefaultGLTexture();
    wgpu::Texture texture = WrapGLTexture(&descriptor, glTexture.Get());
    ASSERT_NE(texture.Get(), nullptr);
}

// Test an unsuccessful wrapping of a GL texture in a WebGPU texture when
// the device does not support the ANGLETextureSharing Feature
TEST_P(GLTextureValidationNoANGLETextureSharingTests, Failure) {
    ScopedGLTexture glTexture = CreateDefaultGLTexture();
    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapGLTexture(&descriptor, glTexture.Get()));
    ASSERT_EQ(texture.Get(), nullptr);
}

// Test a successful wrapping of a GL texture in a texture with DawnTextureInternalUsageDescriptor
TEST_P(GLTextureValidationTests, SuccessWithInternalUsageDescriptor) {
    wgpu::DawnTextureInternalUsageDescriptor internalDesc = {};
    descriptor.nextInChain = &internalDesc;
    internalDesc.internalUsage = wgpu::TextureUsage::CopySrc;
    internalDesc.sType = wgpu::SType::DawnTextureInternalUsageDescriptor;

    ScopedGLTexture glTexture = CreateDefaultGLTexture();
    wgpu::Texture texture = WrapGLTexture(&descriptor, glTexture.Get());
    ASSERT_NE(texture.Get(), nullptr);
}

// Test an error occurs if an invalid sType is the nextInChain
TEST_P(GLTextureValidationTests, InvalidTextureDescriptor) {
    wgpu::ChainedStruct chainedDescriptor;
    chainedDescriptor.sType = wgpu::SType::SurfaceDescriptorFromWindowsUWPSwapChainPanel;
    descriptor.nextInChain = &chainedDescriptor;

    ScopedGLTexture glTexture = CreateDefaultGLTexture();
    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapGLTexture(&descriptor, glTexture.Get()));

    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor dimension isn't 2D
TEST_P(GLTextureValidationTests, InvalidTextureDimension) {
    descriptor.dimension = wgpu::TextureDimension::e3D;

    ScopedGLTexture glTexture = CreateDefaultGLTexture();
    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapGLTexture(&descriptor, glTexture.Get()));

    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor mip level count isn't 1
TEST_P(GLTextureValidationTests, InvalidMipLevelCount) {
    descriptor.mipLevelCount = 2;

    ScopedGLTexture glTexture = CreateDefaultGLTexture();
    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapGLTexture(&descriptor, glTexture.Get()));
    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor depth isn't 1
TEST_P(GLTextureValidationTests, InvalidDepth) {
    descriptor.size.depthOrArrayLayers = 2;

    ScopedGLTexture glTexture = CreateDefaultGLTexture();
    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapGLTexture(&descriptor, glTexture.Get()));
    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor sample count isn't 1
TEST_P(GLTextureValidationTests, InvalidSampleCount) {
    descriptor.sampleCount = 4;

    ScopedGLTexture glTexture = CreateDefaultGLTexture();
    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapGLTexture(&descriptor, glTexture.Get()));
    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor width doesn't match the surface's
TEST_P(GLTextureValidationTests, InvalidWidth) {
    descriptor.size.width = 11;

    ScopedGLTexture glTexture = CreateDefaultGLTexture();
    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapGLTexture(&descriptor, glTexture.Get()));
    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor height doesn't match the surface's
TEST_P(GLTextureValidationTests, InvalidHeight) {
    descriptor.size.height = 11;

    ScopedGLTexture glTexture = CreateDefaultGLTexture();
    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapGLTexture(&descriptor, glTexture.Get()));
    ASSERT_EQ(texture.Get(), nullptr);
}

// Fixture to test using GL textures through different usages.
// These tests are skipped if the harness is using the wire.
class GLTextureUsageTests : public GLTextureTestBase {
  public:
    // Test that clearing using BeginRenderPass writes correct data in the GL texture.
    void DoClearTest(GLuint texture,
                     wgpu::TextureFormat format,
                     GLenum glFormat,
                     GLenum glType,
                     void* data,
                     size_t dataSize) {
        const native::opengl::OpenGLFunctions& gl = mSecondDeviceGL->GetGL();

        // Get a texture view for the GL texture.
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

        wgpu::Texture wrappedTexture = WrapGLTexture(&textureDescriptor, texture);
        ASSERT_NE(wrappedTexture, nullptr);

        wgpu::TextureView wrappedView = wrappedTexture.CreateView();

        utils::ComboRenderPassDescriptor renderPassDescriptor({wrappedView}, {});
        renderPassDescriptor.cColorAttachments[0].clearValue = {1 / 255.0f, 2 / 255.0f, 3 / 255.0f,
                                                                4 / 255.0f};

        // Execute commands to clear the wrapped texture
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
    void DoSampleTest(GLuint texture, wgpu::TextureFormat format, T* data) {
        // Get a texture view for the GL texture.
        wgpu::TextureDescriptor textureDescriptor;

        textureDescriptor.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc;
        textureDescriptor.size = {1, 1, 1};
        textureDescriptor.format = format;

        wgpu::Texture wrappedTexture = WrapGLTexture(&textureDescriptor, texture, true);

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

// Test clearing a R8 GL texture
TEST_P(GLTextureUsageTests, ClearR8GLTexture) {
    ScopedGLTexture glTexture = CreateGLTexture(1, 1, GL_R8, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    uint8_t data = 0x01;
    DoClearTest(glTexture.Get(), wgpu::TextureFormat::R8Unorm, GL_RED, GL_UNSIGNED_BYTE, &data,
                sizeof(data));
}

// Test clearing a RG8 GL texture
TEST_P(GLTextureUsageTests, ClearRG8GLTexture) {
    ScopedGLTexture glTexture = CreateGLTexture(1, 1, GL_RG8, GL_RG, GL_UNSIGNED_BYTE, nullptr);

    uint16_t data = 0x0201;
    DoClearTest(glTexture.Get(), wgpu::TextureFormat::RG8Unorm, GL_RG, GL_UNSIGNED_BYTE, &data,
                sizeof(data));
}

// Test clearing an RGBA8 GL texture
TEST_P(GLTextureUsageTests, ClearRGBA8GLTexture) {
    ScopedGLTexture glTexture = CreateGLTexture(1, 1, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    uint32_t data = 0x04030201;
    DoClearTest(glTexture.Get(), wgpu::TextureFormat::RGBA8Unorm, GL_RGBA, GL_UNSIGNED_BYTE, &data,
                sizeof(data));
}

// Test sampling an imported R8 GL texture
TEST_P(GLTextureUsageTests, SampleR8GLTexture) {
    uint8_t data = 0x42;
    ScopedGLTexture glTexture = CreateGLTexture(1, 1, GL_R8, GL_RED, GL_UNSIGNED_BYTE, &data);

    DoSampleTest(glTexture.Get(), wgpu::TextureFormat::RGBA8Unorm, &data);
}

// Test sampling an imported RG8 GL texture
TEST_P(GLTextureUsageTests, SampleRG8GLTexture) {
    uint16_t data = 0x4221;
    ScopedGLTexture glTexture = CreateGLTexture(1, 1, GL_RG8, GL_RG, GL_UNSIGNED_BYTE, &data);

    DoSampleTest(glTexture.Get(), wgpu::TextureFormat::RGBA8Unorm, &data);
}

// Test sampling an imported RGBA8 GL texture
TEST_P(GLTextureUsageTests, SampleRGBA8GLTexture) {
    uint32_t data = 0x48844221;
    ScopedGLTexture glTexture = CreateGLTexture(1, 1, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, &data);

    DoSampleTest(glTexture.Get(), wgpu::TextureFormat::RGBA8Unorm, &data);
}

DAWN_INSTANTIATE_TEST(GLTextureValidationTests, OpenGLESBackend());
DAWN_INSTANTIATE_TEST(GLTextureValidationNoANGLETextureSharingTests, OpenGLESBackend());
DAWN_INSTANTIATE_TEST(GLTextureUsageTests, OpenGLESBackend());

}  // anonymous namespace
}  // namespace dawn

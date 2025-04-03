// Copyright 2024 The Dawn & Tint Authors
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

#include <memory>
#include <string>
#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderBundleEncoderDescriptor.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"
#include "webgpu/webgpu_glfw.h"

#include "GLFW/glfw3.h"

namespace dawn {
namespace {

struct GLFWindowDestroyer {
    void operator()(GLFWwindow* ptr) { glfwDestroyWindow(ptr); }
};

class SurfaceTests : public DawnTest {
  protected:
    wgpu::Limits GetRequiredLimits(const wgpu::Limits& supported) override {
        // Just copy all the limits, though all we really care about is
        // maxStorageBuffersInFragmentStage
        return supported;
    }

  public:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());

        // TODO(crbug.com/dawn/2531): Failing on newer Linux/Intel driver version.
        // However, IsIntel() and IsMesa() don't work with the null backend.
        DAWN_SUPPRESS_TEST_IF(IsLinux() && IsNull());
        DAWN_SUPPRESS_TEST_IF(IsLinux() && IsVulkan() && IsIntel() && IsMesa("23.2"));

        glfwSetErrorCallback([](int code, const char* message) {
            ErrorLog() << "GLFW error " << code << " " << message;
        });

        // GLFW can fail to start in headless environments, in which SurfaceTests are
        // inapplicable. Skip this cases without producing a test failure.
        if (glfwInit() == GLFW_FALSE) {
            GTEST_SKIP();
        }

        // Set GLFW_NO_API to avoid GLFW bringing up a GL context that we won't use.
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window.reset(glfwCreateWindow(400, 500, "SurfaceTests window", nullptr, nullptr));

        int width;
        int height;
        glfwGetFramebufferSize(window.get(), &width, &height);

        baseConfig.device = device;
        baseConfig.width = width;
        baseConfig.height = height;
        baseConfig.usage = wgpu::TextureUsage::RenderAttachment;
        baseConfig.viewFormatCount = 0;
        baseConfig.viewFormats = nullptr;
    }

    void TearDown() override {
        // Destroy the surface before the window as required by webgpu-native.
        window.reset();
        DawnTest::TearDown();
    }

    wgpu::Surface CreateTestSurface() {
        return wgpu::glfw::CreateSurfaceForWindow(GetInstance(), window.get());
    }

    wgpu::SurfaceConfiguration GetPreferredConfiguration(wgpu::Surface surface) {
        wgpu::SurfaceCapabilities capabilities;
        surface.GetCapabilities(adapter, &capabilities);

        wgpu::SurfaceConfiguration config = baseConfig;
        config.format = capabilities.formats[0];
        config.alphaMode = capabilities.alphaModes[0];
        config.presentMode = capabilities.presentModes[0];
        return config;
    }

    void ClearTexture(wgpu::Texture texture,
                      wgpu::Color color,
                      wgpu::Device preferredDevice = nullptr) {
        if (preferredDevice == nullptr) {
            preferredDevice = device;
        }

        utils::ComboRenderPassDescriptor desc({texture.CreateView()});
        desc.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
        desc.cColorAttachments[0].clearValue = color;

        wgpu::CommandEncoder encoder = preferredDevice.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&desc);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        preferredDevice.GetQueue().Submit(1, &commands);
    }

    void SampleLoadTexture(wgpu::Texture texture, utils::RGBA8 expectedColor) {
        LoadTexture(texture, expectedColor, "texture_2d<f32>", ", 0");
    }

    void StorageLoadTexture(wgpu::Texture texture, utils::RGBA8 expectedColor) {
        LoadTexture(texture, expectedColor,
                    "texture_storage_2d<" +
                        std::string(utils::GetWGSLImageFormatQualifier(texture.GetFormat())) +
                        ", read>",
                    "");
    }

    void LoadTexture(wgpu::Texture texture,
                     utils::RGBA8 expectedColor,
                     std::string wgslType,
                     std::string extraLoadArgs) {
        wgpu::TextureDescriptor texDescriptor;
        texDescriptor.size = {texture.GetWidth(), texture.GetHeight()};
        texDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
        texDescriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
        wgpu::Texture dstTexture = device.CreateTexture(&texDescriptor);

        // Create the storage load blit render pipeline.
        wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
            @vertex fn vs(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
                var pos = array(
                    vec2f(-1.0, -3.0),
                    vec2f(-1.0,  3.0),
                    vec2f( 2.0,  0.0)
                );
                return vec4f(pos[VertexIndex], 0.0, 1.0);
            }

            @group(0) @binding(0) var texture : )" + wgslType + R"(;
            @fragment fn fs(@builtin(position) coord: vec4f) -> @location(0) vec4f {
                return textureLoad(texture, vec2i(coord.xy))" + extraLoadArgs +
                                                                          R"();
            }
        )");

        utils::ComboRenderPipelineDescriptor pipelineDesc;
        pipelineDesc.vertex.module = module;
        pipelineDesc.cFragment.module = module;
        pipelineDesc.cTargets[0].format = texDescriptor.format;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);

        // Submit the commands for the blit.
        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                         {{0, texture.CreateView()}});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPassInfo({dstTexture.CreateView()});
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.Draw(6);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_TEXTURE_EQ(expectedColor, dstTexture, {0, 0});
        EXPECT_TEXTURE_EQ(expectedColor, dstTexture,
                          {texture.GetWidth() - 1, texture.GetHeight() - 1});
    }

    bool SupportsPresentMode(const wgpu::SurfaceCapabilities& capabilities,
                             wgpu::PresentMode mode) {
        for (size_t i = 0; i < capabilities.presentModeCount; ++i) {
            if (capabilities.presentModes[i] == mode) {
                return true;
            }
        }
        return false;
    }

  protected:
    std::unique_ptr<GLFWwindow, GLFWindowDestroyer> window = nullptr;
    wgpu::SurfaceConfiguration baseConfig;
};

// Basic test for creating a surface and presenting one frame.
TEST_P(SurfaceTests, Basic) {
    wgpu::Surface surface = CreateTestSurface();

    // Configure
    wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);
    surface.Configure(&config);

    // Get texture
    wgpu::SurfaceTexture surfaceTexture;
    surface.GetCurrentTexture(&surfaceTexture);
    ASSERT_EQ(surfaceTexture.status, wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal);
    ClearTexture(surfaceTexture.texture, {1.0, 0.0, 0.0, 1.0});

    // Present
    surface.Present();
}

// Test reconfiguring the surface
TEST_P(SurfaceTests, ReconfigureBasic) {
    wgpu::Surface surface = CreateTestSurface();
    wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);

    surface.Configure(&config);

    surface.Configure(&config);
}

// Test reconfiguring the surface after GetCurrentTexture
TEST_P(SurfaceTests, ReconfigureAfterGetCurrentTexture) {
    wgpu::Surface surface = CreateTestSurface();
    wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);

    {
        surface.Configure(&config);
        wgpu::SurfaceTexture surfaceTexture;
        surface.GetCurrentTexture(&surfaceTexture);
        ClearTexture(surfaceTexture.texture, {1.0, 0.0, 0.0, 1.0});
    }

    {
        surface.Configure(&config);
        wgpu::SurfaceTexture surfaceTexture;
        surface.GetCurrentTexture(&surfaceTexture);
        ClearTexture(surfaceTexture.texture, {0.0, 1.0, 0.0, 1.0});
        surface.Present();
    }
}

// Test unconfiguring then reconfiguring the surface
TEST_P(SurfaceTests, ReconfigureAfterUnconfigure) {
    wgpu::Surface surface = CreateTestSurface();
    wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);

    {
        surface.Configure(&config);
        wgpu::SurfaceTexture surfaceTexture;
        surface.GetCurrentTexture(&surfaceTexture);
        ClearTexture(surfaceTexture.texture, {1.0, 0.0, 0.0, 1.0});
        surface.Present();
    }

    surface.Unconfigure();

    {
        surface.Configure(&config);
        wgpu::SurfaceTexture surfaceTexture;
        surface.GetCurrentTexture(&surfaceTexture);
        ClearTexture(surfaceTexture.texture, {0.0, 1.0, 0.0, 1.0});
        surface.Present();
    }
}

// Test unconfiguring after GetCurrentTexture but before the Present
TEST_P(SurfaceTests, UnconfigureAfterGet) {
    wgpu::Surface surface = CreateTestSurface();
    wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);
    wgpu::SurfaceTexture surfaceTexture;

    surface.Configure(&config);
    surface.GetCurrentTexture(&surfaceTexture);
    ClearTexture(surfaceTexture.texture, {1.0, 0.0, 0.0, 1.0});

    surface.Unconfigure();
}
// Test switching between surfaces that have different present modes.
TEST_P(SurfaceTests, SwitchPresentMode) {
    // Fails with "internal drawable creation failed" on the Windows NVIDIA CQ builders but not
    // locally.
    DAWN_SUPPRESS_TEST_IF(IsWindows() && IsVulkan() && IsNvidia());

    // TODO(jiawei.shao@intel.com): find out why this test sometimes hangs on the latest Linux Intel
    // Vulkan drivers.
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsVulkan() && IsIntel());

    // crbug.com/358166481
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsNvidia() && IsVulkan());

    constexpr wgpu::PresentMode kAllPresentModes[] = {
        wgpu::PresentMode::Immediate,
        wgpu::PresentMode::Fifo,
        wgpu::PresentMode::Mailbox,
    };

    wgpu::Surface surface = CreateTestSurface();

    wgpu::SurfaceCapabilities capabilities;
    surface.GetCapabilities(adapter, &capabilities);

    for (wgpu::PresentMode mode1 : kAllPresentModes) {
        if (!SupportsPresentMode(capabilities, mode1)) {
            continue;
        }
        for (wgpu::PresentMode mode2 : kAllPresentModes) {
            if (!SupportsPresentMode(capabilities, mode2)) {
                continue;
            }

            wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);

            {
                config.presentMode = mode1;
                surface.Configure(&config);

                wgpu::SurfaceTexture surfaceTexture;
                surface.GetCurrentTexture(&surfaceTexture);
                ClearTexture(surfaceTexture.texture, {0.0, 0.0, 0.0, 1.0});
                surface.Present();
            }

            {
                config.presentMode = mode2;
                surface.Configure(&config);

                wgpu::SurfaceTexture surfaceTexture;
                surface.GetCurrentTexture(&surfaceTexture);
                ClearTexture(surfaceTexture.texture, {0.0, 0.0, 0.0, 1.0});
                surface.Present();
                surface.Unconfigure();
            }
        }
    }
}

// Test resizing the surface and without resizing the window.
TEST_P(SurfaceTests, ResizingSurfaceOnly) {
    wgpu::Surface surface = CreateTestSurface();

    for (int i = 0; i < 10; i++) {
        wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);
        config.width += i * 10;
        config.height -= i * 10;

        surface.Configure(&config);
        wgpu::SurfaceTexture surfaceTexture;
        surface.GetCurrentTexture(&surfaceTexture);
        ClearTexture(surfaceTexture.texture, {0.05f * i, 0.0, 0.0, 1.0});
        surface.Present();
    }
}

// Test resizing the window but not the surface.
TEST_P(SurfaceTests, ResizingWindowOnly) {
    // Hangs on NVIDIA GTX 1660
    DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsNvidia());

    wgpu::Surface surface = CreateTestSurface();
    wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);

    surface.Configure(&config);

    for (int i = 0; i < 10; i++) {
        glfwSetWindowSize(window.get(), 400 - 10 * i, 400 + 10 * i);
        glfwPollEvents();

        wgpu::SurfaceTexture surfaceTexture;
        surface.GetCurrentTexture(&surfaceTexture);
        ClearTexture(surfaceTexture.texture, {0.05f * i, 0.0, 0.0, 1.0});
        surface.Present();
    }
}

// Test resizing both the window and the surface at the same time.
TEST_P(SurfaceTests, ResizingWindowAndSurface) {
    // TODO(crbug.com/dawn/1205): Currently failing on new NVIDIA GTX 1660s on Linux/Vulkan.
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsVulkan() && IsNvidia());

    wgpu::Surface surface = CreateTestSurface();

    for (int i = 0; i < 10; i++) {
        glfwSetWindowSize(window.get(), 400 - 10 * i, 400 + 10 * i);
        glfwPollEvents();

        int width;
        int height;
        glfwGetFramebufferSize(window.get(), &width, &height);

        wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);
        config.width = width;
        config.height = height;
        surface.Configure(&config);

        wgpu::SurfaceTexture surfaceTexture;
        surface.GetCurrentTexture(&surfaceTexture);
        ClearTexture(surfaceTexture.texture, {0.05f * i, 0.0, 0.0, 1.0});
        surface.Present();
    }
}

// Test switching devices on the same adapter.
TEST_P(SurfaceTests, SwitchingDevice) {
    // TODO(https://crbug.com/dawn/2116): VVLs crash because oldSwapchain is from a different
    // device. The spec says it is ok if it is only the same instance but clarifications yet to be
    // published by Khronos are that the spec is wrong and should require the swapchain to be from
    // the same device. Suppress on all Vulkan devices while a proper fix is done.
    DAWN_SUPPRESS_TEST_IF(IsVulkan());

    // TODO(dawn:269): This isn't implemented yet but could be supported in the future.
    DAWN_SUPPRESS_TEST_IF(IsD3D11() || IsD3D12());

    wgpu::Device device2 = CreateDevice();

    wgpu::Surface surface = CreateTestSurface();

    wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);

    for (int i = 0; i < 3; i++) {
        wgpu::Device deviceToUse;
        if (i % 2 == 0) {
            deviceToUse = device;
        } else {
            deviceToUse = device2;
        }

        config.device = deviceToUse;
        surface.Configure(&config);
        wgpu::SurfaceTexture surfaceTexture;
        surface.GetCurrentTexture(&surfaceTexture);
        ClearTexture(surfaceTexture.texture, {0.0, 1.0, 0.0, 1.0}, deviceToUse);
        surface.Present();
    }
}

// Getting current texture without configuring returns an invalid surface texture
// It cannot raise a device error at this stage since it has never been configured with a device
TEST_P(SurfaceTests, GetWithoutConfigure) {
    wgpu::Surface surface = CreateTestSurface();

    wgpu::SurfaceTexture surfaceTexture;
    surface.GetCurrentTexture(&surfaceTexture);
    EXPECT_EQ(surfaceTexture.status, wgpu::SurfaceGetCurrentTextureStatus::Error);
}

// Getting current texture after unconfiguring fails
TEST_P(SurfaceTests, GetAfterUnconfigure) {
    wgpu::Surface surface = CreateTestSurface();
    wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);
    surface.Configure(&config);

    surface.Unconfigure();

    wgpu::SurfaceTexture surfaceTexture;
    ASSERT_DEVICE_ERROR(surface.GetCurrentTexture(&surfaceTexture));
    EXPECT_EQ(surfaceTexture.status, wgpu::SurfaceGetCurrentTextureStatus::Error);
}

// Getting current texture after losing the device should appear as if we got a texture.
TEST_P(SurfaceTests, GetAfterDeviceLoss) {
    wgpu::Surface surface = CreateTestSurface();
    wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);
    surface.Configure(&config);

    LoseDeviceForTesting();

    wgpu::SurfaceTexture surfaceTexture;
    surface.GetCurrentTexture(&surfaceTexture);
    EXPECT_EQ(surfaceTexture.status, wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal);
}

// Presenting without configuring fails
TEST_P(SurfaceTests, PresentWithoutConfigure) {
    wgpu::Surface surface = CreateTestSurface();
    // TODO(dawn:2320): This cannot throw a device error since the surface is
    // not aware of the device at this stage.
    /*ASSERT_DEVICE_ERROR(*/ surface.Present() /*)*/;
}

// Presenting after unconfiguring fails
TEST_P(SurfaceTests, PresentAfterUnconfigure) {
    wgpu::Surface surface = CreateTestSurface();
    wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);

    surface.Configure(&config);

    surface.Unconfigure();

    ASSERT_DEVICE_ERROR(surface.Present());
}

// Presenting without getting current texture first fails
TEST_P(SurfaceTests, PresentWithoutGet) {
    wgpu::Surface surface = CreateTestSurface();
    wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);

    surface.Configure(&config);
    ASSERT_DEVICE_ERROR(surface.Present());
}

// Check that all surfaces must support RenderAttachment.
TEST_P(SurfaceTests, RenderAttachmentAlwaysSupported) {
    wgpu::Surface surface = CreateTestSurface();
    wgpu::SurfaceCapabilities caps;
    surface.GetCapabilities(adapter, &caps);

    ASSERT_TRUE(caps.usages & wgpu::TextureUsage::RenderAttachment);
}

// Test sampling from the surface when it is supported.
TEST_P(SurfaceTests, Sampling) {
    wgpu::Surface surface = CreateTestSurface();
    wgpu::SurfaceCapabilities caps;
    surface.GetCapabilities(adapter, &caps);

    // Skip all tests if readable surface doesn't support texture binding
    DAWN_TEST_UNSUPPORTED_IF(!(caps.usages & wgpu::TextureUsage::TextureBinding));

    wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);
    config.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment;
    surface.Configure(&config);

    // Clear and sample from the texture
    wgpu::SurfaceTexture t;
    surface.GetCurrentTexture(&t);
    ClearTexture(t.texture, {1.0, 0.0, 0.0, 1.0});
    SampleLoadTexture(t.texture, utils::RGBA8::kRed);

    surface.Present();
}

// Test copying from the surface when it is supported.
TEST_P(SurfaceTests, CopyFrom) {
    wgpu::Surface surface = CreateTestSurface();
    wgpu::SurfaceCapabilities caps;
    surface.GetCapabilities(adapter, &caps);

    // Skip all tests if readable surface doesn't support copy src
    DAWN_TEST_UNSUPPORTED_IF(!(caps.usages & wgpu::TextureUsage::CopySrc));

    wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);
    config.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment;
    surface.Configure(&config);

    // Clear and copy from the texture
    wgpu::SurfaceTexture t;
    surface.GetCurrentTexture(&t);
    ClearTexture(t.texture, {1.0, 0.0, 0.0, 1.0});

    if (t.texture.GetFormat() == wgpu::TextureFormat::BGRA8Unorm ||
        t.texture.GetFormat() == wgpu::TextureFormat::BGRA8UnormSrgb) {
        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kBlue, t.texture, 0, 0);
    } else {
        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, t.texture, 0, 0);
    }

    surface.Present();
}

// Test copying to the surface when it is supported.
TEST_P(SurfaceTests, CopyTo) {
    wgpu::Surface surface = CreateTestSurface();
    wgpu::SurfaceCapabilities caps;
    surface.GetCapabilities(adapter, &caps);

    // Skip all tests if readable surface doesn't support copy dst or if we can't read back.
    DAWN_TEST_UNSUPPORTED_IF(!(caps.usages & wgpu::TextureUsage::CopyDst));
    DAWN_TEST_UNSUPPORTED_IF(
        !(caps.usages & (wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding)));

    wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);
    config.usage = caps.usages & (wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst |
                                  wgpu::TextureUsage::TextureBinding);
    config.width = 1;
    config.height = 1;
    surface.Configure(&config);

    // Write to the texture and read it back somehow.
    wgpu::SurfaceTexture t;
    surface.GetCurrentTexture(&t);

    wgpu::Extent3D writeSize = {1, 1, 1};
    wgpu::TexelCopyTextureInfo dest = {};
    dest.texture = t.texture;
    wgpu::TexelCopyBufferLayout dataLayout = {};
    queue.WriteTexture(&dest, &utils::RGBA8::kRed, sizeof(utils::RGBA8), &dataLayout, &writeSize);

    if (t.texture.GetUsage() & wgpu::TextureUsage::TextureBinding) {
        if (t.texture.GetFormat() == wgpu::TextureFormat::BGRA8Unorm ||
            t.texture.GetFormat() == wgpu::TextureFormat::BGRA8UnormSrgb) {
            SampleLoadTexture(t.texture, utils::RGBA8::kBlue);
        } else {
            SampleLoadTexture(t.texture, utils::RGBA8::kRed);
        }
    }

    if (t.texture.GetUsage() & wgpu::TextureUsage::CopySrc) {
        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, t.texture, 0, 0);
    }

    surface.Present();
}

// Test using the surface as a storage texture when supported.
TEST_P(SurfaceTests, Storage) {
    DAWN_SUPPRESS_TEST_IF(GetSupportedLimits().maxStorageBuffersInFragmentStage < 1);
    wgpu::Surface surface = CreateTestSurface();
    wgpu::SurfaceCapabilities caps;
    surface.GetCapabilities(adapter, &caps);

    // Skip all tests if readable surface doesn't support storage or if we can't find a storage-read
    // capable format.
    DAWN_TEST_UNSUPPORTED_IF(!(caps.usages & wgpu::TextureUsage::StorageBinding));

    wgpu::TextureFormat storageCapableFormat = wgpu::TextureFormat::Undefined;
    for (uint32_t i = 0; i < caps.formatCount; i++) {
        if (utils::TextureFormatSupportsStorageTexture(caps.formats[i], device, false)) {
            storageCapableFormat = caps.formats[i];
            break;
        }
    }
    DAWN_TEST_UNSUPPORTED_IF(storageCapableFormat == wgpu::TextureFormat::Undefined);

    wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);
    config.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageBinding;
    config.format = storageCapableFormat;
    surface.Configure(&config);

    // Clear and storage load from the texture
    wgpu::SurfaceTexture t;
    surface.GetCurrentTexture(&t);
    ClearTexture(t.texture, {1.0, 0.0, 0.0, 1.0});
    StorageLoadTexture(t.texture, utils::RGBA8::kRed);

    surface.Present();
}

DAWN_INSTANTIATE_TEST(SurfaceTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn

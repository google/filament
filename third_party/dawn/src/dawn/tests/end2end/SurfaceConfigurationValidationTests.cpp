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

#include <cmath>
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

namespace dawn::utils {
static constexpr std::array<wgpu::CompositeAlphaMode, 5> kAllAlphaModes = {
    wgpu::CompositeAlphaMode::Auto,          wgpu::CompositeAlphaMode::Opaque,
    wgpu::CompositeAlphaMode::Premultiplied, wgpu::CompositeAlphaMode::Unpremultiplied,
    wgpu::CompositeAlphaMode::Inherit,
};
static constexpr std::array<wgpu::PresentMode, 4> kAllPresentModes = {
    wgpu::PresentMode::Fifo,
    wgpu::PresentMode::FifoRelaxed,
    wgpu::PresentMode::Immediate,
    wgpu::PresentMode::Mailbox,
};
}  // namespace dawn::utils

namespace dawn {
namespace {

struct GLFWindowDestroyer {
    void operator()(GLFWwindow* ptr) { glfwDestroyWindow(ptr); }
};

class SurfaceConfigurationValidationTests : public DawnTest {
  public:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
        DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

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
        window.reset(glfwCreateWindow(500, 400, "SurfaceConfigurationValidationTests window",
                                      nullptr, nullptr));

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

    bool SupportsFormat(const wgpu::SurfaceCapabilities& capabilities, wgpu::TextureFormat format) {
        for (size_t i = 0; i < capabilities.formatCount; ++i) {
            if (capabilities.formats[i] == format) {
                return true;
            }
        }
        return false;
    }

    bool SupportsAlphaMode(const wgpu::SurfaceCapabilities& capabilities,
                           wgpu::CompositeAlphaMode mode) {
        if (mode == wgpu::CompositeAlphaMode::Auto) {
            return true;
        }

        for (size_t i = 0; i < capabilities.alphaModeCount; ++i) {
            if (capabilities.alphaModes[i] == mode) {
                return true;
            }
        }
        return false;
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

// Using undefined format is not valid
TEST_P(SurfaceConfigurationValidationTests, UndefinedFormat) {
    wgpu::SurfaceConfiguration config;
    config.device = device;
    config.format = wgpu::TextureFormat::Undefined;
    ASSERT_DEVICE_ERROR(CreateTestSurface().Configure(&config));
}

// Supports at least one configuration
TEST_P(SurfaceConfigurationValidationTests, AtLeastOneSupportedConfiguration) {
    wgpu::Surface surface = CreateTestSurface();

    wgpu::SurfaceCapabilities capabilities;
    surface.GetCapabilities(adapter, &capabilities);

    ASSERT_GT(capabilities.formatCount, 0u);
    ASSERT_GT(capabilities.alphaModeCount, 0u);
    ASSERT_GT(capabilities.presentModeCount, 0u);
}

// AlphaMode::Auto should never be reported but always supported.
TEST_P(SurfaceConfigurationValidationTests, AlphaModeAuto) {
    wgpu::Surface surface = CreateTestSurface();

    wgpu::SurfaceCapabilities capabilities;
    surface.GetCapabilities(adapter, &capabilities);

    // Auto is never reported.
    for (size_t i = 0; i < capabilities.alphaModeCount; ++i) {
        ASSERT_NE(capabilities.alphaModes[i], wgpu::CompositeAlphaMode::Auto);
    }

    // But always supported.
    wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);
    config.alphaMode = wgpu::CompositeAlphaMode::Auto;
    surface.Configure(&config);
}

// Using any combination of the reported capability is ok for configuring the surface.
TEST_P(SurfaceConfigurationValidationTests, AnyCombinationOfCapabilities) {
    // TODO(dawn:2320): Fails with "internal drawable creation failed" on the Windows NVIDIA CQ
    // builders but not locally. This is a similar limitation to SurfaceTests.SwitchPresentMode.
    DAWN_SUPPRESS_TEST_IF(IsWindows() && IsVulkan() && IsNvidia());

    wgpu::Surface surface = CreateTestSurface();

    wgpu::SurfaceConfiguration config = baseConfig;

    wgpu::SurfaceCapabilities capabilities;
    surface.GetCapabilities(adapter, &capabilities);

    for (wgpu::TextureFormat format : dawn::utils::kAllTextureFormats) {
        for (wgpu::CompositeAlphaMode alphaMode : dawn::utils::kAllAlphaModes) {
            for (wgpu::PresentMode presentMode : dawn::utils::kAllPresentModes) {
                config.format = format;
                config.alphaMode = alphaMode;
                config.presentMode = presentMode;

                if (!SupportsFormat(capabilities, config.format) ||
                    !SupportsAlphaMode(capabilities, config.alphaMode) ||
                    !SupportsPresentMode(capabilities, config.presentMode)) {
                    ASSERT_DEVICE_ERROR(surface.Configure(&config));
                } else {
                    surface.Configure(&config);

                    // Check that we can present
                    wgpu::SurfaceTexture surfaceTexture;
                    surface.GetCurrentTexture(&surfaceTexture);
                    surface.Present();
                }
                device.Tick();
            }
        }
    }
}

// Invalid view format fails
TEST_P(SurfaceConfigurationValidationTests, InvalidViewFormat) {
    wgpu::Surface surface = CreateTestSurface();
    auto invalid = wgpu::TextureFormat::R32Uint;

    wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);
    config.viewFormatCount = 1;
    config.viewFormats = &invalid;
    ASSERT_DEVICE_ERROR(surface.Configure(&config));
}

// View format is valid when it matches the config format
TEST_P(SurfaceConfigurationValidationTests, ValidViewFormat) {
    wgpu::Surface surface = CreateTestSurface();

    wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);
    config.viewFormatCount = 1;
    config.viewFormats = &config.format;
    surface.Configure(&config);

    // TODO(dawn:2320): Also test the equivalent (non-)sRGB view format
}

// A width of 0 fails
TEST_P(SurfaceConfigurationValidationTests, ZeroWidth) {
    wgpu::Surface surface = CreateTestSurface();

    wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);
    config.width = 0;
    ASSERT_DEVICE_ERROR(surface.Configure(&config));
}

// A height of 0 fails
TEST_P(SurfaceConfigurationValidationTests, ZeroHeight) {
    wgpu::Surface surface = CreateTestSurface();

    wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);
    config.height = 0;
    ASSERT_DEVICE_ERROR(surface.Configure(&config));
}

// A width that exceeds the maximum texture size fails
TEST_P(SurfaceConfigurationValidationTests, ExcessiveWidth) {
    wgpu::Surface surface = CreateTestSurface();
    wgpu::SupportedLimits supported;
    device.GetLimits(&supported);

    wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);
    config.width = supported.limits.maxTextureDimension1D + 1;
    ASSERT_DEVICE_ERROR(surface.Configure(&config));
}

// A height that exceeds the maximum texture size fails
TEST_P(SurfaceConfigurationValidationTests, ExcessiveHeight) {
    wgpu::Surface surface = CreateTestSurface();
    wgpu::SupportedLimits supported;
    device.GetLimits(&supported);

    wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);
    config.height = supported.limits.maxTextureDimension2D + 1;
    ASSERT_DEVICE_ERROR(surface.Configure(&config));
}

// A surface that was not configured must not be unconfigured
TEST_P(SurfaceConfigurationValidationTests, UnconfigureNonConfiguredSurfaceFails) {
    // TODO(dawn:2320): With SwiftShader, this throws a device error anyways (maybe because
    // mInstance->ConsumedError calls the device error callback?). We should have a
    // ASSERT_INSTANCE_ERROR to fully fix this test case.
    DAWN_SUPPRESS_TEST_IF(IsSwiftshader());

    // TODO(dawn:2320): This cannot throw a device error since the surface is
    // not aware of the device at this stage.
    /*ASSERT_DEVICE_ERROR(*/ CreateTestSurface().Unconfigure() /*)*/;
}

// Test that including unsupported usage flag will result in error.
TEST_P(SurfaceConfigurationValidationTests, ErrorIncludeUnsupportedUsage) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    wgpu::Surface surface = CreateTestSurface();
    wgpu::SurfaceCapabilities caps;
    surface.GetCapabilities(adapter, &caps);

    // Assuming StorageAttachment is not supported.
    DAWN_TEST_UNSUPPORTED_IF(caps.usages & wgpu::TextureUsage::StorageAttachment);

    wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);
    config.usage = wgpu::TextureUsage::StorageAttachment;
    ASSERT_DEVICE_ERROR_MSG(surface.Configure(&config), testing::HasSubstr("Usages requested"));
}

// Test that validation of format capabilities still happens.
TEST_P(SurfaceConfigurationValidationTests, StorageRequiresCapableFormat) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    wgpu::Surface surface = CreateTestSurface();
    wgpu::SurfaceCapabilities caps;
    surface.GetCapabilities(adapter, &caps);

    // Assuming StorageAttachment is not supported.
    DAWN_TEST_UNSUPPORTED_IF(!(caps.usages & wgpu::TextureUsage::StorageBinding));

    for (uint32_t i = 0; i < caps.formatCount; i++) {
        wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);
        config.usage = wgpu::TextureUsage::StorageBinding;
        config.format = caps.formats[i];

        if (utils::TextureFormatSupportsStorageTexture(config.format, device, false)) {
            surface.Configure(&config);
        } else {
            ASSERT_DEVICE_ERROR_MSG(surface.Configure(&config),
                                    testing::HasSubstr("TextureUsage::StorageBinding"));
        }
    }
}

DAWN_INSTANTIATE_TEST(SurfaceConfigurationValidationTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      NullBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn

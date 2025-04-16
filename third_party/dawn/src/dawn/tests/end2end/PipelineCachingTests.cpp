// Copyright 2022 The Dawn & Tint Authors
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
#include <string_view>

#include "dawn/tests/DawnTest.h"
#include "dawn/tests/mocks/platform/CachingInterfaceMock.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

using ::testing::NiceMock;

// TODO(dawn:549) Add some sort of pipeline descriptor repository to test more caching.

static constexpr std::string_view kComputeShaderDefault = R"(
        @compute @workgroup_size(1) fn main() {}
    )";

static constexpr std::string_view kComputeShaderMultipleEntryPoints = R"(
        @compute @workgroup_size(16) fn main() {}
        @compute @workgroup_size(64) fn main2() {}
    )";

static constexpr std::string_view kVertexShaderDefault = R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 0.0);
        }
    )";

static constexpr std::string_view kVertexShaderMultipleEntryPoints = R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(1.0, 0.0, 0.0, 1.0);
        }

        @vertex fn main2() -> @builtin(position) vec4f {
            return vec4f(0.5, 0.5, 0.5, 1.0);
        }
    )";

static constexpr std::string_view kFragmentShaderDefault = R"(
        @fragment fn main() -> @location(0) vec4f {
            return vec4f(0.1, 0.2, 0.3, 0.4);
        }
    )";

static constexpr std::string_view kFragmentShaderMultipleOutput = R"(
        struct FragmentOut {
            @location(0) fragColor0 : vec4f,
            @location(1) fragColor1 : vec4f,
        }

        @fragment fn main() -> FragmentOut {
            var output : FragmentOut;
            output.fragColor0 = vec4f(0.1, 0.2, 0.3, 0.4);
            output.fragColor1 = vec4f(0.5, 0.6, 0.7, 0.8);
            return output;
        }
    )";

static constexpr std::string_view kFragmentShaderBindGroup00Uniform = R"(
        struct S {
            value : f32
        };

        @group(0) @binding(0) var<uniform> uBuffer : S;

        @fragment fn main() -> @location(0) vec4f {
            return vec4f(uBuffer.value, 0.2, 0.3, 0.4);
        }
    )";

static constexpr std::string_view kFragmentShaderBindGroup01Uniform = R"(
        struct S {
            value : f32
        };

        @group(0) @binding(1) var<uniform> uBuffer : S;

        @fragment fn main() -> @location(0) vec4f {
            return vec4f(uBuffer.value, 0.2, 0.3, 0.4);
        }
    )";

class PipelineCachingTests : public DawnTest {
  protected:
    std::unique_ptr<platform::Platform> CreateTestPlatform() override {
        return std::make_unique<DawnCachingMockPlatform>(&mMockCache);
    }

    struct EntryCounts {
        unsigned pipeline;
        unsigned shaderModule;
    };
    const EntryCounts counts = {
        // pipeline caching is only implemented on D3D12/Vulkan
        IsD3D12() || IsVulkan() ? 1u : 0u,
        // One blob per shader module
        1u,
    };
    NiceMock<CachingInterfaceMock> mMockCache;
};

class SinglePipelineCachingTests : public PipelineCachingTests {
  protected:
    wgpu::Limits GetRequiredLimits(const wgpu::Limits& supported) override {
        // Just copy all the limits, though all we really care about is
        // maxStorageBuffersInFragmentStage
        return supported;
    }
};

// Tests that pipeline creation works fine even if the cache is disabled.
// Note: This tests needs to use more than 1 device since the frontend cache on each device
//   will prevent going out to the blob cache.
TEST_P(SinglePipelineCachingTests, ComputePipelineNoCache) {
    mMockCache.Disable();

    // First time should create and since cache is disabled, it should not write out to the
    // cache.
    {
        wgpu::Device device = CreateDevice();
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module = utils::CreateShaderModule(device, kComputeShaderDefault.data());
        desc.compute.entryPoint = "main";
        EXPECT_CACHE_STATS(mMockCache, Hit(0), Add(0), device.CreateComputePipeline(&desc));
    }

    // Second time should create fine with no cache hits since cache is disabled.
    {
        wgpu::Device device = CreateDevice();
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module = utils::CreateShaderModule(device, kComputeShaderDefault.data());
        desc.compute.entryPoint = "main";
        EXPECT_CACHE_STATS(mMockCache, Hit(0), Add(0), device.CreateComputePipeline(&desc));
    }
}

// Tests that pipeline creation on the same device uses frontend cache when possible.
TEST_P(SinglePipelineCachingTests, ComputePipelineFrontedCache) {
    wgpu::ComputePipelineDescriptor desc;
    desc.layout = utils::MakeBasicPipelineLayout(device, nullptr);
    desc.compute.module = utils::CreateShaderModule(device, kComputeShaderDefault.data());
    desc.compute.entryPoint = "main";

    // First creation should create a cache entry.
    wgpu::ComputePipeline pipeline;
    EXPECT_CACHE_STATS(mMockCache, Hit(0), Add(counts.shaderModule + counts.pipeline),
                       pipeline = device.CreateComputePipeline(&desc));

    // Second creation on the same device should just return from frontend cache and should not
    // call out to the blob cache.
    EXPECT_CALL(mMockCache, LoadData).Times(0);
    wgpu::ComputePipeline samePipeline;
    EXPECT_CACHE_STATS(mMockCache, Hit(0), Add(0),
                       samePipeline = device.CreateComputePipeline(&desc));
    EXPECT_EQ(pipeline.Get() == samePipeline.Get(), !UsesWire());
}

// Tests that pipeline creation hits the cache when it is enabled.
// Note: This test needs to use more than 1 device since the frontend cache on each device
//   will prevent going out to the blob cache.
TEST_P(SinglePipelineCachingTests, ComputePipelineBlobCache) {
    // First time should create and write out to the cache.
    {
        wgpu::Device device = CreateDevice();
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module = utils::CreateShaderModule(device, kComputeShaderDefault.data());
        desc.compute.entryPoint = "main";
        EXPECT_CACHE_STATS(mMockCache, Hit(0), Add(counts.shaderModule + counts.pipeline),
                           device.CreateComputePipeline(&desc));
    }

    // Second time should create using the cache.
    {
        wgpu::Device device = CreateDevice();
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module = utils::CreateShaderModule(device, kComputeShaderDefault.data());
        desc.compute.entryPoint = "main";
        EXPECT_CACHE_STATS(mMockCache, Hit(counts.shaderModule + counts.pipeline), Add(0),
                           device.CreateComputePipeline(&desc));
    }
}

// Tests that pipeline creation hits the cache when using the same pipeline but with explicit
// layout.
TEST_P(SinglePipelineCachingTests, ComputePipelineBlobCacheExplictLayout) {
    // First time should create and write out to the cache.
    {
        wgpu::Device device = CreateDevice();
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module = utils::CreateShaderModule(device, kComputeShaderDefault.data());
        desc.compute.entryPoint = "main";
        EXPECT_CACHE_STATS(mMockCache, Hit(0), Add(counts.shaderModule + counts.pipeline),
                           device.CreateComputePipeline(&desc));
    }

    // Cache should hit: use the same pipeline but with explicit pipeline layout.
    {
        wgpu::Device device = CreateDevice();
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module = utils::CreateShaderModule(device, kComputeShaderDefault.data());
        desc.compute.entryPoint = "main";
        desc.layout = utils::MakeBasicPipelineLayout(device, {});
        EXPECT_CACHE_STATS(mMockCache, Hit(counts.shaderModule + counts.pipeline), Add(0),
                           device.CreateComputePipeline(&desc));
    }
}

// Tests that pipeline creation wouldn't hit the cache if the pipelines are not exactly the same.
TEST_P(SinglePipelineCachingTests, ComputePipelineBlobCacheShaderNegativeCases) {
    // First time should create and write out to the cache.
    {
        wgpu::Device device = CreateDevice();
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module = utils::CreateShaderModule(device, kComputeShaderDefault.data());
        desc.compute.entryPoint = "main";
        EXPECT_CACHE_STATS(mMockCache, Hit(0), Add(counts.shaderModule + counts.pipeline),
                           device.CreateComputePipeline(&desc));
    }

    // Cache should not hit: different shader module.
    {
        wgpu::Device device = CreateDevice();
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module =
            utils::CreateShaderModule(device, kComputeShaderMultipleEntryPoints.data());
        desc.compute.entryPoint = "main";
        EXPECT_CACHE_STATS(mMockCache, Hit(0), Add(counts.shaderModule + counts.pipeline),
                           device.CreateComputePipeline(&desc));
    }

    // Cache should not hit: same shader module but different shader entry point.
    {
        wgpu::Device device = CreateDevice();
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module =
            utils::CreateShaderModule(device, kComputeShaderMultipleEntryPoints.data());
        desc.compute.entryPoint = "main2";
        EXPECT_CACHE_STATS(mMockCache, Hit(0), Add(counts.shaderModule + counts.pipeline),
                           device.CreateComputePipeline(&desc));
    }
}

// Tests that pipeline creation does not hits the cache when it is enabled but we use different
// isolation keys.
TEST_P(SinglePipelineCachingTests, ComputePipelineBlobCacheIsolationKey) {
    // First time should create and write out to the cache.
    {
        wgpu::Device device = CreateDevice("isolation key 1");
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module = utils::CreateShaderModule(device, kComputeShaderDefault.data());
        desc.compute.entryPoint = "main";
        EXPECT_CACHE_STATS(mMockCache, Hit(0), Add(counts.shaderModule + counts.pipeline),
                           device.CreateComputePipeline(&desc));
    }

    // Second time should also create and write out to the cache.
    {
        wgpu::Device device = CreateDevice("isolation key 2");
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module = utils::CreateShaderModule(device, kComputeShaderDefault.data());
        desc.compute.entryPoint = "main";
        EXPECT_CACHE_STATS(mMockCache, Hit(0), Add(counts.shaderModule + counts.pipeline),
                           device.CreateComputePipeline(&desc));
    }
}

// Tests that pipeline creation works fine even if the cache is disabled.
// Note: This tests needs to use more than 1 device since the frontend cache on each device
//   will prevent going out to the blob cache.
TEST_P(SinglePipelineCachingTests, RenderPipelineNoCache) {
    mMockCache.Disable();

    // First time should create and since cache is disabled, it should not write out to the
    // cache.
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShaderDefault.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_STATS(mMockCache, Hit(0), Add(0), device.CreateRenderPipeline(&desc));
    }

    // Second time should create fine with no cache hits since cache is disabled.
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShaderDefault.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_STATS(mMockCache, Hit(0), Add(0), device.CreateRenderPipeline(&desc));
    }
}

// Tests that pipeline creation on the same device uses frontend cache when possible.
TEST_P(SinglePipelineCachingTests, RenderPipelineFrontedCache) {
    utils::ComboRenderPipelineDescriptor desc;
    desc.layout = utils::MakeBasicPipelineLayout(device, nullptr);
    desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
    desc.vertex.entryPoint = "main";
    desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShaderDefault.data());
    desc.cFragment.entryPoint = "main";

    // First creation should create a cache entry.
    wgpu::RenderPipeline pipeline;
    EXPECT_CACHE_STATS(mMockCache, Hit(0), Add(2 * counts.shaderModule + counts.pipeline),
                       pipeline = device.CreateRenderPipeline(&desc));

    // Second creation on the same device should just return from frontend cache and should not
    // call out to the blob cache.
    EXPECT_CALL(mMockCache, LoadData).Times(0);
    wgpu::RenderPipeline samePipeline;
    EXPECT_CACHE_STATS(mMockCache, Hit(0), Add(0),
                       samePipeline = device.CreateRenderPipeline(&desc));
    EXPECT_EQ(pipeline.Get() == samePipeline.Get(), !UsesWire());
}

// Tests that pipeline creation hits the cache when it is enabled.
// Note: This test needs to use more than 1 device since the frontend cache on each device
//   will prevent going out to the blob cache.
TEST_P(SinglePipelineCachingTests, RenderPipelineBlobCache) {
    // First time should create and write out to the cache.
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShaderDefault.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_STATS(mMockCache, Hit(0), Add(2 * counts.shaderModule + counts.pipeline),
                           device.CreateRenderPipeline(&desc));
    }

    // Second time should create using the cache.
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShaderDefault.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_STATS(mMockCache, Hit(2 * counts.shaderModule + counts.pipeline), Add(0),
                           device.CreateRenderPipeline(&desc));
    }
}

// Tests that pipeline creation hits the cache when using the same pipeline but with explicit
// layout.
TEST_P(SinglePipelineCachingTests, RenderPipelineBlobCacheExplictLayout) {
    // First time should create and write out to the cache.
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShaderDefault.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_STATS(mMockCache, Hit(0), Add(2 * counts.shaderModule + counts.pipeline),
                           device.CreateRenderPipeline(&desc));
    }

    // Cache should hit: use the same pipeline but with explicit pipeline layout.
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShaderDefault.data());
        desc.cFragment.entryPoint = "main";
        desc.layout = utils::MakeBasicPipelineLayout(device, {});
        EXPECT_CACHE_STATS(mMockCache, Hit(2 * counts.shaderModule + counts.pipeline), Add(0),
                           device.CreateRenderPipeline(&desc));
    }
}

// Tests that pipeline creation wouldn't hit the cache if the pipelines have different state set in
// the descriptor.
TEST_P(SinglePipelineCachingTests, RenderPipelineBlobCacheDescriptorNegativeCases) {
    // First time should create and write out to the cache.
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShaderDefault.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_STATS(mMockCache, Hit(0), Add(2 * counts.shaderModule + counts.pipeline),
                           device.CreateRenderPipeline(&desc));
    }

    // Cache should hit for shaders, but not pipeline: different pipeline descriptor state.
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.EnableDepthStencil();
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShaderDefault.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_STATS(mMockCache, Hit(2 * counts.shaderModule), Add(counts.pipeline),
                           device.CreateRenderPipeline(&desc));
    }
}

// Tests that pipeline creation wouldn't hit the cache if the pipelines are not exactly the same in
// terms of shader.
TEST_P(SinglePipelineCachingTests, RenderPipelineBlobCacheShaderNegativeCases) {
    // First time should create and write out to the cache.
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShaderDefault.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_STATS(mMockCache, Hit(0), Add(2 * counts.shaderModule + counts.pipeline),
                           device.CreateRenderPipeline(&desc));
    }

    // Cache should not hit for different vertex shader module,
    // Cache should still hit for the same fragment shader module.
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module =
            utils::CreateShaderModule(device, kVertexShaderMultipleEntryPoints.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShaderDefault.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_STATS(mMockCache, Hit(counts.shaderModule),
                           Add(counts.shaderModule + counts.pipeline),
                           device.CreateRenderPipeline(&desc));
    }

    // Cache should not hit: same shader module but different shader entry point.
    // Cache should still hit for the same shader module.
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module =
            utils::CreateShaderModule(device, kVertexShaderMultipleEntryPoints.data());
        desc.vertex.entryPoint = "main2";
        desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShaderDefault.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_STATS(mMockCache, Hit(counts.shaderModule),
                           Add(counts.shaderModule + counts.pipeline),
                           device.CreateRenderPipeline(&desc));
    }
}

// Tests that pipeline creation wouldn't hit the cache if the pipelines are not exactly the same
// (fragment color targets differences).
TEST_P(SinglePipelineCachingTests, RenderPipelineBlobCacheNegativeCasesFragmentColorTargets) {
    // In compat, all targets must have the same writeMask
    DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode());

    // First time should create and write out to the cache.
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.cFragment.targetCount = 2;
        desc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
        desc.cTargets[1].writeMask = wgpu::ColorWriteMask::None;
        desc.cTargets[1].format = wgpu::TextureFormat::RGBA8Unorm;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module =
            utils::CreateShaderModule(device, kFragmentShaderMultipleOutput.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_STATS(mMockCache, Hit(0), Add(2 * counts.shaderModule + counts.pipeline),
                           device.CreateRenderPipeline(&desc));
    }

    // Cache should not hit for the pipeline: different fragment color target state (sparse).
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.cFragment.targetCount = 2;
        desc.cTargets[0].format = wgpu::TextureFormat::Undefined;
        desc.cTargets[1].writeMask = wgpu::ColorWriteMask::None;
        desc.cTargets[1].format = wgpu::TextureFormat::RGBA8Unorm;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module =
            utils::CreateShaderModule(device, kFragmentShaderMultipleOutput.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_STATS(mMockCache, Hit(2 * counts.shaderModule), Add(counts.pipeline),
                           device.CreateRenderPipeline(&desc));
    }

    // Cache should not hit: different fragment color target state (trailing empty).
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.cFragment.targetCount = 2;
        desc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
        desc.cTargets[1].writeMask = wgpu::ColorWriteMask::None;
        desc.cTargets[1].format = wgpu::TextureFormat::Undefined;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module =
            utils::CreateShaderModule(device, kFragmentShaderMultipleOutput.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_STATS(mMockCache, Hit(2 * counts.shaderModule), Add(counts.pipeline),
                           device.CreateRenderPipeline(&desc));
    }
}

// Tests that pipeline creation hits the cache for shaders, but not the pipeline if the
// shaders aren't impacted by the layout. This test is a bit change detecting - but all
// cached backends currently remap shader bindings based on the layout. It can be split
// per-backend as needed.
TEST_P(SinglePipelineCachingTests, RenderPipelineBlobCacheLayout) {
    DAWN_SUPPRESS_TEST_IF(GetSupportedLimits().maxStorageBuffersInFragmentStage < 1);

    // First time should create and write out to the cache.
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module =
            utils::CreateShaderModule(device, kFragmentShaderBindGroup00Uniform.data());
        desc.cFragment.entryPoint = "main";
        desc.layout = utils::MakePipelineLayout(
            device, {
                        utils::MakeBindGroupLayout(
                            device,
                            {
                                {0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform},
                            }),
                    });
        EXPECT_CACHE_STATS(mMockCache, Hit(0), Add(2 * counts.shaderModule + counts.pipeline),
                           device.CreateRenderPipeline(&desc));
    }

    // Cache should hit for the shaders, but not for the pipeline: different layout.
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module =
            utils::CreateShaderModule(device, kFragmentShaderBindGroup00Uniform.data());
        desc.cFragment.entryPoint = "main";
        desc.layout = utils::MakePipelineLayout(
            device,
            {
                utils::MakeBindGroupLayout(
                    device,
                    {
                        {0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform},
                        {1, wgpu::ShaderStage::Fragment,
                         // Note: OpenGL pipeline uses an internal uniform buffer which can be
                         // impacted by the extra uniform buffer binding layout, resulting in a
                         // shader module cache miss.
                         (IsOpenGL() || IsOpenGLES()) ? wgpu::BufferBindingType::ReadOnlyStorage
                                                      : wgpu::BufferBindingType::Uniform},
                    }),
            });
        EXPECT_CACHE_STATS(mMockCache, Hit(2 * counts.shaderModule), Add(counts.pipeline),
                           device.CreateRenderPipeline(&desc));
    }

    // Cache should hit for the shaders, but not for the pipeline: different layout (dynamic).
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module =
            utils::CreateShaderModule(device, kFragmentShaderBindGroup00Uniform.data());
        desc.cFragment.entryPoint = "main";
        desc.layout = utils::MakePipelineLayout(
            device, {
                        utils::MakeBindGroupLayout(device,
                                                   {
                                                       {0, wgpu::ShaderStage::Fragment,
                                                        wgpu::BufferBindingType::Uniform, true},
                                                   }),
                    });
        EXPECT_CACHE_STATS(mMockCache, Hit(2 * counts.shaderModule), Add(counts.pipeline),
                           device.CreateRenderPipeline(&desc));
    }

    // Cache should not hit for the fragment shader, but should hit for the pipeline.
    // On Metal and Vulkan, the shader is different but compiles to the same due to binding number
    // remapping. On other backends, the compiled shader is different and so is the pipeline.
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module =
            utils::CreateShaderModule(device, kFragmentShaderBindGroup01Uniform.data());
        desc.cFragment.entryPoint = "main";
        desc.layout = utils::MakePipelineLayout(
            device, {
                        utils::MakeBindGroupLayout(
                            device,
                            {
                                {1, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform},
                            }),
                    });
        if (IsMetal() || IsVulkan()) {
            EXPECT_CACHE_STATS(mMockCache, Hit(counts.shaderModule + counts.pipeline),
                               Add(counts.shaderModule), device.CreateRenderPipeline(&desc));
        } else {
            EXPECT_CACHE_STATS(mMockCache, Hit(counts.shaderModule),
                               Add(counts.shaderModule + counts.pipeline),
                               device.CreateRenderPipeline(&desc));
        }
    }
}

// Tests that pipeline creation does not hits the cache when it is enabled but we use different
// isolation keys.
TEST_P(SinglePipelineCachingTests, RenderPipelineBlobCacheIsolationKey) {
    // First time should create and write out to the cache.
    {
        wgpu::Device device = CreateDevice("isolation key 1");
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShaderDefault.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_STATS(mMockCache, Hit(0), Add(2 * counts.shaderModule + counts.pipeline),
                           device.CreateRenderPipeline(&desc));
    }

    // Second time should also create and write out to the cache.
    {
        wgpu::Device device = CreateDevice("isolation key 2");
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShaderDefault.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_STATS(mMockCache, Hit(0), Add(2 * counts.shaderModule + counts.pipeline),
                           device.CreateRenderPipeline(&desc));
    }
}

DAWN_INSTANTIATE_TEST(SinglePipelineCachingTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      D3D12Backend({"use_dxc"}),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn

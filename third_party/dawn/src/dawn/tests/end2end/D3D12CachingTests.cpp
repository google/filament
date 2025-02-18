// Copyright 2020 The Dawn & Tint Authors
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
#include <unordered_map>
#include <utility>

#include "dawn/tests/DawnTest.h"
#include "dawn/tests/mocks/platform/CachingInterfaceMock.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

using ::testing::NiceMock;

class D3D12CachingTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        // TODO(dawn:1341) Re-enable tests once shader caching is re-implemented.
        DAWN_SKIP_TEST_IF_BASE(true, "suppressed", "TODO(dawn:1341)");
    }

    std::unique_ptr<platform::Platform> CreateTestPlatform() override {
        return std::make_unique<DawnCachingMockPlatform>(&mMockCache);
    }

    NiceMock<CachingInterfaceMock> mMockCache;
};

// Test that duplicate WGSL still works (and re-compiles HLSL) when the cache is not enabled.
TEST_P(D3D12CachingTests, SameShaderNoCache) {
    mMockCache.Disable();

    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @vertex fn vertex_main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        }

        @fragment fn fragment_main() -> @location(0) vec4f {
          return vec4f(1.0, 0.0, 0.0, 1.0);
        }
    )");

    // Store the WGSL shader into the cache.
    {
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = module;
        desc.cFragment.module = module;
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateRenderPipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 0u);

    // Load the same WGSL shader from the cache.
    {
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = module;
        desc.cFragment.module = module;
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateRenderPipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 0u);
}

// Test creating a pipeline from two entrypoints in multiple stages will cache the correct number
// of HLSL shaders. WGSL shader should result into caching 2 HLSL shaders (stage x
// entrypoints)
TEST_P(D3D12CachingTests, ReuseShaderWithMultipleEntryPointsPerStage) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @vertex fn vertex_main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        }

        @fragment fn fragment_main() -> @location(0) vec4f {
          return vec4f(1.0, 0.0, 0.0, 1.0);
        }
    )");

    // Store the WGSL shader into the cache.
    {
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = module;
        desc.cFragment.module = module;
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateRenderPipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 2u);

    // Load the same WGSL shader from the cache.
    {
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = module;
        desc.cFragment.module = module;
        EXPECT_CACHE_HIT(mMockCache, 2u, device.CreateRenderPipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 2u);

    // Modify the WGSL shader functions and make sure it doesn't hit.
    wgpu::ShaderModule newModule = utils::CreateShaderModule(device, R"(
      @vertex fn vertex_main() -> @builtin(position) vec4f {
          return vec4f(1.0, 1.0, 1.0, 1.0);
      }

      @fragment fn fragment_main() -> @location(0) vec4f {
        return vec4f(1.0, 1.0, 1.0, 1.0);
      }
  )");

    {
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = newModule;
        desc.cFragment.module = newModule;
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateRenderPipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 4u);
}

// Test creating a WGSL shader with two entrypoints in the same stage will cache the correct number
// of HLSL shaders. WGSL shader should result into caching 1 HLSL shader (stage x entrypoints)
TEST_P(D3D12CachingTests, ReuseShaderWithMultipleEntryPoints) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        struct Data {
            data : u32
        }
        @binding(0) @group(0) var<storage, read_write> data : Data;

        @compute @workgroup_size(1) fn write1() {
            data.data = 1u;
        }

        @compute @workgroup_size(1) fn write42() {
            data.data = 42u;
        }
    )");

    // Store the WGSL shader into the cache.
    {
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module = module;
        desc.compute.entryPoint = "write1";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateComputePipeline(&desc));

        desc.compute.module = module;
        desc.compute.entryPoint = "write42";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateComputePipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 2u);

    // Load the same WGSL shader from the cache.
    {
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module = module;
        desc.compute.entryPoint = "write1";
        EXPECT_CACHE_HIT(mMockCache, 1u, device.CreateComputePipeline(&desc));

        desc.compute.module = module;
        desc.compute.entryPoint = "write42";
        EXPECT_CACHE_HIT(mMockCache, 1u, device.CreateComputePipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 2u);
}

DAWN_INSTANTIATE_TEST(D3D12CachingTests, D3D12Backend());

}  // anonymous namespace
}  // namespace dawn

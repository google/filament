// Copyright 2025 The Dawn & Tint Authors
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

static constexpr std::string_view kComputeShaderDefault = R"(
        @compute @workgroup_size(1) fn main() {}
    )";

static constexpr std::string_view kComputeShaderMultipleEntryPoints = R"(
        @compute @workgroup_size(16) fn main() {}
        @compute @workgroup_size(64) fn main2() {}
    )";

class ShaderModuleCachingTests : public DawnTest {
  protected:
    std::unique_ptr<platform::Platform> CreateTestPlatform() override {
        return std::make_unique<DawnCachingMockPlatform>(&mMockCache);
    }

    NiceMock<CachingInterfaceMock> mMockCache;
};

// Tests that shader module creation works fine even if the cache is disabled.
// Note: This tests needs to use more than 1 device since the frontend cache on each device
//   will prevent going out to the blob cache.
TEST_P(ShaderModuleCachingTests, ShaderModuleNoCache) {
    mMockCache.Disable();

    // First time should create and since cache is disabled, it should not write out to the
    // cache.
    {
        wgpu::Device device = CreateDevice();
        EXPECT_CACHE_STATS(mMockCache, Hit(0), Add(0),
                           utils::CreateShaderModule(device, kComputeShaderDefault.data()));
    }

    // Second time should create fine with no cache hits since cache is disabled.
    {
        wgpu::Device device = CreateDevice();
        EXPECT_CACHE_STATS(mMockCache, Hit(0), Add(0),
                           utils::CreateShaderModule(device, kComputeShaderDefault.data()));
    }
}

// Tests that shader module creation on the same device uses frontend cache when possible.
TEST_P(ShaderModuleCachingTests, ShaderModuleFrontedCache) {
    // First creation should create a cache entry.
    wgpu::ShaderModule shaderModule;
    EXPECT_CACHE_STATS(
        mMockCache, Hit(0), Add(1),
        shaderModule = utils::CreateShaderModule(device, kComputeShaderDefault.data()));

    // Second creation on the same device should just return from frontend cache and should not
    // call out to the blob cache.
    EXPECT_CALL(mMockCache, LoadData).Times(0);
    wgpu::ShaderModule sameShaderModule;
    EXPECT_CACHE_STATS(
        mMockCache, Hit(0), Add(0),
        sameShaderModule = utils::CreateShaderModule(device, kComputeShaderDefault.data()));

    EXPECT_EQ(shaderModule.Get() == sameShaderModule.Get(), !UsesWire());
}

// Tests that shader module creation hits the cache when it is enabled.
// Note: This test needs to use more than 1 device since the frontend cache on each device
//   will prevent going out to the blob cache.
TEST_P(ShaderModuleCachingTests, ShaderModuleBlobCache) {
    // First time should create and write out to the blob cache.
    {
        wgpu::Device device = CreateDevice();
        EXPECT_CACHE_STATS(mMockCache, Hit(0), Add(1),
                           utils::CreateShaderModule(device, kComputeShaderDefault.data()));
    }

    // Second time should create shader module using the blob cache.
    {
        wgpu::Device device = CreateDevice();
        EXPECT_CACHE_STATS(mMockCache, Hit(1), Add(0),
                           utils::CreateShaderModule(device, kComputeShaderDefault.data()));
    }
}

// Tests that shader module creation wouldn't hit the cache if the shader modules are not exactly
// the same.
TEST_P(ShaderModuleCachingTests, DifferentShaderModuleBlobCache) {
    // First time should create and write out to the cache.
    {
        wgpu::Device device = CreateDevice();
        EXPECT_CACHE_STATS(mMockCache, Hit(0), Add(1),
                           utils::CreateShaderModule(device, kComputeShaderDefault.data()));
    }

    // Cache should not hit: different shader module.
    {
        wgpu::Device device = CreateDevice();
        EXPECT_CACHE_STATS(
            mMockCache, Hit(0), Add(1),
            utils::CreateShaderModule(device, kComputeShaderMultipleEntryPoints.data()));
    }
}

// Tests that shader module creation does not hits the cache when it is enabled but we use different
// isolation keys.
TEST_P(ShaderModuleCachingTests, ShaderModuleBlobCacheIsolationKey) {
    // First time should create and write out to the cache.
    {
        wgpu::Device device = CreateDevice("isolation key 1");
        EXPECT_CACHE_STATS(mMockCache, Hit(0), Add(1),
                           utils::CreateShaderModule(device, kComputeShaderDefault.data()));
    }

    // Second time should also create and write out to the cache.
    {
        wgpu::Device device = CreateDevice("isolation key 2");
        EXPECT_CACHE_STATS(mMockCache, Hit(0), Add(1),
                           utils::CreateShaderModule(device, kComputeShaderDefault.data()));
    }
}

DAWN_INSTANTIATE_TEST(ShaderModuleCachingTests,
                      D3D11Backend({"blob_cache_hash_validation"}),
                      D3D11Backend({}, {"blob_cache_hash_validation"}),
                      D3D12Backend({"blob_cache_hash_validation"}),
                      D3D12Backend({}, {"blob_cache_hash_validation"}),
                      D3D12Backend({"use_dxc", "blob_cache_hash_validation"}),
                      D3D12Backend({"use_dxc"}, {"blob_cache_hash_validation"}),
                      MetalBackend({"blob_cache_hash_validation"}),
                      MetalBackend({}, {"blob_cache_hash_validation"}),
                      OpenGLBackend({"blob_cache_hash_validation"}),
                      OpenGLBackend({}, {"blob_cache_hash_validation"}),
                      OpenGLESBackend({"blob_cache_hash_validation"}),
                      OpenGLESBackend({}, {"blob_cache_hash_validation"}),
                      VulkanBackend({"blob_cache_hash_validation"}),
                      VulkanBackend({}, {"blob_cache_hash_validation"}));

}  // anonymous namespace
}  // namespace dawn

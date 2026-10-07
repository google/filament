/*
 * Copyright (C) 2026 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "BackendTest.h"
#include "Lifetimes.h"
#include "Shader.h"
#include "SharedShaders.h"
#include "Skip.h"
#include "TrianglePrimitive.h"

#include <private/backend/Driver.h>
#include <private/backend/PlatformFactory.h>

#include <backend/Platform.h>

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <vector>

namespace test {

using namespace filament;
using namespace filament::backend;
using namespace filament::math;

namespace {

// What an application backs with a file: one value, whatever the key. Shared with the Platform's
// callbacks so it outlives the driver, which may still write to it from terminate().
struct BlobStore {
    std::mutex lock;
    std::vector<uint8_t> value;
    int inserts = 0;
    int retrieves = 0;

    static void attach(std::shared_ptr<BlobStore> const& store, Platform& platform) {
        platform.setBlobFunc(
                [store](void const*, size_t, void const* value, size_t size) {
                    std::lock_guard const guard(store->lock);
                    auto const* bytes = static_cast<uint8_t const*>(value);
                    store->value.assign(bytes, bytes + size);
                    store->inserts++;
                },
                [store](void const*, size_t, void* value, size_t size) -> size_t {
                    std::lock_guard const guard(store->lock);
                    store->retrieves++;
                    if (!store->value.empty() && store->value.size() <= size) {
                        std::memcpy(value, store->value.data(), store->value.size());
                    }
                    return store->value.size();
                });
    }
};

// Flush events without a newly compiled pipeline before the backend writes its cache (gc() runs once per
// frame), with some margin.
constexpr int SETTLE_FRAMES = 130;

// VkPipelineCacheHeaderVersionOne, without depending on the Vulkan headers: 32 bytes, with
// pipelineCacheUUID at offset 16.
constexpr size_t CACHE_HEADER_SIZE = 32;
constexpr size_t CACHE_UUID_OFFSET = 16;

} // namespace

class VulkanPipelineCacheTest : public BackendTest {
protected:
    // Draws one triangle, which creates a pipeline, then enough empty frames for the backend to
    // consider pipeline creation settled.
    void drawThenSettle() {
        auto& api = getDriverApi();
        auto swapChain = addCleanup(createSwapChain());
        api.makeCurrent(swapChain, swapChain);
        RenderTargetHandle renderTarget = addCleanup(api.createDefaultRenderTarget());

        Shader shader = SharedShaders::makeShader(api, *mCleanup, ShaderRequest{
                .mVertexType = VertexShaderType::Simple,
                .mFragmentType = FragmentShaderType::SolidColored,
                .mUniformType = ShaderUniformType::Simple,
        });
        TrianglePrimitive triangle(api);

        RenderPassParams params = getClearColorDepthRenderPass();
        params.viewport = getFullViewport();
        PipelineState ps = getColorWritePipelineState();
        shader.addProgramToPipelineState(ps);

        auto ubuffer = addCleanup(api.createBufferObject(sizeof(SimpleMaterialParams),
                BufferObjectBinding::UNIFORM, BufferUsage::STATIC));
        shader.uploadUniform(api, ubuffer, SimpleMaterialParams{
                .color = float4(1, 0, 0, 1),
                .scaleMinusOne = float4(0, 0, -0.5, 0),
                .offset = float4(0, 0, 0, 0),
        });
        shader.bindUniform<SimpleMaterialParams>(api, ubuffer);

        {
            RenderFrame frame(api);
            api.beginRenderPass(renderTarget, params);
            ps.primitiveType = PrimitiveType::TRIANGLES;
            ps.vertexBufferInfo = triangle.getVertexBufferInfo();
            api.bindPipeline(ps);
            api.bindRenderPrimitive(triangle.getRenderPrimitive());
            api.draw2(0, 3, 1);
            api.endRenderPass();
            api.commit(swapChain);
        }
        flushAndWait();

        for (int i = 0; i < SETTLE_FRAMES; i++) {
            RenderFrame frame(api);
        }
        flushAndWait();
    }

    // A second Vulkan driver, created after its blob functions are set: the cache is read when
    // the driver is created, so it cannot be tested on the fixture's own driver.
    static void createAndTerminateDriver(std::shared_ptr<BlobStore> const& store) {
        filament::backend::Backend backend = filament::backend::Backend::VULKAN;
        Platform* platform = PlatformFactory::create(&backend);
        ASSERT_NE(platform, nullptr);
        BlobStore::attach(store, *platform);
        Driver* driver = platform->createDriver(nullptr, getDriverConfig());
        ASSERT_NE(driver, nullptr);
        driver->terminate();
        delete driver;
        delete platform;
    }
};

TEST_F(VulkanPipelineCacheTest, SavesOncePipelineCreationSettles) {
    SKIP_IF_NOT(Backend::VULKAN, "The pipeline cache is a Vulkan backend feature");

    auto store = std::make_shared<BlobStore>();
    BlobStore::attach(store, *getPlatform());
    drawThenSettle();

    std::lock_guard const guard(store->lock);
    EXPECT_EQ(store->inserts, 1);
    EXPECT_FALSE(store->value.empty());
}

TEST_F(VulkanPipelineCacheTest, LoadsWhatItSaved) {
    SKIP_IF_NOT(Backend::VULKAN, "The pipeline cache is a Vulkan backend feature");

    auto saved = std::make_shared<BlobStore>();
    BlobStore::attach(saved, *getPlatform());
    drawThenSettle();
    std::vector<uint8_t> bytes;
    {
        std::lock_guard const guard(saved->lock);
        ASSERT_FALSE(saved->value.empty());
        bytes = saved->value;
    }

    auto loaded = std::make_shared<BlobStore>();
    loaded->value = bytes;
    createAndTerminateDriver(loaded);

    // The new driver read the cache and compiled nothing, so its terminate() wrote nothing.
    std::lock_guard const guard(loaded->lock);
    EXPECT_GE(loaded->retrieves, 1);
    EXPECT_EQ(loaded->inserts, 0);
    EXPECT_EQ(loaded->value, bytes);
}

TEST_F(VulkanPipelineCacheTest, IgnoresDataFromSomewhereElse) {
    SKIP_IF_NOT(Backend::VULKAN, "The pipeline cache is a Vulkan backend feature");

    // Not a pipeline cache at all: the engine must still come up.
    auto store = std::make_shared<BlobStore>();
    store->value.assign(256, uint8_t(0xAB));
    createAndTerminateDriver(store);

    std::lock_guard const guard(store->lock);
    EXPECT_GE(store->retrieves, 1);
}

TEST_F(VulkanPipelineCacheTest, IgnoresCacheFromAnotherDevice) {
    SKIP_IF_NOT(Backend::VULKAN, "The pipeline cache is a Vulkan backend feature");

    // A real cache whose pipelineCacheUUID no longer matches, as after a driver update: rejected
    // before it reaches the driver, which may not ignore it as the spec requires.
    auto saved = std::make_shared<BlobStore>();
    BlobStore::attach(saved, *getPlatform());
    drawThenSettle();
    std::vector<uint8_t> stale;
    {
        std::lock_guard const guard(saved->lock);
        ASSERT_GE(saved->value.size(), CACHE_HEADER_SIZE);
        stale = saved->value;
    }
    stale[CACHE_UUID_OFFSET] ^= 0xFF;

    auto store = std::make_shared<BlobStore>();
    store->value = stale;
    createAndTerminateDriver(store);

    std::lock_guard const guard(store->lock);
    EXPECT_GE(store->retrieves, 1);
}

} // namespace test

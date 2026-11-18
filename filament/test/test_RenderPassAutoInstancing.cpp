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

#include "Allocators.h"
#include "CommandStreamDispatcher.h"
#include "filament_test_resources.h"
#include "MockDriver.h"
#include "RenderPass.h"

#include "details/Engine.h"
#include "details/Material.h"
#include "details/MaterialInstance.h"
#include "details/Scene.h"

#include <private/filament/EngineEnums.h>
#include <private/filament/UibStructs.h>
#include <private/filament/Variant.h>

#include <filament/Box.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>

#include <private/backend/CommandBufferQueue.h>
#include <private/backend/CommandStream.h>

#include <backend/BufferDescriptor.h>
#include <backend/DescriptorSetOffsetArray.h>
#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <utils/Entity.h>
#include <utils/EntityManager.h>
#include <utils/Range.h>

#include <math/mat4.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>

namespace {

using namespace filament;
using namespace filament::backend;
using namespace filament::math;
using namespace utils;

class AutoInstancingSpyDriver : public MockDriver {
public:
    AutoInstancingSpyDriver() {
        ON_CALL(*this, getFeatureLevel()).WillByDefault(Return(FeatureLevel::FEATURE_LEVEL_1));
    }

    struct CreatedBuffer {
        BufferObjectHandle handle;
        uint32_t byteCount = 0;
        BufferObjectBinding bindingType{};
        BufferUsage usage{};
    };

    struct BufferUpload {
        BufferObjectHandle handle;
        uint32_t byteOffset = 0;
        size_t size = 0;
        bool hasCallback = false;
        uintptr_t alignmentCheck = 0;
        std::vector<PerRenderableData> elements;
    };

    struct DescriptorBufferUpdate {
        DescriptorSetHandle dsh;
        descriptor_binding_t binding = 0;
        BufferObjectHandle boh;
        uint32_t offset = 0;
        uint32_t size = 0;
    };

    struct DescriptorBind {
        DescriptorSetHandle dsh;
        descriptor_set_t set = 0;
        uint32_t dynamicOffset0 = 0;
    };

    struct IndexedDraw {
        uint32_t indexOffset = 0;
        uint32_t indexCount = 0;
        uint32_t instanceCount = 0;
    };

    Dispatcher getDispatcher() const noexcept override {
        return ConcreteDispatcher<AutoInstancingSpyDriver>::make();
    }

    void createBufferObjectR(BufferObjectHandle boh, uint32_t byteCount,
            BufferObjectBinding bindingType, BufferUsage usage, ImmutableCString&&) {
        createdBuffers.push_back({ boh, byteCount, bindingType, usage });
    }

    void updateBufferObjectUnsynchronized(
            BufferObjectHandle boh, BufferDescriptor&& data, uint32_t byteOffset) {
        BufferUpload upload;
        upload.handle = boh;
        upload.byteOffset = byteOffset;
        upload.size = data.size;
        upload.hasCallback = data.hasCallback();
        upload.alignmentCheck = reinterpret_cast<uintptr_t>(data.buffer);
        if (data.buffer != nullptr && data.size >= sizeof(PerRenderableData)) {
            size_t const count = data.size / sizeof(PerRenderableData);
            upload.elements.resize(count);
            std::memcpy(upload.elements.data(), data.buffer, count * sizeof(PerRenderableData));
        }
        uploads.push_back(std::move(upload));
        // Letting `data` go out of scope invokes its release callback immediately.
    }

    void updateDescriptorSetBuffer(DescriptorSetHandle dsh, descriptor_binding_t binding,
            BufferObjectHandle boh, uint32_t offset, uint32_t size) {
        descriptorBufferUpdates.push_back({ dsh, binding, boh, offset, size });
    }

    void bindDescriptorSet(DescriptorSetHandle dsh, descriptor_set_t set,
            DescriptorSetOffsetArray&& offsets) {
        uint32_t const dynamicOffset0 = offsets.empty() ? 0u : offsets[0];
        descriptorBinds.push_back({ dsh, set, dynamicOffset0 });
    }

    void draw2(uint32_t indexOffset, uint32_t indexCount, uint32_t instanceCount) {
        draws.push_back({ indexOffset, indexCount, instanceCount });
    }

    void resetSpyRecords() {
        createdBuffers.clear();
        uploads.clear();
        descriptorBufferUpdates.clear();
        descriptorBinds.clear();
        draws.clear();
    }

    std::vector<CreatedBuffer> createdBuffers;
    std::vector<BufferUpload> uploads;
    std::vector<DescriptorBufferUpdate> descriptorBufferUpdates;
    std::vector<DescriptorBind> descriptorBinds;
    std::vector<IndexedDraw> draws;
};

class RenderPassAutoInstancingTest : public ::testing::Test {
protected:
    static constexpr size_t COMMAND_ARENA_SIZE = 4 * 1024 * 1024;
    static constexpr size_t SCENE_ARENA_SIZE = 256 * 1024;

    RenderPassAutoInstancingTest()
            : mEngine(Engine::Builder()
                      .backend(Backend::NOOP)
                      .feature("material.enable_material_instance_uniform_batching", false)
                      .build()),
              mFEngine(downcast(mEngine)),
              mCommandBufferQueue(mFEngine->getMinCommandBufferSize(),
                      3 * mFEngine->getMinCommandBufferSize(), false),
              mCommandStream(mSpyDriver, mCommandBufferQueue.getCircularBuffer()),
              mDriverApi(mCommandStream),
              mCommandArenaStorage(std::make_unique<uint8_t[]>(COMMAND_ARENA_SIZE)),
              mCommandArena("TestRenderPassArena",
                      { mCommandArenaStorage.get(),
                        mCommandArenaStorage.get() + COMMAND_ARENA_SIZE }),
              mSceneArena("TestSceneArena", SCENE_ARENA_SIZE) {
        mEngine->setAutomaticInstancingEnabled(true);
        EXPECT_TRUE(mFEngine->isAutomaticInstancingEnabled());

        mScene = mEngine->createScene();
        mFScene = downcast(mScene);

        static float3 const sPositions[3] = {
            { -0.5f, -0.5f, 0.0f },
            {  0.5f, -0.5f, 0.0f },
            {  0.0f,  0.5f, 0.0f },
        };
        static short4 const sTangents[3] = {
            { 0, 0, 0, 32767 },
            { 0, 0, 0, 32767 },
            { 0, 0, 0, 32767 },
        };
        static uint16_t const sIndices[3] = { 0, 1, 2 };

        mVertexBuffer = VertexBuffer::Builder()
                .vertexCount(3)
                .bufferCount(2)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3)
                .attribute(VertexAttribute::TANGENTS, 1, VertexBuffer::AttributeType::SHORT4)
                .normalized(VertexAttribute::TANGENTS)
                .build(*mEngine);
        mVertexBuffer->setBufferAt(*mEngine, 0,
                VertexBuffer::BufferDescriptor(sPositions, sizeof(sPositions)));
        mVertexBuffer->setBufferAt(*mEngine, 1,
                VertexBuffer::BufferDescriptor(sTangents, sizeof(sTangents)));

        mIndexBuffer = IndexBuffer::Builder()
                .indexCount(3)
                .bufferType(IndexBuffer::IndexType::USHORT)
                .build(*mEngine);
        mIndexBuffer->setBuffer(*mEngine,
                IndexBuffer::BufferDescriptor(sIndices, sizeof(sIndices)));

        mMaterial = Material::Builder()
                .package(FILAMENT_TEST_RESOURCES_TEST_MATERIAL_DATA,
                        FILAMENT_TEST_RESOURCES_TEST_MATERIAL_SIZE)
                .build(*mEngine);
    }

    void TearDown() override {
        for (Entity const e : mEntities) {
            mScene->remove(e);
            mEngine->destroy(e);
            EntityManager::get().destroy(e);
        }
        for (MaterialInstance* const mi : mMaterialInstances) {
            mEngine->destroy(mi);
        }
        mEngine->destroy(mScene);
        mEngine->destroy(mVertexBuffer);
        mEngine->destroy(mIndexBuffer);
        mEngine->destroy(mMaterial);

        flushAndExecute();
        mCommandBufferQueue.requestExit();

        Engine::destroy(&mEngine);
    }

    MaterialInstance* createMaterialInstance() {
        MaterialInstance* const mi = mMaterial->createInstance();
        mMaterialInstances.push_back(mi);
        downcast(mi)->commit(mDriverApi, nullptr);
        return mi;
    }

    Entity addRenderable(MaterialInstance* mi, float xTranslation) {
        Entity const e = EntityManager::get().create();
        mEntities.push_back(e);

        TransformManager& tcm = mEngine->getTransformManager();
        tcm.create(e, {}, mat4f::translation(float3{ xTranslation, 0.0f, 0.0f }));

        RenderableManager::Builder(1)
                .boundingBox({ { 0.0f, 0.0f, 0.0f }, { 0.5f, 0.5f, 0.5f } })
                .material(0, mi)
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES,
                        mVertexBuffer, mIndexBuffer, 0, 3)
                .culling(false)
                .receiveShadows(false)
                .castShadows(false)
                .build(*mEngine, e);

        mScene->addEntity(e);
        return e;
    }

    Range<uint32_t> prepareScene() {
        mFScene->prepare(mFEngine->getJobSystem(), mSceneArena, mat4{}, false, mSceneCache);
        uint32_t const count = uint32_t(mSceneCache.renderableData.size());
        for (uint32_t i = 0; i < count; ++i) {
            mSceneCache.renderableData.elementAt<FScene::VISIBLE_MASK>(i) = 0x1;
            mSceneCache.renderableData.elementAt<FScene::DESCRIPTOR_SET_HANDLE>(i) =
                    DescriptorSetHandle{ 1 };
        }
        Range<uint32_t> const vr{ 0, count };
        mFScene->prepareVisibleRenderables(vr, mSceneCache);
        return vr;
    }

    RenderPass buildDepthPass(Range<uint32_t> vr) {
        return RenderPassBuilder{ mCommandArena }
                .geometry(mSceneCache.renderableData, vr)
                .camera({ 0.0f, 0.0f, 5.0f }, { 0.0f, 0.0f, -1.0f })
                .commandTypeFlags(RenderPass::CommandTypeFlags::SSAO)
                .variant(Variant{ Variant::DEPTH_VARIANT })
                .visibilityMask(0x1)
                .build(*mFEngine, mDriverApi);
    }

    void flushAndExecute() {
        if (mCommandBufferQueue.getCircularBuffer().empty()) {
            return;
        }
        mCommandBufferQueue.flush();
        std::vector<CommandBufferQueue::Range> const buffers =
                mCommandBufferQueue.waitForCommands();
        for (CommandBufferQueue::Range const& buffer : buffers) {
            mCommandStream.execute(buffer.begin);
            mCommandBufferQueue.releaseBuffer(buffer);
        }
    }

    Engine* mEngine = nullptr;
    FEngine* mFEngine = nullptr;
    ::testing::NiceMock<AutoInstancingSpyDriver> mSpyDriver;
    CommandBufferQueue mCommandBufferQueue;
    CommandStream mCommandStream;
    DriverApi& mDriverApi;
    std::unique_ptr<uint8_t[]> mCommandArenaStorage;
    RenderPass::Arena mCommandArena;
    LinearAllocatorArena mSceneArena;
    Scene* mScene = nullptr;
    FScene* mFScene = nullptr;
    FScene::SceneCacheData mSceneCache;
    VertexBuffer* mVertexBuffer = nullptr;
    IndexBuffer* mIndexBuffer = nullptr;
    Material const* mMaterial = nullptr;
    std::vector<MaterialInstance*> mMaterialInstances;
    std::vector<Entity> mEntities;
};

} // anonymous namespace

TEST_F(RenderPassAutoInstancingTest, SparseAutoInstancingInLargePass) {
    // 512 distinct non-instanced renderables + 1 shared pair (2 auto-instanced renderables).
    constexpr size_t UNIQUE_COUNT = 512;
    for (size_t i = 0; i < UNIQUE_COUNT; ++i) {
        MaterialInstance* const uniqueMi = createMaterialInstance();
        addRenderable(uniqueMi, float(i + 1));
    }

    MaterialInstance* const sharedMi = createMaterialInstance();
    addRenderable(sharedMi, 1000.0f);
    addRenderable(sharedMi, 2000.0f);

    Range<uint32_t> const vr = prepareScene();
    ASSERT_EQ(vr.size(), UNIQUE_COUNT + 2);

    flushAndExecute();
    mSpyDriver.resetSpyRecords();

    RenderPass pass = buildDepthPass(vr);
    // 514 draw commands collapse into 512 unique + 1 instanced command = 513 commands.
    EXPECT_EQ(std::distance(pass.begin(), pass.end()), ptrdiff_t(UNIQUE_COUNT + 1));

    pass.finalize(*mFEngine, mDriverApi);
    flushAndExecute();

    // Only the 2 instanced primitives (512 B) should be uploaded, and the GPU UBO should be
    // sized to exactly 1 PerRenderableUib window (16 KiB on desktop/mobile), NOT 514 * 256 B.
    ASSERT_EQ(mSpyDriver.createdBuffers.size(), 1u);
    EXPECT_EQ(mSpyDriver.createdBuffers[0].bindingType, BufferObjectBinding::UNIFORM);
    EXPECT_EQ(mSpyDriver.createdBuffers[0].byteCount, sizeof(PerRenderableUib));

    ASSERT_EQ(mSpyDriver.uploads.size(), 1u);
    EXPECT_EQ(mSpyDriver.uploads[0].handle, mSpyDriver.createdBuffers[0].handle);
    EXPECT_EQ(mSpyDriver.uploads[0].byteOffset, 0u);
    EXPECT_EQ(mSpyDriver.uploads[0].size, 2 * sizeof(PerRenderableData));
    EXPECT_TRUE(mSpyDriver.uploads[0].hasCallback);
    EXPECT_EQ(mSpyDriver.uploads[0].alignmentCheck % alignof(PerRenderableData), 0u);

    ASSERT_EQ(mSpyDriver.uploads[0].elements.size(), 2u);
    float xs[2] = {
        mSpyDriver.uploads[0].elements[0].worldFromModelMatrix[3][0],
        mSpyDriver.uploads[0].elements[1].worldFromModelMatrix[3][0],
    };
    std::sort(std::begin(xs), std::end(xs));
    EXPECT_FLOAT_EQ(xs[0], 1000.0f);
    EXPECT_FLOAT_EQ(xs[1], 2000.0f);
}

TEST_F(RenderPassAutoInstancingTest, ExactMaxInstancesBatch) {
    MaterialInstance* const sharedMi = createMaterialInstance();
    for (size_t i = 0; i < CONFIG_MAX_INSTANCES; ++i) {
        addRenderable(sharedMi, float(i));
    }

    Range<uint32_t> const vr = prepareScene();
    ASSERT_EQ(vr.size(), CONFIG_MAX_INSTANCES);

    flushAndExecute();
    mSpyDriver.resetSpyRecords();

    RenderPass pass = buildDepthPass(vr);
    ASSERT_EQ(std::distance(pass.begin(), pass.end()), 1);
    EXPECT_EQ(pass.begin()->info.instanceCount, CONFIG_MAX_INSTANCES);
    EXPECT_EQ(pass.begin()->info.index, 0u);

    pass.finalize(*mFEngine, mDriverApi);
    flushAndExecute();

    // Exact single-chunk fit: both uploadSize and uboSize equal sizeof(PerRenderableUib) with
    // zero slack (previously uboSize was 2 * sizeof(PerRenderableUib)).
    ASSERT_EQ(mSpyDriver.createdBuffers.size(), 1u);
    EXPECT_EQ(mSpyDriver.createdBuffers[0].byteCount, sizeof(PerRenderableUib));

    ASSERT_EQ(mSpyDriver.uploads.size(), 1u);
    EXPECT_EQ(mSpyDriver.uploads[0].size, sizeof(PerRenderableUib));
}

TEST_F(RenderPassAutoInstancingTest, MultiChunkAutoInstancingAndDynamicOffsets) {
    constexpr size_t TOTAL_INSTANCES = CONFIG_MAX_INSTANCES + 2;
    MaterialInstance* const sharedMi = createMaterialInstance();
    for (size_t i = 0; i < TOTAL_INSTANCES; ++i) {
        addRenderable(sharedMi, float(10 + i));
    }

    Range<uint32_t> const vr = prepareScene();
    ASSERT_EQ(vr.size(), TOTAL_INSTANCES);

    flushAndExecute();
    mSpyDriver.resetSpyRecords();

    RenderPass pass = buildDepthPass(vr);
    // Chunk 0 has CONFIG_MAX_INSTANCES instances at index 0; chunk 1 has 2 instances at index
    // CONFIG_MAX_INSTANCES.
    ASSERT_EQ(std::distance(pass.begin(), pass.end()), 2);
    EXPECT_EQ(pass.begin()[0].info.instanceCount, CONFIG_MAX_INSTANCES);
    EXPECT_EQ(pass.begin()[0].info.index, 0u);
    EXPECT_EQ(pass.begin()[1].info.instanceCount, 2u);
    EXPECT_EQ(pass.begin()[1].info.index, uint32_t(CONFIG_MAX_INSTANCES));

    pass.finalize(*mFEngine, mDriverApi);

    RenderPass::Executor const executor = pass.getExecutor();
    executor.execute(*mFEngine, mDriverApi);
    flushAndExecute();

    size_t const expectedUploadSize = TOTAL_INSTANCES * sizeof(PerRenderableData);
    size_t const expectedMaxDynamicOffset = CONFIG_MAX_INSTANCES * sizeof(PerRenderableData);
    size_t const expectedUboSize = expectedMaxDynamicOffset + sizeof(PerRenderableUib);

    ASSERT_EQ(mSpyDriver.createdBuffers.size(), 1u);
    uint32_t const uboByteCount = mSpyDriver.createdBuffers[0].byteCount;
    EXPECT_EQ(uboByteCount, expectedUboSize);

    ASSERT_EQ(mSpyDriver.uploads.size(), 1u);
    EXPECT_EQ(mSpyDriver.uploads[0].size, expectedUploadSize);
    ASSERT_EQ(mSpyDriver.uploads[0].elements.size(), TOTAL_INSTANCES);

    // Verify all uploaded transforms are present without corruption or zero-fill overwrites.
    std::vector<float> uploadedXs;
    uploadedXs.reserve(TOTAL_INSTANCES);
    for (PerRenderableData const& elem : mSpyDriver.uploads[0].elements) {
        uploadedXs.push_back(elem.worldFromModelMatrix[3][0]);
    }
    std::sort(uploadedXs.begin(), uploadedXs.end());
    for (size_t i = 0; i < TOTAL_INSTANCES; ++i) {
        EXPECT_FLOAT_EQ(uploadedXs[i], float(10 + i));
    }

    // Verify descriptor-set bound range and dynamic offsets stay strictly within uboByteCount.
    ASSERT_EQ(mSpyDriver.descriptorBufferUpdates.size(), 1u);
    DescriptorSetHandle const instancedDsh = mSpyDriver.descriptorBufferUpdates[0].dsh;
    uint32_t const boundRange = mSpyDriver.descriptorBufferUpdates[0].size;
    EXPECT_EQ(boundRange, sizeof(PerRenderableUib));

    std::vector<uint32_t> perRenderableOffsets;
    for (auto const& bind : mSpyDriver.descriptorBinds) {
        if (bind.set == +DescriptorSetBindingPoints::PER_RENDERABLE) {
            EXPECT_EQ(bind.dsh, instancedDsh);
            EXPECT_LE(size_t(bind.dynamicOffset0) + size_t(boundRange), size_t(uboByteCount));
            perRenderableOffsets.push_back(bind.dynamicOffset0);
        }
    }
    ASSERT_EQ(perRenderableOffsets.size(), 2u);
    EXPECT_EQ(perRenderableOffsets[0], 0u);
    EXPECT_EQ(perRenderableOffsets[1], uint32_t(expectedMaxDynamicOffset));

    ASSERT_EQ(mSpyDriver.draws.size(), 2u);
    EXPECT_EQ(mSpyDriver.draws[0].instanceCount, uint32_t(CONFIG_MAX_INSTANCES));
    EXPECT_EQ(mSpyDriver.draws[1].instanceCount, 2u);
}

TEST_F(RenderPassAutoInstancingTest, NoAutoInstancingZeroAllocations) {
    for (size_t i = 0; i < 16; ++i) {
        MaterialInstance* const uniqueMi = createMaterialInstance();
        addRenderable(uniqueMi, float(i));
    }

    Range<uint32_t> const vr = prepareScene();
    flushAndExecute();
    mSpyDriver.resetSpyRecords();

    RenderPass pass = buildDepthPass(vr);
    EXPECT_EQ(std::distance(pass.begin(), pass.end()), 16);

    pass.finalize(*mFEngine, mDriverApi);
    flushAndExecute();

    EXPECT_TRUE(mSpyDriver.createdBuffers.empty());
    EXPECT_TRUE(mSpyDriver.uploads.empty());
    EXPECT_TRUE(mSpyDriver.descriptorBufferUpdates.empty());
}

// A RenderPass destroyed without finalize() must not create or upload the instancing UBO.
//
// This test cannot observe the release of the CPU staging buffer itself: ~RenderPass() runs
// ~BufferDescriptor(), whose callback is a stateless utils::aligned_free() with no side effect
// visible from here. Detecting a leak or double free on that path is delegated to running this
// test under ASan/LSan; the assertions below would pass regardless.
TEST_F(RenderPassAutoInstancingTest, UnfinalizedRenderPassIssuesNoDriverCommands) {
    MaterialInstance* const sharedMi = createMaterialInstance();
    addRenderable(sharedMi, 1.0f);
    addRenderable(sharedMi, 2.0f);
    addRenderable(sharedMi, 3.0f);

    Range<uint32_t> const vr = prepareScene();
    flushAndExecute();
    mSpyDriver.resetSpyRecords();

    {
        RenderPass pass = buildDepthPass(vr);
        ASSERT_EQ(std::distance(pass.begin(), pass.end()), 1);
        EXPECT_EQ(pass.begin()->info.instanceCount, 3u);
        // Intentionally destroy `pass` without calling finalize().
    }

    flushAndExecute();
    EXPECT_TRUE(mSpyDriver.createdBuffers.empty());
    EXPECT_TRUE(mSpyDriver.uploads.empty());
}

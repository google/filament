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

#include "CommandStreamDispatcher.h"
#include "MockDriver.h"

#include "components/RenderableManager.h"

#include "details/Engine.h"
#include "details/Scene.h"
#include "details/View.h"

#include "ds/DescriptorSet.h"
#include "ds/DescriptorSetLayout.h"

#include <private/filament/EngineEnums.h>
#include <private/filament/UibStructs.h>

#include <filament/Box.h>
#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/LightManager.h>
#include <filament/MorphTargetBuffer.h>
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/SkinningBuffer.h>
#include <filament/SwapChain.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>
#include <filament/Viewport.h>

#include <backend/DriverEnums.h>
#include <backend/Handle.h>
#include <backend/Platform.h>

#include <utils/CString.h>
#include <utils/Entity.h>
#include <utils/EntityManager.h>
#include <utils/FixedCapacityVector.h>
#include <utils/ImmutableCString.h>
#include <utils/Mutex.h>

#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <variant>
#include <vector>

namespace {

using namespace filament;
using namespace filament::backend;
using namespace filament::math;
using namespace utils;

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

struct PushConstantCall {
    ShaderStage stage;
    uint8_t index;
    PushConstantVariant value;
};

struct BufferTrace {
    mutable Mutex lock;
    std::unordered_map<HandleBase::HandleId, uint32_t> liveBuffers UTILS_GUARDED_BY(lock);
    std::vector<uint32_t> dynamicUboSizes UTILS_GUARDED_BY(lock);
    std::vector<PushConstantCall> pushConstants UTILS_GUARDED_BY(lock);
};

class TrackingDriver : public MockDriver {
public:
    explicit TrackingDriver(BufferTrace* trace, size_t maxUboSize = 65536)
            : mTrace(trace) {
        ON_CALL(*this, getFeatureLevel()).WillByDefault(Return(FeatureLevel::FEATURE_LEVEL_1));
        ON_CALL(*this, isTextureFormatSupported(_)).WillByDefault(Return(true));
        ON_CALL(*this, isRenderTargetFormatSupported(_)).WillByDefault(Return(true));
        ON_CALL(*this, isTextureFormatMipmappable(_)).WillByDefault(Return(true));
        ON_CALL(*this, isTextureFormatFilterable(_)).WillByDefault(Return(true));
        ON_CALL(*this, getMaxTextureSize(_)).WillByDefault(Return(size_t(2048)));
        ON_CALL(*this, getMaxUniformBufferSize()).WillByDefault(Return(maxUboSize));
        ON_CALL(*this, getUniformBufferOffsetAlignment()).WillByDefault(Return(size_t(256)));
        ON_CALL(*this, getMaxDrawBuffers()).WillByDefault(Return(uint8_t(4)));
    }

    Dispatcher getDispatcher() const noexcept override {
        return ConcreteDispatcher<TrackingDriver>::make();
    }

    void createBufferObjectR(BufferObjectHandle boh, uint32_t byteCount,
            BufferObjectBinding bindingType, BufferUsage usage, ImmutableCString&&) {
        LockGuard const guard(mTrace->lock);
        mTrace->liveBuffers[boh.getId()] = byteCount;
        if (bindingType == BufferObjectBinding::UNIFORM && usage == BufferUsage::DYNAMIC) {
            mTrace->dynamicUboSizes.push_back(byteCount);
        }
    }

    void destroyBufferObject(BufferObjectHandle boh) {
        if (boh) {
            LockGuard const guard(mTrace->lock);
            mTrace->liveBuffers.erase(boh.getId());
        }
    }

    void setPushConstant(ShaderStage stage, uint8_t index, PushConstantVariant value) {
        LockGuard const guard(mTrace->lock);
        mTrace->pushConstants.push_back({ stage, index, value });
    }

private:
    BufferTrace* mTrace;
};

class TrackingPlatform : public Platform {
public:
    explicit TrackingPlatform(BufferTrace* trace, size_t maxUboSize = 65536)
            : mTrace(trace), mMaxUboSize(maxUboSize) {}

    int getOSVersion() const noexcept override { return 0; }

    CString getDeviceInfo(DeviceInfoType, Driver*) const override { return {}; }

    Driver* createDriver(void*, const Platform::DriverConfig&) override {
        return new NiceMock<TrackingDriver>(mTrace, mMaxUboSize);
    }

private:
    BufferTrace* mTrace;
    size_t mMaxUboSize;
};

class DeformationAndViewMemoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        mPlatform = std::make_unique<TrackingPlatform>(&mTrace);
        mEngine = Engine::Builder()
                .backend(Engine::Backend::NOOP)
                .platform(mPlatform.get())
                .build();
        ASSERT_NE(nullptr, mEngine);
    }

    void TearDown() override {
        cleanup();
        Engine::destroy(&mEngine);
        LockGuard const guard(mTrace.lock);
        EXPECT_TRUE(mTrace.liveBuffers.empty());
    }

    Entity createEntity() {
        Entity const e = mEngine->getEntityManager().create();
        mEntities.push_back(e);
        return e;
    }

    Scene* createScene() {
        Scene* const scene = mEngine->createScene();
        mScenes.push_back(scene);
        return scene;
    }

    View* createConfiguredView(Scene* scene) {
        View* const view = mEngine->createView();
        mViews.push_back(view);
        Entity const camEntity = createEntity();
        Camera* const camera = mEngine->createCamera(camEntity);
        mCameraEntities.push_back(camEntity);
        camera->setProjection(45.0, 1.0, 0.1, 100.0);
        camera->lookAt({ 0, 0, 5 }, { 0, 0, 0 }, { 0, 1, 0 });
        view->setCamera(camera);
        view->setViewport({ 0, 0, 64, 64 });
        view->setScene(scene);
        return view;
    }

    VertexBuffer* createSkinnedVertexBuffer() {
        VertexBuffer* const vb = VertexBuffer::Builder()
                .vertexCount(3)
                .bufferCount(3)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3)
                .attribute(VertexAttribute::BONE_INDICES, 1, VertexBuffer::AttributeType::USHORT4)
                .attribute(VertexAttribute::BONE_WEIGHTS, 2, VertexBuffer::AttributeType::FLOAT4)
                .build(*mEngine);
        mVertexBuffers.push_back(vb);
        return vb;
    }

    VertexBuffer* createAdvancedSkinnedVertexBuffer() {
        VertexBuffer* const vb = VertexBuffer::Builder()
                .vertexCount(3)
                .bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3)
                .advancedSkinning(true)
                .build(*mEngine);
        mVertexBuffers.push_back(vb);
        return vb;
    }

    VertexBuffer* createBasicVertexBuffer() {
        VertexBuffer* const vb = VertexBuffer::Builder()
                .vertexCount(3)
                .bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3)
                .build(*mEngine);
        mVertexBuffers.push_back(vb);
        return vb;
    }

    IndexBuffer* createTriangleIndexBuffer() {
        IndexBuffer* const ib = IndexBuffer::Builder()
                .indexCount(3)
                .bufferType(IndexBuffer::IndexType::USHORT)
                .build(*mEngine);
        mIndexBuffers.push_back(ib);
        return ib;
    }

    MorphTargetBuffer* createMorphTargetBuffer(size_t count = 2) {
        MorphTargetBuffer* const mtb = MorphTargetBuffer::Builder()
                .vertexCount(3)
                .count(count)
                .build(*mEngine);
        mMorphTargetBuffers.push_back(mtb);
        return mtb;
    }

    SkinningBuffer* createSkinningBuffer(size_t boneCount = 256) {
        SkinningBuffer* const sb = SkinningBuffer::Builder()
                .boneCount(boneCount)
                .initialize(true)
                .build(*mEngine);
        mSkinningBuffers.push_back(sb);
        return sb;
    }

    void renderFrame(View* view) {
        if (!mRenderer) {
            mRenderer = mEngine->createRenderer();
            mSwapChain = mEngine->createSwapChain(64, 64);
        }
        mVsyncNanos += 16'666'667;
        ASSERT_TRUE(mRenderer->beginFrame(mSwapChain, mVsyncNanos));
        mRenderer->render(view);
        mRenderer->endFrame();
        mEngine->flushAndWait();
    }

    void cleanup() {
        if (!mEngine) {
            return;
        }
        for (View* v : mViews) {
            mEngine->destroy(v);
        }
        mViews.clear();
        for (Scene* s : mScenes) {
            mEngine->destroy(s);
        }
        mScenes.clear();
        for (Entity const camEntity : mCameraEntities) {
            mEngine->destroyCameraComponent(camEntity);
        }
        mCameraEntities.clear();
        auto& em = mEngine->getEntityManager();
        for (Entity const e : mEntities) {
            mEngine->destroy(e);
            if (em.isAlive(e)) {
                em.destroy(e);
            }
        }
        mEntities.clear();
        for (MorphTargetBuffer* mtb : mMorphTargetBuffers) {
            mEngine->destroy(mtb);
        }
        mMorphTargetBuffers.clear();
        for (SkinningBuffer* sb : mSkinningBuffers) {
            mEngine->destroy(sb);
        }
        mSkinningBuffers.clear();
        for (VertexBuffer* vb : mVertexBuffers) {
            mEngine->destroy(vb);
        }
        mVertexBuffers.clear();
        for (IndexBuffer* ib : mIndexBuffers) {
            mEngine->destroy(ib);
        }
        mIndexBuffers.clear();
        if (mRenderer) {
            mEngine->destroy(mSwapChain);
            mEngine->destroy(mRenderer);
            mSwapChain = nullptr;
            mRenderer = nullptr;
        }
    }

    BufferTrace mTrace;
    std::unique_ptr<TrackingPlatform> mPlatform;
    Engine* mEngine = nullptr;
    Renderer* mRenderer = nullptr;
    SwapChain* mSwapChain = nullptr;
    uint64_t mVsyncNanos = 0;
    std::vector<View*> mViews;
    std::vector<Scene*> mScenes;
    std::vector<Entity> mCameraEntities;
    std::vector<Entity> mEntities;
    std::vector<VertexBuffer*> mVertexBuffers;
    std::vector<IndexBuffer*> mIndexBuffers;
    std::vector<MorphTargetBuffer*> mMorphTargetBuffers;
    std::vector<SkinningBuffer*> mSkinningBuffers;
};

} // namespace

TEST_F(DeformationAndViewMemoryTest, SkinnedOnlyRenderableSkipsMorphUboAndBoneWeightsTexture) {
    Scene* const scene = createScene();
    View* const view = createConfiguredView(scene);

    Entity const e = createEntity();
    mEngine->getTransformManager().create(e);
    RenderableManager::Builder(1)
            .boundingBox({ { -1, -1, -1 }, { 1, 1, 1 } })
            .geometry(0, RenderableManager::PrimitiveType::TRIANGLES,
                    createSkinnedVertexBuffer(), createTriangleIndexBuffer(), 0, 3)
            .skinning(8)
            .build(*mEngine, e);
    scene->addEntity(e);

    FEngine& fEngine = *downcast(mEngine);
    FRenderableManager& rcm = fEngine.getRenderableManager();
    auto const ci = rcm.getInstance(e);

    // Skinned-only with <= 4 weights/vertex: only bones.handle should be allocated.
    auto const skinningInfo = rcm.getSkinningBufferInfo(ci);
    auto const morphingInfo = rcm.getMorphingBufferInfo(ci);
    EXPECT_TRUE(bool(skinningInfo.handle));
    EXPECT_FALSE(bool(skinningInfo.boneIndicesAndWeightHandle));
    EXPECT_FALSE(bool(morphingInfo.handle));
    EXPECT_EQ(0u, morphingInfo.count);
    EXPECT_EQ(nullptr, morphingInfo.morphTargetBuffer);

    renderFrame(view);

    // All bindings in the per-renderable descriptor set must be populated (using dummy fallbacks).
    auto const& ds = rcm.getDescriptorSet(ci);
    EXPECT_TRUE(bool(ds.getHandle()));
    EXPECT_EQ(fEngine.getPerRenderableDescriptorSetLayout().getValidDescriptors(),
            ds.getValidDescriptors());

    // SKINNING_OR_MORPHING variants always declare pushConstants.morphingBufferOffset; WebGPU
    // requires immediate data at index 0 to be set even for skinned-only draws without morphing.
    {
        LockGuard const guard(mTrace.lock);
        EXPECT_FALSE(mTrace.pushConstants.empty());
        for (PushConstantCall const& call : mTrace.pushConstants) {
            EXPECT_EQ(ShaderStage::VERTEX, call.stage);
            EXPECT_EQ(+PushConstantIds::MORPHING_BUFFER_OFFSET, call.index);
            int32_t const* const intVal = std::get_if<int32_t>(&call.value);
            ASSERT_NE(nullptr, intVal);
            EXPECT_EQ(0, *intVal);
        }
    }
}

TEST_F(DeformationAndViewMemoryTest,
        SkinnedRenderableWithMoreThanFourWeightsAllocatesWeightsTextureOnly) {
    Scene* const scene = createScene();
    View* const view = createConfiguredView(scene);

    // 3 vertices, each with 6 bone (index, weight) pairs (> 4 triggers texture skinning).
    FixedCapacityVector<FixedCapacityVector<float2>> bonePairs(3);
    for (size_t v = 0; v < 3; ++v) {
        FixedCapacityVector<float2> pairs(6);
        for (size_t b = 0; b < 6; ++b) {
            pairs[b] = float2{ float(b), 1.0f / 6.0f };
        }
        bonePairs[v] = std::move(pairs);
    }

    Entity const e = createEntity();
    mEngine->getTransformManager().create(e);
    RenderableManager::Builder(1)
            .boundingBox({ { -1, -1, -1 }, { 1, 1, 1 } })
            .geometry(0, RenderableManager::PrimitiveType::TRIANGLES,
                    createAdvancedSkinnedVertexBuffer(), createTriangleIndexBuffer(), 0, 3)
            .skinning(8)
            .boneIndicesAndWeights(0, std::move(bonePairs))
            .build(*mEngine, e);
    scene->addEntity(e);

    FEngine& fEngine = *downcast(mEngine);
    FRenderableManager& rcm = fEngine.getRenderableManager();
    auto const ci = rcm.getInstance(e);

    auto const skinningInfo = rcm.getSkinningBufferInfo(ci);
    auto const morphingInfo = rcm.getMorphingBufferInfo(ci);
    EXPECT_TRUE(bool(skinningInfo.handle));
    EXPECT_TRUE(bool(skinningInfo.boneIndicesAndWeightHandle));
    EXPECT_FALSE(bool(morphingInfo.handle));

    renderFrame(view);

    auto const& ds = rcm.getDescriptorSet(ci);
    EXPECT_TRUE(bool(ds.getHandle()));
    EXPECT_EQ(fEngine.getPerRenderableDescriptorSetLayout().getValidDescriptors(),
            ds.getValidDescriptors());
}

TEST_F(DeformationAndViewMemoryTest, MorphOnlyRenderableSkipsBoneUboAndBoneWeightsTexture) {
    Scene* const scene = createScene();
    View* const view = createConfiguredView(scene);
    MorphTargetBuffer* const mtb = createMorphTargetBuffer(2);

    // 1. Standard MorphTargetBuffer morph-only renderable.
    Entity const eStandard = createEntity();
    mEngine->getTransformManager().create(eStandard);
    RenderableManager::Builder(1)
            .boundingBox({ { -1, -1, -1 }, { 1, 1, 1 } })
            .geometry(0, RenderableManager::PrimitiveType::TRIANGLES,
                    createBasicVertexBuffer(), createTriangleIndexBuffer(), 0, 3)
            .morphing(mtb)
            .morphing(0, 0, 0)
            .build(*mEngine, eStandard);
    scene->addEntity(eStandard);

    // 2. Legacy vertex-attribute morph-only renderable (no MorphTargetBuffer).
    Entity const eLegacy = createEntity();
    mEngine->getTransformManager().create(eLegacy);
    RenderableManager::Builder(1)
            .boundingBox({ { -1, -1, -1 }, { 1, 1, 1 } })
            .geometry(0, RenderableManager::PrimitiveType::TRIANGLES,
                    createBasicVertexBuffer(), createTriangleIndexBuffer(), 0, 3)
            .morphing(2)
            .build(*mEngine, eLegacy);
    scene->addEntity(eLegacy);

    FEngine& fEngine = *downcast(mEngine);
    FRenderableManager& rcm = fEngine.getRenderableManager();

    for (Entity const e : { eStandard, eLegacy }) {
        auto const ci = rcm.getInstance(e);
        auto const skinningInfo = rcm.getSkinningBufferInfo(ci);
        auto const morphingInfo = rcm.getMorphingBufferInfo(ci);
        EXPECT_FALSE(bool(skinningInfo.handle));
        EXPECT_FALSE(bool(skinningInfo.boneIndicesAndWeightHandle));
        EXPECT_TRUE(bool(morphingInfo.handle));
        EXPECT_EQ(2u, morphingInfo.count);
        EXPECT_NE(nullptr, morphingInfo.morphTargetBuffer);
    }

    renderFrame(view);

    for (Entity const e : { eStandard, eLegacy }) {
        auto const ci = rcm.getInstance(e);
        auto const& ds = rcm.getDescriptorSet(ci);
        EXPECT_TRUE(bool(ds.getHandle()));
        EXPECT_EQ(fEngine.getPerRenderableDescriptorSetLayout().getValidDescriptors(),
                ds.getValidDescriptors());
    }

    // eStandard uses MorphTargetBuffer (MorphType::POSITION -> SKINNING_OR_MORPHING variant),
    // while legacy .morphing(2) without a MorphTargetBuffer leaves MorphType::NONE.
    {
        LockGuard const guard(mTrace.lock);
        EXPECT_EQ(1u, mTrace.pushConstants.size());
        for (PushConstantCall const& call : mTrace.pushConstants) {
            EXPECT_EQ(ShaderStage::VERTEX, call.stage);
            EXPECT_EQ(+PushConstantIds::MORPHING_BUFFER_OFFSET, call.index);
            int32_t const* const intVal = std::get_if<int32_t>(&call.value);
            ASSERT_NE(nullptr, intVal);
            EXPECT_EQ(0, *intVal);
        }
    }
}

TEST_F(DeformationAndViewMemoryTest, SkinningBufferModePreservesDisabledSkinningAndSkipsInternalUbos) {
    Scene* const scene = createScene();
    View* const view = createConfiguredView(scene);
    SkinningBuffer* const sb = createSkinningBuffer(256);

    Entity const e = createEntity();
    mEngine->getTransformManager().create(e);
    RenderableManager::Builder(1)
            .boundingBox({ { -1, -1, -1 }, { 1, 1, 1 } })
            .geometry(0, RenderableManager::PrimitiveType::TRIANGLES,
                    createSkinnedVertexBuffer(), createTriangleIndexBuffer(), 0, 3)
            .enableSkinningBuffers(true)
            .skinning(sb, 0, 0)
            .build(*mEngine, e);
    scene->addEntity(e);

    FRenderableManager& rcm = downcast(mEngine)->getRenderableManager();
    auto const ci = rcm.getInstance(e);
    EXPECT_TRUE(bool(rcm.getSkinningBufferInfo(ci).handle));
    EXPECT_FALSE(bool(rcm.getSkinningBufferInfo(ci).boneIndicesAndWeightHandle));
    EXPECT_FALSE(bool(rcm.getMorphingBufferInfo(ci).handle));
    EXPECT_FALSE(rcm.getSkinning(ci).skinning);

    // Updating the SkinningBuffer slice must not mutate the renderable's build-time skinning flag.
    rcm.setSkinningBuffer(ci, downcast(sb), 16, 0);
    EXPECT_FALSE(rcm.getSkinning(ci).skinning);

    renderFrame(view);
    EXPECT_TRUE(bool(rcm.getDescriptorSet(ci).getHandle()));

    {
        LockGuard const guard(mTrace.lock);
        EXPECT_TRUE(mTrace.pushConstants.empty());
    }
}

TEST_F(DeformationAndViewMemoryTest, ViewLazilyAllocatesDynamicLightingUbos) {
    Scene* const scene = createScene();
    View* const view = createConfiguredView(scene);
    FEngine& fEngine = *downcast(mEngine);
    size_t const lightsUboSize = CONFIG_MAX_LIGHT_COUNT * sizeof(LightsUib);
    size_t const froxelUboSize = Froxelizer::getFroxelBufferByteCount(fEngine.getDriverApi());
    size_t const recordUboSize = Froxelizer::getFroxelRecordBufferByteCount(fEngine.getDriverApi());

    Entity const meshEntity = createEntity();
    mEngine->getTransformManager().create(meshEntity);
    RenderableManager::Builder(1)
            .boundingBox({ { -1, -1, -1 }, { 1, 1, 1 } })
            .geometry(0, RenderableManager::PrimitiveType::TRIANGLES,
                    createBasicVertexBuffer(), createTriangleIndexBuffer(), 0, 3)
            .build(*mEngine, meshEntity);
    scene->addEntity(meshEntity);

    // Directional light alone should NOT trigger dynamic-lighting UBO allocation.
    Entity const dirLight = createEntity();
    LightManager::Builder(LightManager::Type::DIRECTIONAL)
            .direction({ 0.0f, -1.0f, -1.0f })
            .intensity(100000.0f)
            .build(*mEngine, dirLight);
    scene->addEntity(dirLight);

    renderFrame(view);

    size_t dynamicUboCountBeforePointLight = 0;
    {
        LockGuard const guard(mTrace.lock);
        dynamicUboCountBeforePointLight = mTrace.dynamicUboSizes.size();
        EXPECT_EQ(0, std::count(mTrace.dynamicUboSizes.begin(), mTrace.dynamicUboSizes.end(),
                             uint32_t(lightsUboSize)));
        EXPECT_EQ(0, std::count(mTrace.dynamicUboSizes.begin(), mTrace.dynamicUboSizes.end(),
                             uint32_t(recordUboSize)));
    }

    // Adding a visible point light must lazily allocate LightsUib and both Froxelizer UBOs.
    Entity const pointLight = createEntity();
    LightManager::Builder(LightManager::Type::POINT)
            .position({ 0.0f, 0.0f, 0.0f })
            .falloff(10.0f)
            .intensity(1000.0f)
            .build(*mEngine, pointLight);
    scene->addEntity(pointLight);

    renderFrame(view);

    {
        LockGuard const guard(mTrace.lock);
        ASSERT_EQ(dynamicUboCountBeforePointLight + 3u, mTrace.dynamicUboSizes.size());
        std::vector<uint32_t> newUbos(
                mTrace.dynamicUboSizes.begin() + ptrdiff_t(dynamicUboCountBeforePointLight),
                mTrace.dynamicUboSizes.end());
        EXPECT_NE(newUbos.end(), std::find(newUbos.begin(), newUbos.end(), uint32_t(lightsUboSize)));
        EXPECT_NE(newUbos.end(), std::find(newUbos.begin(), newUbos.end(), uint32_t(froxelUboSize)));
        EXPECT_NE(newUbos.end(), std::find(newUbos.begin(), newUbos.end(), uint32_t(recordUboSize)));
    }
}

TEST(DeformationAndViewMemoryStandaloneTest,
        FroxelBufferSizeMatchesDescriptorRangeForNon64ByteMultipleMaxUbo) {
    // 16400 == 16384 + 16: getFroxelBufferByteCount() returns 16400 (16-byte multiple), while
    // mFroxelBufferEntryCount * sizeof(FroxelEntry) masks down to 16384 (64-byte multiple).
    // Verify Froxelizer::ensureBuffersCreated() allocates the full 16400-byte descriptor range.
    constexpr size_t kCustomMaxUboSize = 16400;
    BufferTrace trace;
    TrackingPlatform platform(&trace, kCustomMaxUboSize);
    Engine* engine = Engine::Builder()
            .backend(Engine::Backend::NOOP)
            .platform(&platform)
            .build();
    ASSERT_NE(engine, nullptr);

    FEngine& fEngine = *downcast(engine);
    size_t const expectedFroxelUboSize = Froxelizer::getFroxelBufferByteCount(fEngine.getDriverApi());
    EXPECT_EQ(kCustomMaxUboSize, expectedFroxelUboSize);

    Scene* const scene = engine->createScene();
    View* const view = engine->createView();
    Entity const cameraEntity = EntityManager::get().create();
    Camera* const camera = engine->createCamera(cameraEntity);
    camera->setProjection(45.0, 1.0, 0.1, 100.0, Camera::Fov::VERTICAL);
    view->setScene(scene);
    view->setCamera(camera);
    view->setViewport({ 0, 0, 128, 128 });

    Renderer* const renderer = engine->createRenderer();
    SwapChain* const swapChain = engine->createSwapChain(128, 128);

    Entity const light = EntityManager::get().create();
    LightManager::Builder(LightManager::Type::POINT)
            .position({ 0.0f, 0.0f, 0.0f })
            .falloff(10.0f)
            .intensity(1000.0f)
            .build(*engine, light);
    scene->addEntity(light);

    size_t dynamicUboCountBefore = 0;
    {
        LockGuard const guard(trace.lock);
        dynamicUboCountBefore = trace.dynamicUboSizes.size();
    }

    if (renderer->beginFrame(swapChain)) {
        renderer->render(view);
        renderer->endFrame();
    }
    engine->flushAndWait();

    {
        LockGuard const guard(trace.lock);
        std::vector<uint32_t> newUbos(
                trace.dynamicUboSizes.begin() + ptrdiff_t(dynamicUboCountBefore),
                trace.dynamicUboSizes.end());
        // Both Froxelizer UBOs (record + froxel) equal kCustomMaxUboSize (16400), not 16384.
        EXPECT_EQ(2, std::count(newUbos.begin(), newUbos.end(), uint32_t(expectedFroxelUboSize)));
        EXPECT_EQ(0, std::count(newUbos.begin(), newUbos.end(), uint32_t(16384)));
    }

    scene->remove(light);
    engine->destroy(light);
    engine->destroyCameraComponent(cameraEntity);
    EntityManager::get().destroy(light);
    EntityManager::get().destroy(cameraEntity);
    engine->destroy(swapChain);
    engine->destroy(renderer);
    engine->destroy(view);
    engine->destroy(scene);
    Engine::destroy(&engine);
}

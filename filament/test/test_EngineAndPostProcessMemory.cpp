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

#include "details/Engine.h"
#include "details/View.h"

#include "ds/SsrPassDescriptorSet.h"

#include <private/filament/UibStructs.h>

#include <filament/Camera.h>
#include <filament/ColorGrading.h>
#include <filament/Engine.h>
#include <filament/Options.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/SwapChain.h>
#include <filament/ToneMapper.h>
#include <filament/View.h>
#include <filament/Viewport.h>

#include <backend/DriverEnums.h>
#include <backend/Handle.h>
#include <backend/PixelBufferDescriptor.h>
#include <backend/Platform.h>
#include <backend/Program.h>

#include <utils/CString.h>
#include <utils/Entity.h>
#include <utils/EntityManager.h>
#include <utils/ImmutableCString.h>
#include <utils/Mutex.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>

namespace {

using namespace filament;
using namespace filament::backend;
using namespace utils;

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

struct ResourceTrace {
    mutable Mutex lock;
    std::unordered_map<HandleBase::HandleId, uint32_t> liveBuffers UTILS_GUARDED_BY(lock);
    std::unordered_map<HandleBase::HandleId, std::tuple<uint32_t, uint32_t, TextureFormat>>
            liveTextures UTILS_GUARDED_BY(lock);
    std::unordered_map<HandleBase::HandleId, std::string> liveDescriptorSets UTILS_GUARDED_BY(lock);
    size_t totalUboCreations UTILS_GUARDED_BY(lock) = 0;
    size_t staticShadowUboCreations UTILS_GUARDED_BY(lock) = 0;
    size_t perViewUboCreations UTILS_GUARDED_BY(lock) = 0;
    size_t starburstTextureCreations UTILS_GUARDED_BY(lock) = 0;
};

class TrackingDriver : public MockDriver {
public:
    explicit TrackingDriver(ResourceTrace* trace)
            : mTrace(trace) {
        ON_CALL(*this, getFeatureLevel()).WillByDefault(Return(FeatureLevel::FEATURE_LEVEL_1));
        ON_CALL(*this, isTextureFormatSupported(_)).WillByDefault(Return(true));
        ON_CALL(*this, isRenderTargetFormatSupported(_)).WillByDefault(Return(true));
        ON_CALL(*this, isTextureFormatMipmappable(_)).WillByDefault(Return(true));
        ON_CALL(*this, isTextureFormatFilterable(_)).WillByDefault(Return(true));
        ON_CALL(*this, getMaxTextureSize(_)).WillByDefault(Return(size_t(2048)));
        ON_CALL(*this, getMaxUniformBufferSize()).WillByDefault(Return(size_t(65536)));
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
        if (bindingType == BufferObjectBinding::UNIFORM) {
            mTrace->totalUboCreations++;
            if (usage == BufferUsage::STATIC && byteCount == sizeof(ShadowUib)) {
                mTrace->staticShadowUboCreations++;
            }
            if (byteCount == sizeof(PerViewUib)) {
                mTrace->perViewUboCreations++;
            }
        }
    }

    void destroyBufferObject(BufferObjectHandle boh) {
        if (boh) {
            LockGuard const guard(mTrace->lock);
            mTrace->liveBuffers.erase(boh.getId());
        }
    }

    void createTextureR(TextureHandle th, SamplerType target, uint8_t, TextureFormat format,
            uint8_t, uint32_t w, uint32_t h, uint32_t, TextureUsage, ImmutableCString&&) {
        LockGuard const guard(mTrace->lock);
        mTrace->liveTextures[th.getId()] = { w, h, format };
        if (target == SamplerType::SAMPLER_2D && format == TextureFormat::R8 && w == 256 &&
                h == 1) {
            mTrace->starburstTextureCreations++;
        }
    }

    void createTextureViewR(TextureHandle th, TextureHandle, uint8_t, uint8_t, ImmutableCString&&) {
        LockGuard const guard(mTrace->lock);
        mTrace->liveTextures[th.getId()] = { 0, 0, TextureFormat::UNUSED };
    }

    void createTextureViewSwizzleR(TextureHandle th, TextureHandle, TextureSwizzle, TextureSwizzle,
            TextureSwizzle, TextureSwizzle, ImmutableCString&&) {
        LockGuard const guard(mTrace->lock);
        mTrace->liveTextures[th.getId()] = { 0, 0, TextureFormat::UNUSED };
    }

    void destroyTexture(TextureHandle th) {
        if (th) {
            LockGuard const guard(mTrace->lock);
            mTrace->liveTextures.erase(th.getId());
        }
    }

    void createDescriptorSetR(DescriptorSetHandle dsh, DescriptorSetLayoutHandle,
            ImmutableCString&& tag) {
        LockGuard const guard(mTrace->lock);
        mTrace->liveDescriptorSets[dsh.getId()] = tag.c_str() ? tag.c_str() : "";
    }

    void destroyDescriptorSet(DescriptorSetHandle dsh) {
        if (dsh) {
            LockGuard const guard(mTrace->lock);
            mTrace->liveDescriptorSets.erase(dsh.getId());
        }
    }

private:
    ResourceTrace* mTrace;
};

class TrackingPlatform : public Platform {
public:
    explicit TrackingPlatform(ResourceTrace* trace)
            : mTrace(trace) {}

    int getOSVersion() const noexcept override { return 0; }

    CString getDeviceInfo(DeviceInfoType, Driver*) const override { return {}; }

    Driver* createDriver(void*, const Platform::DriverConfig&) override {
        return new NiceMock<TrackingDriver>(mTrace);
    }

private:
    ResourceTrace* mTrace;
};

struct LutExportRecord {
    size_t byteSize = 0;
    PixelDataFormat format{};
    PixelDataType type{};
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t depth = 0;
    size_t callCount = 0;
};

void captureExportedLut(void const*, size_t const size, PixelDataFormat const format,
        PixelDataType const type, uint32_t const width, uint32_t const height,
        uint32_t const depth, void* user) {
    auto* const rec = static_cast<LutExportRecord*>(user);
    rec->byteSize = size;
    rec->format = format;
    rec->type = type;
    rec->width = width;
    rec->height = height;
    rec->depth = depth;
    rec->callCount++;
}

} // namespace

// Verifies 4.4.2: Program::shader(ShaderStage, ShaderBlob&&) transfers ownership of the underlying
// FixedCapacityVector buffer without copying or reallocating.
TEST(EngineAndPostProcessMemoryTest, ProgramShaderMoveTransfersBufferWithoutCopy) {
    Program::ShaderBlob blob(128);
    for (size_t i = 0; i < blob.size(); ++i) {
        blob[i] = uint8_t(i ^ 0x5Au);
    }
    uint8_t const* const originalDataPtr = blob.data();
    size_t const originalSize = blob.size();

    Program program;
    program.shader(ShaderStage::VERTEX, std::move(blob));

    EXPECT_TRUE(blob.empty());
    auto const& shaders = program.getShadersSource();
    EXPECT_EQ(originalDataPtr, shaders[size_t(ShaderStage::VERTEX)].data());
    EXPECT_EQ(originalSize, shaders[size_t(ShaderStage::VERTEX)].size());
    EXPECT_TRUE(shaders[size_t(ShaderStage::FRAGMENT)].empty());
}

// Verifies 4.4.3a: ColorGrading allocates 4 B/texel for INTEGER (RGB10_A2), 8 B/texel for FLOAT
// (RGBA16F), and 2 B/texel across 512 elements for 1D LUTs (including with adjustments).
TEST(EngineAndPostProcessMemoryTest, ColorGradingLutByteSizesAnd1DLutBounds) {
    Engine* engine = Engine::Builder()
            .backend(Backend::NOOP)
            .feature("engine.color_grading.use_1d_lut", true)
            .build();
    ASSERT_NE(nullptr, engine);

    // 1. Default 32^3 INTEGER LUT (RGB10_A2, 4 B/texel = 128 KiB).
    LutExportRecord integerLut{};
    ACESToneMapper acesMapper;
    ColorGrading* cgInteger = ColorGrading::Builder()
            .toneMapper(&acesMapper)
            .format(ColorGrading::LutFormat::INTEGER)
            .dimensions(32)
            .exportLut(&captureExportedLut, &integerLut)
            .build(*engine);
    ASSERT_NE(nullptr, cgInteger);
    EXPECT_EQ(1u, integerLut.callCount);
    EXPECT_EQ(PixelDataType::UINT_2_10_10_10_REV, integerLut.type);
    EXPECT_EQ(32u * 32u * 32u * sizeof(uint32_t), integerLut.byteSize);

    // 2. 32^3 FLOAT LUT (RGBA16F, 8 B/texel = 256 KiB).
    LutExportRecord floatLut{};
    ColorGrading* cgFloat = ColorGrading::Builder()
            .toneMapper(&acesMapper)
            .format(ColorGrading::LutFormat::FLOAT)
            .dimensions(32)
            .exportLut(&captureExportedLut, &floatLut)
            .build(*engine);
    ASSERT_NE(nullptr, cgFloat);
    EXPECT_EQ(1u, floatLut.callCount);
    EXPECT_EQ(PixelDataType::HALF, floatLut.type);
    EXPECT_EQ(32u * 32u * 32u * 8u, floatLut.byteSize);

    // 3. 1D HDR LUT (512x1x1 R16F = 1 KiB), exercising Config::precomputed*[512].
    LutExportRecord oneDLut{};
    LinearToneMapper linearMapper;
    ColorGrading* cg1D = ColorGrading::Builder()
            .toneMapper(&linearMapper)
            .exportLut(&captureExportedLut, &oneDLut)
            .build(*engine);
    ASSERT_NE(nullptr, cg1D);
    EXPECT_EQ(1u, oneDLut.callCount);
    EXPECT_EQ(512u, oneDLut.width);
    EXPECT_EQ(1u, oneDLut.height);
    EXPECT_EQ(1u, oneDLut.depth);
    EXPECT_EQ(512u * sizeof(uint16_t), oneDLut.byteSize);

    engine->destroy(cg1D);
    engine->destroy(cgFloat);
    engine->destroy(cgInteger);
    Engine::destroy(&engine);
}

// Verifies 4.4.3b: When no custom ColorGrading::Builder is supplied on Engine::Builder,
// FEngine::mDefaultColorGrading is lazily created only when queried or needed for post-processed
// rendering.
TEST(EngineAndPostProcessMemoryTest, DefaultColorGradingIsLazyWhenNoBuilderSupplied) {
    Engine* engine = Engine::Builder()
            .backend(Backend::NOOP)
            .build();
    ASSERT_NE(nullptr, engine);

    // Engine startup does not eagerly materialize mDefaultColorGrading.
    EXPECT_EQ(0u, engine->getColorGradingCount());

    SwapChain* swapChain = engine->createSwapChain(64, 64);
    Renderer* renderer = engine->createRenderer();
    Scene* scene = engine->createScene();
    View* view = engine->createView();
    Entity const camEntity = engine->getEntityManager().create();
    Camera* camera = engine->createCamera(camEntity);
    camera->setProjection(45.0, 1.0, 0.1, 100.0);
    view->setCamera(camera);
    view->setViewport({ 0, 0, 64, 64 });
    view->setScene(scene);

    // Creating a View still does not materialize mDefaultColorGrading.
    EXPECT_EQ(0u, engine->getColorGradingCount());

    // Rendering with post-processing disabled skips mDefaultColorGrading.
    view->setPostProcessingEnabled(false);
    if (renderer->beginFrame(swapChain)) {
        renderer->render(view);
        renderer->endFrame();
    }
    engine->flushAndWait();
    EXPECT_EQ(0u, engine->getColorGradingCount());

    // Rendering with post-processing enabled AND a custom View ColorGrading also skips default.
    ColorGrading* customCg = ColorGrading::Builder().dimensions(16).build(*engine);
    ASSERT_NE(nullptr, customCg);
    EXPECT_EQ(1u, engine->getColorGradingCount());

    view->setPostProcessingEnabled(true);
    view->setColorGrading(customCg);
    EXPECT_EQ(customCg, view->getColorGrading());
    if (renderer->beginFrame(swapChain)) {
        renderer->render(view);
        renderer->endFrame();
    }
    engine->flushAndWait();
    EXPECT_EQ(1u, engine->getColorGradingCount());

    // Resetting View's ColorGrading to nullptr and querying getColorGrading() lazily builds the
    // default ColorGrading.
    view->setColorGrading(nullptr);
    EXPECT_EQ(1u, engine->getColorGradingCount());

    ColorGrading const* materializedDefault = view->getColorGrading();
    ASSERT_NE(nullptr, materializedDefault);
    EXPECT_NE(customCg, materializedDefault);
    EXPECT_EQ(2u, engine->getColorGradingCount());

    engine->destroy(customCg);
    engine->destroyCameraComponent(camEntity);
    engine->getEntityManager().destroy(camEntity);
    engine->destroy(view);
    engine->destroy(scene);
    engine->destroy(renderer);
    engine->destroy(swapChain);
    Engine::destroy(&engine);
}

// Regression test: A user-supplied Engine::Builder::colorGrading() builder may reference
// caller-scoped objects (ToneMapper*, exportLut user*) whose lifetimes end as soon as
// Engine::Builder::build() returns. FEngine::init() must build it synchronously rather than
// deferring to the first View::getColorGrading() call.
TEST(EngineAndPostProcessMemoryTest, UserSuppliedColorGradingBuilderBuildsSynchronouslyDuringInit) {
    Engine* engine = nullptr;
    {
        auto toneMapper = std::make_unique<ACESToneMapper>();
        auto exportRecord = std::make_unique<LutExportRecord>();
        engine = Engine::Builder()
                .backend(Backend::NOOP)
                .colorGrading(ColorGrading::Builder()
                        .dimensions(16)
                        .toneMapper(toneMapper.get())
                        .exportLut(&captureExportedLut, exportRecord.get()))
                .build();
        ASSERT_NE(nullptr, engine);

        // Must have built synchronously before Engine::Builder::build() returned.
        EXPECT_EQ(1u, engine->getColorGradingCount());
        EXPECT_EQ(1u, exportRecord->callCount);
        EXPECT_EQ(16u, exportRecord->width);
    }
    // Both toneMapper and exportRecord are now destroyed; querying View::getColorGrading() must
    // return the already-constructed default ColorGrading without dereferencing dangling pointers.
    View* view = engine->createView();
    ASSERT_NE(nullptr, view->getColorGrading());
    EXPECT_EQ(1u, engine->getColorGradingCount());

    engine->destroy(view);
    Engine::destroy(&engine);
}

// Verifies 4.4.4: SsrPassDescriptorSet no longer allocates a dedicated ShadowUib UBO, and
// PostProcessManager lazily creates StructureDescriptorSet's GPU UBO/DescriptorSet,
// MotionVectorsDescriptorSet, SsrPassDescriptorSet, and mStarburstTexture, freeing all on shutdown.
TEST(EngineAndPostProcessMemoryTest, PostProcessResourcesAreLazyAndLeakFree) {
    ResourceTrace trace;
    TrackingPlatform platform(&trace);

    Engine* engine = Engine::Builder()
            .backend(Backend::NOOP)
            .platform(&platform)
            .build();
    ASSERT_NE(nullptr, engine);
    engine->flushAndWait();

    FEngine& fengine = *downcast(engine);
    auto hasLiveDescriptorSetTag = [&trace](char const* tag) {
        LockGuard const guard(trace.lock);
        for (auto const& [id, liveTag] : trace.liveDescriptorSets) {
            if (liveTag == tag) {
                return true;
            }
        }
        return false;
    };

    // At Engine startup, none of the optional post-process resources are allocated.
    EXPECT_FALSE(hasLiveDescriptorSetTag("StructureDescriptorSet"));
    EXPECT_FALSE(hasLiveDescriptorSetTag("MotionVectorsDescriptorSet"));
    EXPECT_FALSE(hasLiveDescriptorSetTag("SsrPassDescriptorSet"));
    {
        LockGuard const guard(trace.lock);
        EXPECT_EQ(0u, trace.staticShadowUboCreations);
        EXPECT_EQ(0u, trace.perViewUboCreations);
        EXPECT_EQ(0u, trace.starburstTextureCreations);
    }

    SwapChain* swapChain = engine->createSwapChain(64, 64);
    Renderer* renderer = engine->createRenderer();
    Scene* scene = engine->createScene();
    View* view = engine->createView();
    Entity const camEntity = engine->getEntityManager().create();
    Camera* camera = engine->createCamera(camEntity);
    camera->setProjection(45.0, 1.0, 0.1, 100.0);
    view->setCamera(camera);
    view->setViewport({ 0, 0, 64, 64 });
    view->setScene(scene);

    // Rendering a standard frame (where the structure pass is culled by FrameGraph) keeps
    // StructureDescriptorSet's GPU UBO/DescriptorSet, MotionVectorsDescriptorSet, SSR, and
    // starburst unallocated. Only the View's own PerViewUib UBO exists (1 total).
    if (renderer->beginFrame(swapChain)) {
        renderer->render(view);
        renderer->endFrame();
    }
    engine->flushAndWait();
    EXPECT_FALSE(hasLiveDescriptorSetTag("StructureDescriptorSet"));
    EXPECT_FALSE(hasLiveDescriptorSetTag("MotionVectorsDescriptorSet"));
    EXPECT_FALSE(hasLiveDescriptorSetTag("SsrPassDescriptorSet"));
    {
        LockGuard const guard(trace.lock);
        EXPECT_EQ(1u, trace.perViewUboCreations);
        EXPECT_EQ(0u, trace.starburstTextureCreations);
    }

    // Enabling SSAO un-culls the structure pass and lazily initializes StructureDescriptorSet.
    AmbientOcclusionOptions aoOptions{};
    aoOptions.enabled = true;
    view->setAmbientOcclusionOptions(aoOptions);
    if (renderer->beginFrame(swapChain)) {
        renderer->render(view);
        renderer->endFrame();
    }
    engine->flushAndWait();
    EXPECT_TRUE(hasLiveDescriptorSetTag("StructureDescriptorSet"));
    {
        LockGuard const guard(trace.lock);
        EXPECT_EQ(2u, trace.perViewUboCreations);
        EXPECT_EQ(0u, trace.starburstTextureCreations);
    }

    // Enabling bloom with lensFlare + starburst lazily creates the 256x1 R8 starburst texture once.
    BloomOptions bloomOptions{};
    bloomOptions.enabled = true;
    bloomOptions.lensFlare = true;
    bloomOptions.starburst = true;
    view->setBloomOptions(bloomOptions);
    if (renderer->beginFrame(swapChain)) {
        renderer->render(view);
        renderer->endFrame();
    }
    engine->flushAndWait();
    {
        LockGuard const guard(trace.lock);
        EXPECT_EQ(1u, trace.starburstTextureCreations);
    }

    // Directly exercising SsrPassDescriptorSet::init/setFrameUniforms/commit/terminate must
    // create 0 BufferObjects (reusing FEngine::mDummyUniformBuffer instead of a dedicated
    // ShadowUib UBO) and 1 SsrPassDescriptorSet, then release it on terminate().
    {
        size_t uboBefore = 0;
        {
            LockGuard const guard(trace.lock);
            uboBefore = trace.totalUboCreations;
        }
        SsrPassDescriptorSet ssrDs;
        ssrDs.init(fengine);
        ssrDs.setFrameUniforms(fengine, downcast(view)->getFrameUniforms());
        ssrDs.commit(fengine);
        engine->flushAndWait();
        EXPECT_TRUE(hasLiveDescriptorSetTag("SsrPassDescriptorSet"));
        ssrDs.terminate(fengine.getDriverApi());
        engine->flushAndWait();
        EXPECT_FALSE(hasLiveDescriptorSetTag("SsrPassDescriptorSet"));
        {
            LockGuard const guard(trace.lock);
            EXPECT_EQ(uboBefore, trace.totalUboCreations);
        }
    }

    // Enabling SSR across two frames (frame 1 establishes history, frame 2 runs SSR Pass) lazily
    // initializes PostProcessManager's SsrPassDescriptorSet without allocating any ShadowUib UBO.
    ScreenSpaceReflectionsOptions ssrOptions{};
    ssrOptions.enabled = true;
    view->setScreenSpaceReflectionsOptions(ssrOptions);
    for (int i = 0; i < 2; ++i) {
        if (renderer->beginFrame(swapChain)) {
            renderer->render(view);
            renderer->endFrame();
        }
    }
    engine->flushAndWait();
    EXPECT_TRUE(hasLiveDescriptorSetTag("SsrPassDescriptorSet"));
    {
        LockGuard const guard(trace.lock);
        EXPECT_EQ(0u, trace.staticShadowUboCreations);
    }

    engine->destroyCameraComponent(camEntity);
    engine->getEntityManager().destroy(camEntity);
    engine->destroy(view);
    engine->destroy(scene);
    engine->destroy(renderer);
    engine->destroy(swapChain);
    Engine::destroy(&engine);

    // Every BufferObject, Texture, and DescriptorSet created during the Engine's lifetime must be
    // destroyed cleanly by shutdown().
    {
        LockGuard const guard(trace.lock);
        EXPECT_TRUE(trace.liveBuffers.empty());
        EXPECT_TRUE(trace.liveTextures.empty());
        EXPECT_TRUE(trace.liveDescriptorSets.empty());
    }
}


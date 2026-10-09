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

#include "details/RenderTarget.h"

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/RenderTarget.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/SwapChain.h>
#include <filament/Texture.h>
#include <filament/View.h>
#include <filament/Viewport.h>

#include <backend/DriverEnums.h>
#include <backend/Handle.h>
#include <backend/Platform.h>

#include <utils/CString.h>
#include <utils/Entity.h>
#include <utils/EntityManager.h>
#include <utils/Mutex.h>

#include <math/vec4.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <functional>
#include <initializer_list>
#include <iterator>
#include <vector>

// The render passes Filament asks the backend for when several Views render into the swap chain
// in the same frame, as in a split screen.

using namespace filament;
using namespace filament::backend;
using ::testing::_;
using ::testing::Return;

namespace {

// A render pass, and the render target it renders into.
struct RenderPass {
    HandleBase::HandleId target;
    RenderPassParams params;
};

// A driver that records every render pass, and which render target is the swap chain's.
class RecordingDriver : public MockDriver {
public:
    RecordingDriver(std::vector<RenderPass>* passes, HandleBase::HandleId* swapChainTarget)
            : mPasses(passes), mSwapChainTarget(swapChainTarget) {
        // Enough of a driver for the Engine to build and render.
        ON_CALL(*this, getFeatureLevel()).WillByDefault(Return(FeatureLevel::FEATURE_LEVEL_1));
        ON_CALL(*this, isTextureFormatSupported(_)).WillByDefault(Return(true));
        ON_CALL(*this, getMaxTextureSize(_)).WillByDefault(Return(2048));
        ON_CALL(*this, getMaxUniformBufferSize()).WillByDefault(Return(16384));
        ON_CALL(*this, getMaxDrawBuffers()).WillByDefault(Return(4));
    }

    // These hide MockDriver's, ConcreteDispatcher<RecordingDriver> calls them.
    void createDefaultRenderTargetR(RenderTargetHandle rth, utils::ImmutableCString&&) {
        utils::LockGuard const lock(mLock);
        *mSwapChainTarget = rth.getId();
    }

    void beginRenderPass(RenderTargetHandle rth, RenderPassParams const& params) {
        utils::LockGuard const lock(mLock);
        mPasses->push_back({ rth.getId(), params });
    }

    Dispatcher getDispatcher() const noexcept override {
        return ConcreteDispatcher<RecordingDriver>::make();
    }

private:
    utils::Mutex mLock;
    std::vector<RenderPass>* mPasses;
    HandleBase::HandleId* mSwapChainTarget;
};

class RecordingPlatform : public Platform {
public:
    RecordingPlatform(std::vector<RenderPass>* passes, HandleBase::HandleId* swapChainTarget)
            : mPasses(passes), mSwapChainTarget(swapChainTarget) {}
    int getOSVersion() const noexcept override { return 0; }
    utils::CString getDeviceInfo(DeviceInfoType, Driver*) const override { return {}; }
    Driver* createDriver(void*, Platform::DriverConfig const&) override {
        return new ::testing::NiceMock<RecordingDriver>(mPasses, mSwapChainTarget);
    }

private:
    std::vector<RenderPass>* mPasses;
    HandleBase::HandleId* mSwapChainTarget;
};

constexpr math::double4 kClearColor = { 0.25, 0.5, 0.75, 1.0 };

// the quarters of the 64x64 swap chain
filament::Viewport const kQuarters[] = {
    { 0, 0, 32, 32 }, { 32, 0, 32, 32 }, { 0, 32, 32, 32 }, { 32, 32, 32, 32 },
};

class MultipleViewsTest : public ::testing::Test {
protected:
    void SetUp() override {
        mEngine = Engine::Builder().backend(Backend::NOOP).platform(&mPlatform).build();
        ASSERT_NE(nullptr, mEngine);
        // readable, so that it can be the source of copyFrame()
        mSwapChain = mEngine->createSwapChain(64, 64, SwapChain::CONFIG_READABLE);
        mRenderer = mEngine->createRenderer();
        mRenderer->setClearOptions({ .clearColor = kClearColor, .clear = true });
        mScene = mEngine->createScene();
        mCameraEntity = utils::EntityManager::get().create();
        mCamera = mEngine->createCamera(mCameraEntity);
    }

    void TearDown() override {
        destroyViews();
        mEngine->destroyCameraComponent(mCameraEntity);
        utils::EntityManager::get().destroy(mCameraEntity);
        mEngine->destroy(mScene);
        mEngine->destroy(mRenderer);
        mEngine->destroy(mSwapChain);
        Engine::destroy(&mEngine);
    }

    void addViews(std::initializer_list<filament::Viewport> viewports,
            bool postProcessing = false, RenderTarget* renderTarget = nullptr) {
        for (auto const& viewport : viewports) {
            View* view = mEngine->createView();
            view->setScene(mScene);
            view->setCamera(mCamera);
            view->setViewport(viewport);
            view->setPostProcessingEnabled(postProcessing);
            view->setShadowingEnabled(false);
            view->setRenderTarget(renderTarget);
            mViews.push_back(view);
        }
    }

    void destroyViews() {
        for (View* view : mViews) {
            mEngine->destroy(view);
        }
        mViews.clear();
    }

    // Renders all Views, calling beforeView(i) before rendering the i-th. Returns the render
    // passes of the frame. A few frames are rendered, so that later ones see the state that the
    // earlier ones left behind.
    std::vector<RenderPass> renderFrame(std::function<void(size_t)> const& beforeView = {}) {
        std::vector<RenderPass> frame;
        for (int i = 0; i < 3; i++) {
            mPasses.clear();
            if (mRenderer->beginFrame(mSwapChain)) {
                for (size_t v = 0; v < mViews.size(); v++) {
                    if (beforeView) {
                        beforeView(v);
                    }
                    mRenderer->render(mViews[v]);
                }
                mRenderer->endFrame();
            }
            mEngine->flushAndWait();
            if (!mPasses.empty()) {
                frame = mPasses;
            }
        }
        return frame;
    }

    // The render passes of the frame into the given render target, or into the swap chain.
    std::vector<RenderPassParams> passesInto(std::vector<RenderPass> const& frame,
            RenderTarget const* renderTarget = nullptr) const {
        HandleBase::HandleId const target = renderTarget ?
                downcast(renderTarget)->getHwHandle().getId() : mSwapChainTarget;
        std::vector<RenderPassParams> passes;
        for (auto const& pass : frame) {
            if (pass.target == target) {
                passes.push_back(pass.params);
            }
        }
        return passes;
    }

    std::vector<RenderPass> mPasses;
    HandleBase::HandleId mSwapChainTarget = HandleBase::nullid;
    RecordingPlatform mPlatform{ &mPasses, &mSwapChainTarget };
    Engine* mEngine = nullptr;
    SwapChain* mSwapChain = nullptr;
    Renderer* mRenderer = nullptr;
    Scene* mScene = nullptr;
    utils::Entity mCameraEntity;
    Camera* mCamera = nullptr;
    std::vector<View*> mViews;
};

bool clearsColor(RenderPassParams const& pass) {
    return any(pass.flags.clear & TargetBufferFlags::COLOR);
}

bool discardsColor(RenderPassParams const& pass) {
    return any(pass.flags.discardStart & TargetBufferFlags::COLOR);
}

} // namespace

TEST_F(MultipleViewsTest, DisjointViewsKnowTheirViewportIsCleared) {
    addViews({ kQuarters[0], kQuarters[1], kQuarters[2], kQuarters[3] });
    // overlaps all of the above, so it must load what they drew
    addViews({ { 16, 16, 32, 32 } });

    // changing the clear color after the 1st View doesn't change what the swap chain holds
    auto const passes = passesInto(renderFrame([this](size_t view) {
        mRenderer->setClearOptions({
                .clearColor = view == 0 ? kClearColor : math::double4{ 1, 0, 0, 1 },
                .clear = true });
    }));
    ASSERT_EQ(passes.size(), 5);

    // the 1st View clears the whole swap chain
    EXPECT_TRUE(clearsColor(passes[0]));
    EXPECT_TRUE(discardsColor(passes[0]));
    EXPECT_EQ(passes[0].flags.viewportCleared, TargetBufferFlags::NONE);

    EXPECT_FALSE(clearsColor(passes[4]));
    EXPECT_FALSE(discardsColor(passes[4]));
    EXPECT_EQ(passes[4].flags.viewportCleared, TargetBufferFlags::NONE);

    // the next ones know that their viewport holds the clear color, and say which
    for (size_t i = 1; i < 4; i++) {
        SCOPED_TRACE(i);
        EXPECT_FALSE(clearsColor(passes[i]));
        EXPECT_FALSE(discardsColor(passes[i]));
        EXPECT_EQ(passes[i].flags.viewportCleared, TargetBufferFlags::COLOR);
        EXPECT_EQ(passes[i].clearColor, passes[0].clearColor);
        EXPECT_EQ(passes[i].viewport, kQuarters[i]);
    }
}

// A View over a part of the 1st one only, like a picture-in-picture or a HUD over a full screen
// View, must load what the 1st View drew.
TEST_F(MultipleViewsTest, ViewsOverTheFirstOneLoad) {
    addViews({ { 0, 0, 64, 64 }, { 40, 40, 16, 16 }, { 8, 8, 16, 16 } });

    auto const passes = passesInto(renderFrame());
    ASSERT_EQ(passes.size(), 3);
    EXPECT_TRUE(clearsColor(passes[0]));
    for (size_t i = 1; i < 3; i++) {
        SCOPED_TRACE(i);
        EXPECT_FALSE(clearsColor(passes[i]));
        EXPECT_FALSE(discardsColor(passes[i]));
        EXPECT_EQ(passes[i].flags.viewportCleared, TargetBufferFlags::NONE);
    }
}

// A View that only touches an earlier View's viewport doesn't overlap it; one that shares a
// single row or column with it does.
TEST_F(MultipleViewsTest, OverlapIsToThePixel) {
    filament::Viewport const first = { 16, 16, 32, 32 };    // columns and rows 16 to 47
    struct Case {
        filament::Viewport viewport;
        bool cleared;
    };
    Case const cases[] = {
        { { 48, 16, 16, 32 }, true },   // touches its right side
        { { 0, 16, 16, 32 }, true },    // its left side
        { { 16, 48, 32, 16 }, true },   // its top
        { { 16, 0, 32, 16 }, true },    // its bottom
        { { 47, 16, 16, 32 }, false },  // shares its rightmost column
        { { 0, 16, 17, 32 }, false },   // its leftmost column
        { { 16, 47, 32, 16 }, false },  // its top row
        { { 16, 0, 32, 17 }, false },   // its bottom row
    };
    for (size_t i = 0; i < std::size(cases); i++) {
        SCOPED_TRACE(i);
        destroyViews();
        addViews({ first, cases[i].viewport });
        auto const passes = passesInto(renderFrame());
        ASSERT_EQ(passes.size(), 2);
        EXPECT_EQ(passes[1].flags.viewportCleared,
                cases[i].cleared ? TargetBufferFlags::COLOR : TargetBufferFlags::NONE);
    }
}

TEST_F(MultipleViewsTest, NothingIsClearedWithoutClearOptions) {
    mRenderer->setClearOptions({ .clearColor = kClearColor, .clear = false, .discard = true });
    addViews({ kQuarters[0], kQuarters[1], kQuarters[2], kQuarters[3] });

    auto const passes = passesInto(renderFrame());
    ASSERT_EQ(passes.size(), 4);
    EXPECT_TRUE(discardsColor(passes[0]));
    for (auto const& pass : passes) {
        EXPECT_FALSE(clearsColor(pass));
        EXPECT_EQ(pass.flags.viewportCleared, TargetBufferFlags::NONE);
    }
}

TEST_F(MultipleViewsTest, PostProcessedViewsKnowTheirViewportIsCleared) {
    addViews({ kQuarters[0], kQuarters[1], kQuarters[2], kQuarters[3] }, true);
    // overlaps all of the above, so it must load what they drew
    addViews({ { 16, 16, 32, 32 } }, true);

    // only the passes that write into the swap chain (the last of each View) are concerned
    std::vector<backend::Viewport> cleared;
    for (auto const& pass : renderFrame()) {
        if (any(pass.params.flags.viewportCleared)) {
            EXPECT_EQ(pass.target, mSwapChainTarget);
            EXPECT_EQ(pass.params.flags.viewportCleared, TargetBufferFlags::COLOR);
            EXPECT_FALSE(clearsColor(pass.params));
            EXPECT_FALSE(discardsColor(pass.params));
            cleared.push_back(pass.params.viewport);
        }
    }
    EXPECT_EQ(cleared, std::vector<backend::Viewport>(kQuarters + 1, kQuarters + 4));
}

TEST_F(MultipleViewsTest, OnlyTheSwapChainsViewportsAreKnownToBeCleared) {
    Texture* color = Texture::Builder().width(64).height(64)
            .format(Texture::InternalFormat::RGBA8)
            .usage(Texture::Usage::COLOR_ATTACHMENT).build(*mEngine);
    RenderTarget* renderTarget = RenderTarget::Builder()
            .texture(RenderTarget::AttachmentPoint::COLOR, color).build(*mEngine);

    addViews({ kQuarters[0] });
    // in a render target of their own, so they don't touch the swap chain's quarters
    addViews({ kQuarters[3], kQuarters[2] }, false, renderTarget);
    addViews({ kQuarters[3] });

    auto const frame = renderFrame();
    auto const intoSwapChain = passesInto(frame);
    auto const intoRenderTarget = passesInto(frame, renderTarget);
    ASSERT_EQ(intoSwapChain.size(), 2);
    ASSERT_EQ(intoRenderTarget.size(), 2);

    // other things than the Views can write into a render target's textures
    EXPECT_EQ(intoRenderTarget[1].flags.viewportCleared, TargetBufferFlags::NONE);
    EXPECT_EQ(intoSwapChain[1].flags.viewportCleared, TargetBufferFlags::COLOR);

    destroyViews();
    mEngine->destroy(renderTarget);
    mEngine->destroy(color);
}

// copyFrame() into the swap chain being rendered draws over it, so the Views after it must load.
TEST_F(MultipleViewsTest, CopyFrameIntoTheSwapChainMakesLaterViewsLoad) {
    SwapChain* other = mEngine->createSwapChain(64, 64);
    addViews({ kQuarters[0], kQuarters[1], kQuarters[2] });

    for (SwapChain* dst : { other, mSwapChain }) {
        SCOPED_TRACE(dst == mSwapChain ? "into the swap chain" : "into another swap chain");
        auto const passes = passesInto(renderFrame([&](size_t view) {
            if (view == 2) {
                mRenderer->copyFrame(dst, kQuarters[2], kQuarters[0], Renderer::CLEAR);
            }
        }));
        ASSERT_GE(passes.size(), 3);
        EXPECT_EQ(passes[1].viewport, kQuarters[1]);
        EXPECT_EQ(passes[1].flags.viewportCleared, TargetBufferFlags::COLOR);
        EXPECT_EQ(passes.back().viewport, kQuarters[2]);
        EXPECT_FALSE(clearsColor(passes.back()));
        EXPECT_EQ(passes.back().flags.viewportCleared,
                dst == other ? TargetBufferFlags::COLOR : TargetBufferFlags::NONE);
    }

    mEngine->destroy(other);
}

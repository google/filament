/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include <gtest/gtest.h>

#include <filament/Engine.h>
#include <filament/Renderer.h>
#include <filament/Skybox.h>
#include <filament/Scene.h>
#include <filament/View.h>
#include <filament/Viewport.h>

#include <backend/PixelBufferDescriptor.h>

using namespace filament;
using namespace backend;

class RenderingTest : public testing::Test {
protected:
    Engine* mEngine = nullptr;
    SwapChain* mSurface = nullptr;
    Renderer* mRenderer = nullptr;
    View* mView = nullptr;
    Skybox* mSkybox = nullptr;
    Scene* mScene = nullptr;
    Camera* mCamera = nullptr;

    using closure_t = std::function<void(uint8_t const* rgba, uint32_t width, uint32_t height)>;

    void SetUp() override {
        mEngine = Engine::create();
        mSurface = mEngine->createSwapChain(16, 16);
        mRenderer = mEngine->createRenderer();

        mScene = mEngine->createScene();
        mCamera = mEngine->createCamera();

        mView = mEngine->createView();
        mView->setViewport({0, 0, 16, 16});
        mView->setScene(mScene);
        mView->setCamera(mCamera);

        mSkybox = Skybox::Builder().build(*mEngine);
        mScene->setSkybox(mSkybox);
    }

    void TearDown() override {
        mEngine->destroy(mCamera);
        mEngine->destroy(mScene);
        mEngine->destroy(mView);
        mEngine->destroy(mSkybox);
        mEngine->destroy(mRenderer);
        mEngine->destroy(mSurface);
        Engine::destroy(&mEngine);
    }

    void runTest(closure_t closure) {
        auto* user = new closure_t(std::move(closure));

        size_t size = 16 * 16 * 4;
        void* buffer = malloc(size);
        memset(buffer, 0, size);
        PixelBufferDescriptor pd(buffer, size,
                PixelDataFormat::RGBA, PixelDataType::UBYTE,
                callback, user);

        Renderer* pRenderer = mRenderer;
        pRenderer->beginFrame(mSurface);
        pRenderer->render(mView);
        pRenderer->readPixels(0, 0, 16, 16, std::move(pd));
        pRenderer->endFrame();

        // Note: this is where the runTest() callback will be called.
        mEngine->flushAndWait();
    }

private:
    static void callback(void* buffer, size_t size, void* user) {
        closure_t* closure = (closure_t *)user;
        uint8_t const* rgba = (uint8_t const*)buffer;
        (*closure)(rgba, 16, 16);
        delete closure;
        ::free(buffer);
    }
};

TEST_F(RenderingTest, ClearRed) {
    mSkybox->setColor(LinearColorA{1, 0, 0, 1});
    mView->setToneMapping(View::ToneMapping::LINEAR);
    mView->setDithering(View::Dithering::NONE);
    runTest([this](uint8_t const* rgba, uint32_t width, uint32_t height) {
        EXPECT_EQ(rgba[0], 0xff);
        EXPECT_EQ(rgba[1], 0);
        EXPECT_EQ(rgba[2], 0);
        EXPECT_EQ(rgba[3], 0xff);
    });
}

TEST_F(RenderingTest, ClearGreen) {
    mSkybox->setColor(LinearColorA{0, 1, 0, 1});
    mView->setToneMapping(View::ToneMapping::LINEAR);
    mView->setDithering(View::Dithering::NONE);
    runTest([this](uint8_t const* rgba, uint32_t width, uint32_t height) {
        EXPECT_EQ(rgba[0], 0);
        EXPECT_EQ(rgba[1], 0xff);
        EXPECT_EQ(rgba[2], 0);
        EXPECT_EQ(rgba[3], 0xff);
    });
}

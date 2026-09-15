/*
 * Copyright 2026 The Android Open Source Project
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

#include <viewer/ViewerGui.h>

#include <filament/DebugRegistry.h>
#include <filament/Engine.h>
#include <filament/Scene.h>
#include <filament/View.h>

#include <utils/EntityManager.h>

#include <gtest/gtest.h>

using namespace filament;
using namespace filament::viewer;

static constexpr const char* COMBINE_MULTIVIEW_IMAGES = "d.stereo.combine_multiview_images";

class ViewerGuiTest : public testing::Test {
protected:
    void SetUp() override {
        mEngine = Engine::Builder().backend(backend::Backend::NOOP).build();
        mScene = mEngine->createScene();
        mView = mEngine->createView();
        mView->setScene(mScene);
    }

    void TearDown() override {
        Engine::destroy(&mEngine);
    }

    Engine* mEngine = nullptr;
    Scene* mScene = nullptr;
    View* mView = nullptr;
};

// The engine registers this property from the first Renderer it creates, so a viewer built
// before then has no address to write to.
TEST_F(ViewerGuiTest, ConstructsWithoutARenderer) {
    ASSERT_EQ(mEngine->getDebugRegistry().getPropertyAddress<bool>(COMBINE_MULTIVIEW_IMAGES),
            nullptr);
    ViewerGui viewer(mEngine, mScene, mView);
}

TEST_F(ViewerGuiTest, EnablesCombineMultiviewImagesOnceRegistered) {
    mEngine->createRenderer();
    ViewerGui viewer(mEngine, mScene, mView);
    bool* const combineMultiviewImages =
            mEngine->getDebugRegistry().getPropertyAddress<bool>(COMBINE_MULTIVIEW_IMAGES);
    ASSERT_NE(combineMultiviewImages, nullptr);
    EXPECT_TRUE(*combineMultiviewImages);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

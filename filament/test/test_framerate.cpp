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

#include <filament/Engine.h>
#include <filament/SwapChain.h>

#include <gtest/gtest.h>

#include <cerrno>

using namespace filament;

TEST(FrameRateTest, HeadlessSwapChain) {
    Engine* engine = Engine::create(Engine::Backend::VULKAN);
    ASSERT_NE(engine, nullptr);

    SwapChain* swapChain = engine->createSwapChain(512, 512, 0);
    ASSERT_NE(swapChain, nullptr);

    // Ensure async creation commands are fully flushed to the backend thread
    engine->flushAndWait();

    EXPECT_TRUE(swapChain->isFrameRateChangeSupported().is_false());

    // Calling setFrameRate on an unsupported swapchain should execute asynchronously without crashing
    swapChain->setFrameRate(60.0f);
    engine->flushAndWait();

    engine->destroy(swapChain);
    Engine::destroy(&engine);
}

TEST(FrameRateTest, HeadlessSwapChainOpenGL) {
    Engine* engine = Engine::create(Engine::Backend::OPENGL);
    ASSERT_NE(engine, nullptr);

    SwapChain* swapChain = engine->createSwapChain(512, 512, 0);
    ASSERT_NE(swapChain, nullptr);

    // Ensure async creation commands are fully flushed to the backend thread
    engine->flushAndWait();

    EXPECT_TRUE(swapChain->isFrameRateChangeSupported().is_false());

    // Calling setFrameRate on an unsupported swapchain should execute asynchronously without crashing
    swapChain->setFrameRate(60.0f);
    engine->flushAndWait();

    engine->destroy(swapChain);
    Engine::destroy(&engine);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

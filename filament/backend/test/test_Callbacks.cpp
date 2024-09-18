/*
 * Copyright (C) 2024 The Android Open Source Project
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

using namespace filament;
using namespace filament::backend;

namespace test {

TEST_F(BackendTest, FrameScheduledCallback) {
    auto& api = getDriverApi();

    // Create a SwapChain.
    // In order for the frameScheduledCallback to be called, this must be a real SwapChain (not
    // headless) so we obtain a drawable.
    auto swapChain = createSwapChain();

    Handle<HwRenderTarget> renderTarget = api.createDefaultRenderTarget();

    int callbackCountA = 0;
    api.setFrameScheduledCallback(swapChain, [&callbackCountA](PresentCallable callable) {
        callable();
        callbackCountA++;
    });

    // Render the first frame.
    api.makeCurrent(swapChain, swapChain);
    api.beginFrame(0, 0, 0);
    api.beginRenderPass(renderTarget, {});
    api.endRenderPass(0);
    api.commit(swapChain);
    api.endFrame(0);

    // Render the next frame. The same callback should be called.
    api.makeCurrent(swapChain, swapChain);
    api.beginFrame(0, 0, 0);
    api.beginRenderPass(renderTarget, {});
    api.endRenderPass(0);
    api.commit(swapChain);
    api.endFrame(0);

    // Now switch out the callback.
    int callbackCountB = 0;
    api.setFrameScheduledCallback(swapChain, [&callbackCountB](PresentCallable callable) {
        callable();
        callbackCountB++;
    });

    // Render one final frame.
    api.makeCurrent(swapChain, swapChain);
    api.beginFrame(0, 0, 0);
    api.beginRenderPass(renderTarget, {});
    api.endRenderPass(0);
    api.commit(swapChain);
    api.endFrame(0);

    api.finish();

    executeCommands();
    getDriver().purge();

    EXPECT_EQ(callbackCountA, 2);
    EXPECT_EQ(callbackCountB, 1);
}

TEST_F(BackendTest, FrameCompletedCallback) {
    auto& api = getDriverApi();

    // Create a SwapChain.
    auto swapChain = api.createSwapChainHeadless(256, 256, 0);

    int callbackCountA = 0;
    api.setFrameCompletedCallback(swapChain, nullptr,
            [&callbackCountA]() { callbackCountA++; });

    // Render the first frame.
    api.makeCurrent(swapChain, swapChain);
    api.beginFrame(0, 0, 0);
    api.commit(swapChain);
    api.endFrame(0);

    // Render the next frame. The same callback should be called.
    api.makeCurrent(swapChain, swapChain);
    api.beginFrame(0, 0, 0);
    api.commit(swapChain);
    api.endFrame(0);

    // Now switch out the callback.
    int callbackCountB = 0;
    api.setFrameCompletedCallback(swapChain, nullptr,
            [&callbackCountB]() { callbackCountB++; });

    // Render one final frame.
    api.makeCurrent(swapChain, swapChain);
    api.beginFrame(0, 0, 0);
    api.commit(swapChain);
    api.endFrame(0);

    api.finish();

    executeCommands();
    getDriver().purge();

    EXPECT_EQ(callbackCountA, 2);
    EXPECT_EQ(callbackCountB, 1);
}

} // namespace test

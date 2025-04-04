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

#include "Lifetimes.h"
#include "Skip.h"

using namespace filament;
using namespace filament::backend;

namespace test {

TEST_F(BackendTest, FrameScheduledCallback) {
    SKIP_IF(Backend::OPENGL, "Frame callbacks are unsupported in OpenGL");
    SKIP_IF(Backend::VULKAN, "Frame callbacks are unsupported in Vulkan, see b/417254479");
    SKIP_IF(Backend::WEBGPU, "Frame callbacks are unsupported in WebGPU");

    int callbackCountA = 0;
    int callbackCountB = 0;
    {
        auto& api = getDriverApi();
        Cleanup cleanup(api);
        cleanup.addPostCall([&]() { executeCommands(); });
        cleanup.addPostCall([&]() { getDriver().purge(); });

        // Create a SwapChain.
        // In order for the frameScheduledCallback to be called, this must be a real SwapChain (not
        // headless) so we obtain a drawable.
        auto swapChain = cleanup.add(createSwapChain());

        Handle<HwRenderTarget> renderTarget = cleanup.add(api.createDefaultRenderTarget());

        api.setFrameScheduledCallback(swapChain, nullptr, [&callbackCountA](PresentCallable callable) {
            callable();
            callbackCountA++;
        }, 0);

        // Render the first frame.
        api.makeCurrent(swapChain, swapChain);
        {
            RenderFrame frame(api);
            api.beginRenderPass(renderTarget, {});
            api.endRenderPass(0);
            api.commit(swapChain);
        }

        // Render the next frame. The same callback should be called.
        api.makeCurrent(swapChain, swapChain);
        {
            RenderFrame frame(api);
            api.beginRenderPass(renderTarget, {});
            api.endRenderPass(0);
            api.commit(swapChain);
        }

        // Now switch out the callback.
        api.setFrameScheduledCallback(swapChain, nullptr, [&callbackCountB](PresentCallable callable) {
            callable();
            callbackCountB++;
        }, 0);

        // Render one final frame.
        api.makeCurrent(swapChain, swapChain);
        {
            RenderFrame frame(api);
            api.beginRenderPass(renderTarget, {});
            api.endRenderPass(0);
            api.commit(swapChain);
        }

        api.finish();
    }
    EXPECT_EQ(callbackCountA, 2);
    EXPECT_EQ(callbackCountB, 1);
}

TEST_F(BackendTest, FrameCompletedCallback) {
    SKIP_IF(Backend::OPENGL, "Frame callbacks are unsupported in OpenGL");
    SKIP_IF(Backend::VULKAN, "Frame callbacks are unsupported in Vulkan, see b/417254479");
    SKIP_IF(Backend::WEBGPU, "Frame callbacks are unsupported in WebGPU");

    int callbackCountA = 0;
    int callbackCountB = 0;
    {
        auto& api = getDriverApi();
        Cleanup cleanup(api);
        cleanup.addPostCall([&]() { executeCommands(); });
        cleanup.addPostCall([&]() { getDriver().purge(); });

        // Create a SwapChain.
        auto swapChain = cleanup.add(api.createSwapChainHeadless(256, 256, 0));

        api.setFrameCompletedCallback(swapChain, nullptr,
                [&callbackCountA]() { callbackCountA++; });

        // Render the first frame.
        api.makeCurrent(swapChain, swapChain);
        {
            RenderFrame frame(api);
            api.commit(swapChain);
        }

        // Render the next frame. The same callback should be called.
        api.makeCurrent(swapChain, swapChain);
        {
            RenderFrame frame(api);
            api.commit(swapChain);
        }

        // Now switch out the callback.
        api.setFrameCompletedCallback(swapChain, nullptr,
                [&callbackCountB]() { callbackCountB++; });

        // Render one final frame.
        api.makeCurrent(swapChain, swapChain);
        {
            RenderFrame frame(api);
            api.commit(swapChain);
        }

        api.finish();
    }

    EXPECT_EQ(callbackCountA, 2);
    EXPECT_EQ(callbackCountB, 1);
}

} // namespace test

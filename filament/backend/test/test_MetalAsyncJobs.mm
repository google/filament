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
#include "Skip.h"

#include <Foundation/Foundation.h>

namespace test {

using namespace filament;
using namespace filament::backend;

TEST_F(BackendTest, MetalAsyncJobDrainsAutoreleasePool) {
    SKIP_IF_NOT(Backend::METAL, "the autorelease pool is specific to the Metal backend");

    auto& api = getDriverApi();
    auto swapChain = addCleanup(createSwapChain());
    api.makeCurrent(swapChain, swapChain);

    // BackendTest asks for THREAD_PREFERRED, so the job runs on the ThreadWorker thread, which has
    // no autorelease pool unless the driver gives it one.
    __weak NSObject* weakObject = nil;
    api.queueCommandAsync(
            [&weakObject]() {
                NSObject* object = [NSObject new];
                weakObject = object;
                // The pool ends up with the only reference, like objects Metal returns
                // autoreleased.
                CFAutorelease((__bridge_retained CFTypeRef) object);
            },
            nullptr, nullptr, nullptr);

    // finish() waits for the worker to reach a later job, so the pool of this one is drained.
    api.finish();
    executeCommands();

    EXPECT_EQ(nil, weakObject) << "an object autoreleased by an async job outlived the job";
}

} // namespace test

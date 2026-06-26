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

namespace test {

using namespace filament;
using namespace filament::backend;

TEST_F(BackendTest, FrameRateControl) {
    auto& api = getDriverApi();
    auto swapChain = addCleanup(createSwapChain());

    api.makeCurrent(swapChain, swapChain);


    // Calling setFrameRate should be a safe, fully verified operation across all backends
    api.setFrameRate(swapChain, 60.0f,
            Platform::FrameRateCompatibility::DEFAULT,
            Platform::ChangeFrameRateStrategy::ONLY_IF_SEAMLESS);
    api.setFrameRate(swapChain, 120.0f,
            Platform::FrameRateCompatibility::FIXED_SOURCE,
            Platform::ChangeFrameRateStrategy::ALWAYS);
    api.setFrameRate(swapChain, 0.0f,
            Platform::FrameRateCompatibility::DEFAULT,
            Platform::ChangeFrameRateStrategy::ONLY_IF_SEAMLESS);
}

} // namespace test

/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef TNT_LIFETIMES_H
#define TNT_LIFETIMES_H

#include <vector>
#include <functional>
#include <optional>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "backend/Handle.h"
#include "backend/DriverApiForward.h"
#include "BackendTest.h"

class RenderFrame {
public:
    explicit RenderFrame(filament::backend::DriverApi& api);
    ~RenderFrame();

    RenderFrame(RenderFrame const& rhs) noexcept = delete;
    RenderFrame& operator=(RenderFrame const& rhs) noexcept = delete;
    RenderFrame(RenderFrame const&& rhs) noexcept = delete;
    RenderFrame& operator=(RenderFrame const&& rhs) noexcept = delete;

private:
    filament::backend::DriverApi& mApi;
};

class Cleanup {
public:
    /**
     * @param driverApi Must be valid for the entire lifetime of the Cleanup object.
     */
    explicit Cleanup(filament::backend::DriverApi& driverApi);
    ~Cleanup();

    template <typename HandleType>
    filament::backend::Handle<HandleType> add(filament::backend::Handle<HandleType> handle);

    void addPostCall(std::function<void()> callback);

private:
    template <typename HandleType>
    void addInternal(filament::backend::Handle<HandleType> handle);

    filament::backend::DriverApi & mDriverApi;

    std::vector<filament::backend::SwapChainHandle> mSwapChains;
    std::vector<filament::backend::DescriptorSetHandle> mDescriptorSets;
    std::vector<filament::backend::DescriptorSetLayoutHandle> mDescriptorSetLayouts;
    std::vector<filament::backend::BufferObjectHandle> mBufferObjects;
    std::vector<filament::backend::ProgramHandle> mPrograms;
    std::vector<filament::backend::TextureHandle> mTextures;
    std::vector<filament::backend::RenderTargetHandle> mRenderTargets;

    std::vector<std::function<void()>> mCallbacks;
};

template <typename HandleType>
filament::backend::Handle<HandleType> Cleanup::add(filament::backend::Handle<HandleType> handle) {
    EXPECT_THAT(handle, ::testing::IsTrue()) << "Added a null handle to clean up.";
    addInternal(handle);
    return handle;
}

#endif //TNT_LIFETIMES_H

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
    filament::backend::DriverApi& api_;
    filament::backend::SwapChainHandle swapChain_;
};

class Cleanup {
public:
    explicit Cleanup(const test::BackendTest& testFixture);
    ~Cleanup();

    // These methods can't be overloaded because of a compiler error caused by the type safe casting
    // constructor needing to be resolved for each option.
    void AddSwapChain(filament::backend::SwapChainHandle swapChain);
    void AddDescriptorSet(filament::backend::DescriptorSetHandle descriptorSet);
    void AddDescriptorSetLayout(filament::backend::DescriptorSetLayoutHandle descriptorSetLayout);
    void AddBufferObject(filament::backend::BufferObjectHandle bufferObject);
    void AddProgram(filament::backend::ProgramHandle program);
    void AddTexture(filament::backend::TextureHandle texture);
    void AddRenderTarget(filament::backend::RenderTargetHandle renderTarget);
    void AddPostCall(std::function<void()> callback);

private:
    const test::BackendTest& testFixture_;

    std::vector<filament::backend::SwapChainHandle> swapChains_;
    std::vector<filament::backend::DescriptorSetHandle> descriptorSets_;
    std::vector<filament::backend::DescriptorSetLayoutHandle> descriptorSetLayouts_;
    std::vector<filament::backend::BufferObjectHandle> bufferObjects_;
    std::vector<filament::backend::ProgramHandle> programs_;
    std::vector<filament::backend::TextureHandle> textures_;
    std::vector<filament::backend::RenderTargetHandle> renderTargets_;

    std::vector<std::function<void()>> callbacks_;
};

#endif //TNT_LIFETIMES_H

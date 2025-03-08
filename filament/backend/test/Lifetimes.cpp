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

#include "Lifetimes.h"

#include "backend/Platform.h"
#include "private/backend/DriverApi.h"

RenderFrame::RenderFrame(filament::backend::DriverApi& api): api_(api) {
    api_.beginFrame(0, 0, 0);
}

RenderFrame::~RenderFrame() {
    api_.endFrame(0);
}

Cleanup::Cleanup(const test::BackendTest& testFixture) : testFixture_(testFixture) {}

Cleanup::~Cleanup() {
    auto& api = testFixture_.getDriverApi();

    for (const auto swapChain: swapChains_) {
        api.destroySwapChain(swapChain);
    }
    for (const auto descriptorSet : descriptorSets_) {
        api.destroyDescriptorSet(descriptorSet);
    }
    for (const auto descriptorSetLayout : descriptorSetLayouts_) {
        api.destroyDescriptorSetLayout(descriptorSetLayout);
    }
    for (const auto bufferObject : bufferObjects_) {
        api.destroyBufferObject(bufferObject);
    }
    for (const auto program : programs_) {
        api.destroyProgram(program);
    }
    for (const auto texture : textures_) {
        api.destroyTexture(texture);
    }
    for (const auto renderTarget : renderTargets_) {
        api.destroyRenderTarget(renderTarget);
    }

    for (const auto& callback : callbacks_) {
        callback();
    }
}

void Cleanup::AddSwapChain(filament::backend::SwapChainHandle swapChain) {
    swapChains_.push_back(swapChain);
}
void Cleanup::AddDescriptorSet(filament::backend::DescriptorSetHandle descriptorSet) {
    descriptorSets_.push_back(descriptorSet);
}
void Cleanup::AddDescriptorSetLayout(
        filament::backend::DescriptorSetLayoutHandle descriptorSetLayout) {
    descriptorSetLayouts_.push_back(descriptorSetLayout);
}
void Cleanup::AddBufferObject(filament::backend::BufferObjectHandle bufferObject) {
    bufferObjects_.push_back(bufferObject);
}
void Cleanup::AddProgram(filament::backend::ProgramHandle program) {
    programs_.push_back(program);
}
void Cleanup::AddTexture(filament::backend::TextureHandle texture) {
    textures_.push_back(texture);
}
void Cleanup::AddRenderTarget(filament::backend::RenderTargetHandle renderTarget) {
    renderTargets_.push_back(renderTarget);
}
void Cleanup::AddPostCall(std::function<void()> callback){
    callbacks_.push_back(callback);
}

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

RenderFrame::RenderFrame(filament::backend::DriverApi& api): mApi(api) {
    mApi.beginFrame(0, 0, 0);
}

RenderFrame::~RenderFrame() {
    mApi.endFrame(0);
    mApi.finish();
}

Cleanup::Cleanup(filament::backend::DriverApi& driverApi) : mDriverApi(driverApi) {}

Cleanup::~Cleanup() {
    for (const auto swapChain: mSwapChains) {
        mDriverApi.destroySwapChain(swapChain);
    }
    for (const auto descriptorSet : mDescriptorSets) {
        mDriverApi.destroyDescriptorSet(descriptorSet);
    }
    for (const auto descriptorSetLayout : mDescriptorSetLayouts) {
        mDriverApi.destroyDescriptorSetLayout(descriptorSetLayout);
    }
    for (const auto bufferObject : mBufferObjects) {
        mDriverApi.destroyBufferObject(bufferObject);
    }
    for (const auto program : mPrograms) {
        mDriverApi.destroyProgram(program);
    }
    for (const auto texture : mTextures) {
        mDriverApi.destroyTexture(texture);
    }
    for (const auto renderTarget : mRenderTargets) {
        mDriverApi.destroyRenderTarget(renderTarget);
    }

    for (const auto& callback : mCallbacks) {
        callback();
    }
}

template <>
void Cleanup::addInternal(filament::backend::SwapChainHandle handle) {
    mSwapChains.push_back(handle);
}
template <>
void Cleanup::addInternal(filament::backend::DescriptorSetHandle handle) {
    mDescriptorSets.push_back(handle);
}
template <>
void Cleanup::addInternal(
        filament::backend::DescriptorSetLayoutHandle handle) {
    mDescriptorSetLayouts.push_back(handle);
}
template <>
void Cleanup::addInternal(filament::backend::BufferObjectHandle handle) {
    mBufferObjects.push_back(handle);
}
template <>
void Cleanup::addInternal(filament::backend::ProgramHandle handle) {
    mPrograms.push_back(handle);
}
template <>
void Cleanup::addInternal(filament::backend::TextureHandle handle) {
    mTextures.push_back(handle);
}
template <>
void Cleanup::addInternal(filament::backend::RenderTargetHandle handle) {
    mRenderTargets.push_back(handle);
}

void Cleanup::addPostCall(std::function<void()> callback){
    mCallbacks.emplace_back(std::move(callback));
}

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

#ifndef TNT_FILAMENT_BACKEND_WEBGPUDRIVER_H
#define TNT_FILAMENT_BACKEND_WEBGPUDRIVER_H

#include "WebGPUHandles.h"
#include "webgpu/WebGPUConstants.h"
#include <backend/platforms/WebGPUPlatform.h>

#include "DriverBase.h"
#include "private/backend/Dispatcher.h"
#include "private/backend/Driver.h"
#include "private/backend/HandleAllocator.h"
#include <backend/DriverEnums.h>

#include <utils/compiler.h>

#include <webgpu/webgpu_cpp.h>

#include <cstdint>
#include <memory>

#ifndef FILAMENT_WEBGPU_HANDLE_ARENA_SIZE_IN_MB
#define FILAMENT_WEBGPU_HANDLE_ARENA_SIZE_IN_MB 8
#endif

namespace filament::backend {

class WebGPUSwapChain;

/**
 * WebGPU backend (driver) implementation
 */
class WebGPUDriver final : public DriverBase {
public:
    ~WebGPUDriver() noexcept override;

    [[nodiscard]] Dispatcher getDispatcher() const noexcept final;
    [[nodiscard]] static Driver* create(WebGPUPlatform& platform, const Platform::DriverConfig& driverConfig) noexcept;

private:
    explicit WebGPUDriver(WebGPUPlatform& platform, const Platform::DriverConfig& driverConfig) noexcept;
    [[nodiscard]] ShaderModel getShaderModel() const noexcept final;
    [[nodiscard]] ShaderLanguage getShaderLanguage() const noexcept final;
    [[nodiscard]] wgpu::Sampler makeSampler(SamplerParams const& params);
    [[nodiscard]] static wgpu::AddressMode fWrapModeToWAddressMode(const filament::backend::SamplerWrapMode& fUsage);

    // the platform (e.g. OS) specific aspects of the WebGPU backend are strictly only
    // handled in the WebGPUPlatform
    WebGPUPlatform& mPlatform;
    wgpu::Adapter mAdapter = nullptr;
    wgpu::Device mDevice = nullptr;
    uint32_t mMinUniformBufferOffsetAlignment;
    wgpu::Queue mQueue = nullptr;
    void* mNativeWindow = nullptr;
    WebGPUSwapChain* mSwapChain = nullptr;
    uint64_t mNextFakeHandle = 1;
    wgpu::CommandEncoder mCommandEncoder = nullptr;
    wgpu::TextureView mTextureView = nullptr;
    wgpu::RenderPassEncoder mRenderPassEncoder = nullptr;
    wgpu::CommandBuffer mCommandBuffer = nullptr;
    WGPURenderTarget* mDefaultRenderTarget = nullptr;

    tsl::robin_map<uint32_t, wgpu::RenderPipeline> mPipelineMap;

    struct DescriptorSetBindingInfo{
        wgpu::BindGroup bindGroup;
        size_t offsetCount;
        backend::DescriptorSetOffsetArray offsets;
    };
    std::array<DescriptorSetBindingInfo,MAX_DESCRIPTOR_SET_COUNT> mCurrentDescriptorSets;
    /*
     * Driver interface
     */
    template<typename T>
    friend class ConcreteDispatcher;

#define DECL_DRIVER_API(methodName, paramsDecl, params)                                            \
    UTILS_ALWAYS_INLINE inline void methodName(paramsDecl);

#define DECL_DRIVER_API_SYNCHRONOUS(RetType, methodName, paramsDecl, params)                       \
    RetType methodName(paramsDecl) override;

#define DECL_DRIVER_API_RETURN(RetType, methodName, paramsDecl, params)                            \
    RetType methodName##S() noexcept override;                                                     \
    UTILS_ALWAYS_INLINE inline void methodName##R(RetType, paramsDecl);

#include "private/backend/DriverAPI.inc"

    /*
     * Memory management
     */

    HandleAllocatorWGPU mHandleAllocator;

    template<typename D>
    Handle<D> allocHandle() {
        return mHandleAllocator.allocate<D>();
    }

    template<typename D, typename B, typename... ARGS>
    D* constructHandle(Handle<B>& handle, ARGS&&... args) noexcept {
        return mHandleAllocator.construct<D>(handle, std::forward<ARGS>(args)...);
    }

    template<typename D, typename B>
    D* handleCast(Handle<B> handle) noexcept {
        return mHandleAllocator.handle_cast<D*>(handle);
    }

    template<typename D, typename B>
    void destructHandle(Handle<B>& handle) noexcept {
        auto* p = mHandleAllocator.handle_cast<D*>(handle);
        mHandleAllocator.deallocate(handle, p);
    }
};

}// namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_WEBGPUDRIVER_H

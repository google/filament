/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_VULKANDRIVER_H
#define TNT_FILAMENT_BACKEND_VULKANDRIVER_H

#include "VulkanBlitter.h"
#include "VulkanConstants.h"
#include "VulkanContext.h"
#include "VulkanDisposer.h"
#include "VulkanFboCache.h"
#include "VulkanHandles.h"
#include "VulkanPipelineCache.h"
#include "VulkanReadPixels.h"
#include "VulkanSamplerCache.h"
#include "VulkanStagePool.h"
#include "VulkanUtility.h"

#include "DriverBase.h"
#include "private/backend/Driver.h"
#include "private/backend/HandleAllocator.h"

#include <utils/Allocator.h>
#include <utils/compiler.h>

namespace filament::backend {

class VulkanPlatform;
struct VulkanSamplerGroup;

class VulkanHandleAllocator {
public:
    VulkanHandleAllocator(size_t arenaSize)
        : mHandleAllocatorImpl("Handles", arenaSize) {}

    template<typename D, typename... ARGS>
    inline Handle<D> initHandle(ARGS&&... args) noexcept {
        return mHandleAllocatorImpl.allocateAndConstruct<D>(std::forward<ARGS>(args)...);
    }

    template<typename D>
    inline Handle<D> allocHandle() noexcept {
        return mHandleAllocatorImpl.allocate<D>();
    }

    template<typename D, typename B, typename... ARGS>
    inline typename std::enable_if<std::is_base_of<B, D>::value, D>::type* construct(
            Handle<B> const& handle, ARGS&&... args) noexcept {
        return mHandleAllocatorImpl.construct<D, B>(handle, std::forward<ARGS>(args)...);
    }

    template<typename B, typename D,
            typename = typename std::enable_if<std::is_base_of<B, D>::value, D>::type>
    inline void destruct(Handle<B> handle, D const* p) noexcept {
        return mHandleAllocatorImpl.deallocate(handle, p);
    }

    template<typename Dp, typename B>
    inline typename std::enable_if_t<
            std::is_pointer_v<Dp> && std::is_base_of_v<B, typename std::remove_pointer_t<Dp>>, Dp>
    handle_cast(Handle<B>& handle) noexcept {
        return mHandleAllocatorImpl.handle_cast<Dp, B>(handle);
    }

    template<typename Dp, typename B>
    inline typename std::enable_if_t<
            std::is_pointer_v<Dp> && std::is_base_of_v<B, typename std::remove_pointer_t<Dp>>, Dp>
    handle_cast(Handle<B> const& handle) noexcept {
        return mHandleAllocatorImpl.handle_cast<Dp, B>(handle);
    }

    template<typename D, typename B>
    inline void destruct(Handle<B> handle) noexcept {
        if constexpr (std::is_base_of_v<VulkanIndexBuffer, D>
                      || std::is_base_of_v<VulkanBufferObject, D>) {
            auto ptr = handle_cast<D*>(handle);
            ptr->terminate();
        }
        destruct(handle, handle_cast<D const*>(handle));
    }

    HandleAllocatorVK mHandleAllocatorImpl;
};

class VulkanDriver final : public DriverBase {
public:
    static Driver* create(VulkanPlatform* platform, VulkanContext const& context,
            Platform::DriverConfig const& driverConfig) noexcept;

private:
    void debugCommandBegin(CommandStream* cmds, bool synchronous,
            const char* methodName) noexcept override;

    inline VulkanDriver(VulkanPlatform* platform, VulkanContext const& context,
            Platform::DriverConfig const& driverConfig) noexcept;

    ~VulkanDriver() noexcept override;

    Dispatcher getDispatcher() const noexcept final;

    ShaderModel getShaderModel() const noexcept final;

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

    VulkanDriver(VulkanDriver const&) = delete;
    VulkanDriver& operator=(VulkanDriver const&) = delete;

private:
    inline void setRenderPrimitiveBuffer(Handle<HwRenderPrimitive> rph, Handle<HwVertexBuffer> vbh,
            Handle<HwIndexBuffer> ibh);

    inline void setRenderPrimitiveRange(Handle<HwRenderPrimitive> rph, PrimitiveType pt,
            uint32_t offset, uint32_t minIndex, uint32_t maxIndex, uint32_t count);

    void collectGarbage();

    VulkanPlatform* mPlatform = nullptr;
    std::unique_ptr<VulkanCommands> mCommands;
    std::unique_ptr<VulkanTimestamps> mTimestamps;
    std::unique_ptr<VulkanTexture> mEmptyTexture;

    VulkanSwapChain* mCurrentSwapChain = nullptr;
    VulkanRenderTarget* mDefaultRenderTarget = nullptr;
    VulkanRenderPass mCurrentRenderPass = {};
    VmaAllocator mAllocator = VK_NULL_HANDLE;
    VkDebugReportCallbackEXT mDebugCallback = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT mDebugMessenger = VK_NULL_HANDLE;

    VulkanContext mContext = {};
    VulkanHandleAllocator mHandleAllocator;
    VulkanPipelineCache mPipelineCache;
    VulkanDisposer mDisposer;
    VulkanStagePool mStagePool;
    VulkanFboCache mFramebufferCache;
    VulkanSamplerCache mSamplerCache;
    VulkanBlitter mBlitter;
    VulkanSamplerGroup* mSamplerBindings[VulkanPipelineCache::SAMPLER_BINDING_COUNT] = {};
    VulkanReadPixels mReadPixels;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANDRIVER_H

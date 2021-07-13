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

#ifndef TNT_FILAMENT_DRIVER_VULKANDRIVER_H
#define TNT_FILAMENT_DRIVER_VULKANDRIVER_H

#include "VulkanPipelineCache.h"
#include "VulkanBlitter.h"
#include "VulkanDisposer.h"
#include "VulkanConstants.h"
#include "VulkanContext.h"
#include "VulkanFboCache.h"
#include "VulkanSamplerCache.h"
#include "VulkanStagePool.h"
#include "VulkanUtility.h"

#include "private/backend/Driver.h"
#include "private/backend/HandleAllocator.h"
#include "DriverBase.h"

#include <utils/compiler.h>
#include <utils/Allocator.h>

namespace filament {
namespace backend {

class VulkanPlatform;
struct VulkanRenderTarget;
struct VulkanSamplerGroup;

class VulkanDriver final : public DriverBase {
public:
    static Driver* create(backend::VulkanPlatform* platform,
            const char* const* ppEnabledExtensions, uint32_t enabledExtensionCount) noexcept;

private:

    void debugCommandBegin(CommandStream* cmds, bool synchronous, const char* methodName) noexcept override;

    inline VulkanDriver(backend::VulkanPlatform* platform,
            const char* const* ppEnabledExtensions, uint32_t enabledExtensionCount) noexcept;

    ~VulkanDriver() noexcept override;

    ShaderModel getShaderModel() const noexcept final;

    template<typename T>
    friend class backend::ConcreteDispatcher;

#define DECL_DRIVER_API(methodName, paramsDecl, params) \
    UTILS_ALWAYS_INLINE inline void methodName(paramsDecl);

#define DECL_DRIVER_API_SYNCHRONOUS(RetType, methodName, paramsDecl, params) \
    RetType methodName(paramsDecl) override;

#define DECL_DRIVER_API_RETURN(RetType, methodName, paramsDecl, params) \
    RetType methodName##S() noexcept override; \
    UTILS_ALWAYS_INLINE inline void methodName##R(RetType, paramsDecl);

#include "private/backend/DriverAPI.inc"

    VulkanDriver(VulkanDriver const&) = delete;
    VulkanDriver& operator = (VulkanDriver const&) = delete;

private:

    // See also the explicit template instantiation in HandleAllocator.cpp
    backend::HandleAllocator<16, 64, 880> mHandleAllocator;

    backend::VulkanPlatform& mContextManager;

    template<typename D, typename ... ARGS>
    backend::Handle<D> initHandle(ARGS&& ... args) noexcept {
        return mHandleAllocator.allocateAndConstruct<D>(std::forward<ARGS>(args) ...);
    }

    template<typename D>
    backend::Handle<D> allocHandle() noexcept {
        return mHandleAllocator.allocate<D>();
    }

    template<typename D, typename B, typename ... ARGS>
    typename std::enable_if<std::is_base_of<B, D>::value, D>::type*
    construct(backend::Handle<B> const& handle, ARGS&& ... args) noexcept {
        return mHandleAllocator.construct<D, B>(handle, std::forward<ARGS>(args) ...);
    }

    template<typename B, typename D,
            typename = typename std::enable_if<std::is_base_of<B, D>::value, D>::type>
    void destruct(backend::Handle<B> handle, D const* p) noexcept {
        return mHandleAllocator.deallocate(handle, p);
    }

    template<typename Dp, typename B>
    typename std::enable_if_t<
            std::is_pointer_v<Dp> &&
            std::is_base_of_v<B, typename std::remove_pointer_t<Dp>>, Dp>
    handle_cast(backend::Handle<B>& handle) noexcept {
        return mHandleAllocator.handle_cast<Dp, B>(handle);
    }

    template<typename Dp, typename B>
    inline typename std::enable_if_t<
            std::is_pointer_v<Dp> &&
            std::is_base_of_v<B, typename std::remove_pointer_t<Dp>>, Dp>
    handle_cast(backend::Handle<B> const& handle) noexcept {
        return mHandleAllocator.handle_cast<Dp, B>(handle);
    }

    template<typename D, typename B>
    void destruct(Handle<B> handle) noexcept {
        destruct(handle, handle_cast<D const*>(handle));
    }

    // This version of destruct takes a VulkanContext and calls a terminate(VulkanContext&)
    // on the handle before calling the dtor
    template<typename Dp, typename B>
    void destruct(VulkanContext& context, Handle<B> handle) noexcept {
        auto ptr = handle_cast<Dp*>(handle);
        ptr->terminate(context);
        mHandleAllocator.deallocate(handle, ptr);
    }

    void refreshSwapChain();
    void collectGarbage();

    VulkanContext mContext = {};
    VulkanPipelineCache mPipelineCache;
    VulkanDisposer mDisposer;
    VulkanStagePool mStagePool;
    VulkanFboCache mFramebufferCache;
    VulkanSamplerCache mSamplerCache;
    VulkanBlitter mBlitter;
    VulkanRenderTarget* mCurrentRenderTarget = nullptr;
    VulkanSamplerGroup* mSamplerBindings[VulkanPipelineCache::SAMPLER_BINDING_COUNT] = {};
    VkDebugReportCallbackEXT mDebugCallback = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT mDebugMessenger = VK_NULL_HANDLE;
};

} // namespace backend
} // namespace filament

#endif // TNT_FILAMENT_DRIVER_VULKANDRIVER_H

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

#include "VulkanBinder.h"
#include "VulkanDriverImpl.h"
#include "VulkanFboCache.h"
#include "VulkanSamplerCache.h"
#include "VulkanStagePool.h"

#include "driver/Driver.h"
#include "driver/DriverBase.h"

#include <utils/compiler.h>
#include <utils/Allocator.h>

#include <unordered_map>
#include <vector>

namespace filament {
namespace driver {

struct VulkanRenderTarget;
struct VulkanSamplerBuffer;

class VulkanDriver final : public DriverBase {
public:
    static Driver* create(driver::VulkanPlatform* platform,
            const char* const* ppEnabledExtensions, uint32_t enabledExtensionCount) noexcept;

private:

#ifndef NDEBUG
    void debugCommand(const char* methodName) override;
#endif

    inline VulkanDriver(driver::VulkanPlatform* platform,
            const char* const* ppEnabledExtensions, uint32_t enabledExtensionCount) noexcept;

    ~VulkanDriver() noexcept override;

    ShaderModel getShaderModel() const noexcept final;

    template<typename T>
    friend class ::filament::ConcreteDispatcher;

#define DECL_DRIVER_API(methodName, paramsDecl, params) \
    UTILS_ALWAYS_INLINE void methodName(paramsDecl);

#define DECL_DRIVER_API_SYNCHRONOUS(RetType, methodName, paramsDecl, params) \
    RetType methodName(paramsDecl) override;

#define DECL_DRIVER_API_RETURN(RetType, methodName, paramsDecl, params) \
    RetType methodName##Synchronous() noexcept override; \
    UTILS_ALWAYS_INLINE void methodName(RetType, paramsDecl);

#include "driver/DriverAPI.inc"

    VulkanDriver(VulkanDriver const&) = delete;
    VulkanDriver& operator = (VulkanDriver const&) = delete;

private:
    driver::VulkanPlatform& mContextManager;

    // For now we're not bothering to store handles in pools, just simple on-demand allocation.
    // We have a little map from integer handles to "blobs" which get replaced with the Hw objects.
    using Blob = std::vector<uint8_t>;
    using HandleMap = std::unordered_map<HandleBase::HandleId, Blob>;
    HandleMap mHandleMap;
    HandleBase::HandleId mNextId = 1;

    template<typename Dp, typename B>
    Handle<B> alloc_handle() {
        mHandleMap[mNextId] = Blob(sizeof(Dp));
        return Handle<B>(mNextId++);
    }

    template<typename Dp, typename B>
    Dp* handle_cast(HandleMap& handleMap, Handle<B>& handle) noexcept {
        assert(handle);
        auto iter = handleMap.find(handle.getId());
        assert(iter != handleMap.end());
        Blob& blob = iter->second;
        assert(blob.size() == sizeof(Dp));
        return reinterpret_cast<Dp*>(blob.data());
    }

    template<typename Dp, typename B>
    const Dp* handle_const_cast(HandleMap& handleMap, const Handle<B>& handle) noexcept {
        assert(handle);
        auto iter = handleMap.find(handle.getId());
        assert(iter != handleMap.end());
        Blob& blob = iter->second;
        assert(blob.size() == sizeof(Dp));
        return reinterpret_cast<const Dp*>(blob.data());
    }

    template<typename Dp, typename B, typename ... ARGS>
    Dp* construct_handle(HandleMap& handleMap, Handle<B>& handle, ARGS&& ... args) noexcept {
        auto iter = handleMap.find(handle.getId());
        assert(iter != handleMap.end());
        Blob& blob = iter->second;
        assert(blob.size() == sizeof(Dp));
        Dp* addr = reinterpret_cast<Dp*>(blob.data());
        new(addr) Dp(std::forward<ARGS>(args)...);
        return addr;
    }

    template<typename Dp, typename B>
    void destruct_handle(HandleMap& handleMap, Handle<B>& handle) noexcept {
        // Call the destructor, remove the blob, don't bother reclaiming the integer id.
        handle_cast<Dp>(handleMap, handle)->~Dp();
        handleMap.erase(handle.getId());
    }

    VulkanContext mContext = {};
    VulkanBinder mBinder;
    VulkanStagePool mStagePool;
    VulkanFboCache mFramebufferCache;
    VulkanSamplerCache mSamplerCache;
    VulkanRenderTarget* mCurrentRenderTarget = nullptr;
    VulkanSamplerBuffer* mSamplerBindings[VulkanBinder::NUM_SAMPLER_BINDINGS] = {};
    VkDebugReportCallbackEXT mDebugCallback = VK_NULL_HANDLE;
};

} // namespace driver
} // namespace filament

#endif // TNT_FILAMENT_DRIVER_VULKANDRIVER_H

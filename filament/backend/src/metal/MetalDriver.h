/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DRIVER_METALDRIVER_H
#define TNT_FILAMENT_DRIVER_METALDRIVER_H

#include "private/backend/Driver.h"
#include "DriverBase.h"

#include <utils/compiler.h>
#include <utils/Log.h>

#include <tsl/robin_map.h>

#include <mutex>

namespace filament {
namespace backend {

class MetalPlatform;

namespace metal {

struct MetalUniformBuffer;
struct MetalContext;
struct MetalProgram;
struct UniformBufferState;

class MetalDriver final : public DriverBase {
    explicit MetalDriver(backend::MetalPlatform* platform) noexcept;
    ~MetalDriver() noexcept override;

public:
    static Driver* create(backend::MetalPlatform* platform);

private:

    backend::MetalPlatform& mPlatform;

    MetalContext* mContext;

#ifndef NDEBUG
    void debugCommand(const char* methodName) override;
#endif

    ShaderModel getShaderModel() const noexcept final;

    // Overrides the default implementation by wrapping the call to fn in an @autoreleasepool block.
    void execute(std::function<void(void)> fn) noexcept final;

    /*
     * Driver interface
     */

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

    /*
     * Memory management
     */

    // Copied from VulkanDriver.h

    // For now we're not bothering to store handles in pools, just simple on-demand allocation.
    // We have a little map from integer handles to "blobs" which get replaced with the Hw objects.
    using Blob = void*;
    using HandleMap = tsl::robin_map<HandleBase::HandleId, Blob>;
    std::mutex mHandleMapMutex;
    HandleMap mHandleMap;
    HandleBase::HandleId mNextId = 1;

    template<typename Dp, typename B>
    Handle<B> alloc_handle() {
        std::lock_guard<std::mutex> lock(mHandleMapMutex);
        mHandleMap[mNextId] = malloc(sizeof(Dp));
        return Handle<B>(mNextId++);
    }

    template<typename Dp, typename B, typename ... ARGS>
    Handle<B> alloc_and_construct_handle(ARGS&& ... args) {
        std::lock_guard<std::mutex> lock(mHandleMapMutex);
        Blob blob = mHandleMap[mNextId] = malloc(sizeof(Dp));
        Dp* addr = reinterpret_cast<Dp*>(blob);
        new(addr) Dp(std::forward<ARGS>(args)...);
        return Handle<B>(mNextId++);
    }

    template<typename Dp, typename B>
    Dp* handle_cast(HandleMap& handleMap, Handle<B> handle) noexcept {
        assert(handle);
        if (!handle) return nullptr; // better to get a NPE than random behavior/corruption
        std::lock_guard<std::mutex> lock(mHandleMapMutex);
        auto iter = handleMap.find(handle.getId());
        assert(iter != handleMap.end());
        Blob& blob = iter.value();
        return reinterpret_cast<Dp*>(blob);
    }

    template<typename Dp, typename B>
    const Dp* handle_const_cast(HandleMap& handleMap, const Handle<B>& handle) noexcept {
        assert(handle);
        if (!handle) return nullptr; // better to get a NPE than random behavior/corruption
        std::lock_guard<std::mutex> lock(mHandleMapMutex);
        auto iter = handleMap.find(handle.getId());
        assert(iter != handleMap.end());
        Blob& blob = iter.value();
        return reinterpret_cast<const Dp*>(blob);
    }

    template<typename Dp, typename B, typename ... ARGS>
    Dp* construct_handle(HandleMap& handleMap, Handle<B>& handle, ARGS&& ... args) noexcept {
        assert(handle);
        if (!handle) return nullptr; // better to get a NPE than random behavior/corruption
        std::lock_guard<std::mutex> lock(mHandleMapMutex);
        auto iter = handleMap.find(handle.getId());
        assert(iter != handleMap.end());
        Blob& blob = iter.value();
        Dp* addr = reinterpret_cast<Dp*>(blob);
        new(addr) Dp(std::forward<ARGS>(args)...);
        return addr;
    }

    template<typename Dp, typename B>
    void destruct_handle(HandleMap& handleMap, Handle<B>& handle) noexcept {
        std::lock_guard<std::mutex> lock(mHandleMapMutex);
        assert(handle);
        // Call the destructor, remove the blob, don't bother reclaiming the integer id.
        auto iter = handleMap.find(handle.getId());
        assert(iter != handleMap.end());
        Blob& blob = iter.value();
        reinterpret_cast<Dp*>(blob)->~Dp();
        free(blob);
        handleMap.erase(handle.getId());
    }

    void enumerateSamplerGroups(const MetalProgram* program,
            const std::function<void(const SamplerGroup::Sampler*, size_t)>& f);
    void enumerateBoundUniformBuffers(const std::function<void(const UniformBufferState&,
            MetalUniformBuffer*, uint32_t)>& f);

};

} // namespace metal
} // namespace backend
} // namespace filament

#endif // TNT_FILAMENT_DRIVER_METALDRIVER_H

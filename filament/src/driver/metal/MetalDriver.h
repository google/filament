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

#include "driver/Driver.h"
#include "driver/DriverBase.h"

#include <utils/compiler.h>
#include <utils/Log.h>

#include <tsl/robin_map.h>

#include <mutex>

namespace filament {
namespace driver {
namespace metal {

struct MetalDriverImpl;

struct MetalProgram;

class MetalDriver final : public DriverBase {
    MetalDriver(driver::MetalPlatform* const platform) noexcept;
    virtual ~MetalDriver() noexcept;

public:
    static Driver* create(driver::MetalPlatform* platform);

private:

    driver::MetalPlatform& mPlatform;

    MetalDriverImpl* pImpl;

#ifndef NDEBUG
    void debugCommand(const char* methodName) override;
#endif

    virtual ShaderModel getShaderModel() const noexcept override final { return ShaderModel::GL_CORE_41; }

    /*
     * Driver interface
     */

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

    template<typename Dp, typename B>
    Dp* handle_cast(HandleMap& handleMap, Handle<B>& handle) noexcept {
        std::lock_guard<std::mutex> lock(mHandleMapMutex);
        assert(handle);
        auto iter = handleMap.find(handle.getId());
        assert(iter != handleMap.end());
        Blob& blob = iter.value();
        return reinterpret_cast<Dp*>(blob);
    }

    template<typename Dp, typename B>
    const Dp* handle_const_cast(HandleMap& handleMap, const Handle<B>& handle) noexcept {
        std::lock_guard<std::mutex> lock(mHandleMapMutex);
        assert(handle);
        auto iter = handleMap.find(handle.getId());
        assert(iter != handleMap.end());
        Blob& blob = iter.value();
        return reinterpret_cast<const Dp*>(blob);
    }

    template<typename Dp, typename B, typename ... ARGS>
    Dp* construct_handle(HandleMap& handleMap, Handle<B>& handle, ARGS&& ... args) noexcept {
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

    void enumerateSamplerBuffers(const MetalProgram *program,
            const std::function<void(const SamplerBuffer::Sampler*, uint8_t)>& f);
};

} // namespace metal
} // namespace driver
} // namespace filament

#endif // TNT_FILAMENT_DRIVER_METALDRIVER_H

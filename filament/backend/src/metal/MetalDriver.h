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

#include <backend/DriverEnums.h>
#include "private/backend/Driver.h"
#include "DriverBase.h"

#include "private/backend/HandleAllocator.h"

#include <backend/SamplerDescriptor.h>

#include <utils/compiler.h>
#include <utils/Log.h>
#include <utils/debug.h>

#include <functional>
#include <mutex>
#include <vector>
#include <deque>

namespace filament {
namespace backend {

class MetalPlatform;

class MetalBuffer;
class MetalProgram;
class MetalTexture;
struct MetalUniformBuffer;
struct MetalContext;
struct BufferState;

#ifndef FILAMENT_METAL_HANDLE_ARENA_SIZE_IN_MB
#define FILAMENT_METAL_HANDLE_ARENA_SIZE_IN_MB 8
#endif

class MetalDriver final : public DriverBase {
    explicit MetalDriver(MetalPlatform* platform, const Platform::DriverConfig& driverConfig) noexcept;
    ~MetalDriver() noexcept override;
    Dispatcher getDispatcher() const noexcept final;

public:
    static Driver* create(MetalPlatform* platform, const Platform::DriverConfig& driverConfig);

private:

    friend class MetalSwapChain;
    friend struct MetalDescriptorSet;

    MetalPlatform& mPlatform;
    MetalContext* mContext;

    ShaderModel getShaderModel() const noexcept final;

    // Overrides the default implementation by wrapping the call to fn in an @autoreleasepool block.
    void execute(std::function<void(void)> const& fn) noexcept final;

    /*
     * Tasks run regularly on the driver thread.
     * Not thread-safe; tasks are run from the driver thead and must be enqueued from the driver
     * thread.
     */
    void runAtNextTick(const std::function<void()>& fn) noexcept;
    void executeTickOps() noexcept;
    std::vector<std::function<void()>> mTickOps;

    // Tasks regularly executed on the driver thread after a command buffer has completed
    struct DeferredTask {
        DeferredTask(uint64_t commandBufferId, utils::Invocable<void()>&& fn) noexcept
            : commandBufferId(commandBufferId), fn(std::move(fn)) {}
        uint64_t commandBufferId;     // after this command buffer completes
        utils::Invocable<void()> fn;  // execute this task
    };
    void executeAfterCurrentCommandBufferCompletes(utils::Invocable<void()>&& fn) noexcept;
    void executeDeferredOps() noexcept;
    std::deque<DeferredTask> mDeferredTasks;

    /*
     * Driver interface
     */

    template<typename T>
    friend class ConcreteDispatcher;

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

    HandleAllocatorMTL mHandleAllocator;

    template<typename D>
    Handle<D> alloc_handle() {
        return mHandleAllocator.allocate<D>();
    }

    template<typename D, typename B, typename ... ARGS>
    Handle<B> alloc_and_construct_handle(ARGS&& ... args) {
        return mHandleAllocator.allocateAndConstruct<D>(std::forward<ARGS>(args)...);
    }

    template<typename D, typename B>
    D* handle_cast(Handle<B> handle) noexcept {
        return mHandleAllocator.handle_cast<D*>(handle);
    }

    template<typename D, typename B>
    const D* handle_const_cast(const Handle<B>& handle) noexcept {
        return mHandleAllocator.handle_cast<D*>(handle);
    }

    template<typename D, typename B, typename ... ARGS>
    D* construct_handle(Handle<B>& handle, ARGS&& ... args) noexcept {
        return mHandleAllocator.construct<D>(handle, std::forward<ARGS>(args)...);
    }

    template<typename D, typename B>
    void destruct_handle(Handle<B>& handle) noexcept {
        auto* p = mHandleAllocator.handle_cast<D*>(handle);
        mHandleAllocator.deallocate(handle, p);
    }

    inline void setRenderPrimitiveBuffer(Handle<HwRenderPrimitive> rph, PrimitiveType pt,
            Handle<HwVertexBuffer> vbh, Handle<HwIndexBuffer> ibh);

    void enumerateBoundBuffers(BufferObjectBinding bindingType,
            const std::function<void(const BufferState&, MetalBuffer*, uint32_t)>& f);

    backend::StereoscopicType const mStereoscopicType;
};

} // namespace backend
} // namespace filament

#endif // TNT_FILAMENT_DRIVER_METALDRIVER_H

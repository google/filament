/*
 * Copyright (C) 2024 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_VULKAN_MEMORY_RESOURCEMANAGER_H
#define TNT_FILAMENT_BACKEND_VULKAN_MEMORY_RESOURCEMANAGER_H

#include "vulkan/memory/Resource.h"

#include "vulkan/VulkanAsyncHandles.h"

#include <private/backend/HandleAllocator.h>

#include <utils/Panic.h>

namespace filament::backend::fvkmemory {

class ResourceManager {
public:
    ResourceManager(size_t arenaSize, bool disableUseAfterFreeCheck);

    template<typename D>
    inline Handle<D> allocHandle() noexcept {
        return mHandleAllocatorImpl.allocate<D>();
    }

    inline void associateHandle(HandleBase::HandleId id, utils::CString&& tag) noexcept {
        mHandleAllocatorImpl.associateTagToHandle(id, std::move(tag));
    }

    void gc() noexcept;
    void print() const noexcept;
    void terminate() noexcept;

private:
    using AllocatorImpl = HandleAllocatorVK;

    template<typename D>
    using requires_thread_safety = typename std::disjunction<std::is_same<D, VulkanFence>,
            std::is_same<D, VulkanTimerQuery>>;

    template<typename D, typename B, typename... ARGS>
    inline D* construct(Handle<B> const& handle, ARGS&&... args) noexcept {
        constexpr bool THREAD_SAFETY = requires_thread_safety<D>::value;
        using ResourceT = ResourceImpl<THREAD_SAFETY>;
        D* obj = mHandleAllocatorImpl.construct<D, B>(handle, std::forward<ARGS>(args)...);
        ((ResourceT*) obj)->template init<D>(handle.getId(), this);
        traceConstruction(getTypeEnum<D>(), handle.getId());
        return obj;
    }

    template<typename Dp, typename B>
    inline typename std::enable_if_t<
            std::is_pointer_v<Dp> && std::is_base_of_v<B, typename std::remove_pointer_t<Dp>>, Dp>
    handle_cast(Handle<B> const& handle) noexcept {
        return mHandleAllocatorImpl.handle_cast<Dp, B>(handle);
    }

    template<typename D, typename B>
    inline void destruct(Handle<B> handle) noexcept {
        D* obj = handle_cast<D*>(handle);
        mHandleAllocatorImpl.deallocate(handle, obj);
    }

    // This will post the destruction to a list that will be processed when gc is called from the
    // backend thread.
    inline void destructLaterWithType(ResourceType type, HandleId id) {
        if (isThreadSafeType(type)) {
            std::unique_lock<utils::Mutex> lock(mThreadSafeGcListMutex);
            mThreadSafeGcList.push_back({type, id});
        } else {
            mGcList.push_back({type, id});
        }
    }

    void destroyWithType(ResourceType type, HandleId id);

    void traceConstruction(ResourceType type, HandleId id);

    AllocatorImpl mHandleAllocatorImpl;

    using GcList = std::vector<std::pair<ResourceType, HandleId>>;

    utils::Mutex mThreadSafeGcListMutex;
    GcList mThreadSafeGcList;
    GcList mGcList;

    template<typename D>
    friend struct resource_ptr;

    static constexpr bool IS_THREAD_SAFE = true;
    static constexpr bool IS_NOT_THREAD_SAFE = false;

    friend struct ResourceImpl<IS_THREAD_SAFE>;
    friend struct ResourceImpl<IS_NOT_THREAD_SAFE>;
};

} // namespace filament::backend::fvkmemory

#endif // TNT_FILAMENT_BACKEND_VULKAN_MEMORY_RESOURCEMANAGER_H

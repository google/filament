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
#include "vulkan/memory/ResourceCounter.h"
#include "vulkan/VulkanAsyncHandles.h"

#include <private/backend/HandleAllocator.h>

#include <utils/Panic.h>

namespace filament::backend::fvkmemory {

// Mostly a wrapper over HandleAllocator. This class also contains a "counter" pool that will assign
// a ref-count object for each unique, *constructed* handles. *Constructed*, in this sense, means
// that the handle has been allocated and the constructor was called.
class ResourceManager {
public:
    static void init(size_t arenaSize, bool disableUseAfterFreeCheck) noexcept {
        FILAMENT_CHECK_PRECONDITION(!sSingleton)
                << "Cannot call ResourceManager::init() more than once";
        sSingleton = new ResourceManager(arenaSize, disableUseAfterFreeCheck);
    }

    static void terminate() noexcept {
        FILAMENT_CHECK_PRECONDITION(sSingleton)
                << "Cannot call ResourceManager::terminate() before init()";
        FILAMENT_CHECK_PRECONDITION(sSingleton->mGcList.empty())
                << "Still have unfreed object";
        delete sSingleton;
    }

    template<typename D>
    static inline Handle<D> allocHandle() noexcept {
        auto s = sSingleton;
        return s->mHandleAllocatorImpl.allocate<D>();
    }

    static inline void associateHandle(HandleBase::HandleId id, utils::CString&& tag) noexcept {
        auto s = sSingleton;
        s->mHandleAllocatorImpl.associateTagToHandle(id, std::move(tag));
    }

    static inline void print() {
        auto s = sSingleton;
        s->printImpl();
    }

    static inline void gc() {
        auto s = sSingleton;
        s->gcImpl();
    }

private:
    using AllocatorImpl = HandleAllocatorVK;

    template <typename D>
    using requires_thread_safety = typename std::disjunction<
        std::is_same<D, VulkanFence>, std::is_same<D, VulkanTimerQuery>>;

    ResourceManager(size_t arenaSize, bool disableUseAfterFreeCheck);

    template<typename D, typename B, typename... ARGS>
    static inline RefCounter construct(Handle<B> const& handle, ARGS&&... args) noexcept {
        // TODO: to be implemented by uncommenting
        // constexpr bool THREAD_SAFETY = requires_thread_safety<D>::value;
        // auto s = sSingleton;
        // D* obj = s->mHandleAllocatorImpl.construct<D, B>(handle, std::forward<ARGS>(args)...);
        // obj->counterIndex = s->mPool.create<THREAD_SAFETY>(getTypeEnum<D>(), handle.getId());
        // s->trackIncrement(getTypeEnum<D>(), handle.getId());
        // return s->mPool.get<THREAD_SAFETY>(obj->counterIndex);

        return RefCounter();
    }

    template<typename Dp, typename B>
    static inline typename std::enable_if_t<
            std::is_pointer_v<Dp> && std::is_base_of_v<B, typename std::remove_pointer_t<Dp>>, Dp>
    handle_cast(Handle<B> const& handle) noexcept {
        auto s = sSingleton;
        return s->mHandleAllocatorImpl.handle_cast<Dp, B>(handle);
    }

    template<typename D, typename B>
    static inline void destruct(Handle<B> handle) noexcept {
        // TODO: to be implemented by uncommenting
        // constexpr bool THREAD_SAFETY = requires_thread_safety<D>::value;
        // auto s = sSingleton;
        // D* obj = s->handle_cast<D*>(handle);
        // CounterIndex const counterIndex = obj->counterIndex;
        // s->mPool.free<THREAD_SAFETY>(counterIndex);
        // s->mHandleAllocatorImpl.deallocate(handle, obj);
    }

    template<typename D>
    static RefCounter fromCounterIndex(CounterIndex counterInd) {
        constexpr bool THREAD_SAFETY = requires_thread_safety<D>::value;
        auto s = sSingleton;
        return s->mPool.get<THREAD_SAFETY>(counterInd);
    }

    static void destroyWithType(ResourceType type, HandleId id);

    // This will post the destruction to a list that will be processed when gc is called from the
    // backend thread.
    inline static void destroyLaterWithType(ResourceType type, HandleId id) {
        auto s = sSingleton;
        std::unique_lock<utils::Mutex> lock(s->mGcListMutex);
        s->mGcList.push_back({type, id});
    }

    void gcImpl();
    void printImpl() const;
    void trackIncrement(ResourceType type, HandleId id);

    AllocatorImpl mHandleAllocatorImpl;
    static ResourceManager* sSingleton;

    utils::Mutex mGcListMutex;
    std::vector<std::pair<ResourceType, HandleId>> mGcList;

    RefCounterPool mPool;

    template<typename D>
    friend struct resource_ptr;

    friend struct CounterImpl<true>;
    friend struct CounterImpl<false>;
};

} // namespace filament::backend::fvkmemory

#endif // TNT_FILAMENT_BACKEND_VULKAN_MEMORY_RESOURCEMANAGER_H

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

#include "vulkan/memory/ResourceCounter.h"
#include "vulkan/memory/ResourceManager.h"

namespace filament::backend::fvkmemory {

template void CounterImpl<true>::destroyNow(ResourceType type, HandleId id);
template void CounterImpl<false>::destroyNow(ResourceType type, HandleId id);
template<bool THREAD_SAFE>
void CounterImpl<THREAD_SAFE>::destroyNow(ResourceType type, HandleId id) {
    ResourceManager::destroyWithType(type, id);
}

template void CounterImpl<true>::destroyLater(ResourceType type, HandleId id);
template void CounterImpl<false>::destroyLater(ResourceType type, HandleId id);
template<bool THREAD_SAFE>
void CounterImpl<THREAD_SAFE>::destroyLater(ResourceType type, HandleId id) {
    ResourceManager::destroyLaterWithType(type, id);
}

template void CounterPoolImpl<true>::createCounter(CounterIndex freeInd, ResourceType type,
        HandleId id);

template void CounterPoolImpl<false>::createCounter(CounterIndex freeInd, ResourceType type,
        HandleId id);

// We define this functions in the cpp because they require knowing the destruction methods from
// ResourceManager.
template<bool THREAD_SAFE>
void CounterPoolImpl<THREAD_SAFE>::createCounter(CounterIndex freeInd, ResourceType type,
        HandleId id) {
    auto const impl = [&]() {
        Counter* ret = &mCounters[freeInd];
        new (ret) Counter(type, id);
    };
    if constexpr (THREAD_SAFE) {
        std::unique_lock<utils::Mutex> lock(mCountersMutex);
        impl();
    } else {
        impl();
    }
}

} // namespace filament::backend::fvkmemory {

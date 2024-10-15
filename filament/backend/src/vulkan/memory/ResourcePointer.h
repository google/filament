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

#ifndef TNT_FILAMENT_BACKEND_VULKAN_MEMORY_RESOURCEPOINTER_H
#define TNT_FILAMENT_BACKEND_VULKAN_MEMORY_RESOURCEPOINTER_H

#include "vulkan/memory/Resource.h"
#include "vulkan/memory/ResourceManager.h"

#include <backend/Handle.h>

#include <utility>

namespace filament::backend::fvkmemory {

template<typename D>
struct resource_ptr {

private:
    template<typename B>
    using enabled_resource_ptr =
            resource_ptr<typename std::enable_if<std::is_base_of<B, D>::value, D>::type>;
public:

    template<typename B, typename... ARGS>
    static enabled_resource_ptr<B> make(Handle<B> const& handle, ARGS&&... args) noexcept {
        auto counter = ResourceManager::construct<D, B>(handle, std::forward<ARGS>(args)...);
        return {counter};
    }

    // This will alloc a handle and then construct the object.
    template<typename... ARGS>
    static resource_ptr<D> construct(ARGS&&... args) noexcept {
        auto handle = ResourceManager::allocHandle<D>();
        auto counter = ResourceManager::construct<D, D>(handle, std::forward<ARGS>(args)...);
        return {counter};
    }

    template<typename B>
    static enabled_resource_ptr<B> cast(Handle<B> const& handle) noexcept {
        D* ptr = ResourceManager::handle_cast<D*, B>(handle);
        auto counter = ResourceManager::fromCounterIndex<D>(ptr->counterIndex);
        return {counter};
    }

    static resource_ptr<D> cast(D* ptr) noexcept {
        assert_invariant(ptr);
        auto counter = ResourceManager::fromCounterIndex<D>(ptr->counterIndex);
        return {counter};
    }

    struct Hash {
        size_t operator()(resource_ptr<D> const& s) const {
            size_t seed = 0;
            // Combine hashes of the integer and the pointer
            seed ^= std::hash<size_t>{}(s.id()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= std::hash<uint64_t>{}(reinterpret_cast<uint64_t>(s.get())) + 0x9e3779b9 +
                    (seed << 6) + (seed >> 2);
            return seed;
        }
    };

    ~resource_ptr() {
        if (mCounter) {
            mCounter.dec();
        }
    }

    inline resource_ptr() = default;

    // move constructor operator
    inline resource_ptr(resource_ptr<D>&& rhs) {
        (*this) = std::move(rhs);
    }

    inline resource_ptr(resource_ptr<D> const& rhs) {
        (*this) = rhs;
    }

    template<typename E>
    using is_supported_t =
            typename std::enable_if<std::is_base_of<D, E>::value>::type;
    template<typename E, typename = is_supported_t<E>>
    inline resource_ptr(resource_ptr<E> const& rhs) {
        (*this) = rhs;
    }

    // move operator
    inline resource_ptr<D>& operator=(resource_ptr<D> && rhs) {
        if (mCounter && mCounter != rhs.mCounter) {
            mCounter.dec();
        }
        mCounter = rhs.mCounter; // There should be no change in the reference count.
        mRef = rhs.mRef;
        rhs.mCounter = RefCounter {};
        rhs.mRef = nullptr;
        return *this;
    }

    inline resource_ptr<D>& operator=(resource_ptr<D> const& rhs) {
        if (mCounter == rhs.mCounter) {
            return *this;
        }
        if (mCounter) {
            mCounter.dec();
        }
        mCounter = rhs.mCounter;
        mCounter.inc();
        mRef = rhs.mRef;
        return *this;
    }

    template<typename E, typename = is_supported_t<E>>
    inline resource_ptr<D>& operator=(resource_ptr<E> const& rhs) {
        if (mCounter == rhs.mCounter) {
            return *this;
        }
        if (mCounter) {
            mCounter.dec();
        }
        mCounter = rhs.mCounter;
        mCounter.inc();
        mRef = rhs.mRef;
        return *this;
    }

    inline bool operator==(resource_ptr<D> const& other) const {
        return id() == other.id() && type() == other.type();
    }

    inline explicit operator bool() const {
        return bool(mCounter);
    }

    inline D* operator->() {
        return get();
    }

    inline D const* operator->() const {
        return get();
    }

    inline HandleId id() const {
        if (mCounter) {
            return mCounter.id();
        }
        return HandleBase::nullid;
    }

private:
    inline D* get() const {
        if (!mRef) {
            mRef = ResourceManager::handle_cast<D*>(Handle<D>(id()));
        }
        return mRef;
    }

    inline ResourceType type() const {
        if (mCounter) {
            return mCounter.type();
        }
        return ResourceType::UNDEFINED_TYPE;
    }

    resource_ptr(RefCounter counter)
        : mCounter(counter),
          mRef(nullptr) {
        mCounter.inc();
    }

    RefCounter mCounter;
    mutable D* mRef = nullptr;

    // Allow generic structures to hold resources of different types.  For example,
    //    std::vector<fvkmemory::Resource> resources;
    friend struct resource_ptr<Resource>;
};

} // namespace filament::backend::fvkmemory

#endif // TNT_FILAMENT_BACKEND_VULKAN_MEMORY_RESOURCEPOINTER_H

/*
 * Copyright (C) 2015 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DETAILS_RESOURCELIST_H
#define TNT_FILAMENT_DETAILS_RESOURCELIST_H

#include <utils/compiler.h>
#include <utils/Allocator.h>
#include <utils/Log.h>

#include <tsl/robin_set.h>

#include <algorithm>
#include <mutex>

namespace filament {

class ResourceListBase {
public:
    explicit ResourceListBase(const char* typeName);
    ResourceListBase(ResourceListBase&& rhs) noexcept = default;

    ~ResourceListBase() noexcept;

    void insert(void* item);

    bool remove(void const* item);

    void clear() noexcept;

    bool empty() const noexcept {
        return mList.empty();
    }

    size_t size() const noexcept {
        return mList.size();
    }


    tsl::robin_set<void*> getListAndClear() noexcept {
        return std::move(mList);
    }

    using iterator = typename tsl::robin_set<void*>::iterator;

    iterator begin() noexcept {
        return mList.begin();
    }

    iterator end() noexcept {
        return mList.end();
    }

protected:
    void forEach(void(*f)(void* user, void *p), void* user) const noexcept;
    tsl::robin_set<void*> mList;
#ifndef NDEBUG
private:
    // removing this saves 8-bytes because of padding of derived classes
    const char* const mTypeName;
#endif
};

// The split ResourceListBase / ResourceList allows us to reduce code size by keeping
// common code (operating on void*) separate.
//
template <typename T>
class ResourceList : private ResourceListBase {
    static_assert(sizeof(tsl::robin_set<T*>) == sizeof(tsl::robin_set<void*>),
            "robin_set<void*> and robin_set<T*> are incompatible");

public:
    using ResourceListBase::forEach;

    explicit ResourceList(const char* name) noexcept : ResourceListBase(name) { }
    ResourceList(ResourceList&& rhs) noexcept = default;

    ~ResourceList() noexcept = default;

    void insert(T* item) {
        ResourceListBase::insert(item);
    }
    bool remove(T const* item) {
        return ResourceListBase::remove(item);
    }
    bool empty() const noexcept {
        return ResourceListBase::empty();
    }
    size_t size() const noexcept {
        return ResourceListBase::size();
    }
    void clear() noexcept {
        return ResourceListBase::clear();
    }

    template<typename F>
    inline void forEach(F func) const noexcept {
        // turn closure into function pointer call, we do this to reduce code size.
        this->forEach(+[](void* user, void* p) {
            ((F*)user)->operator()((T*)p);
        }, &func);
    }
};

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_RESOURCELIST_H

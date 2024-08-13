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

#ifndef TNT_FILAMENT_RESOURCELIST_H
#define TNT_FILAMENT_RESOURCELIST_H

#include <utils/compiler.h>

#include <tsl/robin_set.h>

#include <stdint.h>

namespace filament {

class ResourceListBase {
public:
    using iterator = typename tsl::robin_set<void*>::iterator;
    using const_iterator = typename tsl::robin_set<void*>::const_iterator;

    explicit ResourceListBase(const char* typeName);
    ResourceListBase(ResourceListBase&& rhs) noexcept = default;

    ~ResourceListBase() noexcept;

    void insert(void* item);

    bool remove(void const* item);

    iterator find(void const* item);

    void clear() noexcept;

    bool empty() const noexcept {
        return mList.empty();
    }

    size_t size() const noexcept {
        return mList.size();
    }

    iterator begin() noexcept {
        return mList.begin();
    }

    iterator end() noexcept {
        return mList.end();
    }

    const_iterator begin() const noexcept {
        return mList.begin();
    }

    const_iterator end() const noexcept {
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
template<typename T>
class ResourceList : private ResourceListBase {
public:
    using ResourceListBase::ResourceListBase;
    using ResourceListBase::forEach;
    using ResourceListBase::insert;
    using ResourceListBase::remove;
    using ResourceListBase::find;
    using ResourceListBase::empty;
    using ResourceListBase::size;
    using ResourceListBase::clear;
    using ResourceListBase::begin;
    using ResourceListBase::end;

    explicit ResourceList(const char* name) noexcept: ResourceListBase(name) {}

    ResourceList(ResourceList&& rhs) noexcept = default;

    ~ResourceList() noexcept = default;

    template<typename F>
    inline void forEach(F func) const noexcept {
        // turn closure into function pointer call, we do this to reduce code size.
        this->forEach(+[](void* user, void* p) {
            ((F*)user)->operator()((T*)p);
        }, &func);
    }
};

} // namespace filament

#endif // TNT_FILAMENT_RESOURCELIST_H

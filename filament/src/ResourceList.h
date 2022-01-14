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
    UTILS_NOINLINE
    explicit ResourceListBase(const char* typeName)
#ifndef NDEBUG
            : mTypeName(typeName)
#endif
    {
    }

    UTILS_NOINLINE
    ~ResourceListBase() noexcept {
#ifndef NDEBUG
        if (!mList.empty()) {
            utils::slog.d << "leaked " << mList.size() << " " << mTypeName << utils::io::endl;
        }
#endif
    }

    UTILS_NOINLINE
    void insert(void* item) {
        mList.insert(item);
    }

    UTILS_NOINLINE
    bool remove(void const* item) {
        return mList.erase(const_cast<void*>(item)) > 0;
    }

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
template <typename T, typename LockingPolicy = utils::LockingPolicy::NoLock>
class ResourceList : public ResourceListBase {
    static_assert(sizeof(tsl::robin_set<T*>) == sizeof(tsl::robin_set<void*>),
            "robin_set<void*> and robin_set<T*> are incompatible");

public:
    ResourceList(const char* name) noexcept : ResourceListBase(name) { }

    ~ResourceList() noexcept = default;

    void insert(T* item) {
        std::lock_guard<LockingPolicy> guard(mLock);
        ResourceListBase::insert(item);
    }
    bool remove(T const* item) {
        std::lock_guard<LockingPolicy> guard(mLock);
        return ResourceListBase::remove(item);
    }
    bool empty() const noexcept {
        std::lock_guard<LockingPolicy> guard(mLock);
        return ResourceListBase::empty();
    }
    size_t size() const noexcept {
        std::lock_guard<LockingPolicy> guard(mLock);
        return ResourceListBase::size();
    }

    tsl::robin_set<T*> getListAndClear() noexcept {
        std::lock_guard<LockingPolicy> guard(mLock);
        tsl::robin_set<void*> list(ResourceListBase::getListAndClear());
        // this is pretty ugly, but this avoids a copy
        return std::move(reinterpret_cast<tsl::robin_set<T*>&>(list));
    }

    /*
     * the methods below are only safe when LockingPolicy is NoLock, so disable them
     * otherwise
     */

    using iterator = typename tsl::robin_set<T*>::iterator;
    using const_iterator = typename tsl::robin_set<T*>::const_iterator;

    template<typename LP, typename U>
    using enable_if_t = typename std::enable_if<std::is_same<utils::LockingPolicy::NoLock, LP>::value, U>::type;

    template<typename LP = LockingPolicy>
    enable_if_t<LP, iterator> begin() noexcept {
        // this is pretty ugly, but this works
        return reinterpret_cast<tsl::robin_set<T*>&>(mList).begin();
    }

    template<typename LP = LockingPolicy>
    enable_if_t<LP, iterator> end() noexcept {
        // this is pretty ugly, but this works
        return reinterpret_cast<tsl::robin_set<T*>&>(mList).end();
    }

    template<typename LP = LockingPolicy>
    enable_if_t<LP, const_iterator> begin() const noexcept {
        // this is pretty ugly, but this works
        return reinterpret_cast<tsl::robin_set<T*> const&>(mList).begin();
    }

    template<typename LP = LockingPolicy>
    enable_if_t<LP, const_iterator> end() const noexcept {
        // this is pretty ugly, but this works
        return reinterpret_cast<tsl::robin_set<T*> const&>(mList).end();
    }

private:
    mutable LockingPolicy mLock;
};

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_RESOURCELIST_H

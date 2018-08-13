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

#ifndef TNT_FILAMENT_FILAMENT_API_H
#define TNT_FILAMENT_FILAMENT_API_H

#include <utils/compiler.h>

#include <stddef.h>

namespace filament {

/**
 * \privatesection
 * FilamentAPI is used to define an API in filament.
 * It ensures the class defining the API can't be created, destroyed
 * or copied by the caller.
 */
class UTILS_PUBLIC FilamentAPI {
protected:
    // disallow creation on the stack
    FilamentAPI() noexcept = default;
    ~FilamentAPI() = default;

public:
    // disallow copy and assignment
    FilamentAPI(FilamentAPI const&) = delete;
    FilamentAPI(FilamentAPI&&) = delete;
    FilamentAPI& operator=(FilamentAPI const&) = delete;
    FilamentAPI& operator=(FilamentAPI&&) = delete;

    // allow placement-new allocation, don't use "noexcept", to avoid compiler null check
    static void *operator new     (size_t, void* p) { return p; }

    // prevent heap allocation
    static void *operator new     (size_t) = delete;
    static void *operator new[]   (size_t) = delete;
    static void  operator delete  (void*)  = delete;
    static void  operator delete[](void*)  = delete;
};


/**
 * \privatesection
 * BuilderBase is used to hide the implementation details of builders and ensure a higher
 * level of backward binary compatibility.
 * The actual implementation is in src/FilamentAPI-impl.h"
 */
template <typename T>
class BuilderBase {
public:
    // none of these methods must be implemented inline because it's important that their
    // implementation be hidden from the public headers.
    template<typename ... ARGS>
    explicit BuilderBase(ARGS&& ...) noexcept;
    BuilderBase() noexcept;
    ~BuilderBase() noexcept;
    BuilderBase(BuilderBase const& rhs) noexcept;
    BuilderBase& operator = (BuilderBase const& rhs) noexcept;

    // move ctor and copy operator can be implemented inline and don't need to be exported
    BuilderBase(BuilderBase&& rhs) noexcept : mImpl(rhs.mImpl) { rhs.mImpl = nullptr; }
    BuilderBase& operator = (BuilderBase&& rhs) noexcept {
        auto temp = mImpl;
        mImpl = rhs.mImpl;
        rhs.mImpl = temp;
        return *this;
    }

protected:
    T* mImpl;
    inline T* operator->() noexcept { return mImpl; }
    inline T const* operator->() const noexcept { return mImpl; }
};

} // namespace filament

#endif // TNT_FILAMENT_FILAMENT_API_H

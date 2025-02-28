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

#ifndef TNT_FILAMENT_FILAMENTAPI_H
#define TNT_FILAMENT_FILAMENTAPI_H

#include <utils/compiler.h>
#include <utils/PrivateImplementation.h>
#include <utils/CString.h>
#include <utils/StaticString.h>

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
};

template<typename T>
using BuilderBase = utils::PrivateImplementation<T>;

// This needs to be public because it is used in the following template.
UTILS_PUBLIC void builderMakeName(utils::CString& outName, const char* name, size_t len) noexcept;

template <typename Builder>
class UTILS_PUBLIC BuilderNameMixin {
public:
    UTILS_DEPRECATED
    Builder& name(const char* name, size_t len) noexcept {
        builderMakeName(mName, name, len);
        return static_cast<Builder&>(*this);
    }

    Builder& name(utils::StaticString const& name) noexcept {
        builderMakeName(mName, name.data(), name.size());
        return static_cast<Builder&>(*this);
    }

    utils::CString const& getName() const noexcept { return mName; }

private:
    utils::CString mName;
};

} // namespace filament

#endif // TNT_FILAMENT_FILAMENTAPI_H

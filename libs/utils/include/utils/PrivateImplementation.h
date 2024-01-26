/*
 * Copyright (C) 2022 The Android Open Source Project
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

#ifndef UTILS_PRIVATEIMPLEMENTATION_H
#define UTILS_PRIVATEIMPLEMENTATION_H

#include <stddef.h>

namespace utils {

/**
 * \privatesection
 * PrivateImplementation is used to hide the implementation details of a class and ensure a higher
 * level of backward binary compatibility.
 * The actual implementation is in src/PrivateImplementation-impl.h"
 */
template<typename T>
class PrivateImplementation {
public:
    // none of these methods must be implemented inline because it's important that their
    // implementation be hidden from the public headers.
    template<typename ... ARGS>
    explicit PrivateImplementation(ARGS&& ...) noexcept;
    PrivateImplementation() noexcept;
    ~PrivateImplementation() noexcept;
    PrivateImplementation(PrivateImplementation const& rhs) noexcept;
    PrivateImplementation& operator = (PrivateImplementation const& rhs) noexcept;

    // move ctor and copy operator can be implemented inline and don't need to be exported
    PrivateImplementation(PrivateImplementation&& rhs) noexcept : mImpl(rhs.mImpl) { rhs.mImpl = nullptr; }
    PrivateImplementation& operator = (PrivateImplementation&& rhs) noexcept {
        auto temp = mImpl;
        mImpl = rhs.mImpl;
        rhs.mImpl = temp;
        return *this;
    }

protected:
    T* mImpl = nullptr;
    inline T* operator->() noexcept { return mImpl; }
    inline T const* operator->() const noexcept { return mImpl; }
};

} // namespace utils

#endif // UTILS_PRIVATEIMPLEMENTATION_H

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

#ifndef UTILS_PRIVATEIMPLEMENTATION_IMPL_H
#define UTILS_PRIVATEIMPLEMENTATION_IMPL_H

/*
 * This looks like a header file, but really it acts as a .cpp, because it's used to
 * explicitly instantiate the methods of PrivateImplementation<>
 */

#include <utils/PrivateImplementation.h>

#include <utility>

namespace utils {

template<typename T>
PrivateImplementation<T>::PrivateImplementation() noexcept
        : mImpl(new T) {
}

template<typename T>
template<typename ... ARGS>
PrivateImplementation<T>::PrivateImplementation(ARGS&& ... args) noexcept
        : mImpl(new T(std::forward<ARGS>(args)...)) {
}

template<typename T>
PrivateImplementation<T>::~PrivateImplementation() noexcept {
    delete mImpl;
}

#ifndef UTILS_PRIVATE_IMPLEMENTATION_NON_COPYABLE

template<typename T>
PrivateImplementation<T>::PrivateImplementation(PrivateImplementation const& rhs) noexcept
        : mImpl(new T(*rhs.mImpl)) {
}

template<typename T>
PrivateImplementation<T>& PrivateImplementation<T>::operator=(PrivateImplementation<T> const& rhs) noexcept {
    if (this != &rhs) {
        *mImpl = *rhs.mImpl;
    }
    return *this;
}

#endif

} // namespace utils

#endif // UTILS_PRIVATEIMPLEMENTATION_IMPL_H

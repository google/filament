/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef TNT_FILAMENT_FILAMENTAPI_IMPL_H
#define TNT_FILAMENT_FILAMENTAPI_IMPL_H

/*
 * This looks like a header file, but really it acts as a .cpp, because it's used to
 * explicitly instantiate the methods of BuilderBase<>
 */

#include <filament/FilamentAPI.h>

#include <utility>

namespace filament {

template<typename T>
BuilderBase<T>::BuilderBase() noexcept
        : mImpl(new T) {
}

template<typename T>
template<typename ... ARGS>
BuilderBase<T>::BuilderBase(ARGS&& ... args) noexcept
        : mImpl(new T(std::forward<ARGS>(args)...)) {
}

template<typename T>
BuilderBase<T>::~BuilderBase() noexcept {
    delete mImpl;
}

template<typename T>
BuilderBase<T>::BuilderBase(BuilderBase const& rhs) noexcept
        : mImpl(new T(*rhs.mImpl)) {
}

template<typename T>
BuilderBase<T>& BuilderBase<T>::operator=(BuilderBase<T> const& rhs) noexcept {
    *mImpl = *rhs.mImpl;
    return *this;
}

} // namespace filament

#endif // TNT_FILAMENT_FILAMENTAPI_IMPL_H

/*
 * Copyright (C) 2020 The Android Open Source Project
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

#include <iosfwd>
#include <math/mathfwd.h>

#if __has_attribute(visibility)
#    define MATHIO_PUBLIC __attribute__((visibility("default")))
#else
#    define MATHIO_PUBLIC
#endif

namespace filament::math::details {

template<typename T> class TQuaternion;

template<typename T>
MATHIO_PUBLIC
std::ostream& operator<<(std::ostream& out, const TVec2<T>& v) noexcept;

template<typename T>
MATHIO_PUBLIC
std::ostream& operator<<(std::ostream& out, const TVec3<T>& v) noexcept;

template<typename T>
MATHIO_PUBLIC
std::ostream& operator<<(std::ostream& out, const TVec4<T>& v) noexcept;

template<typename T>
MATHIO_PUBLIC
std::ostream& operator<<(std::ostream& out, const TMat22<T>& v) noexcept;

template<typename T>
MATHIO_PUBLIC
std::ostream& operator<<(std::ostream& out, const TMat33<T>& v) noexcept;

template<typename T>
MATHIO_PUBLIC
std::ostream& operator<<(std::ostream& out, const TMat44<T>& v) noexcept;

template<typename T>
MATHIO_PUBLIC
std::ostream& operator<<(std::ostream& out, const TQuaternion<T>& v) noexcept;

}  // namespace filament::math::details

/*
 * Copyright (C) 2021 The Android Open Source Project
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

#ifndef TNT_UTILS_COMPRESSED_PAIR_H
#define TNT_UTILS_COMPRESSED_PAIR_H

#include <type_traits>
#include <utility>

namespace utils {

template<typename T, bool>
struct dependent_type : public T {
};

template<typename T1, typename T2,
        std::enable_if_t<!std::is_same_v<T1, T2>, bool> = true>
struct compressed_pair : private T1, private T2 {

    template<bool Dummy = true, typename = std::enable_if_t<
            dependent_type<std::is_default_constructible<T1>, Dummy>::value &&
            dependent_type<std::is_default_constructible<T2>, Dummy>::value>>
    compressed_pair() : T1(), T2() {}

    template<typename U1, typename U2>
    compressed_pair(U1&& other1, U2&& other2)
            : T1(std::forward<U1>(other1)),
              T2(std::forward<U2>(other2)) {}

    T1& first() noexcept {
        return static_cast<T1&>(*this);
    }

    T2& second() noexcept {
        return static_cast<T2&>(*this);
    }

    T1 const& first() const noexcept {
        return static_cast<T1 const&>(*this);
    }

    T2 const& second() const noexcept {
        return static_cast<T2 const&>(*this);
    }

    void swap(compressed_pair& other) noexcept {
        using std::swap;
        swap(first(), other.first());
        swap(second(), other.second());
    }
};

} // namespace utils

#endif // TNT_UTILS_COMPRESSED_PAIR_H

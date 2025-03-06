// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_UTILS_CONTAINERS_REVERSE_H_
#define SRC_TINT_UTILS_CONTAINERS_REVERSE_H_

#include <iterator>

namespace tint {

namespace detail {
/// Used by tint::Reverse to hold the underlying iterable.
/// begin(ReverseIterable<T>) and end(ReverseIterable<T>) are automatically
/// called for range-for loops, via argument-dependent lookup.
/// See https://en.cppreference.com/w/cpp/language/range-for
template <typename T>
struct ReverseIterable {
    /// The wrapped iterable object.
    T& iterable;
};

template <typename T>
auto begin(ReverseIterable<T> r_it) {
    return std::rbegin(r_it.iterable);
}

template <typename T>
auto end(ReverseIterable<T> r_it) {
    return std::rend(r_it.iterable);
}
}  // namespace detail

/// Reverse returns an iterable wrapper that when used in range-for loops,
/// performs a reverse iteration over the object `iterable`.
/// Example:
/// ```
/// /* Equivalent to:
///  * for (auto it = vec.rbegin(); i != vec.rend(); ++it) {
///  *   auto v = *it;
///  */
/// for (auto v : tint::Reverse(vec)) {
/// }
/// ```
/// @param iterable the object to iterate
/// @returns the reverse iterable object
template <typename T>
detail::ReverseIterable<T> Reverse(T&& iterable) {
    return {iterable};
}

}  // namespace tint

#endif  // SRC_TINT_UTILS_CONTAINERS_REVERSE_H_

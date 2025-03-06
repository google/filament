// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_TINT_UTILS_CONTAINERS_PREDICATES_H_
#define SRC_TINT_UTILS_CONTAINERS_PREDICATES_H_

namespace tint {

/// @param value the value to compare against
/// @return a function with the signature `bool(const T&)` which returns true if the argument is
/// equal to
/// @p value
template <typename T>
auto Eq(const T& value) {
    return [value](const T& v) { return v == value; };
}

/// @param value the value to compare against
/// @return a function with the signature `bool(const T&)` which returns true if the argument is not
/// equal to @p value
template <typename T>
auto Ne(const T& value) {
    return [value](const T& v) { return v != value; };
}

/// @param value the value to compare against
/// @return a function with the signature `bool(const T&)` which returns true if the argument is
/// greater than @p value
template <typename T>
auto Gt(const T& value) {
    return [value](const T& v) { return v > value; };
}

/// @param value the value to compare against
/// @return a function with the signature `bool(const T&)` which returns true if the argument is
/// less than
/// @p value
template <typename T>
auto Lt(const T& value) {
    return [value](const T& v) { return v < value; };
}

/// @param value the value to compare against
/// @return a function with the signature `bool(const T&)` which returns true if the argument is
/// greater or equal to @p value
template <typename T>
auto Ge(const T& value) {
    return [value](const T& v) { return v >= value; };
}

/// @param value the value to compare against
/// @return a function with the signature `bool(const T&)` which returns true if the argument is
/// less than or equal to @p value
template <typename T>
auto Le(const T& value) {
    return [value](const T& v) { return v <= value; };
}

/// @param ptr the pointer
/// @return true if the pointer argument is null.
static inline bool IsNull(const void* ptr) {
    return ptr == nullptr;
}

}  // namespace tint

#endif  // SRC_TINT_UTILS_CONTAINERS_PREDICATES_H_

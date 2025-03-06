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

#ifndef SRC_TINT_UTILS_CONTAINERS_TRANSFORM_H_
#define SRC_TINT_UTILS_CONTAINERS_TRANSFORM_H_

#include <algorithm>
#include <type_traits>
#include <utility>
#include <vector>

#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/rtti/traits.h"

namespace tint {
//! @cond Doxygen_Suppress
// Doxygen gets confused by Transform()'s SFINAE

/// Transform performs an element-wise transformation of a vector.
/// @param in the input vector.
/// @param transform the transformation function with signature: `OUT(IN)`
/// @returns a new vector with each element of the source vector transformed by `transform`.
template <typename IN, typename TRANSFORMER>
auto Transform(const std::vector<IN>& in, TRANSFORMER&& transform)
    -> std::vector<decltype(transform(in[0]))> {
    std::vector<decltype(transform(in[0]))> result(in.size());
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] = transform(in[i]);
    }
    return result;
}

/// Transform performs an element-wise transformation of a vector.
/// @param in the input vector.
/// @param transform the transformation function with signature: `OUT(IN, size_t)`
/// @returns a new vector with each element of the source vector transformed by `transform`.
template <typename IN, typename TRANSFORMER>
auto Transform(const std::vector<IN>& in, TRANSFORMER&& transform)
    -> std::vector<decltype(transform(in[0], 1u))> {
    std::vector<decltype(transform(in[0], 1u))> result(in.size());
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] = transform(in[i], i);
    }
    return result;
}

/// Transform performs an element-wise transformation of a vector.
/// @param in the input vector.
/// @param transform the transformation function with signature: `OUT(IN)`
/// @returns a new vector with each element of the source vector transformed by `transform`.
template <typename IN, size_t N, typename TRANSFORMER>
auto Transform(const Vector<IN, N>& in, TRANSFORMER&& transform)
    -> Vector<decltype(transform(in[0])), N> {
    const auto count = in.Length();
    Vector<decltype(transform(in[0])), N> result;
    result.Reserve(count);
    for (size_t i = 0; i < count; ++i) {
        result.Push(transform(in[i]));
    }
    return result;
}

/// Transform performs an element-wise transformation of a vector.
/// @param in the input vector.
/// @param transform the transformation function with signature: `OUT(IN, size_t)`
/// @returns a new vector with each element of the source vector transformed by `transform`.
template <typename IN, size_t N, typename TRANSFORMER>
auto Transform(const Vector<IN, N>& in, TRANSFORMER&& transform)
    -> Vector<decltype(transform(in[0], 1u)), N> {
    const auto count = in.Length();
    Vector<decltype(transform(in[0], 1u)), N> result;
    result.Reserve(count);
    for (size_t i = 0; i < count; ++i) {
        result.Push(transform(in[i], i));
    }
    return result;
}

/// Transform performs an element-wise transformation of a slice.
/// @param in the input slice.
/// @param transform the transformation function with signature: `OUT(IN)`
/// @tparam N the small-array size of the returned Vector
/// @returns a new vector with each element of the source vector transformed by `transform`.
template <size_t N, typename IN, typename TRANSFORMER>
auto Transform(Slice<IN> in, TRANSFORMER&& transform) -> Vector<decltype(transform(in[0])), N> {
    Vector<decltype(transform(in[0])), N> result;
    result.Reserve(in.len);
    for (size_t i = 0; i < in.len; ++i) {
        result.Push(transform(in[i]));
    }
    return result;
}

/// Transform performs an element-wise transformation of a slice.
/// @param in the input slice.
/// @param transform the transformation function with signature: `OUT(IN, size_t)`
/// @tparam N the small-array size of the returned Vector
/// @returns a new vector with each element of the source vector transformed by `transform`.
template <size_t N, typename IN, typename TRANSFORMER>
auto Transform(Slice<IN> in, TRANSFORMER&& transform) -> Vector<decltype(transform(in[0], 1u)), N> {
    Vector<decltype(transform(in[0], 1u)), N> result;
    result.Reserve(in.len);
    for (size_t i = 0; i < in.len; ++i) {
        result.Push(transform(in[i], i));
    }
    return result;
}

/// Transform performs an element-wise transformation of a vector reference.
/// @param in the input vector.
/// @param transform the transformation function with signature: `OUT(IN)`
/// @tparam N the small-array size of the returned Vector
/// @returns a new vector with each element of the source vector transformed by `transform`.
template <size_t N, typename IN, typename TRANSFORMER>
auto Transform(VectorRef<IN> in, TRANSFORMER&& transform) -> Vector<decltype(transform(in[0])), N> {
    return Transform<N>(in.Slice(), std::forward<TRANSFORMER>(transform));
}

/// Transform performs an element-wise transformation of a vector reference.
/// @param in the input vector.
/// @param transform the transformation function with signature: `OUT(IN, size_t)`
/// @tparam N the small-array size of the returned Vector
/// @returns a new vector with each element of the source vector transformed by `transform`.
template <size_t N, typename IN, typename TRANSFORMER>
auto Transform(VectorRef<IN> in, TRANSFORMER&& transform)
    -> Vector<decltype(transform(in[0], 1u)), N> {
    return Transform<N>(in.Slice(), std::forward<TRANSFORMER>(transform));
}

/// TransformN performs an element-wise transformation of a vector, transforming and returning at
/// most `n` elements.
/// @param in the input vector.
/// @param n the maximum number of elements to transform.
/// @param transform the transformation function with signature: `OUT(IN)`
/// @returns a new vector with at most n-elements of the source vector transformed by `transform`.
template <typename IN, typename TRANSFORMER>
auto TransformN(const std::vector<IN>& in, size_t n, TRANSFORMER&& transform)
    -> std::vector<decltype(transform(in[0]))> {
    const auto count = std::min(n, in.size());
    std::vector<decltype(transform(in[0]))> result(count);
    for (size_t i = 0; i < count; ++i) {
        result[i] = transform(in[i]);
    }
    return result;
}

/// TransformN performs an element-wise transformation of a vector, transforming and returning at
/// most `n` elements.
/// @param in the input vector.
/// @param n the maximum number of elements to transform.
/// @param transform the transformation function with signature: `OUT(IN, size_t)`
/// @returns a new vector with at most n-elements of the source vector transformed by `transform`.
template <typename IN, typename TRANSFORMER>
auto TransformN(const std::vector<IN>& in, size_t n, TRANSFORMER&& transform)
    -> std::vector<decltype(transform(in[0], 1u))> {
    const auto count = std::min(n, in.size());
    std::vector<decltype(transform(in[0], 1u))> result(count);
    for (size_t i = 0; i < count; ++i) {
        result[i] = transform(in[i], i);
    }
    return result;
}

}  // namespace tint

//! @endcond
#endif  // SRC_TINT_UTILS_CONTAINERS_TRANSFORM_H_

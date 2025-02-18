// Copyright 2024 The langsvr Authors
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

#ifndef LANGSVR_SPAN_H_
#define LANGSVR_SPAN_H_

#include <array>
#include <cstddef>
#include <vector>

namespace langsvr {

/// Span represents an immutable view over a sequence of objects of type `T`.
template <typename T>
class Span {
  public:
    /// Constructor
    /// @param first pointer to the first element of the span
    /// @param count number of elements in the span
    Span(T* first, size_t count) : elements_(first), count_(count) {}

    /// Constructor
    /// @param vec vector to create the span from
    Span(const std::vector<T>& vec) : elements_(vec.data()), count_(vec.size()) {}

    /// Constructor
    /// @param arr array to create the span from
    template <size_t N>
    Span(const std::array<T, N>& arr) : elements_(arr.data()), count_(arr.size()) {}

    /// @returns the first element in the span
    const T& front() const { return elements_[0]; }
    /// @returns the last element in the span
    const T& back() const { return elements_[count_ - 1]; }
    /// @returns the @p i 'th element in the span
    const T& operator[](size_t i) const { return elements_[i]; }
    /// @returns a pointer to the first element in the span
    const T* begin() const { return &elements_[0]; }
    /// @returns a pointer to one beyond the last element in the span
    const T* end() const { return &elements_[count_]; }
    /// @returns the number of elements in the span
    size_t size() const { return count_; }

  private:
    const T* const elements_;
    const size_t count_;
};

}  // namespace langsvr

#endif  // LANGSVR_SPAN_H_

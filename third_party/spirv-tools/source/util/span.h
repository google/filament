// Copyright 2025 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SOURCE_UTIL_SPAN_H_
#define SOURCE_UTIL_SPAN_H_

#include <cstddef>
#include <iterator>
#include <type_traits>

namespace spvtools {
namespace utils {

// Implement a subset of the C++20 std::span, using at most C++17 functionality.
// Replace this when SPIRV-Tools can use C++20.
template <class T>
class Span {
 public:
  using element_type = T;
  using value_type = std::remove_cv_t<T>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;
  using iterator = T*;
  using const_iterator = const T*;

  Span() {}
  Span(iterator first, size_type count) : first_(first), count_(count) {}

  iterator begin() const { return first_; }
  iterator end() const { return first_ ? first_ + count_ : nullptr; }
  const_iterator cbegin() const { return first_; }
  const_iterator cend() const { return first_ ? first_ + count_ : nullptr; }

  size_type size() const { return count_; }
  size_type size_bytes() const { return count_ * sizeof(T); }
  bool empty() const { return first_ == nullptr || count_ == 0; }

  reference front() const { return *first_; }
  reference back() const { return *(first_ + count_ - 1); }
  pointer data() const { return first_; }
  reference operator[](size_type idx) const { return first_[idx]; }
  Span<T> subspan(size_type offset) const {
    if (count_ > offset) {
      return Span(first_ + offset, count_ - offset);
    }
    return Span<T>();
  }

 private:
  T* first_ = nullptr;
  size_type count_ = 0;
};

}  // namespace utils
}  // namespace spvtools

#endif  // SOURCE_UTIL_SPAN_H_

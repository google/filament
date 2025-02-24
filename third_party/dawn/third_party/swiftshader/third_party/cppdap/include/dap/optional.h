// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef dap_optional_h
#define dap_optional_h

#include <assert.h>
#include <type_traits>
#include <utility>  // std::move, std::forward

namespace dap {

// optional holds an 'optional' contained value.
// This is similar to C++17's std::optional.
template <typename T>
class optional {
  template <typename U>
  using IsConvertibleToT =
      typename std::enable_if<std::is_convertible<U, T>::value>::type;

 public:
  using value_type = T;

  // constructors
  inline optional() = default;
  inline optional(const optional& other);
  inline optional(optional&& other);
  template <typename U>
  inline optional(const optional<U>& other);
  template <typename U>
  inline optional(optional<U>&& other);
  template <typename U = value_type, typename = IsConvertibleToT<U>>
  inline optional(U&& value);

  // value() returns the contained value.
  // If the optional does not contain a value, then value() will assert.
  inline T& value();
  inline const T& value() const;

  // value() returns the contained value, or defaultValue if the optional does
  // not contain a value.
  inline const T& value(const T& defaultValue) const;

  // operator bool() returns true if the optional contains a value.
  inline explicit operator bool() const noexcept;

  // has_value() returns true if the optional contains a value.
  inline bool has_value() const;

  // assignment
  inline optional& operator=(const optional& other);
  inline optional& operator=(optional&& other) noexcept;
  template <typename U = T, typename = IsConvertibleToT<U>>
  inline optional& operator=(U&& value);
  template <typename U>
  inline optional& operator=(const optional<U>& other);
  template <typename U>
  inline optional& operator=(optional<U>&& other);

  // value access
  inline const T* operator->() const;
  inline T* operator->();
  inline const T& operator*() const;
  inline T& operator*();

 private:
  T val = {};
  bool set = false;
};

template <typename T>
optional<T>::optional(const optional& other) : val(other.val), set(other.set) {}

template <typename T>
optional<T>::optional(optional&& other)
    : val(std::move(other.val)), set(other.set) {}

template <typename T>
template <typename U>
optional<T>::optional(const optional<U>& other) : set(other.has_value()) {
  if (set) {
    val = static_cast<T>(other.value());
  }
}

template <typename T>
template <typename U>
optional<T>::optional(optional<U>&& other) : set(other.has_value()) {
  if (set) {
    val = static_cast<T>(std::move(other.value()));
  }
}

template <typename T>
template <typename U /*= T*/, typename>
optional<T>::optional(U&& value) : val(std::forward<U>(value)), set(true) {}

template <typename T>
T& optional<T>::value() {
  assert(set);
  return val;
}

template <typename T>
const T& optional<T>::value() const {
  assert(set);
  return val;
}

template <typename T>
const T& optional<T>::value(const T& defaultValue) const {
  if (!has_value()) {
    return defaultValue;
  }
  return val;
}

template <typename T>
optional<T>::operator bool() const noexcept {
  return set;
}

template <typename T>
bool optional<T>::has_value() const {
  return set;
}

template <typename T>
optional<T>& optional<T>::operator=(const optional& other) {
  val = other.val;
  set = other.set;
  return *this;
}

template <typename T>
optional<T>& optional<T>::operator=(optional&& other) noexcept {
  val = std::move(other.val);
  set = other.set;
  return *this;
}

template <typename T>
template <typename U /* = T */, typename>
optional<T>& optional<T>::operator=(U&& value) {
  val = std::forward<U>(value);
  set = true;
  return *this;
}

template <typename T>
template <typename U>
optional<T>& optional<T>::operator=(const optional<U>& other) {
  val = other.val;
  set = other.set;
  return *this;
}

template <typename T>
template <typename U>
optional<T>& optional<T>::operator=(optional<U>&& other) {
  val = std::move(other.val);
  set = other.set;
  return *this;
}

template <typename T>
const T* optional<T>::operator->() const {
  assert(set);
  return &val;
}

template <typename T>
T* optional<T>::operator->() {
  assert(set);
  return &val;
}

template <typename T>
const T& optional<T>::operator*() const {
  assert(set);
  return val;
}

template <typename T>
T& optional<T>::operator*() {
  assert(set);
  return val;
}

template <class T, class U>
inline bool operator==(const optional<T>& lhs, const optional<U>& rhs) {
  if (!lhs.has_value() && !rhs.has_value()) {
    return true;
  }
  if (!lhs.has_value() || !rhs.has_value()) {
    return false;
  }
  return lhs.value() == rhs.value();
}

template <class T, class U>
inline bool operator!=(const optional<T>& lhs, const optional<U>& rhs) {
  return !(lhs == rhs);
}

template <class T, class U>
inline bool operator<(const optional<T>& lhs, const optional<U>& rhs) {
  if (!rhs.has_value()) {
    return false;
  }
  if (!lhs.has_value()) {
    return true;
  }
  return lhs.value() < rhs.value();
}

template <class T, class U>
inline bool operator<=(const optional<T>& lhs, const optional<U>& rhs) {
  if (!lhs.has_value()) {
    return true;
  }
  if (!rhs.has_value()) {
    return false;
  }
  return lhs.value() <= rhs.value();
}

template <class T, class U>
inline bool operator>(const optional<T>& lhs, const optional<U>& rhs) {
  if (!lhs.has_value()) {
    return false;
  }
  if (!rhs.has_value()) {
    return true;
  }
  return lhs.value() > rhs.value();
}

template <class T, class U>
inline bool operator>=(const optional<T>& lhs, const optional<U>& rhs) {
  if (!rhs.has_value()) {
    return true;
  }
  if (!lhs.has_value()) {
    return false;
  }
  return lhs.value() >= rhs.value();
}

}  // namespace dap

#endif  // dap_optional_h

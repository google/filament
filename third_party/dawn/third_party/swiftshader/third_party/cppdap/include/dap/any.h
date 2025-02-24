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

#ifndef dap_any_h
#define dap_any_h

#include "typeinfo.h"

#include <assert.h>

namespace dap {

template <typename T>
struct TypeOf;

// any provides a type-safe container for values of any of dap type (boolean,
// integer, number, array, variant, any, null, dap-structs).
class any {
 public:
  // constructors
  inline any() = default;
  inline any(const any& other) noexcept;
  inline any(any&& other) noexcept;

  template <typename T>
  inline any(const T& val);

  // destructors
  inline ~any();

  // replaces the contained value with a null.
  inline void reset();

  // assignment
  inline any& operator=(const any& rhs);
  inline any& operator=(any&& rhs) noexcept;
  template <typename T>
  inline any& operator=(const T& val);

  // get() returns the contained value of the type T.
  // If the any does not contain a value of type T, then get() will assert.
  template <typename T>
  inline T& get() const;

  // is() returns true iff the contained value is of type T.
  template <typename T>
  inline bool is() const;

 private:
  static inline void* alignUp(void* val, size_t alignment);
  inline void alloc(size_t size, size_t align);
  inline void free();
  inline bool isInBuffer(void* ptr) const;

  void* value = nullptr;
  const TypeInfo* type = nullptr;
  void* heap = nullptr;  // heap allocation
  uint8_t buffer[32];    // or internal allocation
};

inline any::~any() {
  reset();
}

template <typename T>
inline any::any(const T& val) {
  *this = val;
}

any::any(const any& other) noexcept : type(other.type) {
  if (other.value != nullptr) {
    alloc(type->size(), type->alignment());
    type->copyConstruct(value, other.value);
  }
}

any::any(any&& other) noexcept : type(other.type) {
  if (other.isInBuffer(other.value)) {
    alloc(type->size(), type->alignment());
    type->copyConstruct(value, other.value);
  } else {
    value = other.value;
  }
  other.value = nullptr;
  other.type = nullptr;
}

void any::reset() {
  if (value != nullptr) {
    type->destruct(value);
    free();
  }
  value = nullptr;
  type = nullptr;
}

any& any::operator=(const any& rhs) {
  reset();
  type = rhs.type;
  if (rhs.value != nullptr) {
    alloc(type->size(), type->alignment());
    type->copyConstruct(value, rhs.value);
  }
  return *this;
}

any& any::operator=(any&& rhs) noexcept {
  reset();
  type = rhs.type;
  if (rhs.isInBuffer(rhs.value)) {
    alloc(type->size(), type->alignment());
    type->copyConstruct(value, rhs.value);
  } else {
    value = rhs.value;
  }
  rhs.value = nullptr;
  rhs.type = nullptr;
  return *this;
}

template <typename T>
any& any::operator=(const T& val) {
  if (!is<T>()) {
    reset();
    type = TypeOf<T>::type();
    alloc(type->size(), type->alignment());
    type->copyConstruct(value, &val);
  } else {
    *reinterpret_cast<T*>(value) = val;
  }
  return *this;
}

template <typename T>
T& any::get() const {
  assert(is<T>());
  return *reinterpret_cast<T*>(value);
}

template <typename T>
bool any::is() const {
  return type == TypeOf<T>::type();
}

template <>
inline bool any::is<std::nullptr_t>() const {
  return value == nullptr;
}

void* any::alignUp(void* val, size_t alignment) {
  auto ptr = reinterpret_cast<uintptr_t>(val);
  return reinterpret_cast<void*>(alignment *
                                 ((ptr + alignment - 1) / alignment));
}

void any::alloc(size_t size, size_t align) {
  assert(value == nullptr);
  value = alignUp(buffer, align);
  if (isInBuffer(reinterpret_cast<uint8_t*>(value) + size - 1)) {
    return;
  }
  heap = new uint8_t[size + align];
  value = alignUp(heap, align);
}

void any::free() {
  assert(value != nullptr);
  if (heap != nullptr) {
    delete[] reinterpret_cast<uint8_t*>(heap);
    heap = nullptr;
  }
  value = nullptr;
}

bool any::isInBuffer(void* ptr) const {
  auto addr = reinterpret_cast<uintptr_t>(ptr);
  return addr >= reinterpret_cast<uintptr_t>(buffer) &&
         addr < reinterpret_cast<uintptr_t>(buffer + sizeof(buffer));
}

}  // namespace dap

#endif  // dap_any_h

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

#ifndef dap_variant_h
#define dap_variant_h

#include "any.h"

namespace dap {

// internal functionality
namespace detail {
template <typename T, typename...>
struct TypeIsIn {
  static constexpr bool value = false;
};

template <typename T, typename List0, typename... ListN>
struct TypeIsIn<T, List0, ListN...> {
  static constexpr bool value =
      std::is_same<T, List0>::value || TypeIsIn<T, ListN...>::value;
};
}  // namespace detail

// variant represents a type-safe union of DAP types.
// variant can hold a value of any of the template argument types.
// variant defaults to a default-constructed T0.
template <typename T0, typename... Types>
class variant {
 public:
  // constructors
  inline variant();
  template <typename T>
  inline variant(const T& val);

  // assignment
  template <typename T>
  inline variant& operator=(const T& val);

  // get() returns the contained value of the type T.
  // If the any does not contain a value of type T, then get() will assert.
  template <typename T>
  inline T& get() const;

  // is() returns true iff the contained value is of type T.
  template <typename T>
  inline bool is() const;

  // accepts() returns true iff the variant accepts values of type T.
  template <typename T>
  static constexpr bool accepts();

 private:
  friend class Serializer;
  friend class Deserializer;
  any value;
};

template <typename T0, typename... Types>
variant<T0, Types...>::variant() : value(T0()) {}

template <typename T0, typename... Types>
template <typename T>
variant<T0, Types...>::variant(const T& v) : value(v) {
  static_assert(accepts<T>(), "variant does not accept template type T");
}

template <typename T0, typename... Types>
template <typename T>
variant<T0, Types...>& variant<T0, Types...>::operator=(const T& v) {
  static_assert(accepts<T>(), "variant does not accept template type T");
  value = v;
  return *this;
}

template <typename T0, typename... Types>
template <typename T>
T& variant<T0, Types...>::get() const {
  static_assert(accepts<T>(), "variant does not accept template type T");
  return value.get<T>();
}

template <typename T0, typename... Types>
template <typename T>
bool variant<T0, Types...>::is() const {
  return value.is<T>();
}

template <typename T0, typename... Types>
template <typename T>
constexpr bool variant<T0, Types...>::accepts() {
  return detail::TypeIsIn<T, T0, Types...>::value;
}

}  // namespace dap

#endif  // dap_variant_h

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

// This file holds the basic serializable types used by the debug adapter
// protocol.

#ifndef dap_types_h
#define dap_types_h

#include "any.h"
#include "optional.h"
#include "variant.h"

#include <unordered_map>
#include <vector>

#include <stdint.h>

namespace dap {

// string is a sequence of characters.
// string defaults to an empty string.
using string = std::string;

// boolean holds a true or false value.
// boolean defaults to false.
class boolean {
 public:
  inline boolean() : val(false) {}
  inline boolean(bool i) : val(i) {}
  inline operator bool() const { return val; }
  inline boolean& operator=(bool i) {
    val = i;
    return *this;
  }

 private:
  bool val;
};

// integer holds a whole signed number.
// integer defaults to 0.
class integer {
 public:
  inline integer() : val(0) {}
  inline integer(int64_t i) : val(i) {}
  inline operator int64_t() const { return val; }
  inline integer& operator=(int64_t i) {
    val = i;
    return *this;
  }
  inline integer operator++(int) {
    auto copy = *this;
    val++;
    return copy;
  }

 private:
  int64_t val;
};

// number holds a 64-bit floating point number.
// number defaults to 0.
class number {
 public:
  inline number() : val(0.0) {}
  inline number(double i) : val(i) {}
  inline operator double() const { return val; }
  inline number& operator=(double i) {
    val = i;
    return *this;
  }

 private:
  double val;
};

// array is a list of items of type T.
// array defaults to an empty list.
template <typename T>
using array = std::vector<T>;

// object is a map of string to any.
// object defaults to an empty map.
using object = std::unordered_map<string, any>;

// null represents no value.
// null is used by any to check for no-value.
using null = std::nullptr_t;

}  // namespace dap

#endif  // dap_types_h

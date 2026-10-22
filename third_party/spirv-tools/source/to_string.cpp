// Copyright (c) 2024 Google LLC
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

#include "source/to_string.h"

#include <cassert>

namespace spvtools {

std::string to_string(uint32_t n) {
  // This implementation avoids using standard library features that access
  // the locale.  Using the locale requires taking a mutex which causes
  // annoying serialization.

  constexpr int max_digits = 10;  // max uint has 10 digits
  // Contains the resulting digits, with least significant digit in the last
  // entry.
  char buf[max_digits];
  int write_index = max_digits - 1;
  if (n == 0) {
    buf[write_index] = '0';
  } else {
    while (n > 0) {
      int units = n % 10;
      buf[write_index--] = "0123456789"[units];
      n = (n - units) / 10;
    }
    write_index++;
  }
  assert(write_index >= 0);
  return std::string(buf + write_index, max_digits - write_index);
}
}  // namespace spvtools

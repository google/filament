// Copyright 2017 Google Inc. All rights reserved.
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

#include "src/utf8_fix.h"

#include <algorithm>
#include <cassert>

namespace protobuf_mutator {

namespace {

void StoreCode(char* e, char32_t code, uint8_t size, uint8_t prefix) {
  while (--size) {
    *(--e) = 0x80 | (code & 0x3F);
    code >>= 6;
  }
  *(--e) = prefix | code;
}

char* FixCode(char* b, const char* e, RandomEngine* random) {
  const char* start = b;
  assert(b < e);

  e = std::min<const char*>(e, b + 4);
  char32_t c = *b++;
  for (; b < e && (*b & 0xC0) == 0x80; ++b) {
    c = (c << 6) + (*b & 0x3F);
  }
  uint8_t size = b - start;
  switch (size) {
    case 1:
      c &= 0x7F;
      StoreCode(b, c, size, 0);
      break;
    case 2:
      c &= 0x7FF;
      if (c < 0x80) {
        // Use uint32_t because uniform_int_distribution does not support
        // char32_t on Windows.
        c = std::uniform_int_distribution<uint32_t>(0x80, 0x7FF)(*random);
      }
      StoreCode(b, c, size, 0xC0);
      break;
    case 3:
      c &= 0xFFFF;

      // [0xD800, 0xE000) are reserved for UTF-16 surrogate halves.
      if (c < 0x800 || (c >= 0xD800 && c < 0xE000)) {
        uint32_t halves = 0xE000 - 0xD800;
        c = std::uniform_int_distribution<uint32_t>(0x800,
                                                    0xFFFF - halves)(*random);
        if (c >= 0xD800) c += halves;
      }
      StoreCode(b, c, size, 0xE0);
      break;
    case 4:
      c &= 0x1FFFFF;
      if (c < 0x10000 || c > 0x10FFFF) {
        c = std::uniform_int_distribution<uint32_t>(0x10000, 0x10FFFF)(*random);
      }
      StoreCode(b, c, size, 0xF0);
      break;
    default:
      assert(false && "Unexpected size of UTF-8 sequence");
  }
  return b;
}

}  // namespace

void FixUtf8String(std::string* str, RandomEngine* random) {
  if (str->empty()) return;
  char* b = &(*str)[0];
  const char* e = b + str->size();
  while (b < e) {
    b = FixCode(b, e, random);
  }
}

}  // namespace protobuf_mutator

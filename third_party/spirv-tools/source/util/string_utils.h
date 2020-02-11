// Copyright (c) 2017 Google Inc.
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

#ifndef SOURCE_UTIL_STRING_UTILS_H_
#define SOURCE_UTIL_STRING_UTILS_H_

#include <assert.h>
#include <sstream>
#include <string>
#include <vector>

#include "source/util/string_utils.h"

namespace spvtools {
namespace utils {

// Converts arithmetic value |val| to its default string representation.
template <class T>
std::string ToString(T val) {
  static_assert(
      std::is_arithmetic<T>::value,
      "spvtools::utils::ToString is restricted to only arithmetic values");
  std::stringstream os;
  os << val;
  return os.str();
}

// Converts cardinal number to ordinal number string.
std::string CardinalToOrdinal(size_t cardinal);

// Splits the string |flag|, of the form '--pass_name[=pass_args]' into two
// strings "pass_name" and "pass_args".  If |flag| has no arguments, the second
// string will be empty.
std::pair<std::string, std::string> SplitFlagArgs(const std::string& flag);

// Encodes a string as a sequence of words, using the SPIR-V encoding.
inline std::vector<uint32_t> MakeVector(std::string input) {
  std::vector<uint32_t> result;
  uint32_t word = 0;
  size_t num_bytes = input.size();
  // SPIR-V strings are null-terminated.  The byte_index == num_bytes
  // case is used to push the terminating null byte.
  for (size_t byte_index = 0; byte_index <= num_bytes; byte_index++) {
    const auto new_byte =
        (byte_index < num_bytes ? uint8_t(input[byte_index]) : uint8_t(0));
    word |= (new_byte << (8 * (byte_index % sizeof(uint32_t))));
    if (3 == (byte_index % sizeof(uint32_t))) {
      result.push_back(word);
      word = 0;
    }
  }
  // Emit a trailing partial word.
  if ((num_bytes + 1) % sizeof(uint32_t)) {
    result.push_back(word);
  }
  return result;
}

// Decode a string from a sequence of words, using the SPIR-V encoding.
template <class VectorType>
inline std::string MakeString(const VectorType& words) {
  std::string result;

  for (uint32_t word : words) {
    for (int byte_index = 0; byte_index < 4; byte_index++) {
      uint32_t extracted_word = (word >> (8 * byte_index)) & 0xFF;
      char c = static_cast<char>(extracted_word);
      if (c == 0) {
        return result;
      }
      result += c;
    }
  }
  assert(false && "Did not find terminating null for the string.");
  return result;
}  // namespace utils

}  // namespace utils
}  // namespace spvtools

#endif  // SOURCE_UTIL_STRING_UTILS_H_

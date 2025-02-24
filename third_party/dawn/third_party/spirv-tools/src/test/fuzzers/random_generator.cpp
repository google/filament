// Copyright (c) 2021 Google Inc.
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

#include "test/fuzzers/random_generator.h"

#include <algorithm>
#include <array>
#include <cassert>

namespace spvtools {
namespace fuzzers {

namespace {
/// Generate integer from uniform distribution
/// @tparam I - integer type
/// @param engine - random number engine to use
/// @param lower - Lower bound of integer generated
/// @param upper - Upper bound of integer generated
/// @returns i, where lower <= i < upper
template <typename I>
I RandomUInt(std::mt19937_64* engine, I lower, I upper) {
  assert(lower < upper && "|lower| must be stictly less than |upper|");
  return std::uniform_int_distribution<I>(lower, upper - 1)(*engine);
}

/// Helper for obtaining a seed bias value for HashCombine with a bit-width
/// dependent on the size of size_t.
template <int SIZE_OF_SIZE_T>
struct HashCombineOffset {};
/// Specialization of HashCombineOffset for size_t == 4.
template <>
struct HashCombineOffset<4> {
  /// @returns the seed bias value for HashCombine()
  static constexpr inline uint32_t value() {
    return 0x9e3779b9;  // Fractional portion of Golden Ratio, suggested by
                        // Linux Kernel and Knuth's Art of Computer Programming
  }
};
/// Specialization of HashCombineOffset for size_t == 8.
template <>
struct HashCombineOffset<8> {
  /// @returns the seed bias value for HashCombine()
  static constexpr inline uint64_t value() {
    return 0x9e3779b97f4a7c16;  // Fractional portion of Golden Ratio, suggested
                                // by Linux Kernel and Knuth's Art of Computer
                                // Programming
  }
};

/// HashCombine "hashes" together an existing hash and hashable values.
template <typename T>
void HashCombine(size_t* hash, const T& value) {
  constexpr size_t offset = HashCombineOffset<sizeof(size_t)>::value();
  *hash ^= std::hash<T>()(value) + offset + (*hash << 6) + (*hash >> 2);
}

/// Calculate the hash for the contents of a C-style data buffer
/// @param data - pointer to buffer to be hashed
/// @param size - number of elements in buffer
/// @returns hash of the data in the buffer
size_t HashBuffer(const uint8_t* data, const size_t size) {
  size_t hash =
      static_cast<size_t>(0xCA8945571519E991);  // seed with an arbitrary prime
  HashCombine(&hash, size);
  for (size_t i = 0; i < size; i++) {
    HashCombine(&hash, data[i]);
  }
  return hash;
}

}  // namespace

RandomGenerator::RandomGenerator(uint64_t seed) : engine_(seed) {}

RandomGenerator::RandomGenerator(const uint8_t* data, size_t size) {
  RandomGenerator(RandomGenerator::CalculateSeed(data, size));
}

spv_target_env RandomGenerator::GetTargetEnv() {
  spv_target_env result;

  // Need to check that the generated value isn't for a deprecated target env.
  do {
    result = static_cast<spv_target_env>(
        RandomUInt(&engine_, 0u, static_cast<unsigned int>(SPV_ENV_MAX)));
  } while (!spvIsValidEnv(result));

  return result;
}

uint32_t RandomGenerator::GetUInt32(uint32_t lower, uint32_t upper) {
  return RandomUInt(&engine_, lower, upper);
}

uint32_t RandomGenerator::GetUInt32(uint32_t bound) {
  assert(bound > 0 && "|bound| must be greater than 0");
  return RandomUInt(&engine_, 0u, bound);
}

uint64_t RandomGenerator::CalculateSeed(const uint8_t* data, size_t size) {
  assert(data != nullptr && "|data| must be !nullptr");

  // Number of bytes we want to skip at the start of data for the hash.
  // Fewer bytes may be skipped when `size` is small.
  // Has lower precedence than kHashDesiredMinBytes.
  static const int64_t kHashDesiredLeadingSkipBytes = 5;
  // Minimum number of bytes we want to use in the hash.
  // Used for short buffers.
  static const int64_t kHashDesiredMinBytes = 4;
  // Maximum number of bytes we want to use in the hash.
  static const int64_t kHashDesiredMaxBytes = 32;
  int64_t size_i64 = static_cast<int64_t>(size);
  int64_t hash_begin_i64 =
      std::min(kHashDesiredLeadingSkipBytes,
               std::max<int64_t>(size_i64 - kHashDesiredMinBytes, 0));
  int64_t hash_end_i64 =
      std::min(hash_begin_i64 + kHashDesiredMaxBytes, size_i64);
  size_t hash_begin = static_cast<size_t>(hash_begin_i64);
  size_t hash_size = static_cast<size_t>(hash_end_i64) - hash_begin;
  return HashBuffer(data + hash_begin, hash_size);
}

}  // namespace fuzzers
}  // namespace spvtools

// Copyright 2018 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_COMMON_HASHUTILS_H_
#define SRC_DAWN_COMMON_HASHUTILS_H_

#include <bitset>
#include <functional>

#include "dawn/common/Platform.h"
#include "dawn/common/TypedInteger.h"
#include "dawn/common/ityp_bitset.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn {

// Wrapper around std::hash to make it a templated function instead of a functor. It is marginally
// nicer, and avoids adding to the std namespace to add hashing of other types.
template <typename T>
size_t Hash(const T& value) {
    return std::hash<T>()(value);
}

template <typename T, partition_alloc::internal::RawPtrTraits Traits>
size_t Hash(const raw_ptr<T, Traits>& value) {
    return Hash(value.get());
}

// Add hashing of TypedIntegers
template <typename Tag, typename T>
size_t Hash(const TypedInteger<Tag, T>& value) {
    return Hash(static_cast<T>(value));
}

// When hashing sparse structures we want to iteratively build a hash value with only parts of the
// data. HashCombine "hashes" together an existing hash and hashable values.
//
// Example usage to compute the hash of a mask and values corresponding to the mask:
//
//    size_t hash = Hash(mask):
//    for (uint32_t i : IterateBitSet(mask)) { HashCombine(&hash, hashables[i]); }
//    return hash;
template <typename T>
void HashCombine(size_t* hash, const T& value) {
#if DAWN_PLATFORM_IS(64_BIT)
    const size_t offset = 0x9e3779b97f4a7c16;
#elif DAWN_PLATFORM_IS(32_BIT)
    const size_t offset = 0x9e3779b9;
#else
#error "Unsupported platform"
#endif
    *hash ^= Hash(value) + offset + (*hash << 6) + (*hash >> 2);
}

template <typename T, typename... Args>
void HashCombine(size_t* hash, const T& value, const Args&... args) {
    HashCombine(hash, value);
    HashCombine(hash, args...);
}

// Workaround a bug between clang++ and libstdlibc++ by defining our own hashing for bitsets.
// When _GLIBCXX_DEBUG is enabled libstdc++ wraps containers into debug containers. For bitset this
// means what is normally std::bitset is defined as std::__cxx1988::bitset and is replaced by the
// debug version of bitset.
// When hashing, std::hash<std::bitset> proxies the call to std::hash<std::__cxx1998::bitset> and
// fails on clang because the latter tries to access the private _M_getdata member of the bitset.
// It looks like it should work because the non-debug bitset declares
//
//     friend struct std::hash<bitset> // bitset is the name of the class itself
//
// which should friend std::hash<std::__cxx1998::bitset> but somehow doesn't work on clang.
#if defined(_GLIBCXX_DEBUG)
template <size_t N>
size_t Hash(const std::bitset<N>& value) {
    constexpr size_t kWindowSize = sizeof(uint64_t);

    std::bitset<N> bits = value;
    size_t hash = 0;
    for (size_t processedBits = 0; processedBits < N; processedBits += kWindowSize) {
        HashCombine(&hash, bits.to_ullong());
        bits >>= kWindowSize;
    }

    return hash;
}
#endif

}  // namespace dawn

namespace std {
template <typename Index, size_t N>
struct hash<dawn::ityp::bitset<Index, N>> {
  public:
    size_t operator()(const dawn::ityp::bitset<Index, N>& value) const {
        return dawn::Hash(static_cast<const std::bitset<N>&>(value));
    }
};
}  // namespace std

#endif  // SRC_DAWN_COMMON_HASHUTILS_H_

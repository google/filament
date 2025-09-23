// Copyright 2025 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_COMMON_SHA3_H_
#define SRC_DAWN_COMMON_SHA3_H_

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace dawn {

// SHA3 uses Keccak-p[1600, 24] function, meaning that there are 1600 bits of state in a 5x5x64
// array. It is designed to operate efficiently on the 64 bits of the Z dimension. When viewed as a
// string of bits it is in the order defined for S in FIPS 202. When accessed as a state,
// A[x, y, z] is the z-th bit of element x + 5y.
using Sha3Lane = uint64_t;
static_assert(25 * 8 * sizeof(Sha3Lane) == 1600);
using Sha3State = std::array<Sha3Lane, 25>;
static_assert(sizeof(Sha3State) == 25 * sizeof(Sha3Lane), "Sha3State must be packed.");

// An instance of the SHA3 algorithm for a given output length.
template <size_t BitOutputLength>
class Sha3 {
  public:
    static constexpr size_t kByteOutputLength = BitOutputLength / 8;
    using Output = std::array<uint8_t, kByteOutputLength>;

    // APIs to stream data into the hash function chunk by chunk by calling Update repeatedly.
    // After Finalize is called, it is no longer valid to use this SHA3 object.
    void Update(const void* data, size_t size);

    template <typename T>
        requires std::is_trivially_copyable_v<T>
    void Update(const T& data) {
        const uint8_t* dataAsBytes = reinterpret_cast<const uint8_t*>(&data);
        size_t size = sizeof(T);
        Update(dataAsBytes, size);
    }

    Output Finalize();

    // Helper function to compute the hash directly.
    static Output Hash(const void* data, size_t size);

  private:
    static_assert(BitOutputLength == 224 || BitOutputLength == 256 || BitOutputLength == 384 ||
                  BitOutputLength == 512);
    static constexpr size_t kByteCapacity = kByteOutputLength * 2;
    // The byte rate of performing Keccak.
    // * Sha3-224: 144 bytes
    // * Sha3-256: 136 bytes
    // * Sha3-384: 104 bytes
    // * Sha3-512: 72 bytes
    static constexpr size_t kByteRate = 1600 / 8 - kByteCapacity;

    Sha3State mState = {};
    size_t mOffsetInState = 0;
};

using Sha3_224 = Sha3<224>;
using Sha3_256 = Sha3<256>;
using Sha3_384 = Sha3<384>;
using Sha3_512 = Sha3<512>;

extern template class Sha3<224>;
extern template class Sha3<256>;
extern template class Sha3<384>;
extern template class Sha3<512>;

Sha3State KeccakForTesting(Sha3State s);

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_SHA3_H_

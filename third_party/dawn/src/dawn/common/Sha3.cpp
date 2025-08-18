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

#include "src/dawn/common/Sha3.h"

#include <algorithm>
#include <bitset>
#include <cstring>

#include "src/dawn/common/Assert.h"

namespace dawn {

// An implementation of FIPS 202 from scratch, checked against some of the official test vectors.
// All references to algorithms, variables or sections are from
// https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.202.pdf
//
// Because this is an implementation of only SHA3 and not the general family of Keccak functions, we
// hardcode b=1600 (the hash works on 64bit "lanes") and nr=24 (there are 24 rounds or state
// mixing). For efficiency the function of the state modify it directly instead of making a copy.

namespace {

constexpr uint32_t kLaneBitWidth = sizeof(Sha3Lane) * 8;  // aka "w"
constexpr uint32_t kLog2LaneBitWidth = 6;                 // aka "l"
static_assert(1 << kLog2LaneBitWidth == kLaneBitWidth);
constexpr uint32_t kRoundCount = 24;  // aka "nr"

// The rotation of a lane by `offset` bits.
Sha3Lane Rotl(Sha3Lane l, size_t offset) {
    DAWN_ASSERT(offset < kLaneBitWidth);
    // Offset should not be 0, as the expected result should be just identical and
    // right-shifting (kLaneBitWidth - offset) == kLaneBitWidth bits on Sha3Lane (having
    // kLaneBitWidth bits) results in undefined behavior.
    DAWN_ASSERT(offset > 0);
    return (l << offset) | (l >> (kLaneBitWidth - offset));
}

// Section 3.2.1: Specification of Theta.
void Theta(Sha3State& a) {
    // Step 1, compute C, the parity of each column.
    std::array<Sha3Lane, 5> c;
    for (size_t x = 0; x < 5; x++) {
        c[x] = a[x + 0] ^ a[x + 5] ^ a[x + 10] ^ a[x + 15] ^ a[x + 20];
    }

    // Combined Step 2 and 3, so as to only store a single lane of D at a time.
    for (size_t x = 0; x < 5; x++) {
        // Step 2, compute D[X] as the parity of two neighboring columns
        Sha3Lane d_x = c[(x + 4) % 5] ^ Rotl(c[(x + 1) % 5], 1);

        // Step 3, mix it in each sheet of A.
        for (size_t y = 0; y < 5; y++) {
            a[x + 5 * y] ^= d_x;
        }
    }
}

// Section 3.2.2: Specification of Rho.
// Rho is a per-lane rotation. Instead of iteratively walking the lanes in a non-linear order and
// computing the offset based on the loop index, we precompute all the offsets at compile time.
static constexpr std::array<uint8_t, 25> kRhoOffsets = []() {
    std::array<uint8_t, 25> offsets = {};

    // Step 1, lane 0 is unmodified.
    offsets[0] = 0;

    // Step 2, the start of the iteration of lines.
    uint32_t x = 1;
    uint32_t y = 0;

    // Step 3, iterate through all but the (0, 0) lane.
    for (uint32_t t = 0; t < 24; t++) {
        // Step 3a, compute the offset for the lane, pre-mod it to make them valid rotation offsets.
        offsets[x + 5 * y] = (((t + 1) * (t + 2)) / 2) % kLaneBitWidth;

        // Step 3b, update the current lane.
        uint32_t previousX = x;
        uint32_t previousY = y;
        x = previousY;
        y = (2 * previousX + 3 * previousY) % 5;
    }

    return offsets;
}();

void Rho(Sha3State& a) {
    // Rotating starts from i = 1, as kRhoOffsets[0] = 0 and the lane 0 is not rotated.
    static_assert(kRhoOffsets[0] == 0);
    for (uint32_t i = 1; i < 25; i++) {
        a[i] = Rotl(a[i], kRhoOffsets[i]);
    }
}

// Section 3.2.3: Specification of Pi
// Pi is a permutation of all the lanes with (0, 0) staying in place and a 24 lane cycle. This
// means that we can efficiently update the state if we know the order of the cycle.
static constexpr std::array<uint8_t, 24> kPiCycleIndices = []() {
    // Compute the index of the next lane we copy from in the permutation.
    std::array<uint32_t, 25> copyFromX = {};
    std::array<uint32_t, 25> copyFromY = {};

    // Step 1
    for (uint32_t x = 0; x < 5; x++) {
        for (uint32_t y = 0; y < 5; y++) {
            copyFromX[x + 5 * y] = (x + 3 * y) % 5;
            copyFromY[x + 5 * y] = x;
        }
    }

    // Invert the copy* to instead get the coordinates of the next lane in the permutation.
    std::array<uint32_t, 25> nextX = {};
    std::array<uint32_t, 25> nextY = {};
    for (uint32_t x = 0; x < 5; x++) {
        for (uint32_t y = 0; y < 5; y++) {
            uint32_t copyX = copyFromX[x + y * 5];
            uint32_t copyY = copyFromY[x + y * 5];

            nextX[copyX + 5 * copyY] = x;
            nextY[copyX + 5 * copyY] = y;
        }
    }

    // Walk the cycle starting anywhere on it, but not (0, 0) as it is fixed point.
    std::array<uint8_t, 24> cycleIndices = {};
    uint32_t x = 1;
    uint32_t y = 0;
    for (size_t i = 0; i < cycleIndices.size(); i++) {
        cycleIndices[i] = x + 5 * y;
        uint32_t previousX = x;
        uint32_t previousY = y;
        x = nextX[previousX + 5 * previousY];
        y = nextY[previousX + 5 * previousY];
    }

    return cycleIndices;
}();

void Pi(Sha3State& a) {
    // The first lane of the cycle is set to the value of the last lane of the cycle.
    Sha3Lane previousLane = a[kPiCycleIndices.back()];

    for (size_t i = 0; i < kPiCycleIndices.size(); i++) {
        Sha3Lane currentLane = a[kPiCycleIndices[i]];
        a[kPiCycleIndices[i]] = previousLane;
        previousLane = currentLane;
    }
}

// Section 3.2.4: Specification of Chi
void Chi(Sha3State& a) {
    // Step 1, Xi mixes the bits of each row so we need to copy each plane out before mixing.
    for (uint32_t y = 0; y < 5; y++) {
        std::array<Sha3Lane, 5> a_y;
        for (uint32_t x = 0; x < 5; x++) {
            a_y[x] = a[x + 5 * y];
        }

        for (uint32_t x = 0; x < 5; x++) {
            a[x + 5 * y] ^= ~a_y[(x + 1) % 5] & a_y[(x + 2) % 5];
        }
    }
}

// Section 3.2.3: Specification of Iota

// Algorithm 5 returns a bit given a variable "t". All computations are done modulo 255 so we
// precompute all possible values (even if we don't end up using all of them). We cannot use
// std::bitset::set as it isn't constexpr until C++23.
static constexpr std::array<bool, 256> kRoundConstantsBits = []() {
    std::array<bool, 256> bits = {};

    // Step 1, return 1 when t is 0.
    bits[0] = true;

    // Step 2
    uint8_t R = 1;

    // Step 3
    for (int i = 1; i < 256; i++) {
        bool R8 = R & (0x80);
        // Step 3a, 3f
        R <<= 1;

        // Step 3b, 3c, 3d, 3e
        if (R8) {
            R ^= 0b01110001;
        }
        bits[i] = R & 1;
    }

    return bits;
}();

// Algorithm 6 computes a round constant based on the round number. Precompute all of them.
static constexpr std::array<Sha3Lane, kRoundCount> kRoundConstants = []() {
    std::array<Sha3Lane, kRoundCount> RCs = {};

    for (uint32_t ir = 0; ir < kRoundCount; ir++) {
        // Step 2
        Sha3Lane RC = 0;

        // Step 3
        for (uint32_t j = 0; j < kLog2LaneBitWidth + 1; j++) {
            if (kRoundConstantsBits[j + 7 * ir]) {
                RC |= uint64_t(1) << ((1 << j) - 1);
            }
        }

        RCs[ir] = RC;
    }

    return RCs;
}();

void Iota(Sha3State& a, uint32_t ir) {
    // Step 4, combine the round constant in the first lane.
    a[0] ^= kRoundConstants[ir];
}

// Section 3.3, Keccak-b[b, nr]
void Keccak(Sha3State& a) {
    for (uint32_t ir = 0; ir < kRoundCount; ir++) {
        Theta(a);
        // TODO(402772741): For efficiency combine Rho and Pi together.
        Rho(a);
        Pi(a);
        Chi(a);
        Iota(a, ir);
    }
}

// TODO(402772741): This could be made more efficient by xoring whole 64bits at a time.
void memxorpy(void* dst, const void* src, size_t n) {
    char* dstChars = static_cast<char*>(dst);
    const char* srcChars = static_cast<const char*>(src);

    while (n > 0) {
        *dstChars ^= *srcChars;
        n--;
        dstChars++;
        srcChars++;
    }
}

}  // anonymous namespace

// Section 4: Sponge Construction
// The spec only defines hashing a full message but for convenience we use an Update/Finalize API.
// This means that Algorithm 8 is spread over multiple function calls. For each chunk of "rate"
// bits we xor them at the start of the state and perform Keccak on the state. SHA3 also adds a
// 01 suffix at the end of the message and pads the remaining bits with 10...0...01. (with 11
// being a valid padding, but not 1).
template <size_t OutputLength>
void Sha3<OutputLength>::Update(const void* data, size_t size) {
    uint8_t* stateAsString = reinterpret_cast<uint8_t*>(&mState);
    const uint8_t* dataAsBytes = static_cast<const uint8_t*>(data);

    while (size > 0) {
        DAWN_ASSERT(mOffsetInState < kByteRate);
        size_t toProcess = std::min(size, kByteRate - mOffsetInState);

        memxorpy(stateAsString + mOffsetInState, dataAsBytes, toProcess);
        size -= toProcess;
        dataAsBytes += toProcess;
        mOffsetInState += toProcess;

        if (mOffsetInState == kByteRate) {
            Keccak(mState);
            mOffsetInState = 0;
        }
    }
}

template <size_t OutputLength>
typename Sha3<OutputLength>::Output Sha3<OutputLength>::Finalize() {
    uint8_t* stateAsString = reinterpret_cast<uint8_t*>(&mState);
    DAWN_ASSERT(mOffsetInState < kByteRate);

    // Add in the 01 suffix for SHA3, as well as the first 1 for the padding.
    uint8_t* suffixByte = stateAsString + mOffsetInState;
    *suffixByte ^= 0b110;

    // Add in the last 1 of the multi-rate padding. The byte may be the same byte as suffixByte.
    uint8_t* endByte = stateAsString + (kByteRate - 1);
    *endByte ^= 0b1000'0000;

    // Do the final Keccak for the absorption in the sponge.
    Keccak(mState);

    // Garble the offset to trigger an ASSERT in Update or Finalize if they are called again.
    mOffsetInState = 0xC0FFEE;

    // The squeeze of the hash value can be done in one step.
    static_assert(sizeof(Output) <= kByteRate);
    Output output;
    memcpy(&output, &mState, sizeof(output));
    return output;
}

// static
template <size_t OutputLength>
typename Sha3<OutputLength>::Output Sha3<OutputLength>::Hash(const void* data, size_t size) {
    Sha3 sha;
    sha.Update(data, size);
    return sha.Finalize();
}

template class Sha3<224>;
template class Sha3<256>;
template class Sha3<384>;
template class Sha3<512>;

Sha3State KeccakForTesting(Sha3State s) {
    Sha3State res = s;
    Keccak(res);
    return res;
}

}  // namespace dawn

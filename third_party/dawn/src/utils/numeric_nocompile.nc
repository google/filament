// Copyright 2026 The Dawn & Tint Authors
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

#include "src/utils/numeric.h"

namespace dawn {

void TestIsDoubleValueRepresentable() {
    (void) IsDoubleValueRepresentable<double>(0.0);  // expected-error-re@numeric.h:* {{static assertion failed due to requirement '{{.*}}': Unsupported type}}
}

// Tests for checked_cast
void TestCheckedCast() {
    { [[maybe_unused]] constexpr uint32_t x = checked_cast<uint32_t>(uint64_t{0x0'FFFF'FFFF}); }
    { [[maybe_unused]] constexpr uint32_t x = checked_cast<uint32_t>(uint64_t{0x1'0000'0000}); }  // expected-error {{must be initialized by a constant expression}}
    { [[maybe_unused]] constexpr uint32_t x = checked_cast<uint32_t>(int16_t{-1}); }  // expected-error {{must be initialized by a constant expression}}

    { [[maybe_unused]] constexpr uint32_t x = checked_cast<uint32_t>(int64_t{0x0'FFFF'FFFF}); }
    { [[maybe_unused]] constexpr uint32_t x = checked_cast<uint32_t>(int64_t{0x1'0000'0000}); }  // expected-error {{must be initialized by a constant expression}}
}

// Basic test for dchecked_cast
void TestDcheckedCast() {
#ifdef DAWN_ENABLE_ASSERTS
    { [[maybe_unused]] constexpr uint32_t x = dchecked_cast<uint32_t>(int32_t{-1}); }  // expected-error {{must be initialized by a constant expression}}
#else
    { [[maybe_unused]] constexpr uint32_t x = dchecked_cast<uint32_t>(int32_t{-1}); }
#endif
}

}

// Copyright 2021 The Dawn & Tint Authors
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

#include "src/tint/utils/generation_id.h"

#include <atomic>

#include "src/tint/utils/ice/ice.h"

namespace tint {

namespace {

std::atomic<uint32_t> next_generation_id{1};

}  // namespace

GenerationID::GenerationID() = default;

GenerationID::GenerationID(uint32_t id) : val(id) {}

GenerationID GenerationID::New() {
    return GenerationID(next_generation_id++);
}

namespace detail {

/// AssertGenerationIDsEqual is called by TINT_ASSERT_GENERATION_IDS_EQUAL() and
/// TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID() to assert that the GenerationIDs
/// `a` and `b` are equal.
void AssertGenerationIDsEqual(GenerationID a,
                              GenerationID b,
                              bool if_valid,
                              const char* msg,
                              const char* file,
                              size_t line) {
    if (a == b) {
        return;  // matched
    }
    if (if_valid && (!a || !b)) {
        return;  //  a or b were not valid
    }
    tint::InternalCompilerError(file, line) << msg;
}

}  // namespace detail
}  // namespace tint

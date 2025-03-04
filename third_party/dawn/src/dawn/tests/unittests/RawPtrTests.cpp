// Copyright 2023 The Dawn & Tint Authors
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

#include <memory>
#include "gtest/gtest.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn {
namespace {

// Check Dawn is configured to crash when a raw_ptr becomes dangling.
TEST(RawPtrTests, DanglingPointerCauseCrash) {
    std::unique_ptr<bool> owner = std::make_unique<bool>(true);
    raw_ptr<bool> ptr = owner.get();
    (void)ptr;  // Unused

    ASSERT_DEATH_IF_SUPPORTED(
        {
            owner.reset();  // DanglingRawPtrDetectedFn handler => no-op.
            ptr = nullptr;  // DanglingRawPtrReleasedFn handler => crash.
        },
        "DanglingPointerDetector: A pointer was dangling!");
}

// The flag `DisableDanglingPtrDetection` must allow a raw_ptr to dangle.
TEST(RawPtrTests, DisableDanglingPtrDetection) {
    std::unique_ptr<bool> owner = std::make_unique<bool>(true);
    raw_ptr<bool, DisableDanglingPtrDetection> ptr = owner.get();
    (void)ptr;  // Unused
    owner.reset();
}

// The flag `DanglingUntriaged` must allow a raw_ptr to dangle.
TEST(RawPtrTests, DanglingUntriaged) {
    std::unique_ptr<bool> owner = std::make_unique<bool>(true);
    raw_ptr<bool, DanglingUntriaged> ptr = owner.get();
    (void)ptr;  // Unused
    owner.reset();
}

}  // anonymous namespace
}  // namespace dawn

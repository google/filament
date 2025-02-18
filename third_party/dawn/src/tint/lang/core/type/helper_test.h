// Copyright 2022 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_CORE_TYPE_HELPER_TEST_H_
#define SRC_TINT_LANG_CORE_TYPE_HELPER_TEST_H_

#include <utility>

#include "gtest/gtest.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"

namespace tint::core::type {

/// Helper class for testing
template <typename BASE>
class TestHelperBase : public BASE, public ProgramBuilder {
  public:
    /// Builds and returns the program. Must only be called once per test
    /// @return the built program
    Program Build() {
        if (!IsValid()) {
            ADD_FAILURE() << "ProgramBuilder is not valid: " << Diagnostics();
        }
        return resolver::Resolve(*this);
    }
};

/// Helper class for testing that derives from testing::Test.
using TestHelper = TestHelperBase<testing::Test>;

/// Helper class for testing that derives from `T`.
template <typename T>
using TestParamHelper = TestHelperBase<testing::TestWithParam<T>>;

}  // namespace tint::core::type

/// Helper macro for testing that a type was as expected
#define EXPECT_TYPE(GOT, EXPECT)                                                                \
    do {                                                                                        \
        const core::type::Type* got = GOT;                                                      \
        const core::type::Type* expect = EXPECT;                                                \
        if (got != expect) {                                                                    \
            ADD_FAILURE() << #GOT " != " #EXPECT "\n"                                           \
                          << "  " #GOT ": " << (got ? got->FriendlyName() : "<null>") << "\n"   \
                          << "  " #EXPECT ": " << (expect ? expect->FriendlyName() : "<null>"); \
        }                                                                                       \
    } while (false)

#endif  // SRC_TINT_LANG_CORE_TYPE_HELPER_TEST_H_

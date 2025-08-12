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

#ifndef SRC_TINT_LANG_CORE_IR_IR_HELPER_TEST_H_
#define SRC_TINT_LANG_CORE_IR_IR_HELPER_TEST_H_

#include <string>

#include "gtest/gtest.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/clone_context.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/ir/module.h"

namespace tint::core::ir {

/// Helper class for testing
template <typename BASE>
class IRTestHelperBase : public BASE {
  public:
    IRTestHelperBase() = default;
    ~IRTestHelperBase() override = default;

    /// The IR module
    Module mod;
    /// The IR builder
    Builder b{mod};
    /// The type manager
    core::type::Manager& ty{mod.Types()};

    /// CloneContext
    CloneContext clone_ctx{mod};

    /// @returns the module as a disassembled string
    std::string str() const { return "\n" + ir::Disassembler(mod).Plain(); }
};

using IRTestHelper = IRTestHelperBase<testing::Test>;

template <typename T>
using IRTestParamHelper = IRTestHelperBase<testing::TestWithParam<T>>;

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_IR_HELPER_TEST_H_

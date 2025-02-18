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

#ifndef SRC_TINT_LANG_CORE_IR_TRANSFORM_HELPER_TEST_H_
#define SRC_TINT_LANG_CORE_IR_TRANSFORM_HELPER_TEST_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/utils/containers/enum_set.h"

namespace tint::core::ir::transform {

/// Helper class for testing IR transforms.
template <typename BASE>
class TransformTestBase : public BASE {
  public:
    /// Transforms the module, using @p transform.
    /// @param transform_func the transform to run
    /// @param args the arguments to the transform function
    template <typename TRANSFORM, typename... ARGS>
    void Run(TRANSFORM&& transform_func, ARGS&&... args) {
        // Run the transform.
        auto result = transform_func(mod, std::forward<ARGS>(args)...);
        EXPECT_EQ(result, Success);
        if (result != Success) {
            return;
        }

        // Validate the output IR.
        EXPECT_EQ(ir::Validate(mod, capabilities), Success);
    }

    /// Calls the `transform` but return the result instead of validating.
    /// @param transform_func the transform to run
    /// @param args the arguments to the transform function
    template <typename TRANSFORM, typename... ARGS>
    Result<SuccessType> RunWithFailure(TRANSFORM&& transform_func, ARGS&&... args) {
        return transform_func(mod, std::forward<ARGS>(args)...);
    }

    /// @returns the transformed module as a disassembled string
    std::string str() { return "\n" + ir::Disassembler(mod).Plain(); }

  protected:
    /// The test IR module.
    ir::Module mod;
    /// The test IR builder.
    ir::Builder b{mod};
    /// The type manager.
    core::type::Manager& ty{mod.Types()};
    /// IR validation capabilities
    Capabilities capabilities;
};

using TransformTest = TransformTestBase<testing::Test>;

template <typename T>
using TransformTestWithParam = TransformTestBase<testing::TestWithParam<T>>;

}  // namespace tint::core::ir::transform

#endif  // SRC_TINT_LANG_CORE_IR_TRANSFORM_HELPER_TEST_H_

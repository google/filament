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

#ifndef SRC_TINT_LANG_WGSL_WRITER_IR_TO_PROGRAM_IR_TO_PROGRAM_TEST_H_
#define SRC_TINT_LANG_WGSL_WRITER_IR_TO_PROGRAM_IR_TO_PROGRAM_TEST_H_

#include <string>
#include <utility>

#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/type/reference.h"

namespace tint::wgsl::writer {

/// Class used for IR to Program tests
class IRToProgramTest : public core::ir::IRTestHelper {
  public:
    /// The result of Run()
    struct Result {
        /// The resulting WGSL
        std::string wgsl;
        /// The resulting AST
        std::string ast;
        /// The resulting IR
        std::string ir;
        /// The resulting error
        std::string err;
    };
    /// @returns the WGSL generated from the IR
    Result Run();

    /// Creates a new `var` declaration with a name and initializer value, using a reference type.
    /// @tparam SPACE the var's address space
    /// @tparam ACCESS the var's access mode
    /// @param name the var name
    /// @param init the var initializer
    /// @returns the instruction
    template <
        core::AddressSpace SPACE = core::AddressSpace::kFunction,
        core::Access ACCESS = core::Access::kReadWrite,
        typename VALUE = void,
        typename = std::enable_if_t<
            !traits::IsTypeOrDerived<std::remove_pointer_t<std::decay_t<VALUE>>, core::type::Type>>>
    core::ir::Var* Var(std::string_view name, VALUE&& init) {
        auto* val = b.Value(std::forward<VALUE>(init));
        if (DAWN_UNLIKELY(!val)) {
            TINT_ASSERT(val);
            return nullptr;
        }
        auto* var = b.Var(name, mod.Types().ref(SPACE, val->Type(), ACCESS));
        var->SetInitializer(val);
        mod.SetName(var->Result(), name);
        return var;
    }

    /// Creates a new `var` declaration
    /// @tparam SPACE the var's address space
    /// @tparam T the storage pointer's element type
    /// @tparam ACCESS the var's access mode
    /// @returns the instruction
    template <core::AddressSpace SPACE, typename T, core::Access ACCESS = core::Access::kReadWrite>
    core::ir::Var* Var() {
        return b.Var(mod.Types().ref<SPACE, T, ACCESS>());
    }

    /// Creates a new `var` declaration with a name
    /// @tparam SPACE the var's address space
    /// @tparam T the storage pointer's element type
    /// @tparam ACCESS the var's access mode
    /// @param name the var name
    /// @returns the instruction
    template <core::AddressSpace SPACE, typename T, core::Access ACCESS = core::Access::kReadWrite>
    core::ir::Var* Var(std::string_view name) {
        return b.Var(name, mod.Types().ref<SPACE, T, ACCESS>());
    }
};

#define EXPECT_WGSL(expected_wgsl)                                                   \
    do {                                                                             \
        if (auto got = Run(); got.err.empty()) {                                     \
            auto expected = std::string(tint::TrimSpace(expected_wgsl));             \
            if (!expected.empty()) {                                                 \
                expected = "\n" + expected + "\n";                                   \
            }                                                                        \
            EXPECT_EQ(expected, got.wgsl) << "IR: " << got.ir;                       \
        } else {                                                                     \
            FAIL() << got.err << "\nIR: " << got.ir << "\nAST: " << got.ast << "\n"; \
        }                                                                            \
    } while (false)

}  // namespace tint::wgsl::writer

#endif  // SRC_TINT_LANG_WGSL_WRITER_IR_TO_PROGRAM_IR_TO_PROGRAM_TEST_H_

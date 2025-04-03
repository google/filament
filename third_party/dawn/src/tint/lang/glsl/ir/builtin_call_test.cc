// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/lang/glsl/ir/builtin_call.h"

#include "gtest/gtest.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/ir/validator.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::glsl::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
                                              //
using IR_GlslBuiltinCallTest = core::ir::IRTestHelper;

TEST_F(IR_GlslBuiltinCallTest, Clone) {
    auto* builtin = b.Call<BuiltinCall>(mod.Types().void_(), BuiltinFn::kBarrier);

    auto* new_b = clone_ctx.Clone(builtin);

    EXPECT_NE(builtin, new_b);
    EXPECT_NE(builtin->Result(), new_b->Result());
    EXPECT_EQ(mod.Types().void_(), new_b->Result()->Type());

    EXPECT_EQ(BuiltinFn::kBarrier, new_b->Func());

    auto args = new_b->Args();
    EXPECT_EQ(0u, args.Length());
}

}  // namespace
}  // namespace tint::glsl::ir

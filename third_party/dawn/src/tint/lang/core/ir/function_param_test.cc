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

#include <string>

#include "src/tint/lang/core/ir/function_param.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using IR_FunctionParamTest = IRTestHelper;
using IR_FunctionParamDeathTest = IR_FunctionParamTest;

TEST_F(IR_FunctionParamDeathTest, Fail_SetDuplicateBuiltin) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            Builder b{mod};
            auto* fp = b.FunctionParam(mod.Types().f32());
            fp->SetBuiltin(BuiltinValue::kVertexIndex);
            fp->SetBuiltin(BuiltinValue::kSampleMask);
        },
        "internal compiler error");
}

TEST_F(IR_FunctionParamTest, CloneEmpty) {
    auto* fp = b.FunctionParam(mod.Types().f32());

    auto* new_fp = clone_ctx.Clone(fp);
    EXPECT_EQ(new_fp->Type(), mod.Types().f32());
    EXPECT_FALSE(new_fp->Builtin().has_value());
    EXPECT_FALSE(new_fp->Location().has_value());
    EXPECT_FALSE(new_fp->BindingPoint().has_value());
    EXPECT_FALSE(new_fp->Invariant());
}

TEST_F(IR_FunctionParamTest, Clone) {
    auto* fp = b.FunctionParam(mod.Types().f32());
    fp->SetBuiltin(BuiltinValue::kVertexIndex);
    fp->SetLocation(1);
    fp->SetColor(2);
    fp->SetInterpolation(
        Interpolation{core::InterpolationType::kFlat, core::InterpolationSampling::kCentroid});
    fp->SetInvariant(true);
    fp->SetBindingPoint(1, 2);

    auto* new_fp = clone_ctx.Clone(fp);

    EXPECT_NE(fp, new_fp);
    EXPECT_EQ(new_fp->Type(), mod.Types().f32());

    EXPECT_TRUE(new_fp->Builtin().has_value());
    EXPECT_EQ(BuiltinValue::kVertexIndex, new_fp->Builtin().value());

    EXPECT_EQ(new_fp->Location(), 1u);
    EXPECT_EQ(new_fp->Color(), 2u);

    auto interp = new_fp->Interpolation();
    EXPECT_TRUE(interp.has_value());
    EXPECT_EQ(interp->type, core::InterpolationType::kFlat);
    EXPECT_EQ(interp->sampling, core::InterpolationSampling::kCentroid);

    EXPECT_TRUE(new_fp->BindingPoint().has_value());
    auto bp = new_fp->BindingPoint();
    EXPECT_EQ(1u, bp->group);
    EXPECT_EQ(2u, bp->binding);

    EXPECT_TRUE(new_fp->Invariant());
}

TEST_F(IR_FunctionParamTest, CloneWithName) {
    auto* fp = b.FunctionParam("fp", mod.Types().f32());
    auto* new_fp = clone_ctx.Clone(fp);

    EXPECT_EQ(std::string("fp"), mod.NameOf(new_fp).Name());
}

}  // namespace
}  // namespace tint::core::ir

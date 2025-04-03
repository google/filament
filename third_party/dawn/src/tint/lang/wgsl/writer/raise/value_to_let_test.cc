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

#include "src/tint/lang/wgsl/writer/raise/value_to_let.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::wgsl::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using WgslWriter_ValueToLetTest = tint::core::ir::transform::TransformTest;

TEST_F(WgslWriter_ValueToLetTest, Empty) {
    auto* expect = R"(
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

////////////////////////////////////////////////////////////////////////////////
// Load / Store
////////////////////////////////////////////////////////////////////////////////
TEST_F(WgslWriter_ValueToLetTest, LoadVar_ThenStoreVar_ThenUseLoad) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var<function, i32>();
        b.Store(var, 1_i);
        auto* load = b.Load(var);
        b.Store(var, 2_i);
        b.Return(fn, load);
    });

    auto* src = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var undef
    store %2, 1i
    %3:i32 = load %2
    store %2, 2i
    ret %3
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var undef
    store %2, 1i
    %3:i32 = load %2
    %4:i32 = let %3
    store %2, 2i
    ret %4
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

////////////////////////////////////////////////////////////////////////////////
// Binary op
////////////////////////////////////////////////////////////////////////////////
TEST_F(WgslWriter_ValueToLetTest, BinaryOpUnsequencedLHSThenUnsequencedRHS) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 0_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn_b = b.Function("b", ty.i32());
    b.Append(fn_b->Block(), [&] {
        auto* lhs = b.Add(ty.i32(), 1_i, 2_i);
        auto* rhs = b.Add(ty.i32(), 3_i, 4_i);
        auto* bin = b.Add(ty.i32(), lhs, rhs);
        b.Return(fn_b, bin);
    });

    auto* src = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 0i
  }
}
%b = func():i32 {
  $B2: {
    %4:i32 = add 1i, 2i
    %5:i32 = add 3i, 4i
    %6:i32 = add %4, %5
    ret %6
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, BinaryOpSequencedLHSThenUnsequencedRHS) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 0_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn_b = b.Function("b", ty.i32());
    b.Append(fn_b->Block(), [&] {
        auto* lhs = b.Call(ty.i32(), fn_a, 1_i);
        auto* rhs = b.Add(ty.i32(), 2_i, 3_i);
        auto* bin = b.Add(ty.i32(), lhs, rhs);
        b.Return(fn_b, bin);
    });

    auto* src = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 0i
  }
}
%b = func():i32 {
  $B2: {
    %4:i32 = call %a, 1i
    %5:i32 = add 2i, 3i
    %6:i32 = add %4, %5
    ret %6
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, BinaryOpUnsequencedLHSThenSequencedRHS) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 0_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn_b = b.Function("b", ty.i32());
    b.Append(fn_b->Block(), [&] {
        auto* lhs = b.Add(ty.i32(), 1_i, 2_i);
        auto* rhs = b.Call(ty.i32(), fn_a, 3_i);
        auto* bin = b.Add(ty.i32(), lhs, rhs);
        b.Return(fn_b, bin);
    });

    auto* src = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 0i
  }
}
%b = func():i32 {
  $B2: {
    %4:i32 = add 1i, 2i
    %5:i32 = call %a, 3i
    %6:i32 = add %4, %5
    ret %6
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, BinaryOpSequencedLHSThenSequencedRHS) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 0_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn_b = b.Function("b", ty.i32());
    b.Append(fn_b->Block(), [&] {
        auto* lhs = b.Call(ty.i32(), fn_a, 1_i);
        auto* rhs = b.Call(ty.i32(), fn_a, 2_i);
        auto* bin = b.Add(ty.i32(), lhs, rhs);
        b.Return(fn_b, bin);
    });

    auto* src = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 0i
  }
}
%b = func():i32 {
  $B2: {
    %4:i32 = call %a, 1i
    %5:i32 = call %a, 2i
    %6:i32 = add %4, %5
    ret %6
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, BinaryOpUnsequencedRHSThenUnsequencedLHS) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 0_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn_b = b.Function("b", ty.i32());
    b.Append(fn_b->Block(), [&] {
        auto* rhs = b.Add(ty.i32(), 3_i, 4_i);
        auto* lhs = b.Add(ty.i32(), 1_i, 2_i);
        auto* bin = b.Add(ty.i32(), lhs, rhs);
        b.Return(fn_b, bin);
    });

    auto* src = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 0i
  }
}
%b = func():i32 {
  $B2: {
    %4:i32 = add 3i, 4i
    %5:i32 = add 1i, 2i
    %6:i32 = add %5, %4
    ret %6
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, BinaryOpUnsequencedRHSThenSequencedLHS) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 0_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn_b = b.Function("b", ty.i32());
    b.Append(fn_b->Block(), [&] {
        auto* rhs = b.Add(ty.i32(), 2_i, 3_i);
        auto* lhs = b.Call(ty.i32(), fn_a, 1_i);
        auto* bin = b.Add(ty.i32(), lhs, rhs);
        b.Return(fn_b, bin);
    });

    auto* src = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 0i
  }
}
%b = func():i32 {
  $B2: {
    %4:i32 = add 2i, 3i
    %5:i32 = call %a, 1i
    %6:i32 = add %5, %4
    ret %6
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, BinaryOpSequencedRHSThenUnsequencedLHS) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 0_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn_b = b.Function("b", ty.i32());
    b.Append(fn_b->Block(), [&] {
        auto* rhs = b.Call(ty.i32(), fn_a, 3_i);
        auto* lhs = b.Add(ty.i32(), 1_i, 2_i);
        auto* bin = b.Add(ty.i32(), lhs, rhs);
        b.Return(fn_b, bin);
    });

    auto* src = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 0i
  }
}
%b = func():i32 {
  $B2: {
    %4:i32 = call %a, 3i
    %5:i32 = add 1i, 2i
    %6:i32 = add %5, %4
    ret %6
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, BinaryOpSequencedRHSThenSequencedLHS) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 0_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn_b = b.Function("b", ty.i32());
    b.Append(fn_b->Block(), [&] {
        auto* rhs = b.Call(ty.i32(), fn_a, 2_i);
        auto* lhs = b.Call(ty.i32(), fn_a, 1_i);
        auto* bin = b.Add(ty.i32(), lhs, rhs);
        b.Return(fn_b, bin);
    });

    auto* src = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 0i
  }
}
%b = func():i32 {
  $B2: {
    %4:i32 = call %a, 2i
    %5:i32 = call %a, 1i
    %6:i32 = add %5, %4
    ret %6
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 0i
  }
}
%b = func():i32 {
  $B2: {
    %4:i32 = call %a, 2i
    %5:i32 = let %4
    %6:i32 = call %a, 1i
    %7:i32 = add %6, %5
    ret %7
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

////////////////////////////////////////////////////////////////////////////////
// Call
////////////////////////////////////////////////////////////////////////////////
TEST_F(WgslWriter_ValueToLetTest, CallSequencedXYZ) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 0_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn_b = b.Function("b", ty.i32());
    b.Append(fn_b->Block(), [&] { b.Return(fn_b, 0_i); });
    fn_b->SetParams(
        {b.FunctionParam(ty.i32()), b.FunctionParam(ty.i32()), b.FunctionParam(ty.i32())});

    auto* fn_c = b.Function("c", ty.i32());
    b.Append(fn_c->Block(), [&] {
        auto* x = b.Call(ty.i32(), fn_a, 1_i);
        auto* y = b.Call(ty.i32(), fn_a, 2_i);
        auto* z = b.Call(ty.i32(), fn_a, 3_i);
        auto* call = b.Call(ty.i32(), fn_b, x, y, z);
        b.Return(fn_c, call);
    });

    auto* src = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 0i
  }
}
%b = func(%4:i32, %5:i32, %6:i32):i32 {
  $B2: {
    ret 0i
  }
}
%c = func():i32 {
  $B3: {
    %8:i32 = call %a, 1i
    %9:i32 = call %a, 2i
    %10:i32 = call %a, 3i
    %11:i32 = call %b, %8, %9, %10
    ret %11
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, CallSequencedYXZ) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 0_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn_b = b.Function("b", ty.i32());
    b.Append(fn_b->Block(), [&] { b.Return(fn_b, 0_i); });
    fn_b->SetParams(
        {b.FunctionParam(ty.i32()), b.FunctionParam(ty.i32()), b.FunctionParam(ty.i32())});

    auto* fn_c = b.Function("c", ty.i32());
    b.Append(fn_c->Block(), [&] {
        auto* y = b.Call(ty.i32(), fn_a, 2_i);
        auto* x = b.Call(ty.i32(), fn_a, 1_i);
        auto* z = b.Call(ty.i32(), fn_a, 3_i);
        auto* call = b.Call(ty.i32(), fn_b, x, y, z);
        b.Return(fn_c, call);
    });

    auto* src = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 0i
  }
}
%b = func(%4:i32, %5:i32, %6:i32):i32 {
  $B2: {
    ret 0i
  }
}
%c = func():i32 {
  $B3: {
    %8:i32 = call %a, 2i
    %9:i32 = call %a, 1i
    %10:i32 = call %a, 3i
    %11:i32 = call %b, %9, %8, %10
    ret %11
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 0i
  }
}
%b = func(%4:i32, %5:i32, %6:i32):i32 {
  $B2: {
    ret 0i
  }
}
%c = func():i32 {
  $B3: {
    %8:i32 = call %a, 2i
    %9:i32 = let %8
    %10:i32 = call %a, 1i
    %11:i32 = call %a, 3i
    %12:i32 = call %b, %10, %9, %11
    ret %12
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, CallSequencedXZY) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 0_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn_b = b.Function("b", ty.i32());
    b.Append(fn_b->Block(), [&] { b.Return(fn_b, 0_i); });
    fn_b->SetParams(
        {b.FunctionParam(ty.i32()), b.FunctionParam(ty.i32()), b.FunctionParam(ty.i32())});

    auto* fn_c = b.Function("c", ty.i32());
    b.Append(fn_c->Block(), [&] {
        auto* x = b.Call(ty.i32(), fn_a, 1_i);
        auto* z = b.Call(ty.i32(), fn_a, 3_i);
        auto* y = b.Call(ty.i32(), fn_a, 2_i);
        auto* call = b.Call(ty.i32(), fn_b, x, y, z);
        b.Return(fn_c, call);
    });

    auto* src = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 0i
  }
}
%b = func(%4:i32, %5:i32, %6:i32):i32 {
  $B2: {
    ret 0i
  }
}
%c = func():i32 {
  $B3: {
    %8:i32 = call %a, 1i
    %9:i32 = call %a, 3i
    %10:i32 = call %a, 2i
    %11:i32 = call %b, %8, %10, %9
    ret %11
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 0i
  }
}
%b = func(%4:i32, %5:i32, %6:i32):i32 {
  $B2: {
    ret 0i
  }
}
%c = func():i32 {
  $B3: {
    %8:i32 = call %a, 1i
    %9:i32 = let %8
    %10:i32 = call %a, 3i
    %11:i32 = let %10
    %12:i32 = call %a, 2i
    %13:i32 = call %b, %9, %12, %11
    ret %13
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, CallSequencedZXY) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 0_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn_b = b.Function("b", ty.i32());
    b.Append(fn_b->Block(), [&] { b.Return(fn_b, 0_i); });
    fn_b->SetParams(
        {b.FunctionParam(ty.i32()), b.FunctionParam(ty.i32()), b.FunctionParam(ty.i32())});

    auto* fn_c = b.Function("c", ty.i32());
    b.Append(fn_c->Block(), [&] {
        auto* z = b.Call(ty.i32(), fn_a, 3_i);
        auto* x = b.Call(ty.i32(), fn_a, 1_i);
        auto* y = b.Call(ty.i32(), fn_a, 2_i);
        auto* call = b.Call(ty.i32(), fn_b, x, y, z);
        b.Return(fn_c, call);
    });

    auto* src = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 0i
  }
}
%b = func(%4:i32, %5:i32, %6:i32):i32 {
  $B2: {
    ret 0i
  }
}
%c = func():i32 {
  $B3: {
    %8:i32 = call %a, 3i
    %9:i32 = call %a, 1i
    %10:i32 = call %a, 2i
    %11:i32 = call %b, %9, %10, %8
    ret %11
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 0i
  }
}
%b = func(%4:i32, %5:i32, %6:i32):i32 {
  $B2: {
    ret 0i
  }
}
%c = func():i32 {
  $B3: {
    %8:i32 = call %a, 3i
    %9:i32 = let %8
    %10:i32 = call %a, 1i
    %11:i32 = call %a, 2i
    %12:i32 = call %b, %10, %11, %9
    ret %12
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, CallSequencedYZX) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 0_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn_b = b.Function("b", ty.i32());
    b.Append(fn_b->Block(), [&] { b.Return(fn_b, 0_i); });
    fn_b->SetParams(
        {b.FunctionParam(ty.i32()), b.FunctionParam(ty.i32()), b.FunctionParam(ty.i32())});

    auto* fn_c = b.Function("c", ty.i32());
    b.Append(fn_c->Block(), [&] {
        auto* y = b.Call(ty.i32(), fn_a, 2_i);
        auto* z = b.Call(ty.i32(), fn_a, 3_i);
        auto* x = b.Call(ty.i32(), fn_a, 1_i);
        auto* call = b.Call(ty.i32(), fn_b, x, y, z);
        b.Return(fn_c, call);
    });

    auto* src = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 0i
  }
}
%b = func(%4:i32, %5:i32, %6:i32):i32 {
  $B2: {
    ret 0i
  }
}
%c = func():i32 {
  $B3: {
    %8:i32 = call %a, 2i
    %9:i32 = call %a, 3i
    %10:i32 = call %a, 1i
    %11:i32 = call %b, %10, %8, %9
    ret %11
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 0i
  }
}
%b = func(%4:i32, %5:i32, %6:i32):i32 {
  $B2: {
    ret 0i
  }
}
%c = func():i32 {
  $B3: {
    %8:i32 = call %a, 2i
    %9:i32 = let %8
    %10:i32 = call %a, 3i
    %11:i32 = let %10
    %12:i32 = call %a, 1i
    %13:i32 = call %b, %12, %9, %11
    ret %13
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, CallSequencedZYX) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 0_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn_b = b.Function("b", ty.i32());
    b.Append(fn_b->Block(), [&] { b.Return(fn_b, 0_i); });
    fn_b->SetParams(
        {b.FunctionParam(ty.i32()), b.FunctionParam(ty.i32()), b.FunctionParam(ty.i32())});

    auto* fn_c = b.Function("c", ty.i32());
    b.Append(fn_c->Block(), [&] {
        auto* z = b.Call(ty.i32(), fn_a, 3_i);
        auto* y = b.Call(ty.i32(), fn_a, 2_i);
        auto* x = b.Call(ty.i32(), fn_a, 1_i);
        auto* call = b.Call(ty.i32(), fn_b, x, y, z);
        b.Return(fn_c, call);
    });

    auto* src = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 0i
  }
}
%b = func(%4:i32, %5:i32, %6:i32):i32 {
  $B2: {
    ret 0i
  }
}
%c = func():i32 {
  $B3: {
    %8:i32 = call %a, 3i
    %9:i32 = call %a, 2i
    %10:i32 = call %a, 1i
    %11:i32 = call %b, %10, %9, %8
    ret %11
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 0i
  }
}
%b = func(%4:i32, %5:i32, %6:i32):i32 {
  $B2: {
    ret 0i
  }
}
%c = func():i32 {
  $B3: {
    %8:i32 = call %a, 3i
    %9:i32 = let %8
    %10:i32 = call %a, 2i
    %11:i32 = let %10
    %12:i32 = call %a, 1i
    %13:i32 = call %b, %12, %11, %9
    ret %13
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, LoadVar_ThenCallVoidFn_ThenUseLoad) {
    auto* fn_a = b.Function("a", ty.void_());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a); });

    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var<function, i32>();
        b.Store(var, 1_i);
        auto* load = b.Load(var);
        b.Call(ty.void_(), fn_a);
        b.Return(fn, load);
    });

    auto* src = R"(
%a = func():void {
  $B1: {
    ret
  }
}
%f = func():i32 {
  $B2: {
    %3:ptr<function, i32, read_write> = var undef
    store %3, 1i
    %4:i32 = load %3
    %5:void = call %a
    ret %4
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%a = func():void {
  $B1: {
    ret
  }
}
%f = func():i32 {
  $B2: {
    %3:ptr<function, i32, read_write> = var undef
    store %3, 1i
    %4:i32 = load %3
    %5:i32 = let %4
    %6:void = call %a
    ret %5
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, LoadVar_ThenCallUnusedi32Fn_ThenUseLoad) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 1_i); });

    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var<function, i32>();
        b.Store(var, 1_i);
        auto* load = b.Load(var);
        b.Call(ty.i32(), fn_a);
        b.Return(fn, load);
    });

    auto* src = R"(
%a = func():i32 {
  $B1: {
    ret 1i
  }
}
%f = func():i32 {
  $B2: {
    %3:ptr<function, i32, read_write> = var undef
    store %3, 1i
    %4:i32 = load %3
    %5:i32 = call %a
    ret %4
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%a = func():i32 {
  $B1: {
    ret 1i
  }
}
%f = func():i32 {
  $B2: {
    %3:ptr<function, i32, read_write> = var undef
    store %3, 1i
    %4:i32 = load %3
    %5:i32 = let %4
    %6:i32 = call %a
    ret %5
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, LoadVar_ThenCalli32Fn_ThenUseLoadBeforeCall) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 1_i); });

    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var<function, i32>();
        b.Store(var, 1_i);
        auto* load = b.Load(var);
        auto* call = b.Call(ty.i32(), fn_a);
        b.Return(fn, b.Add(ty.i32(), load, call));
    });

    auto* src = R"(
%a = func():i32 {
  $B1: {
    ret 1i
  }
}
%f = func():i32 {
  $B2: {
    %3:ptr<function, i32, read_write> = var undef
    store %3, 1i
    %4:i32 = load %3
    %5:i32 = call %a
    %6:i32 = add %4, %5
    ret %6
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%a = func():i32 {
  $B1: {
    ret 1i
  }
}
%f = func():i32 {
  $B2: {
    %3:ptr<function, i32, read_write> = var undef
    store %3, 1i
    %4:i32 = load %3
    %5:i32 = call %a
    %6:i32 = add %4, %5
    ret %6
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, LoadVar_ThenCalli32Fn_ThenUseCallBeforeLoad) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 1_i); });

    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var<function, i32>();
        b.Store(var, 1_i);
        auto* load = b.Load(var);
        auto* call = b.Call(ty.i32(), fn_a);
        b.Return(fn, b.Add(ty.i32(), call, load));
    });

    auto* src = R"(
%a = func():i32 {
  $B1: {
    ret 1i
  }
}
%f = func():i32 {
  $B2: {
    %3:ptr<function, i32, read_write> = var undef
    store %3, 1i
    %4:i32 = load %3
    %5:i32 = call %a
    %6:i32 = add %5, %4
    ret %6
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%a = func():i32 {
  $B1: {
    ret 1i
  }
}
%f = func():i32 {
  $B2: {
    %3:ptr<function, i32, read_write> = var undef
    store %3, 1i
    %4:i32 = load %3
    %5:i32 = let %4
    %6:i32 = call %a
    %7:i32 = add %6, %5
    ret %7
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

////////////////////////////////////////////////////////////////////////////////
// Access
////////////////////////////////////////////////////////////////////////////////
TEST_F(WgslWriter_ValueToLetTest, Access_ArrayOfArrayOfArray_XYZ) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 1_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* arr = b.Var<function, array<array<array<i32, 3>, 4>, 5>>();
        auto* x = b.Call(ty.i32(), fn_a, 1_i);
        auto* y = b.Call(ty.i32(), fn_a, 2_i);
        auto* z = b.Call(ty.i32(), fn_a, 3_i);
        auto* access = b.Access(ty.ptr<function, i32>(), arr, x, y, z);
        b.Return(fn, b.Load(access));
    });

    auto* src = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 1i
  }
}
%f = func():i32 {
  $B2: {
    %4:ptr<function, array<array<array<i32, 3>, 4>, 5>, read_write> = var undef
    %5:i32 = call %a, 1i
    %6:i32 = call %a, 2i
    %7:i32 = call %a, 3i
    %8:ptr<function, i32, read_write> = access %4, %5, %6, %7
    %9:i32 = load %8
    ret %9
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 1i
  }
}
%f = func():i32 {
  $B2: {
    %4:ptr<function, array<array<array<i32, 3>, 4>, 5>, read_write> = var undef
    %5:i32 = call %a, 1i
    %6:i32 = call %a, 2i
    %7:i32 = call %a, 3i
    %8:ptr<function, i32, read_write> = access %4, %5, %6, %7
    %9:i32 = load %8
    ret %9
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, Access_ArrayOfArrayOfArray_YXZ) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 1_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* arr = b.Var<function, array<array<array<i32, 3>, 4>, 5>>();
        auto* y = b.Call(ty.i32(), fn_a, 2_i);
        auto* x = b.Call(ty.i32(), fn_a, 1_i);
        auto* z = b.Call(ty.i32(), fn_a, 3_i);
        auto* access = b.Access(ty.ptr<function, i32>(), arr, x, y, z);
        b.Return(fn, b.Load(access));
    });

    auto* src = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 1i
  }
}
%f = func():i32 {
  $B2: {
    %4:ptr<function, array<array<array<i32, 3>, 4>, 5>, read_write> = var undef
    %5:i32 = call %a, 2i
    %6:i32 = call %a, 1i
    %7:i32 = call %a, 3i
    %8:ptr<function, i32, read_write> = access %4, %6, %5, %7
    %9:i32 = load %8
    ret %9
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 1i
  }
}
%f = func():i32 {
  $B2: {
    %4:ptr<function, array<array<array<i32, 3>, 4>, 5>, read_write> = var undef
    %5:i32 = call %a, 2i
    %6:i32 = let %5
    %7:i32 = call %a, 1i
    %8:i32 = call %a, 3i
    %9:ptr<function, i32, read_write> = access %4, %7, %6, %8
    %10:i32 = load %9
    ret %10
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, Access_ArrayOfArrayOfArray_ZXY) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 1_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* arr = b.Var<function, array<array<array<i32, 3>, 4>, 5>>();
        auto* z = b.Call(ty.i32(), fn_a, 3_i);
        auto* x = b.Call(ty.i32(), fn_a, 1_i);
        auto* y = b.Call(ty.i32(), fn_a, 2_i);
        auto* access = b.Access(ty.ptr<function, i32>(), arr, x, y, z);
        b.Return(fn, b.Load(access));
    });

    auto* src = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 1i
  }
}
%f = func():i32 {
  $B2: {
    %4:ptr<function, array<array<array<i32, 3>, 4>, 5>, read_write> = var undef
    %5:i32 = call %a, 3i
    %6:i32 = call %a, 1i
    %7:i32 = call %a, 2i
    %8:ptr<function, i32, read_write> = access %4, %6, %7, %5
    %9:i32 = load %8
    ret %9
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 1i
  }
}
%f = func():i32 {
  $B2: {
    %4:ptr<function, array<array<array<i32, 3>, 4>, 5>, read_write> = var undef
    %5:i32 = call %a, 3i
    %6:i32 = let %5
    %7:i32 = call %a, 1i
    %8:i32 = call %a, 2i
    %9:ptr<function, i32, read_write> = access %4, %7, %8, %6
    %10:i32 = load %9
    ret %10
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, Access_ArrayOfArrayOfArray_ZYX) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 1_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* arr = b.Var<function, array<array<array<i32, 3>, 4>, 5>>();
        auto* z = b.Call(ty.i32(), fn_a, 3_i);
        auto* y = b.Call(ty.i32(), fn_a, 2_i);
        auto* x = b.Call(ty.i32(), fn_a, 1_i);
        auto* access = b.Access(ty.ptr<function, i32>(), arr, x, y, z);
        b.Return(fn, b.Load(access));
    });

    auto* src = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 1i
  }
}
%f = func():i32 {
  $B2: {
    %4:ptr<function, array<array<array<i32, 3>, 4>, 5>, read_write> = var undef
    %5:i32 = call %a, 3i
    %6:i32 = call %a, 2i
    %7:i32 = call %a, 1i
    %8:ptr<function, i32, read_write> = access %4, %7, %6, %5
    %9:i32 = load %8
    ret %9
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 1i
  }
}
%f = func():i32 {
  $B2: {
    %4:ptr<function, array<array<array<i32, 3>, 4>, 5>, read_write> = var undef
    %5:i32 = call %a, 3i
    %6:i32 = let %5
    %7:i32 = call %a, 2i
    %8:i32 = let %7
    %9:i32 = call %a, 1i
    %10:ptr<function, i32, read_write> = access %4, %9, %8, %6
    %11:i32 = load %10
    ret %11
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, Access_ArrayOfMat3x4f_XYZ) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 1_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn = b.Function("f", ty.f32());
    b.Append(fn->Block(), [&] {
        auto* arr = b.Construct(ty.array<mat3x4<f32>, 5>());
        auto* x = b.Call(ty.i32(), fn_a, 1_i);
        auto* y = b.Call(ty.i32(), fn_a, 2_i);
        auto* z = b.Call(ty.i32(), fn_a, 3_i);
        auto* access = b.Access(ty.f32(), arr, x, y, z);
        b.Return(fn, access);
    });

    auto* src = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 1i
  }
}
%f = func():f32 {
  $B2: {
    %4:array<mat3x4<f32>, 5> = construct
    %5:i32 = call %a, 1i
    %6:i32 = call %a, 2i
    %7:i32 = call %a, 3i
    %8:f32 = access %4, %5, %6, %7
    ret %8
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 1i
  }
}
%f = func():f32 {
  $B2: {
    %4:array<mat3x4<f32>, 5> = construct
    %5:i32 = call %a, 1i
    %6:i32 = call %a, 2i
    %7:i32 = call %a, 3i
    %8:f32 = access %4, %5, %6, %7
    ret %8
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, Access_ArrayOfMat3x4f_YXZ) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 1_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn = b.Function("f", ty.f32());
    b.Append(fn->Block(), [&] {
        auto* arr = b.Construct(ty.array<mat3x4<f32>, 5>());
        auto* y = b.Call(ty.i32(), fn_a, 2_i);
        auto* x = b.Call(ty.i32(), fn_a, 1_i);
        auto* z = b.Call(ty.i32(), fn_a, 3_i);
        auto* access = b.Access(ty.f32(), arr, x, y, z);
        b.Return(fn, access);
    });

    auto* src = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 1i
  }
}
%f = func():f32 {
  $B2: {
    %4:array<mat3x4<f32>, 5> = construct
    %5:i32 = call %a, 2i
    %6:i32 = call %a, 1i
    %7:i32 = call %a, 3i
    %8:f32 = access %4, %6, %5, %7
    ret %8
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 1i
  }
}
%f = func():f32 {
  $B2: {
    %4:array<mat3x4<f32>, 5> = construct
    %5:i32 = call %a, 2i
    %6:i32 = let %5
    %7:i32 = call %a, 1i
    %8:i32 = call %a, 3i
    %9:f32 = access %4, %7, %6, %8
    ret %9
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, Access_ArrayOfMat3x4f_ZXY) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 1_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn = b.Function("f", ty.f32());
    b.Append(fn->Block(), [&] {
        auto* arr = b.Construct(ty.array<mat3x4<f32>, 5>());
        auto* z = b.Call(ty.i32(), fn_a, 3_i);
        auto* x = b.Call(ty.i32(), fn_a, 1_i);
        auto* y = b.Call(ty.i32(), fn_a, 2_i);
        auto* access = b.Access(ty.f32(), arr, x, y, z);
        b.Return(fn, access);
    });

    auto* src = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 1i
  }
}
%f = func():f32 {
  $B2: {
    %4:array<mat3x4<f32>, 5> = construct
    %5:i32 = call %a, 3i
    %6:i32 = call %a, 1i
    %7:i32 = call %a, 2i
    %8:f32 = access %4, %6, %7, %5
    ret %8
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 1i
  }
}
%f = func():f32 {
  $B2: {
    %4:array<mat3x4<f32>, 5> = construct
    %5:i32 = call %a, 3i
    %6:i32 = let %5
    %7:i32 = call %a, 1i
    %8:i32 = call %a, 2i
    %9:f32 = access %4, %7, %8, %6
    ret %9
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, Access_ArrayOfMat3x4f_ZYX) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 1_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn = b.Function("f", ty.f32());
    b.Append(fn->Block(), [&] {
        auto* arr = b.Construct(ty.array<mat3x4<f32>, 5>());
        auto* z = b.Call(ty.i32(), fn_a, 3_i);
        auto* y = b.Call(ty.i32(), fn_a, 2_i);
        auto* x = b.Call(ty.i32(), fn_a, 1_i);
        auto* access = b.Access(ty.f32(), arr, x, y, z);
        b.Return(fn, access);
    });

    auto* src = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 1i
  }
}
%f = func():f32 {
  $B2: {
    %4:array<mat3x4<f32>, 5> = construct
    %5:i32 = call %a, 3i
    %6:i32 = call %a, 2i
    %7:i32 = call %a, 1i
    %8:f32 = access %4, %7, %6, %5
    ret %8
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%a = func(%2:i32):i32 {
  $B1: {
    ret 1i
  }
}
%f = func():f32 {
  $B2: {
    %4:array<mat3x4<f32>, 5> = construct
    %5:i32 = call %a, 3i
    %6:i32 = let %5
    %7:i32 = call %a, 2i
    %8:i32 = let %7
    %9:i32 = call %a, 1i
    %10:f32 = access %4, %9, %8, %6
    ret %10
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

////////////////////////////////////////////////////////////////////////////////
// If
////////////////////////////////////////////////////////////////////////////////
TEST_F(WgslWriter_ValueToLetTest, UnsequencedOutsideIf) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* v = b.Add(ty.i32(), 1_i, 2_i);
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] { b.Return(fn, v); });
        b.Return(fn, 0_i);
    });

    auto* src = R"(
%f = func():i32 {
  $B1: {
    %2:i32 = add 1i, 2i
    if true [t: $B2] {  # if_1
      $B2: {  # true
        ret %2
      }
    }
    ret 0i
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, SequencedOutsideIf) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var<function, i32>();
        var->SetInitializer(b.Constant(1_i));
        auto* v_1 = b.Load(var);
        auto* v_2 = b.Add(ty.i32(), v_1, 2_i);
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] { b.Return(fn, v_2); });
        b.Return(fn, 0_i);
    });

    auto* src = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var 1i
    %3:i32 = load %2
    %4:i32 = add %3, 2i
    if true [t: $B2] {  # if_1
      $B2: {  # true
        ret %4
      }
    }
    ret 0i
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var 1i
    %3:i32 = load %2
    %4:i32 = add %3, 2i
    %5:i32 = let %4
    if true [t: $B2] {  # if_1
      $B2: {  # true
        ret %5
      }
    }
    ret 0i
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, UnsequencedUsedByIfCondition) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* v = b.Equal(ty.bool_(), 1_i, 2_i);
        auto* if_ = b.If(v);
        b.Append(if_->True(), [&] { b.Return(fn, 3_i); });
        b.Return(fn, 0_i);
    });

    auto* src = R"(
%f = func():i32 {
  $B1: {
    %2:bool = eq 1i, 2i
    if %2 [t: $B2] {  # if_1
      $B2: {  # true
        ret 3i
      }
    }
    ret 0i
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%f = func():i32 {
  $B1: {
    %2:bool = eq 1i, 2i
    if %2 [t: $B2] {  # if_1
      $B2: {  # true
        ret 3i
      }
    }
    ret 0i
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, SequencedUsedByIfCondition) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var<function, i32>();
        var->SetInitializer(b.Constant(1_i));
        auto* v_1 = b.Load(var);
        auto* v_2 = b.Equal(ty.bool_(), v_1, 2_i);
        auto* if_ = b.If(v_2);
        b.Append(if_->True(), [&] { b.Return(fn, 3_i); });
        b.Return(fn, 0_i);
    });

    auto* src = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var 1i
    %3:i32 = load %2
    %4:bool = eq %3, 2i
    if %4 [t: $B2] {  # if_1
      $B2: {  # true
        ret 3i
      }
    }
    ret 0i
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var 1i
    %3:i32 = load %2
    %4:bool = eq %3, 2i
    if %4 [t: $B2] {  # if_1
      $B2: {  # true
        ret 3i
      }
    }
    ret 0i
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, LoadVar_ThenWriteToVarInIf_ThenUseLoad) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var<function, i32>();
        b.Store(var, 1_i);
        auto* load = b.Load(var);
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] {
            b.Store(var, 2_i);
            b.ExitIf(if_);
        });
        b.Return(fn, load);
    });

    auto* src = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var undef
    store %2, 1i
    %3:i32 = load %2
    if true [t: $B2] {  # if_1
      $B2: {  # true
        store %2, 2i
        exit_if  # if_1
      }
    }
    ret %3
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var undef
    store %2, 1i
    %3:i32 = load %2
    %4:i32 = let %3
    if true [t: $B2] {  # if_1
      $B2: {  # true
        store %2, 2i
        exit_if  # if_1
      }
    }
    ret %4
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

////////////////////////////////////////////////////////////////////////////////
// Switch
////////////////////////////////////////////////////////////////////////////////
TEST_F(WgslWriter_ValueToLetTest, UnsequencedOutsideSwitch) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* v = b.Add(ty.i32(), 1_i, 2_i);
        auto* switch_ = b.Switch(3_i);
        auto* case_ = b.DefaultCase(switch_);
        b.Append(case_, [&] { b.Return(fn, v); });
        b.Return(fn, 0_i);
    });

    auto* src = R"(
%f = func():i32 {
  $B1: {
    %2:i32 = add 1i, 2i
    switch 3i [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        ret %2
      }
    }
    ret 0i
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, SequencedOutsideSwitch) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var<function, i32>();
        var->SetInitializer(b.Constant(1_i));
        auto* v_1 = b.Load(var);
        auto* v_2 = b.Add(ty.i32(), v_1, 2_i);
        auto* switch_ = b.Switch(3_i);
        auto* case_ = b.DefaultCase(switch_);
        b.Append(case_, [&] { b.Return(fn, v_2); });
        b.Return(fn, 0_i);
    });

    auto* src = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var 1i
    %3:i32 = load %2
    %4:i32 = add %3, 2i
    switch 3i [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        ret %4
      }
    }
    ret 0i
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var 1i
    %3:i32 = load %2
    %4:i32 = add %3, 2i
    %5:i32 = let %4
    switch 3i [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        ret %5
      }
    }
    ret 0i
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, UnsequencedUsedBySwitchCondition) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* v = b.Add(ty.i32(), 1_i, 2_i);
        auto* switch_ = b.Switch(v);
        auto* case_ = b.DefaultCase(switch_);
        b.Append(case_, [&] { b.Return(fn, 3_i); });
        b.Return(fn, 0_i);
    });

    auto* src = R"(
%f = func():i32 {
  $B1: {
    %2:i32 = add 1i, 2i
    switch %2 [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        ret 3i
      }
    }
    ret 0i
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%f = func():i32 {
  $B1: {
    %2:i32 = add 1i, 2i
    switch %2 [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        ret 3i
      }
    }
    ret 0i
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, SequencedUsedBySwitchCondition) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var<function, i32>();
        var->SetInitializer(b.Constant(1_i));
        auto* v_1 = b.Load(var);
        auto* switch_ = b.Switch(v_1);
        auto* case_ = b.DefaultCase(switch_);
        b.Append(case_, [&] { b.Return(fn, 3_i); });
        b.Return(fn, 0_i);
    });

    auto* src = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var 1i
    %3:i32 = load %2
    switch %3 [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        ret 3i
      }
    }
    ret 0i
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var 1i
    %3:i32 = load %2
    switch %3 [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        ret 3i
      }
    }
    ret 0i
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, LoadVar_ThenWriteToVarInSwitch_ThenUseLoad) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var<function, i32>();
        b.Store(var, 1_i);
        auto* load = b.Load(var);
        auto* switch_ = b.Switch(1_i);
        auto* case_ = b.DefaultCase(switch_);
        b.Append(case_, [&] {
            b.Store(var, 2_i);
            b.ExitSwitch(switch_);
        });
        b.Return(fn, load);
    });

    auto* src = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var undef
    store %2, 1i
    %3:i32 = load %2
    switch 1i [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        store %2, 2i
        exit_switch  # switch_1
      }
    }
    ret %3
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var undef
    store %2, 1i
    %3:i32 = load %2
    %4:i32 = let %3
    switch 1i [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        store %2, 2i
        exit_switch  # switch_1
      }
    }
    ret %4
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

////////////////////////////////////////////////////////////////////////////////
// Loop
////////////////////////////////////////////////////////////////////////////////
TEST_F(WgslWriter_ValueToLetTest, UnsequencedOutsideLoopInitializer) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var<function, i32>();
        auto* v = b.Add(ty.i32(), 1_i, 2_i);
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            b.Store(var, v);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Return(fn, 0_i);
    });

    auto* src = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var undef
    %3:i32 = add 1i, 2i
    loop [i: $B2, b: $B3] {  # loop_1
      $B2: {  # initializer
        store %2, %3
        next_iteration  # -> $B3
      }
      $B3: {  # body
        exit_loop  # loop_1
      }
    }
    ret 0i
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, SequencedOutsideLoopInitializer) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var<function, i32>();
        auto* v_1 = b.Load(var);
        auto* v_2 = b.Add(ty.i32(), v_1, 2_i);
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            b.Store(var, v_2);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Return(fn, 0_i);
    });

    auto* src = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var undef
    %3:i32 = load %2
    %4:i32 = add %3, 2i
    loop [i: $B2, b: $B3] {  # loop_1
      $B2: {  # initializer
        store %2, %4
        next_iteration  # -> $B3
      }
      $B3: {  # body
        exit_loop  # loop_1
      }
    }
    ret 0i
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var undef
    %3:i32 = load %2
    %4:i32 = add %3, 2i
    %5:i32 = let %4
    loop [i: $B2, b: $B3] {  # loop_1
      $B2: {  # initializer
        store %2, %5
        next_iteration  # -> $B3
      }
      $B3: {  # body
        exit_loop  # loop_1
      }
    }
    ret 0i
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, LoadVar_ThenWriteToVarInLoopInitializer_ThenUseLoad) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var<function, i32>();
        b.Store(var, 1_i);
        auto* load = b.Load(var);
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            b.Store(var, 2_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Return(fn, load);
    });

    auto* src = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var undef
    store %2, 1i
    %3:i32 = load %2
    loop [i: $B2, b: $B3] {  # loop_1
      $B2: {  # initializer
        store %2, 2i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        exit_loop  # loop_1
      }
    }
    ret %3
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var undef
    store %2, 1i
    %3:i32 = load %2
    %4:i32 = let %3
    loop [i: $B2, b: $B3] {  # loop_1
      $B2: {  # initializer
        store %2, 2i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        exit_loop  # loop_1
      }
    }
    ret %4
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, UnsequencedOutsideLoopBody) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* v = b.Add(ty.i32(), 1_i, 2_i);
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.Return(fn, v); });
        b.Return(fn, 0_i);
    });

    auto* src = R"(
%f = func():i32 {
  $B1: {
    %2:i32 = add 1i, 2i
    loop [b: $B2] {  # loop_1
      $B2: {  # body
        ret %2
      }
    }
    ret 0i
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, SequencedOutsideLoopBody) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var<function, i32>();
        auto* v_1 = b.Load(var);
        auto* v_2 = b.Add(ty.i32(), v_1, 2_i);
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.Return(fn, v_2); });
        b.Return(fn, 0_i);
    });

    auto* src = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var undef
    %3:i32 = load %2
    %4:i32 = add %3, 2i
    loop [b: $B2] {  # loop_1
      $B2: {  # body
        ret %4
      }
    }
    ret 0i
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var undef
    %3:i32 = load %2
    %4:i32 = add %3, 2i
    %5:i32 = let %4
    loop [b: $B2] {  # loop_1
      $B2: {  # body
        ret %5
      }
    }
    ret 0i
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, LoadVar_ThenWriteToVarInLoopBody_ThenUseLoad) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var<function, i32>();
        b.Store(var, 1_i);
        auto* load = b.Load(var);
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            b.Store(var, 2_i);
            b.ExitLoop(loop);
        });
        b.Return(fn, load);
    });

    auto* src = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var undef
    store %2, 1i
    %3:i32 = load %2
    loop [b: $B2] {  # loop_1
      $B2: {  # body
        store %2, 2i
        exit_loop  # loop_1
      }
    }
    ret %3
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var undef
    store %2, 1i
    %3:i32 = load %2
    %4:i32 = let %3
    loop [b: $B2] {  # loop_1
      $B2: {  # body
        store %2, 2i
        exit_loop  # loop_1
      }
    }
    ret %4
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, UnsequencedOutsideLoopContinuing) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* v = b.Add(ty.i32(), 1_i, 2_i);
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.Continue(loop); });
        b.Append(loop->Continuing(), [&] { b.BreakIf(loop, b.Equal(ty.bool_(), v, 3_i)); });
        b.Return(fn, 0_i);
    });

    auto* src = R"(
%f = func():i32 {
  $B1: {
    %2:i32 = add 1i, 2i
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        continue  # -> $B3
      }
      $B3: {  # continuing
        %3:bool = eq %2, 3i
        break_if %3  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret 0i
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, SequencedOutsideLoopContinuing) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var<function, i32>();
        auto* v_1 = b.Load(var);
        auto* v_2 = b.Add(ty.i32(), v_1, 2_i);
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.Continue(loop); });
        b.Append(loop->Continuing(), [&] { b.BreakIf(loop, b.Equal(ty.bool_(), v_2, 3_i)); });
        b.Return(fn, 0_i);
    });

    auto* src = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var undef
    %3:i32 = load %2
    %4:i32 = add %3, 2i
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        continue  # -> $B3
      }
      $B3: {  # continuing
        %5:bool = eq %4, 3i
        break_if %5  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret 0i
  }
}
)";

    EXPECT_EQ(src, str());

    Run(ValueToLet);

    auto* expect = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var undef
    %3:i32 = load %2
    %4:i32 = add %3, 2i
    %5:i32 = let %4
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        continue  # -> $B3
      }
      $B3: {  # continuing
        %6:bool = eq %5, 3i
        break_if %6  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret 0i
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, LoadVar_ThenWriteToVarInLoopContinuing_ThenUseLoad) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var<function, i32>();
        b.Store(var, 1_i);
        auto* load = b.Load(var);
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.Continue(loop); });
        b.Append(loop->Continuing(), [&] {
            b.Store(var, 2_i);
            b.BreakIf(loop, true);
        });
        b.Return(fn, load);
    });

    auto* src = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var undef
    store %2, 1i
    %3:i32 = load %2
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        continue  # -> $B3
      }
      $B3: {  # continuing
        store %2, 2i
        break_if true  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret %3
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var undef
    store %2, 1i
    %3:i32 = load %2
    %4:i32 = let %3
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        continue  # -> $B3
      }
      $B3: {  # continuing
        store %2, 2i
        break_if true  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret %4
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, LoadVarInLoopInitializer_ThenReadAndWriteToVarInLoopBody) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var<function, i32>();
        b.Store(var, 1_i);
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            auto* load = b.Load(var);
            b.NextIteration(loop);
            b.Append(loop->Body(), [&] {
                b.Store(var, b.Add(ty.i32(), load, 1_i));
                b.ExitLoop(loop);
            });
        });
        b.Return(fn, 3_i);
    });

    auto* src = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var undef
    store %2, 1i
    loop [i: $B2, b: $B3] {  # loop_1
      $B2: {  # initializer
        %3:i32 = load %2
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %4:i32 = add %3, 1i
        store %2, %4
        exit_loop  # loop_1
      }
    }
    ret 3i
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var undef
    store %2, 1i
    loop [i: $B2, b: $B3] {  # loop_1
      $B2: {  # initializer
        %3:i32 = load %2
        %4:i32 = let %3
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %5:i32 = add %4, 1i
        store %2, %5
        exit_loop  # loop_1
      }
    }
    ret 3i
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, LoadVarInLoopInitializer_ThenReadAndWriteToVarInLoopContinuing) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var<function, i32>();
        b.Store(var, 1_i);
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            auto* load = b.Load(var);
            b.NextIteration(loop);
            b.Append(loop->Body(), [&] { b.Continue(loop); });
            b.Append(loop->Continuing(), [&] {
                b.Store(var, b.Add(ty.i32(), load, 1_i));
                b.BreakIf(loop, true);
            });
        });
        b.Return(fn, 3_i);
    });

    auto* src = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var undef
    store %2, 1i
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %3:i32 = load %2
        next_iteration  # -> $B3
      }
      $B3: {  # body
        continue  # -> $B4
      }
      $B4: {  # continuing
        %4:i32 = add %3, 1i
        store %2, %4
        break_if true  # -> [t: exit_loop loop_1, f: $B3]
      }
    }
    ret 3i
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var undef
    store %2, 1i
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %3:i32 = load %2
        %4:i32 = let %3
        next_iteration  # -> $B3
      }
      $B3: {  # body
        continue  # -> $B4
      }
      $B4: {  # continuing
        %5:i32 = add %4, 1i
        store %2, %5
        break_if true  # -> [t: exit_loop loop_1, f: $B3]
      }
    }
    ret 3i
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_ValueToLetTest, LoadVarInLoopBody_ThenReadAndWriteToVarInLoopContinuing) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var<function, i32>();
        b.Store(var, 1_i);
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            auto* load = b.Load(var);
            b.Continue(loop);

            b.Append(loop->Continuing(), [&] {
                b.Store(var, b.Add(ty.i32(), load, 1_i));
                b.BreakIf(loop, true);
            });
        });
        b.Return(fn, 3_i);
    });

    auto* src = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var undef
    store %2, 1i
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %3:i32 = load %2
        continue  # -> $B3
      }
      $B3: {  # continuing
        %4:i32 = add %3, 1i
        store %2, %4
        break_if true  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret 3i
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%f = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var undef
    store %2, 1i
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %3:i32 = load %2
        %4:i32 = let %3
        continue  # -> $B3
      }
      $B3: {  # continuing
        %5:i32 = add %4, 1i
        store %2, %5
        break_if true  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret 3i
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::wgsl::writer::raise

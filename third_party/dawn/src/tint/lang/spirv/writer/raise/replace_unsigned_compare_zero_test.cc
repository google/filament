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

#include "src/tint/lang/spirv/writer/raise/replace_unsigned_compare_zero.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::spirv::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using SpirvWriter_ReplaceUnsignedCompareZeroTest = core::ir::transform::TransformTest;

TEST_F(SpirvWriter_ReplaceUnsignedCompareZeroTest, NoModify_SignedEqualZero) {
    auto* val = b.FunctionParam("val", ty.i32());
    auto* func = b.Function("foo", ty.bool_());
    func->SetParams({val});

    b.Append(func->Block(), [&] {
        auto* result = b.Equal(val, 0_i);
        b.Return(func, result);
    });

    auto* expect = R"(
%foo = func(%val:i32):bool {
  $B1: {
    %3:bool = eq %val, 0i
    ret %3
  }
}
)";

    Run(ReplaceUnsignedCompareZero);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ReplaceUnsignedCompareZeroTest, NoModify_FloatEqualZero) {
    auto* val = b.FunctionParam("val", ty.f32());
    auto* func = b.Function("foo", ty.bool_());
    func->SetParams({val});

    b.Append(func->Block(), [&] {
        auto* result = b.Equal(val, 0_f);
        b.Return(func, result);
    });

    auto* expect = R"(
%foo = func(%val:f32):bool {
  $B1: {
    %3:bool = eq %val, 0.0f
    ret %3
  }
}
)";

    Run(ReplaceUnsignedCompareZero);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ReplaceUnsignedCompareZeroTest, NoModify_UnsignedEqualNonZero) {
    auto* val = b.FunctionParam("val", ty.u32());
    auto* func = b.Function("foo", ty.bool_());
    func->SetParams({val});

    b.Append(func->Block(), [&] {
        auto* result = b.Equal(val, 5_u);
        b.Return(func, result);
    });

    auto* expect = R"(
%foo = func(%val:u32):bool {
  $B1: {
    %3:bool = eq %val, 5u
    ret %3
  }
}
)";

    Run(ReplaceUnsignedCompareZero);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ReplaceUnsignedCompareZeroTest, NoModify_UnsignedLessThanZero) {
    auto* val = b.FunctionParam("val", ty.u32());
    auto* func = b.Function("foo", ty.bool_());
    func->SetParams({val});

    b.Append(func->Block(), [&] {
        auto* result = b.LessThan(val, 0_u);
        b.Return(func, result);
    });

    auto* expect = R"(
%foo = func(%val:u32):bool {
  $B1: {
    %3:bool = lt %val, 0u
    ret %3
  }
}
)";

    Run(ReplaceUnsignedCompareZero);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ReplaceUnsignedCompareZeroTest, Scalar_RhsZero) {
    auto* val = b.FunctionParam("val", ty.u32());
    auto* func = b.Function("foo", ty.bool_());
    func->SetParams({val});

    b.Append(func->Block(), [&] {
        auto* result = b.Equal(val, 0_u);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%val:u32):bool {
  $B1: {
    %3:bool = eq %val, 0u
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%val:u32):bool {
  $B1: {
    %3:bool = lt %val, 1u
    ret %3
  }
}
)";

    Run(ReplaceUnsignedCompareZero);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ReplaceUnsignedCompareZeroTest, Scalar_LhsZero) {
    auto* val = b.FunctionParam("val", ty.u32());
    auto* func = b.Function("foo", ty.bool_());
    func->SetParams({val});

    b.Append(func->Block(), [&] {
        auto* result = b.Equal(0_u, val);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%val:u32):bool {
  $B1: {
    %3:bool = eq 0u, %val
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%val:u32):bool {
  $B1: {
    %3:bool = gt 1u, %val
    ret %3
  }
}
)";

    Run(ReplaceUnsignedCompareZero);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ReplaceUnsignedCompareZeroTest, Vector_RhsZero_Vec2u) {
    auto* val = b.FunctionParam("val", ty.vec2u());
    auto* func = b.Function("foo", ty.vec2<bool>());
    func->SetParams({val});

    b.Append(func->Block(), [&] {
        auto* result = b.Equal(val, b.Zero<vec2<u32>>());
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%val:vec2<u32>):vec2<bool> {
  $B1: {
    %3:vec2<bool> = eq %val, vec2<u32>(0u)
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%val:vec2<u32>):vec2<bool> {
  $B1: {
    %3:vec2<bool> = lt %val, vec2<u32>(1u)
    ret %3
  }
}
)";

    Run(ReplaceUnsignedCompareZero);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ReplaceUnsignedCompareZeroTest, Vector_LhsZero_Vec3u) {
    auto* val = b.FunctionParam("val", ty.vec3u());
    auto* func = b.Function("foo", ty.vec3<bool>());
    func->SetParams({val});

    b.Append(func->Block(), [&] {
        auto* result = b.Equal(b.Zero<vec3<u32>>(), val);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%val:vec3<u32>):vec3<bool> {
  $B1: {
    %3:vec3<bool> = eq vec3<u32>(0u), %val
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%val:vec3<u32>):vec3<bool> {
  $B1: {
    %3:vec3<bool> = gt vec3<u32>(1u), %val
    ret %3
  }
}
)";

    Run(ReplaceUnsignedCompareZero);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ReplaceUnsignedCompareZeroTest, Vector_RhsZero_Vec4u) {
    auto* val = b.FunctionParam("val", ty.vec4u());
    auto* func = b.Function("foo", ty.vec4<bool>());
    func->SetParams({val});

    b.Append(func->Block(), [&] {
        auto* result = b.Equal(val, b.Zero<vec4<u32>>());
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%val:vec4<u32>):vec4<bool> {
  $B1: {
    %3:vec4<bool> = eq %val, vec4<u32>(0u)
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%val:vec4<u32>):vec4<bool> {
  $B1: {
    %3:vec4<bool> = lt %val, vec4<u32>(1u)
    ret %3
  }
}
)";

    Run(ReplaceUnsignedCompareZero);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ReplaceUnsignedCompareZeroTest, BothZero) {
    auto* func = b.Function("foo", ty.bool_());

    b.Append(func->Block(), [&] {
        auto* result = b.Equal(0_u, 0_u);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func():bool {
  $B1: {
    %2:bool = eq 0u, 0u
    ret %2
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():bool {
  $B1: {
    %2:bool = lt 0u, 1u
    ret %2
  }
}
)";

    Run(ReplaceUnsignedCompareZero);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::spirv::writer::raise

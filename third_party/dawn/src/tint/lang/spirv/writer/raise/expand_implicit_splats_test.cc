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

#include "src/tint/lang/spirv/writer/raise/expand_implicit_splats.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::spirv::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using SpirvWriter_ExpandImplicitSplatsTest = core::ir::transform::TransformTest;

TEST_F(SpirvWriter_ExpandImplicitSplatsTest, NoModify_Construct_VectorIdentity) {
    auto* vector = b.FunctionParam("vector", ty.vec2i());
    auto* func = b.Function("foo", ty.vec2i());
    func->SetParams({vector});

    b.Append(func->Block(), [&] {
        auto* result = b.Construct(ty.vec2i(), vector);
        b.Return(func, result);
    });

    auto* expect = R"(
%foo = func(%vector:vec2<i32>):vec2<i32> {
  $B1: {
    %3:vec2<i32> = construct %vector
    ret %3
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ExpandImplicitSplatsTest, NoModify_Construct_MixedScalarVector) {
    auto* scalar = b.FunctionParam("scalar", ty.i32());
    auto* vector = b.FunctionParam("vector", ty.vec2i());
    auto* func = b.Function("foo", ty.vec3i());
    func->SetParams({scalar, vector});

    b.Append(func->Block(), [&] {
        auto* result = b.Construct(ty.vec3i(), scalar, vector);
        b.Return(func, result);
    });

    auto* expect = R"(
%foo = func(%scalar:i32, %vector:vec2<i32>):vec3<i32> {
  $B1: {
    %4:vec3<i32> = construct %scalar, %vector
    ret %4
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ExpandImplicitSplatsTest, NoModify_Construct_AllScalars) {
    auto* scalar = b.FunctionParam("scalar", ty.i32());
    auto* func = b.Function("foo", ty.vec3i());
    func->SetParams({scalar});

    b.Append(func->Block(), [&] {
        auto* result = b.Construct(ty.vec3i(), scalar, scalar, scalar);
        b.Return(func, result);
    });

    auto* expect = R"(
%foo = func(%scalar:i32):vec3<i32> {
  $B1: {
    %3:vec3<i32> = construct %scalar, %scalar, %scalar
    ret %3
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ExpandImplicitSplatsTest, Construct_Splat_Vec2i) {
    auto* scalar = b.FunctionParam("scalar", ty.i32());
    auto* func = b.Function("foo", ty.vec2i());
    func->SetParams({scalar});

    b.Append(func->Block(), [&] {
        auto* result = b.Construct(ty.vec2i(), scalar);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%scalar:i32):vec2<i32> {
  $B1: {
    %3:vec2<i32> = construct %scalar
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%scalar:i32):vec2<i32> {
  $B1: {
    %3:vec2<i32> = construct %scalar, %scalar
    ret %3
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ExpandImplicitSplatsTest, Construct_Splat_Vec3u) {
    auto* scalar = b.FunctionParam("scalar", ty.u32());
    auto* func = b.Function("foo", ty.vec3u());
    func->SetParams({scalar});

    b.Append(func->Block(), [&] {
        auto* result = b.Construct(ty.vec3u(), scalar);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%scalar:u32):vec3<u32> {
  $B1: {
    %3:vec3<u32> = construct %scalar
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%scalar:u32):vec3<u32> {
  $B1: {
    %3:vec3<u32> = construct %scalar, %scalar, %scalar
    ret %3
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ExpandImplicitSplatsTest, Construct_Splat_Vec4f) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* func = b.Function("foo", ty.vec4f());
    func->SetParams({scalar});

    b.Append(func->Block(), [&] {
        auto* result = b.Construct(ty.vec4f(), scalar);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%scalar:f32):vec4<f32> {
  $B1: {
    %3:vec4<f32> = construct %scalar
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%scalar:f32):vec4<f32> {
  $B1: {
    %3:vec4<f32> = construct %scalar, %scalar, %scalar, %scalar
    ret %3
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ExpandImplicitSplatsTest, BinaryAdd_VectorScalar_Vec4f) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* vector = b.FunctionParam("vector", ty.vec4f());
    auto* func = b.Function("foo", ty.vec4f());
    func->SetParams({scalar, vector});

    b.Append(func->Block(), [&] {
        auto* result = b.Add(vector, scalar);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> {
  $B1: {
    %4:vec4<f32> = add %vector, %scalar
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> {
  $B1: {
    %4:vec4<f32> = construct %scalar, %scalar, %scalar, %scalar
    %5:vec4<f32> = add %vector, %4
    ret %5
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ExpandImplicitSplatsTest, BinaryAdd_ScalarVector_Vec4f) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* vector = b.FunctionParam("vector", ty.vec4f());
    auto* func = b.Function("foo", ty.vec4f());
    func->SetParams({scalar, vector});

    b.Append(func->Block(), [&] {
        auto* result = b.Add(scalar, vector);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> {
  $B1: {
    %4:vec4<f32> = add %scalar, %vector
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> {
  $B1: {
    %4:vec4<f32> = construct %scalar, %scalar, %scalar, %scalar
    %5:vec4<f32> = add %4, %vector
    ret %5
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ExpandImplicitSplatsTest, BinarySubtract_VectorScalar_Vec4f) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* vector = b.FunctionParam("vector", ty.vec4f());
    auto* func = b.Function("foo", ty.vec4f());
    func->SetParams({scalar, vector});

    b.Append(func->Block(), [&] {
        auto* result = b.Subtract(vector, scalar);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> {
  $B1: {
    %4:vec4<f32> = sub %vector, %scalar
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> {
  $B1: {
    %4:vec4<f32> = construct %scalar, %scalar, %scalar, %scalar
    %5:vec4<f32> = sub %vector, %4
    ret %5
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ExpandImplicitSplatsTest, BinarySubtract_ScalarVector_Vec4f) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* vector = b.FunctionParam("vector", ty.vec4f());
    auto* func = b.Function("foo", ty.vec4f());
    func->SetParams({scalar, vector});

    b.Append(func->Block(), [&] {
        auto* result = b.Subtract(scalar, vector);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> {
  $B1: {
    %4:vec4<f32> = sub %scalar, %vector
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> {
  $B1: {
    %4:vec4<f32> = construct %scalar, %scalar, %scalar, %scalar
    %5:vec4<f32> = sub %4, %vector
    ret %5
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ExpandImplicitSplatsTest, BinaryDivide_VectorScalar_Vec4f) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* vector = b.FunctionParam("vector", ty.vec4f());
    auto* func = b.Function("foo", ty.vec4f());
    func->SetParams({scalar, vector});

    b.Append(func->Block(), [&] {
        auto* result = b.Divide(vector, scalar);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> {
  $B1: {
    %4:vec4<f32> = div %vector, %scalar
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> {
  $B1: {
    %4:vec4<f32> = construct %scalar, %scalar, %scalar, %scalar
    %5:vec4<f32> = div %vector, %4
    ret %5
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ExpandImplicitSplatsTest, BinaryDivide_ScalarVector_Vec4f) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* vector = b.FunctionParam("vector", ty.vec4f());
    auto* func = b.Function("foo", ty.vec4f());
    func->SetParams({scalar, vector});

    b.Append(func->Block(), [&] {
        auto* result = b.Divide(scalar, vector);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> {
  $B1: {
    %4:vec4<f32> = div %scalar, %vector
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> {
  $B1: {
    %4:vec4<f32> = construct %scalar, %scalar, %scalar, %scalar
    %5:vec4<f32> = div %4, %vector
    ret %5
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ExpandImplicitSplatsTest, BinaryModulo_VectorScalar_Vec4f) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* vector = b.FunctionParam("vector", ty.vec4f());
    auto* func = b.Function("foo", ty.vec4f());
    func->SetParams({scalar, vector});

    b.Append(func->Block(), [&] {
        auto* result = b.Modulo(vector, scalar);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> {
  $B1: {
    %4:vec4<f32> = mod %vector, %scalar
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> {
  $B1: {
    %4:vec4<f32> = construct %scalar, %scalar, %scalar, %scalar
    %5:vec4<f32> = mod %vector, %4
    ret %5
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ExpandImplicitSplatsTest, BinaryModulo_ScalarVector_Vec4f) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* vector = b.FunctionParam("vector", ty.vec4f());
    auto* func = b.Function("foo", ty.vec4f());
    func->SetParams({scalar, vector});

    b.Append(func->Block(), [&] {
        auto* result = b.Modulo(scalar, vector);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> {
  $B1: {
    %4:vec4<f32> = mod %scalar, %vector
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> {
  $B1: {
    %4:vec4<f32> = construct %scalar, %scalar, %scalar, %scalar
    %5:vec4<f32> = mod %4, %vector
    ret %5
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ExpandImplicitSplatsTest, BinaryMultiply_VectorScalar_Vec4f) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* vector = b.FunctionParam("vector", ty.vec4f());
    auto* func = b.Function("foo", ty.vec4f());
    func->SetParams({scalar, vector});

    b.Append(func->Block(), [&] {
        auto* result = b.Multiply(vector, scalar);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> {
  $B1: {
    %4:vec4<f32> = mul %vector, %scalar
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> {
  $B1: {
    %4:vec4<f32> = spirv.vector_times_scalar %vector, %scalar
    ret %4
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ExpandImplicitSplatsTest, BinaryMultiply_ScalarVector_Vec4f) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* vector = b.FunctionParam("vector", ty.vec4f());
    auto* func = b.Function("foo", ty.vec4f());
    func->SetParams({scalar, vector});

    b.Append(func->Block(), [&] {
        auto* result = b.Multiply(scalar, vector);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> {
  $B1: {
    %4:vec4<f32> = mul %scalar, %vector
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> {
  $B1: {
    %4:vec4<f32> = spirv.vector_times_scalar %vector, %scalar
    ret %4
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ExpandImplicitSplatsTest, BinaryMultiply_VectorScalar_Vec4i) {
    auto* scalar = b.FunctionParam("scalar", ty.i32());
    auto* vector = b.FunctionParam("vector", ty.vec4i());
    auto* func = b.Function("foo", ty.vec4i());
    func->SetParams({scalar, vector});

    b.Append(func->Block(), [&] {
        auto* result = b.Multiply(vector, scalar);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%scalar:i32, %vector:vec4<i32>):vec4<i32> {
  $B1: {
    %4:vec4<i32> = mul %vector, %scalar
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%scalar:i32, %vector:vec4<i32>):vec4<i32> {
  $B1: {
    %4:vec4<i32> = construct %scalar, %scalar, %scalar, %scalar
    %5:vec4<i32> = mul %vector, %4
    ret %5
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ExpandImplicitSplatsTest, BinaryMultiply_ScalarVector_Vec4i) {
    auto* scalar = b.FunctionParam("scalar", ty.i32());
    auto* vector = b.FunctionParam("vector", ty.vec4i());
    auto* func = b.Function("foo", ty.vec4i());
    func->SetParams({scalar, vector});

    b.Append(func->Block(), [&] {
        auto* result = b.Multiply(scalar, vector);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%scalar:i32, %vector:vec4<i32>):vec4<i32> {
  $B1: {
    %4:vec4<i32> = mul %scalar, %vector
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%scalar:i32, %vector:vec4<i32>):vec4<i32> {
  $B1: {
    %4:vec4<i32> = construct %scalar, %scalar, %scalar, %scalar
    %5:vec4<i32> = mul %4, %vector
    ret %5
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ExpandImplicitSplatsTest, Mix_VectorOperands_ScalarFactor) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec4f());
    auto* arg2 = b.FunctionParam("arg2", ty.vec4f());
    auto* factor = b.FunctionParam("factor", ty.f32());
    auto* func = b.Function("foo", ty.vec4f());
    func->SetParams({arg1, arg2, factor});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4f(), core::BuiltinFn::kMix, arg1, arg2, factor);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%arg1:vec4<f32>, %arg2:vec4<f32>, %factor:f32):vec4<f32> {
  $B1: {
    %5:vec4<f32> = mix %arg1, %arg2, %factor
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%arg1:vec4<f32>, %arg2:vec4<f32>, %factor:f32):vec4<f32> {
  $B1: {
    %5:vec4<f32> = construct %factor, %factor, %factor, %factor
    %6:vec4<f32> = mix %arg1, %arg2, %5
    ret %6
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::spirv::writer::raise

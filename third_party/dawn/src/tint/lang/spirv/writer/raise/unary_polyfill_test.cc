// Copyright 2025 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/writer/raise/unary_polyfill.h"

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::spirv::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using SpirvWriter_UnaryPolyfillTest = core::ir::transform::TransformTest;

TEST_F(SpirvWriter_UnaryPolyfillTest, Negation_Scalar) {
    auto* arg = b.FunctionParam("arg", ty.f32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({arg});

    b.Append(func->Block(), [&] {
        auto* result = b.Negation(arg);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%arg:f32):f32 {
  $B1: {
    %3:f32 = negation %arg
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%arg:f32):f32 {
  $B1: {
    %3:u32 = bitcast<u32> %arg
    %4:u32 = xor %3, 2147483648u
    %5:f32 = bitcast<f32> %4
    ret %5
  }
}
)";

    UnaryPolyfillConfig config;
    config.polyfill_f32_negation = true;
    Run(UnaryPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_UnaryPolyfillTest, Negation_Vector) {
    auto* arg = b.FunctionParam("arg", ty.vec4f());
    auto* func = b.Function("foo", ty.vec4f());
    func->SetParams({arg});

    b.Append(func->Block(), [&] {
        auto* result = b.Negation(arg);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%arg:vec4<f32>):vec4<f32> {
  $B1: {
    %3:vec4<f32> = negation %arg
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%arg:vec4<f32>):vec4<f32> {
  $B1: {
    %3:vec4<u32> = bitcast<vec4<u32>> %arg
    %4:vec4<u32> = xor %3, vec4<u32>(2147483648u)
    %5:vec4<f32> = bitcast<vec4<f32>> %4
    ret %5
  }
}
)";

    UnaryPolyfillConfig config;
    config.polyfill_f32_negation = true;
    Run(UnaryPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_UnaryPolyfillTest, Abs_Scalar) {
    auto* arg = b.FunctionParam("arg", ty.f32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({arg});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.f32(), core::BuiltinFn::kAbs, arg);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%arg:f32):f32 {
  $B1: {
    %3:f32 = abs %arg
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%arg:f32):f32 {
  $B1: {
    %3:u32 = bitcast<u32> %arg
    %4:u32 = and %3, 2147483647u
    %5:f32 = bitcast<f32> %4
    ret %5
  }
}
)";

    UnaryPolyfillConfig config;
    config.polyfill_f32_abs = true;
    Run(UnaryPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_UnaryPolyfillTest, Abs_Vector) {
    auto* arg = b.FunctionParam("arg", ty.vec4f());
    auto* func = b.Function("foo", ty.vec4f());
    func->SetParams({arg});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4f(), core::BuiltinFn::kAbs, arg);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%arg:vec4<f32>):vec4<f32> {
  $B1: {
    %3:vec4<f32> = abs %arg
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%arg:vec4<f32>):vec4<f32> {
  $B1: {
    %3:vec4<u32> = bitcast<vec4<u32>> %arg
    %4:vec4<u32> = and %3, vec4<u32>(2147483647u)
    %5:vec4<f32> = bitcast<vec4<f32>> %4
    ret %5
  }
}
)";

    UnaryPolyfillConfig config;
    config.polyfill_f32_abs = true;
    Run(UnaryPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_UnaryPolyfillTest, Negation_NoPolyfill) {
    auto* arg = b.FunctionParam("arg", ty.f32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({arg});

    b.Append(func->Block(), [&] {
        auto* result = b.Negation(arg);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%arg:f32):f32 {
  $B1: {
    %3:f32 = negation %arg
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    UnaryPolyfillConfig config;
    config.polyfill_f32_negation = false;
    Run(UnaryPolyfill, config);

    EXPECT_EQ(src, str());
}

TEST_F(SpirvWriter_UnaryPolyfillTest, Abs_NoPolyfill) {
    auto* arg = b.FunctionParam("arg", ty.f32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({arg});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.f32(), core::BuiltinFn::kAbs, arg);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%arg:f32):f32 {
  $B1: {
    %3:f32 = abs %arg
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    UnaryPolyfillConfig config;
    config.polyfill_f32_abs = false;
    Run(UnaryPolyfill, config);

    EXPECT_EQ(src, str());
}

}  // namespace
}  // namespace tint::spirv::writer::raise

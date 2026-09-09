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

#include "src/tint/lang/hlsl/writer/raise/decompose_snorm10_10_10_2.h"

#include <gtest/gtest.h>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer::raise {
namespace {

class HlslWriterDecomposeSnorm10_10_10_2Test : public core::ir::transform::TransformTest {
  protected:
    core::IOAttributes Location(uint32_t loc) {
        core::IOAttributes attrs;
        attrs.location = loc;
        return attrs;
    }
};

TEST_F(HlslWriterDecomposeSnorm10_10_10_2Test, NoLocations) {
    auto* inputs =
        ty.Struct(mod.symbols.New("Inputs"), {
                                                 {mod.symbols.New("pos"), ty.vec4f(), Location(0u)},
                                             });

    auto* func = b.Function("foo", ty.vec4f(), core::ir::Function::PipelineStage::kVertex);
    func->SetReturnBuiltin(core::BuiltinValue::kPosition);
    auto* param = b.FunctionParam("input", inputs);
    func->AppendParam(param);

    b.Append(func->Block(), [&] {
        auto* pos_val = b.Access(ty.vec4f(), param, 0_u);
        b.Return(func, pos_val);
    });

    auto* src = R"(
Inputs = struct @align(16) {
  pos:vec4<f32> @offset(0), @location(0)
}

%foo = @vertex func(%input:Inputs):vec4<f32> [@position] {
  $B1: {
    %3:vec4<f32> = access %input, 0u
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;
    Run(DecomposeSnorm10_10_10_2, std::vector<uint32_t>{});

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeSnorm10_10_10_2Test, DecomposeLocation0) {
    auto* inputs = ty.Struct(mod.symbols.New("Inputs"),
                             {
                                 {mod.symbols.New("pos"), ty.vec4f(), Location(0u)},
                                 {mod.symbols.New("norm"), ty.vec4f(), Location(1u)},
                             });

    auto* func = b.Function("foo", ty.vec4f(), core::ir::Function::PipelineStage::kVertex);
    func->SetReturnBuiltin(core::BuiltinValue::kPosition);
    auto* param = b.FunctionParam("input", inputs);
    func->AppendParam(param);

    b.Append(func->Block(), [&] {
        auto* pos_val = b.Access(ty.vec4f(), param, 0_u);
        b.Return(func, pos_val);
    });

    auto* src = R"(
Inputs = struct @align(16) {
  pos:vec4<f32> @offset(0), @location(0)
  norm:vec4<f32> @offset(16), @location(1)
}

%foo = @vertex func(%input:Inputs):vec4<f32> [@position] {
  $B1: {
    %3:vec4<f32> = access %input, 0u
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inputs = struct @align(16) {
  pos:vec4<f32> @offset(0), @location(0)
  norm:vec4<f32> @offset(16), @location(1)
}

%foo = @vertex func(%input:Inputs):vec4<f32> [@position] {
  $B1: {
    %3:vec4<f32> = access %input, 0u
    %4:vec4<f32> = mul %3, vec4<f32>(1023.0f, 1023.0f, 1023.0f, 3.0f)
    %5:vec4<f32> = round %4
    %6:vec4<i32> = convert %5
    %7:vec4<i32> = shl %6, vec4<u32>(22u, 22u, 22u, 30u)
    %8:vec4<i32> = shr %7, vec4<u32>(22u, 22u, 22u, 30u)
    %9:vec4<f32> = convert %8
    %10:vec4<f32> = div %9, vec4<f32>(511.0f, 511.0f, 511.0f, 1.0f)
    %11:vec4<f32> = max %10, vec4<f32>(-1.0f)
    ret %11
  }
}
)";

    Run(DecomposeSnorm10_10_10_2, std::vector<uint32_t>{0u});

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeSnorm10_10_10_2Test, DecomposeStructMember) {
    auto* inputs = ty.Struct(mod.symbols.New("Inputs"),
                             {
                                 {mod.symbols.New("pos"), ty.vec4f(), Location(0u)},
                                 {mod.symbols.New("norm"), ty.vec4f(), Location(1u)},
                             });

    auto* func = b.Function("foo", ty.vec4f(), core::ir::Function::PipelineStage::kVertex);
    func->SetReturnBuiltin(core::BuiltinValue::kPosition);
    auto* param = b.FunctionParam("input", inputs);
    func->AppendParam(param);

    b.Append(func->Block(), [&] {
        auto* norm_val = b.Access(ty.vec4f(), param, 1_u);
        b.Return(func, norm_val);
    });

    auto* src = R"(
Inputs = struct @align(16) {
  pos:vec4<f32> @offset(0), @location(0)
  norm:vec4<f32> @offset(16), @location(1)
}

%foo = @vertex func(%input:Inputs):vec4<f32> [@position] {
  $B1: {
    %3:vec4<f32> = access %input, 1u
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inputs = struct @align(16) {
  pos:vec4<f32> @offset(0), @location(0)
  norm:vec4<f32> @offset(16), @location(1)
}

%foo = @vertex func(%input:Inputs):vec4<f32> [@position] {
  $B1: {
    %3:vec4<f32> = access %input, 1u
    %4:vec4<f32> = mul %3, vec4<f32>(1023.0f, 1023.0f, 1023.0f, 3.0f)
    %5:vec4<f32> = round %4
    %6:vec4<i32> = convert %5
    %7:vec4<i32> = shl %6, vec4<u32>(22u, 22u, 22u, 30u)
    %8:vec4<i32> = shr %7, vec4<u32>(22u, 22u, 22u, 30u)
    %9:vec4<f32> = convert %8
    %10:vec4<f32> = div %9, vec4<f32>(511.0f, 511.0f, 511.0f, 1.0f)
    %11:vec4<f32> = max %10, vec4<f32>(-1.0f)
    ret %11
  }
}
)";

    Run(DecomposeSnorm10_10_10_2, std::vector<uint32_t>{1u});

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::hlsl::writer::raise

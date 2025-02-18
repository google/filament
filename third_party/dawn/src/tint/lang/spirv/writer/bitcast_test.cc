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

#include "src/tint/lang/spirv/writer/common/helper_test.h"

namespace tint::spirv::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

/// A parameterized test case.
struct BitcastCase {
    /// The input type.
    TestElementType in;
    /// The output type.
    TestElementType out;
    /// The expected SPIR-V result type name.
    std::string spirv_type_name;
};
std::string PrintCase(testing::TestParamInfo<BitcastCase> cc) {
    StringStream ss;
    ss << cc.param.in << "_to_" << cc.param.out;
    return ss.str();
}

using Bitcast = SpirvWriterTestWithParam<BitcastCase>;
TEST_P(Bitcast, Scalar) {
    auto& params = GetParam();
    auto* func = b.Function("foo", MakeScalarType(params.out));
    func->SetParams({b.FunctionParam("arg", MakeScalarType(params.in))});
    b.Append(func->Block(), [&] {
        auto* result = b.Bitcast(MakeScalarType(params.out), func->Params()[0]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    if (params.in == params.out) {
        EXPECT_INST("OpReturnValue %arg");
    } else {
        EXPECT_INST("%result = OpBitcast %" + params.spirv_type_name + " %arg");
    }
}
TEST_P(Bitcast, Vector) {
    auto& params = GetParam();
    auto* func = b.Function("foo", MakeVectorType(params.out));
    func->SetParams({b.FunctionParam("arg", MakeVectorType(params.in))});
    b.Append(func->Block(), [&] {
        auto* result = b.Bitcast(MakeVectorType(params.out), func->Params()[0]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    if (params.in == params.out) {
        EXPECT_INST("OpReturnValue %arg");
    } else {
        EXPECT_INST("%result = OpBitcast %v2" + params.spirv_type_name + " %arg");
    }
}
INSTANTIATE_TEST_SUITE_P(SpirvWriterTest,
                         Bitcast,
                         testing::Values(
                             // To f32.
                             BitcastCase{kF32, kF32, "float"},
                             BitcastCase{kI32, kF32, "float"},
                             BitcastCase{kU32, kF32, "float"},

                             // To f16.
                             BitcastCase{kF16, kF16, "half"},

                             // To i32.
                             BitcastCase{kF32, kI32, "int"},
                             BitcastCase{kI32, kI32, "int"},
                             BitcastCase{kU32, kI32, "int"},

                             // To u32.
                             BitcastCase{kF32, kU32, "uint"},
                             BitcastCase{kI32, kU32, "uint"},
                             BitcastCase{kU32, kU32, "uint"}),
                         PrintCase);

TEST_F(SpirvWriterTest, Bitcast_u32_to_vec2h) {
    auto* func = b.Function("foo", ty.vec2<f16>());
    func->SetParams({b.FunctionParam("arg", ty.u32())});
    b.Append(func->Block(), [&] {
        auto* result = b.Bitcast(ty.vec2<f16>(), func->Params()[0]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpBitcast %v2half %arg");
}

TEST_F(SpirvWriterTest, Bitcast_vec2i_to_vec4h) {
    auto* func = b.Function("foo", ty.vec4<f16>());
    func->SetParams({b.FunctionParam("arg", ty.vec2<i32>())});
    b.Append(func->Block(), [&] {
        auto* result = b.Bitcast(ty.vec4<f16>(), func->Params()[0]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpBitcast %v4half %arg");
}

TEST_F(SpirvWriterTest, Bitcast_vec2h_to_u32) {
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({b.FunctionParam("arg", ty.vec2<f16>())});
    b.Append(func->Block(), [&] {
        auto* result = b.Bitcast(ty.u32(), func->Params()[0]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpBitcast %uint %arg");
}

TEST_F(SpirvWriterTest, Bitcast_vec4h_to_vec2i) {
    auto* func = b.Function("foo", ty.vec2<i32>());
    func->SetParams({b.FunctionParam("arg", ty.vec4<f16>())});
    b.Append(func->Block(), [&] {
        auto* result = b.Bitcast(ty.vec2<i32>(), func->Params()[0]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpBitcast %v2int %arg");
}

}  // namespace
}  // namespace tint::spirv::writer

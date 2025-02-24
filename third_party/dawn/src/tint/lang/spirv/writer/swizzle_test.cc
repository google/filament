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

TEST_F(SpirvWriterTest, Swizzle_TwoElements) {
    auto* vec = b.FunctionParam("vec", ty.vec4<i32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({vec});
    b.Append(func->Block(), [&] {
        auto* result = b.Swizzle(ty.vec2<i32>(), vec, {3_u, 2_u});
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpVectorShuffle %v2int %vec %vec 3 2");
}

TEST_F(SpirvWriterTest, Swizzle_ThreeElements) {
    auto* vec = b.FunctionParam("vec", ty.vec4<i32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({vec});
    b.Append(func->Block(), [&] {
        auto* result = b.Swizzle(ty.vec3<i32>(), vec, {3_u, 2_u, 1_u});
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpVectorShuffle %v3int %vec %vec 3 2 1");
}

TEST_F(SpirvWriterTest, Swizzle_FourElements) {
    auto* vec = b.FunctionParam("vec", ty.vec4<i32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({vec});
    b.Append(func->Block(), [&] {
        auto* result = b.Swizzle(ty.vec4<i32>(), vec, {3_u, 2_u, 1_u, 0u});
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpVectorShuffle %v4int %vec %vec 3 2 1 0");
}

TEST_F(SpirvWriterTest, Swizzle_RepeatedElements) {
    auto* vec = b.FunctionParam("vec", ty.vec4<i32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({vec});
    b.Append(func->Block(), [&] {
        auto* result = b.Swizzle(ty.vec4<i32>(), vec, {1_u, 3_u, 1_u, 3_u});
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpVectorShuffle %v4int %vec %vec 1 3 1 3");
}

}  // namespace
}  // namespace tint::spirv::writer

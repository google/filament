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

#include "src/tint/lang/hlsl/writer/helper_test.h"

#include "gmock/gmock.h"

namespace tint::hlsl::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(HlslWriterTest, StripAllNames) {
    auto* str =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.Register("a"), ty.i32()},
                                                   {mod.symbols.Register("b"), ty.vec4<i32>()},
                                               });
    auto* foo = b.Function("foo", ty.u32());
    auto* param = b.FunctionParam("param", ty.u32());
    foo->AppendParam(param);
    b.Append(foo->Block(), [&] {  //
        b.Return(foo, param);
    });

    auto* func = b.ComputeFunction("main");
    auto* idx = b.FunctionParam("idx", ty.u32());
    idx->SetBuiltin(core::BuiltinValue::kLocalInvocationIndex);
    func->AppendParam(idx);
    b.Append(func->Block(), [&] {  //
        auto* var = b.Var("str", ty.ptr<function>(str));
        auto* val = b.Load(var);
        mod.SetName(val, "val");
        auto* a = b.Access<i32>(val, 0_u);
        mod.SetName(a, "a");
        b.Let("let", b.Call<u32>(foo, idx));
        b.Return(func);
    });

    Options options;
    options.remapped_entry_point_name = "tint_entry_point";
    options.strip_all_names = true;
    ASSERT_TRUE(Generate(options)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct tint_struct {
  int tint_member;
  int4 tint_member_1;
};

struct tint_struct_1 {
  uint tint_member_2 : SV_GroupIndex;
};


uint v(uint v_1) {
  return v_1;
}

void v_2(uint v_3) {
  tint_struct v_4 = (tint_struct)0;
  tint_struct v_5 = v_4;
  uint v_6 = v(v_3);
}

[numthreads(1, 1, 1)]
void tint_entry_point(tint_struct_1 v_8) {
  v_2(v_8.tint_member_2);
}

)");
}

}  // namespace
}  // namespace tint::hlsl::writer

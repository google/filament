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

#include "src/tint/lang/core/ir/binary/decode.h"

#include <gmock/gmock.h>

#include <string>
#include <utility>

#include "src/tint/lang/core/ir/binary/encode.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/utils/macros/compiler.h"

TINT_BEGIN_DISABLE_PROTOBUF_WARNINGS();
#include "src/tint/utils/protos/ir/ir.pb.h"
TINT_END_DISABLE_PROTOBUF_WARNINGS();

namespace tint::core::ir::binary {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

using IRBinaryDecodeTest = IRTestParamHelper<testing::Test>;

TEST_F(IRBinaryDecodeTest, InvalidFunctionName) {
    auto* fn = b.Function("Function", ty.void_());
    b.Append(fn->Block(), [&] { b.Return(fn); });

    auto res = EncodeToProto(mod);
    ASSERT_EQ(res, Success);

    auto pb_mod = std::move(res.Get());
    pb_mod->mutable_functions(0)->set_name("0invalid");  // Starts with a digit

    auto decoded = Decode(*pb_mod);
    EXPECT_NE(decoded, Success);
    EXPECT_THAT(decoded.Failure().reason,
                testing::HasSubstr("function name '0invalid' is not a valid WGSL identifier"));
}

TEST_F(IRBinaryDecodeTest, StripInvalidFunctionName) {
    auto* fn = b.Function("Function", ty.void_());
    b.Append(fn->Block(), [&] { b.Return(fn); });

    auto res = EncodeToProto(mod);
    ASSERT_EQ(res, Success);

    auto pb_mod = std::move(res.Get());
    pb_mod->mutable_functions(0)->set_name("0invalid-name");

    DecoderOptions options;
    options.strip_invalid_identifiers = true;
    auto decoded = Decode(*pb_mod, options);
    ASSERT_EQ(decoded, Success);
    EXPECT_FALSE(decoded->NameOf(decoded->functions[0]).IsValid());
}

TEST_F(IRBinaryDecodeTest, InvalidStructName) {
    Vector members{
        ty.Get<core::type::StructMember>(b.ir.symbols.New("a"), ty.i32(), 0u, 0u, 4u, 4u,
                                         core::IOAttributes{}),
    };
    auto* S = ty.Struct(b.ir.symbols.New("S"), std::move(members));
    b.Append(b.ir.root_block, [&] { b.Var(ty.ptr<function, read_write>(S)); });

    auto res = EncodeToProto(mod);
    ASSERT_EQ(res, Success);

    auto pb_mod = std::move(res.Get());
    pb_mod->mutable_types(pb_mod->types_size() - 1)->mutable_struct_()->set_name("__invalid");

    auto decoded = Decode(*pb_mod);
    EXPECT_NE(decoded, Success);
    EXPECT_THAT(decoded.Failure().reason,
                testing::HasSubstr("struct name '__invalid' is not a valid WGSL identifier"));
}

TEST_F(IRBinaryDecodeTest, StripInvalidStructName) {
    auto* S = ty.Struct(b.ir.symbols.New("S"), {{b.ir.symbols.New("a"), ty.i32()}});
    b.Append(b.ir.root_block, [&] { b.Var(ty.ptr<function, read_write>(S)); });

    auto res = EncodeToProto(mod);
    ASSERT_EQ(res, Success);

    auto pb_mod = std::move(res.Get());
    bool found_proto = false;
    for (int i = 0; i < pb_mod->types_size(); ++i) {
        if (pb_mod->types(i).has_struct_()) {
            pb_mod->mutable_types(i)->mutable_struct_()->set_name("__invalid");
            found_proto = true;
            break;
        }
    }
    ASSERT_TRUE(found_proto);

    DecoderOptions options;
    options.strip_invalid_identifiers = true;
    auto decoded = Decode(*pb_mod, options);
    ASSERT_EQ(decoded, Success);

    // Find the struct type and check its name.
    bool found = false;
    for (auto* type : decoded->Types()) {
        if (auto* str = type->As<core::type::Struct>()) {
            EXPECT_TRUE(str->Name().IsValid());
            EXPECT_NE(str->Name().Name(), "__invalid");
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

TEST_F(IRBinaryDecodeTest, StripInvalidStructMemberNames) {
    auto* S = ty.Struct(b.ir.symbols.New("S"), {
                                                   {b.ir.symbols.New("a"), ty.i32()},
                                                   {b.ir.symbols.New("b"), ty.i32()},
                                               });
    b.Append(b.ir.root_block, [&] { b.Var(ty.ptr<function, read_write>(S)); });

    auto res = EncodeToProto(mod);
    ASSERT_EQ(res, Success);

    auto pb_mod = std::move(res.Get());
    bool found_proto = false;
    for (int i = 0; i < pb_mod->types_size(); ++i) {
        if (pb_mod->types(i).has_struct_()) {
            auto* s = pb_mod->mutable_types(i)->mutable_struct_();
            s->mutable_member(0)->set_name("0invalid");
            s->mutable_member(1)->set_name("1invalid");
            found_proto = true;
            break;
        }
    }
    ASSERT_TRUE(found_proto);

    DecoderOptions options;
    options.strip_invalid_identifiers = true;
    auto decoded = Decode(*pb_mod, options);
    ASSERT_EQ(decoded, Success);

    // Find the struct type and check its member names.
    bool found = false;
    for (auto* type : decoded->Types()) {
        if (auto* str = type->As<core::type::Struct>()) {
            ASSERT_EQ(str->Members().Length(), 2u);
            EXPECT_TRUE(str->Members()[0]->Name().IsValid());
            EXPECT_TRUE(str->Members()[1]->Name().IsValid());
            EXPECT_NE(str->Members()[0]->Name().Name(), "0invalid");
            EXPECT_NE(str->Members()[1]->Name().Name(), "1invalid");
            EXPECT_NE(str->Members()[0]->Name(), str->Members()[1]->Name());
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

struct IRBinaryDecodeInternalNameTest : public IRTestHelper,
                                        public testing::WithParamInterface<std::string_view> {};
TEST_P(IRBinaryDecodeInternalNameTest, ValidInternalName) {
    auto name = GetParam();
    auto* S = ty.Struct(b.ir.symbols.New("S"), {
                                                   {b.ir.symbols.New("a"), ty.i32()},
                                               });
    b.Append(b.ir.root_block, [&] { b.Var(ty.ptr<function, read_write>(S)); });

    auto res = EncodeToProto(mod);
    ASSERT_EQ(res, Success);

    auto pb_mod = std::move(res.Get());
    // Find the struct type and rename it to the internal name.
    bool found = false;
    for (int i = 0; i < pb_mod->types_size(); ++i) {
        if (pb_mod->types(i).has_struct_()) {
            pb_mod->mutable_types(i)->mutable_struct_()->set_name(std::string(name));
            found = true;
            break;
        }
    }
    ASSERT_TRUE(found);

    auto decoded = Decode(*pb_mod);
    EXPECT_EQ(decoded, Success);
}

INSTANTIATE_TEST_SUITE_P(FrexpModfResultStructName,
                         IRBinaryDecodeInternalNameTest,
                         testing::Values("__atomic_compare_exchange_result_i32",
                                         "__atomic_compare_exchange_result_u32",
                                         "__frexp_result_abstract",
                                         "__frexp_result_f16",
                                         "__frexp_result_f32",
                                         "__frexp_result_vec2_abstract",
                                         "__frexp_result_vec2_f16",
                                         "__frexp_result_vec2_f32",
                                         "__frexp_result_vec3_abstract",
                                         "__frexp_result_vec3_f16",
                                         "__frexp_result_vec3_f32",
                                         "__frexp_result_vec4_abstract",
                                         "__frexp_result_vec4_f16",
                                         "__frexp_result_vec4_f32",
                                         "__modf_result_abstract",
                                         "__modf_result_f16",
                                         "__modf_result_f32",
                                         "__modf_result_vec2_abstract",
                                         "__modf_result_vec2_f16",
                                         "__modf_result_vec2_f32",
                                         "__modf_result_vec3_abstract",
                                         "__modf_result_vec3_f16",
                                         "__modf_result_vec3_f32",
                                         "__modf_result_vec4_abstract",
                                         "__modf_result_vec4_f16",
                                         "__modf_result_vec4_f32"));

TEST_F(IRBinaryDecodeTest, MultipleEntryPoints) {
    b.ComputeFunction("ep1");
    b.ComputeFunction("ep1");

    auto res = EncodeToProto(mod);
    ASSERT_EQ(res, Success);

    auto pb_mod = std::move(res.Get());

    auto decoded = Decode(*pb_mod);
    EXPECT_EQ(decoded, Success);

    EXPECT_TRUE(decoded.Get().properties.Contains(core::ir::Property::kAllowMultipleEntryPoints));
}

TEST_F(IRBinaryDecodeTest, Overrides) {
    b.Append(mod.root_block, [&] {  //
        b.Override(ty.u32());
    });

    auto res = EncodeToProto(mod);
    ASSERT_EQ(res, Success);

    auto pb_mod = std::move(res.Get());

    auto decoded = Decode(*pb_mod);
    EXPECT_EQ(decoded, Success);

    EXPECT_TRUE(decoded.Get().properties.Contains(core::ir::Property::kAllowOverrides));
}

}  // namespace
}  // namespace tint::core::ir::binary

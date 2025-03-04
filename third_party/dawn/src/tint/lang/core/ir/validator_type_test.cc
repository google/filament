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

#include "src/tint/lang/core/ir/validator_test.h"

#include <string>
#include <tuple>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "src/tint/lang/core/address_space.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/texel_format.h"
#include "src/tint/lang/core/type/abstract_float.h"
#include "src/tint/lang/core/type/abstract_int.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/memory_view.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/storage_texture.h"

namespace tint::core::ir {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace {
template <typename T>
static const core::type::Type* TypeBuilder(core::type::Manager& m) {
    return m.Get<T>();
}

template <typename T>
static const core::type::Type* RefTypeBuilder(core::type::Manager& m) {
    return m.ref<AddressSpace::kFunction, T>();
}

using TypeBuilderFn = decltype(&TypeBuilder<i32>);
}  // namespace

// Note: Just parameterizing abstract int vs float doesn't significantly reduce
// the code size of these tests, and made the code less readable.
//
// Combining them with the non-abstract parameterizing helps a little but adds
// some switching logic to the fixtures since the error strings are different.
// There is also still an issue with needing different fixtures for vec2 vs vec3
// vs vec4, which gets even worst for matrices. There is also variance in what
// is allowed where, so it feels like mostly a wash for code length, and makes
// harder to read code.
//
// There is probably something sophisticated using builders for
// scalar/vector/etc and testing::Combine to parameterize things, but that
// requires handling how the type manager allows templating/dispatch.

TEST_F(IR_ValidatorTest, AbstractFloat_Scalar) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        b.Var("af", function, ty.AFloat());
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:3:5 error: var: abstracts are not permitted
    %af:ptr<function, abstract-float, read_write> = var
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, AbstractInt_Scalar) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        b.Var("ai", function, ty.AInt());
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:3:5 error: var: abstracts are not permitted
    %ai:ptr<function, abstract-int, read_write> = var
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, AbstractFloat_Vector) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        b.Var("af", function, ty.vec2<AFloat>());
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:3:5 error: var: abstracts are not permitted
    %af:ptr<function, vec2<abstract-float>, read_write> = var
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, AbstractInt_Vector) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        b.Var("ai", function, ty.vec3<AInt>());
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(3:5 error: var: abstracts are not permitted
    %ai:ptr<function, vec3<abstract-int>, read_write> = var
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, AbstractFloat_Matrix) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        b.Var("af", function, ty.mat2x2<AFloat>());
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:3:5 error: var: abstracts are not permitted
    %af:ptr<function, mat2x2<abstract-float>, read_write> = var
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, AbstractInt_Matrix) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        b.Var("ai", function, ty.mat3x4<AInt>());
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:3:5 error: var: abstracts are not permitted
    %ai:ptr<function, mat3x4<abstract-int>, read_write> = var
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, AbstractFloat_Struct) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("af"), ty.AFloat(), {}},
                                               });
    auto* v = b.Var(ty.ptr(private_, str_ty));
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:6:3 error: var: abstracts are not permitted
  %1:ptr<private, MyStruct, read_write> = var
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, AbstractInt_Struct) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("ai"), ty.AInt(), {}},
                                               });
    auto* v = b.Var(ty.ptr(private_, str_ty));
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:6:3 error: var: abstracts are not permitted
  %1:ptr<private, MyStruct, read_write> = var
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, AbstractFloat_FunctionParam) {
    auto* f = b.Function("my_func", ty.void_());

    f->SetParams({b.FunctionParam(ty.AFloat())});
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:1:17 error: abstracts are not permitted
%my_func = func(%2:abstract-float):void {
                ^^^^^^^^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, AbstractInt_FunctionParam) {
    auto* f = b.Function("my_func", ty.void_());

    f->SetParams({b.FunctionParam(ty.AInt())});
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:1:17 error: abstracts are not permitted
%my_func = func(%2:abstract-int):void {
                ^^^^^^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

using TypeTest = IRTestParamHelper<std::tuple<
    /* allowed */ bool,
    /* type_builder */ TypeBuilderFn>>;

using Type_VectorElements = TypeTest;

TEST_P(Type_VectorElements, Test) {
    bool allowed = std::get<0>(GetParam());
    auto* type = std::get<1>(GetParam())(ty);
    if (allowed) {
        auto* f = b.Function("my_func", ty.void_());
        b.Append(f->Block(), [&] {
            b.Var("v2", AddressSpace::kFunction, ty.vec2(type));
            b.Var("v3", AddressSpace::kFunction, ty.vec3(type));
            b.Var("v4", AddressSpace::kFunction, ty.vec4(type));
            b.Return(f);
        });

        auto res = ir::Validate(mod);
        ASSERT_EQ(res, Success) << res.Failure().reason.Str();
    } else {
        auto* f = b.Function("my_func", ty.void_());
        b.Append(f->Block(), [&] {
            b.Var("invalid", AddressSpace::kFunction, ty.vec2(type));
            b.Return(f);
        });

        auto res = ir::Validate(mod);
        ASSERT_NE(res, Success) << res.Failure().reason.Str();
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr(R"(:3:5 error: var: vector elements, ')" +
                                       ty.vec2(type)->FriendlyName() + R"(', must be scalars
 )")) << res.Failure().reason.Str();
    }
}

INSTANTIATE_TEST_SUITE_P(IR_ValidatorTest,
                         Type_VectorElements,
                         testing::Values(std::make_tuple(true, TypeBuilder<u32>),
                                         std::make_tuple(true, TypeBuilder<i32>),
                                         std::make_tuple(true, TypeBuilder<f32>),
                                         std::make_tuple(true, TypeBuilder<f16>),
                                         std::make_tuple(true, TypeBuilder<core::type::Bool>),
                                         std::make_tuple(false, TypeBuilder<core::type::Void>)));

using Type_MatrixElements = TypeTest;

TEST_P(Type_MatrixElements, Test) {
    bool allowed = std::get<0>(GetParam());
    auto* type = std::get<1>(GetParam())(ty);
    if (allowed) {
        auto* f = b.Function("my_func", ty.void_());
        b.Append(f->Block(), [&] {
            b.Var("m2x2", AddressSpace::kFunction, ty.mat2x2(type));
            b.Var("m2x3", AddressSpace::kFunction, ty.mat2x3(type));
            b.Var("m2x4", AddressSpace::kFunction, ty.mat2x4(type));
            b.Var("m3x2", AddressSpace::kFunction, ty.mat3x2(type));
            b.Var("m3x3", AddressSpace::kFunction, ty.mat3x3(type));
            b.Var("m3x4", AddressSpace::kFunction, ty.mat3x4(type));
            b.Var("m4x2", AddressSpace::kFunction, ty.mat4x2(type));
            b.Var("m4x3", AddressSpace::kFunction, ty.mat4x3(type));
            b.Var("m4x4", AddressSpace::kFunction, ty.mat4x4(type));
            b.Return(f);
        });

        auto res = ir::Validate(mod);
        ASSERT_EQ(res, Success) << res.Failure().reason.Str();
    } else {
        auto* f = b.Function("my_func", ty.void_());
        b.Append(f->Block(), [&] {
            b.Var("invalid", AddressSpace::kFunction, ty.mat3x3(type));
            b.Return(f);
        });

        auto res = ir::Validate(mod);
        ASSERT_NE(res, Success) << res.Failure().reason.Str();
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr(R"(:3:5 error: var: matrix elements, ')" +
                                       ty.mat3x3(type)->FriendlyName() + R"(', must be float scalars
 )")) << res.Failure().reason.Str();
    }
}

INSTANTIATE_TEST_SUITE_P(IR_ValidatorTest,
                         Type_MatrixElements,
                         testing::Values(std::make_tuple(false, TypeBuilder<u32>),
                                         std::make_tuple(false, TypeBuilder<i32>),
                                         std::make_tuple(true, TypeBuilder<f32>),
                                         std::make_tuple(true, TypeBuilder<f16>),
                                         std::make_tuple(false, TypeBuilder<core::type::Bool>),
                                         std::make_tuple(false, TypeBuilder<core::type::Void>)));

using Type_StorageTextureDimension = IRTestParamHelper<std::tuple<
    /* allowed */ bool,
    /* dim */ core::type::TextureDimension>>;

TEST_P(Type_StorageTextureDimension, Test) {
    bool allowed = std::get<0>(GetParam());
    auto dim = std::get<1>(GetParam());

    auto* v =
        b.Var("v", AddressSpace::kStorage,
              ty.storage_texture(dim, core::TexelFormat::kRgba32Float, core::Access::kReadWrite),
              read_write);
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    if (allowed) {
        auto res = ir::Validate(mod);
        ASSERT_EQ(res, Success) << res.Failure().reason.Str();
    } else {
        auto res = ir::Validate(mod);
        ASSERT_NE(res, Success) << res.Failure().reason.Str();
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr(
                        dim != type::TextureDimension::kNone
                            ? R"(:2:3 error: var: dimension ')" + std::string(ToString(dim)) +
                                  R"(' for storage textures does not in WGSL yet)"
                            : R"(:2:3 error: var: invalid texture dimension 'none')"))
            << res.Failure().reason.Str();
    }
}

INSTANTIATE_TEST_SUITE_P(
    IR_ValidatorTest,
    Type_StorageTextureDimension,
    testing::Values(std::make_tuple(true, core::type::TextureDimension::k2d),
                    std::make_tuple(false, core::type::TextureDimension::kCube),
                    std::make_tuple(false, core::type::TextureDimension::kCubeArray),
                    std::make_tuple(false, core::type::TextureDimension::kNone)));

using IR_ValidatorRefTypeTest = IRTestParamHelper<std::tuple</* holds_ref */ bool,
                                                             /* refs_allowed */ bool,
                                                             /* type_builder */ TypeBuilderFn>>;

TEST_P(IR_ValidatorRefTypeTest, Var) {
    bool holds_ref = std::get<0>(GetParam());
    bool refs_allowed = std::get<1>(GetParam());
    auto* type = std::get<2>(GetParam())(ty);

    auto* fn = b.Function("my_func", ty.void_());
    b.Append(fn->Block(), [&] {
        if (auto* view = type->As<core::type::MemoryView>()) {
            b.Var(view);
        } else {
            b.Var(ty.ptr<function>(type));
        }

        b.Return(fn);
    });

    Capabilities caps;
    if (refs_allowed) {
        caps.Add(Capability::kAllowRefTypes);
    }
    auto res = ir::Validate(mod, caps);
    if (!holds_ref || refs_allowed) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr("3:5 error: var: reference types are not permitted"))
            << res.Failure().reason.Str();
    }
}

TEST_P(IR_ValidatorRefTypeTest, FnParam) {
    bool holds_ref = std::get<0>(GetParam());
    bool refs_allowed = std::get<1>(GetParam());
    auto* type = std::get<2>(GetParam())(ty);

    auto* fn = b.Function("my_func", ty.void_());
    fn->SetParams(Vector{b.FunctionParam(type)});
    b.Append(fn->Block(), [&] { b.Return(fn); });

    Capabilities caps;
    if (refs_allowed) {
        caps.Add(Capability::kAllowRefTypes);
    }
    auto res = ir::Validate(mod, caps);
    if (!holds_ref) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr("reference types are not permitted"))
            << res.Failure().reason.Str();
    }
}

TEST_P(IR_ValidatorRefTypeTest, FnRet) {
    bool holds_ref = std::get<0>(GetParam());
    bool refs_allowed = std::get<1>(GetParam());
    auto* type = std::get<2>(GetParam())(ty);

    auto* fn = b.Function("my_func", type);
    b.Append(fn->Block(), [&] { b.Unreachable(); });

    Capabilities caps;
    if (refs_allowed) {
        caps.Add(Capability::kAllowRefTypes);
    }
    auto res = ir::Validate(mod, caps);
    if (!holds_ref) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr("reference types are not permitted"))
            << res.Failure().reason.Str();
    }
}

TEST_P(IR_ValidatorRefTypeTest, BlockParam) {
    bool holds_ref = std::get<0>(GetParam());
    bool refs_allowed = std::get<1>(GetParam());
    auto* type = std::get<2>(GetParam())(ty);

    auto* fn = b.Function("my_func", ty.void_());
    b.Append(fn->Block(), [&] {
        auto* loop = b.Loop();
        loop->Continuing()->SetParams({b.BlockParam(type)});
        b.Append(loop->Body(), [&] {  //
            b.Continue(loop, nullptr);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.NextIteration(loop);
        });
        b.Unreachable();
    });

    Capabilities caps;
    if (refs_allowed) {
        caps.Add(Capability::kAllowRefTypes);
    }
    auto res = ir::Validate(mod, caps);
    if (!holds_ref) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr("reference types are not permitted"))
            << res.Failure().reason.Str();
    }
}

INSTANTIATE_TEST_SUITE_P(NonRefTypes,
                         IR_ValidatorRefTypeTest,
                         testing::Combine(/* holds_ref */ testing::Values(false),
                                          /* refs_allowed */ testing::Values(false, true),
                                          /* type_builder */
                                          testing::Values(TypeBuilder<i32>,
                                                          TypeBuilder<bool>,
                                                          TypeBuilder<vec4<f32>>,
                                                          TypeBuilder<array<f32, 3>>)));

INSTANTIATE_TEST_SUITE_P(RefTypes,
                         IR_ValidatorRefTypeTest,
                         testing::Combine(/* holds_ref */ testing::Values(true),
                                          /* refs_allowed */ testing::Values(false, true),
                                          /* type_builder */
                                          testing::Values(RefTypeBuilder<i32>,
                                                          RefTypeBuilder<bool>,
                                                          RefTypeBuilder<vec4<f32>>)));

TEST_F(IR_ValidatorTest, PointerToPointer) {
    auto* type = ty.ptr<function, ptr<function, i32>>();
    auto* fn = b.Function("my_func", ty.void_());
    fn->SetParams(Vector{b.FunctionParam(type)});
    b.Append(fn->Block(), [&] {  //
        b.Return(fn);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr("nested pointer types are not permitted"))
        << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, PointerToVoid) {
    auto* type = ty.ptr(AddressSpace::kFunction, ty.void_());
    auto* fn = b.Function("my_func", ty.void_());
    fn->SetParams(Vector{b.FunctionParam(type)});
    b.Append(fn->Block(), [&] {  //
        b.Return(fn);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr("pointers to void are not permitted"))
        << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, ReferenceToReference) {
    auto* type = ty.ref<function>(ty.ref<function, i32>());
    auto* fn = b.Function("my_func", ty.void_());
    b.Append(fn->Block(), [&] {  //
        b.Var(type);
        b.Return(fn);
    });

    Capabilities caps;
    caps.Add(Capability::kAllowRefTypes);

    auto res = ir::Validate(mod, caps);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr("nested reference types are not permitted"))
        << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, ReferenceToVoid) {
    auto* type = ty.ref(AddressSpace::kFunction, ty.void_());
    auto* fn = b.Function("my_func", ty.void_());
    b.Append(fn->Block(), [&] {  //
        b.Var(type);
        b.Return(fn);
    });

    Capabilities caps;
    caps.Add(Capability::kAllowRefTypes);

    auto res = ir::Validate(mod, caps);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr("references to void are not permitted"))
        << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, PointerInStructure_WithoutCapability) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("S"), {
                                            {mod.symbols.New("a"), ty.ptr<private_, i32>()},
                                        });
    mod.root_block->Append(b.Var("my_struct", private_, str_ty));

    auto* fn = b.Function("F", ty.void_());
    b.Append(fn->Block(), [&] { b.Return(fn); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr("nested pointer types are not permitted"))
        << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, PointerInStructure_WithCapability) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("S"), {
                                            {mod.symbols.New("a"), ty.ptr<private_, i32>()},
                                        });

    auto* fn = b.Function("F", ty.void_());
    auto* param = b.FunctionParam("param", str_ty);
    fn->SetParams({param});
    b.Append(fn->Block(), [&] { b.Return(fn); });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowPointersAndHandlesInStructures});
    EXPECT_EQ(res, Success) << res.Failure();
}

using IR_Validator8BitIntTypeTest = IRTestParamHelper<std::tuple<
    /* int8_allowed */ bool,
    /* type_builder */ TypeBuilderFn>>;

TEST_P(IR_Validator8BitIntTypeTest, Var) {
    bool int8_allowed = std::get<0>(GetParam());
    auto* type = std::get<1>(GetParam())(ty);

    auto* fn = b.Function("my_func", ty.void_());
    b.Append(fn->Block(), [&] {
        b.Var(ty.ptr<function>(type));
        b.Return(fn);
    });

    Capabilities caps;
    if (int8_allowed) {
        caps.Add(Capability::kAllow8BitIntegers);
    }
    auto res = ir::Validate(mod, caps);
    if (int8_allowed) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr("3:5 error: var: 8-bit integer types are not permitted"))
            << res.Failure().reason.Str();
    }
}

TEST_P(IR_Validator8BitIntTypeTest, FnParam) {
    bool int8_allowed = std::get<0>(GetParam());
    auto* type = std::get<1>(GetParam())(ty);

    auto* fn = b.Function("my_func", ty.void_());
    fn->SetParams(Vector{b.FunctionParam(type)});
    b.Append(fn->Block(), [&] { b.Return(fn); });

    Capabilities caps;
    if (int8_allowed) {
        caps.Add(Capability::kAllow8BitIntegers);
    }
    auto res = ir::Validate(mod, caps);
    if (int8_allowed) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr("8-bit integer types are not permitted"))
            << res.Failure().reason.Str();
    }
}

TEST_P(IR_Validator8BitIntTypeTest, FnRet) {
    bool int8_allowed = std::get<0>(GetParam());
    auto* type = std::get<1>(GetParam())(ty);

    auto* fn = b.Function("my_func", type);
    b.Append(fn->Block(), [&] { b.Unreachable(); });

    Capabilities caps;
    if (int8_allowed) {
        caps.Add(Capability::kAllow8BitIntegers);
    }
    auto res = ir::Validate(mod, caps);
    if (int8_allowed) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr("8-bit integer types are not permitted"))
            << res.Failure().reason.Str();
    }
}

TEST_P(IR_Validator8BitIntTypeTest, BlockParam) {
    bool int8_allowed = std::get<0>(GetParam());
    auto* type = std::get<1>(GetParam())(ty);

    auto* fn = b.Function("my_func", ty.void_());
    b.Append(fn->Block(), [&] {
        auto* loop = b.Loop();
        loop->Continuing()->SetParams({b.BlockParam(type)});
        b.Append(loop->Body(), [&] {  //
            b.Continue(loop, nullptr);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.NextIteration(loop);
        });
        b.Unreachable();
    });

    Capabilities caps;
    if (int8_allowed) {
        caps.Add(Capability::kAllow8BitIntegers);
    }
    auto res = ir::Validate(mod, caps);
    if (int8_allowed) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr("8-bit integer types are not permitted"))
            << res.Failure().reason.Str();
    }
}

INSTANTIATE_TEST_SUITE_P(Int8Types,
                         IR_Validator8BitIntTypeTest,
                         testing::Combine(
                             /* int8_allowed */ testing::Values(false, true),
                             /* type_builder */
                             testing::Values(TypeBuilder<i8>,
                                             TypeBuilder<u8>,
                                             TypeBuilder<vec4<i8>>,
                                             TypeBuilder<array<u8, 4>>)));

TEST_F(IR_ValidatorTest, Int8Type_InstructionOperand_NotAllowed) {
    auto* fn = b.Function("my_func", ty.void_());
    b.Append(fn->Block(), [&] {
        b.Let("l", u8(1));
        b.Return(fn);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:3:5 error: let: 8-bit integer types are not permitted
    %l:u8 = let 1u8
    ^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Int8Type_InstructionOperand_Allowed) {
    auto* fn = b.Function("my_func", ty.void_());
    b.Append(fn->Block(), [&] {
        b.Let("l", u8(1));
        b.Return(fn);
    });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllow8BitIntegers});
    ASSERT_EQ(res, Success) << res.Failure();
}

using IR_Validator64BitIntTypeTest = IRTestParamHelper<std::tuple<
    /* int64_allowed */ bool,
    /* type_builder */ TypeBuilderFn>>;

TEST_P(IR_Validator64BitIntTypeTest, Var) {
    bool int64_allowed = std::get<0>(GetParam());
    auto* type = std::get<1>(GetParam())(ty);

    auto* fn = b.Function("my_func", ty.void_());
    b.Append(fn->Block(), [&] {
        b.Var(ty.ptr<function>(type));
        b.Return(fn);
    });

    Capabilities caps;
    if (int64_allowed) {
        caps.Add(Capability::kAllow64BitIntegers);
    }
    auto res = ir::Validate(mod, caps);
    if (int64_allowed) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr("3:5 error: var: 64-bit integer types are not permitted"))
            << res.Failure().reason.Str();
    }
}

TEST_P(IR_Validator64BitIntTypeTest, FnParam) {
    bool int64_allowed = std::get<0>(GetParam());
    auto* type = std::get<1>(GetParam())(ty);

    auto* fn = b.Function("my_func", ty.void_());
    fn->SetParams(Vector{b.FunctionParam(type)});
    b.Append(fn->Block(), [&] { b.Return(fn); });

    Capabilities caps;
    if (int64_allowed) {
        caps.Add(Capability::kAllow64BitIntegers);
    }
    auto res = ir::Validate(mod, caps);
    if (int64_allowed) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr("64-bit integer types are not permitted"))
            << res.Failure().reason.Str();
    }
}

TEST_P(IR_Validator64BitIntTypeTest, FnRet) {
    bool int64_allowed = std::get<0>(GetParam());
    auto* type = std::get<1>(GetParam())(ty);

    auto* fn = b.Function("my_func", type);
    b.Append(fn->Block(), [&] { b.Unreachable(); });

    Capabilities caps;
    if (int64_allowed) {
        caps.Add(Capability::kAllow64BitIntegers);
    }
    auto res = ir::Validate(mod, caps);
    if (int64_allowed) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr("64-bit integer types are not permitted"))
            << res.Failure().reason.Str();
    }
}

TEST_P(IR_Validator64BitIntTypeTest, BlockParam) {
    bool int64_allowed = std::get<0>(GetParam());
    auto* type = std::get<1>(GetParam())(ty);

    auto* fn = b.Function("my_func", ty.void_());
    b.Append(fn->Block(), [&] {
        auto* loop = b.Loop();
        loop->Continuing()->SetParams({b.BlockParam(type)});
        b.Append(loop->Body(), [&] {  //
            b.Continue(loop, nullptr);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.NextIteration(loop);
        });
        b.Unreachable();
    });

    Capabilities caps;
    if (int64_allowed) {
        caps.Add(Capability::kAllow64BitIntegers);
    }
    auto res = ir::Validate(mod, caps);
    if (int64_allowed) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr("64-bit integer types are not permitted"))
            << res.Failure().reason.Str();
    }
}

INSTANTIATE_TEST_SUITE_P(Int64Types,
                         IR_Validator64BitIntTypeTest,
                         testing::Combine(
                             /* int64_allowed */ testing::Values(false, true),
                             /* type_builder */
                             testing::Values(TypeBuilder<u64>,  //
                                             TypeBuilder<vec4<u64>>,
                                             TypeBuilder<array<u64, 4>>)));

TEST_F(IR_ValidatorTest, Int64Type_InstructionOperand_NotAllowed) {
    auto* fn = b.Function("my_func", ty.void_());
    b.Append(fn->Block(), [&] {
        b.Let("l", u64(1));
        b.Return(fn);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:3:5 error: let: 64-bit integer types are not permitted
    %l:u64 = let 1u64
    ^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Int64Type_InstructionOperand_Allowed) {
    auto* fn = b.Function("my_func", ty.void_());
    b.Append(fn->Block(), [&] {
        b.Let("l", u64(1));
        b.Return(fn);
    });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllow64BitIntegers});
    ASSERT_EQ(res, Success) << res.Failure();
}

}  // namespace tint::core::ir

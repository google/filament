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

#include <string>
#include <tuple>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/ir/validator_test.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/abstract_float.h"
#include "src/tint/lang/core/type/abstract_int.h"
#include "src/tint/lang/core/type/binding_array.h"
#include "src/tint/lang/core/type/clone_context.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/memory_view.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"

namespace tint::mock {
/// A mock non-core type used for testing the non-core type validation rule.
class NonCoreType final : public Castable<NonCoreType, core::type::Type> {
  public:
    NonCoreType() : Base(0u, core::type::Flags{}) {}
    bool Equals(const UniqueNode& other) const override { return other.Is<NonCoreType>(); }
    std::string FriendlyName() const override { return "NonCoreType"; }
    core::type::Type* Clone(core::type::CloneContext& ctx) const override {
        return ctx.dst.mgr->Get<NonCoreType>();
    }
};
}  // namespace tint::mock

TINT_INSTANTIATE_TYPEINFO(tint::mock::NonCoreType);

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
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:5 error: var: abstracts are not permitted
    %af:ptr<function, abstract-float, read_write> = var undef
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, AbstractInt_Scalar) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        b.Var("ai", function, ty.AInt());
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:5 error: var: abstracts are not permitted
    %ai:ptr<function, abstract-int, read_write> = var undef
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, AbstractFloat_Vector) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        b.Var("af", function, ty.vec2<AFloat>());
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:5 error: var: abstracts are not permitted
    %af:ptr<function, vec2<abstract-float>, read_write> = var undef
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, AbstractInt_Vector) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        b.Var("ai", function, ty.vec3<AInt>());
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(3:5 error: var: abstracts are not permitted
    %ai:ptr<function, vec3<abstract-int>, read_write> = var undef
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, AbstractFloat_Matrix) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        b.Var("af", function, ty.mat2x2<AFloat>());
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:5 error: var: abstracts are not permitted
    %af:ptr<function, mat2x2<abstract-float>, read_write> = var undef
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, AbstractInt_Matrix) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        b.Var("ai", function, ty.mat3x4<AInt>());
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:5 error: var: abstracts are not permitted
    %ai:ptr<function, mat3x4<abstract-int>, read_write> = var undef
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
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
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:6:3 error: var: abstracts are not permitted
  %1:ptr<private, MyStruct, read_write> = var undef
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
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
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:6:3 error: var: abstracts are not permitted
  %1:ptr<private, MyStruct, read_write> = var undef
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, AbstractFloat_FunctionParam) {
    auto* f = b.Function("my_func", ty.void_());

    f->SetParams({b.FunctionParam(ty.AFloat())});
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:1:17 error: abstracts are not permitted
%my_func = func(%2:abstract-float):void {
                ^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, AbstractInt_FunctionParam) {
    auto* f = b.Function("my_func", ty.void_());

    f->SetParams({b.FunctionParam(ty.AInt())});
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:1:17 error: abstracts are not permitted
%my_func = func(%2:abstract-int):void {
                ^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, StructMember_Void) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("v"), ty.void_(), {}},
                                               });
    auto* v = b.Var(ty.ptr(private_, str_ty));
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:6:3 error: var: struct member 0 cannot have void type
  %1:ptr<private, MyStruct, read_write> = var undef
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, StructMember_AlignZero) {
    core::IOAttributes attrs = {};
    tint::Vector<const core::type::StructMember*, 4> members;
    members.Push(ty.Get<core::type::StructMember>(mod.symbols.New("v"), ty.u32(), 0u, 0u,
                                                  /* align */ 0u, 16u, std::move(attrs)));
    auto* str_ty = ty.Get<core::type::Struct>(mod.symbols.New("MyStruct"), std::move(members),
                                              tint::RoundUp(0u, 16u));

    auto* v = b.Var(ty.ptr(private_, str_ty));
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:6:3 error: var: struct member must not have an alignment of 0
  %1:ptr<private, MyStruct, read_write> = var undef
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, StructMember_AlignNotDivisibleByTypeAlignment) {
    core::IOAttributes attrs = {};
    tint::Vector<const core::type::StructMember*, 4> members;
    members.Push(ty.Get<core::type::StructMember>(mod.symbols.New("v"), ty.u32(), 0u, 0u,
                                                  /* align */ 10u, 16u, std::move(attrs)));
    auto* str_ty = ty.Get<core::type::Struct>(mod.symbols.New("MyStruct"), std::move(members),
                                              tint::RoundUp(0u, 16u));

    auto* v = b.Var(ty.ptr(private_, str_ty));
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:6:3 error: var: struct member alignment (10) must be divisible by type alignment (4)
  %1:ptr<private, MyStruct, read_write> = var undef
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Structure_LargePaddingSizeAtEnd) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("S"),
                  Vector{
                      ty.Get<type::StructMember>(mod.symbols.New("a"), ty.array<u32, 3>(), 0u, 0u,
                                                 4u, 4'000'000'000u, IOAttributes{}),
                  });
    mod.root_block->Append(b.Var("my_struct", private_, str_ty));

    auto* fn = b.Function("F", ty.void_());
    b.Append(fn->Block(), [&] { b.Return(fn); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr("struct padding (3999999988) is larger then the max (10485760)"))
        << res.Failure();
}

TEST_F(IR_ValidatorTest, Structure_MemberAlignmentCausesSizeOverflow) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("S"),
                  Vector{
                      ty.Get<type::StructMember>(mod.symbols.New("a"), ty.u32(), 0u, 0u,
                                                 u32::kHighestValue - 63, 128u, IOAttributes{}),
                  });
    mod.root_block->Append(b.Var("my_struct", private_, str_ty));

    auto* fn = b.Function("F", ty.void_());
    b.Append(fn->Block(), [&] { b.Return(fn); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr("struct size (0) is smaller than the end of the last member (4)"))
        << res.Failure();
}

TEST_F(IR_ValidatorTest, StructureMember_LargePaddingSize) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("S"),
                  Vector{
                      ty.Get<type::StructMember>(mod.symbols.New("a"), ty.array<u32, 3>(), 0u, 0u,
                                                 4u, 40'000'000u, IOAttributes{}),
                      ty.Get<type::StructMember>(mod.symbols.New("b"), ty.array<u32, 3>(), 0u, 0u,
                                                 4u, 16u, IOAttributes{}),
                  });
    mod.root_block->Append(b.Var("my_struct", private_, str_ty));

    auto* fn = b.Function("F", ty.void_());
    b.Append(fn->Block(), [&] { b.Return(fn); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr("struct member padding (4294967284) is larger then the max (10485760)"))
        << res.Failure();
}

TEST_F(IR_ValidatorTest, StructureMember_SizeTooSmall) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("S"),
                  Vector{
                      ty.Get<type::StructMember>(mod.symbols.New("a"), ty.array<u32, 3>(), 0u, 0u,
                                                 4u, 4u, IOAttributes{}),
                  });
    mod.root_block->Append(b.Var("my_struct", private_, str_ty));

    auto* fn = b.Function("F", ty.void_());
    b.Append(fn->Block(), [&] { b.Return(fn); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            "struct member 0 with size=4 must be at least as large as the type with size 12"))
        << res.Failure();
}

TEST_F(IR_ValidatorTest, StructMember_RuntimeArrayNotLast) {
    auto* s1 = ty.Struct(mod.symbols.New("S1"), {{mod.symbols.New("a"), ty.u32()}});
    auto* rta = ty.runtime_array(s1);

    auto* str_ty = ty.Struct(mod.symbols.New("OuterS"), {
                                                            {mod.symbols.New("a1"), rta},
                                                            {mod.symbols.New("j"), ty.u32()},
                                                        });

    auto* v = b.Var(ty.ptr(storage, str_ty, read_write));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:11:3 error: var: runtime-sized arrays can only be the last member of a struct
  %1:ptr<storage, OuterS, read_write> = var undef @binding_point(0, 0)
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^)"))
        << res.Failure();
}

TEST_F(IR_ValidatorTest, StructMember_RuntimeArrayIsLast) {
    auto* s1 = ty.Struct(mod.symbols.New("S1"), {{mod.symbols.New("a"), ty.u32()}});
    auto* rta = ty.runtime_array(s1);

    auto* str_ty = ty.Struct(mod.symbols.New("OuterS"), {
                                                            {mod.symbols.New("j"), ty.u32()},
                                                            {mod.symbols.New("a1"), rta},
                                                        });

    auto* v = b.Var(ty.ptr(storage, str_ty, read_write));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, StructMember_MultipleRuntimeArrays) {
    auto* s1 = ty.Struct(mod.symbols.New("S1"), {{mod.symbols.New("a"), ty.u32()}});
    auto* rta = ty.runtime_array(s1);

    auto* str_ty = ty.Struct(mod.symbols.New("OuterS"), {
                                                            {mod.symbols.New("a1"), rta},
                                                            {mod.symbols.New("a2"), rta},
                                                        });

    auto* v = b.Var(ty.ptr(storage, str_ty, read_write));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:11:3 error: var: runtime-sized arrays can only be the last member of a struct
  %1:ptr<storage, OuterS, read_write> = var undef @binding_point(0, 0)
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^)"))
        << res.Failure();
}

TEST_F(IR_ValidatorTest, StructMember_RowMajor_WithoutCapability) {
    auto* mat_ty = ty.mat2x2<f32>();
    auto* member = ty.Get<core::type::StructMember>(
        mod.symbols.New("m"), mat_ty, 0u, 0u, mat_ty->Align(), mat_ty->Size(), IOAttributes{});
    member->SetRowMajor();
    auto* str_ty =
        ty.Get<core::type::Struct>(mod.symbols.New("MyStruct"), Vector{member}, mat_ty->Size());

    auto* v = b.Var(ty.ptr(private_, str_ty));
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:6:3 error: var: Row major annotation not allowed on structures
  %1:ptr<private, MyStruct, read_write> = var undef
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, StructMember_Pointer) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"),
                             {
                                 {mod.symbols.New("p"), ty.ptr<function, i32>(), {}},
                             });
    auto* v = b.Var(ty.ptr(private_, str_ty));
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:6:3 error: var: struct member 0 cannot be a pointer type
  %1:ptr<private, MyStruct, read_write> = var undef
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, StructMember_Pointer_WithCapability) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"),
                             {
                                 {mod.symbols.New("p"), ty.ptr<function, i32>(), {}},
                             });
    auto* v = b.Var(ty.ptr(private_, str_ty));
    mod.root_block->Append(v);

    Capabilities caps;
    caps.Add(Capability::kMslAllowEntryPointInterface);

    auto res = ir::Validate(mod, caps);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, StructMember_Texture) {
    auto* str_ty = ty.Struct(
        mod.symbols.New("MyStruct"),
        {
            {mod.symbols.New("t"), ty.sampled_texture(type::TextureDimension::k2d, ty.f32()), {}},
        });
    auto* v = b.Var(ty.ptr(private_, str_ty));
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:6:3 error: var: struct member 0 cannot be a texture type
  %1:ptr<private, MyStruct, read_write> = var undef
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, StructMember_Texture_WithCapability) {
    auto* str_ty = ty.Struct(
        mod.symbols.New("MyStruct"),
        {
            {mod.symbols.New("t"), ty.sampled_texture(type::TextureDimension::k2d, ty.f32()), {}},
        });
    auto* v = b.Var(ty.ptr(private_, str_ty));
    mod.root_block->Append(v);

    Capabilities caps;
    caps.Add(Capability::kMslAllowEntryPointInterface);

    auto res = ir::Validate(mod, caps);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, StructMember_Sampler) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("s"), ty.sampler(), {}},
                                               });
    auto* v = b.Var(ty.ptr(private_, str_ty));
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:6:3 error: var: struct member 0 cannot be a sampler type
  %1:ptr<private, MyStruct, read_write> = var undef
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, StructMember_Sampler_WithCapability) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("s"), ty.sampler(), {}},
                                               });
    auto* v = b.Var(ty.ptr(private_, str_ty));
    mod.root_block->Append(v);

    Capabilities caps;
    caps.Add(Capability::kMslAllowEntryPointInterface);

    auto res = ir::Validate(mod, caps);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, StructMember_RowMajor_WithCapability) {
    auto* mat_ty = ty.mat2x2<f32>();
    auto* member = ty.Get<core::type::StructMember>(
        mod.symbols.New("m"), mat_ty, 0u, 0u, mat_ty->Align(), mat_ty->Size(), IOAttributes{});
    member->SetRowMajor();
    auto* str_ty =
        ty.Get<core::type::Struct>(mod.symbols.New("MyStruct"), Vector{member}, mat_ty->Size());

    auto* v = b.Var(ty.ptr(private_, str_ty));
    mod.root_block->Append(v);

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowStructMatrixDecorations});
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, StructMember_MatrixStride_WithoutCapability) {
    auto* mat_ty = ty.mat2x2<f32>();
    auto* member = ty.Get<core::type::StructMember>(
        mod.symbols.New("m"), mat_ty, 0u, 0u, mat_ty->Align(), mat_ty->Size(), IOAttributes{});
    member->SetMatrixStride(32);
    auto* str_ty =
        ty.Get<core::type::Struct>(mod.symbols.New("MyStruct"), Vector{member}, mat_ty->Size());

    auto* v = b.Var(ty.ptr(private_, str_ty));
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:6:3 error: var: Matrix stride annotation not allowed on structures
  %1:ptr<private, MyStruct, read_write> = var undef
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, StructMember_MatrixStride_WithCapability) {
    auto* mat_ty = ty.mat2x2<f32>();
    auto* member = ty.Get<core::type::StructMember>(mod.symbols.New("m"), mat_ty, 0u, 0u,
                                                    mat_ty->Align(), 64u, IOAttributes{});
    member->SetMatrixStride(32);
    auto* str_ty = ty.Get<core::type::Struct>(mod.symbols.New("MyStruct"), Vector{member}, 64u);

    auto* v = b.Var(ty.ptr(private_, str_ty));
    mod.root_block->Append(v);

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowStructMatrixDecorations});
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, FunctionParam_InvalidAddressSpaceForHandleType) {
    auto* type = ty.ptr(AddressSpace::kFunction, ty.sampler());
    auto* fn = b.Function("my_func", ty.void_());
    fn->SetParams(Vector{b.FunctionParam(type)});
    b.Append(fn->Block(), [&] {  //
        b.Return(fn);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr("handle types can only be declared in the 'handle' address space"))
        << res.Failure();
}

TEST_F(IR_ValidatorTest, FunctionParam_InvalidTypeForHandleAddressSpace) {
    auto* type = ty.ptr(AddressSpace::kHandle, ty.u32());
    auto* fn = b.Function("my_func", ty.void_());
    fn->SetParams(Vector{b.FunctionParam(type)});
    b.Append(fn->Block(), [&] {  //
        b.Return(fn);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr("the 'handle' address space can only be used for handle types"))
        << res.Failure();
}

TEST_F(IR_ValidatorTest, FunctionParam_InvalidHandlePointer) {
    auto* type =
        ty.ptr(AddressSpace::kHandle, ty.sampled_texture(type::TextureDimension::k1d, ty.f32()));
    auto* fn = b.Function("my_func", ty.void_());
    fn->SetParams(Vector{b.FunctionParam(type)});
    b.Append(fn->Block(), [&] {  //
        b.Return(fn);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr("function parameter type, 'ptr<handle, texture_1d<f32>, read>', "
                                   "must be constructible, a pointer, or a handle"))
        << res.Failure();
}

TEST_F(IR_ValidatorTest, NonCoreType) {
    auto* fn = b.Function("my_func", ty.void_());
    fn->AppendParam(b.FunctionParam(ty.Get<tint::mock::NonCoreType>()));
    b.Append(fn->Block(), [&] {  //
        b.Return(fn);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr("non-core types not allowed in core IR"))
        << res.Failure();
}

using TypeTest = IRTestParamHelper<std::tuple<
    /* allowed */ bool,
    /* type_builder */ TypeBuilderFn>>;

using Type_ArrayElements = TypeTest;

TEST_P(Type_ArrayElements, Test) {
    bool allowed = std::get<0>(GetParam());
    auto* type = std::get<1>(GetParam())(ty);
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        b.Var("v", AddressSpace::kFunction, ty.array(type, 4));
        b.Return(f);
    });

    if (allowed) {
        auto res = ir::Validate(mod);
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        auto res = ir::Validate(mod);
        ASSERT_NE(res, Success) << res.Failure();
        EXPECT_THAT(res.Failure().reason,
                    testing::HasSubstr(R"(:3:5 error: var: array elements, ')" +
                                       ty.array(type, 4)->FriendlyName() +
                                       R"(', must have creation-fixed footprint
 )")) << res.Failure();
    }
}

INSTANTIATE_TEST_SUITE_P(IR_ValidatorTest,
                         Type_ArrayElements,
                         testing::Values(std::make_tuple(true, TypeBuilder<u32>),
                                         std::make_tuple(true, TypeBuilder<i32>),
                                         std::make_tuple(true, TypeBuilder<f32>),
                                         std::make_tuple(true, TypeBuilder<f16>),
                                         std::make_tuple(true, TypeBuilder<core::type::Bool>),
                                         std::make_tuple(false, TypeBuilder<core::type::Void>)));

// Test that validation time does not increase in relation to the size of the array.
TEST_F(IR_ValidatorTest, LargeArrays) {
    // The arrays are all different sizes so that they don't get skipped over by CheckType for
    // having been "seen" already.
    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"),
                  {
                      {mod.symbols.New("a0"), ty.u32(), {}},
                      {mod.symbols.New("a1"), ty.array<u32, (1ull << 32ull) - 4u>(), {}},
                      {mod.symbols.New("a2"), ty.array<u32, (1ull << 32ull) - 8u>(), {}},
                      {mod.symbols.New("a3"), ty.array<u32, (1ull << 32ull) - 12u>(), {}},
                      {mod.symbols.New("a4"), ty.array<u32, (1ull << 32ull) - 16u>(), {}},
                      {mod.symbols.New("a5"), ty.array<u32, (1ull << 32ull) - 20u>(), {}},
                      {mod.symbols.New("a6"), ty.array<u32, (1ull << 32ull) - 24u>(), {}},
                      {mod.symbols.New("a7"), ty.array<u32, (1ull << 32ull) - 28u>(), {}},
                      {mod.symbols.New("a8"), ty.array<u32, (1ull << 32ull) - 32u>(), {}},
                  });
    mod.root_block->Append(b.Var(ty.ptr<workgroup>(str_ty)));
    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

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
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        auto* f = b.Function("my_func", ty.void_());
        b.Append(f->Block(), [&] {
            b.Var("invalid", AddressSpace::kFunction, ty.vec2(type));
            b.Return(f);
        });

        auto res = ir::Validate(mod);
        ASSERT_NE(res, Success) << res.Failure();
        EXPECT_THAT(res.Failure().reason,
                    testing::HasSubstr(R"(:3:5 error: var: vector elements, ')" +
                                       ty.vec2(type)->FriendlyName() + R"(', must be scalars
 )")) << res.Failure();
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
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        auto* f = b.Function("my_func", ty.void_());
        b.Append(f->Block(), [&] {
            b.Var("invalid", AddressSpace::kFunction, ty.mat3x3(type));
            b.Return(f);
        });

        auto res = ir::Validate(mod);
        ASSERT_NE(res, Success) << res.Failure();
        EXPECT_THAT(res.Failure().reason,
                    testing::HasSubstr(R"(:3:5 error: var: matrix elements, ')" +
                                       ty.mat3x3(type)->FriendlyName() + R"(', must be float scalars
 )")) << res.Failure();
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

using Type_AtomicSubType = TypeTest;

TEST_P(Type_AtomicSubType, Test) {
    auto allowed = std::get<0>(GetParam());
    auto* type = std::get<1>(GetParam())(ty);
    b.Append(mod.root_block, [&] {  //
        b.Var("v", AddressSpace::kWorkgroup, ty.atomic(type));
    });

    auto res = ir::Validate(mod);
    if (allowed) {
        EXPECT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason,
                    testing::HasSubstr("error: var: atomic subtype must be i32, u32 or u64"))
            << res.Failure();
    }
}

INSTANTIATE_TEST_SUITE_P(IR_ValidatorTest,
                         Type_AtomicSubType,
                         testing::Values(std::make_tuple(true, TypeBuilder<i32>),
                                         std::make_tuple(true, TypeBuilder<u32>),
                                         std::make_tuple(false, TypeBuilder<f32>),
                                         std::make_tuple(false, TypeBuilder<f16>),
                                         std::make_tuple(false, TypeBuilder<core::type::Bool>),
                                         std::make_tuple(false, TypeBuilder<core::type::Void>)));

using Type_SubgroupMatrixComponentType = TypeTest;

TEST_P(Type_SubgroupMatrixComponentType, Test) {
    bool allowed = std::get<0>(GetParam());
    auto* type = std::get<1>(GetParam())(ty);
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        b.Var("m", AddressSpace::kFunction, ty.subgroup_matrix_result(type, 8u, 8u));
        b.Return(f);
    });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllow8BitIntegers});
    if (allowed) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(
            res.Failure().reason,
            testing::HasSubstr(":3:5 error: var: invalid subgroup matrix component type: '" +
                               type->FriendlyName()))
            << res.Failure();
    }
}

INSTANTIATE_TEST_SUITE_P(IR_ValidatorTest,
                         Type_SubgroupMatrixComponentType,
                         testing::Values(std::make_tuple(true, TypeBuilder<f32>),
                                         std::make_tuple(true, TypeBuilder<f16>),
                                         std::make_tuple(true, TypeBuilder<i8>),
                                         std::make_tuple(true, TypeBuilder<i32>),
                                         std::make_tuple(true, TypeBuilder<u8>),
                                         std::make_tuple(true, TypeBuilder<u32>),
                                         std::make_tuple(false, TypeBuilder<core::type::Bool>),
                                         std::make_tuple(false, TypeBuilder<core::type::Void>)));

using Type_SampledTextureSampledType = TypeTest;

TEST_P(Type_SampledTextureSampledType, Test) {
    bool allowed = std::get<0>(GetParam());
    auto* type = std::get<1>(GetParam())(ty);
    b.Append(mod.root_block, [&] {
        auto* var = b.Var("m", AddressSpace::kHandle,
                          ty.sampled_texture(core::type::TextureDimension::k2d, type));
        var->SetBindingPoint(0, 0);
    });

    auto res = ir::Validate(mod);
    if (allowed) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason,
                    testing::HasSubstr(":2:3 error: var: invalid sampled texture sample type: '" +
                                       type->FriendlyName()))
            << res.Failure();
    }
}

INSTANTIATE_TEST_SUITE_P(IR_ValidatorTest,
                         Type_SampledTextureSampledType,
                         testing::Values(std::make_tuple(true, TypeBuilder<f32>),
                                         std::make_tuple(true, TypeBuilder<i32>),
                                         std::make_tuple(true, TypeBuilder<u32>),
                                         std::make_tuple(false, TypeBuilder<f16>),
                                         std::make_tuple(false, TypeBuilder<core::type::Bool>),
                                         std::make_tuple(false, TypeBuilder<core::type::Void>)));

TEST_F(IR_ValidatorTest, BindingArray) {
    b.Append(mod.root_block, [&] {
        auto* var = b.Var(
            "m", AddressSpace::kHandle,
            ty.binding_array(ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), 4));
        var->SetBindingPoint(0, 0);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, BindingArrayRuntimeCount) {
    b.Append(mod.root_block, [&] {
        auto* var = b.Var("m", AddressSpace::kHandle,
                          ty.Get<core::type::BindingArray>(
                              ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()),
                              ty.Get<core::type::RuntimeArrayCount>()));
        var->SetBindingPoint(0, 0);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:2:3 error: var: binding_array count must be a constant expression
  %m:ptr<handle, binding_array<texture_2d<f32>, >, read> = var undef @binding_point(0, 0)
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^)"));
}

TEST_F(IR_ValidatorTest, BindingArrayNonSampledTexture) {
    b.Append(mod.root_block, [&] {
        auto* var = b.Var("m", AddressSpace::kHandle, ty.binding_array(ty.external_texture(), 5));
        var->SetBindingPoint(0, 0);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:2:3 error: var: binding_array element type must be a sampled texture type
  %m:ptr<handle, binding_array<texture_external, 5>, read> = var undef @binding_point(0, 0)
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^)"));
}

TEST_F(IR_ValidatorTest, BindingArrayInvalidAddressSpace) {
    b.Append(mod.root_block, [&] {
        b.Var("m", AddressSpace::kWorkgroup,
              ty.binding_array(ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), 4));
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:2:3 error: var: handle types can only be declared in the 'handle' address space
  %m:ptr<workgroup, binding_array<texture_2d<f32>, 4>, read_write> = var undef
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^)"));
}

using Type_MultisampledTextureTypeAndDimension =
    IRTestParamHelper<std::tuple<std::tuple<
                                     /* type_allowed */ bool,
                                     /* type_builder */ TypeBuilderFn>,
                                 std::tuple<
                                     /* dim_allowed */ bool,
                                     /* dim */ core::type::TextureDimension>>>;

TEST_P(Type_MultisampledTextureTypeAndDimension, Test) {
    auto type_params = std::get<0>(GetParam());
    bool type_allowed = std::get<0>(type_params);
    auto* type = std::get<1>(type_params)(ty);

    auto dim_params = std::get<1>(GetParam());
    bool dim_allowed = std::get<0>(dim_params);
    auto dim = std::get<1>(dim_params);

    bool allowed = type_allowed && dim_allowed;

    b.Append(mod.root_block, [&] {
        auto* var = b.Var("ms", AddressSpace::kHandle, ty.multisampled_texture(dim, type));
        var->SetBindingPoint(0, 0);
    });

    auto res = ir::Validate(mod);
    if (allowed) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        if (!type_allowed) {
            EXPECT_THAT(
                res.Failure().reason,
                testing::HasSubstr(":2:3 error: var: invalid multisampled texture sample type: '" +
                                   type->FriendlyName()))
                << res.Failure();
        } else {
            EXPECT_THAT(
                res.Failure().reason,
                testing::HasSubstr(":2:3 error: var: invalid multisampled texture dimension: '" +
                                   std::string(ToString(dim))))
                << res.Failure();
        }
    }
}

INSTANTIATE_TEST_SUITE_P(
    IR_ValidatorTest,
    Type_MultisampledTextureTypeAndDimension,
    testing::Combine(testing::Values(std::make_tuple(true, TypeBuilder<f32>),
                                     std::make_tuple(true, TypeBuilder<i32>),
                                     std::make_tuple(true, TypeBuilder<u32>),
                                     std::make_tuple(false, TypeBuilder<f16>),
                                     std::make_tuple(false, TypeBuilder<core::type::Bool>),
                                     std::make_tuple(false, TypeBuilder<core::type::Void>)),
                     testing::Values(std::make_tuple(false, core::type::TextureDimension::k1d),
                                     std::make_tuple(true, core::type::TextureDimension::k2d),
                                     std::make_tuple(false, core::type::TextureDimension::k2dArray),
                                     std::make_tuple(false, core::type::TextureDimension::k3d),
                                     std::make_tuple(false, core::type::TextureDimension::kCube),
                                     std::make_tuple(false,
                                                     core::type::TextureDimension::kCubeArray),
                                     std::make_tuple(false, core::type::TextureDimension::kNone))));

using Type_StorageTextureDimension = IRTestParamHelper<std::tuple<
    /* allowed */ bool,
    /* dim */ core::type::TextureDimension>>;

TEST_P(Type_StorageTextureDimension, Test) {
    bool allowed = std::get<0>(GetParam());
    auto dim = std::get<1>(GetParam());

    auto* v =
        b.Var("v", AddressSpace::kHandle,
              ty.storage_texture(dim, core::TexelFormat::kRgba32Float, core::Access::kReadWrite),
              core::Access::kRead);
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    if (allowed) {
        auto res = ir::Validate(mod);
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        auto res = ir::Validate(mod);
        ASSERT_NE(res, Success) << res.Failure();
        EXPECT_THAT(res.Failure().reason,
                    testing::HasSubstr(
                        dim != type::TextureDimension::kNone
                            ? R"(:2:3 error: var: dimension ')" + std::string(ToString(dim)) +
                                  R"(' for storage textures does not in WGSL yet)"
                            : R"(:2:3 error: var: invalid texture dimension 'none')"))
            << res.Failure();
    }
}

INSTANTIATE_TEST_SUITE_P(
    IR_ValidatorTest,
    Type_StorageTextureDimension,
    testing::Values(std::make_tuple(true, core::type::TextureDimension::k2d),
                    std::make_tuple(false, core::type::TextureDimension::kCube),
                    std::make_tuple(false, core::type::TextureDimension::kCubeArray),
                    std::make_tuple(false, core::type::TextureDimension::kNone)));

using Type_InputAttachmentComponentType = TypeTest;

TEST_P(Type_InputAttachmentComponentType, Test) {
    bool allowed = std::get<0>(GetParam());
    auto* type = std::get<1>(GetParam())(ty);
    b.Append(mod.root_block, [&] {
        auto* var = b.Var("m", AddressSpace::kHandle, ty.input_attachment(type));
        var->SetBindingPoint(0, 0);
    });

    auto res = ir::Validate(mod);
    if (allowed) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(
            res.Failure().reason,
            testing::HasSubstr(":2:3 error: var: invalid input attachment component type: '" +
                               type->FriendlyName()))
            << res.Failure();
    }
}

INSTANTIATE_TEST_SUITE_P(IR_ValidatorTest,
                         Type_InputAttachmentComponentType,
                         testing::Values(std::make_tuple(true, TypeBuilder<f32>),
                                         std::make_tuple(true, TypeBuilder<i32>),
                                         std::make_tuple(true, TypeBuilder<u32>),
                                         std::make_tuple(false, TypeBuilder<f16>),
                                         std::make_tuple(false, TypeBuilder<core::type::Bool>),
                                         std::make_tuple(false, TypeBuilder<core::type::Void>)));

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
        EXPECT_THAT(res.Failure().reason,
                    testing::HasSubstr("3:5 error: var: reference types are not permitted"))
            << res.Failure();
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
        EXPECT_THAT(res.Failure().reason, testing::HasSubstr("reference types are not permitted"))
            << res.Failure();
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
        EXPECT_THAT(res.Failure().reason, testing::HasSubstr("reference types are not permitted"))
            << res.Failure();
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
        EXPECT_THAT(res.Failure().reason, testing::HasSubstr("reference types are not permitted"))
            << res.Failure();
    }
}

INSTANTIATE_TEST_SUITE_P(NonRefTypes,
                         IR_ValidatorRefTypeTest,
                         testing::Combine(/* holds_ref */ testing::Values(false),
                                          /* refs_allowed */ testing::Values(false, true),
                                          /* type_builder */
                                          testing::Values(TypeBuilder<i32>,
                                                          TypeBuilder<bool>,
                                                          TypeBuilder<vec4f>,
                                                          TypeBuilder<array<f32, 3>>)));

INSTANTIATE_TEST_SUITE_P(RefTypes,
                         IR_ValidatorRefTypeTest,
                         testing::Combine(/* holds_ref */ testing::Values(true),
                                          /* refs_allowed */ testing::Values(false, true),
                                          /* type_builder */
                                          testing::Values(RefTypeBuilder<i32>,
                                                          RefTypeBuilder<bool>,
                                                          RefTypeBuilder<vec4f>)));

TEST_F(IR_ValidatorTest, PointerToPointer) {
    auto* type = ty.ptr<function, ptr<function, i32>>();
    auto* fn = b.Function("my_func", ty.void_());
    fn->SetParams(Vector{b.FunctionParam(type)});
    b.Append(fn->Block(), [&] {  //
        b.Return(fn);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr("pointers to pointers are not allowed"))
        << res.Failure();
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
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr("pointers to void are not permitted"))
        << res.Failure();
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
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr("nested reference types are not permitted"))
        << res.Failure();
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
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr("references to void are not permitted"))
        << res.Failure();
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
        EXPECT_THAT(res.Failure().reason,
                    testing::HasSubstr("3:5 error: var: 8-bit integer types are not permitted"))
            << res.Failure();
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
        EXPECT_THAT(res.Failure().reason,
                    testing::HasSubstr("8-bit integer types are not permitted"))
            << res.Failure();
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
        EXPECT_THAT(res.Failure().reason,
                    testing::HasSubstr("8-bit integer types are not permitted"))
            << res.Failure();
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
        EXPECT_THAT(res.Failure().reason,
                    testing::HasSubstr("8-bit integer types are not permitted"))
            << res.Failure();
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
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:5 error: let: 8-bit integer types are not permitted
    %l:u8 = let 1u8
    ^^^^^
)")) << res.Failure();
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
        EXPECT_THAT(res.Failure().reason,
                    testing::HasSubstr("3:5 error: var: 64-bit integer types are not permitted"))
            << res.Failure();
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
        EXPECT_THAT(res.Failure().reason,
                    testing::HasSubstr("64-bit integer types are not permitted"))
            << res.Failure();
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
        EXPECT_THAT(res.Failure().reason,
                    testing::HasSubstr("64-bit integer types are not permitted"))
            << res.Failure();
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
        EXPECT_THAT(res.Failure().reason,
                    testing::HasSubstr("64-bit integer types are not permitted"))
            << res.Failure();
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
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:5 error: let: 64-bit integer types are not permitted
    %l:u64 = let 1u64
    ^^^^^^
)")) << res.Failure();
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

using AddressSpace_AccessMode = IRTestParamHelper<std::tuple<
    /* address */ AddressSpace,
    /* access mode */ core::Access>>;

TEST_P(AddressSpace_AccessMode, Test) {
    auto aspace = std::get<0>(GetParam());
    auto access = std::get<1>(GetParam());

    if (aspace == AddressSpace::kFunction) {
        auto* fn = b.Function("my_func", ty.void_());
        b.Append(fn->Block(), [&] {
            b.Var("v", aspace, ty.u32(), access);
            b.Return(fn);
        });
    } else {
        const core::type::Type* sampler_ty = ty.sampler();
        const core::type::Type* u32_ty = ty.u32();
        auto* type = aspace == AddressSpace::kHandle ? sampler_ty : u32_ty;
        auto* v = b.Var("v", aspace, type, access);
        if (aspace != AddressSpace::kPrivate && aspace != AddressSpace::kWorkgroup &&
            aspace != AddressSpace::kImmediate) {
            v->SetBindingPoint(0, 0);
        }
        mod.root_block->Append(v);
    }

    const char* expected_error = nullptr;
    switch (access) {
        case core::Access::kWrite:
            if (aspace == AddressSpace::kUniform || aspace == AddressSpace::kHandle) {
                expected_error = "uniform and handle pointers must be read access";
            } else if (aspace == AddressSpace::kWorkgroup) {
                expected_error = "workgroup pointers must be read_write access";
            } else if (aspace == AddressSpace::kStorage) {
                expected_error =
                    "vars in the 'storage' address space must have access 'read' or 'read-write'";
            } else if (aspace == AddressSpace::kImmediate) {
                expected_error = "immediate pointers must be read access";
            }
            break;
        case core::Access::kReadWrite:
            if (aspace == AddressSpace::kUniform || aspace == AddressSpace::kHandle) {
                expected_error = "uniform and handle pointers must be read access";
            } else if (aspace == AddressSpace::kImmediate) {
                expected_error = "immediate pointers must be read access";
            }
            break;
        case core::Access::kRead:
            if (aspace == AddressSpace::kWorkgroup) {
                expected_error = "workgroup pointers must be read_write access";
            }
            break;
        default:
            break;
    }
    auto res = ir::Validate(mod);
    if (expected_error == nullptr) {
        ASSERT_EQ(res, Success);
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason, testing::HasSubstr(expected_error));
    }
}

INSTANTIATE_TEST_SUITE_P(IR_ValidatorTest,
                         AddressSpace_AccessMode,
                         testing::Combine(testing::Values(AddressSpace::kFunction,
                                                          AddressSpace::kPrivate,
                                                          AddressSpace::kWorkgroup,
                                                          AddressSpace::kUniform,
                                                          AddressSpace::kStorage,
                                                          AddressSpace::kHandle,
                                                          AddressSpace::kImmediate),
                                          testing::Values(core::Access::kRead,
                                                          core::Access::kWrite,
                                                          core::Access::kReadWrite)));

}  // namespace tint::core::ir

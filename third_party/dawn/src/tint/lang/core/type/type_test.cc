// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/lang/core/type/abstract_float.h"
#include "src/tint/lang/core/type/abstract_int.h"
#include "src/tint/lang/core/type/array_count.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/helper_test.h"
#include "src/tint/lang/core/type/reference.h"

namespace tint::core::type {
namespace {

struct TypeTest : public TestHelper {
    const AbstractFloat* af = create<AbstractFloat>();
    const AbstractInt* ai = create<AbstractInt>();
    const F32* f32 = create<F32>();
    const F16* f16 = create<F16>();
    const I32* i32 = create<I32>();
    const U32* u32 = create<U32>();
    const Vector* vec2_f32 = create<Vector>(f32, 2u);
    const Vector* vec3_f32 = create<Vector>(f32, 3u);
    const Vector* vec3_f16 = create<Vector>(f16, 3u);
    const Vector* vec4_f32 = create<Vector>(f32, 4u);
    const Vector* vec3_u32 = create<Vector>(u32, 3u);
    const Vector* vec3_i32 = create<Vector>(i32, 3u);
    const Vector* vec3_af = create<Vector>(af, 3u);
    const Vector* vec3_ai = create<Vector>(ai, 3u);
    const Matrix* mat2x4_f32 = create<Matrix>(vec4_f32, 2u);
    const Matrix* mat3x4_f32 = create<Matrix>(vec4_f32, 3u);
    const Matrix* mat4x2_f32 = create<Matrix>(vec2_f32, 4u);
    const Matrix* mat4x3_f32 = create<Matrix>(vec3_f32, 4u);
    const Matrix* mat4x3_f16 = create<Matrix>(vec3_f16, 4u);
    const Matrix* mat4x3_af = create<Matrix>(vec3_af, 4u);
    const Reference* ref_u32 =
        create<Reference>(core::AddressSpace::kPrivate, u32, core::Access::kReadWrite);
    const Struct* str_f32 = create<Struct>(Sym("str_f32"),
                                           tint::Vector{
                                               create<StructMember>(
                                                   /* name */ Sym("x"),
                                                   /* type */ f32,
                                                   /* index */ 0u,
                                                   /* offset */ 0u,
                                                   /* align */ 4u,
                                                   /* size */ 4u,
                                                   /* attributes */ core::IOAttributes{}),
                                           },
                                           /* align*/ 4u,
                                           /* size*/ 4u,
                                           /* size_no_padding*/ 4u);
    const Struct* str_f16 = create<Struct>(Sym("str_f16"),
                                           tint::Vector{
                                               create<StructMember>(
                                                   /* name */ Sym("x"),
                                                   /* type */ f16,
                                                   /* index */ 0u,
                                                   /* offset */ 0u,
                                                   /* align */ 4u,
                                                   /* size */ 4u,
                                                   /* attributes */ core::IOAttributes{}),
                                           },
                                           /* align*/ 4u,
                                           /* size*/ 4u,
                                           /* size_no_padding*/ 4u);
    Struct* str_af = create<Struct>(Sym("str_af"),
                                    tint::Vector{
                                        create<StructMember>(
                                            /* name */ Sym("x"),
                                            /* type */ af,
                                            /* index */ 0u,
                                            /* offset */ 0u,
                                            /* align */ 4u,
                                            /* size */ 4u,
                                            /* attributes */ core::IOAttributes{}),
                                    },
                                    /* align*/ 4u,
                                    /* size*/ 4u,
                                    /* size_no_padding*/ 4u);
    const Array* arr_i32 = create<Array>(
        /* element */ i32,
        /* count */ create<ConstantArrayCount>(5u),
        /* align */ 4u,
        /* size */ 5u * 4u,
        /* stride */ 5u * 4u,
        /* implicit_stride */ 5u * 4u);
    const Array* arr_ai = create<Array>(
        /* element */ ai,
        /* count */ create<ConstantArrayCount>(5u),
        /* align */ 4u,
        /* size */ 5u * 4u,
        /* stride */ 5u * 4u,
        /* implicit_stride */ 5u * 4u);
    const Array* arr_vec3_i32 = create<Array>(
        /* element */ vec3_i32,
        /* count */ create<ConstantArrayCount>(5u),
        /* align */ 16u,
        /* size */ 5u * 16u,
        /* stride */ 5u * 16u,
        /* implicit_stride */ 5u * 16u);
    const Array* arr_vec3_ai = create<Array>(
        /* element */ vec3_ai,
        /* count */ create<ConstantArrayCount>(5u),
        /* align */ 16u,
        /* size */ 5u * 16u,
        /* stride */ 5u * 16u,
        /* implicit_stride */ 5u * 16u);
    const Array* arr_mat4x3_f16 = create<Array>(
        /* element */ mat4x3_f16,
        /* count */ create<ConstantArrayCount>(5u),
        /* align */ 32u,
        /* size */ 5u * 32u,
        /* stride */ 5u * 32u,
        /* implicit_stride */ 5u * 32u);
    const Array* arr_mat4x3_f32 = create<Array>(
        /* element */ mat4x3_f32,
        /* count */ create<ConstantArrayCount>(5u),
        /* align */ 64u,
        /* size */ 5u * 64u,
        /* stride */ 5u * 64u,
        /* implicit_stride */ 5u * 64u);
    const Array* arr_mat4x3_af = create<Array>(
        /* element */ mat4x3_af,
        /* count */ create<ConstantArrayCount>(5u),
        /* align */ 64u,
        /* size */ 5u * 64u,
        /* stride */ 5u * 64u,
        /* implicit_stride */ 5u * 64u);
    const Array* arr_str_f16 = create<Array>(
        /* element */ str_f16,
        /* count */ create<ConstantArrayCount>(5u),
        /* align */ 4u,
        /* size */ 5u * 4u,
        /* stride */ 5u * 4u,
        /* implicit_stride */ 5u * 4u);
    const Array* arr_str_af = create<Array>(
        /* element */ str_af,
        /* count */ create<ConstantArrayCount>(5u),
        /* align */ 4u,
        /* size */ 5u * 4u,
        /* stride */ 5u * 4u,
        /* implicit_stride */ 5u * 4u);

    TypeTest() { str_af->SetConcreteTypes(tint::Vector{str_f32, str_f16}); }
};

TEST_F(TypeTest, ConversionRank) {
    EXPECT_EQ(Type::ConversionRank(i32, i32), 0u);
    EXPECT_EQ(Type::ConversionRank(f32, f32), 0u);
    EXPECT_EQ(Type::ConversionRank(u32, u32), 0u);
    EXPECT_EQ(Type::ConversionRank(vec3_f32, vec3_f32), 0u);
    EXPECT_EQ(Type::ConversionRank(vec3_f16, vec3_f16), 0u);
    EXPECT_EQ(Type::ConversionRank(vec4_f32, vec4_f32), 0u);
    EXPECT_EQ(Type::ConversionRank(vec3_u32, vec3_u32), 0u);
    EXPECT_EQ(Type::ConversionRank(vec3_i32, vec3_i32), 0u);
    EXPECT_EQ(Type::ConversionRank(vec3_af, vec3_af), 0u);
    EXPECT_EQ(Type::ConversionRank(vec3_ai, vec3_ai), 0u);
    EXPECT_EQ(Type::ConversionRank(mat3x4_f32, mat3x4_f32), 0u);
    EXPECT_EQ(Type::ConversionRank(mat4x3_f32, mat4x3_f32), 0u);
    EXPECT_EQ(Type::ConversionRank(mat4x3_f16, mat4x3_f16), 0u);
    EXPECT_EQ(Type::ConversionRank(arr_vec3_ai, arr_vec3_ai), 0u);
    EXPECT_EQ(Type::ConversionRank(arr_mat4x3_f16, arr_mat4x3_f16), 0u);
    EXPECT_EQ(Type::ConversionRank(mat4x3_af, mat4x3_af), 0u);
    EXPECT_EQ(Type::ConversionRank(arr_mat4x3_af, arr_mat4x3_af), 0u);
    EXPECT_EQ(Type::ConversionRank(ref_u32, u32), 0u);

    EXPECT_EQ(Type::ConversionRank(af, f32), 1u);
    EXPECT_EQ(Type::ConversionRank(vec3_af, vec3_f32), 1u);
    EXPECT_EQ(Type::ConversionRank(mat4x3_af, mat4x3_f32), 1u);
    EXPECT_EQ(Type::ConversionRank(arr_mat4x3_af, arr_mat4x3_f32), 1u);
    EXPECT_EQ(Type::ConversionRank(af, f16), 2u);
    EXPECT_EQ(Type::ConversionRank(vec3_af, vec3_f16), 2u);
    EXPECT_EQ(Type::ConversionRank(mat4x3_af, mat4x3_f16), 2u);
    EXPECT_EQ(Type::ConversionRank(arr_mat4x3_af, arr_mat4x3_f16), 2u);
    EXPECT_EQ(Type::ConversionRank(ai, i32), 3u);
    EXPECT_EQ(Type::ConversionRank(vec3_ai, vec3_i32), 3u);
    EXPECT_EQ(Type::ConversionRank(arr_ai, arr_i32), 3u);
    EXPECT_EQ(Type::ConversionRank(arr_vec3_ai, arr_vec3_i32), 3u);
    EXPECT_EQ(Type::ConversionRank(ai, u32), 4u);
    EXPECT_EQ(Type::ConversionRank(vec3_ai, vec3_u32), 4u);
    EXPECT_EQ(Type::ConversionRank(ai, af), 5u);
    EXPECT_EQ(Type::ConversionRank(ai, f32), 6u);
    EXPECT_EQ(Type::ConversionRank(ai, f16), 7u);
    EXPECT_EQ(Type::ConversionRank(str_af, str_f32), 1u);
    EXPECT_EQ(Type::ConversionRank(str_af, str_f16), 2u);

    EXPECT_EQ(Type::ConversionRank(i32, f32), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(f32, u32), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(u32, i32), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(vec3_u32, vec3_f32), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(vec3_f32, vec4_f32), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(mat3x4_f32, mat4x3_f32), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(mat4x3_f32, mat3x4_f32), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(mat4x3_f32, mat4x3_af), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(arr_vec3_i32, arr_vec3_ai), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(arr_mat4x3_f32, arr_mat4x3_af), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(arr_mat4x3_f16, arr_mat4x3_f32), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(f32, af), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(f16, af), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(vec3_f16, vec3_af), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(mat4x3_f16, mat4x3_af), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(i32, af), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(u32, af), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(af, ai), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(f32, ai), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(f16, ai), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(str_f32, str_f16), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(str_f16, str_f32), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(str_f32, str_af), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(str_f16, str_af), Type::kNoConversion);
}

TEST_F(TypeTest, Elements) {
    EXPECT_EQ(f32->Elements(), (TypeAndCount{nullptr, 0u}));
    EXPECT_EQ(f16->Elements(), (TypeAndCount{nullptr, 0u}));
    EXPECT_EQ(i32->Elements(), (TypeAndCount{nullptr, 0u}));
    EXPECT_EQ(u32->Elements(), (TypeAndCount{nullptr, 0u}));
    EXPECT_EQ(vec2_f32->Elements(), (TypeAndCount{f32, 2u}));
    EXPECT_EQ(vec3_f16->Elements(), (TypeAndCount{f16, 3u}));
    EXPECT_EQ(vec4_f32->Elements(), (TypeAndCount{f32, 4u}));
    EXPECT_EQ(vec3_u32->Elements(), (TypeAndCount{u32, 3u}));
    EXPECT_EQ(vec3_i32->Elements(), (TypeAndCount{i32, 3u}));
    EXPECT_EQ(mat2x4_f32->Elements(), (TypeAndCount{vec4_f32, 2u}));
    EXPECT_EQ(mat4x2_f32->Elements(), (TypeAndCount{vec2_f32, 4u}));
    EXPECT_EQ(mat4x3_f16->Elements(), (TypeAndCount{vec3_f16, 4u}));
    EXPECT_EQ(str_f16->Elements(), (TypeAndCount{nullptr, 1u}));
    EXPECT_EQ(arr_i32->Elements(), (TypeAndCount{i32, 5u}));
    EXPECT_EQ(arr_vec3_i32->Elements(), (TypeAndCount{vec3_i32, 5u}));
    EXPECT_EQ(arr_mat4x3_f16->Elements(), (TypeAndCount{mat4x3_f16, 5u}));
    EXPECT_EQ(arr_mat4x3_af->Elements(), (TypeAndCount{mat4x3_af, 5u}));
    EXPECT_EQ(arr_str_f16->Elements(), (TypeAndCount{str_f16, 5u}));
}

TEST_F(TypeTest, ElementsWithCustomInvalid) {
    EXPECT_EQ(f32->Elements(f32, 42), (TypeAndCount{f32, 42}));
    EXPECT_EQ(f16->Elements(f16, 42), (TypeAndCount{f16, 42}));
    EXPECT_EQ(i32->Elements(i32, 42), (TypeAndCount{i32, 42}));
    EXPECT_EQ(u32->Elements(u32, 42), (TypeAndCount{u32, 42}));
    EXPECT_EQ(vec2_f32->Elements(vec2_f32, 42), (TypeAndCount{f32, 2u}));
    EXPECT_EQ(vec3_f16->Elements(vec3_f16, 42), (TypeAndCount{f16, 3u}));
    EXPECT_EQ(vec4_f32->Elements(vec4_f32, 42), (TypeAndCount{f32, 4u}));
    EXPECT_EQ(vec3_u32->Elements(vec3_u32, 42), (TypeAndCount{u32, 3u}));
    EXPECT_EQ(vec3_i32->Elements(vec3_i32, 42), (TypeAndCount{i32, 3u}));
    EXPECT_EQ(mat2x4_f32->Elements(mat2x4_f32, 42), (TypeAndCount{vec4_f32, 2u}));
    EXPECT_EQ(mat4x2_f32->Elements(mat4x2_f32, 42), (TypeAndCount{vec2_f32, 4u}));
    EXPECT_EQ(mat4x3_f16->Elements(mat4x3_f16, 42), (TypeAndCount{vec3_f16, 4u}));
    EXPECT_EQ(str_f16->Elements(str_f16, 42), (TypeAndCount{str_f16, 1}));
    EXPECT_EQ(arr_i32->Elements(arr_i32, 42), (TypeAndCount{i32, 5u}));
    EXPECT_EQ(arr_vec3_i32->Elements(arr_vec3_i32, 42), (TypeAndCount{vec3_i32, 5u}));
    EXPECT_EQ(arr_mat4x3_f16->Elements(arr_mat4x3_f16, 42), (TypeAndCount{mat4x3_f16, 5u}));
    EXPECT_EQ(arr_mat4x3_af->Elements(arr_mat4x3_af, 42), (TypeAndCount{mat4x3_af, 5u}));
    EXPECT_EQ(arr_str_f16->Elements(arr_str_f16, 42), (TypeAndCount{str_f16, 5u}));
}

TEST_F(TypeTest, Element) {
    EXPECT_TYPE(f32->Element(0), nullptr);
    EXPECT_TYPE(f16->Element(1), nullptr);
    EXPECT_TYPE(i32->Element(2), nullptr);
    EXPECT_TYPE(u32->Element(3), nullptr);
    EXPECT_TYPE(vec2_f32->Element(0), f32);
    EXPECT_TYPE(vec2_f32->Element(1), f32);
    EXPECT_TYPE(vec2_f32->Element(2), nullptr);
    EXPECT_TYPE(vec3_f16->Element(0), f16);
    EXPECT_TYPE(vec4_f32->Element(3), f32);
    EXPECT_TYPE(vec4_f32->Element(4), nullptr);
    EXPECT_TYPE(vec3_u32->Element(2), u32);
    EXPECT_TYPE(vec3_u32->Element(3), nullptr);
    EXPECT_TYPE(vec3_i32->Element(1), i32);
    EXPECT_TYPE(vec3_i32->Element(4), nullptr);
    EXPECT_TYPE(mat2x4_f32->Element(1), vec4_f32);
    EXPECT_TYPE(mat2x4_f32->Element(2), nullptr);
    EXPECT_TYPE(mat4x2_f32->Element(3), vec2_f32);
    EXPECT_TYPE(mat4x2_f32->Element(4), nullptr);
    EXPECT_TYPE(mat4x3_f16->Element(1), vec3_f16);
    EXPECT_TYPE(mat4x3_f16->Element(5), nullptr);
    EXPECT_TYPE(str_f16->Element(0), f16);
    EXPECT_TYPE(str_f16->Element(1), nullptr);
    EXPECT_TYPE(arr_i32->Element(0), i32);
    EXPECT_TYPE(arr_i32->Element(4), i32);
    EXPECT_TYPE(arr_i32->Element(5), nullptr);
    EXPECT_TYPE(arr_vec3_i32->Element(4), vec3_i32);
    EXPECT_TYPE(arr_vec3_i32->Element(5), nullptr);
    EXPECT_TYPE(arr_mat4x3_f16->Element(1), mat4x3_f16);
    EXPECT_TYPE(arr_mat4x3_f16->Element(10), nullptr);
    EXPECT_TYPE(arr_mat4x3_af->Element(2), mat4x3_af);
    EXPECT_TYPE(arr_mat4x3_af->Element(6), nullptr);
    EXPECT_TYPE(arr_str_f16->Element(0), str_f16);
    EXPECT_TYPE(arr_str_f16->Element(1), str_f16);
    EXPECT_TYPE(arr_str_f16->Element(10), nullptr);
}

TEST_F(TypeTest, DeepestElement) {
    EXPECT_TYPE(f32->DeepestElement(), f32);
    EXPECT_TYPE(f16->DeepestElement(), f16);
    EXPECT_TYPE(i32->DeepestElement(), i32);
    EXPECT_TYPE(u32->DeepestElement(), u32);
    EXPECT_TYPE(vec2_f32->DeepestElement(), f32);
    EXPECT_TYPE(vec3_f16->DeepestElement(), f16);
    EXPECT_TYPE(vec4_f32->DeepestElement(), f32);
    EXPECT_TYPE(vec3_u32->DeepestElement(), u32);
    EXPECT_TYPE(vec3_i32->DeepestElement(), i32);
    EXPECT_TYPE(mat2x4_f32->DeepestElement(), f32);
    EXPECT_TYPE(mat4x2_f32->DeepestElement(), f32);
    EXPECT_TYPE(mat4x3_f16->DeepestElement(), f16);
    EXPECT_TYPE(str_f16->DeepestElement(), str_f16);
    EXPECT_TYPE(arr_i32->DeepestElement(), i32);
    EXPECT_TYPE(arr_vec3_i32->DeepestElement(), i32);
    EXPECT_TYPE(arr_mat4x3_f16->DeepestElement(), f16);
    EXPECT_TYPE(arr_mat4x3_af->DeepestElement(), af);
    EXPECT_TYPE(arr_str_f16->DeepestElement(), str_f16);
}

TEST_F(TypeTest, Common2) {
    EXPECT_TYPE(Type::Common(tint::Vector{ai, ai}), ai);
    EXPECT_TYPE(Type::Common(tint::Vector{af, af}), af);
    EXPECT_TYPE(Type::Common(tint::Vector{f32, f32}), f32);
    EXPECT_TYPE(Type::Common(tint::Vector{f16, f16}), f16);
    EXPECT_TYPE(Type::Common(tint::Vector{i32, i32}), i32);
    EXPECT_TYPE(Type::Common(tint::Vector{u32, u32}), u32);

    EXPECT_TYPE(Type::Common(tint::Vector{i32, u32}), nullptr);
    EXPECT_TYPE(Type::Common(tint::Vector{u32, f32}), nullptr);
    EXPECT_TYPE(Type::Common(tint::Vector{f32, f16}), nullptr);
    EXPECT_TYPE(Type::Common(tint::Vector{f16, i32}), nullptr);

    EXPECT_TYPE(Type::Common(tint::Vector{ai, af}), af);
    EXPECT_TYPE(Type::Common(tint::Vector{ai, f32}), f32);
    EXPECT_TYPE(Type::Common(tint::Vector{ai, f16}), f16);
    EXPECT_TYPE(Type::Common(tint::Vector{ai, i32}), i32);
    EXPECT_TYPE(Type::Common(tint::Vector{ai, u32}), u32);

    EXPECT_TYPE(Type::Common(tint::Vector{af, ai}), af);
    EXPECT_TYPE(Type::Common(tint::Vector{f32, ai}), f32);
    EXPECT_TYPE(Type::Common(tint::Vector{f16, ai}), f16);
    EXPECT_TYPE(Type::Common(tint::Vector{i32, ai}), i32);
    EXPECT_TYPE(Type::Common(tint::Vector{u32, ai}), u32);

    EXPECT_TYPE(Type::Common(tint::Vector{ai, af}), af);
    EXPECT_TYPE(Type::Common(tint::Vector{f32, af}), f32);
    EXPECT_TYPE(Type::Common(tint::Vector{f16, af}), f16);
    EXPECT_TYPE(Type::Common(tint::Vector{i32, af}), nullptr);
    EXPECT_TYPE(Type::Common(tint::Vector{u32, af}), nullptr);

    EXPECT_TYPE(Type::Common(tint::Vector{af, ai}), af);
    EXPECT_TYPE(Type::Common(tint::Vector{af, f32}), f32);
    EXPECT_TYPE(Type::Common(tint::Vector{af, f16}), f16);
    EXPECT_TYPE(Type::Common(tint::Vector{af, i32}), nullptr);
    EXPECT_TYPE(Type::Common(tint::Vector{af, u32}), nullptr);

    EXPECT_TYPE(Type::Common(tint::Vector{vec3_ai, vec3_ai}), vec3_ai);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_af, vec3_af}), vec3_af);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_f32, vec3_f32}), vec3_f32);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_f16, vec3_f16}), vec3_f16);
    EXPECT_TYPE(Type::Common(tint::Vector{vec4_f32, vec4_f32}), vec4_f32);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_u32, vec3_u32}), vec3_u32);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_i32, vec3_i32}), vec3_i32);

    EXPECT_TYPE(Type::Common(tint::Vector{vec3_ai, vec3_f32}), vec3_f32);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_ai, vec3_f16}), vec3_f16);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_ai, vec4_f32}), nullptr);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_ai, vec3_u32}), vec3_u32);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_ai, vec3_i32}), vec3_i32);

    EXPECT_TYPE(Type::Common(tint::Vector{vec3_f32, vec3_ai}), vec3_f32);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_f16, vec3_ai}), vec3_f16);
    EXPECT_TYPE(Type::Common(tint::Vector{vec4_f32, vec3_ai}), nullptr);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_u32, vec3_ai}), vec3_u32);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_i32, vec3_ai}), vec3_i32);

    EXPECT_TYPE(Type::Common(tint::Vector{vec3_af, vec3_f32}), vec3_f32);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_af, vec3_f16}), vec3_f16);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_af, vec4_f32}), nullptr);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_af, vec3_u32}), nullptr);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_af, vec3_i32}), nullptr);

    EXPECT_TYPE(Type::Common(tint::Vector{vec3_f32, vec3_af}), vec3_f32);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_f16, vec3_af}), vec3_f16);
    EXPECT_TYPE(Type::Common(tint::Vector{vec4_f32, vec3_af}), nullptr);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_u32, vec3_af}), nullptr);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_i32, vec3_af}), nullptr);

    EXPECT_TYPE(Type::Common(tint::Vector{mat4x3_af, mat4x3_af}), mat4x3_af);
    EXPECT_TYPE(Type::Common(tint::Vector{mat3x4_f32, mat3x4_f32}), mat3x4_f32);
    EXPECT_TYPE(Type::Common(tint::Vector{mat4x3_f32, mat4x3_f32}), mat4x3_f32);
    EXPECT_TYPE(Type::Common(tint::Vector{mat4x3_f16, mat4x3_f16}), mat4x3_f16);

    EXPECT_TYPE(Type::Common(tint::Vector{mat4x3_af, mat3x4_f32}), nullptr);
    EXPECT_TYPE(Type::Common(tint::Vector{mat4x3_af, mat4x3_f32}), mat4x3_f32);
    EXPECT_TYPE(Type::Common(tint::Vector{mat4x3_af, mat4x3_f16}), mat4x3_f16);

    EXPECT_TYPE(Type::Common(tint::Vector{mat3x4_f32, mat4x3_af}), nullptr);
    EXPECT_TYPE(Type::Common(tint::Vector{mat4x3_f32, mat4x3_af}), mat4x3_f32);
    EXPECT_TYPE(Type::Common(tint::Vector{mat4x3_f16, mat4x3_af}), mat4x3_f16);

    EXPECT_TYPE(Type::Common(tint::Vector{arr_mat4x3_f32, arr_mat4x3_f16}), nullptr);
    EXPECT_TYPE(Type::Common(tint::Vector{arr_mat4x3_f32, arr_mat4x3_af}), arr_mat4x3_f32);
    EXPECT_TYPE(Type::Common(tint::Vector{arr_mat4x3_f16, arr_mat4x3_af}), arr_mat4x3_f16);
}

TEST_F(TypeTest, Common3) {
    EXPECT_TYPE(Type::Common(tint::Vector{ai, ai, ai}), ai);
    EXPECT_TYPE(Type::Common(tint::Vector{af, af, af}), af);
    EXPECT_TYPE(Type::Common(tint::Vector{f32, f32, f32}), f32);
    EXPECT_TYPE(Type::Common(tint::Vector{f16, f16, f16}), f16);
    EXPECT_TYPE(Type::Common(tint::Vector{i32, i32, i32}), i32);
    EXPECT_TYPE(Type::Common(tint::Vector{u32, u32, u32}), u32);

    EXPECT_TYPE(Type::Common(tint::Vector{ai, af, ai}), af);
    EXPECT_TYPE(Type::Common(tint::Vector{ai, f32, ai}), f32);
    EXPECT_TYPE(Type::Common(tint::Vector{ai, f16, ai}), f16);
    EXPECT_TYPE(Type::Common(tint::Vector{ai, i32, ai}), i32);
    EXPECT_TYPE(Type::Common(tint::Vector{ai, u32, ai}), u32);

    EXPECT_TYPE(Type::Common(tint::Vector{af, ai, af}), af);
    EXPECT_TYPE(Type::Common(tint::Vector{f32, ai, f32}), f32);
    EXPECT_TYPE(Type::Common(tint::Vector{f16, ai, f16}), f16);
    EXPECT_TYPE(Type::Common(tint::Vector{i32, ai, i32}), i32);
    EXPECT_TYPE(Type::Common(tint::Vector{u32, ai, u32}), u32);

    EXPECT_TYPE(Type::Common(tint::Vector{ai, f32, ai}), f32);
    EXPECT_TYPE(Type::Common(tint::Vector{ai, f16, ai}), f16);
    EXPECT_TYPE(Type::Common(tint::Vector{ai, i32, ai}), i32);
    EXPECT_TYPE(Type::Common(tint::Vector{ai, u32, ai}), u32);

    EXPECT_TYPE(Type::Common(tint::Vector{f32, ai, f32}), f32);
    EXPECT_TYPE(Type::Common(tint::Vector{f16, ai, f16}), f16);
    EXPECT_TYPE(Type::Common(tint::Vector{i32, ai, i32}), i32);
    EXPECT_TYPE(Type::Common(tint::Vector{u32, ai, u32}), u32);

    EXPECT_TYPE(Type::Common(tint::Vector{af, f32, af}), f32);
    EXPECT_TYPE(Type::Common(tint::Vector{af, f16, af}), f16);
    EXPECT_TYPE(Type::Common(tint::Vector{af, i32, af}), nullptr);
    EXPECT_TYPE(Type::Common(tint::Vector{af, u32, af}), nullptr);

    EXPECT_TYPE(Type::Common(tint::Vector{f32, af, f32}), f32);
    EXPECT_TYPE(Type::Common(tint::Vector{f16, af, f16}), f16);
    EXPECT_TYPE(Type::Common(tint::Vector{i32, af, i32}), nullptr);
    EXPECT_TYPE(Type::Common(tint::Vector{u32, af, u32}), nullptr);

    EXPECT_TYPE(Type::Common(tint::Vector{ai, af, f32}), f32);
    EXPECT_TYPE(Type::Common(tint::Vector{ai, af, f16}), f16);
    EXPECT_TYPE(Type::Common(tint::Vector{ai, af, i32}), nullptr);
    EXPECT_TYPE(Type::Common(tint::Vector{ai, af, u32}), nullptr);

    EXPECT_TYPE(Type::Common(tint::Vector{vec3_ai, vec3_ai, vec3_ai}), vec3_ai);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_af, vec3_af, vec3_af}), vec3_af);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_f32, vec3_f32, vec3_f32}), vec3_f32);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_f16, vec3_f16, vec3_f16}), vec3_f16);
    EXPECT_TYPE(Type::Common(tint::Vector{vec4_f32, vec4_f32, vec4_f32}), vec4_f32);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_u32, vec3_u32, vec3_u32}), vec3_u32);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_i32, vec3_i32, vec3_i32}), vec3_i32);

    EXPECT_TYPE(Type::Common(tint::Vector{vec3_f32, vec3_ai, vec3_f32}), vec3_f32);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_f16, vec3_ai, vec3_f16}), vec3_f16);
    EXPECT_TYPE(Type::Common(tint::Vector{vec4_f32, vec3_ai, vec4_f32}), nullptr);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_u32, vec3_ai, vec3_u32}), vec3_u32);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_i32, vec3_ai, vec3_i32}), vec3_i32);

    EXPECT_TYPE(Type::Common(tint::Vector{vec3_ai, vec3_f32, vec3_ai}), vec3_f32);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_ai, vec3_f16, vec3_ai}), vec3_f16);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_ai, vec4_f32, vec3_ai}), nullptr);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_ai, vec3_u32, vec3_ai}), vec3_u32);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_ai, vec3_i32, vec3_ai}), vec3_i32);

    EXPECT_TYPE(Type::Common(tint::Vector{vec3_f32, vec3_af, vec3_f32}), vec3_f32);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_f16, vec3_af, vec3_f16}), vec3_f16);
    EXPECT_TYPE(Type::Common(tint::Vector{vec4_f32, vec3_af, vec4_f32}), nullptr);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_u32, vec3_af, vec3_u32}), nullptr);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_i32, vec3_af, vec3_i32}), nullptr);

    EXPECT_TYPE(Type::Common(tint::Vector{vec3_af, vec3_f32, vec3_af}), vec3_f32);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_af, vec3_f16, vec3_af}), vec3_f16);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_af, vec4_f32, vec3_af}), nullptr);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_af, vec3_u32, vec3_af}), nullptr);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_af, vec3_i32, vec3_af}), nullptr);

    EXPECT_TYPE(Type::Common(tint::Vector{vec3_ai, vec3_af, vec3_f32}), vec3_f32);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_ai, vec3_af, vec3_f16}), vec3_f16);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_ai, vec3_af, vec4_f32}), nullptr);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_ai, vec3_af, vec3_u32}), nullptr);
    EXPECT_TYPE(Type::Common(tint::Vector{vec3_ai, vec3_af, vec3_i32}), nullptr);

    EXPECT_TYPE(Type::Common(tint::Vector{mat4x3_af, mat4x3_af, mat4x3_af}), mat4x3_af);
    EXPECT_TYPE(Type::Common(tint::Vector{mat3x4_f32, mat3x4_f32, mat3x4_f32}), mat3x4_f32);
    EXPECT_TYPE(Type::Common(tint::Vector{mat4x3_f32, mat4x3_f32, mat4x3_f32}), mat4x3_f32);
    EXPECT_TYPE(Type::Common(tint::Vector{mat4x3_f16, mat4x3_f16, mat4x3_f16}), mat4x3_f16);

    EXPECT_TYPE(Type::Common(tint::Vector{mat3x4_f32, mat4x3_af, mat3x4_f32}), nullptr);
    EXPECT_TYPE(Type::Common(tint::Vector{mat4x3_f32, mat4x3_af, mat4x3_f32}), mat4x3_f32);
    EXPECT_TYPE(Type::Common(tint::Vector{mat4x3_f16, mat4x3_af, mat4x3_f16}), mat4x3_f16);

    EXPECT_TYPE(Type::Common(tint::Vector{mat4x3_af, mat3x4_f32, mat4x3_af}), nullptr);
    EXPECT_TYPE(Type::Common(tint::Vector{mat4x3_af, mat4x3_f32, mat4x3_af}), mat4x3_f32);
    EXPECT_TYPE(Type::Common(tint::Vector{mat4x3_af, mat4x3_f16, mat4x3_af}), mat4x3_f16);

    EXPECT_TYPE(Type::Common(tint::Vector{arr_mat4x3_f16, arr_mat4x3_f32, arr_mat4x3_f16}),
                nullptr);
    EXPECT_TYPE(Type::Common(tint::Vector{arr_mat4x3_af, arr_mat4x3_f32, arr_mat4x3_af}),
                arr_mat4x3_f32);
    EXPECT_TYPE(Type::Common(tint::Vector{arr_mat4x3_af, arr_mat4x3_f16, arr_mat4x3_af}),
                arr_mat4x3_f16);
}

TEST_F(TypeTest, IsAbstract) {
    EXPECT_TRUE(af->IsAbstract());
    EXPECT_TRUE(ai->IsAbstract());
    EXPECT_FALSE(f32->IsAbstract());
    EXPECT_FALSE(f16->IsAbstract());
    EXPECT_FALSE(i32->IsAbstract());
    EXPECT_FALSE(u32->IsAbstract());
    EXPECT_FALSE(vec2_f32->IsAbstract());
    EXPECT_FALSE(vec3_f32->IsAbstract());
    EXPECT_FALSE(vec3_f16->IsAbstract());
    EXPECT_FALSE(vec4_f32->IsAbstract());
    EXPECT_FALSE(vec3_u32->IsAbstract());
    EXPECT_FALSE(vec3_i32->IsAbstract());
    EXPECT_TRUE(vec3_af->IsAbstract());
    EXPECT_TRUE(vec3_ai->IsAbstract());
    EXPECT_FALSE(mat2x4_f32->IsAbstract());
    EXPECT_FALSE(mat3x4_f32->IsAbstract());
    EXPECT_FALSE(mat4x2_f32->IsAbstract());
    EXPECT_FALSE(mat4x3_f32->IsAbstract());
    EXPECT_FALSE(mat4x3_f16->IsAbstract());
    EXPECT_TRUE(mat4x3_af->IsAbstract());
    EXPECT_FALSE(str_f16->IsAbstract());
    EXPECT_TRUE(str_af->IsAbstract());
    EXPECT_FALSE(arr_i32->IsAbstract());
    EXPECT_TRUE(arr_ai->IsAbstract());
    EXPECT_FALSE(arr_vec3_i32->IsAbstract());
    EXPECT_TRUE(arr_vec3_ai->IsAbstract());
    EXPECT_FALSE(arr_mat4x3_f16->IsAbstract());
    EXPECT_FALSE(arr_mat4x3_f32->IsAbstract());
    EXPECT_TRUE(arr_mat4x3_af->IsAbstract());
    EXPECT_FALSE(arr_str_f16->IsAbstract());
    EXPECT_TRUE(arr_str_af->IsAbstract());
}

}  // namespace
}  // namespace tint::core::type

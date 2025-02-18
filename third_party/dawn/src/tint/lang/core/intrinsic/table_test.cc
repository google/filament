// Copyright 2021 The Dawn & Tint Authors
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

#include "src/tint/lang/core/intrinsic/table.h"

#include <utility>

#include "gmock/gmock.h"
#include "src/tint/lang/core/intrinsic/dialect.h"
#include "src/tint/lang/core/intrinsic/table_data.h"
#include "src/tint/lang/core/type/atomic.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/external_texture.h"
#include "src/tint/lang/core/type/helper_test.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/utils/containers/vector.h"

namespace tint::core::intrinsic {
namespace {

using ::testing::HasSubstr;

class CoreIntrinsicTableTest : public testing::Test, public ProgramBuilder {
  public:
    Table<Dialect> table{Types(), Symbols()};
};

TEST_F(CoreIntrinsicTableTest, MatchF32) {
    auto* f32 = create<type::F32>();
    auto result = table.Lookup(BuiltinFn::kCos, Empty, Vector{f32}, EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, f32);
    ASSERT_EQ(result->parameters.Length(), 1u);
    EXPECT_EQ(result->parameters[0].type, f32);
}

TEST_F(CoreIntrinsicTableTest, MismatchF32) {
    auto* i32 = create<type::I32>();
    auto result = table.Lookup(BuiltinFn::kCos, Empty, Vector{i32}, EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    ASSERT_THAT(result.Failure().Plain(), HasSubstr("no matching call"));
}

TEST_F(CoreIntrinsicTableTest, MatchU32) {
    auto* f32 = create<type::F32>();
    auto* u32 = create<type::U32>();
    auto* vec2f = create<type::Vector>(f32, 2u);
    auto result =
        table.Lookup(BuiltinFn::kUnpack2X16Float, Empty, Vector{u32}, EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, vec2f);
    ASSERT_EQ(result->parameters.Length(), 1u);
    EXPECT_EQ(result->parameters[0].type, u32);
}

TEST_F(CoreIntrinsicTableTest, MismatchU32) {
    auto* f32 = create<type::F32>();
    auto result =
        table.Lookup(BuiltinFn::kUnpack2X16Float, Empty, Vector{f32}, EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    ASSERT_THAT(result.Failure().Plain(), HasSubstr("no matching call"));
}

TEST_F(CoreIntrinsicTableTest, MatchI32) {
    auto* f32 = create<type::F32>();
    auto* i32 = create<type::I32>();
    auto* vec4f = create<type::Vector>(f32, 4u);
    auto* tex = create<type::SampledTexture>(type::TextureDimension::k1d, f32);
    auto result = table.Lookup(BuiltinFn::kTextureLoad, Empty, Vector{tex, i32, i32},
                               EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, vec4f);
    ASSERT_EQ(result->parameters.Length(), 3u);
    EXPECT_EQ(result->parameters[0].type, tex);
    EXPECT_EQ(result->parameters[0].usage, ParameterUsage::kTexture);
    EXPECT_EQ(result->parameters[1].type, i32);
    EXPECT_EQ(result->parameters[1].usage, ParameterUsage::kCoords);
    EXPECT_EQ(result->parameters[2].type, i32);
    EXPECT_EQ(result->parameters[2].usage, ParameterUsage::kLevel);
}

TEST_F(CoreIntrinsicTableTest, MismatchI32) {
    auto* f32 = create<type::F32>();
    auto* tex = create<type::SampledTexture>(type::TextureDimension::k1d, f32);
    auto result =
        table.Lookup(BuiltinFn::kTextureLoad, Empty, Vector{tex, f32}, EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    ASSERT_THAT(result.Failure().Plain(), HasSubstr("no matching call"));
}

TEST_F(CoreIntrinsicTableTest, MatchIU32AsI32) {
    auto* i32 = create<type::I32>();
    auto result =
        table.Lookup(BuiltinFn::kCountOneBits, Empty, Vector{i32}, EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, i32);
    ASSERT_EQ(result->parameters.Length(), 1u);
    EXPECT_EQ(result->parameters[0].type, i32);
}

TEST_F(CoreIntrinsicTableTest, MatchIU32AsU32) {
    auto* u32 = create<type::U32>();
    auto result =
        table.Lookup(BuiltinFn::kCountOneBits, Empty, Vector{u32}, EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, u32);
    ASSERT_EQ(result->parameters.Length(), 1u);
    EXPECT_EQ(result->parameters[0].type, u32);
}

TEST_F(CoreIntrinsicTableTest, MismatchIU32) {
    auto* f32 = create<type::F32>();
    auto result =
        table.Lookup(BuiltinFn::kCountOneBits, Empty, Vector{f32}, EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    ASSERT_THAT(result.Failure().Plain(), HasSubstr("no matching call"));
}

TEST_F(CoreIntrinsicTableTest, MatchFIU32AsI32) {
    auto* i32 = create<type::I32>();
    auto result =
        table.Lookup(BuiltinFn::kClamp, Empty, Vector{i32, i32, i32}, EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, i32);
    ASSERT_EQ(result->parameters.Length(), 3u);
    EXPECT_EQ(result->parameters[0].type, i32);
    EXPECT_EQ(result->parameters[1].type, i32);
    EXPECT_EQ(result->parameters[2].type, i32);
}

TEST_F(CoreIntrinsicTableTest, MatchFIU32AsU32) {
    auto* u32 = create<type::U32>();
    auto result =
        table.Lookup(BuiltinFn::kClamp, Empty, Vector{u32, u32, u32}, EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, u32);
    ASSERT_EQ(result->parameters.Length(), 3u);
    EXPECT_EQ(result->parameters[0].type, u32);
    EXPECT_EQ(result->parameters[1].type, u32);
    EXPECT_EQ(result->parameters[2].type, u32);
}

TEST_F(CoreIntrinsicTableTest, MatchFIU32AsF32) {
    auto* f32 = create<type::F32>();
    auto result =
        table.Lookup(BuiltinFn::kClamp, Empty, Vector{f32, f32, f32}, EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, f32);
    ASSERT_EQ(result->parameters.Length(), 3u);
    EXPECT_EQ(result->parameters[0].type, f32);
    EXPECT_EQ(result->parameters[1].type, f32);
    EXPECT_EQ(result->parameters[2].type, f32);
}

TEST_F(CoreIntrinsicTableTest, MismatchFIU32) {
    auto* bool_ = create<type::Bool>();
    auto result = table.Lookup(BuiltinFn::kClamp, Empty, Vector{bool_, bool_, bool_},
                               EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    ASSERT_THAT(result.Failure().Plain(), HasSubstr("no matching call"));
}

TEST_F(CoreIntrinsicTableTest, MatchBool) {
    auto* f32 = create<type::F32>();
    auto* bool_ = create<type::Bool>();
    auto result = table.Lookup(BuiltinFn::kSelect, Empty, Vector{f32, f32, bool_},
                               EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, f32);
    ASSERT_EQ(result->parameters.Length(), 3u);
    EXPECT_EQ(result->parameters[0].type, f32);
    EXPECT_EQ(result->parameters[1].type, f32);
    EXPECT_EQ(result->parameters[2].type, bool_);
}

TEST_F(CoreIntrinsicTableTest, MismatchBool) {
    auto* f32 = create<type::F32>();
    auto result =
        table.Lookup(BuiltinFn::kSelect, Empty, Vector{f32, f32, f32}, EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    ASSERT_THAT(result.Failure().Plain(), HasSubstr("no matching call"));
}

TEST_F(CoreIntrinsicTableTest, MatchPointer) {
    auto* i32 = create<type::I32>();
    auto* atomic_i32 = create<type::Atomic>(i32);
    auto* ptr = create<type::Pointer>(AddressSpace::kWorkgroup, atomic_i32, Access::kReadWrite);
    auto result =
        table.Lookup(BuiltinFn::kAtomicLoad, Empty, Vector{ptr}, EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, i32);
    ASSERT_EQ(result->parameters.Length(), 1u);
    EXPECT_EQ(result->parameters[0].type, ptr);
}

TEST_F(CoreIntrinsicTableTest, MismatchPointer) {
    auto* i32 = create<type::I32>();
    auto* atomic_i32 = create<type::Atomic>(i32);
    auto result =
        table.Lookup(BuiltinFn::kAtomicLoad, Empty, Vector{atomic_i32}, EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    ASSERT_THAT(result.Failure().Plain(), HasSubstr("no matching call"));
}

TEST_F(CoreIntrinsicTableTest, MatchArray) {
    auto* arr =
        create<type::Array>(create<type::U32>(), create<type::RuntimeArrayCount>(), 4u, 4u, 4u, 4u);
    auto* arr_ptr = create<type::Pointer>(AddressSpace::kStorage, arr, Access::kReadWrite);
    auto result =
        table.Lookup(BuiltinFn::kArrayLength, Empty, Vector{arr_ptr}, EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_TRUE(result->return_type->Is<type::U32>());
    ASSERT_EQ(result->parameters.Length(), 1u);
    auto* param_type = result->parameters[0].type;
    ASSERT_TRUE(param_type->Is<type::Pointer>());
    EXPECT_TRUE(param_type->As<type::Pointer>()->StoreType()->Is<type::Array>());
}

TEST_F(CoreIntrinsicTableTest, MismatchArray) {
    auto* f32 = create<type::F32>();
    auto result =
        table.Lookup(BuiltinFn::kArrayLength, Empty, Vector{f32}, EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    ASSERT_THAT(result.Failure().Plain(), HasSubstr("no matching call"));
}

TEST_F(CoreIntrinsicTableTest, MatchSampler) {
    auto* f32 = create<type::F32>();
    auto* vec2f = create<type::Vector>(f32, 2u);
    auto* vec4f = create<type::Vector>(f32, 4u);
    auto* tex = create<type::SampledTexture>(type::TextureDimension::k2d, f32);
    auto* sampler = create<type::Sampler>(type::SamplerKind::kSampler);
    auto result = table.Lookup(BuiltinFn::kTextureSample, Empty, Vector{tex, sampler, vec2f},
                               EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, vec4f);
    ASSERT_EQ(result->parameters.Length(), 3u);
    EXPECT_EQ(result->parameters[0].type, tex);
    EXPECT_EQ(result->parameters[0].usage, ParameterUsage::kTexture);
    EXPECT_EQ(result->parameters[1].type, sampler);
    EXPECT_EQ(result->parameters[1].usage, ParameterUsage::kSampler);
    EXPECT_EQ(result->parameters[2].type, vec2f);
    EXPECT_EQ(result->parameters[2].usage, ParameterUsage::kCoords);
}

TEST_F(CoreIntrinsicTableTest, MismatchSampler) {
    auto* f32 = create<type::F32>();
    auto* vec2f = create<type::Vector>(f32, 2u);
    auto* tex = create<type::SampledTexture>(type::TextureDimension::k2d, f32);
    auto result = table.Lookup(BuiltinFn::kTextureSample, Empty, Vector{tex, f32, vec2f},
                               EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    ASSERT_THAT(result.Failure().Plain(), HasSubstr("no matching call"));
}

TEST_F(CoreIntrinsicTableTest, MatchSampledTexture) {
    auto* i32 = create<type::I32>();
    auto* f32 = create<type::F32>();
    auto* vec2i = create<type::Vector>(i32, 2u);
    auto* vec4f = create<type::Vector>(f32, 4u);
    auto* tex = create<type::SampledTexture>(type::TextureDimension::k2d, f32);
    auto result = table.Lookup(BuiltinFn::kTextureLoad, Empty, Vector{tex, vec2i, i32},
                               EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, vec4f);
    ASSERT_EQ(result->parameters.Length(), 3u);
    EXPECT_EQ(result->parameters[0].type, tex);
    EXPECT_EQ(result->parameters[0].usage, ParameterUsage::kTexture);
    EXPECT_EQ(result->parameters[1].type, vec2i);
    EXPECT_EQ(result->parameters[1].usage, ParameterUsage::kCoords);
    EXPECT_EQ(result->parameters[2].type, i32);
    EXPECT_EQ(result->parameters[2].usage, ParameterUsage::kLevel);
}

TEST_F(CoreIntrinsicTableTest, MatchMultisampledTexture) {
    auto* i32 = create<type::I32>();
    auto* f32 = create<type::F32>();
    auto* vec2i = create<type::Vector>(i32, 2u);
    auto* vec4f = create<type::Vector>(f32, 4u);
    auto* tex = create<type::MultisampledTexture>(type::TextureDimension::k2d, f32);
    auto result = table.Lookup(BuiltinFn::kTextureLoad, Empty, Vector{tex, vec2i, i32},
                               EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, vec4f);
    ASSERT_EQ(result->parameters.Length(), 3u);
    EXPECT_EQ(result->parameters[0].type, tex);
    EXPECT_EQ(result->parameters[0].usage, ParameterUsage::kTexture);
    EXPECT_EQ(result->parameters[1].type, vec2i);
    EXPECT_EQ(result->parameters[1].usage, ParameterUsage::kCoords);
    EXPECT_EQ(result->parameters[2].type, i32);
    EXPECT_EQ(result->parameters[2].usage, ParameterUsage::kSampleIndex);
}

TEST_F(CoreIntrinsicTableTest, MatchDepthTexture) {
    auto* f32 = create<type::F32>();
    auto* i32 = create<type::I32>();
    auto* vec2i = create<type::Vector>(i32, 2u);
    auto* tex = create<type::DepthTexture>(type::TextureDimension::k2d);
    auto result = table.Lookup(BuiltinFn::kTextureLoad, Empty, Vector{tex, vec2i, i32},
                               EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, f32);
    ASSERT_EQ(result->parameters.Length(), 3u);
    EXPECT_EQ(result->parameters[0].type, tex);
    EXPECT_EQ(result->parameters[0].usage, ParameterUsage::kTexture);
    EXPECT_EQ(result->parameters[1].type, vec2i);
    EXPECT_EQ(result->parameters[1].usage, ParameterUsage::kCoords);
    EXPECT_EQ(result->parameters[2].type, i32);
    EXPECT_EQ(result->parameters[2].usage, ParameterUsage::kLevel);
}

TEST_F(CoreIntrinsicTableTest, MatchDepthMultisampledTexture) {
    auto* f32 = create<type::F32>();
    auto* i32 = create<type::I32>();
    auto* vec2i = create<type::Vector>(i32, 2u);
    auto* tex = create<type::DepthMultisampledTexture>(type::TextureDimension::k2d);
    auto result = table.Lookup(BuiltinFn::kTextureLoad, Empty, Vector{tex, vec2i, i32},
                               EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, f32);
    ASSERT_EQ(result->parameters.Length(), 3u);
    EXPECT_EQ(result->parameters[0].type, tex);
    EXPECT_EQ(result->parameters[0].usage, ParameterUsage::kTexture);
    EXPECT_EQ(result->parameters[1].type, vec2i);
    EXPECT_EQ(result->parameters[1].usage, ParameterUsage::kCoords);
    EXPECT_EQ(result->parameters[2].type, i32);
    EXPECT_EQ(result->parameters[2].usage, ParameterUsage::kSampleIndex);
}

TEST_F(CoreIntrinsicTableTest, MatchExternalTexture) {
    auto* f32 = create<type::F32>();
    auto* i32 = create<type::I32>();
    auto* vec2i = create<type::Vector>(i32, 2u);
    auto* vec4f = create<type::Vector>(f32, 4u);
    auto* tex = create<type::ExternalTexture>();
    auto result = table.Lookup(BuiltinFn::kTextureLoad, Empty, Vector{tex, vec2i},
                               EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, vec4f);
    ASSERT_EQ(result->parameters.Length(), 2u);
    EXPECT_EQ(result->parameters[0].type, tex);
    EXPECT_EQ(result->parameters[0].usage, ParameterUsage::kTexture);
    EXPECT_EQ(result->parameters[1].type, vec2i);
    EXPECT_EQ(result->parameters[1].usage, ParameterUsage::kCoords);
}

TEST_F(CoreIntrinsicTableTest, MatchWOStorageTexture) {
    auto* f32 = create<type::F32>();
    auto* i32 = create<type::I32>();
    auto* vec2i = create<type::Vector>(i32, 2u);
    auto* vec4f = create<type::Vector>(f32, 4u);
    auto* subtype = type::StorageTexture::SubtypeFor(TexelFormat::kR32Float, Types());
    auto* tex = create<type::StorageTexture>(type::TextureDimension::k2d, TexelFormat::kR32Float,
                                             Access::kWrite, subtype);

    auto result = table.Lookup(BuiltinFn::kTextureStore, Empty, Vector{tex, vec2i, vec4f},
                               EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_TRUE(result->return_type->Is<type::Void>());
    ASSERT_EQ(result->parameters.Length(), 3u);
    EXPECT_EQ(result->parameters[0].type, tex);
    EXPECT_EQ(result->parameters[0].usage, ParameterUsage::kTexture);
    EXPECT_EQ(result->parameters[1].type, vec2i);
    EXPECT_EQ(result->parameters[1].usage, ParameterUsage::kCoords);
    EXPECT_EQ(result->parameters[2].type, vec4f);
    EXPECT_EQ(result->parameters[2].usage, ParameterUsage::kValue);
}

TEST_F(CoreIntrinsicTableTest, MismatchTexture) {
    auto* f32 = create<type::F32>();
    auto* i32 = create<type::I32>();
    auto* vec2i = create<type::Vector>(i32, 2u);
    auto result = table.Lookup(BuiltinFn::kTextureLoad, Empty, Vector{f32, vec2i},
                               EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    ASSERT_THAT(result.Failure().Plain(), HasSubstr("no matching call"));
}

TEST_F(CoreIntrinsicTableTest, MatchTemplateType) {
    auto* f32 = create<type::F32>();
    auto result =
        table.Lookup(BuiltinFn::kClamp, Empty, Vector{f32, f32, f32}, EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, f32);
    EXPECT_EQ(result->parameters[0].type, f32);
    EXPECT_EQ(result->parameters[1].type, f32);
    EXPECT_EQ(result->parameters[2].type, f32);
}

TEST_F(CoreIntrinsicTableTest, MismatchTemplateType) {
    auto* f32 = create<type::F32>();
    auto* u32 = create<type::U32>();
    auto result =
        table.Lookup(BuiltinFn::kClamp, Empty, Vector{f32, u32, f32}, EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    ASSERT_THAT(result.Failure().Plain(), HasSubstr("no matching call"));
}

TEST_F(CoreIntrinsicTableTest, MatchOpenSizeVector) {
    auto* f32 = create<type::F32>();
    auto* vec2f = create<type::Vector>(f32, 2u);
    auto result = table.Lookup(BuiltinFn::kClamp, Empty, Vector{vec2f, vec2f, vec2f},
                               EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, vec2f);
    ASSERT_EQ(result->parameters.Length(), 3u);
    EXPECT_EQ(result->parameters[0].type, vec2f);
    EXPECT_EQ(result->parameters[1].type, vec2f);
    EXPECT_EQ(result->parameters[2].type, vec2f);
}

TEST_F(CoreIntrinsicTableTest, MismatchOpenSizeVector) {
    auto* f32 = create<type::F32>();
    auto* u32 = create<type::U32>();
    auto* vec2f = create<type::Vector>(f32, 2u);
    auto result = table.Lookup(BuiltinFn::kClamp, Empty, Vector{vec2f, u32, vec2f},
                               EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    ASSERT_THAT(result.Failure().Plain(), HasSubstr("no matching call"));
}

TEST_F(CoreIntrinsicTableTest, MatchOpenSizeMatrix) {
    auto* f32 = create<type::F32>();
    auto* vec3f = create<type::Vector>(f32, 3u);
    auto* mat3x3f = create<type::Matrix>(vec3f, 3u);
    auto result =
        table.Lookup(BuiltinFn::kDeterminant, Empty, Vector{mat3x3f}, EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, f32);
    ASSERT_EQ(result->parameters.Length(), 1u);
    EXPECT_EQ(result->parameters[0].type, mat3x3f);
}

TEST_F(CoreIntrinsicTableTest, MismatchOpenSizeMatrix) {
    auto* f32 = create<type::F32>();
    auto* vec2f = create<type::Vector>(f32, 2u);
    auto* mat3x2f = create<type::Matrix>(vec2f, 3u);
    auto result =
        table.Lookup(BuiltinFn::kDeterminant, Empty, Vector{mat3x2f}, EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    ASSERT_THAT(result.Failure().Plain(), HasSubstr("no matching call"));
}

TEST_F(CoreIntrinsicTableTest, MatchDifferentArgsElementType_Builtin_ConstantEval) {
    auto* f32 = create<type::F32>();
    auto* bool_ = create<type::Bool>();
    auto result = table.Lookup(BuiltinFn::kSelect, Empty, Vector{f32, f32, bool_},
                               EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_NE(result->const_eval_fn, nullptr);
    EXPECT_EQ(result->return_type, f32);
    ASSERT_EQ(result->parameters.Length(), 3u);
    EXPECT_EQ(result->parameters[0].type, f32);
    EXPECT_EQ(result->parameters[1].type, f32);
    EXPECT_EQ(result->parameters[2].type, bool_);
}

TEST_F(CoreIntrinsicTableTest, MatchDifferentArgsElementType_Builtin_RuntimeEval) {
    auto* f32 = create<type::F32>();
    auto result = table.Lookup(BuiltinFn::kSelect, Empty, Vector{f32, f32, create<type::Bool>()},
                               EvaluationStage::kRuntime);
    ASSERT_EQ(result, Success);
    EXPECT_NE(result->const_eval_fn, nullptr);
    EXPECT_TRUE(result->return_type->Is<type::F32>());
    ASSERT_EQ(result->parameters.Length(), 3u);
    EXPECT_TRUE(result->parameters[0].type->Is<type::F32>());
    EXPECT_TRUE(result->parameters[1].type->Is<type::F32>());
    EXPECT_TRUE(result->parameters[2].type->Is<type::Bool>());
}

TEST_F(CoreIntrinsicTableTest, MatchDifferentArgsElementType_Binary_ConstantEval) {
    auto* i32 = create<type::I32>();
    auto* u32 = create<type::U32>();
    auto result = table.Lookup(BinaryOp::kShiftLeft, i32, u32, EvaluationStage::kConstant, false);
    ASSERT_EQ(result, Success);
    ASSERT_NE(result->const_eval_fn, nullptr) << Diagnostics();
    EXPECT_EQ(result->return_type, i32);
    EXPECT_EQ(result->parameters[0].type, i32);
    EXPECT_EQ(result->parameters[1].type, u32);
}

TEST_F(CoreIntrinsicTableTest, MatchDifferentArgsElementType_Binary_RuntimeEval) {
    auto* i32 = create<type::I32>();
    auto* u32 = create<type::U32>();
    auto result = table.Lookup(BinaryOp::kShiftLeft, i32, u32, EvaluationStage::kRuntime, false);
    ASSERT_EQ(result, Success);
    ASSERT_NE(result->const_eval_fn, nullptr) << Diagnostics();
    EXPECT_TRUE(result->return_type->Is<type::I32>());
    EXPECT_TRUE(result->parameters[0].type->Is<type::I32>());
    EXPECT_TRUE(result->parameters[1].type->Is<type::U32>());
}

TEST_F(CoreIntrinsicTableTest, OverloadOrderByNumberOfParameters) {
    // None of the arguments match, so expect the overloads with 2 parameters to
    // come first
    auto* bool_ = create<type::Bool>();
    auto result = table.Lookup(BuiltinFn::kTextureDimensions, Empty, Vector{bool_, bool_},
                               EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    ASSERT_EQ(result.Failure().Plain(),
              R"(no matching call to 'textureDimensions(bool, bool)'

27 candidate functions:
 • 'textureDimensions(texture: texture_depth_2d  ✗ , level: L  ✗ ) -> vec2<u32>' where:
      ✗  'L' is 'i32' or 'u32'
 • 'textureDimensions(texture: texture_depth_2d_array  ✗ , level: L  ✗ ) -> vec2<u32>' where:
      ✗  'L' is 'i32' or 'u32'
 • 'textureDimensions(texture: texture_depth_cube  ✗ , level: L  ✗ ) -> vec2<u32>' where:
      ✗  'L' is 'i32' or 'u32'
 • 'textureDimensions(texture: texture_depth_cube_array  ✗ , level: L  ✗ ) -> vec2<u32>' where:
      ✗  'L' is 'i32' or 'u32'
 • 'textureDimensions(texture: texture_1d<T>  ✗ , level: L  ✗ ) -> u32' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
      ✗  'L' is 'i32' or 'u32'
 • 'textureDimensions(texture: texture_2d<T>  ✗ , level: L  ✗ ) -> vec2<u32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
      ✗  'L' is 'i32' or 'u32'
 • 'textureDimensions(texture: texture_2d_array<T>  ✗ , level: L  ✗ ) -> vec2<u32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
      ✗  'L' is 'i32' or 'u32'
 • 'textureDimensions(texture: texture_3d<T>  ✗ , level: L  ✗ ) -> vec3<u32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
      ✗  'L' is 'i32' or 'u32'
 • 'textureDimensions(texture: texture_cube<T>  ✗ , level: L  ✗ ) -> vec2<u32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
      ✗  'L' is 'i32' or 'u32'
 • 'textureDimensions(texture: texture_cube_array<T>  ✗ , level: L  ✗ ) -> vec2<u32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
      ✗  'L' is 'i32' or 'u32'
 • 'textureDimensions(texture: texture_depth_2d  ✗ ) -> vec2<u32>'
 • 'textureDimensions(texture: texture_depth_2d_array  ✗ ) -> vec2<u32>'
 • 'textureDimensions(texture: texture_depth_cube  ✗ ) -> vec2<u32>'
 • 'textureDimensions(texture: texture_depth_cube_array  ✗ ) -> vec2<u32>'
 • 'textureDimensions(texture: texture_depth_multisampled_2d  ✗ ) -> vec2<u32>'
 • 'textureDimensions(texture: texture_storage_1d<F, A>  ✗ ) -> u32'
 • 'textureDimensions(texture: texture_storage_2d<F, A>  ✗ ) -> vec2<u32>'
 • 'textureDimensions(texture: texture_storage_2d_array<F, A>  ✗ ) -> vec2<u32>'
 • 'textureDimensions(texture: texture_storage_3d<F, A>  ✗ ) -> vec3<u32>'
 • 'textureDimensions(texture: texture_external  ✗ ) -> vec2<u32>'
 • 'textureDimensions(texture: texture_1d<T>  ✗ ) -> u32' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'textureDimensions(texture: texture_2d<T>  ✗ ) -> vec2<u32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'textureDimensions(texture: texture_2d_array<T>  ✗ ) -> vec2<u32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'textureDimensions(texture: texture_3d<T>  ✗ ) -> vec3<u32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'textureDimensions(texture: texture_cube<T>  ✗ ) -> vec2<u32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'textureDimensions(texture: texture_cube_array<T>  ✗ ) -> vec2<u32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'textureDimensions(texture: texture_multisampled_2d<T>  ✗ ) -> vec2<u32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
)");
}

TEST_F(CoreIntrinsicTableTest, OverloadOrderByMatchingParameter) {
    auto* tex = create<type::DepthTexture>(type::TextureDimension::k2d);
    auto* bool_ = create<type::Bool>();
    auto result = table.Lookup(BuiltinFn::kTextureDimensions, Empty, Vector{tex, bool_},
                               EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    ASSERT_EQ(result.Failure().Plain(),
              R"(no matching call to 'textureDimensions(texture_depth_2d, bool)'

27 candidate functions:
 • 'textureDimensions(texture: texture_depth_2d  ✓ , level: L  ✗ ) -> vec2<u32>' where:
      ✗  'L' is 'i32' or 'u32'
 • 'textureDimensions(texture: texture_depth_2d  ✓ ) -> vec2<u32>' where:
      ✗  overload expects 1 argument, call passed 2 arguments
 • 'textureDimensions(texture: texture_depth_2d_array  ✗ , level: L  ✗ ) -> vec2<u32>' where:
      ✗  'L' is 'i32' or 'u32'
 • 'textureDimensions(texture: texture_depth_cube  ✗ , level: L  ✗ ) -> vec2<u32>' where:
      ✗  'L' is 'i32' or 'u32'
 • 'textureDimensions(texture: texture_depth_cube_array  ✗ , level: L  ✗ ) -> vec2<u32>' where:
      ✗  'L' is 'i32' or 'u32'
 • 'textureDimensions(texture: texture_1d<T>  ✗ , level: L  ✗ ) -> u32' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
      ✗  'L' is 'i32' or 'u32'
 • 'textureDimensions(texture: texture_2d<T>  ✗ , level: L  ✗ ) -> vec2<u32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
      ✗  'L' is 'i32' or 'u32'
 • 'textureDimensions(texture: texture_2d_array<T>  ✗ , level: L  ✗ ) -> vec2<u32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
      ✗  'L' is 'i32' or 'u32'
 • 'textureDimensions(texture: texture_3d<T>  ✗ , level: L  ✗ ) -> vec3<u32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
      ✗  'L' is 'i32' or 'u32'
 • 'textureDimensions(texture: texture_cube<T>  ✗ , level: L  ✗ ) -> vec2<u32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
      ✗  'L' is 'i32' or 'u32'
 • 'textureDimensions(texture: texture_cube_array<T>  ✗ , level: L  ✗ ) -> vec2<u32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
      ✗  'L' is 'i32' or 'u32'
 • 'textureDimensions(texture: texture_depth_2d_array  ✗ ) -> vec2<u32>'
 • 'textureDimensions(texture: texture_depth_cube  ✗ ) -> vec2<u32>'
 • 'textureDimensions(texture: texture_depth_cube_array  ✗ ) -> vec2<u32>'
 • 'textureDimensions(texture: texture_depth_multisampled_2d  ✗ ) -> vec2<u32>'
 • 'textureDimensions(texture: texture_storage_1d<F, A>  ✗ ) -> u32'
 • 'textureDimensions(texture: texture_storage_2d<F, A>  ✗ ) -> vec2<u32>'
 • 'textureDimensions(texture: texture_storage_2d_array<F, A>  ✗ ) -> vec2<u32>'
 • 'textureDimensions(texture: texture_storage_3d<F, A>  ✗ ) -> vec3<u32>'
 • 'textureDimensions(texture: texture_external  ✗ ) -> vec2<u32>'
 • 'textureDimensions(texture: texture_1d<T>  ✗ ) -> u32' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'textureDimensions(texture: texture_2d<T>  ✗ ) -> vec2<u32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'textureDimensions(texture: texture_2d_array<T>  ✗ ) -> vec2<u32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'textureDimensions(texture: texture_3d<T>  ✗ ) -> vec3<u32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'textureDimensions(texture: texture_cube<T>  ✗ ) -> vec2<u32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'textureDimensions(texture: texture_cube_array<T>  ✗ ) -> vec2<u32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'textureDimensions(texture: texture_multisampled_2d<T>  ✗ ) -> vec2<u32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
)");
}

TEST_F(CoreIntrinsicTableTest, MatchUnaryOp) {
    auto* i32 = create<type::I32>();
    auto* vec3i = create<type::Vector>(i32, 3u);
    auto result = table.Lookup(UnaryOp::kNegation, vec3i, EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, vec3i);
}

TEST_F(CoreIntrinsicTableTest, MismatchUnaryOp) {
    auto* bool_ = create<type::Bool>();
    auto result = table.Lookup(UnaryOp::kNegation, bool_, EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().Plain(), R"(no matching overload for 'operator - (bool)'

2 candidate operators:
 • 'operator - (T  ✗ ) -> T' where:
      ✗  'T' is 'f32', 'i32' or 'f16'
 • 'operator - (vecN<T>  ✗ ) -> vecN<T>' where:
      ✗  'T' is 'f32', 'i32' or 'f16'
)");
}

TEST_F(CoreIntrinsicTableTest, MatchUnaryOp_Constant) {
    auto* i32 = create<type::I32>();
    auto result = table.Lookup(UnaryOp::kNegation, i32, EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, i32);
}

TEST_F(CoreIntrinsicTableTest, MatchUnaryOp_Runtime) {
    auto* i32 = create<type::I32>();
    auto result = table.Lookup(UnaryOp::kNegation, i32, EvaluationStage::kRuntime);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, i32);
}

TEST_F(CoreIntrinsicTableTest, MatchBinaryOp) {
    auto* i32 = create<type::I32>();
    auto* vec3i = create<type::Vector>(i32, 3u);
    auto result = table.Lookup(BinaryOp::kMultiply, i32, vec3i, EvaluationStage::kConstant,
                               /* is_compound */ false);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, vec3i);
    EXPECT_EQ(result->parameters[0].type, i32);
    EXPECT_EQ(result->parameters[1].type, vec3i);
}

TEST_F(CoreIntrinsicTableTest, MismatchBinaryOp) {
    auto* f32 = create<type::F32>();
    auto* bool_ = create<type::Bool>();
    auto result = table.Lookup(BinaryOp::kMultiply, f32, bool_, EvaluationStage::kConstant,
                               /* is_compound */ false);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().Plain(), R"(no matching overload for 'operator * (f32, bool)'

9 candidate operators:
 • 'operator * (T  ✓ , T  ✗ ) -> T' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (T  ✓ , vecN<T>  ✗ ) -> vecN<T>' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (T  ✓ , matNxM<T>  ✗ ) -> matNxM<T>' where:
      ✓  'T' is 'f32' or 'f16'
 • 'operator * (vecN<T>  ✗ , T  ✗ ) -> vecN<T>' where:
      ✗  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (matNxM<T>  ✗ , T  ✗ ) -> matNxM<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (vecN<T>  ✗ , vecN<T>  ✗ ) -> vecN<T>' where:
      ✗  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (matCxR<T>  ✗ , vecC<T>  ✗ ) -> vecR<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (vecR<T>  ✗ , matCxR<T>  ✗ ) -> vecC<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (matKxR<T>  ✗ , matCxK<T>  ✗ ) -> matCxR<T>' where:
      ✗  'T' is 'f32' or 'f16'
)");
}

TEST_F(CoreIntrinsicTableTest, MatchCompoundOp) {
    auto* i32 = create<type::I32>();
    auto* vec3i = create<type::Vector>(i32, 3u);
    auto result = table.Lookup(BinaryOp::kMultiply, i32, vec3i, EvaluationStage::kConstant,
                               /* is_compound */ true);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, vec3i);
    EXPECT_EQ(result->parameters[0].type, i32);
    EXPECT_EQ(result->parameters[1].type, vec3i);
}

TEST_F(CoreIntrinsicTableTest, MismatchCompoundOp) {
    auto* f32 = create<type::F32>();
    auto* bool_ = create<type::Bool>();
    auto result = table.Lookup(BinaryOp::kMultiply, f32, bool_, EvaluationStage::kConstant,
                               /* is_compound */ true);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().Plain(), R"(no matching overload for 'operator *= (f32, bool)'

9 candidate operators:
 • 'operator *= (T  ✓ , T  ✗ ) -> T' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator *= (T  ✓ , vecN<T>  ✗ ) -> vecN<T>' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator *= (T  ✓ , matNxM<T>  ✗ ) -> matNxM<T>' where:
      ✓  'T' is 'f32' or 'f16'
 • 'operator *= (vecN<T>  ✗ , T  ✗ ) -> vecN<T>' where:
      ✗  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator *= (matNxM<T>  ✗ , T  ✗ ) -> matNxM<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator *= (vecN<T>  ✗ , vecN<T>  ✗ ) -> vecN<T>' where:
      ✗  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator *= (matCxR<T>  ✗ , vecC<T>  ✗ ) -> vecR<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator *= (vecR<T>  ✗ , matCxR<T>  ✗ ) -> vecC<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator *= (matKxR<T>  ✗ , matCxK<T>  ✗ ) -> matCxR<T>' where:
      ✗  'T' is 'f32' or 'f16'
)");
}

TEST_F(CoreIntrinsicTableTest, MatchTypeInitializer) {
    auto* i32 = create<type::I32>();
    auto* vec3i = create<type::Vector>(i32, 3u);
    auto result = table.Lookup(CtorConv::kVec3, Vector{i32}, Vector{i32, i32, i32},
                               EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, vec3i);
    EXPECT_TRUE(result->info->flags.Contains(OverloadFlag::kIsConstructor));
    ASSERT_EQ(result->parameters.Length(), 3u);
    EXPECT_EQ(result->parameters[0].type, i32);
    EXPECT_EQ(result->parameters[1].type, i32);
    EXPECT_EQ(result->parameters[2].type, i32);
    EXPECT_NE(result->const_eval_fn, nullptr);
}

TEST_F(CoreIntrinsicTableTest, MismatchTypeInitializer) {
    auto* i32 = create<type::I32>();
    auto* f32 = create<type::F32>();
    auto result = table.Lookup(CtorConv::kVec3, Vector{i32}, Vector{i32, f32, i32},
                               EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().Plain(),
              R"(no matching constructor for 'vec3<i32>(i32, f32, i32)'

6 candidate constructors:
 • 'vec3<T  ✓ >(x: T  ✓ , y: T  ✗ , z: T  ✓ ) -> vec3<T>' where:
      ✓  'T' is 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec3<T  ✓ >(x: T  ✓ , yz: vec2<T>  ✗ ) -> vec3<T>' where:
      ✓  'T' is 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec3<T  ✓ >(T  ✓ ) -> vec3<T>' where:
      ✗  overload expects 1 argument, call passed 3 arguments
      ✓  'T' is 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec3<T  ✓ >(xy: vec2<T>  ✗ , z: T  ✗ ) -> vec3<T>' where:
      ✓  'T' is 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec3<T  ✓ >(vec3<T>  ✗ ) -> vec3<T>' where:
      ✓  'T' is 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec3<T  ✓ >() -> vec3<T>' where:
      ✗  overload expects 0 arguments, call passed 3 arguments
      ✓  'T' is 'f32', 'f16', 'i32', 'u32' or 'bool'

5 candidate conversions:
 • 'vec3<T  ✓ >(vec3<U>  ✗ ) -> vec3<T>' where:
      ✓  'T' is 'i32'
      ✗  'U' is 'f32', 'f16', 'u32' or 'bool'
 • 'vec3<T  ✗ >(vec3<U>  ✗ ) -> vec3<T>' where:
      ✗  'T' is 'f32'
      ✗  'U' is 'i32', 'f16', 'u32' or 'bool'
 • 'vec3<T  ✗ >(vec3<U>  ✗ ) -> vec3<T>' where:
      ✗  'T' is 'f16'
      ✗  'U' is 'f32', 'i32', 'u32' or 'bool'
 • 'vec3<T  ✗ >(vec3<U>  ✗ ) -> vec3<T>' where:
      ✗  'T' is 'u32'
      ✗  'U' is 'f32', 'f16', 'i32' or 'bool'
 • 'vec3<T  ✗ >(vec3<U>  ✗ ) -> vec3<T>' where:
      ✗  'T' is 'bool'
      ✗  'U' is 'f32', 'f16', 'i32' or 'u32'
)");
}

TEST_F(CoreIntrinsicTableTest, MatchTypeInitializer_ConstantEval) {
    auto* i32 = create<type::I32>();
    auto* vec3i = create<type::Vector>(i32, 3u);
    auto result = table.Lookup(CtorConv::kVec3, Vector{i32}, Vector{i32, i32, i32},
                               EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_NE(result->const_eval_fn, nullptr);
    EXPECT_EQ(result->return_type, vec3i);
    EXPECT_TRUE(result->info->flags.Contains(OverloadFlag::kIsConstructor));
    ASSERT_EQ(result->parameters.Length(), 3u);
    EXPECT_EQ(result->parameters[0].type, i32);
    EXPECT_EQ(result->parameters[1].type, i32);
    EXPECT_EQ(result->parameters[2].type, i32);
    EXPECT_NE(result->const_eval_fn, nullptr);
}

TEST_F(CoreIntrinsicTableTest, MatchTypeInitializer_RuntimeEval) {
    auto* i32 = create<type::I32>();
    auto result = table.Lookup(CtorConv::kVec3, Vector{i32}, Vector{i32, i32, i32},
                               EvaluationStage::kRuntime);
    auto* vec3i = create<type::Vector>(i32, 3u);
    ASSERT_EQ(result, Success);
    EXPECT_NE(result->const_eval_fn, nullptr);
    EXPECT_EQ(result->return_type, vec3i);
    EXPECT_TRUE(result->info->flags.Contains(OverloadFlag::kIsConstructor));
    ASSERT_EQ(result->parameters.Length(), 3u);
    EXPECT_EQ(result->parameters[0].type, i32);
    EXPECT_EQ(result->parameters[1].type, i32);
    EXPECT_EQ(result->parameters[2].type, i32);
    EXPECT_NE(result->const_eval_fn, nullptr);
}

TEST_F(CoreIntrinsicTableTest, MatchTypeConversion) {
    auto* i32 = create<type::I32>();
    auto* vec3i = create<type::Vector>(i32, 3u);
    auto* f32 = create<type::F32>();
    auto* vec3f = create<type::Vector>(f32, 3u);
    auto result =
        table.Lookup(CtorConv::kVec3, Vector{i32}, Vector{vec3f}, EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, vec3i);
    EXPECT_FALSE(result->info->flags.Contains(OverloadFlag::kIsConstructor));
    ASSERT_EQ(result->parameters.Length(), 1u);
    EXPECT_EQ(result->parameters[0].type, vec3f);
}

TEST_F(CoreIntrinsicTableTest, MismatchTypeConversion) {
    auto* arr =
        create<type::Array>(create<type::U32>(), create<type::RuntimeArrayCount>(), 4u, 4u, 4u, 4u);
    auto* f32 = create<type::F32>();
    auto result =
        table.Lookup(CtorConv::kVec3, Vector{f32}, Vector{arr}, EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().Plain(),
              R"(no matching constructor for 'vec3<f32>(array<u32>)'

6 candidate constructors:
 • 'vec3<T  ✓ >(vec3<T>  ✗ ) -> vec3<T>' where:
      ✓  'T' is 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec3<T  ✓ >(T  ✗ ) -> vec3<T>' where:
      ✓  'T' is 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec3<T  ✓ >() -> vec3<T>' where:
      ✗  overload expects 0 arguments, call passed 1 argument
      ✓  'T' is 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec3<T  ✓ >(xy: vec2<T>  ✗ , z: T  ✗ ) -> vec3<T>' where:
      ✓  'T' is 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec3<T  ✓ >(x: T  ✗ , yz: vec2<T>  ✗ ) -> vec3<T>' where:
      ✓  'T' is 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec3<T  ✓ >(x: T  ✗ , y: T  ✗ , z: T  ✗ ) -> vec3<T>' where:
      ✓  'T' is 'f32', 'f16', 'i32', 'u32' or 'bool'

5 candidate conversions:
 • 'vec3<T  ✓ >(vec3<U>  ✗ ) -> vec3<T>' where:
      ✓  'T' is 'f32'
      ✗  'U' is 'i32', 'f16', 'u32' or 'bool'
 • 'vec3<T  ✗ >(vec3<U>  ✗ ) -> vec3<T>' where:
      ✗  'T' is 'f16'
      ✗  'U' is 'f32', 'i32', 'u32' or 'bool'
 • 'vec3<T  ✗ >(vec3<U>  ✗ ) -> vec3<T>' where:
      ✗  'T' is 'i32'
      ✗  'U' is 'f32', 'f16', 'u32' or 'bool'
 • 'vec3<T  ✗ >(vec3<U>  ✗ ) -> vec3<T>' where:
      ✗  'T' is 'u32'
      ✗  'U' is 'f32', 'f16', 'i32' or 'bool'
 • 'vec3<T  ✗ >(vec3<U>  ✗ ) -> vec3<T>' where:
      ✗  'T' is 'bool'
      ✗  'U' is 'f32', 'f16', 'i32' or 'u32'
)");
}

TEST_F(CoreIntrinsicTableTest, MatchTypeConversion_ConstantEval) {
    auto* i32 = create<type::I32>();
    auto* f32 = create<type::F32>();
    auto* vec3i = create<type::Vector>(i32, 3u);
    auto* vec3f = create<type::Vector>(f32, 3u);
    auto result =
        table.Lookup(CtorConv::kVec3, Vector{f32}, Vector{vec3i}, EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_NE(result->const_eval_fn, nullptr);
    // NOTE: Conversions are explicit, so there's no way to have it return abstracts
    EXPECT_EQ(result->return_type, vec3f);
    EXPECT_FALSE(result->info->flags.Contains(OverloadFlag::kIsConstructor));
    ASSERT_EQ(result->parameters.Length(), 1u);
    EXPECT_EQ(result->parameters[0].type, vec3i);
}

TEST_F(CoreIntrinsicTableTest, MatchTypeConversion_RuntimeEval) {
    auto* i32 = create<type::I32>();
    auto* f32 = create<type::F32>();
    auto* vec3i = create<type::Vector>(i32, 3u);
    auto* vec3f = create<type::Vector>(create<type::F32>(), 3u);
    auto result =
        table.Lookup(CtorConv::kVec3, Vector{f32}, Vector{vec3i}, EvaluationStage::kRuntime);
    ASSERT_EQ(result, Success);
    EXPECT_NE(result->const_eval_fn, nullptr);
    EXPECT_EQ(result->return_type, vec3f);
    EXPECT_FALSE(result->info->flags.Contains(OverloadFlag::kIsConstructor));
    ASSERT_EQ(result->parameters.Length(), 1u);
    EXPECT_EQ(result->parameters[0].type, vec3i);
}

TEST_F(CoreIntrinsicTableTest, Err257Arguments) {  // crbug.com/1323605
    auto* f32 = create<type::F32>();
    Vector<const type::Type*, 0> arg_tys;
    arg_tys.Resize(257, f32);
    auto result =
        table.Lookup(BuiltinFn::kAbs, Empty, std::move(arg_tys), EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    ASSERT_THAT(result.Failure().Plain(), HasSubstr("no matching call"));
}

TEST_F(CoreIntrinsicTableTest, MemberFunctionDoesNotMatchNonMemberFunction) {
    auto* arr =
        create<type::Array>(create<type::F32>(), create<type::RuntimeArrayCount>(), 4u, 4u, 4u, 4u);
    auto result =
        table.Lookup(BuiltinFn::kArrayLength, arr, Empty, Empty, EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().Plain(), R"(no matching call to 'arrayLength(array<f32>)'
)");
}

}  // namespace
}  // namespace tint::core::intrinsic

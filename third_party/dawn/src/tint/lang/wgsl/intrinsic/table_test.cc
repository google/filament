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

#include "src/tint/lang/core/intrinsic/table.h"

#include <utility>

#include "gmock/gmock.h"
#include "src/tint/lang/core/intrinsic/table_data.h"
#include "src/tint/lang/core/type/atomic.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/external_texture.h"
#include "src/tint/lang/core/type/helper_test.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/wgsl/intrinsic/dialect.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

namespace tint::wgsl::intrinsic {
namespace {

using ::testing::HasSubstr;
using namespace tint::core::fluent_types;  // NOLINT

using ParameterUsage = core::ParameterUsage;
using OverloadFlag = core::intrinsic::OverloadFlag;

using AFloatV = vec3<AFloat>;
using AIntV = vec3<AInt>;
using f32V = vec3<f32>;
using i32V = vec3<i32>;
using u32V = vec3<u32>;

class WgslIntrinsicTableTest : public testing::Test, public ProgramBuilder {
  public:
    core::intrinsic::Table<Dialect> table{Types(), Symbols()};
};

TEST_F(WgslIntrinsicTableTest, MatchF32) {
    auto* f32 = create<core::type::F32>();
    auto result =
        table.Lookup(wgsl::BuiltinFn::kCos, Empty, Vector{f32}, core::EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, f32);
    ASSERT_EQ(result->parameters.Length(), 1u);
    EXPECT_EQ(result->parameters[0].type, f32);
}

TEST_F(WgslIntrinsicTableTest, MismatchF32) {
    auto* i32 = create<core::type::I32>();
    auto result =
        table.Lookup(wgsl::BuiltinFn::kCos, Empty, Vector{i32}, core::EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    ASSERT_THAT(result.Failure().Plain(), HasSubstr("no matching call"));
}

TEST_F(WgslIntrinsicTableTest, MatchU32) {
    auto* f32 = create<core::type::F32>();
    auto* u32 = create<core::type::U32>();
    auto* vec2f = create<core::type::Vector>(f32, 2u);
    auto result = table.Lookup(wgsl::BuiltinFn::kUnpack2X16Float, Empty, Vector{u32},
                               core::EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, vec2f);
    ASSERT_EQ(result->parameters.Length(), 1u);
    EXPECT_EQ(result->parameters[0].type, u32);
}

TEST_F(WgslIntrinsicTableTest, MismatchU32) {
    auto* f32 = create<core::type::F32>();
    auto result = table.Lookup(wgsl::BuiltinFn::kUnpack2X16Float, Empty, Vector{f32},
                               core::EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    ASSERT_THAT(result.Failure().Plain(), HasSubstr("no matching call"));
}

TEST_F(WgslIntrinsicTableTest, MatchI32) {
    auto* f32 = create<core::type::F32>();
    auto* i32 = create<core::type::I32>();
    auto* vec4f = create<core::type::Vector>(f32, 4u);
    auto* tex = create<core::type::SampledTexture>(core::type::TextureDimension::k1d, f32);
    auto result = table.Lookup(wgsl::BuiltinFn::kTextureLoad, Empty, Vector{tex, i32, i32},
                               core::EvaluationStage::kConstant);
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

TEST_F(WgslIntrinsicTableTest, MismatchI32) {
    auto* f32 = create<core::type::F32>();
    auto* tex = create<core::type::SampledTexture>(core::type::TextureDimension::k1d, f32);
    auto result = table.Lookup(wgsl::BuiltinFn::kTextureLoad, Empty, Vector{tex, f32},
                               core::EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    ASSERT_THAT(result.Failure().Plain(), HasSubstr("no matching call"));
}

TEST_F(WgslIntrinsicTableTest, MatchIU32AsI32) {
    auto* i32 = create<core::type::I32>();
    auto result = table.Lookup(wgsl::BuiltinFn::kCountOneBits, Empty, Vector{i32},
                               core::EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, i32);
    ASSERT_EQ(result->parameters.Length(), 1u);
    EXPECT_EQ(result->parameters[0].type, i32);
}

TEST_F(WgslIntrinsicTableTest, MatchIU32AsU32) {
    auto* u32 = create<core::type::U32>();
    auto result = table.Lookup(wgsl::BuiltinFn::kCountOneBits, Empty, Vector{u32},
                               core::EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, u32);
    ASSERT_EQ(result->parameters.Length(), 1u);
    EXPECT_EQ(result->parameters[0].type, u32);
}

TEST_F(WgslIntrinsicTableTest, MismatchIU32) {
    auto* f32 = create<core::type::F32>();
    auto result = table.Lookup(wgsl::BuiltinFn::kCountOneBits, Empty, Vector{f32},
                               core::EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    ASSERT_THAT(result.Failure().Plain(), HasSubstr("no matching call"));
}

TEST_F(WgslIntrinsicTableTest, MatchFIU32AsI32) {
    auto* i32 = create<core::type::I32>();
    auto result = table.Lookup(wgsl::BuiltinFn::kClamp, Empty, Vector{i32, i32, i32},
                               core::EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, i32);
    ASSERT_EQ(result->parameters.Length(), 3u);
    EXPECT_EQ(result->parameters[0].type, i32);
    EXPECT_EQ(result->parameters[1].type, i32);
    EXPECT_EQ(result->parameters[2].type, i32);
}

TEST_F(WgslIntrinsicTableTest, MatchFIU32AsU32) {
    auto* u32 = create<core::type::U32>();
    auto result = table.Lookup(wgsl::BuiltinFn::kClamp, Empty, Vector{u32, u32, u32},
                               core::EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, u32);
    ASSERT_EQ(result->parameters.Length(), 3u);
    EXPECT_EQ(result->parameters[0].type, u32);
    EXPECT_EQ(result->parameters[1].type, u32);
    EXPECT_EQ(result->parameters[2].type, u32);
}

TEST_F(WgslIntrinsicTableTest, MatchFIU32AsF32) {
    auto* f32 = create<core::type::F32>();
    auto result = table.Lookup(wgsl::BuiltinFn::kClamp, Empty, Vector{f32, f32, f32},
                               core::EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, f32);
    ASSERT_EQ(result->parameters.Length(), 3u);
    EXPECT_EQ(result->parameters[0].type, f32);
    EXPECT_EQ(result->parameters[1].type, f32);
    EXPECT_EQ(result->parameters[2].type, f32);
}

TEST_F(WgslIntrinsicTableTest, MismatchFIU32) {
    auto* bool_ = create<core::type::Bool>();
    auto result = table.Lookup(wgsl::BuiltinFn::kClamp, Empty, Vector{bool_, bool_, bool_},
                               core::EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    ASSERT_THAT(result.Failure().Plain(), HasSubstr("no matching call"));
}

TEST_F(WgslIntrinsicTableTest, MatchBool) {
    auto* f32 = create<core::type::F32>();
    auto* bool_ = create<core::type::Bool>();
    auto result = table.Lookup(wgsl::BuiltinFn::kSelect, Empty, Vector{f32, f32, bool_},
                               core::EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, f32);
    ASSERT_EQ(result->parameters.Length(), 3u);
    EXPECT_EQ(result->parameters[0].type, f32);
    EXPECT_EQ(result->parameters[1].type, f32);
    EXPECT_EQ(result->parameters[2].type, bool_);
}

TEST_F(WgslIntrinsicTableTest, MismatchBool) {
    auto* f32 = create<core::type::F32>();
    auto result = table.Lookup(wgsl::BuiltinFn::kSelect, Empty, Vector{f32, f32, f32},
                               core::EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    ASSERT_THAT(result.Failure().Plain(), HasSubstr("no matching call"));
}

TEST_F(WgslIntrinsicTableTest, MatchPointer) {
    auto* i32 = create<core::type::I32>();
    auto* atomic_i32 = create<core::type::Atomic>(i32);
    auto* ptr = create<core::type::Pointer>(core::AddressSpace::kWorkgroup, atomic_i32,
                                            core::Access::kReadWrite);
    auto result = table.Lookup(wgsl::BuiltinFn::kAtomicLoad, Empty, Vector{ptr},
                               core::EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, i32);
    ASSERT_EQ(result->parameters.Length(), 1u);
    EXPECT_EQ(result->parameters[0].type, ptr);
}

TEST_F(WgslIntrinsicTableTest, MismatchPointer) {
    auto* i32 = create<core::type::I32>();
    auto* atomic_i32 = create<core::type::Atomic>(i32);
    auto result = table.Lookup(wgsl::BuiltinFn::kAtomicLoad, Empty, Vector{atomic_i32},
                               core::EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    ASSERT_THAT(result.Failure().Plain(), HasSubstr("no matching call"));
}

TEST_F(WgslIntrinsicTableTest, MatchArray) {
    auto* arr = create<core::type::Array>(create<core::type::U32>(),
                                          create<core::type::RuntimeArrayCount>(), 4u, 4u, 4u, 4u);
    auto* arr_ptr =
        create<core::type::Pointer>(core::AddressSpace::kStorage, arr, core::Access::kReadWrite);
    auto result = table.Lookup(wgsl::BuiltinFn::kArrayLength, Empty, Vector{arr_ptr},
                               core::EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_TRUE(result->return_type->Is<core::type::U32>());
    ASSERT_EQ(result->parameters.Length(), 1u);
    auto* param_type = result->parameters[0].type;
    ASSERT_TRUE(param_type->Is<core::type::Pointer>());
    EXPECT_TRUE(param_type->As<core::type::Pointer>()->StoreType()->Is<core::type::Array>());
}

TEST_F(WgslIntrinsicTableTest, MismatchArray) {
    auto* f32 = create<core::type::F32>();
    auto result = table.Lookup(wgsl::BuiltinFn::kArrayLength, Empty, Vector{f32},
                               core::EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    ASSERT_THAT(result.Failure().Plain(), HasSubstr("no matching call"));
}

TEST_F(WgslIntrinsicTableTest, MatchSampler) {
    auto* f32 = create<core::type::F32>();
    auto* vec2f = create<core::type::Vector>(f32, 2u);
    auto* vec4f = create<core::type::Vector>(f32, 4u);
    auto* tex = create<core::type::SampledTexture>(core::type::TextureDimension::k2d, f32);
    auto* sampler = create<core::type::Sampler>(core::type::SamplerKind::kSampler);
    auto result = table.Lookup(wgsl::BuiltinFn::kTextureSample, Empty, Vector{tex, sampler, vec2f},
                               core::EvaluationStage::kConstant);
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

TEST_F(WgslIntrinsicTableTest, MismatchSampler) {
    auto* f32 = create<core::type::F32>();
    auto* vec2f = create<core::type::Vector>(f32, 2u);
    auto* tex = create<core::type::SampledTexture>(core::type::TextureDimension::k2d, f32);
    auto result = table.Lookup(wgsl::BuiltinFn::kTextureSample, Empty, Vector{tex, f32, vec2f},
                               core::EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    ASSERT_THAT(result.Failure().Plain(), HasSubstr("no matching call"));
}

TEST_F(WgslIntrinsicTableTest, MatchSampledTexture) {
    auto* i32 = create<core::type::I32>();
    auto* f32 = create<core::type::F32>();
    auto* vec2i = create<core::type::Vector>(i32, 2u);
    auto* vec4f = create<core::type::Vector>(f32, 4u);
    auto* tex = create<core::type::SampledTexture>(core::type::TextureDimension::k2d, f32);
    auto result = table.Lookup(wgsl::BuiltinFn::kTextureLoad, Empty, Vector{tex, vec2i, i32},
                               core::EvaluationStage::kConstant);
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

TEST_F(WgslIntrinsicTableTest, MatchMultisampledTexture) {
    auto* i32 = create<core::type::I32>();
    auto* f32 = create<core::type::F32>();
    auto* vec2i = create<core::type::Vector>(i32, 2u);
    auto* vec4f = create<core::type::Vector>(f32, 4u);
    auto* tex = create<core::type::MultisampledTexture>(core::type::TextureDimension::k2d, f32);
    auto result = table.Lookup(wgsl::BuiltinFn::kTextureLoad, Empty, Vector{tex, vec2i, i32},
                               core::EvaluationStage::kConstant);
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

TEST_F(WgslIntrinsicTableTest, MatchDepthTexture) {
    auto* f32 = create<core::type::F32>();
    auto* i32 = create<core::type::I32>();
    auto* vec2i = create<core::type::Vector>(i32, 2u);
    auto* tex = create<core::type::DepthTexture>(core::type::TextureDimension::k2d);
    auto result = table.Lookup(wgsl::BuiltinFn::kTextureLoad, Empty, Vector{tex, vec2i, i32},
                               core::EvaluationStage::kConstant);
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

TEST_F(WgslIntrinsicTableTest, MatchDepthMultisampledTexture) {
    auto* f32 = create<core::type::F32>();
    auto* i32 = create<core::type::I32>();
    auto* vec2i = create<core::type::Vector>(i32, 2u);
    auto* tex = create<core::type::DepthMultisampledTexture>(core::type::TextureDimension::k2d);
    auto result = table.Lookup(wgsl::BuiltinFn::kTextureLoad, Empty, Vector{tex, vec2i, i32},
                               core::EvaluationStage::kConstant);
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

TEST_F(WgslIntrinsicTableTest, MatchExternalTexture) {
    auto* f32 = create<core::type::F32>();
    auto* i32 = create<core::type::I32>();
    auto* vec2i = create<core::type::Vector>(i32, 2u);
    auto* vec4f = create<core::type::Vector>(f32, 4u);
    auto* tex = create<core::type::ExternalTexture>();
    auto result = table.Lookup(wgsl::BuiltinFn::kTextureLoad, Empty, Vector{tex, vec2i},
                               core::EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, vec4f);
    ASSERT_EQ(result->parameters.Length(), 2u);
    EXPECT_EQ(result->parameters[0].type, tex);
    EXPECT_EQ(result->parameters[0].usage, ParameterUsage::kTexture);
    EXPECT_EQ(result->parameters[1].type, vec2i);
    EXPECT_EQ(result->parameters[1].usage, ParameterUsage::kCoords);
}

TEST_F(WgslIntrinsicTableTest, MatchWOStorageTexture) {
    auto* f32 = create<core::type::F32>();
    auto* i32 = create<core::type::I32>();
    auto* vec2i = create<core::type::Vector>(i32, 2u);
    auto* vec4f = create<core::type::Vector>(f32, 4u);
    auto* tex = Types().storage_texture(core::type::TextureDimension::k2d,
                                        core::TexelFormat::kR32Float, core::Access::kWrite);

    auto result = table.Lookup(wgsl::BuiltinFn::kTextureStore, Empty, Vector{tex, vec2i, vec4f},
                               core::EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_TRUE(result->return_type->Is<core::type::Void>());
    ASSERT_EQ(result->parameters.Length(), 3u);
    EXPECT_EQ(result->parameters[0].type, tex);
    EXPECT_EQ(result->parameters[0].usage, ParameterUsage::kTexture);
    EXPECT_EQ(result->parameters[1].type, vec2i);
    EXPECT_EQ(result->parameters[1].usage, ParameterUsage::kCoords);
    EXPECT_EQ(result->parameters[2].type, vec4f);
    EXPECT_EQ(result->parameters[2].usage, ParameterUsage::kValue);
}

TEST_F(WgslIntrinsicTableTest, MismatchTexture) {
    auto* f32 = create<core::type::F32>();
    auto* i32 = create<core::type::I32>();
    auto* vec2i = create<core::type::Vector>(i32, 2u);
    auto result = table.Lookup(wgsl::BuiltinFn::kTextureLoad, Empty, Vector{f32, vec2i},
                               core::EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    ASSERT_THAT(result.Failure().Plain(), HasSubstr("no matching call"));
}

TEST_F(WgslIntrinsicTableTest, MatchTemplateType) {
    auto* f32 = create<core::type::F32>();
    auto result = table.Lookup(wgsl::BuiltinFn::kClamp, Empty, Vector{f32, f32, f32},
                               core::EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, f32);
    EXPECT_EQ(result->parameters[0].type, f32);
    EXPECT_EQ(result->parameters[1].type, f32);
    EXPECT_EQ(result->parameters[2].type, f32);
}

TEST_F(WgslIntrinsicTableTest, MismatchTemplateType) {
    auto* f32 = create<core::type::F32>();
    auto* u32 = create<core::type::U32>();
    auto result = table.Lookup(wgsl::BuiltinFn::kClamp, Empty, Vector{f32, u32, f32},
                               core::EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    ASSERT_THAT(result.Failure().Plain(), HasSubstr("no matching call"));
}

TEST_F(WgslIntrinsicTableTest, MatchOpenSizeVector) {
    auto* f32 = create<core::type::F32>();
    auto* vec2f = create<core::type::Vector>(f32, 2u);
    auto result = table.Lookup(wgsl::BuiltinFn::kClamp, Empty, Vector{vec2f, vec2f, vec2f},
                               core::EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, vec2f);
    ASSERT_EQ(result->parameters.Length(), 3u);
    EXPECT_EQ(result->parameters[0].type, vec2f);
    EXPECT_EQ(result->parameters[1].type, vec2f);
    EXPECT_EQ(result->parameters[2].type, vec2f);
}

TEST_F(WgslIntrinsicTableTest, MismatchOpenSizeVector) {
    auto* f32 = create<core::type::F32>();
    auto* u32 = create<core::type::U32>();
    auto* vec2f = create<core::type::Vector>(f32, 2u);
    auto result = table.Lookup(wgsl::BuiltinFn::kClamp, Empty, Vector{vec2f, u32, vec2f},
                               core::EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    ASSERT_THAT(result.Failure().Plain(), HasSubstr("no matching call"));
}

TEST_F(WgslIntrinsicTableTest, MatchOpenSizeMatrix) {
    auto* f32 = create<core::type::F32>();
    auto* vec3f = create<core::type::Vector>(f32, 3u);
    auto* mat3x3f = create<core::type::Matrix>(vec3f, 3u);
    auto result = table.Lookup(wgsl::BuiltinFn::kDeterminant, Empty, Vector{mat3x3f},
                               core::EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, f32);
    ASSERT_EQ(result->parameters.Length(), 1u);
    EXPECT_EQ(result->parameters[0].type, mat3x3f);
}

TEST_F(WgslIntrinsicTableTest, MismatchOpenSizeMatrix) {
    auto* f32 = create<core::type::F32>();
    auto* vec2f = create<core::type::Vector>(f32, 2u);
    auto* mat3x2f = create<core::type::Matrix>(vec2f, 3u);
    auto result = table.Lookup(wgsl::BuiltinFn::kDeterminant, Empty, Vector{mat3x2f},
                               core::EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    ASSERT_THAT(result.Failure().Plain(), HasSubstr("no matching call"));
}

TEST_F(WgslIntrinsicTableTest, MatchDifferentArgsElementType_Builtin_ConstantEval) {
    auto* af = create<core::type::AbstractFloat>();
    auto* bool_ = create<core::type::Bool>();
    auto result = table.Lookup(wgsl::BuiltinFn::kSelect, Empty, Vector{af, af, bool_},
                               core::EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_NE(result->const_eval_fn, nullptr);
    EXPECT_EQ(result->return_type, af);
    ASSERT_EQ(result->parameters.Length(), 3u);
    EXPECT_EQ(result->parameters[0].type, af);
    EXPECT_EQ(result->parameters[1].type, af);
    EXPECT_EQ(result->parameters[2].type, bool_);
}

TEST_F(WgslIntrinsicTableTest, MatchDifferentArgsElementType_Builtin_RuntimeEval) {
    auto* af = create<core::type::AbstractFloat>();
    auto result =
        table.Lookup(wgsl::BuiltinFn::kSelect, Empty, Vector{af, af, create<core::type::Bool>()},
                     core::EvaluationStage::kRuntime);
    ASSERT_EQ(result, Success);
    EXPECT_NE(result->const_eval_fn, nullptr);
    EXPECT_TRUE(result->return_type->Is<core::type::F32>());
    ASSERT_EQ(result->parameters.Length(), 3u);
    EXPECT_TRUE(result->parameters[0].type->Is<core::type::F32>());
    EXPECT_TRUE(result->parameters[1].type->Is<core::type::F32>());
    EXPECT_TRUE(result->parameters[2].type->Is<core::type::Bool>());
}

TEST_F(WgslIntrinsicTableTest, MatchDifferentArgsElementType_Binary_ConstantEval) {
    auto* ai = create<core::type::AbstractInt>();
    auto* u32 = create<core::type::U32>();
    auto result =
        table.Lookup(core::BinaryOp::kShiftLeft, ai, u32, core::EvaluationStage::kConstant, false);
    ASSERT_EQ(result, Success);
    ASSERT_NE(result->const_eval_fn, nullptr) << Diagnostics();
    EXPECT_EQ(result->return_type, ai);
    EXPECT_EQ(result->parameters[0].type, ai);
    EXPECT_EQ(result->parameters[1].type, u32);
}

TEST_F(WgslIntrinsicTableTest, MatchDifferentArgsElementType_Binary_RuntimeEval) {
    auto* ai = create<core::type::AbstractInt>();
    auto* u32 = create<core::type::U32>();
    auto result =
        table.Lookup(core::BinaryOp::kShiftLeft, ai, u32, core::EvaluationStage::kRuntime, false);
    ASSERT_EQ(result, Success);
    ASSERT_NE(result->const_eval_fn, nullptr) << Diagnostics();
    EXPECT_TRUE(result->return_type->Is<core::type::I32>());
    EXPECT_TRUE(result->parameters[0].type->Is<core::type::I32>());
    EXPECT_TRUE(result->parameters[1].type->Is<core::type::U32>());
}

TEST_F(WgslIntrinsicTableTest, OverloadOrderByNumberOfParameters) {
    // None of the arguments match, so expect the overloads with 2 parameters to
    // come first
    auto* bool_ = create<core::type::Bool>();
    auto result = table.Lookup(wgsl::BuiltinFn::kTextureDimensions, Empty, Vector{bool_, bool_},
                               core::EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    ASSERT_EQ(result.Failure().Plain(),
              R"(no matching call to 'textureDimensions(bool, bool)'

33 candidate functions:
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
 • 'textureDimensions(texture: texture_storage_1d<F, R>  ✗ ) -> u32' where:
      ✗  'R' is 'read'
 • 'textureDimensions(texture: texture_storage_2d<F, R>  ✗ ) -> vec2<u32>' where:
      ✗  'R' is 'read'
 • 'textureDimensions(texture: texture_storage_2d_array<F, R>  ✗ ) -> vec2<u32>' where:
      ✗  'R' is 'read'
 • 'textureDimensions(texture: texture_storage_3d<F, R>  ✗ ) -> vec3<u32>' where:
      ✗  'R' is 'read'
 • 'textureDimensions(texture: texel_buffer<F, R>  ✗ ) -> u32' where:
      ✗  'R' is 'read'
 • 'textureDimensions(texture: texture_storage_1d<F, W>  ✗ ) -> u32' where:
      ✗  'W' is 'write' or 'read_write'
 • 'textureDimensions(texture: texture_storage_2d<F, W>  ✗ ) -> vec2<u32>' where:
      ✗  'W' is 'write' or 'read_write'
 • 'textureDimensions(texture: texture_storage_2d_array<F, W>  ✗ ) -> vec2<u32>' where:
      ✗  'W' is 'write' or 'read_write'
 • 'textureDimensions(texture: texture_storage_3d<F, W>  ✗ ) -> vec3<u32>' where:
      ✗  'W' is 'write' or 'read_write'
 • 'textureDimensions(texture: texel_buffer<F, W>  ✗ ) -> u32' where:
      ✗  'W' is 'write' or 'read_write'
)");
}

TEST_F(WgslIntrinsicTableTest, OverloadOrderByMatchingParameter) {
    auto* tex = create<core::type::DepthTexture>(core::type::TextureDimension::k2d);
    auto* bool_ = create<core::type::Bool>();
    auto result = table.Lookup(wgsl::BuiltinFn::kTextureDimensions, Empty, Vector{tex, bool_},
                               core::EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    ASSERT_EQ(result.Failure().Plain(),
              R"(no matching call to 'textureDimensions(texture_depth_2d, bool)'

33 candidate functions:
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
 • 'textureDimensions(texture: texture_storage_1d<F, R>  ✗ ) -> u32' where:
      ✗  'R' is 'read'
 • 'textureDimensions(texture: texture_storage_2d<F, R>  ✗ ) -> vec2<u32>' where:
      ✗  'R' is 'read'
 • 'textureDimensions(texture: texture_storage_2d_array<F, R>  ✗ ) -> vec2<u32>' where:
      ✗  'R' is 'read'
 • 'textureDimensions(texture: texture_storage_3d<F, R>  ✗ ) -> vec3<u32>' where:
      ✗  'R' is 'read'
 • 'textureDimensions(texture: texel_buffer<F, R>  ✗ ) -> u32' where:
      ✗  'R' is 'read'
 • 'textureDimensions(texture: texture_storage_1d<F, W>  ✗ ) -> u32' where:
      ✗  'W' is 'write' or 'read_write'
 • 'textureDimensions(texture: texture_storage_2d<F, W>  ✗ ) -> vec2<u32>' where:
      ✗  'W' is 'write' or 'read_write'
 • 'textureDimensions(texture: texture_storage_2d_array<F, W>  ✗ ) -> vec2<u32>' where:
      ✗  'W' is 'write' or 'read_write'
 • 'textureDimensions(texture: texture_storage_3d<F, W>  ✗ ) -> vec3<u32>' where:
      ✗  'W' is 'write' or 'read_write'
 • 'textureDimensions(texture: texel_buffer<F, W>  ✗ ) -> u32' where:
      ✗  'W' is 'write' or 'read_write'
)");
}

TEST_F(WgslIntrinsicTableTest, MatchUnaryOp) {
    auto* i32 = create<core::type::I32>();
    auto* vec3i = create<core::type::Vector>(i32, 3u);
    auto result = table.Lookup(core::UnaryOp::kNegation, vec3i, core::EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, vec3i);
}

TEST_F(WgslIntrinsicTableTest, MismatchUnaryOp) {
    auto* bool_ = create<core::type::Bool>();
    auto result = table.Lookup(core::UnaryOp::kNegation, bool_, core::EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().Plain(), R"(no matching overload for 'operator - (bool)'

2 candidate operators:
 • 'operator - (T  ✗ ) -> T' where:
      ✗  'T' is 'abstract-float', 'abstract-int', 'f32', 'i32' or 'f16'
 • 'operator - (vecN<T>  ✗ ) -> vecN<T>' where:
      ✗  'T' is 'abstract-float', 'abstract-int', 'f32', 'i32' or 'f16'
)");
}

TEST_F(WgslIntrinsicTableTest, MatchUnaryOp_Constant) {
    auto* ai = create<core::type::AbstractInt>();
    auto result = table.Lookup(core::UnaryOp::kNegation, ai, core::EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, ai);
}

TEST_F(WgslIntrinsicTableTest, MatchUnaryOp_Runtime) {
    auto* ai = create<core::type::AbstractInt>();
    auto result = table.Lookup(core::UnaryOp::kNegation, ai, core::EvaluationStage::kRuntime);
    ASSERT_EQ(result, Success);
    EXPECT_NE(result->return_type, ai);
    EXPECT_TRUE(result->return_type->Is<core::type::I32>());
}

TEST_F(WgslIntrinsicTableTest, MatchBinaryOp) {
    auto* i32 = create<core::type::I32>();
    auto* vec3i = create<core::type::Vector>(i32, 3u);
    auto result =
        table.Lookup(core::BinaryOp::kMultiply, i32, vec3i, core::EvaluationStage::kConstant,
                     /* is_compound */ false);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, vec3i);
    EXPECT_EQ(result->parameters[0].type, i32);
    EXPECT_EQ(result->parameters[1].type, vec3i);
}

TEST_F(WgslIntrinsicTableTest, MismatchBinaryOp) {
    auto* f32 = create<core::type::F32>();
    auto* bool_ = create<core::type::Bool>();
    auto result =
        table.Lookup(core::BinaryOp::kMultiply, f32, bool_, core::EvaluationStage::kConstant,
                     /* is_compound */ false);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().Plain(), R"(no matching overload for 'operator * (f32, bool)'

9 candidate operators:
 • 'operator * (T  ✓ , T  ✗ ) -> T' where:
      ✓  'T' is 'abstract-float', 'abstract-int', 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (T  ✓ , vecN<T>  ✗ ) -> vecN<T>' where:
      ✓  'T' is 'abstract-float', 'abstract-int', 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (T  ✓ , matNxM<T>  ✗ ) -> matNxM<T>' where:
      ✓  'T' is 'abstract-float', 'f32' or 'f16'
 • 'operator * (vecN<T>  ✗ , T  ✗ ) -> vecN<T>' where:
      ✗  'T' is 'abstract-float', 'abstract-int', 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (matNxM<T>  ✗ , T  ✗ ) -> matNxM<T>' where:
      ✗  'T' is 'abstract-float', 'f32' or 'f16'
 • 'operator * (vecN<T>  ✗ , vecN<T>  ✗ ) -> vecN<T>' where:
      ✗  'T' is 'abstract-float', 'abstract-int', 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (matCxR<T>  ✗ , vecC<T>  ✗ ) -> vecR<T>' where:
      ✗  'T' is 'abstract-float', 'f32' or 'f16'
 • 'operator * (vecR<T>  ✗ , matCxR<T>  ✗ ) -> vecC<T>' where:
      ✗  'T' is 'abstract-float', 'f32' or 'f16'
 • 'operator * (matKxR<T>  ✗ , matCxK<T>  ✗ ) -> matCxR<T>' where:
      ✗  'T' is 'abstract-float', 'f32' or 'f16'
)");
}

TEST_F(WgslIntrinsicTableTest, MatchCompoundOp) {
    auto* i32 = create<core::type::I32>();
    auto* vec3i = create<core::type::Vector>(i32, 3u);
    auto result =
        table.Lookup(core::BinaryOp::kMultiply, i32, vec3i, core::EvaluationStage::kConstant,
                     /* is_compound */ true);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, vec3i);
    EXPECT_EQ(result->parameters[0].type, i32);
    EXPECT_EQ(result->parameters[1].type, vec3i);
}

TEST_F(WgslIntrinsicTableTest, MismatchCompoundOp) {
    auto* f32 = create<core::type::F32>();
    auto* bool_ = create<core::type::Bool>();
    auto result =
        table.Lookup(core::BinaryOp::kMultiply, f32, bool_, core::EvaluationStage::kConstant,
                     /* is_compound */ true);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().Plain(), R"(no matching overload for 'operator *= (f32, bool)'

9 candidate operators:
 • 'operator *= (T  ✓ , T  ✗ ) -> T' where:
      ✓  'T' is 'abstract-float', 'abstract-int', 'f32', 'i32', 'u32' or 'f16'
 • 'operator *= (T  ✓ , vecN<T>  ✗ ) -> vecN<T>' where:
      ✓  'T' is 'abstract-float', 'abstract-int', 'f32', 'i32', 'u32' or 'f16'
 • 'operator *= (T  ✓ , matNxM<T>  ✗ ) -> matNxM<T>' where:
      ✓  'T' is 'abstract-float', 'f32' or 'f16'
 • 'operator *= (vecN<T>  ✗ , T  ✗ ) -> vecN<T>' where:
      ✗  'T' is 'abstract-float', 'abstract-int', 'f32', 'i32', 'u32' or 'f16'
 • 'operator *= (matNxM<T>  ✗ , T  ✗ ) -> matNxM<T>' where:
      ✗  'T' is 'abstract-float', 'f32' or 'f16'
 • 'operator *= (vecN<T>  ✗ , vecN<T>  ✗ ) -> vecN<T>' where:
      ✗  'T' is 'abstract-float', 'abstract-int', 'f32', 'i32', 'u32' or 'f16'
 • 'operator *= (matCxR<T>  ✗ , vecC<T>  ✗ ) -> vecR<T>' where:
      ✗  'T' is 'abstract-float', 'f32' or 'f16'
 • 'operator *= (vecR<T>  ✗ , matCxR<T>  ✗ ) -> vecC<T>' where:
      ✗  'T' is 'abstract-float', 'f32' or 'f16'
 • 'operator *= (matKxR<T>  ✗ , matCxK<T>  ✗ ) -> matCxR<T>' where:
      ✗  'T' is 'abstract-float', 'f32' or 'f16'
)");
}

TEST_F(WgslIntrinsicTableTest, MatchTypeInitializerImplicit) {
    auto* i32 = create<core::type::I32>();
    auto* vec3i = create<core::type::Vector>(i32, 3u);
    auto result = table.Lookup(CtorConv::kVec3, Empty, Vector{i32, i32, i32},
                               core::EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, vec3i);
    EXPECT_TRUE(result->info->flags.Contains(OverloadFlag::kIsConstructor));
    ASSERT_EQ(result->parameters.Length(), 3u);
    EXPECT_EQ(result->parameters[0].type, i32);
    EXPECT_EQ(result->parameters[1].type, i32);
    EXPECT_EQ(result->parameters[2].type, i32);
    EXPECT_NE(result->const_eval_fn, nullptr);
}

TEST_F(WgslIntrinsicTableTest, MatchTypeInitializerExplicit) {
    auto* i32 = create<core::type::I32>();
    auto* vec3i = create<core::type::Vector>(i32, 3u);
    auto result = table.Lookup(CtorConv::kVec3, Vector{i32}, Vector{i32, i32, i32},
                               core::EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, vec3i);
    EXPECT_TRUE(result->info->flags.Contains(OverloadFlag::kIsConstructor));
    ASSERT_EQ(result->parameters.Length(), 3u);
    EXPECT_EQ(result->parameters[0].type, i32);
    EXPECT_EQ(result->parameters[1].type, i32);
    EXPECT_EQ(result->parameters[2].type, i32);
    EXPECT_NE(result->const_eval_fn, nullptr);
}

TEST_F(WgslIntrinsicTableTest, MismatchTypeInitializerImplicit) {
    auto* i32 = create<core::type::I32>();
    auto* f32 = create<core::type::F32>();
    auto result = table.Lookup(CtorConv::kVec3, Empty, Vector{i32, f32, i32},
                               core::EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().Plain(),
              R"(no matching constructor for 'vec3(i32, f32, i32)'

12 candidate constructors:
 • 'vec3(x: T  ✓ , y: T  ✗ , z: T  ✓ ) -> vec3<T>' where:
      ✓  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec3(xy: vec2<T>  ✗ , z: T  ✓ ) -> vec3<T>' where:
      ✓  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec3(x: T  ✓ , yz: vec2<T>  ✗ ) -> vec3<T>' where:
      ✓  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec3(T  ✓ ) -> vec3<T>' where:
      ✗  overload expects 1 argument, call passed 3 arguments
      ✓  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec3() -> vec3<abstract-int>' where:
      ✗  overload expects 0 arguments, call passed 3 arguments
 • 'vec3(vec3<T>  ✗ ) -> vec3<T>' where:
      ✗  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec3<T  ✗ >(x: T  ✓ , y: T  ✗ , z: T  ✓ ) -> vec3<T>' where:
      ✓  'T' is 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec3<T  ✗ >(xy: vec2<T>  ✗ , z: T  ✓ ) -> vec3<T>' where:
      ✓  'T' is 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec3<T  ✗ >(x: T  ✓ , yz: vec2<T>  ✗ ) -> vec3<T>' where:
      ✓  'T' is 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec3<T  ✗ >(T  ✓ ) -> vec3<T>' where:
      ✗  overload expects 1 argument, call passed 3 arguments
      ✓  'T' is 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec3<T  ✗ >(vec3<T>  ✗ ) -> vec3<T>' where:
      ✗  'T' is 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec3<T  ✗ >() -> vec3<T>' where:
      ✗  overload expects 0 arguments, call passed 3 arguments
      ✗  'T' is 'f32', 'f16', 'i32', 'u32' or 'bool'

5 candidate conversions:
 • 'vec3<T  ✗ >(vec3<U>  ✗ ) -> vec3<T>' where:
      ✗  'T' is 'f32'
      ✗  'U' is 'abstract-int', 'abstract-float', 'i32', 'f16', 'u32' or 'bool'
 • 'vec3<T  ✗ >(vec3<U>  ✗ ) -> vec3<T>' where:
      ✗  'T' is 'f16'
      ✗  'U' is 'abstract-int', 'abstract-float', 'f32', 'i32', 'u32' or 'bool'
 • 'vec3<T  ✗ >(vec3<U>  ✗ ) -> vec3<T>' where:
      ✗  'T' is 'i32'
      ✗  'U' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'u32' or 'bool'
 • 'vec3<T  ✗ >(vec3<U>  ✗ ) -> vec3<T>' where:
      ✗  'T' is 'u32'
      ✗  'U' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32' or 'bool'
 • 'vec3<T  ✗ >(vec3<U>  ✗ ) -> vec3<T>' where:
      ✗  'T' is 'bool'
      ✗  'U' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32' or 'u32'
)");
}

TEST_F(WgslIntrinsicTableTest, MismatchTypeInitializerExplicit) {
    auto* i32 = create<core::type::I32>();
    auto* f32 = create<core::type::F32>();
    auto result = table.Lookup(CtorConv::kVec3, Vector{i32}, Vector{i32, f32, i32},
                               core::EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().Plain(),
              R"(no matching constructor for 'vec3<i32>(i32, f32, i32)'

12 candidate constructors:
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
 • 'vec3(x: T  ✓ , y: T  ✗ , z: T  ✓ ) -> vec3<T>' where:
      ✓  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec3(xy: vec2<T>  ✗ , z: T  ✓ ) -> vec3<T>' where:
      ✓  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec3(x: T  ✓ , yz: vec2<T>  ✗ ) -> vec3<T>' where:
      ✓  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec3(T  ✓ ) -> vec3<T>' where:
      ✗  overload expects 1 argument, call passed 3 arguments
      ✗  overload expects 0 template arguments, call passed 1 argument
      ✓  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec3() -> vec3<abstract-int>' where:
      ✗  overload expects 0 arguments, call passed 3 arguments
      ✗  overload expects 0 template arguments, call passed 1 argument
 • 'vec3(vec3<T>  ✗ ) -> vec3<T>' where:
      ✗  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'

5 candidate conversions:
 • 'vec3<T  ✓ >(vec3<U>  ✗ ) -> vec3<T>' where:
      ✓  'T' is 'i32'
      ✗  'U' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'u32' or 'bool'
 • 'vec3<T  ✗ >(vec3<U>  ✗ ) -> vec3<T>' where:
      ✗  'T' is 'f32'
      ✗  'U' is 'abstract-int', 'abstract-float', 'i32', 'f16', 'u32' or 'bool'
 • 'vec3<T  ✗ >(vec3<U>  ✗ ) -> vec3<T>' where:
      ✗  'T' is 'f16'
      ✗  'U' is 'abstract-int', 'abstract-float', 'f32', 'i32', 'u32' or 'bool'
 • 'vec3<T  ✗ >(vec3<U>  ✗ ) -> vec3<T>' where:
      ✗  'T' is 'u32'
      ✗  'U' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32' or 'bool'
 • 'vec3<T  ✗ >(vec3<U>  ✗ ) -> vec3<T>' where:
      ✗  'T' is 'bool'
      ✗  'U' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32' or 'u32'
)");
}

TEST_F(WgslIntrinsicTableTest, MatchTypeInitializerImplicitVecFromVecAbstract) {
    auto* ai = create<core::type::AbstractInt>();
    auto* vec3_ai = create<core::type::Vector>(ai, 3u);
    auto result =
        table.Lookup(CtorConv::kVec3, Empty, Vector{vec3_ai}, core::EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, vec3_ai);
    EXPECT_TRUE(result->info->flags.Contains(OverloadFlag::kIsConstructor));
    ASSERT_EQ(result->parameters.Length(), 1u);
    EXPECT_EQ(result->parameters[0].type, vec3_ai);
    EXPECT_NE(result->const_eval_fn, nullptr);
}

TEST_F(WgslIntrinsicTableTest, MatchTypeInitializerImplicitMatFromVec) {
    auto* af = create<core::type::AbstractFloat>();
    auto* vec2_ai = create<core::type::Vector>(create<core::type::AbstractInt>(), 2u);
    auto* vec2_af = create<core::type::Vector>(af, 2u);
    auto* mat2x2_af = create<core::type::Matrix>(vec2_af, 2u);
    auto result = table.Lookup(CtorConv::kMat2x2, Empty, Vector{vec2_ai, vec2_ai},
                               core::EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_TYPE(result->return_type, mat2x2_af);
    EXPECT_TRUE(result->info->flags.Contains(OverloadFlag::kIsConstructor));
    ASSERT_EQ(result->parameters.Length(), 2u);
    EXPECT_TYPE(result->parameters[0].type, vec2_af);
    EXPECT_TYPE(result->parameters[1].type, vec2_af);
    EXPECT_NE(result->const_eval_fn, nullptr);
}

TEST_F(WgslIntrinsicTableTest, MatchTypeInitializer_ConstantEval) {
    auto* ai = create<core::type::AbstractInt>();
    auto* vec3_ai = create<core::type::Vector>(ai, 3u);
    auto result =
        table.Lookup(CtorConv::kVec3, Empty, Vector{ai, ai, ai}, core::EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_NE(result->const_eval_fn, nullptr);
    EXPECT_EQ(result->return_type, vec3_ai);
    EXPECT_TRUE(result->info->flags.Contains(OverloadFlag::kIsConstructor));
    ASSERT_EQ(result->parameters.Length(), 3u);
    EXPECT_EQ(result->parameters[0].type, ai);
    EXPECT_EQ(result->parameters[1].type, ai);
    EXPECT_EQ(result->parameters[2].type, ai);
    EXPECT_NE(result->const_eval_fn, nullptr);
}

TEST_F(WgslIntrinsicTableTest, MatchTypeInitializer_RuntimeEval) {
    auto* ai = create<core::type::AbstractInt>();
    auto result =
        table.Lookup(CtorConv::kVec3, Empty, Vector{ai, ai, ai}, core::EvaluationStage::kRuntime);
    auto* i32 = create<core::type::I32>();
    auto* vec3i = create<core::type::Vector>(i32, 3u);
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

TEST_F(WgslIntrinsicTableTest, MatchTypeConversion) {
    auto* i32 = create<core::type::I32>();
    auto* vec3i = create<core::type::Vector>(i32, 3u);
    auto* f32 = create<core::type::F32>();
    auto* vec3f = create<core::type::Vector>(f32, 3u);
    auto result =
        table.Lookup(CtorConv::kVec3, Vector{i32}, Vector{vec3f}, core::EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, vec3i);
    EXPECT_FALSE(result->info->flags.Contains(OverloadFlag::kIsConstructor));
    ASSERT_EQ(result->parameters.Length(), 1u);
    EXPECT_EQ(result->parameters[0].type, vec3f);
}

TEST_F(WgslIntrinsicTableTest, MismatchTypeConversion) {
    auto* arr = create<core::type::Array>(create<core::type::U32>(),
                                          create<core::type::RuntimeArrayCount>(), 4u, 4u, 4u, 4u);
    auto* f32 = create<core::type::F32>();
    auto result =
        table.Lookup(CtorConv::kVec3, Vector{f32}, Vector{arr}, core::EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().Plain(),
              R"(no matching constructor for 'vec3<f32>(array<u32>)'

12 candidate constructors:
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
 • 'vec3(T  ✗ ) -> vec3<T>' where:
      ✗  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec3() -> vec3<abstract-int>' where:
      ✗  overload expects 0 arguments, call passed 1 argument
      ✗  overload expects 0 template arguments, call passed 1 argument
 • 'vec3(vec3<T>  ✗ ) -> vec3<T>' where:
      ✗  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec3(x: T  ✗ , yz: vec2<T>  ✗ ) -> vec3<T>' where:
      ✗  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec3(xy: vec2<T>  ✗ , z: T  ✗ ) -> vec3<T>' where:
      ✗  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec3(x: T  ✗ , y: T  ✗ , z: T  ✗ ) -> vec3<T>' where:
      ✗  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'

5 candidate conversions:
 • 'vec3<T  ✓ >(vec3<U>  ✗ ) -> vec3<T>' where:
      ✓  'T' is 'f32'
      ✗  'U' is 'abstract-int', 'abstract-float', 'i32', 'f16', 'u32' or 'bool'
 • 'vec3<T  ✗ >(vec3<U>  ✗ ) -> vec3<T>' where:
      ✗  'T' is 'f16'
      ✗  'U' is 'abstract-int', 'abstract-float', 'f32', 'i32', 'u32' or 'bool'
 • 'vec3<T  ✗ >(vec3<U>  ✗ ) -> vec3<T>' where:
      ✗  'T' is 'i32'
      ✗  'U' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'u32' or 'bool'
 • 'vec3<T  ✗ >(vec3<U>  ✗ ) -> vec3<T>' where:
      ✗  'T' is 'u32'
      ✗  'U' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32' or 'bool'
 • 'vec3<T  ✗ >(vec3<U>  ✗ ) -> vec3<T>' where:
      ✗  'T' is 'bool'
      ✗  'U' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32' or 'u32'
)");
}

TEST_F(WgslIntrinsicTableTest, MatchTypeConversion_ConstantEval) {
    auto* ai = create<core::type::AbstractInt>();
    auto* af = create<core::type::AbstractFloat>();
    auto* vec3_ai = create<core::type::Vector>(ai, 3u);
    auto* f32 = create<core::type::F32>();
    auto* vec3f = create<core::type::Vector>(f32, 3u);
    auto result = table.Lookup(CtorConv::kVec3, Vector{af}, Vector{vec3_ai},
                               core::EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_NE(result->const_eval_fn, nullptr);
    // NOTE: Conversions are explicit, so there's no way to have it return abstracts
    EXPECT_EQ(result->return_type, vec3f);
    EXPECT_FALSE(result->info->flags.Contains(OverloadFlag::kIsConstructor));
    ASSERT_EQ(result->parameters.Length(), 1u);
    EXPECT_EQ(result->parameters[0].type, vec3_ai);
}

TEST_F(WgslIntrinsicTableTest, MatchTypeConversion_RuntimeEval) {
    auto* ai = create<core::type::AbstractInt>();
    auto* af = create<core::type::AbstractFloat>();
    auto* vec3_ai = create<core::type::Vector>(ai, 3u);
    auto* vec3f = create<core::type::Vector>(create<core::type::F32>(), 3u);
    auto* vec3i = create<core::type::Vector>(create<core::type::I32>(), 3u);
    auto result =
        table.Lookup(CtorConv::kVec3, Vector{af}, Vector{vec3_ai}, core::EvaluationStage::kRuntime);
    ASSERT_EQ(result, Success);
    EXPECT_NE(result->const_eval_fn, nullptr);
    EXPECT_EQ(result->return_type, vec3f);
    EXPECT_FALSE(result->info->flags.Contains(OverloadFlag::kIsConstructor));
    ASSERT_EQ(result->parameters.Length(), 1u);
    EXPECT_EQ(result->parameters[0].type, vec3i);
}

TEST_F(WgslIntrinsicTableTest, Err257Arguments) {  // crbug.com/1323605
    auto* f32 = create<core::type::F32>();
    Vector<const core::type::Type*, 0> arg_tys;
    arg_tys.Resize(257, f32);
    auto result = table.Lookup(wgsl::BuiltinFn::kAbs, Empty, std::move(arg_tys),
                               core::EvaluationStage::kConstant);
    ASSERT_NE(result, Success);
    ASSERT_THAT(result.Failure().Plain(), HasSubstr("no matching call"));
}

TEST_F(WgslIntrinsicTableTest, OverloadResolution) {
    // i32(abstract-int) produces candidates for both:
    //    ctor i32(i32) -> i32
    //    conv i32<T: scalar_no_i32>(T) -> i32
    // The first should win overload resolution.
    auto* ai = create<core::type::AbstractInt>();
    auto* i32 = create<core::type::I32>();
    auto result = table.Lookup(CtorConv::kI32, Empty, Vector{ai}, core::EvaluationStage::kConstant);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result->return_type, i32);
    EXPECT_EQ(result->parameters.Length(), 1u);
    EXPECT_EQ(result->parameters[0].type, ai);
}

////////////////////////////////////////////////////////////////////////////////
// AbstractBinaryTests
////////////////////////////////////////////////////////////////////////////////
namespace AbstractBinaryTests {
struct Case {
    template <typename RESULT,
              typename PARAM_LHS,
              typename PARAM_RHS,
              typename ARG_LHS,
              typename ARG_RHS>
    static Case Create(bool match = true) {
        return {
            match,                                        //
            resolver::builder::DataType<RESULT>::Sem,     //
            resolver::builder::DataType<PARAM_LHS>::Sem,  //
            resolver::builder::DataType<PARAM_RHS>::Sem,  //
            resolver::builder::DataType<ARG_LHS>::Sem,    //
            resolver::builder::DataType<ARG_RHS>::Sem,    //
        };
    }
    bool expected_match;
    resolver::builder::sem_type_func_ptr expected_result;
    resolver::builder::sem_type_func_ptr expected_param_lhs;
    resolver::builder::sem_type_func_ptr expected_param_rhs;
    resolver::builder::sem_type_func_ptr arg_lhs;
    resolver::builder::sem_type_func_ptr arg_rhs;
};

struct IntrinsicTableAbstractBinaryTest : public resolver::ResolverTestWithParam<Case> {
    core::intrinsic::Table<Dialect> table{Types(), Symbols()};
};

TEST_P(IntrinsicTableAbstractBinaryTest, MatchAdd) {
    auto* arg_lhs = GetParam().arg_lhs(*this);
    auto* arg_rhs = GetParam().arg_rhs(*this);
    auto result =
        table.Lookup(core::BinaryOp::kAdd, arg_lhs, arg_rhs, core::EvaluationStage::kConstant,
                     /* is_compound */ false);

    bool matched = result == Success;
    bool expected_match = GetParam().expected_match;
    EXPECT_EQ(matched, expected_match) << Diagnostics();

    if (matched) {
        auto* expected_result = GetParam().expected_result(*this);
        EXPECT_TYPE(result->return_type, expected_result);

        auto* expected_param_lhs = GetParam().expected_param_lhs(*this);
        EXPECT_TYPE(result->parameters[0].type, expected_param_lhs);

        auto* expected_param_rhs = GetParam().expected_param_rhs(*this);
        EXPECT_TYPE(result->parameters[1].type, expected_param_rhs);
    }
}

INSTANTIATE_TEST_SUITE_P(AFloat_AInt,
                         IntrinsicTableAbstractBinaryTest,
                         testing::Values(  // clang-format off
//            result   | param lhs | param rhs |  arg lhs  |  arg rhs
Case::Create<AFloat,     AFloat,     AFloat,     AFloat,     AFloat>(),
Case::Create<AFloat,     AFloat,     AFloat,     AFloat,     AInt>(),
Case::Create<AFloat,     AFloat,     AFloat,     AInt,       AFloat>(),
Case::Create<AInt,       AInt,       AInt,       AInt,       AInt>()
                             ));  // clang-format on

INSTANTIATE_TEST_SUITE_P(VecAFloat_VecAInt,
                         IntrinsicTableAbstractBinaryTest,
                         testing::Values(  // clang-format off
//            result   | param lhs | param rhs |  arg lhs  |  arg rhs
Case::Create<AFloatV,    AFloatV,    AFloatV,    AFloatV,    AFloatV>(),
Case::Create<AFloatV,    AFloatV,    AFloatV,    AFloatV,    AIntV>(),
Case::Create<AFloatV,    AFloatV,    AFloatV,    AIntV,      AFloatV>(),
Case::Create<AIntV,      AIntV,      AIntV,      AIntV,      AIntV>()
                             ));  // clang-format on

INSTANTIATE_TEST_SUITE_P(AFloat_f32,
                         IntrinsicTableAbstractBinaryTest,
                         testing::Values(  // clang-format off
//            result   | param lhs | param rhs |  arg lhs  |  arg rhs
Case::Create<f32,        f32,        f32,        AFloat,     f32>(),
Case::Create<f32,        f32,        f32,        f32,        AFloat>()
                             ));  // clang-format on

INSTANTIATE_TEST_SUITE_P(VecAFloat_Vecf32,
                         IntrinsicTableAbstractBinaryTest,
                         testing::Values(  // clang-format off
//            result   | param lhs | param rhs |  arg lhs  |  arg rhs
Case::Create<f32V,       f32V,       f32V,       AFloatV,    f32V>(),
Case::Create<f32V,       f32V,       f32V,       f32V,       AFloatV>()
                             ));  // clang-format on

INSTANTIATE_TEST_SUITE_P(AFloat_i32,
                         IntrinsicTableAbstractBinaryTest,
                         testing::Values(  // clang-format off
//            result   | param lhs | param rhs |  arg lhs  |  arg rhs
Case::Create<void,        void,        void,        AFloat,     i32>(false),
Case::Create<void,        void,        void,        i32,        AFloat>(false)
                             ));  // clang-format on

INSTANTIATE_TEST_SUITE_P(VecAFloat_Veci32,
                         IntrinsicTableAbstractBinaryTest,
                         testing::Values(  // clang-format off
//            result   | param lhs | param rhs |  arg lhs  |  arg rhs
Case::Create<void,        void,        void,        AFloatV,    i32V>(false),
Case::Create<void,        void,        void,        i32V,       AFloatV>(false)
                             ));  // clang-format on

INSTANTIATE_TEST_SUITE_P(AFloat_u32,
                         IntrinsicTableAbstractBinaryTest,
                         testing::Values(  // clang-format off
//            result   | param lhs | param rhs |  arg lhs  |  arg rhs
Case::Create<void,        void,        void,        AFloat,     u32>(false),
Case::Create<void,        void,        void,        u32,        AFloat>(false)
                             ));  // clang-format on

INSTANTIATE_TEST_SUITE_P(VecAFloat_Vecu32,
                         IntrinsicTableAbstractBinaryTest,
                         testing::Values(  // clang-format off
//            result   | param lhs | param rhs |  arg lhs  |  arg rhs
Case::Create<void,        void,        void,        AFloatV,    u32V>(false),
Case::Create<void,        void,        void,        u32V,       AFloatV>(false)
                             ));  // clang-format on

INSTANTIATE_TEST_SUITE_P(AInt_f32,
                         IntrinsicTableAbstractBinaryTest,
                         testing::Values(  // clang-format off
//            result   | param lhs | param rhs |  arg lhs  |  arg rhs
Case::Create<f32,        f32,        f32,        AInt,       f32>(),
Case::Create<f32,        f32,        f32,        f32,        AInt>()
                             ));  // clang-format on

INSTANTIATE_TEST_SUITE_P(VecAInt_Vecf32,
                         IntrinsicTableAbstractBinaryTest,
                         testing::Values(  // clang-format off
//            result   | param lhs | param rhs |  arg lhs  |  arg rhs
Case::Create<f32V,       f32V,       f32V,       AIntV,      f32V>(),
Case::Create<f32V,       f32V,       f32V,       f32V,       AIntV>()
                             ));  // clang-format on

INSTANTIATE_TEST_SUITE_P(AInt_i32,
                         IntrinsicTableAbstractBinaryTest,
                         testing::Values(  // clang-format off
//            result   | param lhs | param rhs |  arg lhs  |  arg rhs
Case::Create<i32,        i32,        i32,        AInt,       i32>(),
Case::Create<i32,        i32,        i32,        i32,        AInt>()
                             ));  // clang-format on

INSTANTIATE_TEST_SUITE_P(VecAInt_Veci32,
                         IntrinsicTableAbstractBinaryTest,
                         testing::Values(  // clang-format off
//            result   | param lhs | param rhs |  arg lhs  |  arg rhs
Case::Create<i32V,       i32V,       i32V,       AIntV,      i32V>(),
Case::Create<i32V,       i32V,       i32V,       i32V,       AIntV>()
                             ));  // clang-format on

INSTANTIATE_TEST_SUITE_P(AInt_u32,
                         IntrinsicTableAbstractBinaryTest,
                         testing::Values(  // clang-format off
//            result   | param lhs | param rhs |  arg lhs  |  arg rhs
Case::Create<u32,        u32,        u32,        AInt,       u32>(),
Case::Create<u32,        u32,        u32,        u32,        AInt>()
                             ));  // clang-format on

INSTANTIATE_TEST_SUITE_P(VecAInt_Vecu32,
                         IntrinsicTableAbstractBinaryTest,
                         testing::Values(  // clang-format off
//            result   | param lhs | param rhs |  arg lhs  |  arg rhs
Case::Create<u32V,       u32V,       u32V,       AIntV,      u32V>(),
Case::Create<u32V,       u32V,       u32V,       u32V,       AIntV>()
                             ));  // clang-format on

}  // namespace AbstractBinaryTests

////////////////////////////////////////////////////////////////////////////////
// AbstractTernaryTests
////////////////////////////////////////////////////////////////////////////////
namespace AbstractTernaryTests {
struct Case {
    template <typename RESULT,
              typename PARAM_A,
              typename PARAM_B,
              typename PARAM_C,
              typename ARG_A,
              typename ARG_B,
              typename ARG_C>
    static Case Create(bool match = true) {
        return {
            match,
            resolver::builder::DataType<RESULT>::Sem,   //
            resolver::builder::DataType<PARAM_A>::Sem,  //
            resolver::builder::DataType<PARAM_B>::Sem,  //
            resolver::builder::DataType<PARAM_C>::Sem,  //
            resolver::builder::DataType<ARG_A>::Sem,    //
            resolver::builder::DataType<ARG_B>::Sem,    //
            resolver::builder::DataType<ARG_C>::Sem,    //
        };
    }
    bool expected_match;
    resolver::builder::sem_type_func_ptr expected_result;
    resolver::builder::sem_type_func_ptr expected_param_a;
    resolver::builder::sem_type_func_ptr expected_param_b;
    resolver::builder::sem_type_func_ptr expected_param_c;
    resolver::builder::sem_type_func_ptr arg_a;
    resolver::builder::sem_type_func_ptr arg_b;
    resolver::builder::sem_type_func_ptr arg_c;
};

struct IntrinsicTableAbstractTernaryTest : public resolver::ResolverTestWithParam<Case> {
    core::intrinsic::Table<Dialect> table{Types(), Symbols()};
};

TEST_P(IntrinsicTableAbstractTernaryTest, MatchClamp) {
    auto* arg_a = GetParam().arg_a(*this);
    auto* arg_b = GetParam().arg_b(*this);
    auto* arg_c = GetParam().arg_c(*this);
    auto builtin = table.Lookup(wgsl::BuiltinFn::kClamp, Empty, Vector{arg_a, arg_b, arg_c},
                                core::EvaluationStage::kConstant);

    bool expected_match = GetParam().expected_match;
    EXPECT_EQ(builtin == Success, expected_match) << Diagnostics();

    auto* result = builtin == Success ? builtin->return_type : nullptr;
    auto* expected_result = GetParam().expected_result(*this);
    EXPECT_TYPE(result, expected_result);

    auto* param_a = builtin == Success ? builtin->parameters[0].type : nullptr;
    auto* expected_param_a = GetParam().expected_param_a(*this);
    EXPECT_TYPE(param_a, expected_param_a);

    auto* param_b = builtin == Success ? builtin->parameters[1].type : nullptr;
    auto* expected_param_b = GetParam().expected_param_b(*this);
    EXPECT_TYPE(param_b, expected_param_b);

    auto* param_c = builtin == Success ? builtin->parameters[2].type : nullptr;
    auto* expected_param_c = GetParam().expected_param_c(*this);
    EXPECT_TYPE(param_c, expected_param_c);
}

INSTANTIATE_TEST_SUITE_P(
    AFloat_AInt,
    IntrinsicTableAbstractTernaryTest,
    testing::Values(  // clang-format off
//           result  | param a | param b | param c |  arg a  |  arg b  |  arg c
Case::Create<AFloat,    AFloat,  AFloat,   AFloat,   AFloat,   AFloat,   AFloat>(),
Case::Create<AFloat,    AFloat,  AFloat,   AFloat,   AFloat,   AFloat,   AInt>(),
Case::Create<AFloat,    AFloat,  AFloat,   AFloat,   AFloat,   AInt,     AFloat>(),
Case::Create<AFloat,    AFloat,  AFloat,   AFloat,   AFloat,   AInt,     AInt>(),
Case::Create<AFloat,    AFloat,  AFloat,   AFloat,   AInt,     AFloat,   AFloat>(),
Case::Create<AFloat,    AFloat,  AFloat,   AFloat,   AInt,     AFloat,   AInt>(),
Case::Create<AFloat,    AFloat,  AFloat,   AFloat,   AInt,     AInt,     AFloat>(),
Case::Create<AInt,      AInt,    AInt,     AInt,     AInt,     AInt,     AInt>()
        // clang-format on
        ));

INSTANTIATE_TEST_SUITE_P(
    VecAFloat_VecAInt,
    IntrinsicTableAbstractTernaryTest,
    testing::Values(  // clang-format off
//           result  | param a | param b | param c |  arg a  |  arg b  |  arg c
Case::Create<AFloatV,  AFloatV,  AFloatV,  AFloatV,  AFloatV,  AFloatV,  AFloatV>(),
Case::Create<AFloatV,  AFloatV,  AFloatV,  AFloatV,  AFloatV,  AFloatV,  AIntV>(),
Case::Create<AFloatV,  AFloatV,  AFloatV,  AFloatV,  AFloatV,  AIntV,    AFloatV>(),
Case::Create<AFloatV,  AFloatV,  AFloatV,  AFloatV,  AFloatV,  AIntV,    AIntV>(),
Case::Create<AFloatV,  AFloatV,  AFloatV,  AFloatV,  AIntV,    AFloatV,  AFloatV>(),
Case::Create<AFloatV,  AFloatV,  AFloatV,  AFloatV,  AIntV,    AFloatV,  AIntV>(),
Case::Create<AFloatV,  AFloatV,  AFloatV,  AFloatV,  AIntV,    AIntV,    AFloatV>(),
Case::Create<AIntV,    AIntV,    AIntV,    AIntV,    AIntV,    AIntV,    AIntV>()
        // clang-format on
        ));

INSTANTIATE_TEST_SUITE_P(
    AFloat_f32,
    IntrinsicTableAbstractTernaryTest,
    testing::Values(  // clang-format off
//           result  | param a | param b | param c |  arg a  |  arg b  |  arg c
Case::Create<f32,      f32,      f32,      f32,      AFloat,   AFloat,   f32>(),
Case::Create<f32,      f32,      f32,      f32,      AFloat,   f32,      AFloat>(),
Case::Create<f32,      f32,      f32,      f32,      AFloat,   f32,      f32>(),
Case::Create<f32,      f32,      f32,      f32,      f32,      AFloat,   AFloat>(),
Case::Create<f32,      f32,      f32,      f32,      f32,      AFloat,   f32>(),
Case::Create<f32,      f32,      f32,      f32,      f32,      f32,      AFloat>()
        // clang-format on
        ));

INSTANTIATE_TEST_SUITE_P(
    VecAFloat_Vecf32,
    IntrinsicTableAbstractTernaryTest,
    testing::Values(  // clang-format off
//           result  | param a | param b | param c |  arg a  |  arg b  |  arg c
Case::Create<f32V,     f32V,     f32V,     f32V,     AFloatV,  AFloatV,  f32V>(),
Case::Create<f32V,     f32V,     f32V,     f32V,     AFloatV,  f32V,     AFloatV>(),
Case::Create<f32V,     f32V,     f32V,     f32V,     AFloatV,  f32V,     f32V>(),
Case::Create<f32V,     f32V,     f32V,     f32V,     f32V,     AFloatV,  AFloatV>(),
Case::Create<f32V,     f32V,     f32V,     f32V,     f32V,     AFloatV,  f32V>(),
Case::Create<f32V,     f32V,     f32V,     f32V,     f32V,     f32V,     AFloatV> ()
        // clang-format on
        ));

INSTANTIATE_TEST_SUITE_P(
    AFloat_i32,
    IntrinsicTableAbstractTernaryTest,
    testing::Values(  // clang-format off
//           result  | param a | param b | param c |  arg a  |  arg b  |  arg c
Case::Create<void,     void,     void,     void,     AFloat,   AFloat,   i32>(false),
Case::Create<void,     void,     void,     void,     AFloat,   i32,      AFloat>(false),
Case::Create<void,     void,     void,     void,     AFloat,   i32,      i32>(false),
Case::Create<void,     void,     void,     void,     i32,      AFloat,   AFloat>(false),
Case::Create<void,     void,     void,     void,     i32,      AFloat,   i32>(false),
Case::Create<void,     void,     void,     void,     i32,      i32,      AFloat>(false)
        // clang-format on
        ));

INSTANTIATE_TEST_SUITE_P(
    VecAFloat_Veci32,
    IntrinsicTableAbstractTernaryTest,
    testing::Values(  // clang-format off
//           result  | param a | param b | param c |  arg a  |  arg b  |  arg c
Case::Create<void,     void,     void,     void,     AFloatV,  AFloatV,  i32V>(false),
Case::Create<void,     void,     void,     void,     AFloatV,  i32V,     AFloatV>(false),
Case::Create<void,     void,     void,     void,     AFloatV,  i32V,     i32V>(false),
Case::Create<void,     void,     void,     void,     i32V,     AFloatV,  AFloatV>(false),
Case::Create<void,     void,     void,     void,     i32V,     AFloatV,  i32V>(false),
Case::Create<void,     void,     void,     void,     i32V,     i32V,     AFloatV>(false)
        // clang-format on
        ));

INSTANTIATE_TEST_SUITE_P(
    AFloat_u32,
    IntrinsicTableAbstractTernaryTest,
    testing::Values(  // clang-format off
//           result  | param a | param b | param c |  arg a  |  arg b  |  arg c
Case::Create<void,     void,     void,     void,     AFloat,   AFloat,   u32>(false),
Case::Create<void,     void,     void,     void,     AFloat,   u32,      AFloat>(false),
Case::Create<void,     void,     void,     void,     AFloat,   u32,      u32>(false),
Case::Create<void,     void,     void,     void,     u32,      AFloat,   AFloat>(false),
Case::Create<void,     void,     void,     void,     u32,      AFloat,   u32>(false),
Case::Create<void,     void,     void,     void,     u32,      u32,      AFloat>(false)
        // clang-format on
        ));

INSTANTIATE_TEST_SUITE_P(
    VecAFloat_Vecu32,
    IntrinsicTableAbstractTernaryTest,
    testing::Values(  // clang-format off
//           result  | param a | param b | param c |  arg a  |  arg b  |  arg c
Case::Create<void,     void,     void,     void,     AFloatV,  AFloatV,  u32V>(false),
Case::Create<void,     void,     void,     void,     AFloatV,  u32V,     AFloatV>(false),
Case::Create<void,     void,     void,     void,     AFloatV,  u32V,     u32V>(false),
Case::Create<void,     void,     void,     void,     u32V,     AFloatV,  AFloatV>(false),
Case::Create<void,     void,     void,     void,     u32V,     AFloatV,  u32V>(false),
Case::Create<void,     void,     void,     void,     u32V,     u32V,     AFloatV>(false)
        // clang-format on
        ));

INSTANTIATE_TEST_SUITE_P(
    AInt_f32,
    IntrinsicTableAbstractTernaryTest,
    testing::Values(  // clang-format off
//           result  | param a | param b | param c |  arg a  |  arg b  |  arg c
Case::Create<f32,      f32,      f32,      f32,      AInt,     AInt,     f32>(),
Case::Create<f32,      f32,      f32,      f32,      AInt,     f32,      AInt>(),
Case::Create<f32,      f32,      f32,      f32,      AInt,     f32,      f32>(),
Case::Create<f32,      f32,      f32,      f32,      f32,      AInt,     AInt>(),
Case::Create<f32,      f32,      f32,      f32,      f32,      AInt,     f32>(),
Case::Create<f32,      f32,      f32,      f32,      f32,      f32,      AInt>()
        // clang-format on
        ));

INSTANTIATE_TEST_SUITE_P(
    VecAInt_Vecf32,
    IntrinsicTableAbstractTernaryTest,
    testing::Values(  // clang-format off
//           result  | param a | param b | param c |  arg a  |  arg b  |  arg c
Case::Create<f32V,     f32V,     f32V,     f32V,     AIntV,    AIntV,    f32V>(),
Case::Create<f32V,     f32V,     f32V,     f32V,     AIntV,    f32V,     AIntV>(),
Case::Create<f32V,     f32V,     f32V,     f32V,     AIntV,    f32V,     f32V>(),
Case::Create<f32V,     f32V,     f32V,     f32V,     f32V,     AIntV,    AIntV>(),
Case::Create<f32V,     f32V,     f32V,     f32V,     f32V,     AIntV,    f32V>(),
Case::Create<f32V,     f32V,     f32V,     f32V,     f32V,     f32V,     AIntV>()
        // clang-format on
        ));

INSTANTIATE_TEST_SUITE_P(
    AInt_i32,
    IntrinsicTableAbstractTernaryTest,
    testing::Values(  // clang-format off
//           result  | param a | param b | param c |  arg a  |  arg b  |  arg c
Case::Create<i32,      i32,      i32,      i32,      AInt,     AInt,     i32>(),
Case::Create<i32,      i32,      i32,      i32,      AInt,     i32,      AInt>(),
Case::Create<i32,      i32,      i32,      i32,      AInt,     i32,      i32>(),
Case::Create<i32,      i32,      i32,      i32,      i32,      AInt,     AInt>(),
Case::Create<i32,      i32,      i32,      i32,      i32,      AInt,     i32>(),
Case::Create<i32,      i32,      i32,      i32,      i32,      i32,      AInt>()
        // clang-format on
        ));

INSTANTIATE_TEST_SUITE_P(
    VecAInt_Veci32,
    IntrinsicTableAbstractTernaryTest,
    testing::Values(  // clang-format off
//           result  | param a | param b | param c |  arg a  |  arg b  |  arg c
Case::Create<i32V,     i32V,     i32V,     i32V,     AIntV,    AIntV,     i32V>(),
Case::Create<i32V,     i32V,     i32V,     i32V,     AIntV,    i32V,      AIntV>(),
Case::Create<i32V,     i32V,     i32V,     i32V,     AIntV,    i32V,      i32V>(),
Case::Create<i32V,     i32V,     i32V,     i32V,     i32V,     AIntV,     AIntV>(),
Case::Create<i32V,     i32V,     i32V,     i32V,     i32V,     AIntV,     i32V>(),
Case::Create<i32V,     i32V,     i32V,     i32V,     i32V,     i32V,      AIntV>()
        // clang-format on
        ));

INSTANTIATE_TEST_SUITE_P(
    AInt_u32,
    IntrinsicTableAbstractTernaryTest,
    testing::Values(  // clang-format off
//           result  | param a | param b | param c |  arg a  |  arg b  |  arg c
Case::Create<u32,      u32,      u32,      u32,      AInt,     AInt,     u32>(),
Case::Create<u32,      u32,      u32,      u32,      AInt,     u32,      AInt>(),
Case::Create<u32,      u32,      u32,      u32,      AInt,     u32,      u32>(),
Case::Create<u32,      u32,      u32,      u32,      u32,      AInt,     AInt>(),
Case::Create<u32,      u32,      u32,      u32,      u32,      AInt,     u32>(),
Case::Create<u32,      u32,      u32,      u32,      u32,      u32,      AInt>()
        // clang-format on
        ));

INSTANTIATE_TEST_SUITE_P(
    VecAInt_Vecu32,
    IntrinsicTableAbstractTernaryTest,
    testing::Values(  // clang-format off
//           result  | param a | param b | param c |  arg a  |  arg b  |  arg c
Case::Create<u32V,     u32V,     u32V,     u32V,     AIntV,    AIntV,    u32V>(),
Case::Create<u32V,     u32V,     u32V,     u32V,     AIntV,    u32V,     AIntV>(),
Case::Create<u32V,     u32V,     u32V,     u32V,     AIntV,    u32V,     u32V>(),
Case::Create<u32V,     u32V,     u32V,     u32V,     u32V,     AIntV,    AIntV>(),
Case::Create<u32V,     u32V,     u32V,     u32V,     u32V,     AIntV,    u32V>(),
Case::Create<u32V,     u32V,     u32V,     u32V,     u32V,     u32V,     AIntV>()
        // clang-format on
        ));

}  // namespace AbstractTernaryTests

}  // namespace
}  // namespace tint::wgsl::intrinsic

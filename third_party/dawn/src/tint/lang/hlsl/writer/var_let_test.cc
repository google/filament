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

#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/sampler.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/lang/hlsl/writer/helper_test.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer {
namespace {

TEST_F(HlslWriterTest, Var) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        b.Var("a", 1_u);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void main() {
  uint a = 1u;
}

)");
}

TEST_F(HlslWriterTest, VarZeroInit) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        b.Var("a", function, ty.f32());
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void main() {
  float a = 0.0f;
}

)");
}

TEST_F(HlslWriterTest, Let) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        b.Let("a", 2_f);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void main() {
  float a = 2.0f;
}

)");
}

TEST_F(HlslWriterTest, VarSampler) {
    auto* s = b.Var("s", ty.ptr<handle>(ty.sampler()));
    s->SetBindingPoint(1, 0);

    b.ir.root_block->Append(s);

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
SamplerState s : register(s0, space1);
[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, VarSamplerComparison) {
    auto* s = b.Var("s", ty.ptr<handle>(ty.comparison_sampler()));
    s->SetBindingPoint(0, 0);

    b.ir.root_block->Append(s);

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
SamplerComparisonState s : register(s0);
[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

struct HlslDepthTextureData {
    core::type::TextureDimension dim;
    std::string result;
};
inline std::ostream& operator<<(std::ostream& out, HlslDepthTextureData data) {
    StringStream str;
    str << data.dim;
    out << str.str();
    return out;
}

using VarDepthTextureTest = HlslWriterTestWithParam<HlslDepthTextureData>;
TEST_P(VarDepthTextureTest, Emit) {
    auto params = GetParam();

    auto* s = b.Var("tex", ty.ptr<handle>(ty.Get<core::type::DepthTexture>(params.dim)));
    s->SetBindingPoint(2, 1);

    b.ir.root_block->Append(s);

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, "\n" + params.result + R"(
[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}
INSTANTIATE_TEST_SUITE_P(
    HlslWriterTest,
    VarDepthTextureTest,
    testing::Values(HlslDepthTextureData{core::type::TextureDimension::k2d,
                                         "Texture2D tex : register(t1, space2);"},
                    HlslDepthTextureData{core::type::TextureDimension::k2dArray,
                                         "Texture2DArray tex : register(t1, space2);"},
                    HlslDepthTextureData{core::type::TextureDimension::kCube,
                                         "TextureCube tex : register(t1, space2);"},
                    HlslDepthTextureData{core::type::TextureDimension::kCubeArray,
                                         "TextureCubeArray tex : register(t1, space2);"}));

TEST_F(HlslWriterTest, VarDepthMultiSampled) {
    auto* s = b.Var("tex", ty.ptr<handle>(ty.Get<core::type::DepthMultisampledTexture>(
                               core::type::TextureDimension::k2d)));
    s->SetBindingPoint(2, 1);

    b.ir.root_block->Append(s);

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2DMS<float4> tex : register(t1, space2);
[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

enum class TextureDataType : uint8_t { F32, U32, I32 };
struct HlslSampledTextureData {
    core::type::TextureDimension dim;
    TextureDataType datatype;
    std::string result;
};

inline std::ostream& operator<<(std::ostream& out, HlslSampledTextureData data) {
    StringStream str;
    str << data.dim;
    out << str.str();
    return out;
}

using VarSampledTextureTest = HlslWriterTestWithParam<HlslSampledTextureData>;
TEST_P(VarSampledTextureTest, Emit) {
    auto params = GetParam();

    const core::type::Type* datatype;
    switch (params.datatype) {
        case TextureDataType::F32:
            datatype = ty.f32();
            break;
        case TextureDataType::U32:
            datatype = ty.u32();
            break;
        case TextureDataType::I32:
            datatype = ty.i32();
            break;
    }

    auto* s =
        b.Var("tex", ty.ptr<handle>(ty.Get<core::type::SampledTexture>(params.dim, datatype)));
    s->SetBindingPoint(2, 1);

    b.ir.root_block->Append(s);

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, "\n" + params.result + R"(
[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

INSTANTIATE_TEST_SUITE_P(HlslWriterTest,
                         VarSampledTextureTest,
                         testing::Values(
                             HlslSampledTextureData{
                                 core::type::TextureDimension::k1d,
                                 TextureDataType::F32,
                                 "Texture1D<float4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 core::type::TextureDimension::k2d,
                                 TextureDataType::F32,
                                 "Texture2D<float4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 core::type::TextureDimension::k2dArray,
                                 TextureDataType::F32,
                                 "Texture2DArray<float4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 core::type::TextureDimension::k3d,
                                 TextureDataType::F32,
                                 "Texture3D<float4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 core::type::TextureDimension::kCube,
                                 TextureDataType::F32,
                                 "TextureCube<float4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 core::type::TextureDimension::kCubeArray,
                                 TextureDataType::F32,
                                 "TextureCubeArray<float4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 core::type::TextureDimension::k1d,
                                 TextureDataType::U32,
                                 "Texture1D<uint4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 core::type::TextureDimension::k2d,
                                 TextureDataType::U32,
                                 "Texture2D<uint4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 core::type::TextureDimension::k2dArray,
                                 TextureDataType::U32,
                                 "Texture2DArray<uint4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 core::type::TextureDimension::k3d,
                                 TextureDataType::U32,
                                 "Texture3D<uint4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 core::type::TextureDimension::kCube,
                                 TextureDataType::U32,
                                 "TextureCube<uint4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 core::type::TextureDimension::kCubeArray,
                                 TextureDataType::U32,
                                 "TextureCubeArray<uint4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 core::type::TextureDimension::k1d,
                                 TextureDataType::I32,
                                 "Texture1D<int4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 core::type::TextureDimension::k2d,
                                 TextureDataType::I32,
                                 "Texture2D<int4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 core::type::TextureDimension::k2dArray,
                                 TextureDataType::I32,
                                 "Texture2DArray<int4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 core::type::TextureDimension::k3d,
                                 TextureDataType::I32,
                                 "Texture3D<int4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 core::type::TextureDimension::kCube,
                                 TextureDataType::I32,
                                 "TextureCube<int4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 core::type::TextureDimension::kCubeArray,
                                 TextureDataType::I32,
                                 "TextureCubeArray<int4> tex : register(t1, space2);",
                             }));

TEST_F(HlslWriterTest, VarMultisampledTexture) {
    auto* s = b.Var("tex", ty.ptr<handle>(ty.Get<core::type::MultisampledTexture>(
                               core::type::TextureDimension::k2d, ty.f32())));
    s->SetBindingPoint(2, 1);

    b.ir.root_block->Append(s);

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2DMS<float4> tex : register(t1, space2);
[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

struct HlslStorageTextureData {
    core::type::TextureDimension dim;
    core::TexelFormat imgfmt;
    core::Access access;
    std::string result;
};

inline std::ostream& operator<<(std::ostream& out, HlslStorageTextureData data) {
    StringStream str;
    str << data.dim;
    out << str.str();
    return out;
}

using VarStorageTextureTest = HlslWriterTestWithParam<HlslStorageTextureData>;
TEST_P(VarStorageTextureTest, Emit) {
    auto params = GetParam();

    auto* s = b.Var("tex", ty.ptr<handle>(ty.Get<core::type::StorageTexture>(
                               params.dim, params.imgfmt, params.access, ty.f32())));
    s->SetBindingPoint(2, 1);

    b.ir.root_block->Append(s);

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, "\n" + params.result + R"(
[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

INSTANTIATE_TEST_SUITE_P(
    HlslWriterTest,
    VarStorageTextureTest,
    testing::Values(
        HlslStorageTextureData{core::type::TextureDimension::k1d, core::TexelFormat::kRgba8Unorm,
                               core::Access::kWrite,
                               "RWTexture1D<float4> tex : register(u1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k2d, core::TexelFormat::kRgba16Float,
                               core::Access::kWrite,
                               "RWTexture2D<float4> tex : register(u1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k2dArray, core::TexelFormat::kR32Float,
                               core::Access::kWrite,
                               "RWTexture2DArray<float4> tex : register(u1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k3d, core::TexelFormat::kRg32Float,
                               core::Access::kWrite,
                               "RWTexture3D<float4> tex : register(u1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k1d, core::TexelFormat::kRgba32Float,
                               core::Access::kWrite,
                               "RWTexture1D<float4> tex : register(u1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k2d, core::TexelFormat::kRgba16Uint,
                               core::Access::kWrite,
                               "RWTexture2D<uint4> tex : register(u1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k2dArray, core::TexelFormat::kR32Uint,
                               core::Access::kWrite,
                               "RWTexture2DArray<uint4> tex : register(u1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k3d, core::TexelFormat::kRg32Uint,
                               core::Access::kWrite,
                               "RWTexture3D<uint4> tex : register(u1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k1d, core::TexelFormat::kRgba32Uint,
                               core::Access::kWrite,
                               "RWTexture1D<uint4> tex : register(u1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k2d, core::TexelFormat::kRgba16Sint,
                               core::Access::kWrite,
                               "RWTexture2D<int4> tex : register(u1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k2dArray, core::TexelFormat::kR32Sint,
                               core::Access::kWrite,
                               "RWTexture2DArray<int4> tex : register(u1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k3d, core::TexelFormat::kRg32Sint,
                               core::Access::kWrite,
                               "RWTexture3D<int4> tex : register(u1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k1d, core::TexelFormat::kRgba32Sint,
                               core::Access::kWrite,
                               "RWTexture1D<int4> tex : register(u1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k1d, core::TexelFormat::kRgba8Unorm,
                               core::Access::kRead,
                               "Texture1D<float4> tex : register(t1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k2d, core::TexelFormat::kRgba16Float,
                               core::Access::kRead,
                               "Texture2D<float4> tex : register(t1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k2dArray, core::TexelFormat::kR32Float,
                               core::Access::kRead,
                               "Texture2DArray<float4> tex : register(t1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k3d, core::TexelFormat::kRg32Float,
                               core::Access::kRead,
                               "Texture3D<float4> tex : register(t1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k1d, core::TexelFormat::kRgba32Float,
                               core::Access::kRead,
                               "Texture1D<float4> tex : register(t1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k2d, core::TexelFormat::kRgba16Uint,
                               core::Access::kRead, "Texture2D<uint4> tex : register(t1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k2dArray, core::TexelFormat::kR32Uint,
                               core::Access::kRead,
                               "Texture2DArray<uint4> tex : register(t1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k3d, core::TexelFormat::kRg32Uint,
                               core::Access::kRead, "Texture3D<uint4> tex : register(t1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k1d, core::TexelFormat::kRgba32Uint,
                               core::Access::kRead, "Texture1D<uint4> tex : register(t1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k2d, core::TexelFormat::kRgba16Sint,
                               core::Access::kRead, "Texture2D<int4> tex : register(t1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k2dArray, core::TexelFormat::kR32Sint,
                               core::Access::kRead,
                               "Texture2DArray<int4> tex : register(t1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k3d, core::TexelFormat::kRg32Sint,
                               core::Access::kRead, "Texture3D<int4> tex : register(t1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k1d, core::TexelFormat::kRgba32Sint,
                               core::Access::kRead, "Texture1D<int4> tex : register(t1, space2);"}

        ));

TEST_F(HlslWriterTest, VarUniform) {
    auto* s = b.Var("u", ty.ptr<uniform>(ty.vec4<f32>()));
    s->SetBindingPoint(2, 1);

    b.ir.root_block->Append(s);

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
cbuffer cbuffer_u : register(b1, space2) {
  uint4 u[1];
};
[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, VarStorageRead) {
    auto* s = b.Var("u", ty.ptr<storage, core::Access::kRead>(ty.vec4<f32>()));
    s->SetBindingPoint(2, 1);

    b.ir.root_block->Append(s);

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
ByteAddressBuffer u : register(t1, space2);
[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, VarStorageReadWrite) {
    auto* s = b.Var("u", ty.ptr<storage, core::Access::kReadWrite>(ty.vec4<f32>()));
    s->SetBindingPoint(2, 1);

    b.ir.root_block->Append(s);

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer u : register(u1, space2);
[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, VarPrivate) {
    auto* s = b.Var("u", ty.ptr<private_>(ty.vec4<f32>()));

    b.ir.root_block->Append(s);

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
static float4 u = (0.0f).xxxx;
[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, VarWorkgroup) {
    auto* s = b.Var("u", ty.ptr<workgroup>(ty.vec4<f32>()));

    b.ir.root_block->Append(s);

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
groupshared float4 u;
[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

}  // namespace
}  // namespace tint::hlsl::writer

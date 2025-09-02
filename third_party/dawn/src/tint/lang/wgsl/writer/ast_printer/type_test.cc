// Copyright 2020 The Dawn & Tint Authors
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

#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/wgsl/writer/ast_printer/helper_test.h"
#include "src/tint/utils/text/string_stream.h"

#include "gmock/gmock.h"

namespace tint::wgsl::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using WgslASTPrinterTest = TestHelper;

TEST_F(WgslASTPrinterTest, EmitType_Alias) {
    auto* alias = Alias("alias", ty.f32());
    auto type = Alias("make_type_reachable", ty.Of(alias))->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "alias");
}

TEST_F(WgslASTPrinterTest, EmitType_Array) {
    auto type = Alias("make_type_reachable", ty.array<bool, 4u>())->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "array<bool, 4u>");
}

TEST_F(WgslASTPrinterTest, EmitType_Array_Attribute) {
    auto type = Alias("make_type_reachable", ty.array(ty.bool_(), 4_u, Vector{Stride(16)}))->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "@stride(16) array<bool, 4u>");
}

TEST_F(WgslASTPrinterTest, EmitType_RuntimeArray) {
    auto type = Alias("make_type_reachable", ty.array(ty.bool_()))->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "array<bool>");
}

TEST_F(WgslASTPrinterTest, EmitType_Bool) {
    auto type = Alias("make_type_reachable", ty.bool_())->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "bool");
}

TEST_F(WgslASTPrinterTest, EmitType_F32) {
    auto type = Alias("make_type_reachable", ty.f32())->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "f32");
}

TEST_F(WgslASTPrinterTest, EmitType_F16) {
    Enable(wgsl::Extension::kF16);

    auto type = Alias("make_type_reachable", ty.f16())->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "f16");
}

TEST_F(WgslASTPrinterTest, EmitType_I32) {
    auto type = Alias("make_type_reachable", ty.i32())->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "i32");
}

TEST_F(WgslASTPrinterTest, EmitType_Matrix_F32) {
    auto type = Alias("make_type_reachable", ty.mat2x3<f32>())->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "mat2x3<f32>");
}

TEST_F(WgslASTPrinterTest, EmitType_Matrix_F16) {
    Enable(wgsl::Extension::kF16);

    auto type = Alias("make_type_reachable", ty.mat2x3<f16>())->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "mat2x3<f16>");
}

TEST_F(WgslASTPrinterTest, EmitType_Pointer) {
    auto type = Alias("make_type_reachable", ty.ptr<workgroup, f32>())->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "ptr<workgroup, f32>");
}

TEST_F(WgslASTPrinterTest, EmitType_PointerAccessMode) {
    auto type = Alias("make_type_reachable", ty.ptr<storage, f32, read_write>())->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "ptr<storage, f32, read_write>");
}

TEST_F(WgslASTPrinterTest, EmitType_Struct) {
    auto* s = Structure("S", Vector{
                                 Member("a", ty.i32()),
                                 Member("b", ty.f32()),
                             });
    auto type = Alias("make_reachable", ty.Of(s))->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "S");
}

TEST_F(WgslASTPrinterTest, EmitType_StructOffsetDecl) {
    auto* s = Structure("S", Vector{
                                 Member("a", ty.i32(), Vector{MemberOffset(8_a)}),
                                 Member("b", ty.f32(), Vector{MemberOffset(16_a)}),
                             });

    ASTPrinter& gen = Build();

    gen.EmitStructType(s);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(struct S {
  @size(8)
  padding_0 : u32,
  /* @offset(8) */
  a : i32,
  @size(4)
  padding_1 : u32,
  /* @offset(16) */
  b : f32,
}
)");
}

TEST_F(WgslASTPrinterTest, EmitType_StructOffsetDecl_ExceedStaticVectorSize) {
    auto* s = Structure("S", Vector{
                                 Member("a", ty.i32(), Vector{MemberOffset(i32(8 * 0))}),
                                 Member("b", ty.i32(), Vector{MemberOffset(i32(8 * 1))}),
                                 Member("c", ty.i32(), Vector{MemberOffset(i32(8 * 2))}),
                                 Member("d", ty.i32(), Vector{MemberOffset(i32(8 * 3))}),
                                 Member("e", ty.i32(), Vector{MemberOffset(i32(8 * 4))}),
                                 Member("f", ty.i32(), Vector{MemberOffset(i32(8 * 5))}),
                                 Member("g", ty.i32(), Vector{MemberOffset(i32(8 * 6))}),
                                 Member("h", ty.i32(), Vector{MemberOffset(i32(8 * 7))}),
                                 Member("i", ty.i32(), Vector{MemberOffset(i32(8 * 8))}),
                                 Member("j", ty.i32(), Vector{MemberOffset(i32(8 * 9))}),
                             });

    ASTPrinter& gen = Build();

    gen.EmitStructType(s);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(struct S {
  /* @offset(0i) */
  a : i32,
  @size(4)
  padding_0 : u32,
  /* @offset(8i) */
  b : i32,
  @size(4)
  padding_1 : u32,
  /* @offset(16i) */
  c : i32,
  @size(4)
  padding_2 : u32,
  /* @offset(24i) */
  d : i32,
  @size(4)
  padding_3 : u32,
  /* @offset(32i) */
  e : i32,
  @size(4)
  padding_4 : u32,
  /* @offset(40i) */
  f : i32,
  @size(4)
  padding_5 : u32,
  /* @offset(48i) */
  g : i32,
  @size(4)
  padding_6 : u32,
  /* @offset(56i) */
  h : i32,
  @size(4)
  padding_7 : u32,
  /* @offset(64i) */
  i : i32,
  @size(4)
  padding_8 : u32,
  /* @offset(72i) */
  j : i32,
}
)");
}

TEST_F(WgslASTPrinterTest, EmitType_StructOffsetDecl_WithSymbolCollisions) {
    auto* s = Structure("S", Vector{
                                 Member("tint_0_padding", ty.i32(), Vector{MemberOffset(8_a)}),
                                 Member("tint_2_padding", ty.f32(), Vector{MemberOffset(16_a)}),
                             });

    ASTPrinter& gen = Build();

    gen.EmitStructType(s);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(struct S {
  @size(8)
  padding_0 : u32,
  /* @offset(8) */
  tint_0_padding : i32,
  @size(4)
  padding_1 : u32,
  /* @offset(16) */
  tint_2_padding : f32,
}
)");
}

TEST_F(WgslASTPrinterTest, EmitType_StructAlignDecl) {
    auto* s = Structure("S", Vector{
                                 Member("a", ty.i32(), Vector{MemberAlign(8_a)}),
                                 Member("b", ty.f32(), Vector{MemberAlign(16_a)}),
                             });

    ASTPrinter& gen = Build();

    gen.EmitStructType(s);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(struct S {
  @align(8)
  a : i32,
  @align(16)
  b : f32,
}
)");
}

TEST_F(WgslASTPrinterTest, EmitType_StructSizeDecl) {
    auto* s = Structure("S", Vector{
                                 Member("a", ty.i32(), Vector{MemberSize(16_a)}),
                                 Member("b", ty.f32(), Vector{MemberSize(32_a)}),
                             });

    ASTPrinter& gen = Build();

    gen.EmitStructType(s);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(struct S {
  @size(16)
  a : i32,
  @size(32)
  b : f32,
}
)");
}

TEST_F(WgslASTPrinterTest, EmitType_Struct_WithAttribute) {
    auto* s = Structure("S", Vector{
                                 Member("a", ty.i32()),
                                 Member("b", ty.f32(), Vector{MemberAlign(8_a)}),
                             });

    ASTPrinter& gen = Build();

    gen.EmitStructType(s);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(struct S {
  a : i32,
  @align(8)
  b : f32,
}
)");
}

TEST_F(WgslASTPrinterTest, EmitType_Struct_WithEntryPointAttributes) {
    auto* s =
        Structure("S", Vector{
                           Member("a", ty.u32(), Vector{Builtin(core::BuiltinValue::kVertexIndex)}),
                           Member("b", ty.f32(), Vector{Location(2_a)}),
                       });

    ASTPrinter& gen = Build();

    gen.EmitStructType(s);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(struct S {
  @builtin(vertex_index)
  a : u32,
  @location(2)
  b : f32,
}
)");
}

TEST_F(WgslASTPrinterTest, EmitType_U32) {
    auto type = Alias("make_type_reachable", ty.u32())->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "u32");
}

TEST_F(WgslASTPrinterTest, EmitType_Vector_F32) {
    auto type = Alias("make_type_reachable", ty.vec3<f32>())->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "vec3<f32>");
}

TEST_F(WgslASTPrinterTest, EmitType_Vector_F16) {
    Enable(wgsl::Extension::kF16);

    auto type = Alias("make_type_reachable", ty.vec3<f16>())->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "vec3<f16>");
}

struct TextureData {
    core::type::TextureDimension dim;
    const char* name;
};
inline std::ostream& operator<<(std::ostream& out, TextureData data) {
    out << data.name;
    return out;
}
using WgslGenerator_DepthTextureTest = TestParamHelper<TextureData>;

TEST_P(WgslGenerator_DepthTextureTest, EmitType_DepthTexture) {
    auto param = GetParam();

    auto type = Alias("make_type_reachable", ty.depth_texture(param.dim))->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), param.name);
}
INSTANTIATE_TEST_SUITE_P(
    WgslASTPrinterTest,
    WgslGenerator_DepthTextureTest,
    testing::Values(TextureData{core::type::TextureDimension::k2d, "texture_depth_2d"},
                    TextureData{core::type::TextureDimension::k2dArray, "texture_depth_2d_array"},
                    TextureData{core::type::TextureDimension::kCube, "texture_depth_cube"},
                    TextureData{core::type::TextureDimension::kCubeArray,
                                "texture_depth_cube_array"}));

using WgslGenerator_SampledTextureTest = TestParamHelper<TextureData>;
TEST_P(WgslGenerator_SampledTextureTest, EmitType_SampledTexture_F32) {
    auto param = GetParam();

    auto t = ty.sampled_texture(param.dim, ty.f32());
    auto type = Alias("make_type_reachable", t)->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), std::string(param.name) + "<f32>");
}

TEST_P(WgslGenerator_SampledTextureTest, EmitType_SampledTexture_I32) {
    auto param = GetParam();

    auto t = ty.sampled_texture(param.dim, ty.i32());
    auto type = Alias("make_type_reachable", t)->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), std::string(param.name) + "<i32>");
}

TEST_P(WgslGenerator_SampledTextureTest, EmitType_SampledTexture_U32) {
    auto param = GetParam();

    auto t = ty.sampled_texture(param.dim, ty.u32());
    auto type = Alias("make_type_reachable", t)->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), std::string(param.name) + "<u32>");
}
INSTANTIATE_TEST_SUITE_P(
    WgslASTPrinterTest,
    WgslGenerator_SampledTextureTest,
    testing::Values(TextureData{core::type::TextureDimension::k1d, "texture_1d"},
                    TextureData{core::type::TextureDimension::k2d, "texture_2d"},
                    TextureData{core::type::TextureDimension::k2dArray, "texture_2d_array"},
                    TextureData{core::type::TextureDimension::k3d, "texture_3d"},
                    TextureData{core::type::TextureDimension::kCube, "texture_cube"},
                    TextureData{core::type::TextureDimension::kCubeArray, "texture_cube_array"}));

using WgslGenerator_MultiampledTextureTest = TestParamHelper<TextureData>;
TEST_P(WgslGenerator_MultiampledTextureTest, EmitType_MultisampledTexture_F32) {
    auto param = GetParam();

    auto t = ty.multisampled_texture(param.dim, ty.f32());
    auto type = Alias("make_type_reachable", t)->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), std::string(param.name) + "<f32>");
}

TEST_P(WgslGenerator_MultiampledTextureTest, EmitType_MultisampledTexture_I32) {
    auto param = GetParam();

    auto t = ty.multisampled_texture(param.dim, ty.i32());
    auto type = Alias("make_type_reachable", t)->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), std::string(param.name) + "<i32>");
}

TEST_P(WgslGenerator_MultiampledTextureTest, EmitType_MultisampledTexture_U32) {
    auto param = GetParam();

    auto t = ty.multisampled_texture(param.dim, ty.u32());
    auto type = Alias("make_type_reachable", t)->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), std::string(param.name) + "<u32>");
}
INSTANTIATE_TEST_SUITE_P(WgslASTPrinterTest,
                         WgslGenerator_MultiampledTextureTest,
                         testing::Values(TextureData{core::type::TextureDimension::k2d,
                                                     "texture_multisampled_2d"}));

struct StorageTextureData {
    core::TexelFormat fmt;
    core::type::TextureDimension dim;
    core::Access access;
    const char* name;
};
inline std::ostream& operator<<(std::ostream& out, StorageTextureData data) {
    out << data.name;
    return out;
}
using WgslGenerator_StorageTextureTest = TestParamHelper<StorageTextureData>;
TEST_P(WgslGenerator_StorageTextureTest, EmitType_StorageTexture) {
    auto param = GetParam();

    auto s = ty.storage_texture(param.dim, param.fmt, param.access);
    auto type = GlobalVar("g", s, Binding(1_a), Group(2_a))->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), param.name);
}
INSTANTIATE_TEST_SUITE_P(
    WgslASTPrinterTest,
    WgslGenerator_StorageTextureTest,
    testing::Values(
        StorageTextureData{core::TexelFormat::kRgba8Sint, core::type::TextureDimension::k1d,
                           core::Access::kWrite, "texture_storage_1d<rgba8sint, write>"},
        StorageTextureData{core::TexelFormat::kRgba8Sint, core::type::TextureDimension::k2d,
                           core::Access::kWrite, "texture_storage_2d<rgba8sint, write>"},
        StorageTextureData{core::TexelFormat::kRgba8Sint, core::type::TextureDimension::k2dArray,
                           core::Access::kWrite, "texture_storage_2d_array<rgba8sint, write>"},
        StorageTextureData{core::TexelFormat::kRgba8Sint, core::type::TextureDimension::k3d,
                           core::Access::kWrite, "texture_storage_3d<rgba8sint, write>"}));

struct ImageFormatData {
    core::TexelFormat fmt;
    const char* name;
};
inline std::ostream& operator<<(std::ostream& out, ImageFormatData data) {
    out << data.name;
    return out;
}
using WgslGenerator_ImageFormatTest = TestParamHelper<ImageFormatData>;
TEST_P(WgslGenerator_ImageFormatTest, EmitType_StorageTexture_ImageFormat) {
    auto param = GetParam();

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitImageFormat(out, param.fmt);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), param.name);
}

INSTANTIATE_TEST_SUITE_P(
    WgslASTPrinterTest,
    WgslGenerator_ImageFormatTest,
    testing::Values(ImageFormatData{core::TexelFormat::kR32Uint, "r32uint"},
                    ImageFormatData{core::TexelFormat::kR32Sint, "r32sint"},
                    ImageFormatData{core::TexelFormat::kR32Float, "r32float"},
                    ImageFormatData{core::TexelFormat::kRgba8Unorm, "rgba8unorm"},
                    ImageFormatData{core::TexelFormat::kRgba8Snorm, "rgba8snorm"},
                    ImageFormatData{core::TexelFormat::kRgba8Uint, "rgba8uint"},
                    ImageFormatData{core::TexelFormat::kRgba8Sint, "rgba8sint"},
                    ImageFormatData{core::TexelFormat::kRg32Uint, "rg32uint"},
                    ImageFormatData{core::TexelFormat::kRg32Sint, "rg32sint"},
                    ImageFormatData{core::TexelFormat::kRg32Float, "rg32float"},
                    ImageFormatData{core::TexelFormat::kRgba16Uint, "rgba16uint"},
                    ImageFormatData{core::TexelFormat::kRgba16Sint, "rgba16sint"},
                    ImageFormatData{core::TexelFormat::kRgba16Float, "rgba16float"},
                    ImageFormatData{core::TexelFormat::kRgba32Uint, "rgba32uint"},
                    ImageFormatData{core::TexelFormat::kRgba32Sint, "rgba32sint"},
                    ImageFormatData{core::TexelFormat::kRgba32Float, "rgba32float"}));

TEST_F(WgslASTPrinterTest, EmitType_Sampler) {
    auto sampler = ty.sampler(core::type::SamplerKind::kSampler);
    auto type = Alias("make_type_reachable", sampler)->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "sampler");
}

TEST_F(WgslASTPrinterTest, EmitType_SamplerComparison) {
    auto sampler = ty.sampler(core::type::SamplerKind::kComparisonSampler);
    auto type = Alias("make_type_reachable", sampler)->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "sampler_comparison");
}

}  // namespace
}  // namespace tint::wgsl::writer

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

#include "gmock/gmock.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/sampler.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/hlsl/writer/ast_printer/helper_test.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/ast/stage_attribute.h"
#include "src/tint/utils/text/string_stream.h"

using ::testing::HasSubstr;

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer {
namespace {

using HlslASTPrinterTest_Type = TestHelper;

TEST_F(HlslASTPrinterTest_Type, EmitType_Array) {
    auto arr = ty.array<bool, 4>();
    ast::Type ty = GlobalVar("G", arr, core::AddressSpace::kPrivate)->type;

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitType(out, program->TypeOf(ty), core::AddressSpace::kUndefined,
                             core::Access::kReadWrite, "ary"))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "bool ary[4]");
}

TEST_F(HlslASTPrinterTest_Type, EmitType_ArrayOfArray) {
    auto arr = ty.array(ty.array<bool, 4>(), 5_u);
    ast::Type ty = GlobalVar("G", arr, core::AddressSpace::kPrivate)->type;

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitType(out, program->TypeOf(ty), core::AddressSpace::kUndefined,
                             core::Access::kReadWrite, "ary"))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "bool ary[5][4]");
}

TEST_F(HlslASTPrinterTest_Type, EmitType_ArrayOfArrayOfArray) {
    auto arr = ty.array(ty.array(ty.array<bool, 4>(), 5_u), 6_u);
    ast::Type ty = GlobalVar("G", arr, core::AddressSpace::kPrivate)->type;

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitType(out, program->TypeOf(ty), core::AddressSpace::kUndefined,
                             core::Access::kReadWrite, "ary"))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "bool ary[6][5][4]");
}

TEST_F(HlslASTPrinterTest_Type, EmitType_Array_WithoutName) {
    auto arr = ty.array<bool, 4>();
    ast::Type ty = GlobalVar("G", arr, core::AddressSpace::kPrivate)->type;

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitType(out, program->TypeOf(ty), core::AddressSpace::kUndefined,
                             core::Access::kReadWrite, ""))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "bool[4]");
}

TEST_F(HlslASTPrinterTest_Type, EmitType_Bool) {
    auto* bool_ = create<core::type::Bool>();

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(
        gen.EmitType(out, bool_, core::AddressSpace::kUndefined, core::Access::kReadWrite, ""))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "bool");
}

TEST_F(HlslASTPrinterTest_Type, EmitType_F16) {
    auto* f16 = create<core::type::F16>();

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(
        gen.EmitType(out, f16, core::AddressSpace::kUndefined, core::Access::kReadWrite, ""))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "float16_t");
}

TEST_F(HlslASTPrinterTest_Type, EmitType_F32) {
    auto* f32 = create<core::type::F32>();

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(
        gen.EmitType(out, f32, core::AddressSpace::kUndefined, core::Access::kReadWrite, ""))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "float");
}

TEST_F(HlslASTPrinterTest_Type, EmitType_I32) {
    auto* i32 = create<core::type::I32>();

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(
        gen.EmitType(out, i32, core::AddressSpace::kUndefined, core::Access::kReadWrite, ""))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "int");
}

TEST_F(HlslASTPrinterTest_Type, EmitType_Matrix_F16) {
    auto* f16 = create<core::type::F16>();
    auto* vec3 = create<core::type::Vector>(f16, 3u);
    auto* mat2x3 = create<core::type::Matrix>(vec3, 2u);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(
        gen.EmitType(out, mat2x3, core::AddressSpace::kUndefined, core::Access::kReadWrite, ""))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "matrix<float16_t, 2, 3>");
}

TEST_F(HlslASTPrinterTest_Type, EmitType_Matrix_F32) {
    auto* f32 = create<core::type::F32>();
    auto* vec3 = create<core::type::Vector>(f32, 3u);
    auto* mat2x3 = create<core::type::Matrix>(vec3, 2u);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(
        gen.EmitType(out, mat2x3, core::AddressSpace::kUndefined, core::Access::kReadWrite, ""))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "float2x3");
}

TEST_F(HlslASTPrinterTest_Type, EmitType_StructDecl) {
    auto* s = Structure("S", Vector{
                                 Member("a", ty.i32()),
                                 Member("b", ty.f32()),
                             });
    GlobalVar("g", ty.Of(s), core::AddressSpace::kPrivate);

    ASTPrinter& gen = Build();

    tint::TextGenerator::TextBuffer buf;
    auto* str = program->TypeOf(s)->As<core::type::Struct>();
    ASSERT_TRUE(gen.EmitStructType(&buf, str)) << gen.Diagnostics();
    EXPECT_EQ(buf.String(), R"(struct S {
  int a;
  float b;
};
)");
}

TEST_F(HlslASTPrinterTest_Type, EmitType_StructDecl_OmittedIfStorageBuffer) {
    auto* s = Structure("S", Vector{
                                 Member("a", ty.i32()),
                                 Member("b", ty.f32()),
                             });
    GlobalVar("g", ty.Of(s), core::AddressSpace::kStorage, core::Access::kReadWrite, Binding(0_a),
              Group(0_a));

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), "RWByteAddressBuffer g : register(u0);\n");
}

TEST_F(HlslASTPrinterTest_Type, EmitType_Struct) {
    auto* s = Structure("S", Vector{
                                 Member("a", ty.i32()),
                                 Member("b", ty.f32()),
                             });
    GlobalVar("g", ty.Of(s), core::AddressSpace::kPrivate);

    ASTPrinter& gen = Build();

    auto* str = program->TypeOf(s)->As<core::type::Struct>();
    StringStream out;
    ASSERT_TRUE(
        gen.EmitType(out, str, core::AddressSpace::kUndefined, core::Access::kReadWrite, ""))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "S");
}

TEST_F(HlslASTPrinterTest_Type, EmitType_Struct_NameCollision) {
    auto* s = Structure("S", Vector{
                                 Member("double", ty.i32()),
                                 Member("float", ty.f32()),
                             });
    GlobalVar("g", ty.Of(s), core::AddressSpace::kPrivate);

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.Result(), HasSubstr(R"(struct S {
  int tint_symbol;
  float tint_symbol_1;
};
)"));
}

TEST_F(HlslASTPrinterTest_Type, EmitType_Struct_WithOffsetAttributes) {
    auto* s = Structure("S", Vector{
                                 Member("a", ty.i32(), Vector{MemberOffset(0_a)}),
                                 Member("b", ty.f32(), Vector{MemberOffset(8_a)}),
                             });
    GlobalVar("g", ty.Of(s), core::AddressSpace::kPrivate);

    ASTPrinter& gen = Build();

    tint::TextGenerator::TextBuffer buf;
    auto* str = program->TypeOf(s)->As<core::type::Struct>();
    ASSERT_TRUE(gen.EmitStructType(&buf, str)) << gen.Diagnostics();
    EXPECT_EQ(buf.String(), R"(struct S {
  int a;
  float b;
};
)");
}

TEST_F(HlslASTPrinterTest_Type, EmitType_U32) {
    auto* u32 = create<core::type::U32>();

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(
        gen.EmitType(out, u32, core::AddressSpace::kUndefined, core::Access::kReadWrite, ""))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "uint");
}

TEST_F(HlslASTPrinterTest_Type, EmitType_Vector) {
    auto* f32 = create<core::type::F32>();
    auto* vec3 = create<core::type::Vector>(f32, 3u);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(
        gen.EmitType(out, vec3, core::AddressSpace::kUndefined, core::Access::kReadWrite, ""))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "float3");
}

TEST_F(HlslASTPrinterTest_Type, EmitType_Void) {
    auto* void_ = create<core::type::Void>();

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(
        gen.EmitType(out, void_, core::AddressSpace::kUndefined, core::Access::kReadWrite, ""))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "void");
}

TEST_F(HlslASTPrinterTest_Type, EmitSampler) {
    auto* sampler = create<core::type::Sampler>(core::type::SamplerKind::kSampler);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(
        gen.EmitType(out, sampler, core::AddressSpace::kUndefined, core::Access::kReadWrite, ""))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "SamplerState");
}

TEST_F(HlslASTPrinterTest_Type, EmitSamplerComparison) {
    auto* sampler = create<core::type::Sampler>(core::type::SamplerKind::kComparisonSampler);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(
        gen.EmitType(out, sampler, core::AddressSpace::kUndefined, core::Access::kReadWrite, ""))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "SamplerComparisonState");
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
using HlslDepthTexturesTest = TestParamHelper<HlslDepthTextureData>;
TEST_P(HlslDepthTexturesTest, Emit) {
    auto params = GetParam();

    auto t = ty.depth_texture(params.dim);

    GlobalVar("tex", t, Binding(1_a), Group(2_a));

    Func("main", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("v", Call("textureDimensions", "tex"))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.Result(), HasSubstr(params.result));
}
INSTANTIATE_TEST_SUITE_P(
    HlslASTPrinterTest_Type,
    HlslDepthTexturesTest,
    testing::Values(HlslDepthTextureData{core::type::TextureDimension::k2d,
                                         "Texture2D tex : register(t1, space2);"},
                    HlslDepthTextureData{core::type::TextureDimension::k2dArray,
                                         "Texture2DArray tex : register(t1, space2);"},
                    HlslDepthTextureData{core::type::TextureDimension::kCube,
                                         "TextureCube tex : register(t1, space2);"},
                    HlslDepthTextureData{core::type::TextureDimension::kCubeArray,
                                         "TextureCubeArray tex : register(t1, space2);"}));

using HlslDepthMultisampledTexturesTest = TestHelper;
TEST_F(HlslDepthMultisampledTexturesTest, Emit) {
    auto t = ty.depth_multisampled_texture(core::type::TextureDimension::k2d);

    GlobalVar("tex", t, Binding(1_a), Group(2_a));

    Func("main", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("v", Call("textureDimensions", "tex"))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.Result(), HasSubstr("Texture2DMS<float4> tex : register(t1, space2);"));
}

enum class TextureDataType { F32, U32, I32 };
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
using HlslSampledTexturesTest = TestParamHelper<HlslSampledTextureData>;
TEST_P(HlslSampledTexturesTest, Emit) {
    auto params = GetParam();

    ast::Type datatype;
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
    ast::Type t = ty.sampled_texture(params.dim, datatype);

    GlobalVar("tex", t, Binding(1_a), Group(2_a));

    Func("main", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("v", Call("textureDimensions", "tex"))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.Result(), HasSubstr(params.result));
}
INSTANTIATE_TEST_SUITE_P(HlslASTPrinterTest_Type,
                         HlslSampledTexturesTest,
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

TEST_F(HlslASTPrinterTest_Type, EmitMultisampledTexture) {
    auto* f32 = create<core::type::F32>();
    auto* s = create<core::type::MultisampledTexture>(core::type::TextureDimension::k2d, f32);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitType(out, s, core::AddressSpace::kUndefined, core::Access::kReadWrite, ""))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "Texture2DMS<float4>");
}

struct HlslStorageTextureData {
    core::type::TextureDimension dim;
    core::TexelFormat imgfmt;
    std::string result;
};
inline std::ostream& operator<<(std::ostream& out, HlslStorageTextureData data) {
    StringStream str;
    str << data.dim;
    out << str.str();
    return out;
}
using HlslStorageTexturesTest = TestParamHelper<HlslStorageTextureData>;
TEST_P(HlslStorageTexturesTest, Emit) {
    auto params = GetParam();

    auto t = ty.storage_texture(params.dim, params.imgfmt, core::Access::kWrite);

    GlobalVar("tex", t,
              Vector{
                  Group(2_a),
                  Binding(1_a),
              });

    Func("main", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("v", Call("textureDimensions", "tex"))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.Result(), HasSubstr(params.result));
}
INSTANTIATE_TEST_SUITE_P(
    HlslASTPrinterTest_Type,
    HlslStorageTexturesTest,
    testing::Values(
        HlslStorageTextureData{core::type::TextureDimension::k1d, core::TexelFormat::kRgba8Unorm,
                               "RWTexture1D<float4> tex : register(u1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k2d, core::TexelFormat::kRgba16Float,
                               "RWTexture2D<float4> tex : register(u1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k2dArray, core::TexelFormat::kR32Float,
                               "RWTexture2DArray<float4> tex : register(u1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k3d, core::TexelFormat::kRg32Float,
                               "RWTexture3D<float4> tex : register(u1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k1d, core::TexelFormat::kRgba32Float,
                               "RWTexture1D<float4> tex : register(u1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k2d, core::TexelFormat::kRgba16Uint,
                               "RWTexture2D<uint4> tex : register(u1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k2dArray, core::TexelFormat::kR32Uint,
                               "RWTexture2DArray<uint4> tex : register(u1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k3d, core::TexelFormat::kRg32Uint,
                               "RWTexture3D<uint4> tex : register(u1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k1d, core::TexelFormat::kRgba32Uint,
                               "RWTexture1D<uint4> tex : register(u1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k2d, core::TexelFormat::kRgba16Sint,
                               "RWTexture2D<int4> tex : register(u1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k2dArray, core::TexelFormat::kR32Sint,
                               "RWTexture2DArray<int4> tex : register(u1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k3d, core::TexelFormat::kRg32Sint,
                               "RWTexture3D<int4> tex : register(u1, space2);"},
        HlslStorageTextureData{core::type::TextureDimension::k1d, core::TexelFormat::kRgba32Sint,
                               "RWTexture1D<int4> tex : register(u1, space2);"}));

}  // namespace
}  // namespace tint::hlsl::writer

// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/core/type/type.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/lang/spirv/writer/common/helper_test.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::spirv::writer {
namespace {

TEST_F(SpirvWriterTest, Type_Void) {
    auto* fn = b.Function("f", ty.void_());
    b.Append(fn->Block(), [&] { b.Return(fn); });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%void = OpTypeVoid");
}

TEST_F(SpirvWriterTest, Type_Bool) {
    b.Append(b.ir.root_block, [&] {  //
        b.Var<private_, bool, read_write>("v");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%bool = OpTypeBool");
}

TEST_F(SpirvWriterTest, Type_I32) {
    b.Append(b.ir.root_block, [&] {  //
        b.Var<private_, i32, read_write>("v");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%int = OpTypeInt 32 1");
}

TEST_F(SpirvWriterTest, Type_U32) {
    b.Append(b.ir.root_block, [&] {  //
        b.Var<private_, u32, read_write>("v");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%uint = OpTypeInt 32 0");
}

TEST_F(SpirvWriterTest, Type_F32) {
    b.Append(b.ir.root_block, [&] {  //
        b.Var<private_, f32, read_write>("v");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%float = OpTypeFloat 32");
}

TEST_F(SpirvWriterTest, Type_F16) {
    b.Append(b.ir.root_block, [&] {  //
        b.Var<private_, f16, read_write>("v");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpCapability Float16");
    EXPECT_INST("OpCapability UniformAndStorageBuffer16BitAccess");
    EXPECT_INST("OpCapability StorageBuffer16BitAccess");
    EXPECT_INST("%half = OpTypeFloat 16");
}

TEST_F(SpirvWriterTest, Type_Vec2i) {
    b.Append(b.ir.root_block, [&] {  //
        b.Var<private_, vec2<i32>, read_write>("v");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v2int = OpTypeVector %int 2");
}

TEST_F(SpirvWriterTest, Type_Vec3u) {
    b.Append(b.ir.root_block, [&] {  //
        b.Var<private_, vec3<u32>, read_write>("v");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v3uint = OpTypeVector %uint 3");
}

TEST_F(SpirvWriterTest, Type_Vec4f) {
    b.Append(b.ir.root_block, [&] {  //
        b.Var<private_, vec4<f32>, read_write>("v");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v4float = OpTypeVector %float 4");
}

TEST_F(SpirvWriterTest, Type_Vec2h) {
    b.Append(b.ir.root_block, [&] {  //
        b.Var<private_, vec2<f16>, read_write>("v");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v2half = OpTypeVector %half 2");
}

TEST_F(SpirvWriterTest, Type_Vec4Bool) {
    b.Append(b.ir.root_block, [&] {  //
        b.Var<private_, vec4<bool>, read_write>("v");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v4bool = OpTypeVector %bool 4");
}

TEST_F(SpirvWriterTest, Type_Mat2x3f) {
    b.Append(b.ir.root_block, [&] {  //
        b.Var<private_, mat2x3<f32>, read_write>("v");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%mat2v3float = OpTypeMatrix %v3float 2");
}

TEST_F(SpirvWriterTest, Type_Mat4x2h) {
    b.Append(b.ir.root_block, [&] {  //
        b.Var<private_, mat4x2<f16>, read_write>("v");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%mat4v2half = OpTypeMatrix %v2half 4");
}

TEST_F(SpirvWriterTest, Type_Array_DefaultStride) {
    b.Append(b.ir.root_block, [&] {  //
        b.Var<private_, array<f32, 4>, read_write>("v");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpDecorate %_arr_float_uint_4 ArrayStride 4");
    EXPECT_INST("%_arr_float_uint_4 = OpTypeArray %float %uint_4");
}

TEST_F(SpirvWriterTest, Type_Array_ExplicitStride) {
    b.Append(b.ir.root_block, [&] {  //
        b.Var("v", ty.ptr<private_, read_write>(ty.array<f32, 4>(16)));
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpDecorate %_arr_float_uint_4 ArrayStride 16");
    EXPECT_INST("%_arr_float_uint_4 = OpTypeArray %float %uint_4");
}

TEST_F(SpirvWriterTest, Type_Array_NestedArray) {
    b.Append(b.ir.root_block, [&] {  //
        b.Var<private_, array<array<f32, 64>, 4>, read_write>("v");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpDecorate %_arr_float_uint_64 ArrayStride 4");
    EXPECT_INST("OpDecorate %_arr__arr_float_uint_64_uint_4 ArrayStride 256");
    EXPECT_INST("%_arr_float_uint_64 = OpTypeArray %float %uint_64");
    EXPECT_INST("%_arr__arr_float_uint_64_uint_4 = OpTypeArray %_arr_float_uint_64 %uint_4");
}

TEST_F(SpirvWriterTest, Type_RuntimeArray_DefaultStride) {
    b.Append(b.ir.root_block, [&] {  //
        auto* v = b.Var<storage, array<f32>, read_write>("v");
        v->SetBindingPoint(0, 0);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpDecorate %_runtimearr_float ArrayStride 4");
    EXPECT_INST("%_runtimearr_float = OpTypeRuntimeArray %float");
}

TEST_F(SpirvWriterTest, Type_RuntimeArray_ExplicitStride) {
    b.Append(b.ir.root_block, [&] {  //
        auto* v = b.Var("v", ty.ptr<storage, read_write>(ty.array<f32>(16)));
        v->SetBindingPoint(0, 0);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpDecorate %_runtimearr_float ArrayStride 16");
    EXPECT_INST("%_runtimearr_float = OpTypeRuntimeArray %float");
}

TEST_F(SpirvWriterTest, Type_Struct) {
    auto* str =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.Register("a"), ty.f32()},
                                                   {mod.symbols.Register("b"), ty.vec4<i32>()},
                                               });
    b.Append(b.ir.root_block, [&] {  //
        b.Var("v", ty.ptr<private_, read_write>(str));
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpMemberName %MyStruct 0 \"a\"");
    EXPECT_INST("OpMemberName %MyStruct 1 \"b\"");
    EXPECT_INST("OpName %MyStruct \"MyStruct\"");
    EXPECT_INST("OpMemberDecorate %MyStruct 0 Offset 0");
    EXPECT_INST("OpMemberDecorate %MyStruct 1 Offset 16");
    EXPECT_INST("%MyStruct = OpTypeStruct %float %v4int");
}

TEST_F(SpirvWriterTest, Type_Struct_MatrixLayout) {
    auto* str = ty.Struct(
        mod.symbols.New("MyStruct"),
        {
            {mod.symbols.Register("m"), ty.mat3x3<f32>()},
            // Matrices nested inside arrays need layout decorations on the struct member too.
            {mod.symbols.Register("arr"), ty.array(ty.array(ty.mat2x4<f16>(), 4), 4)},
        });
    b.Append(b.ir.root_block, [&] {  //
        b.Var("v", ty.ptr<private_, read_write>(str));
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpMemberDecorate %MyStruct 0 ColMajor");
    EXPECT_INST("OpMemberDecorate %MyStruct 0 MatrixStride 16");
    EXPECT_INST("OpMemberDecorate %MyStruct 1 ColMajor");
    EXPECT_INST("OpMemberDecorate %MyStruct 1 MatrixStride 8");
    EXPECT_INST("%MyStruct = OpTypeStruct %mat3v3float %_arr__arr_mat2v4half_uint_4_uint_4");
}

TEST_F(SpirvWriterTest, Type_Atomic) {
    b.Append(b.ir.root_block, [&] {  //
        b.Var<private_, atomic<i32>, read_write>("v");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%int = OpTypeInt 32 1");
}

TEST_F(SpirvWriterTest, Type_Sampler) {
    b.Append(b.ir.root_block, [&] {  //
        auto* v = b.Var("v", ty.ptr<handle, read_write>(ty.sampler()));
        v->SetBindingPoint(0, 0);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(" = OpTypeSampler");
}

TEST_F(SpirvWriterTest, Type_SamplerComparison) {
    b.Append(b.ir.root_block, [&] {  //
        auto* v = b.Var("v", ty.ptr<handle, read_write>(ty.comparison_sampler()));
        v->SetBindingPoint(0, 0);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(" = OpTypeSampler");
}

TEST_F(SpirvWriterTest, Type_Samplers_Dedup) {
    b.Append(b.ir.root_block, [&] {
        auto* v1 = b.Var("v1", ty.ptr<handle, read_write>(ty.sampler()));
        auto* v2 = b.Var("v2", ty.ptr<handle, read_write>(ty.comparison_sampler()));
        v1->SetBindingPoint(0, 1);
        v2->SetBindingPoint(0, 2);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%3 = OpTypeSampler");
    EXPECT_INST("%_ptr_UniformConstant_3 = OpTypePointer UniformConstant %3");
    EXPECT_INST("%v1 = OpVariable %_ptr_UniformConstant_3 UniformConstant");
    EXPECT_INST("%_ptr_UniformConstant_3_0 = OpTypePointer UniformConstant %3");
    EXPECT_INST("%v2 = OpVariable %_ptr_UniformConstant_3_0 UniformConstant");
}

TEST_F(SpirvWriterTest, Type_StorageTexture_Dedup) {
    b.Append(b.ir.root_block, [&] {
        auto* v1 = b.Var("v1", ty.ptr<handle, read_write>(ty.Get<core::type::StorageTexture>(
                                   core::type::TextureDimension::k2dArray,
                                   core::TexelFormat::kR32Uint, core::Access::kRead, ty.u32())));
        auto* v2 = b.Var("v2", ty.ptr<handle, read_write>(ty.Get<core::type::StorageTexture>(
                                   core::type::TextureDimension::k2dArray,
                                   core::TexelFormat::kR32Uint, core::Access::kWrite, ty.u32())));
        v1->SetBindingPoint(0, 1);
        v2->SetBindingPoint(0, 2);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%3 = OpTypeImage %uint 2D 0 1 0 2 R32ui");
    EXPECT_INST("%_ptr_UniformConstant_3 = OpTypePointer UniformConstant %3");
    EXPECT_INST("%v1 = OpVariable %_ptr_UniformConstant_3 UniformConstant");
    EXPECT_INST("%_ptr_UniformConstant_3_0 = OpTypePointer UniformConstant %3");
    EXPECT_INST("%v2 = OpVariable %_ptr_UniformConstant_3_0 UniformConstant");
}

using Dim = core::type::TextureDimension;
struct TextureCase {
    std::string result;
    Dim dim;
    TestElementType format = kF32;
};

using Type_SampledTexture = SpirvWriterTestWithParam<TextureCase>;
TEST_P(Type_SampledTexture, Emit) {
    auto params = GetParam();
    b.Append(b.ir.root_block, [&] {
        auto* v = b.Var("v", ty.ptr<handle, read_write>(ty.Get<core::type::SampledTexture>(
                                 params.dim, MakeScalarType(params.format))));
        v->SetBindingPoint(0, 0);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(params.result);
}
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest,
    Type_SampledTexture,
    testing::Values(
        TextureCase{" = OpTypeImage %float 1D 0 0 0 1 Unknown", Dim::k1d, kF32},
        TextureCase{" = OpTypeImage %float 2D 0 0 0 1 Unknown", Dim::k2d, kF32},
        TextureCase{" = OpTypeImage %float 2D 0 1 0 1 Unknown", Dim::k2dArray, kF32},
        TextureCase{" = OpTypeImage %float 3D 0 0 0 1 Unknown", Dim::k3d, kF32},
        TextureCase{" = OpTypeImage %float Cube 0 0 0 1 Unknown", Dim::kCube, kF32},
        TextureCase{" = OpTypeImage %float Cube 0 1 0 1 Unknown", Dim::kCubeArray, kF32},
        TextureCase{" = OpTypeImage %int 1D 0 0 0 1 Unknown", Dim::k1d, kI32},
        TextureCase{" = OpTypeImage %int 2D 0 0 0 1 Unknown", Dim::k2d, kI32},
        TextureCase{" = OpTypeImage %int 2D 0 1 0 1 Unknown", Dim::k2dArray, kI32},
        TextureCase{" = OpTypeImage %int 3D 0 0 0 1 Unknown", Dim::k3d, kI32},
        TextureCase{" = OpTypeImage %int Cube 0 0 0 1 Unknown", Dim::kCube, kI32},
        TextureCase{" = OpTypeImage %int Cube 0 1 0 1 Unknown", Dim::kCubeArray, kI32},
        TextureCase{" = OpTypeImage %uint 1D 0 0 0 1 Unknown", Dim::k1d, kU32},
        TextureCase{" = OpTypeImage %uint 2D 0 0 0 1 Unknown", Dim::k2d, kU32},
        TextureCase{" = OpTypeImage %uint 2D 0 1 0 1 Unknown", Dim::k2dArray, kU32},
        TextureCase{" = OpTypeImage %uint 3D 0 0 0 1 Unknown", Dim::k3d, kU32},
        TextureCase{" = OpTypeImage %uint Cube 0 0 0 1 Unknown", Dim::kCube, kU32},
        TextureCase{" = OpTypeImage %uint Cube 0 1 0 1 Unknown", Dim::kCubeArray, kU32}));

using Type_MultisampledTexture = SpirvWriterTestWithParam<TextureCase>;
TEST_P(Type_MultisampledTexture, Emit) {
    auto params = GetParam();
    b.Append(b.ir.root_block, [&] {
        auto* v = b.Var("v", ty.ptr<handle, read_write>(ty.Get<core::type::MultisampledTexture>(
                                 params.dim, MakeScalarType(params.format))));
        v->SetBindingPoint(0, 0);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(params.result);
}
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest,
    Type_MultisampledTexture,
    testing::Values(TextureCase{" = OpTypeImage %float 2D 0 0 1 1 Unknown", Dim::k2d, kF32},
                    TextureCase{" = OpTypeImage %int 2D 0 0 1 1 Unknown", Dim::k2d, kI32},
                    TextureCase{" = OpTypeImage %uint 2D 0 0 1 1 Unknown", Dim::k2d, kU32}));

using Type_DepthTexture = SpirvWriterTestWithParam<TextureCase>;
TEST_P(Type_DepthTexture, Emit) {
    auto params = GetParam();
    b.Append(b.ir.root_block, [&] {  //
        auto* v =
            b.Var("v", ty.ptr<handle, read_write>(ty.Get<core::type::DepthTexture>(params.dim)));
        v->SetBindingPoint(0, 0);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(params.result);
}
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest,
    Type_DepthTexture,
    testing::Values(TextureCase{" = OpTypeImage %float 2D 0 0 0 1 Unknown", Dim::k2d},
                    TextureCase{" = OpTypeImage %float 2D 0 1 0 1 Unknown", Dim::k2dArray},
                    TextureCase{" = OpTypeImage %float Cube 0 0 0 1 Unknown", Dim::kCube},
                    TextureCase{" = OpTypeImage %float Cube 0 1 0 1 Unknown", Dim::kCubeArray}));

TEST_F(SpirvWriterTest, Type_DepthTexture_DedupWithSampledTexture) {
    b.Append(b.ir.root_block, [&] {
        auto* v1 = b.Var("v1", ty.ptr<handle, read_write>(
                                   ty.Get<core::type::SampledTexture>(Dim::k2d, ty.f32())));
        auto* v2 =
            b.Var("v2", ty.ptr<handle, read_write>(ty.Get<core::type::DepthTexture>(Dim::k2d)));
        v1->SetBindingPoint(0, 1);
        v2->SetBindingPoint(0, 2);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(      %float = OpTypeFloat 32
          %3 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_3 = OpTypePointer UniformConstant %3
         %v1 = OpVariable %_ptr_UniformConstant_3 UniformConstant   ; DescriptorSet 0, Binding 1, Coherent
%_ptr_UniformConstant_3_0 = OpTypePointer UniformConstant %3
         %v2 = OpVariable %_ptr_UniformConstant_3_0 UniformConstant     ; DescriptorSet 0, Binding 2, Coherent
       %void = OpTypeVoid
          %9 = OpTypeFunction %void
)");
}

TEST_F(SpirvWriterTest, Type_DepthMultiSampledTexture) {
    b.Append(b.ir.root_block, [&] {
        auto* v = b.Var("v", ty.ptr<handle, read_write>(
                                 ty.Get<core::type::DepthMultisampledTexture>(Dim::k2d)));
        v->SetBindingPoint(0, 0);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(" = OpTypeImage %float 2D 0 0 1 1 Unknown");
}

TEST_F(SpirvWriterTest, Type_DepthMultisampledTexture_DedupWithMultisampledTexture) {
    b.Append(b.ir.root_block, [&] {
        auto* v1 = b.Var("v1", ty.ptr<handle, read_write>(
                                   ty.Get<core::type::MultisampledTexture>(Dim::k2d, ty.f32())));
        auto* v2 = b.Var("v2", ty.ptr<handle, read_write>(
                                   ty.Get<core::type::DepthMultisampledTexture>(Dim::k2d)));
        v1->SetBindingPoint(0, 1);
        v2->SetBindingPoint(0, 2);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(      %float = OpTypeFloat 32
          %3 = OpTypeImage %float 2D 0 0 1 1 Unknown
%_ptr_UniformConstant_3 = OpTypePointer UniformConstant %3
         %v1 = OpVariable %_ptr_UniformConstant_3 UniformConstant   ; DescriptorSet 0, Binding 1, Coherent
%_ptr_UniformConstant_3_0 = OpTypePointer UniformConstant %3
         %v2 = OpVariable %_ptr_UniformConstant_3_0 UniformConstant     ; DescriptorSet 0, Binding 2, Coherent
       %void = OpTypeVoid
          %9 = OpTypeFunction %void
)");
}

using Format = core::TexelFormat;
struct StorageTextureCase {
    std::string result;
    Dim dim;
    Format format;
};
using Type_StorageTexture = SpirvWriterTestWithParam<StorageTextureCase>;
TEST_P(Type_StorageTexture, Emit) {
    auto params = GetParam();
    b.Append(b.ir.root_block, [&] {
        auto* v =
            b.Var("v", ty.ptr<handle, read_write>(ty.Get<core::type::StorageTexture>(
                           params.dim, params.format, core::Access::kWrite,
                           core::type::StorageTexture::SubtypeFor(params.format, mod.Types()))));
        v->SetBindingPoint(0, 0);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(params.result);
    if (params.format == core::TexelFormat::kRg32Uint ||
        params.format == core::TexelFormat::kRg32Sint ||
        params.format == core::TexelFormat::kRg32Float) {
        EXPECT_INST("OpCapability StorageImageExtendedFormats");
    }
}
INSTANTIATE_TEST_SUITE_P(SpirvWriterTest,
                         Type_StorageTexture,
                         testing::Values(
                             // Test all the dimensions with a single format.
                             StorageTextureCase{" = OpTypeImage %float 1D 0 0 0 2 R32f",  //
                                                Dim::k1d, Format::kR32Float},
                             StorageTextureCase{" = OpTypeImage %float 2D 0 0 0 2 R32f",  //
                                                Dim::k2d, Format::kR32Float},
                             StorageTextureCase{" = OpTypeImage %float 2D 0 1 0 2 R32f",  //
                                                Dim::k2dArray, Format::kR32Float},
                             StorageTextureCase{" = OpTypeImage %float 3D 0 0 0 2 R32f",  //
                                                Dim::k3d, Format::kR32Float},

                             // Test all the formats with 2D.
                             StorageTextureCase{" = OpTypeImage %int 2D 0 0 0 2 R32i",  //
                                                Dim::k2d, Format::kR32Sint},
                             StorageTextureCase{" = OpTypeImage %uint 2D 0 0 0 2 R32u",  //
                                                Dim::k2d, Format::kR32Uint},
                             StorageTextureCase{" = OpTypeImage %float 2D 0 0 0 2 Rg32f",  //
                                                Dim::k2d, Format::kRg32Float},
                             StorageTextureCase{" = OpTypeImage %int 2D 0 0 0 2 Rg32i",  //
                                                Dim::k2d, Format::kRg32Sint},
                             StorageTextureCase{" = OpTypeImage %uint 2D 0 0 0 2 Rg32ui",  //
                                                Dim::k2d, Format::kRg32Uint},
                             StorageTextureCase{" = OpTypeImage %float 2D 0 0 0 2 Rgba16f",  //
                                                Dim::k2d, Format::kRgba16Float},
                             StorageTextureCase{" = OpTypeImage %int 2D 0 0 0 2 Rgba16i",  //
                                                Dim::k2d, Format::kRgba16Sint},
                             StorageTextureCase{" = OpTypeImage %uint 2D 0 0 0 2 Rgba16ui",  //
                                                Dim::k2d, Format::kRgba16Uint},
                             StorageTextureCase{" = OpTypeImage %float 2D 0 0 0 2 Rgba32f",  //
                                                Dim::k2d, Format::kRgba32Float},
                             StorageTextureCase{" = OpTypeImage %int 2D 0 0 0 2 Rgba32i",  //
                                                Dim::k2d, Format::kRgba32Sint},
                             StorageTextureCase{" = OpTypeImage %uint 2D 0 0 0 2 Rgba32ui",  //
                                                Dim::k2d, Format::kRgba32Uint},
                             StorageTextureCase{" = OpTypeImage %int 2D 0 0 0 2 Rgba8i",  //
                                                Dim::k2d, Format::kRgba8Sint},
                             StorageTextureCase{" = OpTypeImage %float 2D 0 0 0 2 Rgba8Snorm",  //
                                                Dim::k2d, Format::kRgba8Snorm},
                             StorageTextureCase{" = OpTypeImage %uint 2D 0 0 0 2 Rgba8ui",  //
                                                Dim::k2d, Format::kRgba8Uint},
                             StorageTextureCase{" = OpTypeImage %float 2D 0 0 0 2 Rgba8",  //
                                                Dim::k2d, Format::kRgba8Unorm}));

TEST_F(SpirvWriterTest, Type_SubgroupMatrix) {
    b.Append(b.ir.root_block, [&] {  //
        b.Var("left", ty.ptr<private_>(ty.subgroup_matrix_left(ty.f32(), 8, 4)));
        b.Var("right", ty.ptr<private_>(ty.subgroup_matrix_right(ty.u32(), 4, 8)));
        b.Var("result", ty.ptr<private_>(ty.subgroup_matrix_result(ty.i32(), 2, 2)));
    });

    Options options;
    options.use_vulkan_memory_model = true;
    ASSERT_TRUE(Generate(options)) << Error() << output_;
    EXPECT_INST("OpCapability CooperativeMatrixKHR");
    EXPECT_INST("OpExtension \"SPV_KHR_cooperative_matrix\"");
    EXPECT_INST("%3 = OpTypeCooperativeMatrixKHR %float %uint_3 %uint_4 %uint_8 %uint_0");
    EXPECT_INST("%13 = OpTypeCooperativeMatrixKHR %uint %uint_3 %uint_8 %uint_4 %uint_1");
    EXPECT_INST("%18 = OpTypeCooperativeMatrixKHR %int %uint_3 %uint_2 %uint_2 %uint_2");
}

// Test that we can emit multiple types.
// Includes types with the same opcode but different parameters.
TEST_F(SpirvWriterTest, Type_Multiple) {
    b.Append(b.ir.root_block, [&] {
        b.Var<private_, i32, read_write>("v1");
        b.Var<private_, u32, read_write>("v2");
        b.Var<private_, f32, read_write>("v3");
        b.Var<private_, f16, read_write>("v4");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%int = OpTypeInt 32 1");
    EXPECT_INST("%uint = OpTypeInt 32 0");
    EXPECT_INST("%float = OpTypeFloat 32");
    EXPECT_INST("%half = OpTypeFloat 16");
}

// Test that we do not emit the same type more than once.
TEST_F(SpirvWriterTest, Type_Deduplicate) {
    b.Append(b.ir.root_block, [&] {
        b.Var<private_, i32, read_write>("v1");
        b.Var<private_, i32, read_write>("v2");
        b.Var<private_, i32, read_write>("v3");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%_ptr_Private_int = OpTypePointer Private %int");
    EXPECT_INST("%v1 = OpVariable %_ptr_Private_int Private %4");
    EXPECT_INST("%v2 = OpVariable %_ptr_Private_int Private %4");
    EXPECT_INST("%v3 = OpVariable %_ptr_Private_int Private %4");
}

}  // namespace
}  // namespace tint::spirv::writer

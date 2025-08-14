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

#include "src/tint/lang/spirv/reader/lower/texture.h"

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/spirv/ir/builtin_call.h"
#include "src/tint/lang/spirv/type/image.h"
#include "src/tint/lang/spirv/type/sampled_image.h"

namespace tint::spirv::reader::lower {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using SpirvReader_TextureTest = core::ir::transform::TransformTest;

using Dim = spirv::type::Dim;
using Depth = spirv::type::Depth;
using Arrayed = spirv::type::Arrayed;
using Multisampled = spirv::type::Multisampled;
using Sampled = spirv::type::Sampled;

TEST_F(SpirvReader_TextureTest, Type_Image_1d) {
    b.Append(mod.root_block, [&] {
        auto* v = b.Var("wg", ty.ptr(handle,
                                     ty.Get<spirv::type::Image>(
                                         ty.f32(), Dim::kD1, Depth::kNotDepth, Arrayed::kNonArrayed,
                                         Multisampled::kSingleSampled, Sampled::kSamplingCompatible,
                                         core::TexelFormat::kUndefined, core::Access::kRead),
                                     read));
        v->SetBindingPoint(1, 2);
    });

    auto* src = R"(
$B1: {  # root
  %wg:ptr<handle, spirv.image<f32, 1d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read>, read> = var undef @binding_point(1, 2)
}

)";
    ASSERT_EQ(src, str());
    Run(Texture);

    auto* expect = R"(
$B1: {  # root
  %wg:ptr<handle, texture_1d<f32>, read> = var undef @binding_point(1, 2)
}

)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_TextureTest, Type_Image_2d) {
    b.Append(mod.root_block, [&] {
        auto* v = b.Var("wg", ty.ptr(handle,
                                     ty.Get<spirv::type::Image>(
                                         ty.f32(), Dim::kD2, Depth::kNotDepth, Arrayed::kNonArrayed,
                                         Multisampled::kSingleSampled, Sampled::kSamplingCompatible,
                                         core::TexelFormat::kUndefined, core::Access::kRead),
                                     read));
        v->SetBindingPoint(1, 2);
    });

    auto* src = R"(
$B1: {  # root
  %wg:ptr<handle, spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read>, read> = var undef @binding_point(1, 2)
}

)";
    ASSERT_EQ(src, str());
    Run(Texture);

    auto* expect = R"(
$B1: {  # root
  %wg:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 2)
}

)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_TextureTest, Type_Image_3d) {
    b.Append(mod.root_block, [&] {
        auto* v = b.Var("wg", ty.ptr(handle,
                                     ty.Get<spirv::type::Image>(
                                         ty.f32(), Dim::kD3, Depth::kNotDepth, Arrayed::kNonArrayed,
                                         Multisampled::kSingleSampled, Sampled::kSamplingCompatible,
                                         core::TexelFormat::kUndefined, core::Access::kRead),
                                     read));
        v->SetBindingPoint(1, 2);
    });

    auto* src = R"(
$B1: {  # root
  %wg:ptr<handle, spirv.image<f32, 3d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read>, read> = var undef @binding_point(1, 2)
}

)";
    ASSERT_EQ(src, str());
    Run(Texture);

    auto* expect = R"(
$B1: {  # root
  %wg:ptr<handle, texture_3d<f32>, read> = var undef @binding_point(1, 2)
}

)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_TextureTest, Type_Image_Cube) {
    b.Append(mod.root_block, [&] {
        auto* v =
            b.Var("wg", ty.ptr(handle,
                               ty.Get<spirv::type::Image>(
                                   ty.f32(), Dim::kCube, Depth::kNotDepth, Arrayed::kNonArrayed,
                                   Multisampled::kSingleSampled, Sampled::kSamplingCompatible,
                                   core::TexelFormat::kUndefined, core::Access::kRead),
                               read));
        v->SetBindingPoint(1, 2);
    });

    auto* src = R"(
$B1: {  # root
  %wg:ptr<handle, spirv.image<f32, cube, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read>, read> = var undef @binding_point(1, 2)
}

)";
    ASSERT_EQ(src, str());
    Run(Texture);

    auto* expect = R"(
$B1: {  # root
  %wg:ptr<handle, texture_cube<f32>, read> = var undef @binding_point(1, 2)
}

)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_TextureTest, Type_Image_SubpassData) {
    b.Append(mod.root_block, [&] {
        auto* v = b.Var(
            "wg", ty.ptr(handle,
                         ty.Get<spirv::type::Image>(
                             ty.f32(), Dim::kSubpassData, Depth::kNotDepth, Arrayed::kNonArrayed,
                             Multisampled::kSingleSampled, Sampled::kSamplingCompatible,
                             core::TexelFormat::kUndefined, core::Access::kRead),
                         read));
        v->SetBindingPoint(1, 2);
    });

    auto* src = R"(
$B1: {  # root
  %wg:ptr<handle, spirv.image<f32, subpass_data, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read>, read> = var undef @binding_point(1, 2)
}

)";
    ASSERT_EQ(src, str());
    Run(Texture);

    auto* expect = R"(
$B1: {  # root
  %wg:ptr<handle, input_attachment<f32>, read> = var undef @binding_point(1, 2)
}

)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_TextureTest, Type_Image_Depth) {
    b.Append(mod.root_block, [&] {
        auto* v = b.Var("wg", ty.ptr(handle,
                                     ty.Get<spirv::type::Image>(
                                         ty.f32(), Dim::kD2, Depth::kDepth, Arrayed::kNonArrayed,
                                         Multisampled::kSingleSampled, Sampled::kSamplingCompatible,
                                         core::TexelFormat::kUndefined, core::Access::kRead),
                                     read));
        v->SetBindingPoint(1, 2);
    });

    auto* src = R"(
$B1: {  # root
  %wg:ptr<handle, spirv.image<f32, 2d, depth, non_arrayed, single_sampled, sampling_compatible, undefined, read>, read> = var undef @binding_point(1, 2)
}

)";
    ASSERT_EQ(src, str());
    Run(Texture);

    auto* expect = R"(
$B1: {  # root
  %wg:ptr<handle, texture_depth_2d, read> = var undef @binding_point(1, 2)
}

)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_TextureTest, Type_Image_DepthUnknown) {
    b.Append(mod.root_block, [&] {
        auto* v = b.Var("wg", ty.ptr(handle,
                                     ty.Get<spirv::type::Image>(
                                         ty.f32(), Dim::kD2, Depth::kUnknown, Arrayed::kNonArrayed,
                                         Multisampled::kSingleSampled, Sampled::kSamplingCompatible,
                                         core::TexelFormat::kUndefined, core::Access::kRead),
                                     read));
        v->SetBindingPoint(1, 2);
    });

    auto* src = R"(
$B1: {  # root
  %wg:ptr<handle, spirv.image<f32, 2d, depth_unknown, non_arrayed, single_sampled, sampling_compatible, undefined, read>, read> = var undef @binding_point(1, 2)
}

)";
    ASSERT_EQ(src, str());
    Run(Texture);

    auto* expect = R"(
$B1: {  # root
  %wg:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 2)
}

)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_TextureTest, Type_Image_Arrayed) {
    b.Append(mod.root_block, [&] {
        auto* v = b.Var("wg", ty.ptr(handle,
                                     ty.Get<spirv::type::Image>(
                                         ty.f32(), Dim::kD2, Depth::kNotDepth, Arrayed::kArrayed,
                                         Multisampled::kSingleSampled, Sampled::kSamplingCompatible,
                                         core::TexelFormat::kUndefined, core::Access::kRead),
                                     read));
        v->SetBindingPoint(1, 2);
    });

    auto* src = R"(
$B1: {  # root
  %wg:ptr<handle, spirv.image<f32, 2d, not_depth, arrayed, single_sampled, sampling_compatible, undefined, read>, read> = var undef @binding_point(1, 2)
}

)";
    ASSERT_EQ(src, str());
    Run(Texture);

    auto* expect = R"(
$B1: {  # root
  %wg:ptr<handle, texture_2d_array<f32>, read> = var undef @binding_point(1, 2)
}

)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_TextureTest, Type_Image_RW_Ops) {
    b.Append(mod.root_block, [&] {
        auto* v =
            b.Var("wg", ty.ptr(handle,
                               ty.Get<spirv::type::Image>(
                                   ty.f32(), Dim::kD1, Depth::kNotDepth, Arrayed::kNonArrayed,
                                   Multisampled::kSingleSampled, Sampled::kReadWriteOpCompatible,
                                   core::TexelFormat::kRg32Float, core::Access::kRead),
                               read));
        v->SetBindingPoint(1, 2);
    });

    auto* src = R"(
$B1: {  # root
  %wg:ptr<handle, spirv.image<f32, 1d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rg32float, read>, read> = var undef @binding_point(1, 2)
}

)";
    ASSERT_EQ(src, str());
    Run(Texture);

    auto* expect = R"(
$B1: {  # root
  %wg:ptr<handle, texture_storage_1d<rg32float, read>, read> = var undef @binding_point(1, 2)
}

)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_TextureTest, Type_Image_TexelFormat) {
    b.Append(mod.root_block, [&] {
        auto* v = b.Var("wg", ty.ptr(handle,
                                     ty.Get<spirv::type::Image>(
                                         ty.f32(), Dim::kD1, Depth::kNotDepth, Arrayed::kNonArrayed,
                                         Multisampled::kSingleSampled, Sampled::kSamplingCompatible,
                                         core::TexelFormat::kRg32Float, core::Access::kRead),
                                     read));
        v->SetBindingPoint(1, 2);
    });

    auto* src = R"(
$B1: {  # root
  %wg:ptr<handle, spirv.image<f32, 1d, not_depth, non_arrayed, single_sampled, sampling_compatible, rg32float, read>, read> = var undef @binding_point(1, 2)
}

)";
    ASSERT_EQ(src, str());
    Run(Texture);

    auto* expect = R"(
$B1: {  # root
  %wg:ptr<handle, texture_1d<f32>, read> = var undef @binding_point(1, 2)
}

)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_TextureTest, UserCall) {
    auto* img_ty = ty.Get<spirv::type::Image>(
        ty.f32(), Dim::kD2, Depth::kNotDepth, Arrayed::kNonArrayed, Multisampled::kSingleSampled,
        Sampled::kSamplingCompatible, core::TexelFormat::kUndefined, core::Access::kRead);

    core::ir::Var* var = nullptr;
    b.Append(mod.root_block, [&] {
        var = b.Var("wg", ty.ptr(handle, img_ty, read));
        var->SetBindingPoint(0, 0);
    });

    auto* ld = b.Function("load", ty.void_());
    auto* pt = b.FunctionParam(img_ty);
    ld->AppendParam(pt);
    ld->Block()->Append(b.Return(ld));

    auto* main = b.Function("main", ty.void_());
    b.Append(main->Block(), [&] {
        auto* tex = b.Load(var);
        b.Call(ld, tex);
        b.Return(main);
    });

    auto* src = R"(
$B1: {  # root
  %wg:ptr<handle, spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read>, read> = var undef @binding_point(0, 0)
}

%load = func(%3:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read>):void {
  $B2: {
    ret
  }
}
%main = func():void {
  $B3: {
    %5:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read> = load %wg
    %6:void = call %load, %5
    ret
  }
}
)";
    ASSERT_EQ(src, str());
    Run(Texture);

    auto* expect = R"(
$B1: {  # root
  %wg:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
}

%main = func():void {
  $B2: {
    %3:texture_2d<f32> = load %wg
    %4:void = call %load, %3
    ret
  }
}
%load = func(%6:texture_2d<f32>):void {
  $B3: {
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::spirv::reader::lower

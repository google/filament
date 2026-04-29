// Copyright 2026 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/writer/analysis/relaxed_precision_decorations.h"

#include "gmock/gmock.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/spirv/ir/builtin_call.h"
#include "src/tint/lang/spirv/type/image.h"
#include "src/tint/lang/spirv/type/literal.h"
#include "src/tint/lang/spirv/type/sampled_image.h"

namespace tint::spirv::writer::analysis {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class SpirvWriter_RelaxedPrecisionDecorationsTest : public core::ir::IRTestHelper {
  protected:
    const core::ir::Capabilities kValidationCapabilities{
        core::ir::Capability::kAllowNonCoreTypes,
    };

    type::Image* MakeSampledImage() {
        auto dim = type::Dim::kD2;
        auto depth = type::Depth::kNotDepth;
        auto arrayed = type::Arrayed::kNonArrayed;
        auto ms = type::Multisampled::kSingleSampled;
        auto sampled = type::Sampled::kSamplingCompatible;
        auto format = core::TexelFormat::kUndefined;
        auto access = core::Access::kRead;
        return ty.Get<type::Image>(ty.f32(), dim, depth, arrayed, ms, sampled, format, access);
    }

    type::Image* MakeStorageImage(core::TexelFormat format) {
        auto dim = type::Dim::kD2;
        auto depth = type::Depth::kNotDepth;
        auto arrayed = type::Arrayed::kNonArrayed;
        auto ms = type::Multisampled::kSingleSampled;
        auto sampled = type::Sampled::kReadWriteOpCompatible;
        auto access = core::Access::kReadWrite;
        return ty.Get<type::Image>(ty.f32(), dim, depth, arrayed, ms, sampled, format, access);
    }

    /// @returns a literal operand with the given value
    core::ir::Value* Literal(uint32_t value) {
        return b.Constant(mod.constant_values.Get<core::constant::Scalar<u32>>(
            ty.Get<type::Literal>(), u32(value)));
    }
};

TEST_F(SpirvWriter_RelaxedPrecisionDecorationsTest, EmptyRootBlock) {
    auto* ep = b.ComputeFunction("main");
    b.Append(ep->Block(), [&] {  //
        b.Return(ep);
    });

    auto* src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto decorations = GetRelaxedPrecisionDecorations(mod);
    EXPECT_THAT(decorations, testing::UnorderedElementsAre());
}

TEST_F(SpirvWriter_RelaxedPrecisionDecorationsTest, NonImageVars) {
    b.Append(mod.root_block, [&] {
        auto* buffer = b.Var<storage, f32>("wgvar");
        buffer->SetBindingPoint(0, 0);
        b.Var<workgroup, f32>("wgvar");
    });

    auto* ep = b.ComputeFunction("main");
    b.Append(ep->Block(), [&] {  //
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %wgvar:ptr<storage, f32, read_write> = var undef @binding_point(0, 0)
  %wgvar_1:ptr<workgroup, f32, read_write> = var undef  # %wgvar_1: 'wgvar'
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto decorations = GetRelaxedPrecisionDecorations(mod);
    EXPECT_THAT(decorations, testing::UnorderedElementsAre());
}

TEST_F(SpirvWriter_RelaxedPrecisionDecorationsTest, StorageTexture_F16Format_Read_ConvertedToF16) {
    auto* image = b.Var("img", ty.ptr<handle>(MakeStorageImage(core::TexelFormat::kRgba16Float)));
    image->SetBindingPoint(0, 0);
    mod.root_block->Append(image);

    core::ir::Value* texel = nullptr;
    auto* ep = b.ComputeFunction("main");
    b.Append(ep->Block(), [&] {  //
        auto* coords = b.Zero<vec2u>();
        texel = b.Call<ir::BuiltinCall>(ty.vec4f(), BuiltinFn::kImageRead, b.Load(image), coords,
                                        Literal(0u))
                    ->Result();
        b.Let("result", b.Multiply(b.Convert<vec4h>(texel), 2_h));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %img:ptr<handle, spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba16float, read_write>, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba16float, read_write> = load %img
    %4:vec4<f32> = spirv.image_read %3, vec2<u32>(0u), 0u
    %5:vec4<f16> = convert %4
    %6:vec4<f16> = mul %5, 2.0h
    %result:vec4<f16> = let %6
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod, kValidationCapabilities), Success);

    // The image variable will be relaxed precision due to the f16 texel format.
    // The texel value will be relaxed precision since it is converted to f16.
    auto decorations = GetRelaxedPrecisionDecorations(mod);
    EXPECT_THAT(decorations, testing::UnorderedElementsAre(image->Result(), texel));
}

TEST_F(SpirvWriter_RelaxedPrecisionDecorationsTest,
       StorageTexture_F16Format_ConvertedToF16_ViaLets) {
    auto* image = b.Var("img", ty.ptr<handle>(MakeStorageImage(core::TexelFormat::kRgba16Float)));
    image->SetBindingPoint(0, 0);
    mod.root_block->Append(image);

    core::ir::Value* texel = nullptr;
    auto* ep = b.ComputeFunction("main");
    b.Append(ep->Block(), [&] {  //
        auto* coords = b.Zero<vec2u>();
        texel = b.Call<ir::BuiltinCall>(ty.vec4f(), BuiltinFn::kImageRead, b.Load(image), coords,
                                        Literal(0u))
                    ->Result();
        auto* tmp1 = b.Let("tmp1", texel);
        auto* tmp2 = b.Let("tmp2", tmp1);
        b.Let("result", b.Multiply(b.Convert<vec4h>(tmp2), 2_h));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %img:ptr<handle, spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba16float, read_write>, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba16float, read_write> = load %img
    %4:vec4<f32> = spirv.image_read %3, vec2<u32>(0u), 0u
    %tmp1:vec4<f32> = let %4
    %tmp2:vec4<f32> = let %tmp1
    %7:vec4<f16> = convert %tmp2
    %8:vec4<f16> = mul %7, 2.0h
    %result:vec4<f16> = let %8
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod, kValidationCapabilities), Success);

    // The image variable will be relaxed precision due to the f16 texel format.
    // The texel value will be relaxed precision since it is converted to f16.
    auto decorations = GetRelaxedPrecisionDecorations(mod);
    EXPECT_THAT(decorations, testing::UnorderedElementsAre(texel, image->Result()));
}

TEST_F(SpirvWriter_RelaxedPrecisionDecorationsTest,
       StorageTexture_F16Format_Read_NotConvertedToF16) {
    auto* image = b.Var("img", ty.ptr<handle>(MakeStorageImage(core::TexelFormat::kRgba16Float)));
    image->SetBindingPoint(0, 0);
    mod.root_block->Append(image);

    core::ir::Value* texel = nullptr;
    auto* ep = b.ComputeFunction("main");
    b.Append(ep->Block(), [&] {  //
        auto* coords = b.Zero<vec2u>();
        texel = b.Call<ir::BuiltinCall>(ty.vec4f(), BuiltinFn::kImageRead, b.Load(image), coords,
                                        Literal(0u))
                    ->Result();
        b.Let("result", b.Multiply(texel, 2_f));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %img:ptr<handle, spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba16float, read_write>, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba16float, read_write> = load %img
    %4:vec4<f32> = spirv.image_read %3, vec2<u32>(0u), 0u
    %5:vec4<f32> = mul %4, 2.0f
    %result:vec4<f32> = let %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod, kValidationCapabilities), Success);

    // The image variable will be relaxed precision due to the f16 texel format.
    // The texel value will NOT be relaxed precision since it is used as an f32 value.
    auto decorations = GetRelaxedPrecisionDecorations(mod);
    EXPECT_THAT(decorations, testing::UnorderedElementsAre(image->Result()));
}

TEST_F(SpirvWriter_RelaxedPrecisionDecorationsTest,
       StorageTexture_F16Format_Write_ConvertedFromF16) {
    auto* image = b.Var("img", ty.ptr<handle>(MakeStorageImage(core::TexelFormat::kRgba16Float)));
    image->SetBindingPoint(0, 0);
    mod.root_block->Append(image);

    core::ir::Value* texel = nullptr;
    auto* ep = b.ComputeFunction("main");
    b.Append(ep->Block(), [&] {  //
        auto* coords = b.Zero<vec2u>();
        texel = b.Convert<vec4f>(b.Splat<vec4h>(1_h))->Result();
        b.Call<ir::BuiltinCall>(ty.void_(), BuiltinFn::kImageWrite, b.Load(image), coords, texel,
                                Literal(0u));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %img:ptr<handle, spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba16float, read_write>, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:vec4<f32> = convert vec4<f16>(1.0h)
    %4:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba16float, read_write> = load %img
    %5:void = spirv.image_write %4, vec2<u32>(0u), %3, 0u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod, kValidationCapabilities), Success);

    // The image variable will be relaxed precision due to the f16 texel format.
    // The texel value will be relaxed precision since it is converted from f16.
    auto decorations = GetRelaxedPrecisionDecorations(mod);
    EXPECT_THAT(decorations, testing::UnorderedElementsAre(image->Result(), texel));
}

TEST_F(SpirvWriter_RelaxedPrecisionDecorationsTest,
       StorageTexture_F16Format_Write_ConvertedFromF16_ViaLets) {
    auto* image = b.Var("img", ty.ptr<handle>(MakeStorageImage(core::TexelFormat::kRgba32Float)));
    image->SetBindingPoint(0, 0);
    mod.root_block->Append(image);

    core::ir::Value* texel = nullptr;
    auto* ep = b.ComputeFunction("main");
    b.Append(ep->Block(), [&] {  //
        auto* coords = b.Zero<vec2u>();
        auto* val_h = b.Splat<vec4h>(1_h);
        auto* converted_f = b.Convert<vec4f>(val_h)->Result();
        auto* tmp1 = b.Let("tmp1", converted_f);
        auto* tmp2 = b.Let("tmp2", tmp1);
        texel = tmp2->Result();
        b.Call<ir::BuiltinCall>(ty.void_(), BuiltinFn::kImageWrite, b.Load(image), coords, texel,
                                Literal(0u));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %img:ptr<handle, spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba32float, read_write>, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:vec4<f32> = convert vec4<f16>(1.0h)
    %tmp1:vec4<f32> = let %3
    %tmp2:vec4<f32> = let %tmp1
    %6:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba32float, read_write> = load %img
    %7:void = spirv.image_write %6, vec2<u32>(0u), %tmp2, 0u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod, kValidationCapabilities), Success);

    // The image variable will be relaxed precision since it is used with an f16 conversion.
    // The texel value will be relaxed precision since it is converted from f16.
    auto decorations = GetRelaxedPrecisionDecorations(mod);
    EXPECT_THAT(decorations, testing::UnorderedElementsAre(image->Result(), texel));
}

TEST_F(SpirvWriter_RelaxedPrecisionDecorationsTest, StorageTexture_F16Format_Write_F32) {
    auto* image = b.Var("img", ty.ptr<handle>(MakeStorageImage(core::TexelFormat::kRgba16Float)));
    image->SetBindingPoint(0, 0);
    mod.root_block->Append(image);

    core::ir::Value* texel = nullptr;
    auto* ep = b.ComputeFunction("main");
    b.Append(ep->Block(), [&] {  //
        auto* coords = b.Zero<vec2u>();
        texel = b.Let("value", b.Splat<vec4f>(1_f))->Result();
        b.Call<ir::BuiltinCall>(ty.void_(), BuiltinFn::kImageWrite, b.Load(image), coords, texel,
                                Literal(0u));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %img:ptr<handle, spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba16float, read_write>, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %value:vec4<f32> = let vec4<f32>(1.0f)
    %4:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba16float, read_write> = load %img
    %5:void = spirv.image_write %4, vec2<u32>(0u), %value, 0u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod, kValidationCapabilities), Success);

    // The image variable will be relaxed precision due to the f16 texel format.
    // The texel value will NOT be relaxed precision since it is not converted from f16.
    auto decorations = GetRelaxedPrecisionDecorations(mod);
    EXPECT_THAT(decorations, testing::UnorderedElementsAre(image->Result()));
}

TEST_F(SpirvWriter_RelaxedPrecisionDecorationsTest, StorageTexture_F32Format_Read_ConvertedToF16) {
    auto* image = b.Var("img", ty.ptr<handle>(MakeStorageImage(core::TexelFormat::kRgba32Float)));
    image->SetBindingPoint(0, 0);
    mod.root_block->Append(image);

    core::ir::Value* texel = nullptr;
    auto* ep = b.ComputeFunction("main");
    b.Append(ep->Block(), [&] {  //
        auto* coords = b.Zero<vec2u>();
        texel = b.Call<ir::BuiltinCall>(ty.vec4f(), BuiltinFn::kImageRead, b.Load(image), coords,
                                        Literal(0u))
                    ->Result();
        b.Let("result", b.Multiply(b.Convert<vec4h>(texel), 2_h));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %img:ptr<handle, spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba32float, read_write>, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba32float, read_write> = load %img
    %4:vec4<f32> = spirv.image_read %3, vec2<u32>(0u), 0u
    %5:vec4<f16> = convert %4
    %6:vec4<f16> = mul %5, 2.0h
    %result:vec4<f16> = let %6
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod, kValidationCapabilities), Success);

    // The image variable will be relaxed precision since all accesses are converted to/from f16.
    // The texel value will be relaxed precision since it is converted to f16.
    auto decorations = GetRelaxedPrecisionDecorations(mod);
    EXPECT_THAT(decorations, testing::UnorderedElementsAre(image->Result(), texel));
}

TEST_F(SpirvWriter_RelaxedPrecisionDecorationsTest,
       StorageTexture_F32Format_Read_NotConvertedToF16) {
    auto* image = b.Var("img", ty.ptr<handle>(MakeStorageImage(core::TexelFormat::kRgba32Float)));
    image->SetBindingPoint(0, 0);
    mod.root_block->Append(image);

    core::ir::Value* texel = nullptr;
    auto* ep = b.ComputeFunction("main");
    b.Append(ep->Block(), [&] {  //
        auto* coords = b.Zero<vec2u>();
        texel = b.Call<ir::BuiltinCall>(ty.vec4f(), BuiltinFn::kImageRead, b.Load(image), coords,
                                        Literal(0u))
                    ->Result();
        b.Let("result", b.Multiply(texel, 2_f));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %img:ptr<handle, spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba32float, read_write>, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba32float, read_write> = load %img
    %4:vec4<f32> = spirv.image_read %3, vec2<u32>(0u), 0u
    %5:vec4<f32> = mul %4, 2.0f
    %result:vec4<f32> = let %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod, kValidationCapabilities), Success);

    // Neither the image variable nor the texel value get relaxed precision.
    auto decorations = GetRelaxedPrecisionDecorations(mod);
    EXPECT_THAT(decorations, testing::UnorderedElementsAre());
}

TEST_F(SpirvWriter_RelaxedPrecisionDecorationsTest,
       StorageTexture_F32Format_Write_ConvertedFromF16) {
    auto* image = b.Var("img", ty.ptr<handle>(MakeStorageImage(core::TexelFormat::kRgba32Float)));
    image->SetBindingPoint(0, 0);
    mod.root_block->Append(image);

    core::ir::Value* texel = nullptr;
    auto* ep = b.ComputeFunction("main");
    b.Append(ep->Block(), [&] {  //
        auto* coords = b.Zero<vec2u>();
        texel = b.Convert<vec4f>(b.Splat<vec4h>(1_h))->Result();
        b.Call<ir::BuiltinCall>(ty.void_(), BuiltinFn::kImageWrite, b.Load(image), coords, texel,
                                Literal(0u));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %img:ptr<handle, spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba32float, read_write>, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:vec4<f32> = convert vec4<f16>(1.0h)
    %4:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba32float, read_write> = load %img
    %5:void = spirv.image_write %4, vec2<u32>(0u), %3, 0u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod, kValidationCapabilities), Success);

    // The image variable and the texel value will be relaxed precision.
    auto decorations = GetRelaxedPrecisionDecorations(mod);
    EXPECT_THAT(decorations, testing::UnorderedElementsAre(image->Result(), texel));
}

TEST_F(SpirvWriter_RelaxedPrecisionDecorationsTest,
       StorageTexture_F32Format_Write_NotConvertedFromF16) {
    auto* image = b.Var("img", ty.ptr<handle>(MakeStorageImage(core::TexelFormat::kRgba32Float)));
    image->SetBindingPoint(0, 0);
    mod.root_block->Append(image);

    core::ir::Value* texel = nullptr;
    auto* ep = b.ComputeFunction("main");
    b.Append(ep->Block(), [&] {  //
        auto* coords = b.Zero<vec2u>();
        texel = b.Let("value", b.Splat<vec4f>(1_f))->Result();
        b.Call<ir::BuiltinCall>(ty.void_(), BuiltinFn::kImageWrite, b.Load(image), coords, texel,
                                Literal(0u));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %img:ptr<handle, spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba32float, read_write>, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %value:vec4<f32> = let vec4<f32>(1.0f)
    %4:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba32float, read_write> = load %img
    %5:void = spirv.image_write %4, vec2<u32>(0u), %value, 0u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod, kValidationCapabilities), Success);

    // Neither the image variable nor the texel value get relaxed precision.
    auto decorations = GetRelaxedPrecisionDecorations(mod);
    EXPECT_THAT(decorations, testing::UnorderedElementsAre());
}

TEST_F(SpirvWriter_RelaxedPrecisionDecorationsTest,
       StorageTexture_F32Format_TwoReads_OneConvertedToF16) {
    auto* image = b.Var("img", ty.ptr<handle>(MakeStorageImage(core::TexelFormat::kRgba32Float)));
    image->SetBindingPoint(0, 0);
    mod.root_block->Append(image);

    core::ir::Value* texel1 = nullptr;
    core::ir::Value* texel2 = nullptr;
    auto* ep = b.ComputeFunction("main");
    b.Append(ep->Block(), [&] {  //
        auto* coords = b.Zero<vec2u>();
        texel1 = b.Call<ir::BuiltinCall>(ty.vec4f(), BuiltinFn::kImageRead, b.Load(image), coords,
                                         Literal(0u))
                     ->Result();
        b.Let("result1", b.Multiply(b.Convert<vec4h>(texel1), 2_h));

        texel2 = b.Call<ir::BuiltinCall>(ty.vec4f(), BuiltinFn::kImageRead, b.Load(image), coords,
                                         Literal(0u))
                     ->Result();
        b.Let("result2", b.Multiply(texel2, 2_f));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %img:ptr<handle, spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba32float, read_write>, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba32float, read_write> = load %img
    %4:vec4<f32> = spirv.image_read %3, vec2<u32>(0u), 0u
    %5:vec4<f16> = convert %4
    %6:vec4<f16> = mul %5, 2.0h
    %result1:vec4<f16> = let %6
    %8:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba32float, read_write> = load %img
    %9:vec4<f32> = spirv.image_read %8, vec2<u32>(0u), 0u
    %10:vec4<f32> = mul %9, 2.0f
    %result2:vec4<f32> = let %10
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod, kValidationCapabilities), Success);

    // The first texel value will be relaxed precision since it is converted to f16.
    // The image variable and second texel value will NOT be relaxed precision.
    auto decorations = GetRelaxedPrecisionDecorations(mod);
    EXPECT_THAT(decorations, testing::UnorderedElementsAre(texel1));
}

TEST_F(SpirvWriter_RelaxedPrecisionDecorationsTest,
       StorageTexture_F32Format_TwoWrites_OneConvertedFromF16) {
    auto* image = b.Var("img", ty.ptr<handle>(MakeStorageImage(core::TexelFormat::kRgba32Float)));
    image->SetBindingPoint(0, 0);
    mod.root_block->Append(image);

    core::ir::Value* texel1 = nullptr;
    core::ir::Value* texel2 = nullptr;
    auto* ep = b.ComputeFunction("main");
    b.Append(ep->Block(), [&] {  //
        auto* coords = b.Zero<vec2u>();

        texel1 = b.Convert<vec4f>(b.Splat<vec4h>(1_h))->Result();
        b.Call<ir::BuiltinCall>(ty.void_(), BuiltinFn::kImageWrite, b.Load(image), coords, texel1,
                                Literal(0u));

        texel2 = b.Let("value", b.Splat<vec4f>(1_f))->Result();
        b.Call<ir::BuiltinCall>(ty.void_(), BuiltinFn::kImageWrite, b.Load(image), coords, texel2,
                                Literal(0u));

        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %img:ptr<handle, spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba32float, read_write>, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:vec4<f32> = convert vec4<f16>(1.0h)
    %4:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba32float, read_write> = load %img
    %5:void = spirv.image_write %4, vec2<u32>(0u), %3, 0u
    %value:vec4<f32> = let vec4<f32>(1.0f)
    %7:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba32float, read_write> = load %img
    %8:void = spirv.image_write %7, vec2<u32>(0u), %value, 0u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod, kValidationCapabilities), Success);

    // The image variable will NOT be relaxed precision since one write is not relaxed.
    // The first texel value will be relaxed precision since it is converted from f16.
    auto decorations = GetRelaxedPrecisionDecorations(mod);
    EXPECT_THAT(decorations, testing::UnorderedElementsAre(texel1));
}

TEST_F(SpirvWriter_RelaxedPrecisionDecorationsTest, SampledTexture_Read_ConvertedToF16) {
    auto* image_type = MakeSampledImage();
    auto* image = b.Var("img", ty.ptr<handle>(image_type));
    image->SetBindingPoint(0, 0);
    mod.root_block->Append(image);

    auto* sampler = b.Var("smp", ty.ptr<handle>(ty.sampler()));
    sampler->SetBindingPoint(0, 1);
    mod.root_block->Append(sampler);

    core::ir::Value* sample = nullptr;
    auto* ep = b.ComputeFunction("main");
    b.Append(ep->Block(), [&] {  //
        auto* coords = b.Zero<vec2f>();
        auto* loaded_image = b.Load(image);
        auto* loaded_sampler = b.Load(sampler);
        auto* sampled_image = b.CallExplicit<ir::BuiltinCall>(
            ty.Get<type::SampledImage>(image_type), BuiltinFn::kOpSampledImage, Vector{image_type},
            loaded_image, loaded_sampler);

        sample = b.Call<ir::BuiltinCall>(ty.vec4f(), BuiltinFn::kImageSampleImplicitLod,
                                         sampled_image, coords, Literal(0u))
                     ->Result();
        b.Let("result", b.Convert<vec4h>(sample));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %img:ptr<handle, spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read>, read> = var undef @binding_point(0, 0)
  %smp:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read> = load %img
    %5:sampler = load %smp
    %6:spirv.sampled_image<spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read>> = spirv.op_sampled_image<spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read>> %4, %5
    %7:vec4<f32> = spirv.image_sample_implicit_lod %6, vec2<f32>(0.0f), 0u
    %8:vec4<f16> = convert %7
    %result:vec4<f16> = let %8
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod, kValidationCapabilities), Success);

    // The image variable will be relaxed precision since all accesses are converted to/from f16.
    // The sampled value will be relaxed precision since it is converted to f16.
    auto decorations = GetRelaxedPrecisionDecorations(mod);
    EXPECT_THAT(decorations, testing::UnorderedElementsAre(image->Result(), sample));
}

TEST_F(SpirvWriter_RelaxedPrecisionDecorationsTest, SampledTexture_Read_NotConvertedToF16) {
    auto* image_type = MakeSampledImage();
    auto* image = b.Var("img", ty.ptr<handle>(image_type));
    image->SetBindingPoint(0, 0);
    mod.root_block->Append(image);

    auto* sampler = b.Var("smp", ty.ptr<handle>(ty.sampler()));
    sampler->SetBindingPoint(0, 1);
    mod.root_block->Append(sampler);

    core::ir::Value* sample = nullptr;
    auto* ep = b.ComputeFunction("main");
    b.Append(ep->Block(), [&] {  //
        auto* coords = b.Zero<vec2f>();
        auto* loaded_image = b.Load(image);
        auto* loaded_sampler = b.Load(sampler);
        auto* sampled_image = b.CallExplicit<ir::BuiltinCall>(
            ty.Get<type::SampledImage>(image_type), BuiltinFn::kOpSampledImage, Vector{image_type},
            loaded_image, loaded_sampler);

        sample = b.Call<ir::BuiltinCall>(ty.vec4f(), BuiltinFn::kImageSampleImplicitLod,
                                         sampled_image, coords, Literal(0u))
                     ->Result();
        b.Let("result", b.Multiply(sample, 2_f));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %img:ptr<handle, spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read>, read> = var undef @binding_point(0, 0)
  %smp:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read> = load %img
    %5:sampler = load %smp
    %6:spirv.sampled_image<spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read>> = spirv.op_sampled_image<spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read>> %4, %5
    %7:vec4<f32> = spirv.image_sample_implicit_lod %6, vec2<f32>(0.0f), 0u
    %8:vec4<f32> = mul %7, 2.0f
    %result:vec4<f32> = let %8
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod, kValidationCapabilities), Success);

    // Neither the image variable nor the texel value get relaxed precision.
    auto decorations = GetRelaxedPrecisionDecorations(mod);
    EXPECT_THAT(decorations, testing::UnorderedElementsAre());
}

TEST_F(SpirvWriter_RelaxedPrecisionDecorationsTest, StorageTexture_F16Format_Read_ConvertedToI32) {
    auto* image = b.Var("img", ty.ptr<handle>(MakeStorageImage(core::TexelFormat::kRgba16Float)));
    image->SetBindingPoint(0, 0);
    mod.root_block->Append(image);

    core::ir::Value* texel = nullptr;
    auto* ep = b.ComputeFunction("main");
    b.Append(ep->Block(), [&] {  //
        auto* coords = b.Zero<vec2u>();
        texel = b.Call<ir::BuiltinCall>(ty.vec4f(), BuiltinFn::kImageRead, b.Load(image), coords,
                                        Literal(0u))
                    ->Result();
        b.Let("result", b.Convert<vec4i>(texel));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %img:ptr<handle, spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba16float, read_write>, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba16float, read_write> = load %img
    %4:vec4<f32> = spirv.image_read %3, vec2<u32>(0u), 0u
    %5:vec4<i32> = convert %4
    %result:vec4<i32> = let %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod, kValidationCapabilities), Success);

    // The image variable will be relaxed precision due to the f16 texel format.
    // The texel value will NOT be relaxed precision since it is converted to i32.
    auto decorations = GetRelaxedPrecisionDecorations(mod);
    EXPECT_THAT(decorations, testing::UnorderedElementsAre(image->Result()));
}

TEST_F(SpirvWriter_RelaxedPrecisionDecorationsTest, StorageTexture_IntFormat_Read_ConvertedToF16) {
    auto* image_type = ty.Get<type::Image>(
        ty.i32(), type::Dim::kD2, type::Depth::kNotDepth, type::Arrayed::kNonArrayed,
        type::Multisampled::kSingleSampled, type::Sampled::kReadWriteOpCompatible,
        core::TexelFormat::kRgba16Sint, core::Access::kReadWrite);
    auto* image = b.Var("img", ty.ptr<handle>(image_type));
    image->SetBindingPoint(0, 0);
    mod.root_block->Append(image);

    core::ir::Value* texel = nullptr;
    auto* ep = b.ComputeFunction("main");
    b.Append(ep->Block(), [&] {  //
        auto* coords = b.Zero<vec2u>();
        texel = b.Call<ir::BuiltinCall>(ty.vec4i(), BuiltinFn::kImageRead, b.Load(image), coords,
                                        Literal(0u))
                    ->Result();
        b.Let("result", b.Convert<vec4h>(texel));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %img:ptr<handle, spirv.image<i32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba16sint, read_write>, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:spirv.image<i32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba16sint, read_write> = load %img
    %4:vec4<i32> = spirv.image_read %3, vec2<u32>(0u), 0u
    %5:vec4<f16> = convert %4
    %result:vec4<f16> = let %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod, kValidationCapabilities), Success);

    // The image variable will NOT be relaxed precision because it is an integer format.
    // The texel value WILL be relaxed precision since it is converted to f16.
    auto decorations = GetRelaxedPrecisionDecorations(mod);
    EXPECT_THAT(decorations, testing::UnorderedElementsAre(texel));
}

TEST_F(SpirvWriter_RelaxedPrecisionDecorationsTest,
       FunctionParam_StorageTexture_F32Format_Read_ConvertedToF16) {
    auto* image_type = MakeStorageImage(core::TexelFormat::kRgba32Float);
    auto* image = b.Var("img", ty.ptr<handle>(image_type));
    image->SetBindingPoint(0, 0);
    mod.root_block->Append(image);

    core::ir::Value* texel = nullptr;
    auto* foo = b.Function("foo", ty.void_());
    auto* image_param = b.FunctionParam("image_param", image_type);
    foo->AppendParam(image_param);
    b.Append(foo->Block(), [&] {  //
        auto* coords = b.Zero<vec2u>();
        texel = b.Call<ir::BuiltinCall>(ty.vec4f(), BuiltinFn::kImageRead, image_param, coords,
                                        Literal(0u))
                    ->Result();
        b.Let("result", b.Multiply(b.Convert<vec4h>(texel), 2_h));
        b.Return(foo);
    });

    auto* ep = b.ComputeFunction("main");
    b.Append(ep->Block(), [&] {  //
        b.Call(ty.void_(), foo, b.Load(image));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %img:ptr<handle, spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba32float, read_write>, read> = var undef @binding_point(0, 0)
}

%foo = func(%image_param:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba32float, read_write>):void {
  $B2: {
    %4:vec4<f32> = spirv.image_read %image_param, vec2<u32>(0u), 0u
    %5:vec4<f16> = convert %4
    %6:vec4<f16> = mul %5, 2.0h
    %result:vec4<f16> = let %6
    ret
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %9:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba32float, read_write> = load %img
    %10:void = call %foo, %9
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod, kValidationCapabilities), Success);

    // The image variable will be relaxed precision since all accesses are converted to/from f16.
    // The texel value will be relaxed precision since it is converted to f16.
    auto decorations = GetRelaxedPrecisionDecorations(mod);
    EXPECT_THAT(decorations, testing::UnorderedElementsAre(image->Result(), image_param, texel));
}

TEST_F(SpirvWriter_RelaxedPrecisionDecorationsTest,
       FunctionParam_StorageTexture_F32Format_Read_NotConvertedToF16) {
    auto* image_type = MakeStorageImage(core::TexelFormat::kRgba32Float);
    auto* image = b.Var("img", ty.ptr<handle>(image_type));
    image->SetBindingPoint(0, 0);
    mod.root_block->Append(image);

    auto* foo = b.Function("foo", ty.void_());
    auto* image_param = b.FunctionParam("image_param", image_type);
    foo->AppendParam(image_param);
    b.Append(foo->Block(), [&] {  //
        auto* coords = b.Zero<vec2u>();
        auto* texel = b.Call<ir::BuiltinCall>(ty.vec4f(), BuiltinFn::kImageRead, image_param,
                                              coords, Literal(0u));
        b.Let("result", b.Multiply(texel, 2_f));
        b.Return(foo);
    });

    auto* ep = b.ComputeFunction("main");
    b.Append(ep->Block(), [&] {  //
        b.Call(ty.void_(), foo, b.Load(image));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %img:ptr<handle, spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba32float, read_write>, read> = var undef @binding_point(0, 0)
}

%foo = func(%image_param:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba32float, read_write>):void {
  $B2: {
    %4:vec4<f32> = spirv.image_read %image_param, vec2<u32>(0u), 0u
    %5:vec4<f32> = mul %4, 2.0f
    %result:vec4<f32> = let %5
    ret
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %8:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba32float, read_write> = load %img
    %9:void = call %foo, %8
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod, kValidationCapabilities), Success);

    // Neither the image variable, function parameter, nor the texel value get relaxed precision.
    auto decorations = GetRelaxedPrecisionDecorations(mod);
    EXPECT_THAT(decorations, testing::UnorderedElementsAre());
}

TEST_F(SpirvWriter_RelaxedPrecisionDecorationsTest,
       FunctionParam_StorageTexture_F32Format_Read_ConvertedToF16_Chain) {
    auto* image_type = MakeStorageImage(core::TexelFormat::kRgba32Float);
    auto* image = b.Var("img", ty.ptr<handle>(image_type));
    image->SetBindingPoint(0, 0);
    mod.root_block->Append(image);

    core::ir::Value* texel = nullptr;
    auto* bar = b.Function("bar", ty.void_());
    auto* bar_param = b.FunctionParam("bar_param", image_type);
    bar->AppendParam(bar_param);
    b.Append(bar->Block(), [&] {  //
        auto* coords = b.Zero<vec2u>();
        texel = b.Call<ir::BuiltinCall>(ty.vec4f(), BuiltinFn::kImageRead, bar_param, coords,
                                        Literal(0u))
                    ->Result();
        b.Let("result", b.Convert<vec4h>(texel));
        b.Return(bar);
    });

    auto* foo = b.Function("foo", ty.void_());
    auto* foo_param = b.FunctionParam("foo_param", image_type);
    foo->AppendParam(foo_param);
    b.Append(foo->Block(), [&] {  //
        b.Call(ty.void_(), bar, foo_param);
        b.Return(foo);
    });

    auto* ep = b.ComputeFunction("main");
    b.Append(ep->Block(), [&] {  //
        b.Call(ty.void_(), foo, b.Load(image));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %img:ptr<handle, spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba32float, read_write>, read> = var undef @binding_point(0, 0)
}

%bar = func(%bar_param:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba32float, read_write>):void {
  $B2: {
    %4:vec4<f32> = spirv.image_read %bar_param, vec2<u32>(0u), 0u
    %5:vec4<f16> = convert %4
    %result:vec4<f16> = let %5
    ret
  }
}
%foo = func(%foo_param:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba32float, read_write>):void {
  $B3: {
    %9:void = call %bar, %foo_param
    ret
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B4: {
    %11:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba32float, read_write> = load %img
    %12:void = call %foo, %11
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod, kValidationCapabilities), Success);

    // The image variable, both function parameters, and the texel value will be relaxed precision.
    auto decorations = GetRelaxedPrecisionDecorations(mod);
    EXPECT_THAT(decorations,
                testing::UnorderedElementsAre(image->Result(), foo_param, bar_param, texel));
}

TEST_F(SpirvWriter_RelaxedPrecisionDecorationsTest,
       FunctionParam_StorageTexture_F32Format_TwoReads_OneConvertedToF16) {
    auto* image_type = MakeStorageImage(core::TexelFormat::kRgba32Float);
    auto* image = b.Var("img", ty.ptr<handle>(image_type));
    image->SetBindingPoint(0, 0);
    mod.root_block->Append(image);

    core::ir::Value* texel1 = nullptr;
    core::ir::Value* texel2 = nullptr;
    auto* foo = b.Function("foo", ty.void_());
    auto* image_param = b.FunctionParam("image_param", image_type);
    foo->AppendParam(image_param);
    b.Append(foo->Block(), [&] {  //
        auto* coords = b.Zero<vec2u>();
        texel1 = b.Call<ir::BuiltinCall>(ty.vec4f(), BuiltinFn::kImageRead, image_param, coords,
                                         Literal(0u))
                     ->Result();
        b.Let("result1", b.Convert<vec4h>(texel1));

        texel2 = b.Call<ir::BuiltinCall>(ty.vec4f(), BuiltinFn::kImageRead, image_param, coords,
                                         Literal(0u))
                     ->Result();
        b.Let("result2", b.Multiply(texel2, 2_f));
        b.Return(foo);
    });

    auto* ep = b.ComputeFunction("main");
    b.Append(ep->Block(), [&] {  //
        b.Call(ty.void_(), foo, b.Load(image));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %img:ptr<handle, spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba32float, read_write>, read> = var undef @binding_point(0, 0)
}

%foo = func(%image_param:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba32float, read_write>):void {
  $B2: {
    %4:vec4<f32> = spirv.image_read %image_param, vec2<u32>(0u), 0u
    %5:vec4<f16> = convert %4
    %result1:vec4<f16> = let %5
    %7:vec4<f32> = spirv.image_read %image_param, vec2<u32>(0u), 0u
    %8:vec4<f32> = mul %7, 2.0f
    %result2:vec4<f32> = let %8
    ret
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %11:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, rgba32float, read_write> = load %img
    %12:void = call %foo, %11
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod, kValidationCapabilities), Success);

    // The first texel value will be relaxed precision since it is converted to f16.
    // The image variable, function parameter, and second texel value will NOT be relaxed precision.
    auto decorations = GetRelaxedPrecisionDecorations(mod);
    EXPECT_THAT(decorations, testing::UnorderedElementsAre(texel1));
}

}  // namespace
}  // namespace tint::spirv::writer::analysis

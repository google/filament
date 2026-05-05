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

#include "gtest/gtest.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/ir/validator_test.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/abstract_float.h"
#include "src/tint/lang/core/type/abstract_int.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/struct.h"

namespace tint::core::ir {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(IR_ValidatorTest, Builtin_DuplicateInput) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "sm1", BuiltinValue::kSampleMask, ty.u32());
    AddBuiltinParam(f, "sm2", BuiltinValue::kSampleMask, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:46 error: duplicate instance of builtin 'sample_mask' on entry point input, must be unique per entry point i/o direction
%f = @fragment func(%sm1:u32 [@sample_mask], %sm2:u32 [@sample_mask]):void {
                                             ^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_DuplicateOutput) {
    auto* sm1 = b.Var("sm1", AddressSpace::kOut, ty.u32());
    sm1->SetBuiltin(BuiltinValue::kSampleMask);
    mod.root_block->Append(sm1);

    auto* sm2 = b.Var("sm2", AddressSpace::kOut, ty.u32());
    sm2->SetBuiltin(BuiltinValue::kSampleMask);
    mod.root_block->Append(sm2);

    auto* f = FragmentEntryPoint();

    b.Append(f->Block(), [&] {
        b.Store(sm1, 0_u);
        b.Store(sm2, 0_u);
        b.Unreachable();
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:3:38 error: var: duplicate instance of builtin 'sample_mask' on entry point output, must be unique per entry point i/o direction
  %sm2:ptr<__out, u32, read_write> = var undef @builtin(sample_mask)
                                     ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_InputOutput) {
    auto* sm_out = b.Var("sm_out", AddressSpace::kOut, ty.u32());
    sm_out->SetBuiltin(BuiltinValue::kSampleMask);
    mod.root_block->Append(sm_out);

    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "sm_in", BuiltinValue::kSampleMask, ty.u32());

    b.Append(f->Block(), [&] {
        b.Store(sm_out, 0_u);
        b.Unreachable();
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Builtin_ClipDistance_Duplicate) {
    const auto attr = IOAttributes{.builtin = BuiltinValue::kClipDistances};
    auto* str_ty = ty.Struct(mod.symbols.New("OutputStruct"),
                             {{mod.symbols.New("cd1"), ty.array<f32, 2>(), attr},
                              {mod.symbols.New("cd2"), ty.array<f32, 2>(), attr}});
    auto* outs = b.Var("outs", AddressSpace::kOut, str_ty);
    mod.root_block->Append(outs);

    auto* f = VertexEntryPoint();

    b.Append(f->Block(), [&] {
        auto* cd1 = b.Access(ty.ptr(AddressSpace::kOut, ty.array<f32, 2>()), outs, 0_u);
        auto* cd2 = b.Access(ty.ptr(AddressSpace::kOut, ty.array<f32, 2>()), outs, 1_u);
        b.Store(b.Access(ty.ptr(AddressSpace::kOut, ty.f32()), cd1, 0_u), 0_f);
        b.Store(b.Access(ty.ptr(AddressSpace::kOut, ty.f32()), cd2, 0_u), 0_f);
        b.Unreachable();
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:7:48 error: var: duplicate instance of builtin 'clip_distances' on entry point output, must be unique per entry point i/o direction
  %outs:ptr<__out, OutputStruct, read_write> = var undef
                                               ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_ClipDistance_Duplicate_WithCapability) {
    const auto attr = IOAttributes{.builtin = BuiltinValue::kClipDistances};
    auto* str_ty = ty.Struct(mod.symbols.New("OutputStruct"),
                             {{mod.symbols.New("cd1"), ty.array<f32, 2>(), attr},
                              {mod.symbols.New("cd2"), ty.array<f32, 2>(), attr}});
    auto* outs = b.Var("outs", AddressSpace::kOut, str_ty);
    mod.root_block->Append(outs);

    auto* f = VertexEntryPoint();

    b.Append(f->Block(), [&] {
        auto* cd1 = b.Access(ty.ptr(AddressSpace::kOut, ty.array<f32, 2>()), outs, 0_u);
        auto* cd2 = b.Access(ty.ptr(AddressSpace::kOut, ty.array<f32, 2>()), outs, 1_u);
        b.Store(b.Access(ty.ptr(AddressSpace::kOut, ty.f32()), cd1, 0_u), 0_f);
        b.Store(b.Access(ty.ptr(AddressSpace::kOut, ty.f32()), cd2, 0_u), 0_f);
        b.Unreachable();
    });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowClipDistancesOnF32ScalarAndVector});
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_ClipDistance_Triple_WithCapability) {
    const auto attr = IOAttributes{.builtin = BuiltinValue::kClipDistances};
    auto* str_ty = ty.Struct(mod.symbols.New("OutputStruct"),
                             {{mod.symbols.New("cd1"), ty.array<f32, 2>(), attr},
                              {mod.symbols.New("cd2"), ty.array<f32, 2>(), attr},
                              {mod.symbols.New("cd3"), ty.array<f32, 2>(), attr}});
    auto* outs = b.Var("outs", AddressSpace::kOut, str_ty);
    mod.root_block->Append(outs);

    auto* f = VertexEntryPoint();

    b.Append(f->Block(), [&] {
        auto* cd1 = b.Access(ty.ptr(AddressSpace::kOut, ty.array<f32, 2>()), outs, 0_u);
        auto* cd2 = b.Access(ty.ptr(AddressSpace::kOut, ty.array<f32, 2>()), outs, 1_u);
        auto* cd3 = b.Access(ty.ptr(AddressSpace::kOut, ty.array<f32, 2>()), outs, 1_u);
        b.Store(b.Access(ty.ptr(AddressSpace::kOut, ty.f32()), cd1, 0_u), 0_f);
        b.Store(b.Access(ty.ptr(AddressSpace::kOut, ty.f32()), cd2, 0_u), 0_f);
        b.Store(b.Access(ty.ptr(AddressSpace::kOut, ty.f32()), cd3, 0_u), 0_f);
        b.Unreachable();
    });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowClipDistancesOnF32ScalarAndVector});
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:8:48 error: var: too many instances of builtin 'clip_distances' on entry point output, only two allowed with 'kAllowClipDistancesOnF32ScalarAndVector' capability enabled
  %outs:ptr<__out, OutputStruct, read_write> = var undef
                                               ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_PointSize_WrongStage) {
    auto* f = FragmentEntryPoint();
    AddBuiltinReturn(f, "size", BuiltinValue::kPointSize, ty.f32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:1 error: __point_size cannot be used on a fragment shader output. It can only be used on a vertex shader output.
%f = @fragment func():f32 [@__point_size] {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_OnStructReturn_BuiltinChecker) {
    auto* f = VertexEntryPoint();
    auto* str_ty = ty.Struct(mod.symbols.New("OutputStruct"), {
                                                                  {mod.symbols.New(""), ty.i32()},
                                                              });
    f->SetReturnType(str_ty);

    IOAttributes attr;
    attr.builtin = BuiltinValue::kPointSize;
    f->SetReturnAttributes(attr);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:5:1 error: __point_size must be a f32
%f = @vertex func():OutputStruct [@__point_size] {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_OnStructReturn_Position) {
    auto* f = VertexEntryPoint();
    auto* str_ty =
        ty.Struct(mod.symbols.New("OutputStruct"), {
                                                       {mod.symbols.New(""), ty.array(ty.f32(), 4)},
                                                   });
    f->SetReturnType(str_ty);

    IOAttributes attr;
    attr.builtin = BuiltinValue::kPosition;
    f->SetReturnAttributes(attr);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:5:1 error: position must be an vec4<f32>
%f = @vertex func():OutputStruct [@position] {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_OnStructReturn_SampleMask) {
    auto* f = FragmentEntryPoint();
    auto* str_ty = ty.Struct(mod.symbols.New("OutputStruct"), {
                                                                  {mod.symbols.New(""), ty.u32()},
                                                              });
    f->SetReturnType(str_ty);

    IOAttributes attr;
    attr.builtin = BuiltinValue::kSampleMask;
    f->SetReturnAttributes(attr);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:5:1 error: sample_mask must be an u32
%f = @fragment func():OutputStruct [@sample_mask] {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_OnStructReturn_ClipDistances) {
    auto* f = VertexEntryPoint();
    auto* str_ty =
        ty.Struct(mod.symbols.New("OutputStruct"), {
                                                       {mod.symbols.New(""), ty.array(ty.f32(), 2)},
                                                   });
    f->SetReturnType(str_ty);

    IOAttributes attr;
    attr.builtin = BuiltinValue::kClipDistances;
    f->SetReturnAttributes(attr);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:5:1 error: clip_distances must be an array<f32, N>, where N <= 8
%f = @vertex func():OutputStruct [@clip_distances] {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_PointSize_WrongIODirection) {
    auto* f = VertexEntryPoint();
    AddBuiltinParam(f, "size", BuiltinValue::kPointSize, ty.f32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:19 error: __point_size cannot be used on a vertex shader input. It can only be used on a vertex shader output.
%f = @vertex func(%size:f32 [@__point_size]):vec4<f32> [@position] {
                  ^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_PointSize_WrongType) {
    auto* f = VertexEntryPoint();
    AddBuiltinReturn(f, "size", BuiltinValue::kPointSize, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:6:1 error: __point_size must be a f32
%f = @vertex func():OutputStruct {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_ClipDistances_WrongStage) {
    auto* f = FragmentEntryPoint();
    AddBuiltinReturn(f, "distances", BuiltinValue::kClipDistances, ty.array<f32, 2>());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:1 error: clip_distances cannot be used on a fragment shader output. It can only be used on a vertex shader output.
%f = @fragment func():array<f32, 2> [@clip_distances] {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_ClipDistances_WrongIODirection) {
    auto* f = VertexEntryPoint();
    AddBuiltinParam(f, "distances", BuiltinValue::kClipDistances, ty.array<f32, 2>());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:19 error: clip_distances cannot be used on a vertex shader input. It can only be used on a vertex shader output.
%f = @vertex func(%distances:array<f32, 2> [@clip_distances]):vec4<f32> [@position] {
                  ^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_ClipDistances_WrongType_Vec) {
    auto* f = VertexEntryPoint();
    AddBuiltinReturn(f, "distances", BuiltinValue::kClipDistances, ty.vec2f());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:6:1 error: clip_distances must be an array<f32, N>, where N <= 8
%f = @vertex func():OutputStruct {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_ClipDistances_WrongType) {
    auto* f = VertexEntryPoint();
    AddBuiltinReturn(f, "distances", BuiltinValue::kClipDistances, ty.f32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:6:1 error: clip_distances must be an array<f32, N>, where N <= 8
%f = @vertex func():OutputStruct {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_FragDepth_WrongStage) {
    auto* f = VertexEntryPoint();
    AddBuiltinReturn(f, "depth", BuiltinValue::kFragDepth, ty.f32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:6:1 error: frag_depth cannot be used on a vertex shader output. It can only be used on a fragment shader output.
%f = @vertex func():OutputStruct {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_FragDepth_WrongIODirection) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "depth", BuiltinValue::kFragDepth, ty.f32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:21 error: frag_depth cannot be used on a fragment shader input. It can only be used on a fragment shader output.
%f = @fragment func(%depth:f32 [@frag_depth]):void {
                    ^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_FragDepth_WrongType) {
    auto* f = FragmentEntryPoint();
    AddBuiltinReturn(f, "depth", BuiltinValue::kFragDepth, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:1:1 error: frag_depth must be a f32
%f = @fragment func():u32 [@frag_depth] {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_NonFragDepth_NonUndefinedDepthMode) {
    auto* f = VertexEntryPoint();
    f->SetReturnDepthMode(BuiltinDepthMode::kAny);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:1 error: position cannot have a depth mode of any. It can only be undefined.
%f = @vertex func():vec4<f32> [@position] {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, MissingBuiltin_WithFragDepth) {
    auto* f = ComputeEntryPoint();
    AddReturn(f, "pos", ty.vec4f());
    f->SetReturnDepthMode(BuiltinDepthMode::kAny);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(
                                          R"(:1:1 error: cannot have a depth_mode without a builtin
%f = @compute @workgroup_size(1u, 1u, 1u) func():vec4<f32> {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_FragDepth_UndefinedDepthMode) {
    auto* f = FragmentEntryPoint();
    AddBuiltinReturn(f, "depth", BuiltinValue::kFragDepth, ty.f32());
    f->SetReturnDepthMode(BuiltinDepthMode::kUndefined);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Builtin_FragDepth_AnyDepthMode) {
    auto* f = FragmentEntryPoint();
    AddBuiltinReturn(f, "depth", BuiltinValue::kFragDepth, ty.f32());
    f->SetReturnDepthMode(BuiltinDepthMode::kAny);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_FragDepth_GreaterDepthMode) {
    auto* f = FragmentEntryPoint();
    AddBuiltinReturn(f, "depth", BuiltinValue::kFragDepth, ty.f32());
    f->SetReturnDepthMode(BuiltinDepthMode::kGreater);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_FragDepth_LessDepthMode) {
    auto* f = FragmentEntryPoint();
    AddBuiltinReturn(f, "depth", BuiltinValue::kFragDepth, ty.f32());
    f->SetReturnDepthMode(BuiltinDepthMode::kLess);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_FrontFacing_WrongStage) {
    auto* f = VertexEntryPoint();
    AddBuiltinParam(f, "facing", BuiltinValue::kFrontFacing, ty.bool_());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:19 error: front_facing cannot be used on a vertex shader input. It can only be used on a fragment shader input.
%f = @vertex func(%facing:bool [@front_facing]):vec4<f32> [@position] {
                  ^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_FrontFacing_WrongIODirection) {
    auto* f = FragmentEntryPoint();
    AddBuiltinReturn(f, "facing", BuiltinValue::kFrontFacing, ty.bool_());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:1 error: front_facing cannot be used on a fragment shader output. It can only be used on a fragment shader input.
%f = @fragment func():bool [@front_facing] {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_FrontFacing_WrongType) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "facing", BuiltinValue::kFrontFacing, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:1:21 error: front_facing must be a bool
%f = @fragment func(%facing:u32 [@front_facing]):void {
                    ^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_GlobalInvocationId_WrongStage) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "invocation", BuiltinValue::kGlobalInvocationId, ty.vec3u());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:21 error: global_invocation_id cannot be used on a fragment shader input. It can only be used on a compute shader input.
%f = @fragment func(%invocation:vec3<u32> [@global_invocation_id]):void {
                    ^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_GlobalInvocationId_WrongIODirection) {
    // This will also trigger the compute entry points should have void returns check
    auto* f = ComputeEntryPoint();
    AddBuiltinReturn(f, "invocation", BuiltinValue::kGlobalInvocationId, ty.vec3u());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:1 error: global_invocation_id cannot be used on a compute shader output. It can only be used on a compute shader input.
%f = @compute @workgroup_size(1u, 1u, 1u) func():vec3<u32> [@global_invocation_id] {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_GlobalInvocationId_WrongType) {
    auto* f = ComputeEntryPoint();
    AddBuiltinParam(f, "invocation", BuiltinValue::kGlobalInvocationId, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(
                                          R"(:1:48 error: global_invocation_id must be an vec3<u32>
%f = @compute @workgroup_size(1u, 1u, 1u) func(%invocation:u32 [@global_invocation_id]):void {
                                               ^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_InstanceIndex_WrongStage) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "instance", BuiltinValue::kInstanceIndex, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(1:21 error: instance_index cannot be used on a fragment shader input. It can only be used on a vertex shader input.
%f = @fragment func(%instance:u32 [@instance_index]):void {
                    ^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_InstanceIndex_WrongIODirection) {
    auto* f = VertexEntryPoint();
    AddBuiltinReturn(f, "instance", BuiltinValue::kInstanceIndex, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:6:1 error: instance_index cannot be used on a vertex shader output. It can only be used on a vertex shader input.
%f = @vertex func():OutputStruct {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_InstanceIndex_WrongType) {
    auto* f = VertexEntryPoint();
    AddBuiltinParam(f, "instance", BuiltinValue::kInstanceIndex, ty.i32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:19 error: instance_index must be an u32
%f = @vertex func(%instance:i32 [@instance_index]):vec4<f32> [@position] {
                  ^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_LocalInvocationId_WrongStage) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "id", BuiltinValue::kLocalInvocationId, ty.vec3u());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:21 error: local_invocation_id cannot be used on a fragment shader input. It can only be used on a compute shader input.
%f = @fragment func(%id:vec3<u32> [@local_invocation_id]):void {
                    ^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_LocalInvocationId_WrongIODirection) {
    // This will also trigger the compute entry points should have void returns check
    auto* f = ComputeEntryPoint();
    AddBuiltinReturn(f, "id", BuiltinValue::kLocalInvocationId, ty.vec3u());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:1 error: local_invocation_id cannot be used on a compute shader output. It can only be used on a compute shader input.
%f = @compute @workgroup_size(1u, 1u, 1u) func():vec3<u32> [@local_invocation_id] {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_LocalInvocationId_WrongType) {
    auto* f = ComputeEntryPoint();
    AddBuiltinParam(f, "id", BuiltinValue::kLocalInvocationId, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:48 error: local_invocation_id must be an vec3<u32>
%f = @compute @workgroup_size(1u, 1u, 1u) func(%id:u32 [@local_invocation_id]):void {
                                               ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_LocalInvocationIndex_WrongStage) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "index", BuiltinValue::kLocalInvocationIndex, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:21 error: local_invocation_index cannot be used on a fragment shader input. It can only be used on a compute shader input.
%f = @fragment func(%index:u32 [@local_invocation_index]):void {
                    ^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_LocalInvocationIndex_WrongIODirection) {
    // This will also trigger the compute entry points should have void returns check
    auto* f = ComputeEntryPoint();
    AddBuiltinReturn(f, "index", BuiltinValue::kLocalInvocationIndex, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:1 error: local_invocation_index cannot be used on a compute shader output. It can only be used on a compute shader input.
%f = @compute @workgroup_size(1u, 1u, 1u) func():u32 [@local_invocation_index] {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_LocalInvocationIndex_WrongType) {
    auto* f = ComputeEntryPoint();
    AddBuiltinParam(f, "index", BuiltinValue::kLocalInvocationIndex, ty.i32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:48 error: local_invocation_index must be an u32
%f = @compute @workgroup_size(1u, 1u, 1u) func(%index:i32 [@local_invocation_index]):void {
                                               ^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_NumWorkgroups_WrongStage) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "num", BuiltinValue::kNumWorkgroups, ty.vec3u());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:21 error: num_workgroups cannot be used on a fragment shader input. It can only be used on a compute shader input.
%f = @fragment func(%num:vec3<u32> [@num_workgroups]):void {
                    ^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_NumWorkgroups_WrongIODirection) {
    // This will also trigger the compute entry points should have void returns check
    auto* f = ComputeEntryPoint();
    AddBuiltinReturn(f, "num", BuiltinValue::kNumWorkgroups, ty.vec3u());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:1 error: num_workgroups cannot be used on a compute shader output. It can only be used on a compute shader input.
%f = @compute @workgroup_size(1u, 1u, 1u) func():vec3<u32> [@num_workgroups] {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_NumWorkgroups_WrongType) {
    auto* f = ComputeEntryPoint();
    AddBuiltinParam(f, "num", BuiltinValue::kNumWorkgroups, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:48 error: num_workgroups must be an vec3<u32>
%f = @compute @workgroup_size(1u, 1u, 1u) func(%num:u32 [@num_workgroups]):void {
                                               ^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_SampleIndex_WrongStage) {
    auto* f = VertexEntryPoint();
    AddBuiltinParam(f, "index", BuiltinValue::kSampleIndex, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:19 error: sample_index cannot be used on a vertex shader input. It can only be used on a fragment shader input.
%f = @vertex func(%index:u32 [@sample_index]):vec4<f32> [@position] {
                  ^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_SampleIndex_WrongIODirection) {
    auto* f = FragmentEntryPoint();
    AddBuiltinReturn(f, "index", BuiltinValue::kSampleIndex, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:1 error: sample_index cannot be used on a fragment shader output. It can only be used on a fragment shader input.
%f = @fragment func():u32 [@sample_index] {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_SampleIndex_WrongType) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "index", BuiltinValue::kSampleIndex, ty.f32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:1:21 error: sample_index must be an u32
%f = @fragment func(%index:f32 [@sample_index]):void {
                    ^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_VertexIndex_WrongStage) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "index", BuiltinValue::kVertexIndex, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:21 error: vertex_index cannot be used on a fragment shader input. It can only be used on a vertex shader input.
%f = @fragment func(%index:u32 [@vertex_index]):void {
                    ^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_VertexIndex_WrongIODirection) {
    auto* f = VertexEntryPoint();
    AddBuiltinReturn(f, "index", BuiltinValue::kVertexIndex, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:6:1 error: vertex_index cannot be used on a vertex shader output. It can only be used on a vertex shader input.
%f = @vertex func():OutputStruct {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_VertexIndex_WrongType) {
    auto* f = VertexEntryPoint();
    AddBuiltinParam(f, "index", BuiltinValue::kVertexIndex, ty.f32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:1:19 error: vertex_index must be an u32
%f = @vertex func(%index:f32 [@vertex_index]):vec4<f32> [@position] {
                  ^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_WorkgroupId_WrongStage) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "id", BuiltinValue::kWorkgroupId, ty.vec3u());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:21 error: workgroup_id cannot be used on a fragment shader input. It can only be used on a compute shader input.
%f = @fragment func(%id:vec3<u32> [@workgroup_id]):void {
                    ^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_WorkgroupId_WrongIODirection) {
    // This will also trigger the compute entry points should have void returns check
    auto* f = ComputeEntryPoint();
    AddBuiltinReturn(f, "id", BuiltinValue::kWorkgroupId, ty.vec3u());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:1 error: workgroup_id cannot be used on a compute shader output. It can only be used on a compute shader input.
%f = @compute @workgroup_size(1u, 1u, 1u) func():vec3<u32> [@workgroup_id] {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_WorkgroupId_WrongType) {
    auto* f = ComputeEntryPoint();
    AddBuiltinParam(f, "id", BuiltinValue::kWorkgroupId, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:48 error: workgroup_id must be an vec3<u32>
%f = @compute @workgroup_size(1u, 1u, 1u) func(%id:u32 [@workgroup_id]):void {
                                               ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_Position_WrongStage) {
    auto* f = ComputeEntryPoint();
    AddBuiltinParam(f, "pos", BuiltinValue::kPosition, ty.vec4f());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:48 error: position cannot be used on a compute shader input. It can only be used on one of [ fragment shader input, vertex shader output ]
%f = @compute @workgroup_size(1u, 1u, 1u) func(%pos:vec4<f32> [@position]):void {
                                               ^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_Position_WrongIODirectionForVertex) {
    auto* f = VertexEntryPoint();
    AddBuiltinParam(f, "pos", BuiltinValue::kPosition, ty.vec4f());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:19 error: position cannot be used on a vertex shader input. It can only be used on one of [ fragment shader input, vertex shader output ]
%f = @vertex func(%pos:vec4<f32> [@position]):vec4<f32> [@position] {
                  ^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_Position_WrongIODirectionForFragment) {
    auto* f = FragmentEntryPoint();
    AddBuiltinReturn(f, "pos", BuiltinValue::kPosition, ty.vec4f());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:1 error: position cannot be used on a fragment shader output. It can only be used on one of [ fragment shader input, vertex shader output ]
%f = @fragment func():vec4<f32> [@position] {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_Position_WrongType) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "pos", BuiltinValue::kPosition, ty.f32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:21 error: position must be an vec4<f32>
%f = @fragment func(%pos:f32 [@position]):void {
                    ^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_SampleMask_WrongStage) {
    auto* f = VertexEntryPoint();
    AddBuiltinParam(f, "mask", BuiltinValue::kSampleMask, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:19 error: sample_mask cannot be used on a vertex shader input. It can only be used on one of [ fragment shader input, fragment shader output ]
%f = @vertex func(%mask:u32 [@sample_mask]):vec4<f32> [@position] {
                  ^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_SampleMask_InputValid) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "mask", BuiltinValue::kSampleMask, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_SampleMask_OutputValid) {
    auto* f = FragmentEntryPoint();
    AddBuiltinReturn(f, "mask", BuiltinValue::kSampleMask, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_SampleMask_WrongType) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "mask", BuiltinValue::kSampleMask, ty.f32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:1:21 error: sample_mask must be an u32
%f = @fragment func(%mask:f32 [@sample_mask]):void {
                    ^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_SubgroupId_WrongStage) {
    auto* f = VertexEntryPoint();
    AddBuiltinParam(f, "id", BuiltinValue::kSubgroupId, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:19 error: subgroup_id cannot be used on a vertex shader input. It can only be used on a compute shader input.
%f = @vertex func(%id:u32 [@subgroup_id]):vec4<f32> [@position] {
                  ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_SubgroupId_WrongIODirection) {
    auto* f = ComputeEntryPoint();
    AddBuiltinReturn(f, "id", BuiltinValue::kSubgroupId, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:1 error: subgroup_id cannot be used on a compute shader output. It can only be used on a compute shader input.
%f = @compute @workgroup_size(1u, 1u, 1u) func():u32 [@subgroup_id] {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_SubgroupId_WrongType) {
    auto* f = ComputeEntryPoint();
    AddBuiltinParam(f, "id", BuiltinValue::kSubgroupId, ty.i32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:1:48 error: subgroup_id must be an u32
%f = @compute @workgroup_size(1u, 1u, 1u) func(%id:i32 [@subgroup_id]):void {
                                               ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_SubgroupSize_WrongStage) {
    auto* f = VertexEntryPoint();
    AddBuiltinParam(f, "size", BuiltinValue::kSubgroupSize, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:19 error: subgroup_size cannot be used on a vertex shader input. It can only be used on one of [ compute shader input, fragment shader input ]
%f = @vertex func(%size:u32 [@subgroup_size]):vec4<f32> [@position] {
                  ^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_SubgroupSize_WrongIODirection) {
    auto* f = FragmentEntryPoint();
    AddBuiltinReturn(f, "size", BuiltinValue::kSubgroupSize, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:1 error: subgroup_size cannot be used on a fragment shader output. It can only be used on one of [ compute shader input, fragment shader input ]
%f = @fragment func():u32 [@subgroup_size] {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_SubgroupSize_WrongType) {
    auto* f = ComputeEntryPoint();
    AddBuiltinParam(f, "size", BuiltinValue::kSubgroupSize, ty.i32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:48 error: subgroup_size must be an u32
%f = @compute @workgroup_size(1u, 1u, 1u) func(%size:i32 [@subgroup_size]):void {
                                               ^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_SubgroupInvocationId_WrongStage) {
    auto* f = VertexEntryPoint();
    AddBuiltinParam(f, "id", BuiltinValue::kSubgroupInvocationId, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:19 error: subgroup_invocation_id cannot be used on a vertex shader input. It can only be used on one of [ compute shader input, fragment shader input ]
%f = @vertex func(%id:u32 [@subgroup_invocation_id]):vec4<f32> [@position] {
                  ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_SubgroupInvocationId_WrongIODirection) {
    auto* f = FragmentEntryPoint();
    AddBuiltinReturn(f, "id", BuiltinValue::kSubgroupInvocationId, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:1 error: subgroup_invocation_id cannot be used on a fragment shader output. It can only be used on one of [ compute shader input, fragment shader input ]
%f = @fragment func():u32 [@subgroup_invocation_id] {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_SubgroupInvocationId_WrongType) {
    auto* f = ComputeEntryPoint();
    AddBuiltinParam(f, "id", BuiltinValue::kSubgroupInvocationId, ty.i32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:48 error: subgroup_invocation_id must be an u32
%f = @compute @workgroup_size(1u, 1u, 1u) func(%id:i32 [@subgroup_invocation_id]):void {
                                               ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_NumSubgroups_WrongStage) {
    auto* f = VertexEntryPoint();
    AddBuiltinParam(f, "num", BuiltinValue::kNumSubgroups, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:19 error: num_subgroups cannot be used on a vertex shader input. It can only be used on a compute shader input.
%f = @vertex func(%num:u32 [@num_subgroups]):vec4<f32> [@position] {
                  ^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_NumSubgroups_WrongIODirection) {
    auto* f = ComputeEntryPoint();
    AddBuiltinReturn(f, "num", BuiltinValue::kNumSubgroups, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:1 error: num_subgroups cannot be used on a compute shader output. It can only be used on a compute shader input.
%f = @compute @workgroup_size(1u, 1u, 1u) func():u32 [@num_subgroups] {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_NumSubgroups_WrongType) {
    auto* f = ComputeEntryPoint();
    AddBuiltinParam(f, "num", BuiltinValue::kNumSubgroups, ty.i32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:48 error: num_subgroups must be an u32
%f = @compute @workgroup_size(1u, 1u, 1u) func(%num:i32 [@num_subgroups]):void {
                                               ^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_PointSize_WithoutCapability) {
    const auto position_attr = IOAttributes{.builtin = core::BuiltinValue::kPosition};
    const auto point_size_attr = IOAttributes{.builtin = core::BuiltinValue::kPointSize};
    auto* str_ty = ty.Struct(mod.symbols.New("OutputStruct"),
                             {
                                 {mod.symbols.New("pos"), ty.vec4f(), position_attr},
                                 {mod.symbols.New("size"), ty.f32(), point_size_attr},
                             });

    auto* f = VertexEntryPoint();
    f->SetReturnType(str_ty);
    f->SetReturnAttributes({});

    b.Append(f->Block(), [&] {  //
        b.Return(f, b.Zero(str_ty));
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:6:1 error: use of point_size builtin requires kAllowPointSizeBuiltin
%f = @vertex func():OutputStruct {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_PointSize_WithCapability) {
    const auto position_attr = IOAttributes{.builtin = core::BuiltinValue::kPosition};
    const auto point_size_attr = IOAttributes{.builtin = core::BuiltinValue::kPointSize};
    auto* str_ty = ty.Struct(mod.symbols.New("OutputStruct"),
                             {
                                 {mod.symbols.New("pos"), ty.vec4f(), position_attr},
                                 {mod.symbols.New("size"), ty.f32(), point_size_attr},
                             });

    auto* f = VertexEntryPoint();
    f->SetReturnType(str_ty);
    f->SetReturnAttributes({});

    b.Append(f->Block(), [&] {  //
        b.Return(f, b.Zero(str_ty));
    });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowPointSizeBuiltin});
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Builtin_NoStage) {
    auto* f = b.Function("f", ty.void_());
    AddBuiltinParam(f, "id", BuiltinValue::kLocalInvocationId, ty.vec3u());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(
                                          R"(builtins can only be decorated on entry point params
)")) << res.Failure();
}

namespace {
template <typename T>
static const core::type::Type* TypeBuilder(core::type::Manager& m) {
    return m.Get<T>();
}

using TypeBuilderFn = decltype(&TypeBuilder<i32>);
}  // namespace

using BitcastTypeTest = IRTestParamHelper<std::tuple<
    /* bitcast allowed */ bool,
    /* src type_builder */ TypeBuilderFn,
    /* dest type_builder */ TypeBuilderFn>>;

TEST_P(BitcastTypeTest, Check) {
    bool bitcast_allowed = std::get<0>(GetParam());
    auto* src_ty = std::get<1>(GetParam())(ty);
    auto* dest_ty = std::get<2>(GetParam())(ty);

    auto* fn = b.Function("my_func", ty.void_());
    b.Append(fn->Block(), [&] {
        b.Bitcast(dest_ty, b.Zero(src_ty));
        b.Return(fn);
    });

    auto res = ir::Validate(mod);
    if (bitcast_allowed) {
        ASSERT_EQ(res, Success) << "Bitcast should be defined for '" << src_ty->FriendlyName()
                                << "' -> '" << dest_ty->FriendlyName() << "': " << res.Failure();
    } else {
        ASSERT_NE(res, Success) << "Bitcast should NOT be defined for '" << src_ty->FriendlyName()
                                << "' -> '" << dest_ty->FriendlyName() << "'";
        EXPECT_THAT(res.Failure().reason, testing::HasSubstr("no matching call to 'bitcast"));
    }
}

INSTANTIATE_TEST_SUITE_P(
    IR_ValidatorTest,
    BitcastTypeTest,
    testing::Values(
        // Scalar identity
        std::make_tuple(true, TypeBuilder<u32>, TypeBuilder<u32>),
        std::make_tuple(true, TypeBuilder<i32>, TypeBuilder<i32>),
        std::make_tuple(true, TypeBuilder<f32>, TypeBuilder<f32>),
        std::make_tuple(true, TypeBuilder<f16>, TypeBuilder<f16>),

        // Scalar reinterpretation
        std::make_tuple(true, TypeBuilder<u32>, TypeBuilder<i32>),
        std::make_tuple(true, TypeBuilder<u32>, TypeBuilder<f32>),
        std::make_tuple(true, TypeBuilder<u32>, TypeBuilder<vec2h>),
        std::make_tuple(true, TypeBuilder<i32>, TypeBuilder<u32>),
        std::make_tuple(true, TypeBuilder<i32>, TypeBuilder<f32>),
        std::make_tuple(true, TypeBuilder<i32>, TypeBuilder<vec2h>),
        std::make_tuple(true, TypeBuilder<f32>, TypeBuilder<u32>),
        std::make_tuple(true, TypeBuilder<f32>, TypeBuilder<i32>),
        std::make_tuple(true, TypeBuilder<f32>, TypeBuilder<vec2h>),
        std::make_tuple(false, TypeBuilder<u32>, TypeBuilder<f16>),
        std::make_tuple(false, TypeBuilder<i32>, TypeBuilder<f16>),
        std::make_tuple(false, TypeBuilder<f32>, TypeBuilder<f16>),
        std::make_tuple(false, TypeBuilder<f16>, TypeBuilder<u32>),
        std::make_tuple(false, TypeBuilder<f16>, TypeBuilder<i32>),
        std::make_tuple(false, TypeBuilder<f16>, TypeBuilder<f32>),
        std::make_tuple(false, TypeBuilder<f16>, TypeBuilder<vec2h>),

        // Component-wise identity, sparsely (non-exhaustively) covering types and vector sizes
        std::make_tuple(true, TypeBuilder<vec2u>, TypeBuilder<vec2u>),
        std::make_tuple(true, TypeBuilder<vec3i>, TypeBuilder<vec3i>),
        std::make_tuple(true, TypeBuilder<vec4f>, TypeBuilder<vec4f>),
        std::make_tuple(true, TypeBuilder<vec3h>, TypeBuilder<vec3h>),
        std::make_tuple(false, TypeBuilder<vec2u>, TypeBuilder<vec3u>),
        std::make_tuple(false, TypeBuilder<vec2u>, TypeBuilder<vec4u>),
        std::make_tuple(false, TypeBuilder<vec3i>, TypeBuilder<vec2i>),
        std::make_tuple(false, TypeBuilder<vec3i>, TypeBuilder<vec4i>),
        std::make_tuple(false, TypeBuilder<vec4f>, TypeBuilder<vec2f>),
        std::make_tuple(false, TypeBuilder<vec4f>, TypeBuilder<vec3f>),

        // Component-wise reinterpretation, sparsely (non-exhaustively) covering types and
        // vector sizes
        std::make_tuple(true, TypeBuilder<vec2u>, TypeBuilder<vec2i>),
        std::make_tuple(true, TypeBuilder<vec3u>, TypeBuilder<vec3f>),
        std::make_tuple(true, TypeBuilder<vec4i>, TypeBuilder<vec4u>),
        std::make_tuple(true, TypeBuilder<vec3i>, TypeBuilder<vec3f>),
        std::make_tuple(true, TypeBuilder<vec3f>, TypeBuilder<vec3u>),
        std::make_tuple(true, TypeBuilder<vec2f>, TypeBuilder<vec2i>),
        std::make_tuple(true, TypeBuilder<vec2u>, TypeBuilder<vec4h>),
        std::make_tuple(true, TypeBuilder<vec2i>, TypeBuilder<vec4h>),
        std::make_tuple(true, TypeBuilder<vec2f>, TypeBuilder<vec4h>),
        std::make_tuple(false, TypeBuilder<vec4u>, TypeBuilder<vec4h>),
        std::make_tuple(false, TypeBuilder<vec2i>, TypeBuilder<vec2h>),
        std::make_tuple(false, TypeBuilder<vec4f>, TypeBuilder<vec4h>),
        std::make_tuple(false, TypeBuilder<f16>, TypeBuilder<u32>),
        std::make_tuple(false, TypeBuilder<f16>, TypeBuilder<i32>),
        std::make_tuple(false, TypeBuilder<f16>, TypeBuilder<f32>),
        std::make_tuple(false, TypeBuilder<f16>, TypeBuilder<vec2h>)));

}  // namespace tint::core::ir

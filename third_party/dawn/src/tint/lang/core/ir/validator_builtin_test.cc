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

#include "src/tint/lang/core/ir/validator_test.h"

#include <string>

#include "gtest/gtest.h"

#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/abstract_float.h"
#include "src/tint/lang/core/type/abstract_int.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/struct.h"

namespace tint::core::ir {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(IR_ValidatorTest, Builtin_PointSize_WrongStage) {
    auto* f = FragmentEntryPoint();
    AddBuiltinReturn(f, "size", BuiltinValue::kPointSize, ty.f32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(R"(:1:1 error: __point_size must be used in a vertex shader entry point
%f = @fragment func():f32 [@__point_size] {
^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_PointSize_WrongIODirection) {
    auto* f = VertexEntryPoint();
    AddBuiltinParam(f, "size", BuiltinValue::kPointSize, ty.f32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(R"(:1:19 error: __point_size must be an output of a shader entry point
%f = @vertex func(%size:f32 [@__point_size]):vec4<f32> [@position] {
                  ^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_PointSize_WrongType) {
    auto* f = VertexEntryPoint();
    AddBuiltinReturn(f, "size", BuiltinValue::kPointSize, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:6:1 error: __point_size must be a f32
%f = @vertex func():OutputStruct {
^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_ClipDistances_WrongStage) {
    auto* f = FragmentEntryPoint();
    AddBuiltinReturn(f, "distances", BuiltinValue::kClipDistances, ty.array<f32, 2>());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(R"(:1:1 error: clip_distances must be used in a vertex shader entry point
%f = @fragment func():array<f32, 2> [@clip_distances] {
^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_ClipDistances_WrongIODirection) {
    auto* f = VertexEntryPoint();
    AddBuiltinParam(f, "distances", BuiltinValue::kClipDistances, ty.array<f32, 2>());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(R"(:1:19 error: clip_distances must be an output of a shader entry point
%f = @vertex func(%distances:array<f32, 2> [@clip_distances]):vec4<f32> [@position] {
                  ^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_ClipDistances_WrongType) {
    auto* f = VertexEntryPoint();
    AddBuiltinReturn(f, "distances", BuiltinValue::kClipDistances, ty.f32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(R"(:6:1 error: clip_distances must be an array<f32, N>, where N <= 8
%f = @vertex func():OutputStruct {
^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_FragDepth_WrongStage) {
    auto* f = VertexEntryPoint();
    AddBuiltinReturn(f, "depth", BuiltinValue::kFragDepth, ty.f32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(R"(:6:1 error: frag_depth must be used in a fragment shader entry point
%f = @vertex func():OutputStruct {
^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_FragDepth_WrongIODirection) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "depth", BuiltinValue::kFragDepth, ty.f32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(R"(:1:21 error: frag_depth must be an output of a shader entry point
%f = @fragment func(%depth:f32 [@frag_depth]):void {
                    ^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_FragDepth_WrongType) {
    auto* f = FragmentEntryPoint();
    AddBuiltinReturn(f, "depth", BuiltinValue::kFragDepth, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:1:1 error: frag_depth must be a f32
%f = @fragment func():u32 [@frag_depth] {
^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_FrontFacing_WrongStage) {
    auto* f = VertexEntryPoint();
    AddBuiltinParam(f, "facing", BuiltinValue::kFrontFacing, ty.bool_());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(
                    R"(:1:19 error: front_facing must be used in a fragment shader entry point
%f = @vertex func(%facing:bool [@front_facing]):vec4<f32> [@position] {
                  ^^^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_FrontFacing_WrongIODirection) {
    auto* f = FragmentEntryPoint();
    AddBuiltinReturn(f, "facing", BuiltinValue::kFrontFacing, ty.bool_());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(R"(:1:1 error: front_facing must be an input of a shader entry point
%f = @fragment func():bool [@front_facing] {
^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_FrontFacing_WrongType) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "facing", BuiltinValue::kFrontFacing, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:1:21 error: front_facing must be a bool
%f = @fragment func(%facing:u32 [@front_facing]):void {
                    ^^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_GlobalInvocationId_WrongStage) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "invocation", BuiltinValue::kGlobalInvocationId, ty.vec3<u32>());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(
            R"(:1:21 error: global_invocation_id must be used in a compute shader entry point
%f = @fragment func(%invocation:vec3<u32> [@global_invocation_id]):void {
                    ^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_GlobalInvocationId_WrongIODirection) {
    // This will also trigger the compute entry points should have void returns check
    auto* f = ComputeEntryPoint();
    AddBuiltinReturn(f, "invocation", BuiltinValue::kGlobalInvocationId, ty.vec3<u32>());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(
                    R"(:1:1 error: global_invocation_id must be an input of a shader entry point
%f = @compute @workgroup_size(1u, 1u, 1u) func():vec3<u32> [@global_invocation_id] {
^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_GlobalInvocationId_WrongType) {
    auto* f = ComputeEntryPoint();
    AddBuiltinParam(f, "invocation", BuiltinValue::kGlobalInvocationId, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:1:48 error: global_invocation_id must be an vec3<u32>
%f = @compute @workgroup_size(1u, 1u, 1u) func(%invocation:u32 [@global_invocation_id]):void {
                                               ^^^^^^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_InstanceIndex_WrongStage) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "instance", BuiltinValue::kInstanceIndex, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(
                    R"(:1:21 error: instance_index must be used in a vertex shader entry point
%f = @fragment func(%instance:u32 [@instance_index]):void {
                    ^^^^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_InstanceIndex_WrongIODirection) {
    auto* f = VertexEntryPoint();
    AddBuiltinReturn(f, "instance", BuiltinValue::kInstanceIndex, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(R"(:6:1 error: instance_index must be an input of a shader entry point
%f = @vertex func():OutputStruct {
^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_InstanceIndex_WrongType) {
    auto* f = VertexEntryPoint();
    AddBuiltinParam(f, "instance", BuiltinValue::kInstanceIndex, ty.i32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:1:19 error: instance_index must be an u32
%f = @vertex func(%instance:i32 [@instance_index]):vec4<f32> [@position] {
                  ^^^^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_LocalInvocationId_WrongStage) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "id", BuiltinValue::kLocalInvocationId, ty.vec3<u32>());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(
                    R"(:1:21 error: local_invocation_id must be used in a compute shader entry point
%f = @fragment func(%id:vec3<u32> [@local_invocation_id]):void {
                    ^^^^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_LocalInvocationId_WrongIODirection) {
    // This will also trigger the compute entry points should have void returns check
    auto* f = ComputeEntryPoint();
    AddBuiltinReturn(f, "id", BuiltinValue::kLocalInvocationId, ty.vec3<u32>());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(
                    R"(:1:1 error: local_invocation_id must be an input of a shader entry point
%f = @compute @workgroup_size(1u, 1u, 1u) func():vec3<u32> [@local_invocation_id] {
^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_LocalInvocationId_WrongType) {
    auto* f = ComputeEntryPoint();
    AddBuiltinParam(f, "id", BuiltinValue::kLocalInvocationId, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:1:48 error: local_invocation_id must be an vec3<u32>
%f = @compute @workgroup_size(1u, 1u, 1u) func(%id:u32 [@local_invocation_id]):void {
                                               ^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_LocalInvocationIndex_WrongStage) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "index", BuiltinValue::kLocalInvocationIndex, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(
            R"(:1:21 error: local_invocation_index must be used in a compute shader entry point
%f = @fragment func(%index:u32 [@local_invocation_index]):void {
                    ^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_LocalInvocationIndex_WrongIODirection) {
    // This will also trigger the compute entry points should have void returns check
    auto* f = ComputeEntryPoint();
    AddBuiltinReturn(f, "index", BuiltinValue::kLocalInvocationIndex, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(
                    R"(:1:1 error: local_invocation_index must be an input of a shader entry point
%f = @compute @workgroup_size(1u, 1u, 1u) func():u32 [@local_invocation_index] {
^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_LocalInvocationIndex_WrongType) {
    auto* f = ComputeEntryPoint();
    AddBuiltinParam(f, "index", BuiltinValue::kLocalInvocationIndex, ty.i32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:1:48 error: local_invocation_index must be an u32
%f = @compute @workgroup_size(1u, 1u, 1u) func(%index:i32 [@local_invocation_index]):void {
                                               ^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_NumWorkgroups_WrongStage) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "num", BuiltinValue::kNumWorkgroups, ty.vec3<u32>());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(
                    R"(:1:21 error: num_workgroups must be used in a compute shader entry point
%f = @fragment func(%num:vec3<u32> [@num_workgroups]):void {
                    ^^^^^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_NumWorkgroups_WrongIODirection) {
    // This will also trigger the compute entry points should have void returns check
    auto* f = ComputeEntryPoint();
    AddBuiltinReturn(f, "num", BuiltinValue::kNumWorkgroups, ty.vec3<u32>());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(R"(:1:1 error: num_workgroups must be an input of a shader entry point
%f = @compute @workgroup_size(1u, 1u, 1u) func():vec3<u32> [@num_workgroups] {
^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_NumWorkgroups_WrongType) {
    auto* f = ComputeEntryPoint();
    AddBuiltinParam(f, "num", BuiltinValue::kNumWorkgroups, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:1:48 error: num_workgroups must be an vec3<u32>
%f = @compute @workgroup_size(1u, 1u, 1u) func(%num:u32 [@num_workgroups]):void {
                                               ^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_SampleIndex_WrongStage) {
    auto* f = VertexEntryPoint();
    AddBuiltinParam(f, "index", BuiltinValue::kSampleIndex, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(
                    R"(:1:19 error: sample_index must be used in a fragment shader entry point
%f = @vertex func(%index:u32 [@sample_index]):vec4<f32> [@position] {
                  ^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_SampleIndex_WrongIODirection) {
    auto* f = FragmentEntryPoint();
    AddBuiltinReturn(f, "index", BuiltinValue::kSampleIndex, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(R"(:1:1 error: sample_index must be an input of a shader entry point
%f = @fragment func():u32 [@sample_index] {
^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_SampleIndex_WrongType) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "index", BuiltinValue::kSampleIndex, ty.f32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:1:21 error: sample_index must be an u32
%f = @fragment func(%index:f32 [@sample_index]):void {
                    ^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_VertexIndex_WrongStage) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "index", BuiltinValue::kVertexIndex, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(R"(:1:21 error: vertex_index must be used in a vertex shader entry point
%f = @fragment func(%index:u32 [@vertex_index]):void {
                    ^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_VertexIndex_WrongIODirection) {
    auto* f = VertexEntryPoint();
    AddBuiltinReturn(f, "index", BuiltinValue::kVertexIndex, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(R"(:6:1 error: vertex_index must be an input of a shader entry point
%f = @vertex func():OutputStruct {
^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_VertexIndex_WrongType) {
    auto* f = VertexEntryPoint();
    AddBuiltinParam(f, "index", BuiltinValue::kVertexIndex, ty.f32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:1:19 error: vertex_index must be an u32
%f = @vertex func(%index:f32 [@vertex_index]):vec4<f32> [@position] {
                  ^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_WorkgroupId_WrongStage) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "id", BuiltinValue::kWorkgroupId, ty.vec3<u32>());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(R"(1:21 error: workgroup_id must be used in a compute shader entry point
%f = @fragment func(%id:vec3<u32> [@workgroup_id]):void {
                    ^^^^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_WorkgroupId_WrongIODirection) {
    // This will also trigger the compute entry points should have void returns check
    auto* f = ComputeEntryPoint();
    AddBuiltinReturn(f, "id", BuiltinValue::kWorkgroupId, ty.vec3<u32>());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(R"(:1:1 error: workgroup_id must be an input of a shader entry point
%f = @compute @workgroup_size(1u, 1u, 1u) func():vec3<u32> [@workgroup_id] {
^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_WorkgroupId_WrongType) {
    auto* f = ComputeEntryPoint();
    AddBuiltinParam(f, "id", BuiltinValue::kWorkgroupId, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:1:48 error: workgroup_id must be an vec3<u32>
%f = @compute @workgroup_size(1u, 1u, 1u) func(%id:u32 [@workgroup_id]):void {
                                               ^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_Position_WrongStage) {
    auto* f = ComputeEntryPoint();
    AddBuiltinParam(f, "pos", BuiltinValue::kPosition, ty.vec4<f32>());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(
                    R"(:1:48 error: position must be used in a fragment or vertex shader entry point
%f = @compute @workgroup_size(1u, 1u, 1u) func(%pos:vec4<f32> [@position]):void {
                                               ^^^^^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_Position_WrongIODirectionForVertex) {
    auto* f = VertexEntryPoint();
    AddBuiltinParam(f, "pos", BuiltinValue::kPosition, ty.vec4<f32>());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(R"(:1:19 error: position must be an output for a vertex entry point
%f = @vertex func(%pos:vec4<f32> [@position]):vec4<f32> [@position] {
                  ^^^^^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_Position_WrongIODirectionForFragment) {
    auto* f = FragmentEntryPoint();
    AddBuiltinReturn(f, "pos", BuiltinValue::kPosition, ty.vec4<f32>());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(R"(:1:1 error: position must be an input for a fragment entry point
%f = @fragment func():vec4<f32> [@position] {
^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_Position_WrongType) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "pos", BuiltinValue::kPosition, ty.f32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:1:21 error: position must be an vec4<f32>
%f = @fragment func(%pos:f32 [@position]):void {
                    ^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_SampleMask_WrongStage) {
    auto* f = VertexEntryPoint();
    AddBuiltinParam(f, "mask", BuiltinValue::kSampleMask, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(R"(:1:19 error: sample_mask must be used in a fragment entry point
%f = @vertex func(%mask:u32 [@sample_mask]):vec4<f32> [@position] {
                  ^^^^^^^^^
)")) << res.Failure().reason.Str();
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
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:1:21 error: sample_mask must be an u32
%f = @fragment func(%mask:f32 [@sample_mask]):void {
                    ^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_SubgroupSize_WrongStage) {
    auto* f = VertexEntryPoint();
    AddBuiltinParam(f, "size", BuiltinValue::kSubgroupSize, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(
            R"(:1:19 error: subgroup_size must be used in a compute or fragment shader entry point
%f = @vertex func(%size:u32 [@subgroup_size]):vec4<f32> [@position] {
                  ^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_SubgroupSize_WrongIODirection) {
    auto* f = FragmentEntryPoint();
    AddBuiltinReturn(f, "size", BuiltinValue::kSubgroupSize, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(R"(:1:1 error: subgroup_size must be an input of a shader entry point
%f = @fragment func():u32 [@subgroup_size] {
^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_SubgroupSize_WrongType) {
    auto* f = ComputeEntryPoint();
    AddBuiltinParam(f, "size", BuiltinValue::kSubgroupSize, ty.i32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:1:48 error: subgroup_size must be an u32
%f = @compute @workgroup_size(1u, 1u, 1u) func(%size:i32 [@subgroup_size]):void {
                                               ^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_SubgroupInvocationId_WrongStage) {
    auto* f = VertexEntryPoint();
    AddBuiltinParam(f, "id", BuiltinValue::kSubgroupInvocationId, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(
            R"(:1:19 error: subgroup_invocation_id must be used in a compute or fragment shader entry point
%f = @vertex func(%id:u32 [@subgroup_invocation_id]):vec4<f32> [@position] {
                  ^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_SubgroupInvocationId_WrongIODirection) {
    auto* f = FragmentEntryPoint();
    AddBuiltinReturn(f, "id", BuiltinValue::kSubgroupInvocationId, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(
                    R"(:1:1 error: subgroup_invocation_id must be an input of a shader entry point
%f = @fragment func():u32 [@subgroup_invocation_id] {
^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Builtin_SubgroupInvocationId_WrongType) {
    auto* f = ComputeEntryPoint();
    AddBuiltinParam(f, "id", BuiltinValue::kSubgroupInvocationId, ty.i32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:1:48 error: subgroup_invocation_id must be an u32
%f = @compute @workgroup_size(1u, 1u, 1u) func(%id:i32 [@subgroup_invocation_id]):void {
                                               ^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Bitcast_MissingArg) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* bitcast = b.Bitcast(ty.i32(), 1_u);
        bitcast->ClearOperands();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:3:14 error: bitcast: expected exactly 1 operands, got 0
    %2:i32 = bitcast
             ^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Bitcast_NullArg) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Bitcast(ty.i32(), nullptr);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:3:22 error: bitcast: operand is undefined
    %2:i32 = bitcast undef
                     ^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Bitcast_MissingResult) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* bitcast = b.Bitcast(ty.i32(), 1_u);
        bitcast->ClearResults();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:3:13 error: bitcast: expected exactly 1 results, got 0
    undef = bitcast 1u
            ^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Bitcast_NullResult) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* c = b.Bitcast(ty.i32(), 1_u);
        c->SetResults(Vector<InstructionResult*, 1>{nullptr});
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:3:5 error: bitcast: result is undefined
    undef = bitcast 1u
    ^^^^^
)")) << res.Failure().reason.Str();
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
        EXPECT_THAT(res.Failure().reason.Str(), testing::HasSubstr("bitcast is not defined"));
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
        std::make_tuple(true, TypeBuilder<u32>, TypeBuilder<vec2<f16>>),
        std::make_tuple(true, TypeBuilder<i32>, TypeBuilder<u32>),
        std::make_tuple(true, TypeBuilder<i32>, TypeBuilder<f32>),
        std::make_tuple(true, TypeBuilder<i32>, TypeBuilder<vec2<f16>>),
        std::make_tuple(true, TypeBuilder<f32>, TypeBuilder<u32>),
        std::make_tuple(true, TypeBuilder<f32>, TypeBuilder<i32>),
        std::make_tuple(true, TypeBuilder<f32>, TypeBuilder<vec2<f16>>),
        std::make_tuple(false, TypeBuilder<u32>, TypeBuilder<f16>),
        std::make_tuple(false, TypeBuilder<i32>, TypeBuilder<f16>),
        std::make_tuple(false, TypeBuilder<f32>, TypeBuilder<f16>),
        std::make_tuple(false, TypeBuilder<f16>, TypeBuilder<u32>),
        std::make_tuple(false, TypeBuilder<f16>, TypeBuilder<i32>),
        std::make_tuple(false, TypeBuilder<f16>, TypeBuilder<f32>),
        std::make_tuple(false, TypeBuilder<f16>, TypeBuilder<vec2<f16>>),

        // Component-wise identity, sparsely (non-exhaustively) covering types and vector sizes
        std::make_tuple(true, TypeBuilder<vec2<u32>>, TypeBuilder<vec2<u32>>),
        std::make_tuple(true, TypeBuilder<vec3<i32>>, TypeBuilder<vec3<i32>>),
        std::make_tuple(true, TypeBuilder<vec4<f32>>, TypeBuilder<vec4<f32>>),
        std::make_tuple(true, TypeBuilder<vec3<f16>>, TypeBuilder<vec3<f16>>),
        std::make_tuple(false, TypeBuilder<vec2<u32>>, TypeBuilder<vec3<u32>>),
        std::make_tuple(false, TypeBuilder<vec2<u32>>, TypeBuilder<vec4<u32>>),
        std::make_tuple(false, TypeBuilder<vec3<i32>>, TypeBuilder<vec2<i32>>),
        std::make_tuple(false, TypeBuilder<vec3<i32>>, TypeBuilder<vec4<i32>>),
        std::make_tuple(false, TypeBuilder<vec4<f32>>, TypeBuilder<vec2<f32>>),
        std::make_tuple(false, TypeBuilder<vec4<f32>>, TypeBuilder<vec3<f32>>),

        // Component-wise reinterpretation, sparsely (non-exhaustively) covering types and
        // vector sizes
        std::make_tuple(true, TypeBuilder<vec2<u32>>, TypeBuilder<vec2<i32>>),
        std::make_tuple(true, TypeBuilder<vec3<u32>>, TypeBuilder<vec3<f32>>),
        std::make_tuple(true, TypeBuilder<vec4<i32>>, TypeBuilder<vec4<u32>>),
        std::make_tuple(true, TypeBuilder<vec3<i32>>, TypeBuilder<vec3<f32>>),
        std::make_tuple(true, TypeBuilder<vec3<f32>>, TypeBuilder<vec3<u32>>),
        std::make_tuple(true, TypeBuilder<vec2<f32>>, TypeBuilder<vec2<i32>>),
        std::make_tuple(true, TypeBuilder<vec2<u32>>, TypeBuilder<vec4<f16>>),
        std::make_tuple(true, TypeBuilder<vec2<i32>>, TypeBuilder<vec4<f16>>),
        std::make_tuple(true, TypeBuilder<vec2<f32>>, TypeBuilder<vec4<f16>>),
        std::make_tuple(false, TypeBuilder<vec4<u32>>, TypeBuilder<vec4<f16>>),
        std::make_tuple(false, TypeBuilder<vec2<i32>>, TypeBuilder<vec2<f16>>),
        std::make_tuple(false, TypeBuilder<vec4<f32>>, TypeBuilder<vec4<f16>>),
        std::make_tuple(false, TypeBuilder<f16>, TypeBuilder<u32>),
        std::make_tuple(false, TypeBuilder<f16>, TypeBuilder<i32>),
        std::make_tuple(false, TypeBuilder<f16>, TypeBuilder<f32>),
        std::make_tuple(false, TypeBuilder<f16>, TypeBuilder<vec2<f16>>)));

}  // namespace tint::core::ir

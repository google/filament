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

#include "src/tint/lang/core/ir/transform/vertex_pulling.h"

#include "gtest/gtest.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::transform {
namespace {

class MslWriter_VertexPullingTest : public core::ir::transform::TransformTest {
  protected:
    core::IOAttributes Location(uint32_t loc) {
        core::IOAttributes attrs;
        attrs.location = loc;
        return attrs;
    }

    core::IOAttributes VertexIndex() {
        core::IOAttributes attrs;
        attrs.builtin = core::BuiltinValue::kVertexIndex;
        return attrs;
    }

    core::IOAttributes InstanceIndex() {
        core::IOAttributes attrs;
        attrs.builtin = core::BuiltinValue::kInstanceIndex;
        return attrs;
    }
};

TEST_F(MslWriter_VertexPullingTest, NoModify_NoInputs) {
    auto* ep = b.Function("foo", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Zero<vec4<f32>>());
    });

    auto* src = R"(
%foo = @vertex func():vec4<f32> [@position] {
  $B1: {
    ret vec4<f32>(0.0f)
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    VertexPullingConfig cfg;
    Run(VertexPulling, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_VertexPullingTest, OneAttribute_Param) {
    auto* ep = b.Function("foo", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    auto* param = b.FunctionParam("input", ty.f32());
    param->SetLocation(0);
    ep->AppendParam(param);
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Construct<vec4<f32>>(param, 0_f, 0_f, 1_f));
    });

    auto* src = R"(
%foo = @vertex func(%input:f32 [@location(0)]):vec4<f32> [@position] {
  $B1: {
    %3:vec4<f32> = construct %input, 0.0f, 0.0f, 1.0f
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tint_vertex_buffer_0:ptr<storage, array<u32>, read> = var @binding_point(4, 0)
}

%foo = @vertex func(%tint_vertex_index:u32 [@vertex_index]):vec4<f32> [@position] {
  $B2: {
    %4:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %tint_vertex_index
    %5:u32 = load %4
    %6:f32 = bitcast %5
    %7:vec4<f32> = construct %6, 0.0f, 0.0f, 1.0f
    ret %7
  }
}
)";

    VertexPullingConfig cfg;
    cfg.pulling_group = 4u;
    cfg.vertex_state = {{{4, VertexStepMode::kVertex, {{VertexFormat::kFloat32, 0, 0}}}}};
    Run(VertexPulling, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_VertexPullingTest, OneAttribute_Struct) {
    auto* inputs =
        ty.Struct(mod.symbols.New("Inputs"), {
                                                 {mod.symbols.New("loc0"), ty.f32(), Location(0)},
                                             });

    auto* ep = b.Function("foo", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    auto* param = b.FunctionParam("input", inputs);
    ep->AppendParam(param);
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Construct<vec4<f32>>(b.Access<f32>(param, 0_u), 0_f, 0_f, 1_f));
    });

    auto* src = R"(
Inputs = struct @align(4) {
  loc0:f32 @offset(0), @location(0)
}

%foo = @vertex func(%input:Inputs):vec4<f32> [@position] {
  $B1: {
    %3:f32 = access %input, 0u
    %4:vec4<f32> = construct %3, 0.0f, 0.0f, 1.0f
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inputs = struct @align(4) {
  loc0:f32 @offset(0), @location(0)
}

$B1: {  # root
  %tint_vertex_buffer_0:ptr<storage, array<u32>, read> = var @binding_point(4, 0)
}

%foo = @vertex func(%tint_vertex_index:u32 [@vertex_index]):vec4<f32> [@position] {
  $B2: {
    %4:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %tint_vertex_index
    %5:u32 = load %4
    %6:f32 = bitcast %5
    %7:Inputs = construct %6
    %8:f32 = access %7, 0u
    %9:vec4<f32> = construct %8, 0.0f, 0.0f, 1.0f
    ret %9
  }
}
)";

    VertexPullingConfig cfg;
    cfg.pulling_group = 4u;
    cfg.vertex_state = {{{4, VertexStepMode::kVertex, {{VertexFormat::kFloat32, 0, 0}}}}};
    Run(VertexPulling, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_VertexPullingTest, OneAttribute_NonDefaultArrayStride) {
    auto* ep = b.Function("foo", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    auto* param = b.FunctionParam("input", ty.f32());
    param->SetLocation(0);
    ep->AppendParam(param);
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Construct<vec4<f32>>(param, 0_f, 0_f, 1_f));
    });

    auto* src = R"(
%foo = @vertex func(%input:f32 [@location(0)]):vec4<f32> [@position] {
  $B1: {
    %3:vec4<f32> = construct %input, 0.0f, 0.0f, 1.0f
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tint_vertex_buffer_0:ptr<storage, array<u32>, read> = var @binding_point(4, 0)
}

%foo = @vertex func(%tint_vertex_index:u32 [@vertex_index]):vec4<f32> [@position] {
  $B2: {
    %tint_vertex_buffer_0_base:u32 = mul %tint_vertex_index, 4u
    %5:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %tint_vertex_buffer_0_base
    %6:u32 = load %5
    %7:f32 = bitcast %6
    %8:vec4<f32> = construct %7, 0.0f, 0.0f, 1.0f
    ret %8
  }
}
)";

    VertexPullingConfig cfg;
    cfg.pulling_group = 4u;
    cfg.vertex_state = {{{16, VertexStepMode::kVertex, {{VertexFormat::kFloat32, 0, 0}}}}};
    Run(VertexPulling, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_VertexPullingTest, OneAttribute_NonDefaultArrayStrideAndOffset) {
    auto* ep = b.Function("foo", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    auto* param = b.FunctionParam("input", ty.f32());
    param->SetLocation(0);
    ep->AppendParam(param);
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Construct<vec4<f32>>(param, 0_f, 0_f, 1_f));
    });

    auto* src = R"(
%foo = @vertex func(%input:f32 [@location(0)]):vec4<f32> [@position] {
  $B1: {
    %3:vec4<f32> = construct %input, 0.0f, 0.0f, 1.0f
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tint_vertex_buffer_0:ptr<storage, array<u32>, read> = var @binding_point(4, 0)
}

%foo = @vertex func(%tint_vertex_index:u32 [@vertex_index]):vec4<f32> [@position] {
  $B2: {
    %tint_vertex_buffer_0_base:u32 = mul %tint_vertex_index, 4u
    %5:u32 = add %tint_vertex_buffer_0_base, 2u
    %6:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %5
    %7:u32 = load %6
    %8:f32 = bitcast %7
    %9:vec4<f32> = construct %8, 0.0f, 0.0f, 1.0f
    ret %9
  }
}
)";

    VertexPullingConfig cfg;
    cfg.pulling_group = 4u;
    cfg.vertex_state = {{{16, VertexStepMode::kVertex, {{VertexFormat::kFloat32, 8, 0}}}}};
    Run(VertexPulling, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_VertexPullingTest, InstanceStepMode) {
    auto* ep = b.Function("foo", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    auto* param = b.FunctionParam("input", ty.f32());
    param->SetLocation(0);
    ep->AppendParam(param);
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Construct<vec4<f32>>(param, 0_f, 0_f, 1_f));
    });

    auto* src = R"(
%foo = @vertex func(%input:f32 [@location(0)]):vec4<f32> [@position] {
  $B1: {
    %3:vec4<f32> = construct %input, 0.0f, 0.0f, 1.0f
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tint_vertex_buffer_0:ptr<storage, array<u32>, read> = var @binding_point(4, 0)
}

%foo = @vertex func(%tint_instance_index:u32 [@instance_index]):vec4<f32> [@position] {
  $B2: {
    %4:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %tint_instance_index
    %5:u32 = load %4
    %6:f32 = bitcast %5
    %7:vec4<f32> = construct %6, 0.0f, 0.0f, 1.0f
    ret %7
  }
}
)";

    VertexPullingConfig cfg;
    cfg.pulling_group = 4u;
    cfg.vertex_state = {{{4, VertexStepMode::kInstance, {{VertexFormat::kFloat32, 0, 0}}}}};
    Run(VertexPulling, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_VertexPullingTest, InstanceStepMode_WithArrayStride) {
    auto* ep = b.Function("foo", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    auto* param = b.FunctionParam("input", ty.f32());
    param->SetLocation(0);
    ep->AppendParam(param);
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Construct<vec4<f32>>(param, 0_f, 0_f, 1_f));
    });

    auto* src = R"(
%foo = @vertex func(%input:f32 [@location(0)]):vec4<f32> [@position] {
  $B1: {
    %3:vec4<f32> = construct %input, 0.0f, 0.0f, 1.0f
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tint_vertex_buffer_0:ptr<storage, array<u32>, read> = var @binding_point(4, 0)
}

%foo = @vertex func(%tint_instance_index:u32 [@instance_index]):vec4<f32> [@position] {
  $B2: {
    %tint_vertex_buffer_0_base:u32 = mul %tint_instance_index, 4u
    %5:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %tint_vertex_buffer_0_base
    %6:u32 = load %5
    %7:f32 = bitcast %6
    %8:vec4<f32> = construct %7, 0.0f, 0.0f, 1.0f
    ret %8
  }
}
)";

    VertexPullingConfig cfg;
    cfg.pulling_group = 4u;
    cfg.vertex_state = {{{16, VertexStepMode::kInstance, {{VertexFormat::kFloat32, 0, 0}}}}};
    Run(VertexPulling, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_VertexPullingTest, MultipleAttributes_SameBuffer_Params) {
    auto* ep = b.Function("foo", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    auto* input_0 = b.FunctionParam("input_0", ty.f32());
    auto* input_1 = b.FunctionParam("input_1", ty.u32());
    auto* input_2 = b.FunctionParam("input_2", ty.i32());
    input_0->SetLocation(0);
    input_1->SetLocation(1);
    input_2->SetLocation(2);
    ep->SetParams({input_0, input_1, input_2});
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    b.Append(ep->Block(), [&] {  //
        auto* f1 = b.Convert<f32>(input_1);
        auto* f2 = b.Convert<f32>(input_2);
        b.Return(ep, b.Construct<vec4<f32>>(input_0, f1, f2, 1_f));
    });

    auto* src = R"(
%foo = @vertex func(%input_0:f32 [@location(0)], %input_1:u32 [@location(1)], %input_2:i32 [@location(2)]):vec4<f32> [@position] {
  $B1: {
    %5:f32 = convert %input_1
    %6:f32 = convert %input_2
    %7:vec4<f32> = construct %input_0, %5, %6, 1.0f
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tint_vertex_buffer_0:ptr<storage, array<u32>, read> = var @binding_point(4, 0)
}

%foo = @vertex func(%tint_vertex_index:u32 [@vertex_index]):vec4<f32> [@position] {
  $B2: {
    %tint_vertex_buffer_0_base:u32 = mul %tint_vertex_index, 4u
    %5:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %tint_vertex_buffer_0_base
    %6:u32 = load %5
    %7:f32 = bitcast %6
    %8:u32 = add %tint_vertex_buffer_0_base, 1u
    %9:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %8
    %10:u32 = load %9
    %11:u32 = add %tint_vertex_buffer_0_base, 2u
    %12:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %11
    %13:u32 = load %12
    %14:i32 = bitcast %13
    %15:f32 = convert %10
    %16:f32 = convert %14
    %17:vec4<f32> = construct %7, %15, %16, 1.0f
    ret %17
  }
}
)";

    VertexPullingConfig cfg;
    cfg.pulling_group = 4u;
    cfg.vertex_state = {{{16,
                          VertexStepMode::kVertex,
                          {
                              {VertexFormat::kFloat32, 0, 0},
                              {VertexFormat::kUint32, 4, 1},
                              {VertexFormat::kSint32, 8, 2},
                          }}}};
    Run(VertexPulling, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_VertexPullingTest, MultipleAttributes_SameBuffer_Struct) {
    auto* inputs =
        ty.Struct(mod.symbols.New("Inputs"), {
                                                 {mod.symbols.New("loc0"), ty.f32(), Location(0)},
                                                 {mod.symbols.New("loc1"), ty.u32(), Location(1)},
                                                 {mod.symbols.New("loc2"), ty.i32(), Location(2)},
                                             });

    auto* ep = b.Function("foo", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    auto* param = b.FunctionParam("input", inputs);
    ep->AppendParam(param);
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    b.Append(ep->Block(), [&] {  //
        auto* f1 = b.Convert<f32>(b.Access<u32>(param, 1_u));
        auto* f2 = b.Convert<f32>(b.Access<i32>(param, 2_u));
        b.Return(ep, b.Construct<vec4<f32>>(b.Access<f32>(param, 0_u), f1, f2, 1_f));
    });

    auto* src = R"(
Inputs = struct @align(4) {
  loc0:f32 @offset(0), @location(0)
  loc1:u32 @offset(4), @location(1)
  loc2:i32 @offset(8), @location(2)
}

%foo = @vertex func(%input:Inputs):vec4<f32> [@position] {
  $B1: {
    %3:u32 = access %input, 1u
    %4:f32 = convert %3
    %5:i32 = access %input, 2u
    %6:f32 = convert %5
    %7:f32 = access %input, 0u
    %8:vec4<f32> = construct %7, %4, %6, 1.0f
    ret %8
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inputs = struct @align(4) {
  loc0:f32 @offset(0), @location(0)
  loc1:u32 @offset(4), @location(1)
  loc2:i32 @offset(8), @location(2)
}

$B1: {  # root
  %tint_vertex_buffer_0:ptr<storage, array<u32>, read> = var @binding_point(4, 0)
}

%foo = @vertex func(%tint_vertex_index:u32 [@vertex_index]):vec4<f32> [@position] {
  $B2: {
    %tint_vertex_buffer_0_base:u32 = mul %tint_vertex_index, 4u
    %5:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %tint_vertex_buffer_0_base
    %6:u32 = load %5
    %7:f32 = bitcast %6
    %8:u32 = add %tint_vertex_buffer_0_base, 1u
    %9:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %8
    %10:u32 = load %9
    %11:u32 = add %tint_vertex_buffer_0_base, 2u
    %12:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %11
    %13:u32 = load %12
    %14:i32 = bitcast %13
    %15:Inputs = construct %7, %10, %14
    %16:u32 = access %15, 1u
    %17:f32 = convert %16
    %18:i32 = access %15, 2u
    %19:f32 = convert %18
    %20:f32 = access %15, 0u
    %21:vec4<f32> = construct %20, %17, %19, 1.0f
    ret %21
  }
}
)";

    VertexPullingConfig cfg;
    cfg.pulling_group = 4u;
    cfg.vertex_state = {{{16,
                          VertexStepMode::kVertex,
                          {
                              {VertexFormat::kFloat32, 0, 0},
                              {VertexFormat::kUint32, 4, 1},
                              {VertexFormat::kSint32, 8, 2},
                          }}}};
    Run(VertexPulling, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_VertexPullingTest, MultipleAttributes_DifferentBuffers) {
    auto* ep = b.Function("foo", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    auto* input_0 = b.FunctionParam("input_0", ty.f32());
    auto* input_1 = b.FunctionParam("input_1", ty.u32());
    auto* input_2 = b.FunctionParam("input_2", ty.i32());
    input_0->SetLocation(0);
    input_1->SetLocation(1);
    input_2->SetLocation(2);
    ep->SetParams({input_0, input_1, input_2});
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    b.Append(ep->Block(), [&] {  //
        auto* f1 = b.Convert<f32>(input_1);
        auto* f2 = b.Convert<f32>(input_2);
        b.Return(ep, b.Construct<vec4<f32>>(input_0, f1, f2, 1_f));
    });

    auto* src = R"(
%foo = @vertex func(%input_0:f32 [@location(0)], %input_1:u32 [@location(1)], %input_2:i32 [@location(2)]):vec4<f32> [@position] {
  $B1: {
    %5:f32 = convert %input_1
    %6:f32 = convert %input_2
    %7:vec4<f32> = construct %input_0, %5, %6, 1.0f
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tint_vertex_buffer_0:ptr<storage, array<u32>, read> = var @binding_point(4, 0)
  %tint_vertex_buffer_1:ptr<storage, array<u32>, read> = var @binding_point(4, 1)
  %tint_vertex_buffer_2:ptr<storage, array<u32>, read> = var @binding_point(4, 2)
}

%foo = @vertex func(%tint_vertex_index:u32 [@vertex_index], %tint_instance_index:u32 [@instance_index]):vec4<f32> [@position] {
  $B2: {
    %tint_vertex_buffer_1_base:u32 = mul %tint_instance_index, 2u
    %tint_vertex_buffer_2_base:u32 = mul %tint_vertex_index, 4u
    %9:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %tint_vertex_index
    %10:u32 = load %9
    %11:f32 = bitcast %10
    %12:ptr<storage, u32, read> = access %tint_vertex_buffer_1, %tint_vertex_buffer_1_base
    %13:u32 = load %12
    %14:u32 = add %tint_vertex_buffer_2_base, 2u
    %15:ptr<storage, u32, read> = access %tint_vertex_buffer_2, %14
    %16:u32 = load %15
    %17:i32 = bitcast %16
    %18:f32 = convert %13
    %19:f32 = convert %17
    %20:vec4<f32> = construct %11, %18, %19, 1.0f
    ret %20
  }
}
)";

    VertexPullingConfig cfg;
    cfg.pulling_group = 4u;
    cfg.vertex_state = {{
        {4,
         VertexStepMode::kVertex,
         {
             {VertexFormat::kFloat32, 0, 0},
         }},
        {8,
         VertexStepMode::kInstance,
         {
             {VertexFormat::kUint32, 0, 1},
         }},
        {16,
         VertexStepMode::kVertex,
         {
             {VertexFormat::kSint32, 8, 2},
         }},
    }};
    Run(VertexPulling, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_VertexPullingTest, ExistingVertexAndIndexAttribute_Params) {
    auto* inputs =
        ty.Struct(mod.symbols.New("Inputs"), {
                                                 {mod.symbols.New("loc0"), ty.f32(), Location(0)},
                                                 {mod.symbols.New("loc1"), ty.u32(), Location(1)},
                                                 {mod.symbols.New("loc2"), ty.i32(), Location(2)},
                                             });

    auto* ep = b.Function("foo", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    auto* param = b.FunctionParam("input", inputs);
    auto* vertex_index = b.FunctionParam("vertex_index", ty.u32());
    auto* instance_index = b.FunctionParam("instance_index", ty.u32());
    vertex_index->SetBuiltin(core::BuiltinValue::kVertexIndex);
    instance_index->SetBuiltin(core::BuiltinValue::kInstanceIndex);
    ep->SetParams({param, vertex_index, instance_index});
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    b.Append(ep->Block(), [&] {  //
        auto* f1 = b.Convert<f32>(b.Access<u32>(param, 1_u));
        auto* f2 = b.Convert<f32>(b.Access<i32>(param, 2_u));
        b.Let("idx_add", b.Add<u32>(vertex_index, instance_index));
        b.Return(ep, b.Construct<vec4<f32>>(b.Access<f32>(param, 0_u), f1, f2, 1_f));
    });

    auto* src = R"(
Inputs = struct @align(4) {
  loc0:f32 @offset(0), @location(0)
  loc1:u32 @offset(4), @location(1)
  loc2:i32 @offset(8), @location(2)
}

%foo = @vertex func(%input:Inputs, %vertex_index:u32 [@vertex_index], %instance_index:u32 [@instance_index]):vec4<f32> [@position] {
  $B1: {
    %5:u32 = access %input, 1u
    %6:f32 = convert %5
    %7:i32 = access %input, 2u
    %8:f32 = convert %7
    %9:u32 = add %vertex_index, %instance_index
    %idx_add:u32 = let %9
    %11:f32 = access %input, 0u
    %12:vec4<f32> = construct %11, %6, %8, 1.0f
    ret %12
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inputs = struct @align(4) {
  loc0:f32 @offset(0), @location(0)
  loc1:u32 @offset(4), @location(1)
  loc2:i32 @offset(8), @location(2)
}

$B1: {  # root
  %tint_vertex_buffer_0:ptr<storage, array<u32>, read> = var @binding_point(4, 0)
  %tint_vertex_buffer_1:ptr<storage, array<u32>, read> = var @binding_point(4, 1)
  %tint_vertex_buffer_2:ptr<storage, array<u32>, read> = var @binding_point(4, 2)
}

%foo = @vertex func(%tint_vertex_index:u32 [@vertex_index], %tint_instance_index:u32 [@instance_index]):vec4<f32> [@position] {
  $B2: {
    %tint_vertex_buffer_1_base:u32 = mul %tint_instance_index, 2u
    %tint_vertex_buffer_2_base:u32 = mul %tint_vertex_index, 4u
    %9:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %tint_vertex_index
    %10:u32 = load %9
    %11:f32 = bitcast %10
    %12:ptr<storage, u32, read> = access %tint_vertex_buffer_1, %tint_vertex_buffer_1_base
    %13:u32 = load %12
    %14:u32 = add %tint_vertex_buffer_2_base, 2u
    %15:ptr<storage, u32, read> = access %tint_vertex_buffer_2, %14
    %16:u32 = load %15
    %17:i32 = bitcast %16
    %18:Inputs = construct %11, %13, %17
    %19:u32 = access %18, 1u
    %20:f32 = convert %19
    %21:i32 = access %18, 2u
    %22:f32 = convert %21
    %23:u32 = add %tint_vertex_index, %tint_instance_index
    %idx_add:u32 = let %23
    %25:f32 = access %18, 0u
    %26:vec4<f32> = construct %25, %20, %22, 1.0f
    ret %26
  }
}
)";

    VertexPullingConfig cfg;
    cfg.pulling_group = 4u;
    cfg.vertex_state = {{
        {4,
         VertexStepMode::kVertex,
         {
             {VertexFormat::kFloat32, 0, 0},
         }},
        {8,
         VertexStepMode::kInstance,
         {
             {VertexFormat::kUint32, 0, 1},
         }},
        {16,
         VertexStepMode::kVertex,
         {
             {VertexFormat::kSint32, 8, 2},
         }},
    }};
    Run(VertexPulling, cfg);

    EXPECT_EQ(expect, str());
}

// Test user-declared indices being used with multiple buffers, without strides and offsets.
// See crbug.com/390568194.
TEST_F(MslWriter_VertexPullingTest, ExistingVertexAndIndexAttribute_Params_MultipleBuffers) {
    auto* ep = b.Function("foo", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    auto* vertex_index = b.FunctionParam("vertex_index", ty.u32());
    auto* instance_index = b.FunctionParam("instance_index", ty.u32());
    auto* loc0 = b.FunctionParam("loc0", ty.f32());
    auto* loc1 = b.FunctionParam("loc1", ty.f32());
    auto* loc2 = b.FunctionParam("loc2", ty.f32());
    auto* loc3 = b.FunctionParam("loc3", ty.f32());
    loc0->SetLocation(0);
    loc1->SetLocation(1);
    loc2->SetLocation(2);
    loc3->SetLocation(3);
    vertex_index->SetBuiltin(core::BuiltinValue::kVertexIndex);
    instance_index->SetBuiltin(core::BuiltinValue::kInstanceIndex);
    ep->SetParams({vertex_index, instance_index, loc0, loc1, loc2, loc3});
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    b.Append(ep->Block(), [&] {  //
        b.Let("idx_add", b.Add<u32>(vertex_index, instance_index));
        b.Return(ep, b.Construct<vec4<f32>>(loc0, loc1, loc2, loc3));
    });

    auto* src = R"(
%foo = @vertex func(%vertex_index:u32 [@vertex_index], %instance_index:u32 [@instance_index], %loc0:f32 [@location(0)], %loc1:f32 [@location(1)], %loc2:f32 [@location(2)], %loc3:f32 [@location(3)]):vec4<f32> [@position] {
  $B1: {
    %8:u32 = add %vertex_index, %instance_index
    %idx_add:u32 = let %8
    %10:vec4<f32> = construct %loc0, %loc1, %loc2, %loc3
    ret %10
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tint_vertex_buffer_0:ptr<storage, array<u32>, read> = var @binding_point(4, 0)
  %tint_vertex_buffer_1:ptr<storage, array<u32>, read> = var @binding_point(4, 1)
  %tint_vertex_buffer_2:ptr<storage, array<u32>, read> = var @binding_point(4, 2)
  %tint_vertex_buffer_3:ptr<storage, array<u32>, read> = var @binding_point(4, 3)
}

%foo = @vertex func(%tint_vertex_index:u32 [@vertex_index], %tint_instance_index:u32 [@instance_index]):vec4<f32> [@position] {
  $B2: {
    %8:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %tint_vertex_index
    %9:u32 = load %8
    %10:f32 = bitcast %9
    %11:ptr<storage, u32, read> = access %tint_vertex_buffer_1, %tint_instance_index
    %12:u32 = load %11
    %13:f32 = bitcast %12
    %14:ptr<storage, u32, read> = access %tint_vertex_buffer_2, %tint_vertex_index
    %15:u32 = load %14
    %16:f32 = bitcast %15
    %17:ptr<storage, u32, read> = access %tint_vertex_buffer_3, %tint_instance_index
    %18:u32 = load %17
    %19:f32 = bitcast %18
    %20:u32 = add %tint_vertex_index, %tint_instance_index
    %idx_add:u32 = let %20
    %22:vec4<f32> = construct %10, %13, %16, %19
    ret %22
  }
}
)";

    VertexPullingConfig cfg;
    cfg.pulling_group = 4u;
    cfg.vertex_state = {{
        {4,
         VertexStepMode::kVertex,
         {
             {VertexFormat::kFloat32, 0, 0},
         }},
        {4,
         VertexStepMode::kInstance,
         {
             {VertexFormat::kFloat32, 0, 1},
         }},
        {4,
         VertexStepMode::kVertex,
         {
             {VertexFormat::kFloat32, 0, 2},
         }},
        {4,
         VertexStepMode::kInstance,
         {
             {VertexFormat::kFloat32, 0, 3},
         }},
    }};
    Run(VertexPulling, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_VertexPullingTest, ExistingVertexIndex_NotUsedByBuffer) {
    auto* ep = b.Function("foo", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    auto* vertex_index = b.FunctionParam("vertex_index", ty.u32());
    auto* instance_index = b.FunctionParam("instance_index", ty.u32());
    auto* loc0 = b.FunctionParam("loc0", ty.f32());
    loc0->SetLocation(0);
    vertex_index->SetBuiltin(core::BuiltinValue::kVertexIndex);
    instance_index->SetBuiltin(core::BuiltinValue::kInstanceIndex);
    ep->SetParams({vertex_index, instance_index, loc0});
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    b.Append(ep->Block(), [&] {  //
        b.Let("idx_add", b.Add<u32>(vertex_index, instance_index));
        b.Return(ep, b.Construct<vec4<f32>>(loc0));
    });

    auto* src = R"(
%foo = @vertex func(%vertex_index:u32 [@vertex_index], %instance_index:u32 [@instance_index], %loc0:f32 [@location(0)]):vec4<f32> [@position] {
  $B1: {
    %5:u32 = add %vertex_index, %instance_index
    %idx_add:u32 = let %5
    %7:vec4<f32> = construct %loc0
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tint_vertex_buffer_0:ptr<storage, array<u32>, read> = var @binding_point(4, 0)
}

%foo = @vertex func(%vertex_index:u32 [@vertex_index], %tint_instance_index:u32 [@instance_index]):vec4<f32> [@position] {
  $B2: {
    %5:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %tint_instance_index
    %6:u32 = load %5
    %7:f32 = bitcast %6
    %8:u32 = add %vertex_index, %tint_instance_index
    %idx_add:u32 = let %8
    %10:vec4<f32> = construct %7
    ret %10
  }
}
)";

    VertexPullingConfig cfg;
    cfg.pulling_group = 4u;
    cfg.vertex_state = {{
        {4,
         VertexStepMode::kInstance,
         {
             {VertexFormat::kFloat32, 0, 0},
         }},
    }};
    Run(VertexPulling, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_VertexPullingTest, ExistingInstanceIndex_NotUsedByBuffer) {
    auto* ep = b.Function("foo", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    auto* vertex_index = b.FunctionParam("vertex_index", ty.u32());
    auto* instance_index = b.FunctionParam("instance_index", ty.u32());
    auto* loc0 = b.FunctionParam("loc0", ty.f32());
    loc0->SetLocation(0);
    vertex_index->SetBuiltin(core::BuiltinValue::kVertexIndex);
    instance_index->SetBuiltin(core::BuiltinValue::kInstanceIndex);
    ep->SetParams({vertex_index, instance_index, loc0});
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    b.Append(ep->Block(), [&] {  //
        b.Let("idx_add", b.Add<u32>(vertex_index, instance_index));
        b.Return(ep, b.Construct<vec4<f32>>(loc0));
    });

    auto* src = R"(
%foo = @vertex func(%vertex_index:u32 [@vertex_index], %instance_index:u32 [@instance_index], %loc0:f32 [@location(0)]):vec4<f32> [@position] {
  $B1: {
    %5:u32 = add %vertex_index, %instance_index
    %idx_add:u32 = let %5
    %7:vec4<f32> = construct %loc0
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tint_vertex_buffer_0:ptr<storage, array<u32>, read> = var @binding_point(4, 0)
}

%foo = @vertex func(%instance_index:u32 [@instance_index], %tint_vertex_index:u32 [@vertex_index]):vec4<f32> [@position] {
  $B2: {
    %5:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %tint_vertex_index
    %6:u32 = load %5
    %7:f32 = bitcast %6
    %8:u32 = add %tint_vertex_index, %instance_index
    %idx_add:u32 = let %8
    %10:vec4<f32> = construct %7
    ret %10
  }
}
)";

    VertexPullingConfig cfg;
    cfg.pulling_group = 4u;
    cfg.vertex_state = {{
        {4,
         VertexStepMode::kVertex,
         {
             {VertexFormat::kFloat32, 0, 0},
         }},
    }};
    Run(VertexPulling, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_VertexPullingTest, ExistingVertexAndIndexAttribute_SameStruct) {
    auto* inputs = ty.Struct(mod.symbols.New("Inputs"),
                             {
                                 {mod.symbols.New("loc0"), ty.f32(), Location(0)},
                                 {mod.symbols.New("loc1"), ty.u32(), Location(1)},
                                 {mod.symbols.New("loc2"), ty.i32(), Location(2)},
                                 {mod.symbols.New("vertex_index"), ty.u32(), VertexIndex()},
                                 {mod.symbols.New("instance_index"), ty.u32(), InstanceIndex()},
                             });

    auto* ep = b.Function("foo", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    auto* param = b.FunctionParam("input", inputs);
    ep->SetParams({param});
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    b.Append(ep->Block(), [&] {  //
        auto* f1 = b.Convert<f32>(b.Access<u32>(param, 1_u));
        auto* f2 = b.Convert<f32>(b.Access<i32>(param, 2_u));
        auto* vertex_index = b.Access<u32>(param, 3_u);
        auto* instance_index = b.Access<u32>(param, 4_u);
        b.Let("idx_add", b.Add<u32>(vertex_index, instance_index));
        b.Return(ep, b.Construct<vec4<f32>>(b.Access<f32>(param, 0_u), f1, f2, 1_f));
    });

    auto* src = R"(
Inputs = struct @align(4) {
  loc0:f32 @offset(0), @location(0)
  loc1:u32 @offset(4), @location(1)
  loc2:i32 @offset(8), @location(2)
  vertex_index:u32 @offset(12), @builtin(vertex_index)
  instance_index:u32 @offset(16), @builtin(instance_index)
}

%foo = @vertex func(%input:Inputs):vec4<f32> [@position] {
  $B1: {
    %3:u32 = access %input, 1u
    %4:f32 = convert %3
    %5:i32 = access %input, 2u
    %6:f32 = convert %5
    %7:u32 = access %input, 3u
    %8:u32 = access %input, 4u
    %9:u32 = add %7, %8
    %idx_add:u32 = let %9
    %11:f32 = access %input, 0u
    %12:vec4<f32> = construct %11, %4, %6, 1.0f
    ret %12
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inputs = struct @align(4) {
  loc0:f32 @offset(0), @location(0)
  loc1:u32 @offset(4), @location(1)
  loc2:i32 @offset(8), @location(2)
  vertex_index:u32 @offset(12), @builtin(vertex_index)
  instance_index:u32 @offset(16), @builtin(instance_index)
}

$B1: {  # root
  %tint_vertex_buffer_0:ptr<storage, array<u32>, read> = var @binding_point(4, 0)
  %tint_vertex_buffer_1:ptr<storage, array<u32>, read> = var @binding_point(4, 1)
  %tint_vertex_buffer_2:ptr<storage, array<u32>, read> = var @binding_point(4, 2)
}

%foo = @vertex func(%tint_vertex_index:u32 [@vertex_index], %tint_instance_index:u32 [@instance_index]):vec4<f32> [@position] {
  $B2: {
    %tint_vertex_buffer_1_base:u32 = mul %tint_instance_index, 2u
    %tint_vertex_buffer_2_base:u32 = mul %tint_vertex_index, 4u
    %9:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %tint_vertex_index
    %10:u32 = load %9
    %11:f32 = bitcast %10
    %12:ptr<storage, u32, read> = access %tint_vertex_buffer_1, %tint_vertex_buffer_1_base
    %13:u32 = load %12
    %14:u32 = add %tint_vertex_buffer_2_base, 2u
    %15:ptr<storage, u32, read> = access %tint_vertex_buffer_2, %14
    %16:u32 = load %15
    %17:i32 = bitcast %16
    %18:Inputs = construct %11, %13, %17, %tint_vertex_index, %tint_instance_index
    %19:u32 = access %18, 1u
    %20:f32 = convert %19
    %21:i32 = access %18, 2u
    %22:f32 = convert %21
    %23:u32 = access %18, 3u
    %24:u32 = access %18, 4u
    %25:u32 = add %23, %24
    %idx_add:u32 = let %25
    %27:f32 = access %18, 0u
    %28:vec4<f32> = construct %27, %20, %22, 1.0f
    ret %28
  }
}
)";

    VertexPullingConfig cfg;
    cfg.pulling_group = 4u;
    cfg.vertex_state = {{
        {4,
         VertexStepMode::kVertex,
         {
             {VertexFormat::kFloat32, 0, 0},
         }},
        {8,
         VertexStepMode::kInstance,
         {
             {VertexFormat::kUint32, 0, 1},
         }},
        {16,
         VertexStepMode::kVertex,
         {
             {VertexFormat::kSint32, 8, 2},
         }},
    }};
    Run(VertexPulling, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_VertexPullingTest, ExistingVertexAndIndexAttribute_DifferentStruct) {
    auto* inputs =
        ty.Struct(mod.symbols.New("Inputs"), {
                                                 {mod.symbols.New("loc0"), ty.f32(), Location(0)},
                                                 {mod.symbols.New("loc1"), ty.u32(), Location(1)},
                                                 {mod.symbols.New("loc2"), ty.i32(), Location(2)},
                                             });
    auto* indices_ty = ty.Struct(mod.symbols.New("Indices"),
                                 {
                                     {mod.symbols.New("vertex_index"), ty.u32(), VertexIndex()},
                                     {mod.symbols.New("instance_index"), ty.u32(), InstanceIndex()},
                                 });

    auto* ep = b.Function("foo", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    auto* param = b.FunctionParam("input", inputs);
    auto* indices = b.FunctionParam("indices", indices_ty);
    ep->SetParams({param, indices});
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    b.Append(ep->Block(), [&] {  //
        auto* f1 = b.Convert<f32>(b.Access<u32>(param, 1_u));
        auto* f2 = b.Convert<f32>(b.Access<i32>(param, 2_u));
        auto* vertex_index = b.Access<u32>(indices, 0_u);
        auto* instance_index = b.Access<u32>(indices, 1_u);
        b.Let("idx_add", b.Add<u32>(vertex_index, instance_index));
        b.Return(ep, b.Construct<vec4<f32>>(b.Access<f32>(param, 0_u), f1, f2, 1_f));
    });

    auto* src = R"(
Inputs = struct @align(4) {
  loc0:f32 @offset(0), @location(0)
  loc1:u32 @offset(4), @location(1)
  loc2:i32 @offset(8), @location(2)
}

Indices = struct @align(4) {
  vertex_index:u32 @offset(0), @builtin(vertex_index)
  instance_index:u32 @offset(4), @builtin(instance_index)
}

%foo = @vertex func(%input:Inputs, %indices:Indices):vec4<f32> [@position] {
  $B1: {
    %4:u32 = access %input, 1u
    %5:f32 = convert %4
    %6:i32 = access %input, 2u
    %7:f32 = convert %6
    %8:u32 = access %indices, 0u
    %9:u32 = access %indices, 1u
    %10:u32 = add %8, %9
    %idx_add:u32 = let %10
    %12:f32 = access %input, 0u
    %13:vec4<f32> = construct %12, %5, %7, 1.0f
    ret %13
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inputs = struct @align(4) {
  loc0:f32 @offset(0), @location(0)
  loc1:u32 @offset(4), @location(1)
  loc2:i32 @offset(8), @location(2)
}

Indices = struct @align(4) {
  vertex_index:u32 @offset(0), @builtin(vertex_index)
  instance_index:u32 @offset(4), @builtin(instance_index)
}

$B1: {  # root
  %tint_vertex_buffer_0:ptr<storage, array<u32>, read> = var @binding_point(4, 0)
  %tint_vertex_buffer_1:ptr<storage, array<u32>, read> = var @binding_point(4, 1)
  %tint_vertex_buffer_2:ptr<storage, array<u32>, read> = var @binding_point(4, 2)
}

%foo = @vertex func(%tint_vertex_index:u32 [@vertex_index], %tint_instance_index:u32 [@instance_index]):vec4<f32> [@position] {
  $B2: {
    %tint_vertex_buffer_1_base:u32 = mul %tint_instance_index, 2u
    %tint_vertex_buffer_2_base:u32 = mul %tint_vertex_index, 4u
    %9:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %tint_vertex_index
    %10:u32 = load %9
    %11:f32 = bitcast %10
    %12:ptr<storage, u32, read> = access %tint_vertex_buffer_1, %tint_vertex_buffer_1_base
    %13:u32 = load %12
    %14:u32 = add %tint_vertex_buffer_2_base, 2u
    %15:ptr<storage, u32, read> = access %tint_vertex_buffer_2, %14
    %16:u32 = load %15
    %17:i32 = bitcast %16
    %18:Inputs = construct %11, %13, %17
    %19:Indices = construct %tint_vertex_index, %tint_instance_index
    %20:u32 = access %18, 1u
    %21:f32 = convert %20
    %22:i32 = access %18, 2u
    %23:f32 = convert %22
    %24:u32 = access %19, 0u
    %25:u32 = access %19, 1u
    %26:u32 = add %24, %25
    %idx_add:u32 = let %26
    %28:f32 = access %18, 0u
    %29:vec4<f32> = construct %28, %21, %23, 1.0f
    ret %29
  }
}
)";

    VertexPullingConfig cfg;
    cfg.pulling_group = 4u;
    cfg.vertex_state = {{
        {4,
         VertexStepMode::kVertex,
         {
             {VertexFormat::kFloat32, 0, 0},
         }},
        {8,
         VertexStepMode::kInstance,
         {
             {VertexFormat::kUint32, 0, 1},
         }},
        {16,
         VertexStepMode::kVertex,
         {
             {VertexFormat::kSint32, 8, 2},
         }},
    }};
    Run(VertexPulling, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_VertexPullingTest, Formats_F32) {
    auto* inputs =
        ty.Struct(mod.symbols.New("Inputs"),
                  {
                      {mod.symbols.New("unorm8"), ty.f32(), Location(0)},                  //
                      {mod.symbols.New("unorm8x2"), ty.vec2<f32>(), Location(1)},          //
                      {mod.symbols.New("unorm8x4"), ty.vec4<f32>(), Location(2)},          //
                      {mod.symbols.New("snorm8"), ty.f32(), Location(3)},                  //
                      {mod.symbols.New("snorm8x2"), ty.vec2<f32>(), Location(4)},          //
                      {mod.symbols.New("snorm8x4"), ty.vec4<f32>(), Location(5)},          //
                      {mod.symbols.New("unorm16"), ty.f32(), Location(6)},                 //
                      {mod.symbols.New("unorm16x2"), ty.vec2<f32>(), Location(7)},         //
                      {mod.symbols.New("unorm16x4"), ty.vec4<f32>(), Location(8)},         //
                      {mod.symbols.New("snorm16"), ty.f32(), Location(9)},                 //
                      {mod.symbols.New("snorm16x2"), ty.vec2<f32>(), Location(10)},        //
                      {mod.symbols.New("snorm16x4"), ty.vec4<f32>(), Location(11)},        //
                      {mod.symbols.New("float16"), ty.f32(), Location(12)},                //
                      {mod.symbols.New("float16x2"), ty.vec2<f32>(), Location(13)},        //
                      {mod.symbols.New("float16x4"), ty.vec4<f32>(), Location(14)},        //
                      {mod.symbols.New("float32"), ty.f32(), Location(15)},                //
                      {mod.symbols.New("float32x2"), ty.vec2<f32>(), Location(16)},        //
                      {mod.symbols.New("float32x3"), ty.vec3<f32>(), Location(17)},        //
                      {mod.symbols.New("float32x4"), ty.vec4<f32>(), Location(18)},        //
                      {mod.symbols.New("unorm10_10_10_2"), ty.vec4<f32>(), Location(19)},  //
                      {mod.symbols.New("unorm8x4_bgra"), ty.vec4<f32>(), Location(20)},    //
                  });

    auto* ep = b.Function("foo", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    auto* param = b.FunctionParam("input", inputs);
    ep->AppendParam(param);
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    b.Append(ep->Block(), [&] {  //
        mod.SetName(b.Access(ty.f32(), param, 0_u), "unorm8");
        mod.SetName(b.Access(ty.vec2<f32>(), param, 1_u), "unorm8x2");
        mod.SetName(b.Access(ty.vec4<f32>(), param, 2_u), "unorm8x4");
        mod.SetName(b.Access(ty.f32(), param, 3_u), "snorm8");
        mod.SetName(b.Access(ty.vec2<f32>(), param, 4_u), "snorm8x2");
        mod.SetName(b.Access(ty.vec4<f32>(), param, 5_u), "snorm8x4");
        mod.SetName(b.Access(ty.f32(), param, 6_u), "unorm16");
        mod.SetName(b.Access(ty.vec2<f32>(), param, 7_u), "unorm16x2");
        mod.SetName(b.Access(ty.vec4<f32>(), param, 8_u), "unorm16x4");
        mod.SetName(b.Access(ty.f32(), param, 9_u), "snorm16");
        mod.SetName(b.Access(ty.vec2<f32>(), param, 10_u), "snorm16x2");
        mod.SetName(b.Access(ty.vec4<f32>(), param, 11_u), "snorm16x4");
        mod.SetName(b.Access(ty.f32(), param, 12_u), "float16");
        mod.SetName(b.Access(ty.vec2<f32>(), param, 13_u), "float16x2");
        mod.SetName(b.Access(ty.vec4<f32>(), param, 14_u), "float16x4");
        mod.SetName(b.Access(ty.f32(), param, 15_u), "float32");
        mod.SetName(b.Access(ty.vec2<f32>(), param, 16_u), "float32x2");
        mod.SetName(b.Access(ty.vec3<f32>(), param, 17_u), "float32x3");
        mod.SetName(b.Access(ty.vec4<f32>(), param, 18_u), "float32x4");
        mod.SetName(b.Access(ty.vec4<f32>(), param, 19_u), "unorm10_10_10_2");
        mod.SetName(b.Access(ty.vec4<f32>(), param, 20_u), "unorm8x4_bgra");
        b.Return(ep, b.Zero<vec4<f32>>());
    });

    auto* src = R"(
Inputs = struct @align(16) {
  unorm8:f32 @offset(0), @location(0)
  unorm8x2:vec2<f32> @offset(8), @location(1)
  unorm8x4:vec4<f32> @offset(16), @location(2)
  snorm8:f32 @offset(32), @location(3)
  snorm8x2:vec2<f32> @offset(40), @location(4)
  snorm8x4:vec4<f32> @offset(48), @location(5)
  unorm16:f32 @offset(64), @location(6)
  unorm16x2:vec2<f32> @offset(72), @location(7)
  unorm16x4:vec4<f32> @offset(80), @location(8)
  snorm16:f32 @offset(96), @location(9)
  snorm16x2:vec2<f32> @offset(104), @location(10)
  snorm16x4:vec4<f32> @offset(112), @location(11)
  float16:f32 @offset(128), @location(12)
  float16x2:vec2<f32> @offset(136), @location(13)
  float16x4:vec4<f32> @offset(144), @location(14)
  float32:f32 @offset(160), @location(15)
  float32x2:vec2<f32> @offset(168), @location(16)
  float32x3:vec3<f32> @offset(176), @location(17)
  float32x4:vec4<f32> @offset(192), @location(18)
  unorm10_10_10_2:vec4<f32> @offset(208), @location(19)
  unorm8x4_bgra:vec4<f32> @offset(224), @location(20)
}

%foo = @vertex func(%input:Inputs):vec4<f32> [@position] {
  $B1: {
    %unorm8:f32 = access %input, 0u
    %unorm8x2:vec2<f32> = access %input, 1u
    %unorm8x4:vec4<f32> = access %input, 2u
    %snorm8:f32 = access %input, 3u
    %snorm8x2:vec2<f32> = access %input, 4u
    %snorm8x4:vec4<f32> = access %input, 5u
    %unorm16:f32 = access %input, 6u
    %unorm16x2:vec2<f32> = access %input, 7u
    %unorm16x4:vec4<f32> = access %input, 8u
    %snorm16:f32 = access %input, 9u
    %snorm16x2:vec2<f32> = access %input, 10u
    %snorm16x4:vec4<f32> = access %input, 11u
    %float16:f32 = access %input, 12u
    %float16x2:vec2<f32> = access %input, 13u
    %float16x4:vec4<f32> = access %input, 14u
    %float32:f32 = access %input, 15u
    %float32x2:vec2<f32> = access %input, 16u
    %float32x3:vec3<f32> = access %input, 17u
    %float32x4:vec4<f32> = access %input, 18u
    %unorm10_10_10_2:vec4<f32> = access %input, 19u
    %unorm8x4_bgra:vec4<f32> = access %input, 20u
    ret vec4<f32>(0.0f)
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inputs = struct @align(16) {
  unorm8:f32 @offset(0), @location(0)
  unorm8x2:vec2<f32> @offset(8), @location(1)
  unorm8x4:vec4<f32> @offset(16), @location(2)
  snorm8:f32 @offset(32), @location(3)
  snorm8x2:vec2<f32> @offset(40), @location(4)
  snorm8x4:vec4<f32> @offset(48), @location(5)
  unorm16:f32 @offset(64), @location(6)
  unorm16x2:vec2<f32> @offset(72), @location(7)
  unorm16x4:vec4<f32> @offset(80), @location(8)
  snorm16:f32 @offset(96), @location(9)
  snorm16x2:vec2<f32> @offset(104), @location(10)
  snorm16x4:vec4<f32> @offset(112), @location(11)
  float16:f32 @offset(128), @location(12)
  float16x2:vec2<f32> @offset(136), @location(13)
  float16x4:vec4<f32> @offset(144), @location(14)
  float32:f32 @offset(160), @location(15)
  float32x2:vec2<f32> @offset(168), @location(16)
  float32x3:vec3<f32> @offset(176), @location(17)
  float32x4:vec4<f32> @offset(192), @location(18)
  unorm10_10_10_2:vec4<f32> @offset(208), @location(19)
  unorm8x4_bgra:vec4<f32> @offset(224), @location(20)
}

$B1: {  # root
  %tint_vertex_buffer_0:ptr<storage, array<u32>, read> = var @binding_point(4, 0)
}

%foo = @vertex func(%tint_vertex_index:u32 [@vertex_index]):vec4<f32> [@position] {
  $B2: {
    %tint_vertex_buffer_0_base:u32 = mul %tint_vertex_index, 64u
    %5:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %tint_vertex_buffer_0_base
    %6:u32 = load %5
    %7:vec4<f32> = unpack4x8unorm %6
    %8:f32 = access %7, 0u
    %9:u32 = add %tint_vertex_buffer_0_base, 1u
    %10:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %9
    %11:u32 = load %10
    %12:vec4<f32> = unpack4x8unorm %11
    %13:vec2<f32> = swizzle %12, xy
    %14:u32 = add %tint_vertex_buffer_0_base, 2u
    %15:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %14
    %16:u32 = load %15
    %17:vec4<f32> = unpack4x8unorm %16
    %18:u32 = add %tint_vertex_buffer_0_base, 3u
    %19:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %18
    %20:u32 = load %19
    %21:vec4<f32> = unpack4x8snorm %20
    %22:f32 = access %21, 0u
    %23:u32 = add %tint_vertex_buffer_0_base, 4u
    %24:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %23
    %25:u32 = load %24
    %26:vec4<f32> = unpack4x8snorm %25
    %27:vec2<f32> = swizzle %26, xy
    %28:u32 = add %tint_vertex_buffer_0_base, 5u
    %29:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %28
    %30:u32 = load %29
    %31:vec4<f32> = unpack4x8snorm %30
    %32:u32 = add %tint_vertex_buffer_0_base, 6u
    %33:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %32
    %34:u32 = load %33
    %35:vec2<f32> = unpack2x16unorm %34
    %36:f32 = access %35, 0u
    %37:u32 = add %tint_vertex_buffer_0_base, 7u
    %38:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %37
    %39:u32 = load %38
    %40:vec2<f32> = unpack2x16unorm %39
    %41:u32 = add %tint_vertex_buffer_0_base, 8u
    %42:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %41
    %43:u32 = load %42
    %44:u32 = add %tint_vertex_buffer_0_base, 9u
    %45:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %44
    %46:u32 = load %45
    %47:vec2<f32> = unpack2x16unorm %43
    %48:vec2<f32> = unpack2x16unorm %46
    %49:vec4<f32> = construct %47, %48
    %50:u32 = add %tint_vertex_buffer_0_base, 10u
    %51:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %50
    %52:u32 = load %51
    %53:vec2<f32> = unpack2x16snorm %52
    %54:f32 = access %53, 0u
    %55:u32 = add %tint_vertex_buffer_0_base, 11u
    %56:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %55
    %57:u32 = load %56
    %58:vec2<f32> = unpack2x16snorm %57
    %59:u32 = add %tint_vertex_buffer_0_base, 12u
    %60:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %59
    %61:u32 = load %60
    %62:u32 = add %tint_vertex_buffer_0_base, 13u
    %63:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %62
    %64:u32 = load %63
    %65:vec2<f32> = unpack2x16snorm %61
    %66:vec2<f32> = unpack2x16snorm %64
    %67:vec4<f32> = construct %65, %66
    %68:u32 = add %tint_vertex_buffer_0_base, 14u
    %69:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %68
    %70:u32 = load %69
    %71:vec2<f32> = unpack2x16float %70
    %72:f32 = access %71, 0u
    %73:u32 = add %tint_vertex_buffer_0_base, 15u
    %74:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %73
    %75:u32 = load %74
    %76:vec2<f32> = unpack2x16float %75
    %77:u32 = add %tint_vertex_buffer_0_base, 16u
    %78:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %77
    %79:u32 = load %78
    %80:u32 = add %tint_vertex_buffer_0_base, 17u
    %81:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %80
    %82:u32 = load %81
    %83:vec2<f32> = unpack2x16float %79
    %84:vec2<f32> = unpack2x16float %82
    %85:vec4<f32> = construct %83, %84
    %86:u32 = add %tint_vertex_buffer_0_base, 18u
    %87:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %86
    %88:u32 = load %87
    %89:f32 = bitcast %88
    %90:u32 = add %tint_vertex_buffer_0_base, 20u
    %91:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %90
    %92:u32 = load %91
    %93:f32 = bitcast %92
    %94:u32 = add %tint_vertex_buffer_0_base, 21u
    %95:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %94
    %96:u32 = load %95
    %97:f32 = bitcast %96
    %98:vec2<f32> = construct %93, %97
    %99:u32 = add %tint_vertex_buffer_0_base, 24u
    %100:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %99
    %101:u32 = load %100
    %102:f32 = bitcast %101
    %103:u32 = add %tint_vertex_buffer_0_base, 25u
    %104:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %103
    %105:u32 = load %104
    %106:f32 = bitcast %105
    %107:u32 = add %tint_vertex_buffer_0_base, 26u
    %108:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %107
    %109:u32 = load %108
    %110:f32 = bitcast %109
    %111:vec3<f32> = construct %102, %106, %110
    %112:u32 = add %tint_vertex_buffer_0_base, 28u
    %113:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %112
    %114:u32 = load %113
    %115:f32 = bitcast %114
    %116:u32 = add %tint_vertex_buffer_0_base, 29u
    %117:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %116
    %118:u32 = load %117
    %119:f32 = bitcast %118
    %120:u32 = add %tint_vertex_buffer_0_base, 30u
    %121:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %120
    %122:u32 = load %121
    %123:f32 = bitcast %122
    %124:u32 = add %tint_vertex_buffer_0_base, 31u
    %125:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %124
    %126:u32 = load %125
    %127:f32 = bitcast %126
    %128:vec4<f32> = construct %115, %119, %123, %127
    %129:u32 = add %tint_vertex_buffer_0_base, 32u
    %130:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %129
    %131:u32 = load %130
    %132:vec4<u32> = construct %131
    %133:vec4<u32> = shr %132, vec4<u32>(0u, 10u, 20u, 30u)
    %134:vec4<u32> = and %133, vec4<u32>(1023u, 1023u, 1023u, 3u)
    %135:vec4<f32> = convert %134
    %136:vec4<f32> = div %135, vec4<f32>(1023.0f, 1023.0f, 1023.0f, 3.0f)
    %137:u32 = add %tint_vertex_buffer_0_base, 33u
    %138:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %137
    %139:u32 = load %138
    %140:vec4<f32> = unpack4x8unorm %139
    %141:vec4<f32> = swizzle %140, zyxw
    %142:Inputs = construct %8, %13, %17, %22, %27, %31, %36, %40, %49, %54, %58, %67, %72, %76, %85, %89, %98, %111, %128, %136, %141
    %unorm8:f32 = access %142, 0u
    %unorm8x2:vec2<f32> = access %142, 1u
    %unorm8x4:vec4<f32> = access %142, 2u
    %snorm8:f32 = access %142, 3u
    %snorm8x2:vec2<f32> = access %142, 4u
    %snorm8x4:vec4<f32> = access %142, 5u
    %unorm16:f32 = access %142, 6u
    %unorm16x2:vec2<f32> = access %142, 7u
    %unorm16x4:vec4<f32> = access %142, 8u
    %snorm16:f32 = access %142, 9u
    %snorm16x2:vec2<f32> = access %142, 10u
    %snorm16x4:vec4<f32> = access %142, 11u
    %float16:f32 = access %142, 12u
    %float16x2:vec2<f32> = access %142, 13u
    %float16x4:vec4<f32> = access %142, 14u
    %float32:f32 = access %142, 15u
    %float32x2:vec2<f32> = access %142, 16u
    %float32x3:vec3<f32> = access %142, 17u
    %float32x4:vec4<f32> = access %142, 18u
    %unorm10_10_10_2:vec4<f32> = access %142, 19u
    %unorm8x4_bgra:vec4<f32> = access %142, 20u
    ret vec4<f32>(0.0f)
  }
}
)";

    VertexPullingConfig cfg;
    cfg.pulling_group = 4u;
    cfg.vertex_state = {{{256,
                          VertexStepMode::kVertex,
                          {
                              {VertexFormat::kUnorm8, 0, 0},              //
                              {VertexFormat::kUnorm8x2, 4, 1},            //
                              {VertexFormat::kUnorm8x4, 8, 2},            //
                              {VertexFormat::kSnorm8, 12, 3},             //
                              {VertexFormat::kSnorm8x2, 16, 4},           //
                              {VertexFormat::kSnorm8x4, 20, 5},           //
                              {VertexFormat::kUnorm16, 24, 6},            //
                              {VertexFormat::kUnorm16x2, 28, 7},          //
                              {VertexFormat::kUnorm16x4, 32, 8},          //
                              {VertexFormat::kSnorm16, 40, 9},            //
                              {VertexFormat::kSnorm16x2, 44, 10},         //
                              {VertexFormat::kSnorm16x4, 48, 11},         //
                              {VertexFormat::kFloat16, 56, 12},           //
                              {VertexFormat::kFloat16x2, 60, 13},         //
                              {VertexFormat::kFloat16x4, 64, 14},         //
                              {VertexFormat::kFloat32, 72, 15},           //
                              {VertexFormat::kFloat32x2, 80, 16},         //
                              {VertexFormat::kFloat32x3, 96, 17},         //
                              {VertexFormat::kFloat32x4, 112, 18},        //
                              {VertexFormat::kUnorm10_10_10_2, 128, 19},  //
                              {VertexFormat::kUnorm8x4BGRA, 132, 20},     //
                          }}}};
    Run(VertexPulling, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_VertexPullingTest, Formats_F16) {
    auto* inputs =
        ty.Struct(mod.symbols.New("Inputs"),
                  {
                      {mod.symbols.New("unorm8"), ty.f16(), Location(0)},                  //
                      {mod.symbols.New("unorm8x2"), ty.vec2<f16>(), Location(1)},          //
                      {mod.symbols.New("unorm8x4"), ty.vec4<f16>(), Location(2)},          //
                      {mod.symbols.New("snorm8"), ty.f16(), Location(3)},                  //
                      {mod.symbols.New("snorm8x2"), ty.vec2<f16>(), Location(4)},          //
                      {mod.symbols.New("snorm8x4"), ty.vec4<f16>(), Location(5)},          //
                      {mod.symbols.New("unorm16"), ty.f16(), Location(6)},                 //
                      {mod.symbols.New("unorm16x2"), ty.vec2<f16>(), Location(7)},         //
                      {mod.symbols.New("unorm16x4"), ty.vec4<f16>(), Location(8)},         //
                      {mod.symbols.New("snorm16"), ty.f16(), Location(9)},                 //
                      {mod.symbols.New("snorm16x2"), ty.vec2<f16>(), Location(10)},        //
                      {mod.symbols.New("snorm16x4"), ty.vec4<f16>(), Location(11)},        //
                      {mod.symbols.New("float16"), ty.f16(), Location(12)},                //
                      {mod.symbols.New("float16x2"), ty.vec2<f16>(), Location(13)},        //
                      {mod.symbols.New("float16x4"), ty.vec4<f16>(), Location(14)},        //
                      {mod.symbols.New("float32"), ty.f16(), Location(15)},                //
                      {mod.symbols.New("float32x2"), ty.vec2<f16>(), Location(16)},        //
                      {mod.symbols.New("float32x3"), ty.vec3<f16>(), Location(17)},        //
                      {mod.symbols.New("float32x4"), ty.vec4<f16>(), Location(18)},        //
                      {mod.symbols.New("unorm10_10_10_2"), ty.vec4<f16>(), Location(19)},  //
                      {mod.symbols.New("unorm8x4_bgra"), ty.vec4<f16>(), Location(20)},    //
                  });

    auto* ep = b.Function("foo", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    auto* param = b.FunctionParam("input", inputs);
    ep->AppendParam(param);
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    b.Append(ep->Block(), [&] {  //
        mod.SetName(b.Access(ty.f16(), param, 0_u), "unorm8");
        mod.SetName(b.Access(ty.vec2<f16>(), param, 1_u), "unorm8x2");
        mod.SetName(b.Access(ty.vec4<f16>(), param, 2_u), "unorm8x4");
        mod.SetName(b.Access(ty.f16(), param, 3_u), "snorm8");
        mod.SetName(b.Access(ty.vec2<f16>(), param, 4_u), "snorm8x2");
        mod.SetName(b.Access(ty.vec4<f16>(), param, 5_u), "snorm8x4");
        mod.SetName(b.Access(ty.f16(), param, 6_u), "unorm16");
        mod.SetName(b.Access(ty.vec2<f16>(), param, 7_u), "unorm16x2");
        mod.SetName(b.Access(ty.vec4<f16>(), param, 8_u), "unorm16x4");
        mod.SetName(b.Access(ty.f16(), param, 9_u), "snorm16");
        mod.SetName(b.Access(ty.vec2<f16>(), param, 10_u), "snorm16x2");
        mod.SetName(b.Access(ty.vec4<f16>(), param, 11_u), "snorm16x4");
        mod.SetName(b.Access(ty.f16(), param, 12_u), "float16");
        mod.SetName(b.Access(ty.vec2<f16>(), param, 13_u), "float16x2");
        mod.SetName(b.Access(ty.vec4<f16>(), param, 14_u), "float16x4");
        mod.SetName(b.Access(ty.f16(), param, 15_u), "float32");
        mod.SetName(b.Access(ty.vec2<f16>(), param, 16_u), "float32x2");
        mod.SetName(b.Access(ty.vec3<f16>(), param, 17_u), "float32x3");
        mod.SetName(b.Access(ty.vec4<f16>(), param, 18_u), "float32x4");
        mod.SetName(b.Access(ty.vec4<f16>(), param, 19_u), "unorm10_10_10_2");
        mod.SetName(b.Access(ty.vec4<f16>(), param, 20_u), "unorm8x4_bgra");
        b.Return(ep, b.Zero<vec4<f32>>());
    });

    auto* src = R"(
Inputs = struct @align(8) {
  unorm8:f16 @offset(0), @location(0)
  unorm8x2:vec2<f16> @offset(4), @location(1)
  unorm8x4:vec4<f16> @offset(8), @location(2)
  snorm8:f16 @offset(16), @location(3)
  snorm8x2:vec2<f16> @offset(20), @location(4)
  snorm8x4:vec4<f16> @offset(24), @location(5)
  unorm16:f16 @offset(32), @location(6)
  unorm16x2:vec2<f16> @offset(36), @location(7)
  unorm16x4:vec4<f16> @offset(40), @location(8)
  snorm16:f16 @offset(48), @location(9)
  snorm16x2:vec2<f16> @offset(52), @location(10)
  snorm16x4:vec4<f16> @offset(56), @location(11)
  float16:f16 @offset(64), @location(12)
  float16x2:vec2<f16> @offset(68), @location(13)
  float16x4:vec4<f16> @offset(72), @location(14)
  float32:f16 @offset(80), @location(15)
  float32x2:vec2<f16> @offset(84), @location(16)
  float32x3:vec3<f16> @offset(88), @location(17)
  float32x4:vec4<f16> @offset(96), @location(18)
  unorm10_10_10_2:vec4<f16> @offset(104), @location(19)
  unorm8x4_bgra:vec4<f16> @offset(112), @location(20)
}

%foo = @vertex func(%input:Inputs):vec4<f32> [@position] {
  $B1: {
    %unorm8:f16 = access %input, 0u
    %unorm8x2:vec2<f16> = access %input, 1u
    %unorm8x4:vec4<f16> = access %input, 2u
    %snorm8:f16 = access %input, 3u
    %snorm8x2:vec2<f16> = access %input, 4u
    %snorm8x4:vec4<f16> = access %input, 5u
    %unorm16:f16 = access %input, 6u
    %unorm16x2:vec2<f16> = access %input, 7u
    %unorm16x4:vec4<f16> = access %input, 8u
    %snorm16:f16 = access %input, 9u
    %snorm16x2:vec2<f16> = access %input, 10u
    %snorm16x4:vec4<f16> = access %input, 11u
    %float16:f16 = access %input, 12u
    %float16x2:vec2<f16> = access %input, 13u
    %float16x4:vec4<f16> = access %input, 14u
    %float32:f16 = access %input, 15u
    %float32x2:vec2<f16> = access %input, 16u
    %float32x3:vec3<f16> = access %input, 17u
    %float32x4:vec4<f16> = access %input, 18u
    %unorm10_10_10_2:vec4<f16> = access %input, 19u
    %unorm8x4_bgra:vec4<f16> = access %input, 20u
    ret vec4<f32>(0.0f)
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inputs = struct @align(8) {
  unorm8:f16 @offset(0), @location(0)
  unorm8x2:vec2<f16> @offset(4), @location(1)
  unorm8x4:vec4<f16> @offset(8), @location(2)
  snorm8:f16 @offset(16), @location(3)
  snorm8x2:vec2<f16> @offset(20), @location(4)
  snorm8x4:vec4<f16> @offset(24), @location(5)
  unorm16:f16 @offset(32), @location(6)
  unorm16x2:vec2<f16> @offset(36), @location(7)
  unorm16x4:vec4<f16> @offset(40), @location(8)
  snorm16:f16 @offset(48), @location(9)
  snorm16x2:vec2<f16> @offset(52), @location(10)
  snorm16x4:vec4<f16> @offset(56), @location(11)
  float16:f16 @offset(64), @location(12)
  float16x2:vec2<f16> @offset(68), @location(13)
  float16x4:vec4<f16> @offset(72), @location(14)
  float32:f16 @offset(80), @location(15)
  float32x2:vec2<f16> @offset(84), @location(16)
  float32x3:vec3<f16> @offset(88), @location(17)
  float32x4:vec4<f16> @offset(96), @location(18)
  unorm10_10_10_2:vec4<f16> @offset(104), @location(19)
  unorm8x4_bgra:vec4<f16> @offset(112), @location(20)
}

$B1: {  # root
  %tint_vertex_buffer_0:ptr<storage, array<u32>, read> = var @binding_point(4, 0)
}

%foo = @vertex func(%tint_vertex_index:u32 [@vertex_index]):vec4<f32> [@position] {
  $B2: {
    %tint_vertex_buffer_0_base:u32 = mul %tint_vertex_index, 64u
    %5:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %tint_vertex_buffer_0_base
    %6:u32 = load %5
    %7:vec4<f32> = unpack4x8unorm %6
    %8:f32 = access %7, 0u
    %9:f16 = convert %8
    %10:u32 = add %tint_vertex_buffer_0_base, 1u
    %11:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %10
    %12:u32 = load %11
    %13:vec4<f32> = unpack4x8unorm %12
    %14:vec2<f32> = swizzle %13, xy
    %15:vec2<f16> = convert %14
    %16:u32 = add %tint_vertex_buffer_0_base, 2u
    %17:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %16
    %18:u32 = load %17
    %19:vec4<f32> = unpack4x8unorm %18
    %20:vec4<f16> = convert %19
    %21:u32 = add %tint_vertex_buffer_0_base, 3u
    %22:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %21
    %23:u32 = load %22
    %24:vec4<f32> = unpack4x8snorm %23
    %25:f32 = access %24, 0u
    %26:f16 = convert %25
    %27:u32 = add %tint_vertex_buffer_0_base, 4u
    %28:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %27
    %29:u32 = load %28
    %30:vec4<f32> = unpack4x8snorm %29
    %31:vec2<f32> = swizzle %30, xy
    %32:vec2<f16> = convert %31
    %33:u32 = add %tint_vertex_buffer_0_base, 5u
    %34:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %33
    %35:u32 = load %34
    %36:vec4<f32> = unpack4x8snorm %35
    %37:vec4<f16> = convert %36
    %38:u32 = add %tint_vertex_buffer_0_base, 6u
    %39:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %38
    %40:u32 = load %39
    %41:vec2<f32> = unpack2x16unorm %40
    %42:f32 = access %41, 0u
    %43:f16 = convert %42
    %44:u32 = add %tint_vertex_buffer_0_base, 7u
    %45:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %44
    %46:u32 = load %45
    %47:vec2<f32> = unpack2x16unorm %46
    %48:vec2<f16> = convert %47
    %49:u32 = add %tint_vertex_buffer_0_base, 8u
    %50:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %49
    %51:u32 = load %50
    %52:u32 = add %tint_vertex_buffer_0_base, 9u
    %53:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %52
    %54:u32 = load %53
    %55:vec2<f32> = unpack2x16unorm %51
    %56:vec2<f32> = unpack2x16unorm %54
    %57:vec4<f32> = construct %55, %56
    %58:vec4<f16> = convert %57
    %59:u32 = add %tint_vertex_buffer_0_base, 10u
    %60:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %59
    %61:u32 = load %60
    %62:vec2<f32> = unpack2x16snorm %61
    %63:f32 = access %62, 0u
    %64:f16 = convert %63
    %65:u32 = add %tint_vertex_buffer_0_base, 11u
    %66:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %65
    %67:u32 = load %66
    %68:vec2<f32> = unpack2x16snorm %67
    %69:vec2<f16> = convert %68
    %70:u32 = add %tint_vertex_buffer_0_base, 12u
    %71:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %70
    %72:u32 = load %71
    %73:u32 = add %tint_vertex_buffer_0_base, 13u
    %74:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %73
    %75:u32 = load %74
    %76:vec2<f32> = unpack2x16snorm %72
    %77:vec2<f32> = unpack2x16snorm %75
    %78:vec4<f32> = construct %76, %77
    %79:vec4<f16> = convert %78
    %80:u32 = add %tint_vertex_buffer_0_base, 14u
    %81:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %80
    %82:u32 = load %81
    %83:vec2<f16> = bitcast %82
    %84:f16 = access %83, 0u
    %85:u32 = add %tint_vertex_buffer_0_base, 15u
    %86:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %85
    %87:u32 = load %86
    %88:vec2<f16> = bitcast %87
    %89:u32 = add %tint_vertex_buffer_0_base, 16u
    %90:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %89
    %91:u32 = load %90
    %92:u32 = add %tint_vertex_buffer_0_base, 17u
    %93:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %92
    %94:u32 = load %93
    %95:vec2<f16> = bitcast %91
    %96:vec2<f16> = bitcast %94
    %97:vec4<f16> = construct %95, %96
    %98:u32 = add %tint_vertex_buffer_0_base, 18u
    %99:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %98
    %100:u32 = load %99
    %101:f32 = bitcast %100
    %102:f16 = convert %101
    %103:u32 = add %tint_vertex_buffer_0_base, 20u
    %104:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %103
    %105:u32 = load %104
    %106:f32 = bitcast %105
    %107:u32 = add %tint_vertex_buffer_0_base, 21u
    %108:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %107
    %109:u32 = load %108
    %110:f32 = bitcast %109
    %111:vec2<f32> = construct %106, %110
    %112:vec2<f16> = convert %111
    %113:u32 = add %tint_vertex_buffer_0_base, 24u
    %114:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %113
    %115:u32 = load %114
    %116:f32 = bitcast %115
    %117:u32 = add %tint_vertex_buffer_0_base, 25u
    %118:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %117
    %119:u32 = load %118
    %120:f32 = bitcast %119
    %121:u32 = add %tint_vertex_buffer_0_base, 26u
    %122:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %121
    %123:u32 = load %122
    %124:f32 = bitcast %123
    %125:vec3<f32> = construct %116, %120, %124
    %126:vec3<f16> = convert %125
    %127:u32 = add %tint_vertex_buffer_0_base, 28u
    %128:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %127
    %129:u32 = load %128
    %130:f32 = bitcast %129
    %131:u32 = add %tint_vertex_buffer_0_base, 29u
    %132:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %131
    %133:u32 = load %132
    %134:f32 = bitcast %133
    %135:u32 = add %tint_vertex_buffer_0_base, 30u
    %136:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %135
    %137:u32 = load %136
    %138:f32 = bitcast %137
    %139:u32 = add %tint_vertex_buffer_0_base, 31u
    %140:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %139
    %141:u32 = load %140
    %142:f32 = bitcast %141
    %143:vec4<f32> = construct %130, %134, %138, %142
    %144:vec4<f16> = convert %143
    %145:u32 = add %tint_vertex_buffer_0_base, 32u
    %146:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %145
    %147:u32 = load %146
    %148:vec4<u32> = construct %147
    %149:vec4<u32> = shr %148, vec4<u32>(0u, 10u, 20u, 30u)
    %150:vec4<u32> = and %149, vec4<u32>(1023u, 1023u, 1023u, 3u)
    %151:vec4<f32> = convert %150
    %152:vec4<f32> = div %151, vec4<f32>(1023.0f, 1023.0f, 1023.0f, 3.0f)
    %153:vec4<f16> = convert %152
    %154:u32 = add %tint_vertex_buffer_0_base, 33u
    %155:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %154
    %156:u32 = load %155
    %157:vec4<f32> = unpack4x8unorm %156
    %158:vec4<f32> = swizzle %157, zyxw
    %159:vec4<f16> = convert %158
    %160:Inputs = construct %9, %15, %20, %26, %32, %37, %43, %48, %58, %64, %69, %79, %84, %88, %97, %102, %112, %126, %144, %153, %159
    %unorm8:f16 = access %160, 0u
    %unorm8x2:vec2<f16> = access %160, 1u
    %unorm8x4:vec4<f16> = access %160, 2u
    %snorm8:f16 = access %160, 3u
    %snorm8x2:vec2<f16> = access %160, 4u
    %snorm8x4:vec4<f16> = access %160, 5u
    %unorm16:f16 = access %160, 6u
    %unorm16x2:vec2<f16> = access %160, 7u
    %unorm16x4:vec4<f16> = access %160, 8u
    %snorm16:f16 = access %160, 9u
    %snorm16x2:vec2<f16> = access %160, 10u
    %snorm16x4:vec4<f16> = access %160, 11u
    %float16:f16 = access %160, 12u
    %float16x2:vec2<f16> = access %160, 13u
    %float16x4:vec4<f16> = access %160, 14u
    %float32:f16 = access %160, 15u
    %float32x2:vec2<f16> = access %160, 16u
    %float32x3:vec3<f16> = access %160, 17u
    %float32x4:vec4<f16> = access %160, 18u
    %unorm10_10_10_2:vec4<f16> = access %160, 19u
    %unorm8x4_bgra:vec4<f16> = access %160, 20u
    ret vec4<f32>(0.0f)
  }
}
)";

    VertexPullingConfig cfg;
    cfg.pulling_group = 4u;
    cfg.vertex_state = {{{256,
                          VertexStepMode::kVertex,
                          {
                              {VertexFormat::kUnorm8, 0, 0},              //
                              {VertexFormat::kUnorm8x2, 4, 1},            //
                              {VertexFormat::kUnorm8x4, 8, 2},            //
                              {VertexFormat::kSnorm8, 12, 3},             //
                              {VertexFormat::kSnorm8x2, 16, 4},           //
                              {VertexFormat::kSnorm8x4, 20, 5},           //
                              {VertexFormat::kUnorm16, 24, 6},            //
                              {VertexFormat::kUnorm16x2, 28, 7},          //
                              {VertexFormat::kUnorm16x4, 32, 8},          //
                              {VertexFormat::kSnorm16, 40, 9},            //
                              {VertexFormat::kSnorm16x2, 44, 10},         //
                              {VertexFormat::kSnorm16x4, 48, 11},         //
                              {VertexFormat::kFloat16, 56, 12},           //
                              {VertexFormat::kFloat16x2, 60, 13},         //
                              {VertexFormat::kFloat16x4, 64, 14},         //
                              {VertexFormat::kFloat32, 72, 15},           //
                              {VertexFormat::kFloat32x2, 80, 16},         //
                              {VertexFormat::kFloat32x3, 96, 17},         //
                              {VertexFormat::kFloat32x4, 112, 18},        //
                              {VertexFormat::kUnorm10_10_10_2, 128, 19},  //
                              {VertexFormat::kUnorm8x4BGRA, 132, 20},     //
                          }}}};
    Run(VertexPulling, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_VertexPullingTest, Formats_SInt) {
    auto* inputs = ty.Struct(mod.symbols.New("Inputs"),
                             {
                                 {mod.symbols.New("sint8"), ty.i32(), Location(0)},           //
                                 {mod.symbols.New("sint8x2"), ty.vec2<i32>(), Location(1)},   //
                                 {mod.symbols.New("sint8x4"), ty.vec4<i32>(), Location(2)},   //
                                 {mod.symbols.New("sint16"), ty.i32(), Location(3)},          //
                                 {mod.symbols.New("sint16x2"), ty.vec2<i32>(), Location(4)},  //
                                 {mod.symbols.New("sint16x4"), ty.vec4<i32>(), Location(5)},  //
                                 {mod.symbols.New("sint32"), ty.i32(), Location(6)},          //
                                 {mod.symbols.New("sint32x2"), ty.vec2<i32>(), Location(7)},  //
                                 {mod.symbols.New("sint32x3"), ty.vec3<i32>(), Location(8)},  //
                                 {mod.symbols.New("sint32x4"), ty.vec4<i32>(), Location(9)},  //
                             });

    auto* ep = b.Function("foo", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    auto* param = b.FunctionParam("input", inputs);
    ep->AppendParam(param);
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    b.Append(ep->Block(), [&] {  //
        mod.SetName(b.Access(ty.i32(), param, 0_u), "sint8");
        mod.SetName(b.Access(ty.vec2<i32>(), param, 1_u), "sint8x2");
        mod.SetName(b.Access(ty.vec4<i32>(), param, 2_u), "sint8x4");
        mod.SetName(b.Access(ty.i32(), param, 3_u), "sint16");
        mod.SetName(b.Access(ty.vec2<i32>(), param, 4_u), "sint16x2");
        mod.SetName(b.Access(ty.vec4<i32>(), param, 5_u), "sint16x4");
        mod.SetName(b.Access(ty.i32(), param, 6_u), "sint32");
        mod.SetName(b.Access(ty.vec2<i32>(), param, 7_u), "sint32x2");
        mod.SetName(b.Access(ty.vec3<i32>(), param, 8_u), "sint32x3");
        mod.SetName(b.Access(ty.vec4<i32>(), param, 9_u), "sint32x4");
        b.Return(ep, b.Zero<vec4<f32>>());
    });

    auto* src = R"(
Inputs = struct @align(16) {
  sint8:i32 @offset(0), @location(0)
  sint8x2:vec2<i32> @offset(8), @location(1)
  sint8x4:vec4<i32> @offset(16), @location(2)
  sint16:i32 @offset(32), @location(3)
  sint16x2:vec2<i32> @offset(40), @location(4)
  sint16x4:vec4<i32> @offset(48), @location(5)
  sint32:i32 @offset(64), @location(6)
  sint32x2:vec2<i32> @offset(72), @location(7)
  sint32x3:vec3<i32> @offset(80), @location(8)
  sint32x4:vec4<i32> @offset(96), @location(9)
}

%foo = @vertex func(%input:Inputs):vec4<f32> [@position] {
  $B1: {
    %sint8:i32 = access %input, 0u
    %sint8x2:vec2<i32> = access %input, 1u
    %sint8x4:vec4<i32> = access %input, 2u
    %sint16:i32 = access %input, 3u
    %sint16x2:vec2<i32> = access %input, 4u
    %sint16x4:vec4<i32> = access %input, 5u
    %sint32:i32 = access %input, 6u
    %sint32x2:vec2<i32> = access %input, 7u
    %sint32x3:vec3<i32> = access %input, 8u
    %sint32x4:vec4<i32> = access %input, 9u
    ret vec4<f32>(0.0f)
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inputs = struct @align(16) {
  sint8:i32 @offset(0), @location(0)
  sint8x2:vec2<i32> @offset(8), @location(1)
  sint8x4:vec4<i32> @offset(16), @location(2)
  sint16:i32 @offset(32), @location(3)
  sint16x2:vec2<i32> @offset(40), @location(4)
  sint16x4:vec4<i32> @offset(48), @location(5)
  sint32:i32 @offset(64), @location(6)
  sint32x2:vec2<i32> @offset(72), @location(7)
  sint32x3:vec3<i32> @offset(80), @location(8)
  sint32x4:vec4<i32> @offset(96), @location(9)
}

$B1: {  # root
  %tint_vertex_buffer_0:ptr<storage, array<u32>, read> = var @binding_point(4, 0)
}

%foo = @vertex func(%tint_vertex_index:u32 [@vertex_index]):vec4<f32> [@position] {
  $B2: {
    %tint_vertex_buffer_0_base:u32 = mul %tint_vertex_index, 64u
    %5:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %tint_vertex_buffer_0_base
    %6:u32 = load %5
    %7:i32 = bitcast %6
    %8:i32 = shl %7, 24u
    %9:i32 = shr %8, 24u
    %10:u32 = add %tint_vertex_buffer_0_base, 1u
    %11:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %10
    %12:u32 = load %11
    %13:i32 = bitcast %12
    %14:vec2<i32> = construct %13
    %15:vec2<i32> = shl %14, vec2<u32>(24u, 16u)
    %16:vec2<i32> = shr %15, vec2<u32>(24u)
    %17:u32 = add %tint_vertex_buffer_0_base, 2u
    %18:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %17
    %19:u32 = load %18
    %20:i32 = bitcast %19
    %21:vec4<i32> = construct %20
    %22:vec4<i32> = shl %21, vec4<u32>(24u, 16u, 8u, 0u)
    %23:vec4<i32> = shr %22, vec4<u32>(24u)
    %24:u32 = add %tint_vertex_buffer_0_base, 3u
    %25:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %24
    %26:u32 = load %25
    %27:i32 = bitcast %26
    %28:i32 = shl %27, 16u
    %29:i32 = shr %28, 16u
    %30:u32 = add %tint_vertex_buffer_0_base, 4u
    %31:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %30
    %32:u32 = load %31
    %33:i32 = bitcast %32
    %34:vec2<i32> = construct %33
    %35:vec2<i32> = shl %34, vec2<u32>(16u, 0u)
    %36:vec2<i32> = shr %35, vec2<u32>(16u)
    %37:u32 = add %tint_vertex_buffer_0_base, 6u
    %38:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %37
    %39:u32 = load %38
    %40:i32 = bitcast %39
    %41:vec2<i32> = construct %40
    %42:vec2<i32> = shl %41, vec2<u32>(16u, 0u)
    %43:vec2<i32> = shr %42, vec2<u32>(16u)
    %44:u32 = add %tint_vertex_buffer_0_base, 7u
    %45:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %44
    %46:u32 = load %45
    %47:i32 = bitcast %46
    %48:vec2<i32> = construct %47
    %49:vec2<i32> = shl %48, vec2<u32>(16u, 0u)
    %50:vec2<i32> = shr %49, vec2<u32>(16u)
    %51:vec4<i32> = construct %43, %50
    %52:u32 = add %tint_vertex_buffer_0_base, 7u
    %53:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %52
    %54:u32 = load %53
    %55:i32 = bitcast %54
    %56:u32 = add %tint_vertex_buffer_0_base, 8u
    %57:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %56
    %58:u32 = load %57
    %59:i32 = bitcast %58
    %60:u32 = add %tint_vertex_buffer_0_base, 9u
    %61:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %60
    %62:u32 = load %61
    %63:i32 = bitcast %62
    %64:vec2<i32> = construct %59, %63
    %65:u32 = add %tint_vertex_buffer_0_base, 12u
    %66:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %65
    %67:u32 = load %66
    %68:i32 = bitcast %67
    %69:u32 = add %tint_vertex_buffer_0_base, 13u
    %70:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %69
    %71:u32 = load %70
    %72:i32 = bitcast %71
    %73:u32 = add %tint_vertex_buffer_0_base, 14u
    %74:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %73
    %75:u32 = load %74
    %76:i32 = bitcast %75
    %77:vec3<i32> = construct %68, %72, %76
    %78:u32 = add %tint_vertex_buffer_0_base, 16u
    %79:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %78
    %80:u32 = load %79
    %81:i32 = bitcast %80
    %82:u32 = add %tint_vertex_buffer_0_base, 17u
    %83:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %82
    %84:u32 = load %83
    %85:i32 = bitcast %84
    %86:u32 = add %tint_vertex_buffer_0_base, 18u
    %87:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %86
    %88:u32 = load %87
    %89:i32 = bitcast %88
    %90:u32 = add %tint_vertex_buffer_0_base, 19u
    %91:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %90
    %92:u32 = load %91
    %93:i32 = bitcast %92
    %94:vec4<i32> = construct %81, %85, %89, %93
    %95:Inputs = construct %9, %16, %23, %29, %36, %51, %55, %64, %77, %94
    %sint8:i32 = access %95, 0u
    %sint8x2:vec2<i32> = access %95, 1u
    %sint8x4:vec4<i32> = access %95, 2u
    %sint16:i32 = access %95, 3u
    %sint16x2:vec2<i32> = access %95, 4u
    %sint16x4:vec4<i32> = access %95, 5u
    %sint32:i32 = access %95, 6u
    %sint32x2:vec2<i32> = access %95, 7u
    %sint32x3:vec3<i32> = access %95, 8u
    %sint32x4:vec4<i32> = access %95, 9u
    ret vec4<f32>(0.0f)
  }
}
)";

    VertexPullingConfig cfg;
    cfg.pulling_group = 4u;
    cfg.vertex_state = {{{256,
                          VertexStepMode::kVertex,
                          {
                              {VertexFormat::kSint8, 0, 0},      //
                              {VertexFormat::kSint8x2, 4, 1},    //
                              {VertexFormat::kSint8x4, 8, 2},    //
                              {VertexFormat::kSint16, 12, 3},    //
                              {VertexFormat::kSint16x2, 16, 4},  //
                              {VertexFormat::kSint16x4, 24, 5},  //
                              {VertexFormat::kSint32, 28, 6},    //
                              {VertexFormat::kSint32x2, 32, 7},  //
                              {VertexFormat::kSint32x3, 48, 8},  //
                              {VertexFormat::kSint32x4, 64, 9},  //
                          }}}};
    Run(VertexPulling, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_VertexPullingTest, Formats_UInt) {
    auto* inputs = ty.Struct(mod.symbols.New("Inputs"),
                             {
                                 {mod.symbols.New("uint8"), ty.u32(), Location(0)},           //
                                 {mod.symbols.New("uint8x2"), ty.vec2<u32>(), Location(1)},   //
                                 {mod.symbols.New("uint8x4"), ty.vec4<u32>(), Location(2)},   //
                                 {mod.symbols.New("uint16"), ty.u32(), Location(3)},          //
                                 {mod.symbols.New("uint16x2"), ty.vec2<u32>(), Location(4)},  //
                                 {mod.symbols.New("uint16x4"), ty.vec4<u32>(), Location(5)},  //
                                 {mod.symbols.New("uint32"), ty.u32(), Location(6)},          //
                                 {mod.symbols.New("uint32x2"), ty.vec2<u32>(), Location(7)},  //
                                 {mod.symbols.New("uint32x3"), ty.vec3<u32>(), Location(8)},  //
                                 {mod.symbols.New("uint32x4"), ty.vec4<u32>(), Location(9)},  //
                             });

    auto* ep = b.Function("foo", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    auto* param = b.FunctionParam("input", inputs);
    ep->AppendParam(param);
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    b.Append(ep->Block(), [&] {  //
        mod.SetName(b.Access(ty.u32(), param, 0_u), "uint8");
        mod.SetName(b.Access(ty.vec2<u32>(), param, 1_u), "uint8x2");
        mod.SetName(b.Access(ty.vec4<u32>(), param, 2_u), "uint8x4");
        mod.SetName(b.Access(ty.u32(), param, 3_u), "uint16");
        mod.SetName(b.Access(ty.vec2<u32>(), param, 4_u), "uint16x2");
        mod.SetName(b.Access(ty.vec4<u32>(), param, 5_u), "uint16x4");
        mod.SetName(b.Access(ty.u32(), param, 6_u), "uint32");
        mod.SetName(b.Access(ty.vec2<u32>(), param, 7_u), "uint32x2");
        mod.SetName(b.Access(ty.vec3<u32>(), param, 8_u), "uint32x3");
        mod.SetName(b.Access(ty.vec4<u32>(), param, 9_u), "uint32x4");
        b.Return(ep, b.Zero<vec4<f32>>());
    });

    auto* src = R"(
Inputs = struct @align(16) {
  uint8:u32 @offset(0), @location(0)
  uint8x2:vec2<u32> @offset(8), @location(1)
  uint8x4:vec4<u32> @offset(16), @location(2)
  uint16:u32 @offset(32), @location(3)
  uint16x2:vec2<u32> @offset(40), @location(4)
  uint16x4:vec4<u32> @offset(48), @location(5)
  uint32:u32 @offset(64), @location(6)
  uint32x2:vec2<u32> @offset(72), @location(7)
  uint32x3:vec3<u32> @offset(80), @location(8)
  uint32x4:vec4<u32> @offset(96), @location(9)
}

%foo = @vertex func(%input:Inputs):vec4<f32> [@position] {
  $B1: {
    %uint8:u32 = access %input, 0u
    %uint8x2:vec2<u32> = access %input, 1u
    %uint8x4:vec4<u32> = access %input, 2u
    %uint16:u32 = access %input, 3u
    %uint16x2:vec2<u32> = access %input, 4u
    %uint16x4:vec4<u32> = access %input, 5u
    %uint32:u32 = access %input, 6u
    %uint32x2:vec2<u32> = access %input, 7u
    %uint32x3:vec3<u32> = access %input, 8u
    %uint32x4:vec4<u32> = access %input, 9u
    ret vec4<f32>(0.0f)
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inputs = struct @align(16) {
  uint8:u32 @offset(0), @location(0)
  uint8x2:vec2<u32> @offset(8), @location(1)
  uint8x4:vec4<u32> @offset(16), @location(2)
  uint16:u32 @offset(32), @location(3)
  uint16x2:vec2<u32> @offset(40), @location(4)
  uint16x4:vec4<u32> @offset(48), @location(5)
  uint32:u32 @offset(64), @location(6)
  uint32x2:vec2<u32> @offset(72), @location(7)
  uint32x3:vec3<u32> @offset(80), @location(8)
  uint32x4:vec4<u32> @offset(96), @location(9)
}

$B1: {  # root
  %tint_vertex_buffer_0:ptr<storage, array<u32>, read> = var @binding_point(4, 0)
}

%foo = @vertex func(%tint_vertex_index:u32 [@vertex_index]):vec4<f32> [@position] {
  $B2: {
    %tint_vertex_buffer_0_base:u32 = mul %tint_vertex_index, 64u
    %5:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %tint_vertex_buffer_0_base
    %6:u32 = load %5
    %7:u32 = and %6, 255u
    %8:u32 = add %tint_vertex_buffer_0_base, 1u
    %9:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %8
    %10:u32 = load %9
    %11:vec2<u32> = construct %10
    %12:vec2<u32> = shl %11, vec2<u32>(24u, 16u)
    %13:vec2<u32> = shr %12, vec2<u32>(24u)
    %14:u32 = add %tint_vertex_buffer_0_base, 2u
    %15:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %14
    %16:u32 = load %15
    %17:vec4<u32> = construct %16
    %18:vec4<u32> = shl %17, vec4<u32>(24u, 16u, 8u, 0u)
    %19:vec4<u32> = shr %18, vec4<u32>(24u)
    %20:u32 = add %tint_vertex_buffer_0_base, 3u
    %21:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %20
    %22:u32 = load %21
    %23:u32 = and %22, 65535u
    %24:u32 = add %tint_vertex_buffer_0_base, 4u
    %25:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %24
    %26:u32 = load %25
    %27:vec2<u32> = construct %26
    %28:vec2<u32> = shl %27, vec2<u32>(16u, 0u)
    %29:vec2<u32> = shr %28, vec2<u32>(16u)
    %30:u32 = add %tint_vertex_buffer_0_base, 6u
    %31:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %30
    %32:u32 = load %31
    %33:vec2<u32> = construct %32
    %34:vec2<u32> = shl %33, vec2<u32>(16u, 0u)
    %35:vec2<u32> = shr %34, vec2<u32>(16u)
    %36:u32 = add %tint_vertex_buffer_0_base, 7u
    %37:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %36
    %38:u32 = load %37
    %39:vec2<u32> = construct %38
    %40:vec2<u32> = shl %39, vec2<u32>(16u, 0u)
    %41:vec2<u32> = shr %40, vec2<u32>(16u)
    %42:vec4<u32> = construct %35, %41
    %43:u32 = add %tint_vertex_buffer_0_base, 7u
    %44:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %43
    %45:u32 = load %44
    %46:u32 = add %tint_vertex_buffer_0_base, 8u
    %47:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %46
    %48:u32 = load %47
    %49:u32 = add %tint_vertex_buffer_0_base, 9u
    %50:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %49
    %51:u32 = load %50
    %52:vec2<u32> = construct %48, %51
    %53:u32 = add %tint_vertex_buffer_0_base, 12u
    %54:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %53
    %55:u32 = load %54
    %56:u32 = add %tint_vertex_buffer_0_base, 13u
    %57:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %56
    %58:u32 = load %57
    %59:u32 = add %tint_vertex_buffer_0_base, 14u
    %60:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %59
    %61:u32 = load %60
    %62:vec3<u32> = construct %55, %58, %61
    %63:u32 = add %tint_vertex_buffer_0_base, 16u
    %64:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %63
    %65:u32 = load %64
    %66:u32 = add %tint_vertex_buffer_0_base, 17u
    %67:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %66
    %68:u32 = load %67
    %69:u32 = add %tint_vertex_buffer_0_base, 18u
    %70:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %69
    %71:u32 = load %70
    %72:u32 = add %tint_vertex_buffer_0_base, 19u
    %73:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %72
    %74:u32 = load %73
    %75:vec4<u32> = construct %65, %68, %71, %74
    %76:Inputs = construct %7, %13, %19, %23, %29, %42, %45, %52, %62, %75
    %uint8:u32 = access %76, 0u
    %uint8x2:vec2<u32> = access %76, 1u
    %uint8x4:vec4<u32> = access %76, 2u
    %uint16:u32 = access %76, 3u
    %uint16x2:vec2<u32> = access %76, 4u
    %uint16x4:vec4<u32> = access %76, 5u
    %uint32:u32 = access %76, 6u
    %uint32x2:vec2<u32> = access %76, 7u
    %uint32x3:vec3<u32> = access %76, 8u
    %uint32x4:vec4<u32> = access %76, 9u
    ret vec4<f32>(0.0f)
  }
}
)";

    VertexPullingConfig cfg;
    cfg.pulling_group = 4u;
    cfg.vertex_state = {{{256,
                          VertexStepMode::kVertex,
                          {
                              {VertexFormat::kUint8, 0, 0},      //
                              {VertexFormat::kUint8x2, 4, 1},    //
                              {VertexFormat::kUint8x4, 8, 2},    //
                              {VertexFormat::kUint16, 12, 3},    //
                              {VertexFormat::kUint16x2, 16, 4},  //
                              {VertexFormat::kUint16x4, 24, 5},  //
                              {VertexFormat::kUint32, 28, 6},    //
                              {VertexFormat::kUint32x2, 32, 7},  //
                              {VertexFormat::kUint32x3, 48, 8},  //
                              {VertexFormat::kUint32x4, 64, 9},  //
                          }}}};
    Run(VertexPulling, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_VertexPullingTest, ShaderTypeTooFewComponents) {
    auto* inputs = ty.Struct(mod.symbols.New("Inputs"),
                             {
                                 {mod.symbols.New("uint32"), ty.u32(), Location(0)},          //
                                 {mod.symbols.New("uint32x2"), ty.vec2<u32>(), Location(1)},  //
                                 {mod.symbols.New("uint32x3"), ty.vec3<u32>(), Location(2)},  //
                             });

    auto* ep = b.Function("foo", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    auto* param = b.FunctionParam("input", inputs);
    ep->AppendParam(param);
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    b.Append(ep->Block(), [&] {  //
        mod.SetName(b.Access(ty.u32(), param, 0_u), "uint32x2");
        mod.SetName(b.Access(ty.vec2<u32>(), param, 1_u), "uint32x4");
        mod.SetName(b.Access(ty.vec3<u32>(), param, 2_u), "uint32x4");
        b.Return(ep, b.Zero<vec4<f32>>());
    });

    auto* src = R"(
Inputs = struct @align(16) {
  uint32:u32 @offset(0), @location(0)
  uint32x2:vec2<u32> @offset(8), @location(1)
  uint32x3:vec3<u32> @offset(16), @location(2)
}

%foo = @vertex func(%input:Inputs):vec4<f32> [@position] {
  $B1: {
    %uint32x2:u32 = access %input, 0u
    %uint32x4:vec2<u32> = access %input, 1u
    %uint32x4_1:vec3<u32> = access %input, 2u  # %uint32x4_1: 'uint32x4'
    ret vec4<f32>(0.0f)
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inputs = struct @align(16) {
  uint32:u32 @offset(0), @location(0)
  uint32x2:vec2<u32> @offset(8), @location(1)
  uint32x3:vec3<u32> @offset(16), @location(2)
}

$B1: {  # root
  %tint_vertex_buffer_0:ptr<storage, array<u32>, read> = var @binding_point(4, 0)
}

%foo = @vertex func(%tint_vertex_index:u32 [@vertex_index]):vec4<f32> [@position] {
  $B2: {
    %tint_vertex_buffer_0_base:u32 = mul %tint_vertex_index, 64u
    %5:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %tint_vertex_buffer_0_base
    %6:u32 = load %5
    %7:u32 = add %tint_vertex_buffer_0_base, 1u
    %8:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %7
    %9:u32 = load %8
    %10:vec2<u32> = construct %6, %9
    %11:u32 = swizzle %10, x
    %12:u32 = add %tint_vertex_buffer_0_base, 4u
    %13:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %12
    %14:u32 = load %13
    %15:u32 = add %tint_vertex_buffer_0_base, 5u
    %16:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %15
    %17:u32 = load %16
    %18:u32 = add %tint_vertex_buffer_0_base, 6u
    %19:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %18
    %20:u32 = load %19
    %21:u32 = add %tint_vertex_buffer_0_base, 7u
    %22:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %21
    %23:u32 = load %22
    %24:vec4<u32> = construct %14, %17, %20, %23
    %25:vec2<u32> = swizzle %24, xy
    %26:u32 = add %tint_vertex_buffer_0_base, 8u
    %27:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %26
    %28:u32 = load %27
    %29:u32 = add %tint_vertex_buffer_0_base, 9u
    %30:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %29
    %31:u32 = load %30
    %32:u32 = add %tint_vertex_buffer_0_base, 10u
    %33:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %32
    %34:u32 = load %33
    %35:u32 = add %tint_vertex_buffer_0_base, 11u
    %36:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %35
    %37:u32 = load %36
    %38:vec4<u32> = construct %28, %31, %34, %37
    %39:vec3<u32> = swizzle %38, xyz
    %40:Inputs = construct %11, %25, %39
    %uint32x2:u32 = access %40, 0u
    %uint32x4:vec2<u32> = access %40, 1u
    %uint32x4_1:vec3<u32> = access %40, 2u  # %uint32x4_1: 'uint32x4'
    ret vec4<f32>(0.0f)
  }
}
)";

    VertexPullingConfig cfg;
    cfg.pulling_group = 4u;
    cfg.vertex_state = {{{256,
                          VertexStepMode::kVertex,
                          {
                              {VertexFormat::kUint32x2, 0, 0},   //
                              {VertexFormat::kUint32x4, 16, 1},  //
                              {VertexFormat::kUint32x4, 32, 2},  //
                          }}}};
    Run(VertexPulling, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_VertexPullingTest, ShaderTypeTooManyComponents_U32) {
    auto* inputs =
        ty.Struct(mod.symbols.New("Inputs"),
                  {
                      {mod.symbols.New("vec2u_from_u32"), ty.vec2<u32>(), Location(0)},   //
                      {mod.symbols.New("vec3u_from_u32"), ty.vec3<u32>(), Location(1)},   //
                      {mod.symbols.New("vec4u_from_u32"), ty.vec4<u32>(), Location(2)},   //
                      {mod.symbols.New("vec3u_from_vec2"), ty.vec3<u32>(), Location(3)},  //
                      {mod.symbols.New("vec4u_from_vec2"), ty.vec4<u32>(), Location(4)},  //
                      {mod.symbols.New("vec4u_from_vec3"), ty.vec4<u32>(), Location(5)},  //
                  });

    auto* ep = b.Function("foo", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    auto* param = b.FunctionParam("input", inputs);
    ep->AppendParam(param);
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    b.Append(ep->Block(), [&] {  //
        mod.SetName(b.Access(ty.vec2<u32>(), param, 0_u), "vec2u_from_u32");
        mod.SetName(b.Access(ty.vec3<u32>(), param, 1_u), "vec3u_from_u32");
        mod.SetName(b.Access(ty.vec4<u32>(), param, 2_u), "vec4u_from_u32");
        mod.SetName(b.Access(ty.vec3<u32>(), param, 3_u), "vec3u_from_vec2");
        mod.SetName(b.Access(ty.vec4<u32>(), param, 4_u), "vec4u_from_vec2");
        mod.SetName(b.Access(ty.vec4<u32>(), param, 5_u), "vec4u_from_vec3");
        b.Return(ep, b.Zero<vec4<f32>>());
    });

    auto* src = R"(
Inputs = struct @align(16) {
  vec2u_from_u32:vec2<u32> @offset(0), @location(0)
  vec3u_from_u32:vec3<u32> @offset(16), @location(1)
  vec4u_from_u32:vec4<u32> @offset(32), @location(2)
  vec3u_from_vec2:vec3<u32> @offset(48), @location(3)
  vec4u_from_vec2:vec4<u32> @offset(64), @location(4)
  vec4u_from_vec3:vec4<u32> @offset(80), @location(5)
}

%foo = @vertex func(%input:Inputs):vec4<f32> [@position] {
  $B1: {
    %vec2u_from_u32:vec2<u32> = access %input, 0u
    %vec3u_from_u32:vec3<u32> = access %input, 1u
    %vec4u_from_u32:vec4<u32> = access %input, 2u
    %vec3u_from_vec2:vec3<u32> = access %input, 3u
    %vec4u_from_vec2:vec4<u32> = access %input, 4u
    %vec4u_from_vec3:vec4<u32> = access %input, 5u
    ret vec4<f32>(0.0f)
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inputs = struct @align(16) {
  vec2u_from_u32:vec2<u32> @offset(0), @location(0)
  vec3u_from_u32:vec3<u32> @offset(16), @location(1)
  vec4u_from_u32:vec4<u32> @offset(32), @location(2)
  vec3u_from_vec2:vec3<u32> @offset(48), @location(3)
  vec4u_from_vec2:vec4<u32> @offset(64), @location(4)
  vec4u_from_vec3:vec4<u32> @offset(80), @location(5)
}

$B1: {  # root
  %tint_vertex_buffer_0:ptr<storage, array<u32>, read> = var @binding_point(4, 0)
}

%foo = @vertex func(%tint_vertex_index:u32 [@vertex_index]):vec4<f32> [@position] {
  $B2: {
    %tint_vertex_buffer_0_base:u32 = mul %tint_vertex_index, 64u
    %5:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %tint_vertex_buffer_0_base
    %6:u32 = load %5
    %7:vec2<u32> = construct %6, 0u
    %8:u32 = add %tint_vertex_buffer_0_base, 4u
    %9:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %8
    %10:u32 = load %9
    %11:vec3<u32> = construct %10, 0u, 0u
    %12:u32 = add %tint_vertex_buffer_0_base, 8u
    %13:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %12
    %14:u32 = load %13
    %15:vec4<u32> = construct %14, 0u, 0u, 1u
    %16:u32 = add %tint_vertex_buffer_0_base, 12u
    %17:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %16
    %18:u32 = load %17
    %19:u32 = add %tint_vertex_buffer_0_base, 13u
    %20:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %19
    %21:u32 = load %20
    %22:vec2<u32> = construct %18, %21
    %23:vec3<u32> = construct %22, 0u
    %24:u32 = add %tint_vertex_buffer_0_base, 16u
    %25:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %24
    %26:u32 = load %25
    %27:u32 = add %tint_vertex_buffer_0_base, 17u
    %28:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %27
    %29:u32 = load %28
    %30:vec2<u32> = construct %26, %29
    %31:vec4<u32> = construct %30, 0u, 1u
    %32:u32 = add %tint_vertex_buffer_0_base, 20u
    %33:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %32
    %34:u32 = load %33
    %35:u32 = add %tint_vertex_buffer_0_base, 21u
    %36:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %35
    %37:u32 = load %36
    %38:u32 = add %tint_vertex_buffer_0_base, 22u
    %39:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %38
    %40:u32 = load %39
    %41:vec3<u32> = construct %34, %37, %40
    %42:vec4<u32> = construct %41, 1u
    %43:Inputs = construct %7, %11, %15, %23, %31, %42
    %vec2u_from_u32:vec2<u32> = access %43, 0u
    %vec3u_from_u32:vec3<u32> = access %43, 1u
    %vec4u_from_u32:vec4<u32> = access %43, 2u
    %vec3u_from_vec2:vec3<u32> = access %43, 3u
    %vec4u_from_vec2:vec4<u32> = access %43, 4u
    %vec4u_from_vec3:vec4<u32> = access %43, 5u
    ret vec4<f32>(0.0f)
  }
}
)";

    VertexPullingConfig cfg;
    cfg.pulling_group = 4u;
    cfg.vertex_state = {{{256,
                          VertexStepMode::kVertex,
                          {
                              {VertexFormat::kUint32, 0, 0},     //
                              {VertexFormat::kUint32, 16, 1},    //
                              {VertexFormat::kUint32, 32, 2},    //
                              {VertexFormat::kUint32x2, 48, 3},  //
                              {VertexFormat::kUint32x2, 64, 4},  //
                              {VertexFormat::kUint32x3, 80, 5},  //
                          }}}};
    Run(VertexPulling, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_VertexPullingTest, ShaderTypeTooManyComponents_I32) {
    auto* inputs =
        ty.Struct(mod.symbols.New("Inputs"),
                  {
                      {mod.symbols.New("vec2i_from_i32"), ty.vec2<i32>(), Location(0)},   //
                      {mod.symbols.New("vec3i_from_i32"), ty.vec3<i32>(), Location(1)},   //
                      {mod.symbols.New("vec4i_from_i32"), ty.vec4<i32>(), Location(2)},   //
                      {mod.symbols.New("vec3i_from_vec2"), ty.vec3<i32>(), Location(3)},  //
                      {mod.symbols.New("vec4i_from_vec2"), ty.vec4<i32>(), Location(4)},  //
                      {mod.symbols.New("vec4i_from_vec3"), ty.vec4<i32>(), Location(5)},  //
                  });

    auto* ep = b.Function("foo", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    auto* param = b.FunctionParam("input", inputs);
    ep->AppendParam(param);
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    b.Append(ep->Block(), [&] {  //
        mod.SetName(b.Access(ty.vec2<i32>(), param, 0_u), "vec2i_from_i32");
        mod.SetName(b.Access(ty.vec3<i32>(), param, 1_u), "vec3i_from_i32");
        mod.SetName(b.Access(ty.vec4<i32>(), param, 2_u), "vec4i_from_i32");
        mod.SetName(b.Access(ty.vec3<i32>(), param, 3_u), "vec3i_from_vec2");
        mod.SetName(b.Access(ty.vec4<i32>(), param, 4_u), "vec4i_from_vec2");
        mod.SetName(b.Access(ty.vec4<i32>(), param, 5_u), "vec4i_from_vec3");
        b.Return(ep, b.Zero<vec4<f32>>());
    });

    auto* src = R"(
Inputs = struct @align(16) {
  vec2i_from_i32:vec2<i32> @offset(0), @location(0)
  vec3i_from_i32:vec3<i32> @offset(16), @location(1)
  vec4i_from_i32:vec4<i32> @offset(32), @location(2)
  vec3i_from_vec2:vec3<i32> @offset(48), @location(3)
  vec4i_from_vec2:vec4<i32> @offset(64), @location(4)
  vec4i_from_vec3:vec4<i32> @offset(80), @location(5)
}

%foo = @vertex func(%input:Inputs):vec4<f32> [@position] {
  $B1: {
    %vec2i_from_i32:vec2<i32> = access %input, 0u
    %vec3i_from_i32:vec3<i32> = access %input, 1u
    %vec4i_from_i32:vec4<i32> = access %input, 2u
    %vec3i_from_vec2:vec3<i32> = access %input, 3u
    %vec4i_from_vec2:vec4<i32> = access %input, 4u
    %vec4i_from_vec3:vec4<i32> = access %input, 5u
    ret vec4<f32>(0.0f)
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inputs = struct @align(16) {
  vec2i_from_i32:vec2<i32> @offset(0), @location(0)
  vec3i_from_i32:vec3<i32> @offset(16), @location(1)
  vec4i_from_i32:vec4<i32> @offset(32), @location(2)
  vec3i_from_vec2:vec3<i32> @offset(48), @location(3)
  vec4i_from_vec2:vec4<i32> @offset(64), @location(4)
  vec4i_from_vec3:vec4<i32> @offset(80), @location(5)
}

$B1: {  # root
  %tint_vertex_buffer_0:ptr<storage, array<u32>, read> = var @binding_point(4, 0)
}

%foo = @vertex func(%tint_vertex_index:u32 [@vertex_index]):vec4<f32> [@position] {
  $B2: {
    %tint_vertex_buffer_0_base:u32 = mul %tint_vertex_index, 64u
    %5:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %tint_vertex_buffer_0_base
    %6:u32 = load %5
    %7:i32 = bitcast %6
    %8:vec2<i32> = construct %7, 0i
    %9:u32 = add %tint_vertex_buffer_0_base, 4u
    %10:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %9
    %11:u32 = load %10
    %12:i32 = bitcast %11
    %13:vec3<i32> = construct %12, 0i, 0i
    %14:u32 = add %tint_vertex_buffer_0_base, 8u
    %15:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %14
    %16:u32 = load %15
    %17:i32 = bitcast %16
    %18:vec4<i32> = construct %17, 0i, 0i, 1i
    %19:u32 = add %tint_vertex_buffer_0_base, 12u
    %20:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %19
    %21:u32 = load %20
    %22:i32 = bitcast %21
    %23:u32 = add %tint_vertex_buffer_0_base, 13u
    %24:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %23
    %25:u32 = load %24
    %26:i32 = bitcast %25
    %27:vec2<i32> = construct %22, %26
    %28:vec3<i32> = construct %27, 0i
    %29:u32 = add %tint_vertex_buffer_0_base, 16u
    %30:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %29
    %31:u32 = load %30
    %32:i32 = bitcast %31
    %33:u32 = add %tint_vertex_buffer_0_base, 17u
    %34:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %33
    %35:u32 = load %34
    %36:i32 = bitcast %35
    %37:vec2<i32> = construct %32, %36
    %38:vec4<i32> = construct %37, 0i, 1i
    %39:u32 = add %tint_vertex_buffer_0_base, 20u
    %40:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %39
    %41:u32 = load %40
    %42:i32 = bitcast %41
    %43:u32 = add %tint_vertex_buffer_0_base, 21u
    %44:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %43
    %45:u32 = load %44
    %46:i32 = bitcast %45
    %47:u32 = add %tint_vertex_buffer_0_base, 22u
    %48:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %47
    %49:u32 = load %48
    %50:i32 = bitcast %49
    %51:vec3<i32> = construct %42, %46, %50
    %52:vec4<i32> = construct %51, 1i
    %53:Inputs = construct %8, %13, %18, %28, %38, %52
    %vec2i_from_i32:vec2<i32> = access %53, 0u
    %vec3i_from_i32:vec3<i32> = access %53, 1u
    %vec4i_from_i32:vec4<i32> = access %53, 2u
    %vec3i_from_vec2:vec3<i32> = access %53, 3u
    %vec4i_from_vec2:vec4<i32> = access %53, 4u
    %vec4i_from_vec3:vec4<i32> = access %53, 5u
    ret vec4<f32>(0.0f)
  }
}
)";

    VertexPullingConfig cfg;
    cfg.pulling_group = 4u;
    cfg.vertex_state = {{{256,
                          VertexStepMode::kVertex,
                          {
                              {VertexFormat::kSint32, 0, 0},     //
                              {VertexFormat::kSint32, 16, 1},    //
                              {VertexFormat::kSint32, 32, 2},    //
                              {VertexFormat::kSint32x2, 48, 3},  //
                              {VertexFormat::kSint32x2, 64, 4},  //
                              {VertexFormat::kSint32x3, 80, 5},  //
                          }}}};
    Run(VertexPulling, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_VertexPullingTest, ShaderTypeTooManyComponents_F32) {
    auto* inputs =
        ty.Struct(mod.symbols.New("Inputs"),
                  {
                      {mod.symbols.New("vec2f_from_f16"), ty.vec2<f32>(), Location(0)},           //
                      {mod.symbols.New("vec3f_from_f32"), ty.vec3<f32>(), Location(1)},           //
                      {mod.symbols.New("vec4f_from_unorm"), ty.vec4<f32>(), Location(2)},         //
                      {mod.symbols.New("vec3f_from_snormx2"), ty.vec3<f32>(), Location(3)},       //
                      {mod.symbols.New("vec4f_from_f32x2"), ty.vec4<f32>(), Location(4)},         //
                      {mod.symbols.New("vec4f_from_unorm_packed"), ty.vec4<f32>(), Location(5)},  //
                  });

    auto* ep = b.Function("foo", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    auto* param = b.FunctionParam("input", inputs);
    ep->AppendParam(param);
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    b.Append(ep->Block(), [&] {  //
        mod.SetName(b.Access(ty.vec2<f32>(), param, 0_u), "vec2f_from_f16");
        mod.SetName(b.Access(ty.vec3<f32>(), param, 1_u), "vec3f_from_f32");
        mod.SetName(b.Access(ty.vec4<f32>(), param, 2_u), "vec4f_from_unorm");
        mod.SetName(b.Access(ty.vec3<f32>(), param, 3_u), "vec3f_from_snormx2");
        mod.SetName(b.Access(ty.vec4<f32>(), param, 4_u), "vec4f_from_f32x2");
        mod.SetName(b.Access(ty.vec4<f32>(), param, 5_u), "vec4f_from_unorm_packed");
        b.Return(ep, b.Zero<vec4<f32>>());
    });

    auto* src = R"(
Inputs = struct @align(16) {
  vec2f_from_f16:vec2<f32> @offset(0), @location(0)
  vec3f_from_f32:vec3<f32> @offset(16), @location(1)
  vec4f_from_unorm:vec4<f32> @offset(32), @location(2)
  vec3f_from_snormx2:vec3<f32> @offset(48), @location(3)
  vec4f_from_f32x2:vec4<f32> @offset(64), @location(4)
  vec4f_from_unorm_packed:vec4<f32> @offset(80), @location(5)
}

%foo = @vertex func(%input:Inputs):vec4<f32> [@position] {
  $B1: {
    %vec2f_from_f16:vec2<f32> = access %input, 0u
    %vec3f_from_f32:vec3<f32> = access %input, 1u
    %vec4f_from_unorm:vec4<f32> = access %input, 2u
    %vec3f_from_snormx2:vec3<f32> = access %input, 3u
    %vec4f_from_f32x2:vec4<f32> = access %input, 4u
    %vec4f_from_unorm_packed:vec4<f32> = access %input, 5u
    ret vec4<f32>(0.0f)
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inputs = struct @align(16) {
  vec2f_from_f16:vec2<f32> @offset(0), @location(0)
  vec3f_from_f32:vec3<f32> @offset(16), @location(1)
  vec4f_from_unorm:vec4<f32> @offset(32), @location(2)
  vec3f_from_snormx2:vec3<f32> @offset(48), @location(3)
  vec4f_from_f32x2:vec4<f32> @offset(64), @location(4)
  vec4f_from_unorm_packed:vec4<f32> @offset(80), @location(5)
}

$B1: {  # root
  %tint_vertex_buffer_0:ptr<storage, array<u32>, read> = var @binding_point(4, 0)
}

%foo = @vertex func(%tint_vertex_index:u32 [@vertex_index]):vec4<f32> [@position] {
  $B2: {
    %tint_vertex_buffer_0_base:u32 = mul %tint_vertex_index, 64u
    %5:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %tint_vertex_buffer_0_base
    %6:u32 = load %5
    %7:vec2<f32> = unpack2x16float %6
    %8:f32 = access %7, 0u
    %9:vec2<f32> = construct %8, 0.0f
    %10:u32 = add %tint_vertex_buffer_0_base, 4u
    %11:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %10
    %12:u32 = load %11
    %13:f32 = bitcast %12
    %14:vec3<f32> = construct %13, 0.0f, 0.0f
    %15:u32 = add %tint_vertex_buffer_0_base, 8u
    %16:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %15
    %17:u32 = load %16
    %18:vec4<f32> = unpack4x8unorm %17
    %19:f32 = access %18, 0u
    %20:vec4<f32> = construct %19, 0.0f, 0.0f, 1.0f
    %21:u32 = add %tint_vertex_buffer_0_base, 12u
    %22:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %21
    %23:u32 = load %22
    %24:vec2<f32> = unpack2x16snorm %23
    %25:vec3<f32> = construct %24, 0.0f
    %26:u32 = add %tint_vertex_buffer_0_base, 16u
    %27:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %26
    %28:u32 = load %27
    %29:f32 = bitcast %28
    %30:u32 = add %tint_vertex_buffer_0_base, 17u
    %31:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %30
    %32:u32 = load %31
    %33:f32 = bitcast %32
    %34:vec2<f32> = construct %29, %33
    %35:vec4<f32> = construct %34, 0.0f, 1.0f
    %36:u32 = add %tint_vertex_buffer_0_base, 20u
    %37:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %36
    %38:u32 = load %37
    %39:vec4<u32> = construct %38
    %40:vec4<u32> = shr %39, vec4<u32>(0u, 10u, 20u, 30u)
    %41:vec4<u32> = and %40, vec4<u32>(1023u, 1023u, 1023u, 3u)
    %42:vec4<f32> = convert %41
    %43:vec4<f32> = div %42, vec4<f32>(1023.0f, 1023.0f, 1023.0f, 3.0f)
    %44:Inputs = construct %9, %14, %20, %25, %35, %43
    %vec2f_from_f16:vec2<f32> = access %44, 0u
    %vec3f_from_f32:vec3<f32> = access %44, 1u
    %vec4f_from_unorm:vec4<f32> = access %44, 2u
    %vec3f_from_snormx2:vec3<f32> = access %44, 3u
    %vec4f_from_f32x2:vec4<f32> = access %44, 4u
    %vec4f_from_unorm_packed:vec4<f32> = access %44, 5u
    ret vec4<f32>(0.0f)
  }
}
)";

    VertexPullingConfig cfg;
    cfg.pulling_group = 4u;
    cfg.vertex_state = {{{256,
                          VertexStepMode::kVertex,
                          {
                              {VertexFormat::kFloat16, 0, 0},           //
                              {VertexFormat::kFloat32, 16, 1},          //
                              {VertexFormat::kUnorm8, 32, 2},           //
                              {VertexFormat::kSnorm16x2, 48, 3},        //
                              {VertexFormat::kFloat32x2, 64, 4},        //
                              {VertexFormat::kUnorm10_10_10_2, 80, 5},  //
                          }}}};
    Run(VertexPulling, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_VertexPullingTest, ShaderTypeTooManyComponents_F16) {
    auto* inputs =
        ty.Struct(mod.symbols.New("Inputs"),
                  {
                      {mod.symbols.New("vec2h_from_f16"), ty.vec2<f16>(), Location(0)},           //
                      {mod.symbols.New("vec3h_from_f32"), ty.vec3<f16>(), Location(1)},           //
                      {mod.symbols.New("vec4h_from_unorm"), ty.vec4<f16>(), Location(2)},         //
                      {mod.symbols.New("vec3h_from_snormx2"), ty.vec3<f16>(), Location(3)},       //
                      {mod.symbols.New("vec4h_from_f16x2"), ty.vec4<f16>(), Location(4)},         //
                      {mod.symbols.New("vec4h_from_f32x2"), ty.vec4<f16>(), Location(5)},         //
                      {mod.symbols.New("vec4h_from_unorm_packed"), ty.vec4<f16>(), Location(6)},  //
                  });

    auto* ep = b.Function("foo", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    auto* param = b.FunctionParam("input", inputs);
    ep->AppendParam(param);
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    b.Append(ep->Block(), [&] {  //
        mod.SetName(b.Access(ty.vec2<f16>(), param, 0_u), "vec2h_from_f16");
        mod.SetName(b.Access(ty.vec3<f16>(), param, 1_u), "vec3h_from_f32");
        mod.SetName(b.Access(ty.vec4<f16>(), param, 2_u), "vec4h_from_unorm");
        mod.SetName(b.Access(ty.vec3<f16>(), param, 3_u), "vec3h_from_snormx2");
        mod.SetName(b.Access(ty.vec4<f16>(), param, 4_u), "vec4h_from_f16x2");
        mod.SetName(b.Access(ty.vec4<f16>(), param, 5_u), "vec4h_from_f32x2");
        mod.SetName(b.Access(ty.vec4<f16>(), param, 6_u), "vec4h_from_unorm_packed");
        b.Return(ep, b.Zero<vec4<f32>>());
    });

    auto* src = R"(
Inputs = struct @align(8) {
  vec2h_from_f16:vec2<f16> @offset(0), @location(0)
  vec3h_from_f32:vec3<f16> @offset(8), @location(1)
  vec4h_from_unorm:vec4<f16> @offset(16), @location(2)
  vec3h_from_snormx2:vec3<f16> @offset(24), @location(3)
  vec4h_from_f16x2:vec4<f16> @offset(32), @location(4)
  vec4h_from_f32x2:vec4<f16> @offset(40), @location(5)
  vec4h_from_unorm_packed:vec4<f16> @offset(48), @location(6)
}

%foo = @vertex func(%input:Inputs):vec4<f32> [@position] {
  $B1: {
    %vec2h_from_f16:vec2<f16> = access %input, 0u
    %vec3h_from_f32:vec3<f16> = access %input, 1u
    %vec4h_from_unorm:vec4<f16> = access %input, 2u
    %vec3h_from_snormx2:vec3<f16> = access %input, 3u
    %vec4h_from_f16x2:vec4<f16> = access %input, 4u
    %vec4h_from_f32x2:vec4<f16> = access %input, 5u
    %vec4h_from_unorm_packed:vec4<f16> = access %input, 6u
    ret vec4<f32>(0.0f)
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inputs = struct @align(8) {
  vec2h_from_f16:vec2<f16> @offset(0), @location(0)
  vec3h_from_f32:vec3<f16> @offset(8), @location(1)
  vec4h_from_unorm:vec4<f16> @offset(16), @location(2)
  vec3h_from_snormx2:vec3<f16> @offset(24), @location(3)
  vec4h_from_f16x2:vec4<f16> @offset(32), @location(4)
  vec4h_from_f32x2:vec4<f16> @offset(40), @location(5)
  vec4h_from_unorm_packed:vec4<f16> @offset(48), @location(6)
}

$B1: {  # root
  %tint_vertex_buffer_0:ptr<storage, array<u32>, read> = var @binding_point(4, 0)
}

%foo = @vertex func(%tint_vertex_index:u32 [@vertex_index]):vec4<f32> [@position] {
  $B2: {
    %tint_vertex_buffer_0_base:u32 = mul %tint_vertex_index, 64u
    %5:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %tint_vertex_buffer_0_base
    %6:u32 = load %5
    %7:vec2<f16> = bitcast %6
    %8:f16 = access %7, 0u
    %9:vec2<f16> = construct %8, 0.0h
    %10:u32 = add %tint_vertex_buffer_0_base, 4u
    %11:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %10
    %12:u32 = load %11
    %13:f32 = bitcast %12
    %14:f16 = convert %13
    %15:vec3<f16> = construct %14, 0.0h, 0.0h
    %16:u32 = add %tint_vertex_buffer_0_base, 8u
    %17:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %16
    %18:u32 = load %17
    %19:vec4<f32> = unpack4x8unorm %18
    %20:f32 = access %19, 0u
    %21:f16 = convert %20
    %22:vec4<f16> = construct %21, 0.0h, 0.0h, 1.0h
    %23:u32 = add %tint_vertex_buffer_0_base, 12u
    %24:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %23
    %25:u32 = load %24
    %26:vec2<f32> = unpack2x16snorm %25
    %27:vec2<f16> = convert %26
    %28:vec3<f16> = construct %27, 0.0h
    %29:u32 = add %tint_vertex_buffer_0_base, 16u
    %30:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %29
    %31:u32 = load %30
    %32:vec2<f16> = bitcast %31
    %33:vec4<f16> = construct %32, 0.0h, 1.0h
    %34:u32 = add %tint_vertex_buffer_0_base, 20u
    %35:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %34
    %36:u32 = load %35
    %37:f32 = bitcast %36
    %38:u32 = add %tint_vertex_buffer_0_base, 21u
    %39:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %38
    %40:u32 = load %39
    %41:f32 = bitcast %40
    %42:vec2<f32> = construct %37, %41
    %43:vec2<f16> = convert %42
    %44:vec4<f16> = construct %43, 0.0h, 1.0h
    %45:u32 = add %tint_vertex_buffer_0_base, 24u
    %46:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %45
    %47:u32 = load %46
    %48:vec4<u32> = construct %47
    %49:vec4<u32> = shr %48, vec4<u32>(0u, 10u, 20u, 30u)
    %50:vec4<u32> = and %49, vec4<u32>(1023u, 1023u, 1023u, 3u)
    %51:vec4<f32> = convert %50
    %52:vec4<f32> = div %51, vec4<f32>(1023.0f, 1023.0f, 1023.0f, 3.0f)
    %53:vec4<f16> = convert %52
    %54:Inputs = construct %9, %15, %22, %28, %33, %44, %53
    %vec2h_from_f16:vec2<f16> = access %54, 0u
    %vec3h_from_f32:vec3<f16> = access %54, 1u
    %vec4h_from_unorm:vec4<f16> = access %54, 2u
    %vec3h_from_snormx2:vec3<f16> = access %54, 3u
    %vec4h_from_f16x2:vec4<f16> = access %54, 4u
    %vec4h_from_f32x2:vec4<f16> = access %54, 5u
    %vec4h_from_unorm_packed:vec4<f16> = access %54, 6u
    ret vec4<f32>(0.0f)
  }
}
)";

    VertexPullingConfig cfg;
    cfg.pulling_group = 4u;
    cfg.vertex_state = {{{256,
                          VertexStepMode::kVertex,
                          {
                              {VertexFormat::kFloat16, 0, 0},           //
                              {VertexFormat::kFloat32, 16, 1},          //
                              {VertexFormat::kUnorm8, 32, 2},           //
                              {VertexFormat::kSnorm16x2, 48, 3},        //
                              {VertexFormat::kFloat16x2, 64, 4},        //
                              {VertexFormat::kFloat32x2, 80, 5},        //
                              {VertexFormat::kUnorm10_10_10_2, 96, 6},  //
                          }}}};
    Run(VertexPulling, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_VertexPullingTest, Unaligned_Uint8) {
    auto* inputs = ty.Struct(mod.symbols.New("Inputs"),
                             {
                                 {mod.symbols.New("uint8_offset1"), ty.u32(), Location(0)},  //
                                 {mod.symbols.New("uint8_offset2"), ty.u32(), Location(1)},  //
                                 {mod.symbols.New("uint8_offset3"), ty.u32(), Location(2)},  //
                             });

    auto* ep = b.Function("foo", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    auto* param = b.FunctionParam("input", inputs);
    ep->AppendParam(param);
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    b.Append(ep->Block(), [&] {  //
        mod.SetName(b.Access(ty.u32(), param, 0_u), "uint8_offset1");
        mod.SetName(b.Access(ty.u32(), param, 1_u), "uint8_offset2");
        mod.SetName(b.Access(ty.u32(), param, 2_u), "uint8_offset3");
        b.Return(ep, b.Zero<vec4<f32>>());
    });

    auto* src = R"(
Inputs = struct @align(4) {
  uint8_offset1:u32 @offset(0), @location(0)
  uint8_offset2:u32 @offset(4), @location(1)
  uint8_offset3:u32 @offset(8), @location(2)
}

%foo = @vertex func(%input:Inputs):vec4<f32> [@position] {
  $B1: {
    %uint8_offset1:u32 = access %input, 0u
    %uint8_offset2:u32 = access %input, 1u
    %uint8_offset3:u32 = access %input, 2u
    ret vec4<f32>(0.0f)
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inputs = struct @align(4) {
  uint8_offset1:u32 @offset(0), @location(0)
  uint8_offset2:u32 @offset(4), @location(1)
  uint8_offset3:u32 @offset(8), @location(2)
}

$B1: {  # root
  %tint_vertex_buffer_0:ptr<storage, array<u32>, read> = var @binding_point(4, 0)
}

%foo = @vertex func(%tint_vertex_index:u32 [@vertex_index]):vec4<f32> [@position] {
  $B2: {
    %tint_vertex_buffer_0_base:u32 = mul %tint_vertex_index, 64u
    %5:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %tint_vertex_buffer_0_base
    %6:u32 = load %5
    %7:u32 = shr %6, 8u
    %8:u32 = and %7, 255u
    %9:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %tint_vertex_buffer_0_base
    %10:u32 = load %9
    %11:u32 = shr %10, 16u
    %12:u32 = and %11, 255u
    %13:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %tint_vertex_buffer_0_base
    %14:u32 = load %13
    %15:u32 = shr %14, 24u
    %16:u32 = and %15, 255u
    %17:Inputs = construct %8, %12, %16
    %uint8_offset1:u32 = access %17, 0u
    %uint8_offset2:u32 = access %17, 1u
    %uint8_offset3:u32 = access %17, 2u
    ret vec4<f32>(0.0f)
  }
}
)";

    VertexPullingConfig cfg;
    cfg.pulling_group = 4u;
    cfg.vertex_state = {{{256,
                          VertexStepMode::kVertex,
                          {
                              {VertexFormat::kUint8, 1, 0},  //
                              {VertexFormat::kUint8, 2, 1},  //
                              {VertexFormat::kUint8, 3, 2},  //
                          }}}};
    Run(VertexPulling, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_VertexPullingTest, Unaligned_Uint8x2) {
    auto* inputs =
        ty.Struct(mod.symbols.New("Inputs"),
                  {
                      {mod.symbols.New("uint8x2_offset2"), ty.vec2<u32>(), Location(0)},  //
                  });

    auto* ep = b.Function("foo", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    auto* param = b.FunctionParam("input", inputs);
    ep->AppendParam(param);
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    b.Append(ep->Block(), [&] {  //
        mod.SetName(b.Access(ty.vec2<u32>(), param, 0_u), "uint8x2_offset2");
        b.Return(ep, b.Zero<vec4<f32>>());
    });

    auto* src = R"(
Inputs = struct @align(8) {
  uint8x2_offset2:vec2<u32> @offset(0), @location(0)
}

%foo = @vertex func(%input:Inputs):vec4<f32> [@position] {
  $B1: {
    %uint8x2_offset2:vec2<u32> = access %input, 0u
    ret vec4<f32>(0.0f)
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inputs = struct @align(8) {
  uint8x2_offset2:vec2<u32> @offset(0), @location(0)
}

$B1: {  # root
  %tint_vertex_buffer_0:ptr<storage, array<u32>, read> = var @binding_point(4, 0)
}

%foo = @vertex func(%tint_vertex_index:u32 [@vertex_index]):vec4<f32> [@position] {
  $B2: {
    %tint_vertex_buffer_0_base:u32 = mul %tint_vertex_index, 64u
    %5:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %tint_vertex_buffer_0_base
    %6:u32 = load %5
    %7:u32 = shr %6, 16u
    %8:vec2<u32> = construct %7
    %9:vec2<u32> = shl %8, vec2<u32>(24u, 16u)
    %10:vec2<u32> = shr %9, vec2<u32>(24u)
    %11:Inputs = construct %10
    %uint8x2_offset2:vec2<u32> = access %11, 0u
    ret vec4<f32>(0.0f)
  }
}
)";

    VertexPullingConfig cfg;
    cfg.pulling_group = 4u;
    cfg.vertex_state = {{{256,
                          VertexStepMode::kVertex,
                          {
                              {VertexFormat::kUint8x2, 2, 0},  //
                          }}}};
    Run(VertexPulling, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_VertexPullingTest, Unaligned_Uint16) {
    auto* inputs = ty.Struct(mod.symbols.New("Inputs"),
                             {
                                 {mod.symbols.New("uint16_offset2"), ty.u32(), Location(0)},  //
                             });

    auto* ep = b.Function("foo", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    auto* param = b.FunctionParam("input", inputs);
    ep->AppendParam(param);
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    b.Append(ep->Block(), [&] {  //
        mod.SetName(b.Access(ty.u32(), param, 0_u), "uint16_offset2");
        b.Return(ep, b.Zero<vec4<f32>>());
    });

    auto* src = R"(
Inputs = struct @align(4) {
  uint16_offset2:u32 @offset(0), @location(0)
}

%foo = @vertex func(%input:Inputs):vec4<f32> [@position] {
  $B1: {
    %uint16_offset2:u32 = access %input, 0u
    ret vec4<f32>(0.0f)
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inputs = struct @align(4) {
  uint16_offset2:u32 @offset(0), @location(0)
}

$B1: {  # root
  %tint_vertex_buffer_0:ptr<storage, array<u32>, read> = var @binding_point(4, 0)
}

%foo = @vertex func(%tint_vertex_index:u32 [@vertex_index]):vec4<f32> [@position] {
  $B2: {
    %tint_vertex_buffer_0_base:u32 = mul %tint_vertex_index, 64u
    %5:ptr<storage, u32, read> = access %tint_vertex_buffer_0, %tint_vertex_buffer_0_base
    %6:u32 = load %5
    %7:u32 = shr %6, 16u
    %8:u32 = and %7, 65535u
    %9:Inputs = construct %8
    %uint16_offset2:u32 = access %9, 0u
    ret vec4<f32>(0.0f)
  }
}
)";

    VertexPullingConfig cfg;
    cfg.pulling_group = 4u;
    cfg.vertex_state = {{{256,
                          VertexStepMode::kVertex,
                          {
                              {VertexFormat::kUint16, 2, 0},  //
                          }}}};
    Run(VertexPulling, cfg);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform

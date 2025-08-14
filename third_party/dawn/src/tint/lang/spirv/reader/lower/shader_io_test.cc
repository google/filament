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

#include "src/tint/lang/spirv/reader/lower/shader_io.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::spirv::reader::lower {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class SpirvReader_ShaderIOTest : public core::ir::transform::TransformTest {
  public:
    void SetUp() override { capabilities.Add(core::ir::Capability::kAllowMultipleEntryPoints); }

  protected:
    core::IOAttributes BuiltinAttrs(core::BuiltinValue builtin) {
        core::IOAttributes attrs;
        attrs.builtin = builtin;
        return attrs;
    }
    core::IOAttributes LocationAttrs(
        uint32_t location,
        std::optional<core::Interpolation> interpolation = std::nullopt) {
        core::IOAttributes attrs;
        attrs.location = location;
        attrs.interpolation = interpolation;
        return attrs;
    }
};

TEST_F(SpirvReader_ShaderIOTest, NoInputsOrOutputs) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, Inputs) {
    auto* front_facing = b.Var("front_facing", ty.ptr(core::AddressSpace::kIn, ty.bool_()));
    front_facing->SetBuiltin(core::BuiltinValue::kFrontFacing);

    auto* position = b.Var("position", ty.ptr(core::AddressSpace::kIn, ty.vec4<f32>()));
    position->SetBuiltin(core::BuiltinValue::kPosition);
    position->SetInvariant(true);

    auto* color1 = b.Var("color1", ty.ptr(core::AddressSpace::kIn, ty.f32()));
    color1->SetLocation(0);

    auto* color2 = b.Var("color2", ty.ptr(core::AddressSpace::kIn, ty.f32()));
    color2->SetLocation(1);
    color2->SetInterpolation(core::Interpolation{core::InterpolationType::kLinear,
                                                 core::InterpolationSampling::kSample});

    mod.root_block->Append(front_facing);
    mod.root_block->Append(position);
    mod.root_block->Append(color1);
    mod.root_block->Append(color2);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(ep->Block(), [&] {
        auto* ifelse = b.If(b.Load(front_facing));
        b.Append(ifelse->True(), [&] {
            auto* position_value = b.Load(position);
            auto* color1_value = b.Load(color1);
            auto* color2_value = b.Load(color2);
            b.Multiply(ty.vec4<f32>(), position_value, b.Add(ty.f32(), color1_value, color2_value));
            b.ExitIf(ifelse);
        });
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %front_facing:ptr<__in, bool, read> = var undef @builtin(front_facing)
  %position:ptr<__in, vec4<f32>, read> = var undef @invariant @builtin(position)
  %color1:ptr<__in, f32, read> = var undef @location(0)
  %color2:ptr<__in, f32, read> = var undef @location(1) @interpolate(linear, sample)
}

%foo = @fragment func():void {
  $B2: {
    %6:bool = load %front_facing
    if %6 [t: $B3] {  # if_1
      $B3: {  # true
        %7:vec4<f32> = load %position
        %8:f32 = load %color1
        %9:f32 = load %color2
        %10:f32 = add %8, %9
        %11:vec4<f32> = mul %7, %10
        exit_if  # if_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func(%front_facing:bool [@front_facing], %position:vec4<f32> [@invariant, @position], %color1:f32 [@location(0)], %color2:f32 [@location(1), @interpolate(linear, sample)]):void {
  $B1: {
    if %front_facing [t: $B2] {  # if_1
      $B2: {  # true
        %6:f32 = add %color1, %color2
        %7:vec4<f32> = mul %position, %6
        exit_if  # if_1
      }
    }
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, Inputs_UsedByHelper) {
    auto* front_facing = b.Var("front_facing", ty.ptr(core::AddressSpace::kIn, ty.bool_()));
    front_facing->SetBuiltin(core::BuiltinValue::kFrontFacing);

    auto* position = b.Var("position", ty.ptr(core::AddressSpace::kIn, ty.vec4<f32>()));
    position->SetBuiltin(core::BuiltinValue::kPosition);
    position->SetInvariant(true);

    auto* color1 = b.Var("color1", ty.ptr(core::AddressSpace::kIn, ty.f32()));
    color1->SetLocation(0);

    auto* color2 = b.Var("color2", ty.ptr(core::AddressSpace::kIn, ty.f32()));
    color2->SetLocation(1);
    color2->SetInterpolation(core::Interpolation{core::InterpolationType::kLinear,
                                                 core::InterpolationSampling::kSample});

    mod.root_block->Append(front_facing);
    mod.root_block->Append(position);
    mod.root_block->Append(color1);
    mod.root_block->Append(color2);

    // Inner function has an existing parameter.
    auto* param = b.FunctionParam("existing_param", ty.f32());
    auto* foo = b.Function("foo", ty.void_());
    foo->SetParams({param});
    b.Append(foo->Block(), [&] {
        auto* ifelse = b.If(b.Load(front_facing));
        b.Append(ifelse->True(), [&] {
            auto* position_value = b.Load(position);
            auto* color1_value = b.Load(color1);
            auto* color2_value = b.Load(color2);
            auto* add = b.Add(ty.f32(), color1_value, color2_value);
            auto* mul = b.Multiply(ty.vec4<f32>(), position_value, add);
            b.Divide(ty.vec4<f32>(), mul, param);
            b.ExitIf(ifelse);
        });
        b.Return(foo);
    });

    // Intermediate function has no existing parameters.
    auto* bar = b.Function("bar", ty.void_());
    b.Append(bar->Block(), [&] {
        b.Call(foo, 42_f);
        b.Return(bar);
    });

    auto* ep = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(ep->Block(), [&] {
        b.Call(bar);
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %front_facing:ptr<__in, bool, read> = var undef @builtin(front_facing)
  %position:ptr<__in, vec4<f32>, read> = var undef @invariant @builtin(position)
  %color1:ptr<__in, f32, read> = var undef @location(0)
  %color2:ptr<__in, f32, read> = var undef @location(1) @interpolate(linear, sample)
}

%foo = func(%existing_param:f32):void {
  $B2: {
    %7:bool = load %front_facing
    if %7 [t: $B3] {  # if_1
      $B3: {  # true
        %8:vec4<f32> = load %position
        %9:f32 = load %color1
        %10:f32 = load %color2
        %11:f32 = add %9, %10
        %12:vec4<f32> = mul %8, %11
        %13:vec4<f32> = div %12, %existing_param
        exit_if  # if_1
      }
    }
    ret
  }
}
%bar = func():void {
  $B4: {
    %15:void = call %foo, 42.0f
    ret
  }
}
%main = @fragment func():void {
  $B5: {
    %17:void = call %bar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%existing_param:f32, %front_facing:bool, %position:vec4<f32>, %color1:f32, %color2:f32):void {
  $B1: {
    if %front_facing [t: $B2] {  # if_1
      $B2: {  # true
        %7:f32 = add %color1, %color2
        %8:vec4<f32> = mul %position, %7
        %9:vec4<f32> = div %8, %existing_param
        exit_if  # if_1
      }
    }
    ret
  }
}
%bar = func(%front_facing_1:bool, %position_1:vec4<f32>, %color1_1:f32, %color2_1:f32):void {  # %front_facing_1: 'front_facing', %position_1: 'position', %color1_1: 'color1', %color2_1: 'color2'
  $B3: {
    %15:void = call %foo, 42.0f, %front_facing_1, %position_1, %color1_1, %color2_1
    ret
  }
}
%main = @fragment func(%front_facing_2:bool [@front_facing], %position_2:vec4<f32> [@invariant, @position], %color1_2:f32 [@location(0)], %color2_2:f32 [@location(1), @interpolate(linear, sample)]):void {  # %front_facing_2: 'front_facing', %position_2: 'position', %color1_2: 'color1', %color2_2: 'color2'
  $B4: {
    %21:void = call %bar, %front_facing_2, %position_2, %color1_2, %color2_2
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, Inputs_Copied) {
    auto* val = b.Var("val", ty.ptr(core::AddressSpace::kIn, ty.u32()));
    val->SetBuiltin(core::BuiltinValue::kSampleIndex);
    mod.root_block->Append(val);

    auto* f = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(f->Block(), [&] {
        auto* c = b.Let("copy", val);
        b.Let("res", b.Load(c));
        b.Return(f);
    });

    auto* src = R"(
$B1: {  # root
  %val:ptr<__in, u32, read> = var undef @builtin(sample_index)
}

%main = @fragment func():void {
  $B2: {
    %copy:ptr<__in, u32, read> = let %val
    %4:u32 = load %copy
    %res:u32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%main = @fragment func(%val:u32 [@sample_index]):void {
  $B1: {
    %res:u32 = let %val
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, Inputs_Copied_Chain) {
    auto* val = b.Var("val", ty.ptr(core::AddressSpace::kIn, ty.u32()));
    val->SetBuiltin(core::BuiltinValue::kSampleIndex);
    mod.root_block->Append(val);

    auto* f = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(f->Block(), [&] {
        auto* c1 = b.Let("c1", val);
        auto* c2 = b.Let("c2", c1);
        auto* c3 = b.Let("c3", c2);
        auto* c4 = b.Let("c4", c3);
        auto* c5 = b.Let("c5", c4);
        auto* c6 = b.Let("c6", c5);
        b.Let("res", b.Load(c6));
        b.Return(f);
    });

    auto* src = R"(
$B1: {  # root
  %val:ptr<__in, u32, read> = var undef @builtin(sample_index)
}

%main = @fragment func():void {
  $B2: {
    %c1:ptr<__in, u32, read> = let %val
    %c2:ptr<__in, u32, read> = let %c1
    %c3:ptr<__in, u32, read> = let %c2
    %c4:ptr<__in, u32, read> = let %c3
    %c5:ptr<__in, u32, read> = let %c4
    %c6:ptr<__in, u32, read> = let %c5
    %9:u32 = load %c6
    %res:u32 = let %9
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%main = @fragment func(%val:u32 [@sample_index]):void {
  $B1: {
    %res:u32 = let %val
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, Inputs_UsedEntryPointAndHelper) {
    auto* gid = b.Var("gid", ty.ptr(core::AddressSpace::kIn, ty.vec3<u32>()));
    gid->SetBuiltin(core::BuiltinValue::kGlobalInvocationId);

    auto* lid = b.Var("lid", ty.ptr(core::AddressSpace::kIn, ty.vec3<u32>()));
    lid->SetBuiltin(core::BuiltinValue::kLocalInvocationId);

    auto* group_id = b.Var("group_id", ty.ptr(core::AddressSpace::kIn, ty.vec3<u32>()));
    group_id->SetBuiltin(core::BuiltinValue::kWorkgroupId);

    mod.root_block->Append(gid);
    mod.root_block->Append(lid);
    mod.root_block->Append(group_id);

    // Use a subset of the inputs in the helper.
    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {
        auto* gid_value = b.Load(gid);
        auto* lid_value = b.Load(lid);
        b.Add(ty.vec3<u32>(), gid_value, lid_value);
        b.Return(foo);
    });

    // Use a different subset of the inputs in the entry point.
    auto* ep = b.ComputeFunction("main1");
    b.Append(ep->Block(), [&] {
        auto* group_value = b.Load(group_id);
        auto* gid_value = b.Load(gid);
        b.Add(ty.vec3<u32>(), group_value, gid_value);
        b.Call(foo);
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %gid:ptr<__in, vec3<u32>, read> = var undef @builtin(global_invocation_id)
  %lid:ptr<__in, vec3<u32>, read> = var undef @builtin(local_invocation_id)
  %group_id:ptr<__in, vec3<u32>, read> = var undef @builtin(workgroup_id)
}

%foo = func():void {
  $B2: {
    %5:vec3<u32> = load %gid
    %6:vec3<u32> = load %lid
    %7:vec3<u32> = add %5, %6
    ret
  }
}
%main1 = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %9:vec3<u32> = load %group_id
    %10:vec3<u32> = load %gid
    %11:vec3<u32> = add %9, %10
    %12:void = call %foo
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%gid:vec3<u32>, %lid:vec3<u32>):void {
  $B1: {
    %4:vec3<u32> = add %gid, %lid
    ret
  }
}
%main1 = @compute @workgroup_size(1u, 1u, 1u) func(%gid_1:vec3<u32> [@global_invocation_id], %lid_1:vec3<u32> [@local_invocation_id], %group_id:vec3<u32> [@workgroup_id]):void {  # %gid_1: 'gid', %lid_1: 'lid'
  $B2: {
    %9:vec3<u32> = add %group_id, %gid_1
    %10:void = call %foo, %gid_1, %lid_1
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, Inputs_UsedEntryPointAndHelper_ForwardReference) {
    auto* gid = b.Var("gid", ty.ptr(core::AddressSpace::kIn, ty.vec3<u32>()));
    gid->SetBuiltin(core::BuiltinValue::kGlobalInvocationId);

    auto* lid = b.Var("lid", ty.ptr(core::AddressSpace::kIn, ty.vec3<u32>()));
    lid->SetBuiltin(core::BuiltinValue::kLocalInvocationId);

    auto* group_id = b.Var("group_id", ty.ptr(core::AddressSpace::kIn, ty.vec3<u32>()));
    group_id->SetBuiltin(core::BuiltinValue::kWorkgroupId);

    mod.root_block->Append(gid);
    mod.root_block->Append(lid);
    mod.root_block->Append(group_id);

    auto* ep = b.ComputeFunction("main1");
    auto* foo = b.Function("foo", ty.void_());

    // Use a subset of the inputs in the entry point.
    b.Append(ep->Block(), [&] {
        auto* group_value = b.Load(group_id);
        auto* gid_value = b.Load(gid);
        b.Add(ty.vec3<u32>(), group_value, gid_value);
        b.Call(foo);
        b.Return(ep);
    });

    // Use a different subset of the variables in the helper.
    b.Append(foo->Block(), [&] {
        auto* gid_value = b.Load(gid);
        auto* lid_value = b.Load(lid);
        b.Add(ty.vec3<u32>(), gid_value, lid_value);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %gid:ptr<__in, vec3<u32>, read> = var undef @builtin(global_invocation_id)
  %lid:ptr<__in, vec3<u32>, read> = var undef @builtin(local_invocation_id)
  %group_id:ptr<__in, vec3<u32>, read> = var undef @builtin(workgroup_id)
}

%main1 = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:vec3<u32> = load %group_id
    %6:vec3<u32> = load %gid
    %7:vec3<u32> = add %5, %6
    %8:void = call %foo
    ret
  }
}
%foo = func():void {
  $B3: {
    %10:vec3<u32> = load %gid
    %11:vec3<u32> = load %lid
    %12:vec3<u32> = add %10, %11
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%main1 = @compute @workgroup_size(1u, 1u, 1u) func(%gid:vec3<u32> [@global_invocation_id], %lid:vec3<u32> [@local_invocation_id], %group_id:vec3<u32> [@workgroup_id]):void {
  $B1: {
    %5:vec3<u32> = add %group_id, %gid
    %6:void = call %foo, %gid, %lid
    ret
  }
}
%foo = func(%gid_1:vec3<u32>, %lid_1:vec3<u32>):void {  # %gid_1: 'gid', %lid_1: 'lid'
  $B2: {
    %10:vec3<u32> = add %gid_1, %lid_1
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, Inputs_UsedByMultipleEntryPoints) {
    auto* gid = b.Var("gid", ty.ptr(core::AddressSpace::kIn, ty.vec3<u32>()));
    gid->SetBuiltin(core::BuiltinValue::kGlobalInvocationId);

    auto* lid = b.Var("lid", ty.ptr(core::AddressSpace::kIn, ty.vec3<u32>()));
    lid->SetBuiltin(core::BuiltinValue::kLocalInvocationId);

    auto* group_id = b.Var("group_id", ty.ptr(core::AddressSpace::kIn, ty.vec3<u32>()));
    group_id->SetBuiltin(core::BuiltinValue::kWorkgroupId);

    mod.root_block->Append(gid);
    mod.root_block->Append(lid);
    mod.root_block->Append(group_id);

    // Use a subset of the inputs in the helper.
    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {
        auto* gid_value = b.Load(gid);
        auto* lid_value = b.Load(lid);
        b.Add(ty.vec3<u32>(), gid_value, lid_value);
        b.Return(foo);
    });

    // Call the helper without directly referencing any inputs.
    auto* ep1 = b.ComputeFunction("main1");
    b.Append(ep1->Block(), [&] {
        b.Call(foo);
        b.Return(ep1);
    });

    // Reference another input and then call the helper.
    auto* ep2 = b.ComputeFunction("main2");
    b.Append(ep2->Block(), [&] {
        auto* group_value = b.Load(group_id);
        b.Add(ty.vec3<u32>(), group_value, group_value);
        b.Call(foo);
        b.Return(ep1);
    });

    auto* src = R"(
$B1: {  # root
  %gid:ptr<__in, vec3<u32>, read> = var undef @builtin(global_invocation_id)
  %lid:ptr<__in, vec3<u32>, read> = var undef @builtin(local_invocation_id)
  %group_id:ptr<__in, vec3<u32>, read> = var undef @builtin(workgroup_id)
}

%foo = func():void {
  $B2: {
    %5:vec3<u32> = load %gid
    %6:vec3<u32> = load %lid
    %7:vec3<u32> = add %5, %6
    ret
  }
}
%main1 = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %9:void = call %foo
    ret
  }
}
%main2 = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B4: {
    %11:vec3<u32> = load %group_id
    %12:vec3<u32> = add %11, %11
    %13:void = call %foo
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%gid:vec3<u32>, %lid:vec3<u32>):void {
  $B1: {
    %4:vec3<u32> = add %gid, %lid
    ret
  }
}
%main1 = @compute @workgroup_size(1u, 1u, 1u) func(%gid_1:vec3<u32> [@global_invocation_id], %lid_1:vec3<u32> [@local_invocation_id]):void {  # %gid_1: 'gid', %lid_1: 'lid'
  $B2: {
    %8:void = call %foo, %gid_1, %lid_1
    ret
  }
}
%main2 = @compute @workgroup_size(1u, 1u, 1u) func(%gid_2:vec3<u32> [@global_invocation_id], %lid_2:vec3<u32> [@local_invocation_id], %group_id:vec3<u32> [@workgroup_id]):void {  # %gid_2: 'gid', %lid_2: 'lid'
  $B3: {
    %13:vec3<u32> = add %group_id, %group_id
    %14:void = call %foo, %gid_2, %lid_2
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, Input_LoadVectorElement) {
    auto* lid = b.Var("lid", ty.ptr(core::AddressSpace::kIn, ty.vec3<u32>()));
    lid->SetBuiltin(core::BuiltinValue::kLocalInvocationId);

    mod.root_block->Append(lid);

    auto* ep = b.ComputeFunction("foo");
    b.Append(ep->Block(), [&] {
        b.LoadVectorElement(lid, 2_u);
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %lid:ptr<__in, vec3<u32>, read> = var undef @builtin(local_invocation_id)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = load_vector_element %lid, 2u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%lid:vec3<u32> [@local_invocation_id]):void {
  $B1: {
    %3:u32 = access %lid, 2u
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, Inputs_Struct_LocationOnEachMember) {
    auto* colors_str = ty.Struct(
        mod.symbols.New("Colors"),
        Vector{
            core::type::Manager::StructMemberDesc{
                mod.symbols.New("color1"),
                ty.vec4<f32>(),
                LocationAttrs(1),
            },
            core::type::Manager::StructMemberDesc{
                mod.symbols.New("color2"),
                ty.vec4<f32>(),
                LocationAttrs(2u, core::Interpolation{core::InterpolationType::kLinear,
                                                      core::InterpolationSampling::kCentroid}),
            },
        });
    auto* colors = b.Var("colors", ty.ptr(core::AddressSpace::kIn, colors_str));
    mod.root_block->Append(colors);

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {
        auto* ptr = ty.ptr(core::AddressSpace::kIn, ty.vec4<f32>());
        auto* color1_value = b.Load(b.Access(ptr, colors, 0_u));
        auto* color2_z_value = b.LoadVectorElement(b.Access(ptr, colors, 1_u), 2_u);
        b.Multiply(ty.vec4<f32>(), color1_value, color2_z_value);
        b.Return(foo);
    });

    auto* ep = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(ep->Block(), [&] {
        b.Call(foo);
        b.Return(ep);
    });

    auto* src = R"(
Colors = struct @align(16) {
  color1:vec4<f32> @offset(0), @location(1)
  color2:vec4<f32> @offset(16), @location(2), @interpolate(linear, centroid)
}

$B1: {  # root
  %colors:ptr<__in, Colors, read> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<__in, vec4<f32>, read> = access %colors, 0u
    %4:vec4<f32> = load %3
    %5:ptr<__in, vec4<f32>, read> = access %colors, 1u
    %6:f32 = load_vector_element %5, 2u
    %7:vec4<f32> = mul %4, %6
    ret
  }
}
%main = @fragment func():void {
  $B3: {
    %9:void = call %foo
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Colors = struct @align(16) {
  color1:vec4<f32> @offset(0)
  color2:vec4<f32> @offset(16)
}

%foo = func(%colors:Colors):void {
  $B1: {
    %3:vec4<f32> = access %colors, 0u
    %4:vec4<f32> = access %colors, 1u
    %5:f32 = access %4, 2u
    %6:vec4<f32> = mul %3, %5
    ret
  }
}
%main = @fragment func(%8:vec4<f32> [@location(1)], %9:vec4<f32> [@location(2), @interpolate(linear, centroid)]):void {
  $B2: {
    %colors_1:Colors = construct %8, %9  # %colors_1: 'colors'
    %11:void = call %foo, %colors_1
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, Inputs_Struct_LocationOnVariable) {
    auto* colors_str = ty.Struct(
        mod.symbols.New("Colors"),
        Vector{
            core::type::Manager::StructMemberDesc{
                mod.symbols.New("color1"),
                ty.vec4<f32>(),
            },
            core::type::Manager::StructMemberDesc{
                mod.symbols.New("color2"),
                ty.vec4<f32>(),
                core::IOAttributes{
                    .interpolation = core::Interpolation{core::InterpolationType::kPerspective,
                                                         core::InterpolationSampling::kCentroid},
                },
            },
        });
    auto* colors = b.Var("colors", ty.ptr(core::AddressSpace::kIn, colors_str));
    colors->SetLocation(1u);

    mod.root_block->Append(colors);

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {
        auto* ptr = ty.ptr(core::AddressSpace::kIn, ty.vec4<f32>());
        auto* color1_value = b.Load(b.Access(ptr, colors, 0_u));
        auto* color2_z_value = b.LoadVectorElement(b.Access(ptr, colors, 1_u), 2_u);
        b.Multiply(ty.vec4<f32>(), color1_value, color2_z_value);
        b.Return(foo);
    });

    auto* ep = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(ep->Block(), [&] {
        b.Call(foo);
        b.Return(ep);
    });

    auto* src = R"(
Colors = struct @align(16) {
  color1:vec4<f32> @offset(0)
  color2:vec4<f32> @offset(16), @interpolate(perspective, centroid)
}

$B1: {  # root
  %colors:ptr<__in, Colors, read> = var undef @location(1)
}

%foo = func():void {
  $B2: {
    %3:ptr<__in, vec4<f32>, read> = access %colors, 0u
    %4:vec4<f32> = load %3
    %5:ptr<__in, vec4<f32>, read> = access %colors, 1u
    %6:f32 = load_vector_element %5, 2u
    %7:vec4<f32> = mul %4, %6
    ret
  }
}
%main = @fragment func():void {
  $B3: {
    %9:void = call %foo
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Colors = struct @align(16) {
  color1:vec4<f32> @offset(0)
  color2:vec4<f32> @offset(16)
}

%foo = func(%colors:Colors):void {
  $B1: {
    %3:vec4<f32> = access %colors, 0u
    %4:vec4<f32> = access %colors, 1u
    %5:f32 = access %4, 2u
    %6:vec4<f32> = mul %3, %5
    ret
  }
}
%main = @fragment func(%8:vec4<f32> [@location(1)], %9:vec4<f32> [@location(2), @interpolate(perspective, centroid)]):void {
  $B2: {
    %colors_1:Colors = construct %8, %9  # %colors_1: 'colors'
    %11:void = call %foo, %colors_1
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, Inputs_Struct_InterpolateOnVariable) {
    auto* colors_str = ty.Struct(
        mod.symbols.New("Colors"),
        Vector{
            core::type::Manager::StructMemberDesc{
                mod.symbols.New("color1"),
                ty.vec4<f32>(),
                LocationAttrs(1),
            },
            core::type::Manager::StructMemberDesc{
                mod.symbols.New("color2"),
                ty.vec4<f32>(),
                LocationAttrs(2u, core::Interpolation{core::InterpolationType::kLinear,
                                                      core::InterpolationSampling::kSample}),
            },
        });
    auto* colors = b.Var("colors", ty.ptr(core::AddressSpace::kIn, colors_str));
    colors->SetInterpolation(core::Interpolation{core::InterpolationType::kPerspective,
                                                 core::InterpolationSampling::kCentroid});

    mod.root_block->Append(colors);

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {
        auto* ptr = ty.ptr(core::AddressSpace::kIn, ty.vec4<f32>());
        auto* color1_value = b.Load(b.Access(ptr, colors, 0_u));
        auto* color2_z_value = b.LoadVectorElement(b.Access(ptr, colors, 1_u), 2_u);
        b.Multiply(ty.vec4<f32>(), color1_value, color2_z_value);
        b.Return(foo);
    });

    auto* ep = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(ep->Block(), [&] {
        b.Call(foo);
        b.Return(ep);
    });

    auto* src = R"(
Colors = struct @align(16) {
  color1:vec4<f32> @offset(0), @location(1)
  color2:vec4<f32> @offset(16), @location(2), @interpolate(linear, sample)
}

$B1: {  # root
  %colors:ptr<__in, Colors, read> = var undef @interpolate(perspective, centroid)
}

%foo = func():void {
  $B2: {
    %3:ptr<__in, vec4<f32>, read> = access %colors, 0u
    %4:vec4<f32> = load %3
    %5:ptr<__in, vec4<f32>, read> = access %colors, 1u
    %6:f32 = load_vector_element %5, 2u
    %7:vec4<f32> = mul %4, %6
    ret
  }
}
%main = @fragment func():void {
  $B3: {
    %9:void = call %foo
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Colors = struct @align(16) {
  color1:vec4<f32> @offset(0)
  color2:vec4<f32> @offset(16)
}

%foo = func(%colors:Colors):void {
  $B1: {
    %3:vec4<f32> = access %colors, 0u
    %4:vec4<f32> = access %colors, 1u
    %5:f32 = access %4, 2u
    %6:vec4<f32> = mul %3, %5
    ret
  }
}
%main = @fragment func(%8:vec4<f32> [@location(1), @interpolate(perspective, centroid)], %9:vec4<f32> [@location(2), @interpolate(linear, sample)]):void {
  $B2: {
    %colors_1:Colors = construct %8, %9  # %colors_1: 'colors'
    %11:void = call %foo, %colors_1
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, Inputs_Struct_LoadWholeStruct) {
    auto* colors_str = ty.Struct(
        mod.symbols.New("Colors"),
        Vector{
            core::type::Manager::StructMemberDesc{
                mod.symbols.New("color1"),
                ty.vec4<f32>(),
                LocationAttrs(1),
            },
            core::type::Manager::StructMemberDesc{
                mod.symbols.New("color2"),
                ty.vec4<f32>(),
                LocationAttrs(2u, core::Interpolation{core::InterpolationType::kLinear,
                                                      core::InterpolationSampling::kCentroid}),
            },
        });
    auto* colors = b.Var("colors", ty.ptr(core::AddressSpace::kIn, colors_str));
    mod.root_block->Append(colors);

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {
        auto* load = b.Load(colors);
        auto* color1_value = b.Access<vec4<f32>>(load, 0_u);
        auto* color2_z_value = b.Access<f32>(load, 1_u, 2_u);
        b.Multiply(ty.vec4<f32>(), color1_value, color2_z_value);
        b.Return(foo);
    });

    auto* ep = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(ep->Block(), [&] {
        b.Call(foo);
        b.Return(ep);
    });

    auto* src = R"(
Colors = struct @align(16) {
  color1:vec4<f32> @offset(0), @location(1)
  color2:vec4<f32> @offset(16), @location(2), @interpolate(linear, centroid)
}

$B1: {  # root
  %colors:ptr<__in, Colors, read> = var undef
}

%foo = func():void {
  $B2: {
    %3:Colors = load %colors
    %4:vec4<f32> = access %3, 0u
    %5:f32 = access %3, 1u, 2u
    %6:vec4<f32> = mul %4, %5
    ret
  }
}
%main = @fragment func():void {
  $B3: {
    %8:void = call %foo
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Colors = struct @align(16) {
  color1:vec4<f32> @offset(0)
  color2:vec4<f32> @offset(16)
}

%foo = func(%colors:Colors):void {
  $B1: {
    %3:vec4<f32> = access %colors, 0u
    %4:f32 = access %colors, 1u, 2u
    %5:vec4<f32> = mul %3, %4
    ret
  }
}
%main = @fragment func(%7:vec4<f32> [@location(1)], %8:vec4<f32> [@location(2), @interpolate(linear, centroid)]):void {
  $B2: {
    %colors_1:Colors = construct %7, %8  # %colors_1: 'colors'
    %10:void = call %foo, %colors_1
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, SingleOutput_Builtin) {
    auto* position = b.Var("position", ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>()));
    position->SetBuiltin(core::BuiltinValue::kPosition);

    mod.root_block->Append(position);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {  //
        b.Store(position, b.Splat<vec4<f32>>(1_f));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %position:ptr<__out, vec4<f32>, read_write> = var undef @builtin(position)
}

%foo = @vertex func():void {
  $B2: {
    store %position, vec4<f32>(1.0f)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %position:ptr<private, vec4<f32>, read_write> = var undef
}

%foo_inner = func():void {
  $B2: {
    store %position, vec4<f32>(1.0f)
    ret
  }
}
%foo = @vertex func():vec4<f32> [@position] {
  $B3: {
    %4:void = call %foo_inner
    %5:vec4<f32> = load %position
    ret %5
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, SingleOutput_Builtin_WithInvariant) {
    auto* position = b.Var("position", ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>()));
    position->SetBuiltin(core::BuiltinValue::kPosition);
    position->SetInvariant(true);

    mod.root_block->Append(position);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {  //
        b.Store(position, b.Splat<vec4<f32>>(1_f));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %position:ptr<__out, vec4<f32>, read_write> = var undef @invariant @builtin(position)
}

%foo = @vertex func():void {
  $B2: {
    store %position, vec4<f32>(1.0f)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %position:ptr<private, vec4<f32>, read_write> = var undef
}

%foo_inner = func():void {
  $B2: {
    store %position, vec4<f32>(1.0f)
    ret
  }
}
%foo = @vertex func():vec4<f32> [@invariant, @position] {
  $B3: {
    %4:void = call %foo_inner
    %5:vec4<f32> = load %position
    ret %5
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, SingleOutput_Location) {
    auto* color = b.Var("color", ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>()));
    color->SetLocation(1u);

    mod.root_block->Append(color);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(ep->Block(), [&] {  //
        b.Store(color, b.Splat<vec4<f32>>(1_f));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %color:ptr<__out, vec4<f32>, read_write> = var undef @location(1)
}

%foo = @fragment func():void {
  $B2: {
    store %color, vec4<f32>(1.0f)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %color:ptr<private, vec4<f32>, read_write> = var undef
}

%foo_inner = func():void {
  $B2: {
    store %color, vec4<f32>(1.0f)
    ret
  }
}
%foo = @fragment func():vec4<f32> [@location(1)] {
  $B3: {
    %4:void = call %foo_inner
    %5:vec4<f32> = load %color
    ret %5
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, SingleOutput_Location_WithInterpolation) {
    auto* color = b.Var("color", ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>()));
    color->SetLocation(1u);
    color->SetInterpolation(core::Interpolation{core::InterpolationType::kPerspective,
                                                core::InterpolationSampling::kCentroid});

    mod.root_block->Append(color);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(ep->Block(), [&] {  //
        b.Store(color, b.Splat<vec4<f32>>(1_f));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %color:ptr<__out, vec4<f32>, read_write> = var undef @location(1) @interpolate(perspective, centroid)
}

%foo = @fragment func():void {
  $B2: {
    store %color, vec4<f32>(1.0f)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %color:ptr<private, vec4<f32>, read_write> = var undef
}

%foo_inner = func():void {
  $B2: {
    store %color, vec4<f32>(1.0f)
    ret
  }
}
%foo = @fragment func():vec4<f32> [@location(1), @interpolate(perspective, centroid)] {
  $B3: {
    %4:void = call %foo_inner
    %5:vec4<f32> = load %color
    ret %5
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, MultipleOutputs) {
    auto* position = b.Var("position", ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>()));
    position->SetBuiltin(core::BuiltinValue::kPosition);
    position->SetInvariant(true);

    auto* color1 = b.Var("color1", ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>()));
    color1->SetLocation(1u);

    auto* color2 = b.Var("color2", ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>()));
    color2->SetLocation(1u);
    color2->SetInterpolation(core::Interpolation{core::InterpolationType::kPerspective,
                                                 core::InterpolationSampling::kCentroid});

    mod.root_block->Append(position);
    mod.root_block->Append(color1);
    mod.root_block->Append(color2);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {  //
        b.Store(position, b.Splat<vec4<f32>>(1_f));
        b.Store(color1, b.Splat<vec4<f32>>(0.5_f));
        b.Store(color2, b.Splat<vec4<f32>>(0.25_f));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %position:ptr<__out, vec4<f32>, read_write> = var undef @invariant @builtin(position)
  %color1:ptr<__out, vec4<f32>, read_write> = var undef @location(1)
  %color2:ptr<__out, vec4<f32>, read_write> = var undef @location(1) @interpolate(perspective, centroid)
}

%foo = @vertex func():void {
  $B2: {
    store %position, vec4<f32>(1.0f)
    store %color1, vec4<f32>(0.5f)
    store %color2, vec4<f32>(0.25f)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_symbol = struct @align(16) {
  position:vec4<f32> @offset(0), @invariant, @builtin(position)
  color1:vec4<f32> @offset(16), @location(1)
  color2:vec4<f32> @offset(32), @location(1), @interpolate(perspective, centroid)
}

$B1: {  # root
  %position:ptr<private, vec4<f32>, read_write> = var undef
  %color1:ptr<private, vec4<f32>, read_write> = var undef
  %color2:ptr<private, vec4<f32>, read_write> = var undef
}

%foo_inner = func():void {
  $B2: {
    store %position, vec4<f32>(1.0f)
    store %color1, vec4<f32>(0.5f)
    store %color2, vec4<f32>(0.25f)
    ret
  }
}
%foo = @vertex func():tint_symbol {
  $B3: {
    %6:void = call %foo_inner
    %7:vec4<f32> = load %position
    %8:vec4<f32> = load %color1
    %9:vec4<f32> = load %color2
    %10:tint_symbol = construct %7, %8, %9
    ret %10
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, Outputs_Struct_LocationOnEachMember) {
    auto* builtin_str =
        ty.Struct(mod.symbols.New("Builtins"), Vector{
                                                   core::type::Manager::StructMemberDesc{
                                                       mod.symbols.New("position"),
                                                       ty.vec4<f32>(),
                                                       BuiltinAttrs(core::BuiltinValue::kPosition),
                                                   },
                                               });
    auto* colors_str = ty.Struct(
        mod.symbols.New("Colors"),
        Vector{
            core::type::Manager::StructMemberDesc{
                mod.symbols.New("color1"),
                ty.vec4<f32>(),
                LocationAttrs(1),
            },
            core::type::Manager::StructMemberDesc{
                mod.symbols.New("color2"),
                ty.vec4<f32>(),
                LocationAttrs(2u, core::Interpolation{core::InterpolationType::kPerspective,
                                                      core::InterpolationSampling::kCentroid}),
            },
        });

    auto* builtins = b.Var("builtins", ty.ptr(core::AddressSpace::kOut, builtin_str));
    auto* colors = b.Var("colors", ty.ptr(core::AddressSpace::kOut, colors_str));
    mod.root_block->Append(builtins);
    mod.root_block->Append(colors);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {  //
        auto* ptr = ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>());
        b.Store(b.Access(ptr, builtins, 0_u), b.Splat<vec4<f32>>(1_f));
        b.Store(b.Access(ptr, colors, 0_u), b.Splat<vec4<f32>>(0.5_f));
        b.Store(b.Access(ptr, colors, 1_u), b.Splat<vec4<f32>>(0.25_f));
        b.Return(ep);
    });

    auto* src = R"(
Builtins = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
}

Colors = struct @align(16) {
  color1:vec4<f32> @offset(0), @location(1)
  color2:vec4<f32> @offset(16), @location(2), @interpolate(perspective, centroid)
}

$B1: {  # root
  %builtins:ptr<__out, Builtins, read_write> = var undef
  %colors:ptr<__out, Colors, read_write> = var undef
}

%foo = @vertex func():void {
  $B2: {
    %4:ptr<__out, vec4<f32>, read_write> = access %builtins, 0u
    store %4, vec4<f32>(1.0f)
    %5:ptr<__out, vec4<f32>, read_write> = access %colors, 0u
    store %5, vec4<f32>(0.5f)
    %6:ptr<__out, vec4<f32>, read_write> = access %colors, 1u
    store %6, vec4<f32>(0.25f)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Builtins = struct @align(16) {
  position:vec4<f32> @offset(0)
}

Colors = struct @align(16) {
  color1:vec4<f32> @offset(0)
  color2:vec4<f32> @offset(16)
}

tint_symbol = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
  color1:vec4<f32> @offset(16), @location(1)
  color2:vec4<f32> @offset(32), @location(2), @interpolate(perspective, centroid)
}

$B1: {  # root
  %builtins:ptr<private, Builtins, read_write> = var undef
  %colors:ptr<private, Colors, read_write> = var undef
}

%foo_inner = func():void {
  $B2: {
    %4:ptr<private, vec4<f32>, read_write> = access %builtins, 0u
    store %4, vec4<f32>(1.0f)
    %5:ptr<private, vec4<f32>, read_write> = access %colors, 0u
    store %5, vec4<f32>(0.5f)
    %6:ptr<private, vec4<f32>, read_write> = access %colors, 1u
    store %6, vec4<f32>(0.25f)
    ret
  }
}
%foo = @vertex func():tint_symbol {
  $B3: {
    %8:void = call %foo_inner
    %9:ptr<private, vec4<f32>, read_write> = access %builtins, 0u
    %10:vec4<f32> = load %9
    %11:ptr<private, vec4<f32>, read_write> = access %colors, 0u
    %12:vec4<f32> = load %11
    %13:ptr<private, vec4<f32>, read_write> = access %colors, 1u
    %14:vec4<f32> = load %13
    %15:tint_symbol = construct %10, %12, %14
    ret %15
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, Outputs_Struct_LocationOnVariable) {
    auto* builtin_str =
        ty.Struct(mod.symbols.New("Builtins"), Vector{
                                                   core::type::Manager::StructMemberDesc{
                                                       mod.symbols.New("position"),
                                                       ty.vec4<f32>(),
                                                       BuiltinAttrs(core::BuiltinValue::kPosition),
                                                   },
                                               });
    auto* colors_str = ty.Struct(
        mod.symbols.New("Colors"),
        Vector{
            core::type::Manager::StructMemberDesc{
                mod.symbols.New("color1"),
                ty.vec4<f32>(),
            },
            core::type::Manager::StructMemberDesc{
                mod.symbols.New("color2"),
                ty.vec4<f32>(),
                core::IOAttributes{
                    .interpolation = core::Interpolation{core::InterpolationType::kPerspective,
                                                         core::InterpolationSampling::kCentroid},
                },
            },
        });

    auto* builtins = b.Var("builtins", ty.ptr(core::AddressSpace::kOut, builtin_str));
    auto* colors = b.Var("colors", ty.ptr(core::AddressSpace::kOut, colors_str));
    colors->SetLocation(1u);

    mod.root_block->Append(builtins);
    mod.root_block->Append(colors);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {  //
        auto* ptr = ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>());
        b.Store(b.Access(ptr, builtins, 0_u), b.Splat<vec4<f32>>(1_f));
        b.Store(b.Access(ptr, colors, 0_u), b.Splat<vec4<f32>>(0.5_f));
        b.Store(b.Access(ptr, colors, 1_u), b.Splat<vec4<f32>>(0.25_f));
        b.Return(ep);
    });

    auto* src = R"(
Builtins = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
}

Colors = struct @align(16) {
  color1:vec4<f32> @offset(0)
  color2:vec4<f32> @offset(16), @interpolate(perspective, centroid)
}

$B1: {  # root
  %builtins:ptr<__out, Builtins, read_write> = var undef
  %colors:ptr<__out, Colors, read_write> = var undef @location(1)
}

%foo = @vertex func():void {
  $B2: {
    %4:ptr<__out, vec4<f32>, read_write> = access %builtins, 0u
    store %4, vec4<f32>(1.0f)
    %5:ptr<__out, vec4<f32>, read_write> = access %colors, 0u
    store %5, vec4<f32>(0.5f)
    %6:ptr<__out, vec4<f32>, read_write> = access %colors, 1u
    store %6, vec4<f32>(0.25f)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Builtins = struct @align(16) {
  position:vec4<f32> @offset(0)
}

Colors = struct @align(16) {
  color1:vec4<f32> @offset(0)
  color2:vec4<f32> @offset(16)
}

tint_symbol = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
  color1:vec4<f32> @offset(16), @location(1)
  color2:vec4<f32> @offset(32), @location(2), @interpolate(perspective, centroid)
}

$B1: {  # root
  %builtins:ptr<private, Builtins, read_write> = var undef
  %colors:ptr<private, Colors, read_write> = var undef
}

%foo_inner = func():void {
  $B2: {
    %4:ptr<private, vec4<f32>, read_write> = access %builtins, 0u
    store %4, vec4<f32>(1.0f)
    %5:ptr<private, vec4<f32>, read_write> = access %colors, 0u
    store %5, vec4<f32>(0.5f)
    %6:ptr<private, vec4<f32>, read_write> = access %colors, 1u
    store %6, vec4<f32>(0.25f)
    ret
  }
}
%foo = @vertex func():tint_symbol {
  $B3: {
    %8:void = call %foo_inner
    %9:ptr<private, vec4<f32>, read_write> = access %builtins, 0u
    %10:vec4<f32> = load %9
    %11:ptr<private, vec4<f32>, read_write> = access %colors, 0u
    %12:vec4<f32> = load %11
    %13:ptr<private, vec4<f32>, read_write> = access %colors, 1u
    %14:vec4<f32> = load %13
    %15:tint_symbol = construct %10, %12, %14
    ret %15
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, Outputs_Struct_InterpolateOnVariable) {
    auto* builtin_str =
        ty.Struct(mod.symbols.New("Builtins"), Vector{
                                                   core::type::Manager::StructMemberDesc{
                                                       mod.symbols.New("position"),
                                                       ty.vec4<f32>(),
                                                       BuiltinAttrs(core::BuiltinValue::kPosition),
                                                   },
                                               });
    auto* colors_str =
        ty.Struct(mod.symbols.New("Colors"),
                  Vector{
                      core::type::Manager::StructMemberDesc{
                          mod.symbols.New("color1"),
                          ty.vec4<f32>(),
                          LocationAttrs(2),
                      },
                      core::type::Manager::StructMemberDesc{
                          mod.symbols.New("color2"),
                          ty.vec4<f32>(),
                          LocationAttrs(3, core::Interpolation{core::InterpolationType::kFlat}),
                      },
                  });

    auto* builtins = b.Var("builtins", ty.ptr(core::AddressSpace::kOut, builtin_str));
    auto* colors = b.Var("colors", ty.ptr(core::AddressSpace::kOut, colors_str));
    colors->SetInterpolation(core::Interpolation{core::InterpolationType::kPerspective,
                                                 core::InterpolationSampling::kCentroid});

    mod.root_block->Append(builtins);
    mod.root_block->Append(colors);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {  //
        auto* ptr = ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>());
        b.Store(b.Access(ptr, builtins, 0_u), b.Splat<vec4<f32>>(1_f));
        b.Store(b.Access(ptr, colors, 0_u), b.Splat<vec4<f32>>(0.5_f));
        b.Store(b.Access(ptr, colors, 1_u), b.Splat<vec4<f32>>(0.25_f));
        b.Return(ep);
    });

    auto* src = R"(
Builtins = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
}

Colors = struct @align(16) {
  color1:vec4<f32> @offset(0), @location(2)
  color2:vec4<f32> @offset(16), @location(3), @interpolate(flat)
}

$B1: {  # root
  %builtins:ptr<__out, Builtins, read_write> = var undef
  %colors:ptr<__out, Colors, read_write> = var undef @interpolate(perspective, centroid)
}

%foo = @vertex func():void {
  $B2: {
    %4:ptr<__out, vec4<f32>, read_write> = access %builtins, 0u
    store %4, vec4<f32>(1.0f)
    %5:ptr<__out, vec4<f32>, read_write> = access %colors, 0u
    store %5, vec4<f32>(0.5f)
    %6:ptr<__out, vec4<f32>, read_write> = access %colors, 1u
    store %6, vec4<f32>(0.25f)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Builtins = struct @align(16) {
  position:vec4<f32> @offset(0)
}

Colors = struct @align(16) {
  color1:vec4<f32> @offset(0)
  color2:vec4<f32> @offset(16)
}

tint_symbol = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
  color1:vec4<f32> @offset(16), @location(2), @interpolate(perspective, centroid)
  color2:vec4<f32> @offset(32), @location(3), @interpolate(flat)
}

$B1: {  # root
  %builtins:ptr<private, Builtins, read_write> = var undef
  %colors:ptr<private, Colors, read_write> = var undef
}

%foo_inner = func():void {
  $B2: {
    %4:ptr<private, vec4<f32>, read_write> = access %builtins, 0u
    store %4, vec4<f32>(1.0f)
    %5:ptr<private, vec4<f32>, read_write> = access %colors, 0u
    store %5, vec4<f32>(0.5f)
    %6:ptr<private, vec4<f32>, read_write> = access %colors, 1u
    store %6, vec4<f32>(0.25f)
    ret
  }
}
%foo = @vertex func():tint_symbol {
  $B3: {
    %8:void = call %foo_inner
    %9:ptr<private, vec4<f32>, read_write> = access %builtins, 0u
    %10:vec4<f32> = load %9
    %11:ptr<private, vec4<f32>, read_write> = access %colors, 0u
    %12:vec4<f32> = load %11
    %13:ptr<private, vec4<f32>, read_write> = access %colors, 1u
    %14:vec4<f32> = load %13
    %15:tint_symbol = construct %10, %12, %14
    ret %15
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, Outputs_UsedByMultipleEntryPoints) {
    auto* position = b.Var("position", ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>()));
    position->SetBuiltin(core::BuiltinValue::kPosition);
    position->SetInvariant(true);

    auto* color1 = b.Var("color1", ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>()));
    color1->SetLocation(1u);

    auto* color2 = b.Var("color2", ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>()));
    color2->SetLocation(1u);
    color2->SetInterpolation(core::Interpolation{core::InterpolationType::kPerspective,
                                                 core::InterpolationSampling::kCentroid});

    mod.root_block->Append(position);
    mod.root_block->Append(color1);
    mod.root_block->Append(color2);

    auto* ep1 = b.Function("main1", ty.void_(), core::ir::Function::PipelineStage::kVertex);
    b.Append(ep1->Block(), [&] {  //
        b.Store(position, b.Splat<vec4<f32>>(1_f));
        b.Return(ep1);
    });

    auto* ep2 = b.Function("main2", ty.void_(), core::ir::Function::PipelineStage::kVertex);
    b.Append(ep2->Block(), [&] {  //
        b.Store(position, b.Splat<vec4<f32>>(1_f));
        b.Store(color1, b.Splat<vec4<f32>>(0.5_f));
        b.Return(ep2);
    });

    auto* ep3 = b.Function("main3", ty.void_(), core::ir::Function::PipelineStage::kVertex);
    b.Append(ep3->Block(), [&] {  //
        b.Store(position, b.Splat<vec4<f32>>(1_f));
        b.Store(color2, b.Splat<vec4<f32>>(0.25_f));
        b.Return(ep3);
    });

    auto* src = R"(
$B1: {  # root
  %position:ptr<__out, vec4<f32>, read_write> = var undef @invariant @builtin(position)
  %color1:ptr<__out, vec4<f32>, read_write> = var undef @location(1)
  %color2:ptr<__out, vec4<f32>, read_write> = var undef @location(1) @interpolate(perspective, centroid)
}

%main1 = @vertex func():void {
  $B2: {
    store %position, vec4<f32>(1.0f)
    ret
  }
}
%main2 = @vertex func():void {
  $B3: {
    store %position, vec4<f32>(1.0f)
    store %color1, vec4<f32>(0.5f)
    ret
  }
}
%main3 = @vertex func():void {
  $B4: {
    store %position, vec4<f32>(1.0f)
    store %color2, vec4<f32>(0.25f)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_symbol = struct @align(16) {
  position:vec4<f32> @offset(0), @invariant, @builtin(position)
  color1:vec4<f32> @offset(16), @location(1)
}

tint_symbol_1 = struct @align(16) {
  position:vec4<f32> @offset(0), @invariant, @builtin(position)
  color2:vec4<f32> @offset(16), @location(1), @interpolate(perspective, centroid)
}

$B1: {  # root
  %position:ptr<private, vec4<f32>, read_write> = var undef
  %color1:ptr<private, vec4<f32>, read_write> = var undef
  %color2:ptr<private, vec4<f32>, read_write> = var undef
}

%main1_inner = func():void {
  $B2: {
    store %position, vec4<f32>(1.0f)
    ret
  }
}
%main2_inner = func():void {
  $B3: {
    store %position, vec4<f32>(1.0f)
    store %color1, vec4<f32>(0.5f)
    ret
  }
}
%main3_inner = func():void {
  $B4: {
    store %position, vec4<f32>(1.0f)
    store %color2, vec4<f32>(0.25f)
    ret
  }
}
%main1 = @vertex func():vec4<f32> [@invariant, @position] {
  $B5: {
    %8:void = call %main1_inner
    %9:vec4<f32> = load %position
    ret %9
  }
}
%main2 = @vertex func():tint_symbol {
  $B6: {
    %11:void = call %main2_inner
    %12:vec4<f32> = load %position
    %13:vec4<f32> = load %color1
    %14:tint_symbol = construct %12, %13
    ret %14
  }
}
%main3 = @vertex func():tint_symbol_1 {
  $B7: {
    %16:void = call %main3_inner
    %17:vec4<f32> = load %position
    %18:vec4<f32> = load %color2
    %19:tint_symbol_1 = construct %17, %18
    ret %19
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, Output_LoadAndStore) {
    auto* color = b.Var("color", ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>()));
    color->SetLocation(1u);

    mod.root_block->Append(color);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(ep->Block(), [&] {  //
        b.Store(color, b.Splat<vec4<f32>>(1_f));
        auto* load = b.Load(color);
        auto* mul = b.Multiply<vec4<f32>>(load, 2_f);
        b.Store(color, mul);
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %color:ptr<__out, vec4<f32>, read_write> = var undef @location(1)
}

%foo = @fragment func():void {
  $B2: {
    store %color, vec4<f32>(1.0f)
    %3:vec4<f32> = load %color
    %4:vec4<f32> = mul %3, 2.0f
    store %color, %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %color:ptr<private, vec4<f32>, read_write> = var undef
}

%foo_inner = func():void {
  $B2: {
    store %color, vec4<f32>(1.0f)
    %3:vec4<f32> = load %color
    %4:vec4<f32> = mul %3, 2.0f
    store %color, %4
    ret
  }
}
%foo = @fragment func():vec4<f32> [@location(1)] {
  $B3: {
    %6:void = call %foo_inner
    %7:vec4<f32> = load %color
    ret %7
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, Output_LoadVectorElementAndStoreVectorElement) {
    auto* color = b.Var("color", ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>()));
    color->SetLocation(1u);

    mod.root_block->Append(color);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(ep->Block(), [&] {  //
        b.Store(color, b.Splat<vec4<f32>>(1_f));
        auto* load = b.LoadVectorElement(color, 2_u);
        auto* mul = b.Multiply<f32>(load, 2_f);
        b.StoreVectorElement(color, 2_u, mul);
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %color:ptr<__out, vec4<f32>, read_write> = var undef @location(1)
}

%foo = @fragment func():void {
  $B2: {
    store %color, vec4<f32>(1.0f)
    %3:f32 = load_vector_element %color, 2u
    %4:f32 = mul %3, 2.0f
    store_vector_element %color, 2u, %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %color:ptr<private, vec4<f32>, read_write> = var undef
}

%foo_inner = func():void {
  $B2: {
    store %color, vec4<f32>(1.0f)
    %3:f32 = load_vector_element %color, 2u
    %4:f32 = mul %3, 2.0f
    store_vector_element %color, 2u, %4
    ret
  }
}
%foo = @fragment func():vec4<f32> [@location(1)] {
  $B3: {
    %6:void = call %foo_inner
    %7:vec4<f32> = load %color
    ret %7
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, Inputs_And_Outputs) {
    auto* position = b.Var("position", ty.ptr(core::AddressSpace::kIn, ty.vec4<f32>()));
    position->SetBuiltin(core::BuiltinValue::kPosition);
    position->SetInvariant(true);

    auto* color_in = b.Var("color_in", ty.ptr(core::AddressSpace::kIn, ty.vec4<f32>()));
    color_in->SetLocation(0);

    auto* color_out_1 = b.Var("color_out_1", ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>()));
    color_out_1->SetLocation(1);

    auto* color_out_2 = b.Var("color_out_2", ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>()));
    color_out_2->SetLocation(2);

    mod.root_block->Append(position);
    mod.root_block->Append(color_in);
    mod.root_block->Append(color_out_1);
    mod.root_block->Append(color_out_2);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(ep->Block(), [&] {
        auto* position_value = b.Load(position);
        auto* color_in_value = b.Load(color_in);
        b.Store(color_out_1, position_value);
        b.Store(color_out_2, color_in_value);
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %position:ptr<__in, vec4<f32>, read> = var undef @invariant @builtin(position)
  %color_in:ptr<__in, vec4<f32>, read> = var undef @location(0)
  %color_out_1:ptr<__out, vec4<f32>, read_write> = var undef @location(1)
  %color_out_2:ptr<__out, vec4<f32>, read_write> = var undef @location(2)
}

%foo = @fragment func():void {
  $B2: {
    %6:vec4<f32> = load %position
    %7:vec4<f32> = load %color_in
    store %color_out_1, %6
    store %color_out_2, %7
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_symbol = struct @align(16) {
  color_out_1:vec4<f32> @offset(0), @location(1)
  color_out_2:vec4<f32> @offset(16), @location(2)
}

$B1: {  # root
  %color_out_1:ptr<private, vec4<f32>, read_write> = var undef
  %color_out_2:ptr<private, vec4<f32>, read_write> = var undef
}

%foo_inner = func(%position:vec4<f32>, %color_in:vec4<f32>):void {
  $B2: {
    store %color_out_1, %position
    store %color_out_2, %color_in
    ret
  }
}
%foo = @fragment func(%position_1:vec4<f32> [@invariant, @position], %color_in_1:vec4<f32> [@location(0)]):tint_symbol {  # %position_1: 'position', %color_in_1: 'color_in'
  $B3: {
    %9:void = call %foo_inner, %position_1, %color_in_1
    %10:vec4<f32> = load %color_out_1
    %11:vec4<f32> = load %color_out_2
    %12:tint_symbol = construct %10, %11
    ret %12
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, InstanceIndex_i32) {
    auto* idx = b.Var("inst_idx", ty.ptr(core::AddressSpace::kIn, ty.i32()));
    idx->SetBuiltin(core::BuiltinValue::kInstanceIndex);
    mod.root_block->Append(idx);

    auto* pos = b.Var("pos", ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>()));
    pos->SetBuiltin(core::BuiltinValue::kPosition);
    mod.root_block->Append(pos);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {
        auto* idx_value = b.Load(idx);
        auto* doubled = b.Multiply(ty.i32(), idx_value, 2_i);
        auto* conv = b.Convert(ty.f32(), doubled);
        b.Store(pos, b.Construct(ty.vec4<f32>(), conv));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %inst_idx:ptr<__in, i32, read> = var undef @builtin(instance_index)
  %pos:ptr<__out, vec4<f32>, read_write> = var undef @builtin(position)
}

%foo = @vertex func():void {
  $B2: {
    %4:i32 = load %inst_idx
    %5:i32 = mul %4, 2i
    %6:f32 = convert %5
    %7:vec4<f32> = construct %6
    store %pos, %7
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %pos:ptr<private, vec4<f32>, read_write> = var undef
}

%foo_inner = func(%inst_idx:i32):void {
  $B2: {
    %4:i32 = mul %inst_idx, 2i
    %5:f32 = convert %4
    %6:vec4<f32> = construct %5
    store %pos, %6
    ret
  }
}
%foo = @vertex func(%inst_idx_1:u32 [@instance_index]):vec4<f32> [@position] {  # %inst_idx_1: 'inst_idx'
  $B3: {
    %9:i32 = convert %inst_idx_1
    %10:void = call %foo_inner, %9
    %11:vec4<f32> = load %pos
    ret %11
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, InstanceIndex_u32) {
    auto* idx = b.Var("inst_idx", ty.ptr(core::AddressSpace::kIn, ty.u32()));
    idx->SetBuiltin(core::BuiltinValue::kInstanceIndex);
    mod.root_block->Append(idx);

    auto* pos = b.Var("pos", ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>()));
    pos->SetBuiltin(core::BuiltinValue::kPosition);
    mod.root_block->Append(pos);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {
        auto* idx_value = b.Load(idx);
        auto* doubled = b.Multiply(ty.u32(), idx_value, 2_u);
        auto* conv = b.Convert(ty.f32(), doubled);
        b.Store(pos, b.Construct(ty.vec4<f32>(), conv));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %inst_idx:ptr<__in, u32, read> = var undef @builtin(instance_index)
  %pos:ptr<__out, vec4<f32>, read_write> = var undef @builtin(position)
}

%foo = @vertex func():void {
  $B2: {
    %4:u32 = load %inst_idx
    %5:u32 = mul %4, 2u
    %6:f32 = convert %5
    %7:vec4<f32> = construct %6
    store %pos, %7
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %pos:ptr<private, vec4<f32>, read_write> = var undef
}

%foo_inner = func(%inst_idx:u32):void {
  $B2: {
    %4:u32 = mul %inst_idx, 2u
    %5:f32 = convert %4
    %6:vec4<f32> = construct %5
    store %pos, %6
    ret
  }
}
%foo = @vertex func(%inst_idx_1:u32 [@instance_index]):vec4<f32> [@position] {  # %inst_idx_1: 'inst_idx'
  $B3: {
    %9:void = call %foo_inner, %inst_idx_1
    %10:vec4<f32> = load %pos
    ret %10
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, VertexIndex_i32) {
    auto* idx = b.Var("inst_idx", ty.ptr(core::AddressSpace::kIn, ty.i32()));
    idx->SetBuiltin(core::BuiltinValue::kVertexIndex);
    mod.root_block->Append(idx);

    auto* pos = b.Var("pos", ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>()));
    pos->SetBuiltin(core::BuiltinValue::kPosition);
    mod.root_block->Append(pos);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {
        auto* idx_value = b.Load(idx);
        auto* doubled = b.Multiply(ty.i32(), idx_value, 2_i);
        auto* conv = b.Convert(ty.f32(), doubled);
        b.Store(pos, b.Construct(ty.vec4<f32>(), conv));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %inst_idx:ptr<__in, i32, read> = var undef @builtin(vertex_index)
  %pos:ptr<__out, vec4<f32>, read_write> = var undef @builtin(position)
}

%foo = @vertex func():void {
  $B2: {
    %4:i32 = load %inst_idx
    %5:i32 = mul %4, 2i
    %6:f32 = convert %5
    %7:vec4<f32> = construct %6
    store %pos, %7
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %pos:ptr<private, vec4<f32>, read_write> = var undef
}

%foo_inner = func(%inst_idx:i32):void {
  $B2: {
    %4:i32 = mul %inst_idx, 2i
    %5:f32 = convert %4
    %6:vec4<f32> = construct %5
    store %pos, %6
    ret
  }
}
%foo = @vertex func(%inst_idx_1:u32 [@vertex_index]):vec4<f32> [@position] {  # %inst_idx_1: 'inst_idx'
  $B3: {
    %9:i32 = convert %inst_idx_1
    %10:void = call %foo_inner, %9
    %11:vec4<f32> = load %pos
    ret %11
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, VertexIndex_u32) {
    auto* idx = b.Var("inst_idx", ty.ptr(core::AddressSpace::kIn, ty.u32()));
    idx->SetBuiltin(core::BuiltinValue::kVertexIndex);
    mod.root_block->Append(idx);

    auto* pos = b.Var("pos", ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>()));
    pos->SetBuiltin(core::BuiltinValue::kPosition);
    mod.root_block->Append(pos);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {
        auto* idx_value = b.Load(idx);
        auto* doubled = b.Multiply(ty.u32(), idx_value, 2_u);
        auto* conv = b.Convert(ty.f32(), doubled);
        b.Store(pos, b.Construct(ty.vec4<f32>(), conv));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %inst_idx:ptr<__in, u32, read> = var undef @builtin(vertex_index)
  %pos:ptr<__out, vec4<f32>, read_write> = var undef @builtin(position)
}

%foo = @vertex func():void {
  $B2: {
    %4:u32 = load %inst_idx
    %5:u32 = mul %4, 2u
    %6:f32 = convert %5
    %7:vec4<f32> = construct %6
    store %pos, %7
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %pos:ptr<private, vec4<f32>, read_write> = var undef
}

%foo_inner = func(%inst_idx:u32):void {
  $B2: {
    %4:u32 = mul %inst_idx, 2u
    %5:f32 = convert %4
    %6:vec4<f32> = construct %5
    store %pos, %6
    ret
  }
}
%foo = @vertex func(%inst_idx_1:u32 [@vertex_index]):vec4<f32> [@position] {  # %inst_idx_1: 'inst_idx'
  $B3: {
    %9:void = call %foo_inner, %inst_idx_1
    %10:vec4<f32> = load %pos
    ret %10
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, LocalInvocationIndex_i32) {
    auto* idx = b.Var("idx", ty.ptr(core::AddressSpace::kIn, ty.i32()));
    idx->SetBuiltin(core::BuiltinValue::kLocalInvocationIndex);
    mod.root_block->Append(idx);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    ep->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));
    b.Append(ep->Block(), [&] {
        auto* idx_value = b.Load(idx);
        b.Let("a", b.Multiply(ty.i32(), idx_value, 2_i));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %idx:ptr<__in, i32, read> = var undef @builtin(local_invocation_index)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:i32 = load %idx
    %4:i32 = mul %3, 2i
    %a:i32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%idx:u32 [@local_invocation_index]):void {
  $B1: {
    %3:i32 = convert %idx
    %4:i32 = mul %3, 2i
    %a:i32 = let %4
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, LocalInvocationIndex_u32) {
    auto* idx = b.Var("idx", ty.ptr(core::AddressSpace::kIn, ty.u32()));
    idx->SetBuiltin(core::BuiltinValue::kLocalInvocationIndex);
    mod.root_block->Append(idx);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    ep->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));
    b.Append(ep->Block(), [&] {
        auto* idx_value = b.Load(idx);
        b.Let("a", b.Multiply(ty.u32(), idx_value, 2_u));

        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %idx:ptr<__in, u32, read> = var undef @builtin(local_invocation_index)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = load %idx
    %4:u32 = mul %3, 2u
    %a:u32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%idx:u32 [@local_invocation_index]):void {
  $B1: {
    %3:u32 = mul %idx, 2u
    %a:u32 = let %3
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, SubgroupInvocationId_i32) {
    auto* idx = b.Var("idx", ty.ptr(core::AddressSpace::kIn, ty.i32()));
    idx->SetBuiltin(core::BuiltinValue::kSubgroupInvocationId);
    mod.root_block->Append(idx);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    ep->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));
    b.Append(ep->Block(), [&] {
        auto* idx_value = b.Load(idx);
        b.Let("a", b.Multiply(ty.i32(), idx_value, 2_i));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %idx:ptr<__in, i32, read> = var undef @builtin(subgroup_invocation_id)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:i32 = load %idx
    %4:i32 = mul %3, 2i
    %a:i32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%idx:u32 [@subgroup_invocation_id]):void {
  $B1: {
    %3:i32 = convert %idx
    %4:i32 = mul %3, 2i
    %a:i32 = let %4
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, SubgroupInvocationId_u32) {
    auto* idx = b.Var("idx", ty.ptr(core::AddressSpace::kIn, ty.u32()));
    idx->SetBuiltin(core::BuiltinValue::kSubgroupInvocationId);
    mod.root_block->Append(idx);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    ep->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));
    b.Append(ep->Block(), [&] {
        auto* idx_value = b.Load(idx);
        b.Let("a", b.Multiply(ty.u32(), idx_value, 2_u));

        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %idx:ptr<__in, u32, read> = var undef @builtin(subgroup_invocation_id)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = load %idx
    %4:u32 = mul %3, 2u
    %a:u32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%idx:u32 [@subgroup_invocation_id]):void {
  $B1: {
    %3:u32 = mul %idx, 2u
    %a:u32 = let %3
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, SubgroupSize_i32) {
    auto* idx = b.Var("idx", ty.ptr(core::AddressSpace::kIn, ty.i32()));
    idx->SetBuiltin(core::BuiltinValue::kSubgroupSize);
    mod.root_block->Append(idx);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    ep->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));
    b.Append(ep->Block(), [&] {
        auto* idx_value = b.Load(idx);
        b.Let("a", b.Multiply(ty.i32(), idx_value, 2_i));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %idx:ptr<__in, i32, read> = var undef @builtin(subgroup_size)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:i32 = load %idx
    %4:i32 = mul %3, 2i
    %a:i32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%idx:u32 [@subgroup_size]):void {
  $B1: {
    %3:i32 = convert %idx
    %4:i32 = mul %3, 2i
    %a:i32 = let %4
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, SubgroupSize_u32) {
    auto* idx = b.Var("idx", ty.ptr(core::AddressSpace::kIn, ty.u32()));
    idx->SetBuiltin(core::BuiltinValue::kSubgroupSize);
    mod.root_block->Append(idx);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    ep->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));
    b.Append(ep->Block(), [&] {
        auto* idx_value = b.Load(idx);
        b.Let("a", b.Multiply(ty.u32(), idx_value, 2_u));

        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %idx:ptr<__in, u32, read> = var undef @builtin(subgroup_size)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = load %idx
    %4:u32 = mul %3, 2u
    %a:u32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%idx:u32 [@subgroup_size]):void {
  $B1: {
    %3:u32 = mul %idx, 2u
    %a:u32 = let %3
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, LocalInvocationId_i32) {
    auto* idx = b.Var("idx", ty.ptr(core::AddressSpace::kIn, ty.vec3<i32>()));
    idx->SetBuiltin(core::BuiltinValue::kLocalInvocationId);
    mod.root_block->Append(idx);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    ep->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));
    b.Append(ep->Block(), [&] {
        auto* idx_value = b.Load(idx);
        b.Let("a", b.Multiply(ty.vec3<i32>(), idx_value, b.Splat(ty.vec3<i32>(), 2_i)));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %idx:ptr<__in, vec3<i32>, read> = var undef @builtin(local_invocation_id)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:vec3<i32> = load %idx
    %4:vec3<i32> = mul %3, vec3<i32>(2i)
    %a:vec3<i32> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%idx:vec3<u32> [@local_invocation_id]):void {
  $B1: {
    %3:vec3<i32> = convert %idx
    %4:vec3<i32> = mul %3, vec3<i32>(2i)
    %a:vec3<i32> = let %4
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, LocalInvocationId_u32) {
    auto* idx = b.Var("idx", ty.ptr(core::AddressSpace::kIn, ty.vec3<u32>()));
    idx->SetBuiltin(core::BuiltinValue::kLocalInvocationId);
    mod.root_block->Append(idx);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    ep->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));
    b.Append(ep->Block(), [&] {
        auto* idx_value = b.Load(idx);
        b.Let("a", b.Multiply(ty.vec3<u32>(), idx_value, b.Splat(ty.vec3<u32>(), 2_u)));

        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %idx:ptr<__in, vec3<u32>, read> = var undef @builtin(local_invocation_id)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:vec3<u32> = load %idx
    %4:vec3<u32> = mul %3, vec3<u32>(2u)
    %a:vec3<u32> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%idx:vec3<u32> [@local_invocation_id]):void {
  $B1: {
    %3:vec3<u32> = mul %idx, vec3<u32>(2u)
    %a:vec3<u32> = let %3
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, GlobalInvocationId_i32) {
    auto* idx = b.Var("idx", ty.ptr(core::AddressSpace::kIn, ty.vec3<i32>()));
    idx->SetBuiltin(core::BuiltinValue::kGlobalInvocationId);
    mod.root_block->Append(idx);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    ep->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));
    b.Append(ep->Block(), [&] {
        auto* idx_value = b.Load(idx);
        b.Let("a", b.Multiply(ty.vec3<i32>(), idx_value, b.Splat(ty.vec3<i32>(), 2_i)));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %idx:ptr<__in, vec3<i32>, read> = var undef @builtin(global_invocation_id)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:vec3<i32> = load %idx
    %4:vec3<i32> = mul %3, vec3<i32>(2i)
    %a:vec3<i32> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%idx:vec3<u32> [@global_invocation_id]):void {
  $B1: {
    %3:vec3<i32> = convert %idx
    %4:vec3<i32> = mul %3, vec3<i32>(2i)
    %a:vec3<i32> = let %4
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, GlobalInvocationId_u32) {
    auto* idx = b.Var("idx", ty.ptr(core::AddressSpace::kIn, ty.vec3<u32>()));
    idx->SetBuiltin(core::BuiltinValue::kGlobalInvocationId);
    mod.root_block->Append(idx);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    ep->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));
    b.Append(ep->Block(), [&] {
        auto* idx_value = b.Load(idx);
        b.Let("a", b.Multiply(ty.vec3<u32>(), idx_value, b.Splat(ty.vec3<u32>(), 2_u)));

        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %idx:ptr<__in, vec3<u32>, read> = var undef @builtin(global_invocation_id)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:vec3<u32> = load %idx
    %4:vec3<u32> = mul %3, vec3<u32>(2u)
    %a:vec3<u32> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%idx:vec3<u32> [@global_invocation_id]):void {
  $B1: {
    %3:vec3<u32> = mul %idx, vec3<u32>(2u)
    %a:vec3<u32> = let %3
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, WorkgroupId_i32) {
    auto* idx = b.Var("idx", ty.ptr(core::AddressSpace::kIn, ty.vec3<i32>()));
    idx->SetBuiltin(core::BuiltinValue::kWorkgroupId);
    mod.root_block->Append(idx);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    ep->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));
    b.Append(ep->Block(), [&] {
        auto* idx_value = b.Load(idx);
        b.Let("a", b.Multiply(ty.vec3<i32>(), idx_value, b.Splat(ty.vec3<i32>(), 2_i)));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %idx:ptr<__in, vec3<i32>, read> = var undef @builtin(workgroup_id)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:vec3<i32> = load %idx
    %4:vec3<i32> = mul %3, vec3<i32>(2i)
    %a:vec3<i32> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%idx:vec3<u32> [@workgroup_id]):void {
  $B1: {
    %3:vec3<i32> = convert %idx
    %4:vec3<i32> = mul %3, vec3<i32>(2i)
    %a:vec3<i32> = let %4
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, WorkgroupId_u32) {
    auto* idx = b.Var("idx", ty.ptr(core::AddressSpace::kIn, ty.vec3<u32>()));
    idx->SetBuiltin(core::BuiltinValue::kWorkgroupId);
    mod.root_block->Append(idx);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    ep->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));
    b.Append(ep->Block(), [&] {
        auto* idx_value = b.Load(idx);
        b.Let("a", b.Multiply(ty.vec3<u32>(), idx_value, b.Splat(ty.vec3<u32>(), 2_u)));

        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %idx:ptr<__in, vec3<u32>, read> = var undef @builtin(workgroup_id)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:vec3<u32> = load %idx
    %4:vec3<u32> = mul %3, vec3<u32>(2u)
    %a:vec3<u32> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%idx:vec3<u32> [@workgroup_id]):void {
  $B1: {
    %3:vec3<u32> = mul %idx, vec3<u32>(2u)
    %a:vec3<u32> = let %3
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, NumWorkgroups_i32) {
    auto* idx = b.Var("idx", ty.ptr(core::AddressSpace::kIn, ty.vec3<i32>()));
    idx->SetBuiltin(core::BuiltinValue::kNumWorkgroups);
    mod.root_block->Append(idx);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    ep->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));
    b.Append(ep->Block(), [&] {
        auto* idx_value = b.Load(idx);
        b.Let("a", b.Multiply(ty.vec3<i32>(), idx_value, b.Splat(ty.vec3<i32>(), 2_i)));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %idx:ptr<__in, vec3<i32>, read> = var undef @builtin(num_workgroups)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:vec3<i32> = load %idx
    %4:vec3<i32> = mul %3, vec3<i32>(2i)
    %a:vec3<i32> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%idx:vec3<u32> [@num_workgroups]):void {
  $B1: {
    %3:vec3<i32> = convert %idx
    %4:vec3<i32> = mul %3, vec3<i32>(2i)
    %a:vec3<i32> = let %4
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, NumWorkgroups_u32) {
    auto* idx = b.Var("idx", ty.ptr(core::AddressSpace::kIn, ty.vec3<u32>()));
    idx->SetBuiltin(core::BuiltinValue::kNumWorkgroups);
    mod.root_block->Append(idx);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    ep->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));
    b.Append(ep->Block(), [&] {
        auto* idx_value = b.Load(idx);
        b.Let("a", b.Multiply(ty.vec3<u32>(), idx_value, b.Splat(ty.vec3<u32>(), 2_u)));

        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %idx:ptr<__in, vec3<u32>, read> = var undef @builtin(num_workgroups)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:vec3<u32> = load %idx
    %4:vec3<u32> = mul %3, vec3<u32>(2u)
    %a:vec3<u32> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%idx:vec3<u32> [@num_workgroups]):void {
  $B1: {
    %3:vec3<u32> = mul %idx, vec3<u32>(2u)
    %a:vec3<u32> = let %3
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, SampleIndex_i32) {
    auto* idx = b.Var("idx", ty.ptr(core::AddressSpace::kIn, ty.i32()));
    idx->SetBuiltin(core::BuiltinValue::kSampleIndex);
    mod.root_block->Append(idx);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(ep->Block(), [&] {
        auto* idx_value = b.Load(idx);
        b.Let("a", b.Multiply(ty.i32(), idx_value, 2_i));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %idx:ptr<__in, i32, read> = var undef @builtin(sample_index)
}

%foo = @fragment func():void {
  $B2: {
    %3:i32 = load %idx
    %4:i32 = mul %3, 2i
    %a:i32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func(%idx:u32 [@sample_index]):void {
  $B1: {
    %3:i32 = convert %idx
    %4:i32 = mul %3, 2i
    %a:i32 = let %4
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, SampleIndex_u32) {
    auto* idx = b.Var("idx", ty.ptr(core::AddressSpace::kIn, ty.u32()));
    idx->SetBuiltin(core::BuiltinValue::kSampleIndex);
    mod.root_block->Append(idx);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(ep->Block(), [&] {
        auto* idx_value = b.Load(idx);
        b.Let("a", b.Multiply(ty.u32(), idx_value, 2_u));

        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %idx:ptr<__in, u32, read> = var undef @builtin(sample_index)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = load %idx
    %4:u32 = mul %3, 2u
    %a:u32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func(%idx:u32 [@sample_index]):void {
  $B1: {
    %3:u32 = mul %idx, 2u
    %a:u32 = let %3
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

// Test that a sample mask array is converted to a scalar u32 for the entry point.
TEST_F(SpirvReader_ShaderIOTest, SampleMask) {
    auto* arr = ty.array<u32, 1>();
    auto* mask_in = b.Var("mask_in", ty.ptr(core::AddressSpace::kIn, arr));
    mask_in->SetBuiltin(core::BuiltinValue::kSampleMask);

    auto* mask_out = b.Var("mask_out", ty.ptr(core::AddressSpace::kOut, arr));
    mask_out->SetBuiltin(core::BuiltinValue::kSampleMask);

    mod.root_block->Append(mask_in);
    mod.root_block->Append(mask_out);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(ep->Block(), [&] {
        auto* mask_value = b.Load(mask_in);
        auto* doubled = b.Multiply(ty.u32(), b.Access(ty.u32(), mask_value, 0_u), 2_u);
        b.Store(mask_out, b.Construct(arr, doubled));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %mask_in:ptr<__in, array<u32, 1>, read> = var undef @builtin(sample_mask)
  %mask_out:ptr<__out, array<u32, 1>, read_write> = var undef @builtin(sample_mask)
}

%foo = @fragment func():void {
  $B2: {
    %4:array<u32, 1> = load %mask_in
    %5:u32 = access %4, 0u
    %6:u32 = mul %5, 2u
    %7:array<u32, 1> = construct %6
    store %mask_out, %7
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %mask_out:ptr<private, array<u32, 1>, read_write> = var undef
}

%foo_inner = func(%mask_in:array<u32, 1>):void {
  $B2: {
    %4:u32 = access %mask_in, 0u
    %5:u32 = mul %4, 2u
    %6:array<u32, 1> = construct %5
    store %mask_out, %6
    ret
  }
}
%foo = @fragment func(%mask_in_1:u32 [@sample_mask]):u32 [@sample_mask] {  # %mask_in_1: 'mask_in'
  $B3: {
    %9:array<u32, 1> = construct %mask_in_1
    %10:void = call %foo_inner, %9
    %11:array<u32, 1> = load %mask_out
    %12:u32 = access %11, 0u
    ret %12
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, SampleMask_I32) {
    auto* arr = ty.array<i32, 1>();
    auto* mask_in = b.Var("mask_in", ty.ptr(core::AddressSpace::kIn, arr));
    mask_in->SetBuiltin(core::BuiltinValue::kSampleMask);

    auto* mask_out = b.Var("mask_out", ty.ptr(core::AddressSpace::kOut, arr));
    mask_out->SetBuiltin(core::BuiltinValue::kSampleMask);

    mod.root_block->Append(mask_in);
    mod.root_block->Append(mask_out);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(ep->Block(), [&] {
        auto* mask_value = b.Load(mask_in);
        auto* doubled = b.Multiply(ty.i32(), b.Access(ty.i32(), mask_value, 0_u), 2_i);
        b.Store(mask_out, b.Construct(arr, doubled));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %mask_in:ptr<__in, array<i32, 1>, read> = var undef @builtin(sample_mask)
  %mask_out:ptr<__out, array<i32, 1>, read_write> = var undef @builtin(sample_mask)
}

%foo = @fragment func():void {
  $B2: {
    %4:array<i32, 1> = load %mask_in
    %5:i32 = access %4, 0u
    %6:i32 = mul %5, 2i
    %7:array<i32, 1> = construct %6
    store %mask_out, %7
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %mask_out:ptr<private, array<i32, 1>, read_write> = var undef
}

%foo_inner = func(%mask_in:array<i32, 1>):void {
  $B2: {
    %4:i32 = access %mask_in, 0u
    %5:i32 = mul %4, 2i
    %6:array<i32, 1> = construct %5
    store %mask_out, %6
    ret
  }
}
%foo = @fragment func(%mask_in_1:u32 [@sample_mask]):u32 [@sample_mask] {  # %mask_in_1: 'mask_in'
  $B3: {
    %9:i32 = convert %mask_in_1
    %10:array<i32, 1> = construct %9
    %11:void = call %foo_inner, %10
    %12:array<i32, 1> = load %mask_out
    %13:i32 = access %12, 0u
    %14:u32 = convert %13
    ret %14
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, PointSize_Struct) {
    auto* builtin_str =
        ty.Struct(mod.symbols.New("Builtins"), Vector{
                                                   core::type::Manager::StructMemberDesc{
                                                       mod.symbols.New("position"),
                                                       ty.vec4<f32>(),
                                                       BuiltinAttrs(core::BuiltinValue::kPosition),
                                                   },
                                                   core::type::Manager::StructMemberDesc{
                                                       mod.symbols.New("point_size"),
                                                       ty.f32(),
                                                       BuiltinAttrs(core::BuiltinValue::kPointSize),
                                                   },
                                               });
    auto* builtins = b.Var("builtins", ty.ptr(core::AddressSpace::kOut, builtin_str));
    mod.root_block->Append(builtins);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {  //
        auto* ptr = ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>());
        b.Store(b.Access(ptr, builtins, 0_u), b.Splat<vec4<f32>>(1_f));
        b.Store(b.Access(ty.ptr(core::AddressSpace::kOut, ty.f32()), builtins, 1_u), 1_f);
        b.Return(ep);
    });

    auto* src = R"(
Builtins = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
  point_size:f32 @offset(16), @builtin(__point_size)
}

$B1: {  # root
  %builtins:ptr<__out, Builtins, read_write> = var undef
}

%foo = @vertex func():void {
  $B2: {
    %3:ptr<__out, vec4<f32>, read_write> = access %builtins, 0u
    store %3, vec4<f32>(1.0f)
    %4:ptr<__out, f32, read_write> = access %builtins, 1u
    store %4, 1.0f
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Builtins = struct @align(16) {
  position:vec4<f32> @offset(0)
  point_size:f32 @offset(16)
}

$B1: {  # root
  %builtins:ptr<private, Builtins, read_write> = var undef
}

%foo_inner = func():void {
  $B2: {
    %3:ptr<private, vec4<f32>, read_write> = access %builtins, 0u
    store %3, vec4<f32>(1.0f)
    %4:ptr<private, f32, read_write> = access %builtins, 1u
    store %4, 1.0f
    ret
  }
}
%foo = @vertex func():vec4<f32> [@position] {
  $B3: {
    %6:void = call %foo_inner
    %7:ptr<private, vec4<f32>, read_write> = access %builtins, 0u
    %8:vec4<f32> = load %7
    ret %8
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, PointSize_Struct_StoreNotOne) {
    auto* builtin_str =
        ty.Struct(mod.symbols.New("Builtins"), Vector{
                                                   core::type::Manager::StructMemberDesc{
                                                       mod.symbols.New("position"),
                                                       ty.vec4<f32>(),
                                                       BuiltinAttrs(core::BuiltinValue::kPosition),
                                                   },
                                                   core::type::Manager::StructMemberDesc{
                                                       mod.symbols.New("point_size"),
                                                       ty.f32(),
                                                       BuiltinAttrs(core::BuiltinValue::kPointSize),
                                                   },
                                               });
    auto* builtins = b.Var("builtins", ty.ptr(core::AddressSpace::kOut, builtin_str));
    mod.root_block->Append(builtins);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {  //
        auto* ptr = ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>());
        b.Store(b.Access(ptr, builtins, 0_u), b.Splat<vec4<f32>>(1_f));
        b.Store(b.Access(ty.ptr(core::AddressSpace::kOut, ty.f32()), builtins, 1_u), 2_f);
        b.Return(ep);
    });

    auto* src = R"(
Builtins = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
  point_size:f32 @offset(16), @builtin(__point_size)
}

$B1: {  # root
  %builtins:ptr<__out, Builtins, read_write> = var undef
}

%foo = @vertex func():void {
  $B2: {
    %3:ptr<__out, vec4<f32>, read_write> = access %builtins, 0u
    store %3, vec4<f32>(1.0f)
    %4:ptr<__out, f32, read_write> = access %builtins, 1u
    store %4, 2.0f
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto result = ShaderIO(mod);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().reason, "store to point_size is not 1.0");
}

TEST_F(SpirvReader_ShaderIOTest, PointSize_Var) {
    auto* ps = b.Var("point_size", ty.ptr(core::AddressSpace::kOut, ty.f32()));
    ps->SetBuiltin(core::BuiltinValue::kPointSize);
    mod.root_block->Append(ps);

    auto* o = b.Var("other", ty.ptr<private_, f32>());
    mod.root_block->Append(o);

    auto* pos = b.Var("position", ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>()));
    pos->SetBuiltin(core::BuiltinValue::kPosition);
    mod.root_block->Append(pos);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {  //
        auto* v = b.Load(ps);
        b.Store(o, v);

        b.Store(pos, b.Zero(ty.vec4<f32>()));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %point_size:ptr<__out, f32, read_write> = var undef @builtin(__point_size)
  %other:ptr<private, f32, read_write> = var undef
  %position:ptr<__out, vec4<f32>, read_write> = var undef @builtin(position)
}

%foo = @vertex func():void {
  $B2: {
    %5:f32 = load %point_size
    store %other, %5
    store %position, vec4<f32>(0.0f)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %point_size:ptr<private, f32, read_write> = var 1.0f
  %other:ptr<private, f32, read_write> = var undef
  %position:ptr<private, vec4<f32>, read_write> = var undef
}

%foo_inner = func():void {
  $B2: {
    %5:f32 = load %point_size
    store %other, %5
    store %position, vec4<f32>(0.0f)
    ret
  }
}
%foo = @vertex func():vec4<f32> [@position] {
  $B3: {
    %7:void = call %foo_inner
    %8:vec4<f32> = load %position
    ret %8
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, PointSize_Var_StoreNotOne) {
    auto* position = b.Var("position", ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>()));
    position->SetBuiltin(core::BuiltinValue::kPosition);
    mod.root_block->Append(position);

    auto* pointsize = b.Var("pointsize", ty.ptr(core::AddressSpace::kOut, ty.f32()));
    pointsize->SetBuiltin(core::BuiltinValue::kPointSize);
    mod.root_block->Append(pointsize);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {  //
        b.Store(position, b.Splat<vec4<f32>>(0_f));
        b.Store(pointsize, 2_f);
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %position:ptr<__out, vec4<f32>, read_write> = var undef @builtin(position)
  %pointsize:ptr<__out, f32, read_write> = var undef @builtin(__point_size)
}

%foo = @vertex func():void {
  $B2: {
    store %position, vec4<f32>(0.0f)
    store %pointsize, 2.0f
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto result = ShaderIO(mod);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().reason, "store to point_size is not 1.0");
}

TEST_F(SpirvReader_ShaderIOTest, PointSize_Var_StoreNotOne_ViaLet) {
    auto* position = b.Var("position", ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>()));
    position->SetBuiltin(core::BuiltinValue::kPosition);
    mod.root_block->Append(position);

    auto* pointsize = b.Var("pointsize", ty.ptr(core::AddressSpace::kOut, ty.f32()));
    pointsize->SetBuiltin(core::BuiltinValue::kPointSize);
    mod.root_block->Append(pointsize);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {  //
        b.Store(position, b.Splat<vec4<f32>>(0_f));
        auto* ptr = b.Let(pointsize);
        b.Store(ptr, 2_f);
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %position:ptr<__out, vec4<f32>, read_write> = var undef @builtin(position)
  %pointsize:ptr<__out, f32, read_write> = var undef @builtin(__point_size)
}

%foo = @vertex func():void {
  $B2: {
    store %position, vec4<f32>(0.0f)
    %4:ptr<__out, f32, read_write> = let %pointsize
    store %4, 2.0f
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto result = ShaderIO(mod);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().reason, "store to point_size is not 1.0");
}

TEST_F(SpirvReader_ShaderIOTest, Outputs_Struct_UnusedOutputs) {
    auto* builtin_str = ty.Struct(mod.symbols.New("Builtins"),
                                  Vector{
                                      core::type::Manager::StructMemberDesc{
                                          mod.symbols.New("position"),
                                          ty.vec4<f32>(),
                                          BuiltinAttrs(core::BuiltinValue::kPosition),
                                      },
                                      core::type::Manager::StructMemberDesc{
                                          mod.symbols.New("clip_distance"),
                                          ty.array<f32, 3>(),
                                          BuiltinAttrs(core::BuiltinValue::kClipDistances),
                                      },
                                      core::type::Manager::StructMemberDesc{
                                          mod.symbols.New("point_size"),
                                          ty.f32(),
                                          BuiltinAttrs(core::BuiltinValue::kPointSize),
                                      },
                                      core::type::Manager::StructMemberDesc{
                                          mod.symbols.New("cull_distance"),
                                          ty.array<f32, 3>(),
                                          BuiltinAttrs(core::BuiltinValue::kCullDistance),
                                      },
                                  });
    auto* builtins = b.Var("builtins", ty.ptr(core::AddressSpace::kOut, builtin_str));
    mod.root_block->Append(builtins);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {  //
        auto* ptr = ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>());
        b.Store(b.Access(ptr, builtins, 0_u), b.Splat<vec4<f32>>(1_f));
        b.Return(ep);
    });

    auto* src = R"(
Builtins = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
  clip_distance:array<f32, 3> @offset(16), @builtin(clip_distances)
  point_size:f32 @offset(28), @builtin(__point_size)
  cull_distance:array<f32, 3> @offset(32), @builtin(__cull_distance)
}

$B1: {  # root
  %builtins:ptr<__out, Builtins, read_write> = var undef
}

%foo = @vertex func():void {
  $B2: {
    %3:ptr<__out, vec4<f32>, read_write> = access %builtins, 0u
    store %3, vec4<f32>(1.0f)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Builtins = struct @align(16) {
  position:vec4<f32> @offset(0)
  clip_distance:array<f32, 3> @offset(16)
  point_size:f32 @offset(28)
  cull_distance:array<f32, 3> @offset(32)
}

$B1: {  # root
  %builtins:ptr<private, Builtins, read_write> = var undef
}

%foo_inner = func():void {
  $B2: {
    %3:ptr<private, vec4<f32>, read_write> = access %builtins, 0u
    store %3, vec4<f32>(1.0f)
    ret
  }
}
%foo = @vertex func():vec4<f32> [@position] {
  $B3: {
    %5:void = call %foo_inner
    %6:ptr<private, vec4<f32>, read_write> = access %builtins, 0u
    %7:vec4<f32> = load %6
    ret %7
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, Outputs_Struct_MultipledUsed) {
    auto* builtin_str = ty.Struct(mod.symbols.New("Builtins"),
                                  Vector{
                                      core::type::Manager::StructMemberDesc{
                                          mod.symbols.New("position"),
                                          ty.vec4<f32>(),
                                          BuiltinAttrs(core::BuiltinValue::kPosition),
                                      },
                                      core::type::Manager::StructMemberDesc{
                                          mod.symbols.New("clip_distance"),
                                          ty.array<f32, 3>(),
                                          BuiltinAttrs(core::BuiltinValue::kClipDistances),
                                      },
                                  });
    auto* builtins = b.Var("builtins", ty.ptr(core::AddressSpace::kOut, builtin_str));
    mod.root_block->Append(builtins);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {  //
        auto* ptr1 = ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>());
        b.Store(b.Access(ptr1, builtins, 0_u), b.Splat<vec4<f32>>(1_f));

        auto* ptr2 = ty.ptr(core::AddressSpace::kOut, ty.array<f32, 3>());
        b.Store(b.Access(ptr2, builtins, 1_u), b.Splat<array<f32, 3>>(1_f));
        b.Return(ep);
    });

    auto* src = R"(
Builtins = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
  clip_distance:array<f32, 3> @offset(16), @builtin(clip_distances)
}

$B1: {  # root
  %builtins:ptr<__out, Builtins, read_write> = var undef
}

%foo = @vertex func():void {
  $B2: {
    %3:ptr<__out, vec4<f32>, read_write> = access %builtins, 0u
    store %3, vec4<f32>(1.0f)
    %4:ptr<__out, array<f32, 3>, read_write> = access %builtins, 1u
    store %4, array<f32, 3>(1.0f)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Builtins = struct @align(16) {
  position:vec4<f32> @offset(0)
  clip_distance:array<f32, 3> @offset(16)
}

tint_symbol = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
  clip_distance:array<f32, 3> @offset(16), @builtin(clip_distances)
}

$B1: {  # root
  %builtins:ptr<private, Builtins, read_write> = var undef
}

%foo_inner = func():void {
  $B2: {
    %3:ptr<private, vec4<f32>, read_write> = access %builtins, 0u
    store %3, vec4<f32>(1.0f)
    %4:ptr<private, array<f32, 3>, read_write> = access %builtins, 1u
    store %4, array<f32, 3>(1.0f)
    ret
  }
}
%foo = @vertex func():tint_symbol {
  $B3: {
    %6:void = call %foo_inner
    %7:ptr<private, vec4<f32>, read_write> = access %builtins, 0u
    %8:vec4<f32> = load %7
    %9:ptr<private, array<f32, 3>, read_write> = access %builtins, 1u
    %10:array<f32, 3> = load %9
    %11:tint_symbol = construct %8, %10
    ret %11
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, Outputs_ThroughLet) {
    auto* position = b.Var("position", ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>()));
    position->SetBuiltin(core::BuiltinValue::kPosition);

    mod.root_block->Append(position);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {  //
        b.Let("tmp", position);
        b.Store(position, b.Splat<vec4<f32>>(1_f));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %position:ptr<__out, vec4<f32>, read_write> = var undef @builtin(position)
}

%foo = @vertex func():void {
  $B2: {
    %tmp:ptr<__out, vec4<f32>, read_write> = let %position
    store %position, vec4<f32>(1.0f)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %position:ptr<private, vec4<f32>, read_write> = var undef
}

%foo_inner = func():void {
  $B2: {
    %tmp:ptr<private, vec4<f32>, read_write> = let %position
    store %position, vec4<f32>(1.0f)
    ret
  }
}
%foo = @vertex func():vec4<f32> [@position] {
  $B3: {
    %5:void = call %foo_inner
    %6:vec4<f32> = load %position
    ret %6
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, Input_Array) {
    auto* ary = b.Var("ary", ty.ptr(core::AddressSpace::kIn, ty.array<f32, 2>()));
    ary->SetLocation(1);
    ary->SetInterpolation(core::Interpolation{
        .type = core::InterpolationType::kFlat,
    });
    mod.root_block->Append(ary);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(ep->Block(), [&] {
        auto* ld = b.Load(ary);
        auto* access = b.Access(ty.f32(), ld, 1_u);
        b.Add(ty.f32(), access, access);
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %ary:ptr<__in, array<f32, 2>, read> = var undef @location(1) @interpolate(flat)
}

%foo = @fragment func():void {
  $B2: {
    %3:array<f32, 2> = load %ary
    %4:f32 = access %3, 1u
    %5:f32 = add %4, %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func(%2:f32 [@location(1), @interpolate(flat)], %3:f32 [@location(2), @interpolate(flat)]):void {
  $B1: {
    %ary:array<f32, 2> = construct %2, %3
    %5:f32 = access %ary, 1u
    %6:f32 = add %5, %5
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, Input_Matrix) {
    auto* mat = b.Var("mat", ty.ptr(core::AddressSpace::kIn, ty.mat2x4<f32>()));
    mat->SetLocation(1);
    mat->SetInterpolation(core::Interpolation{
        .type = core::InterpolationType::kFlat,
    });
    mod.root_block->Append(mat);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(ep->Block(), [&] {
        auto* ld = b.Load(mat);
        auto* access = b.Access(ty.f32(), ld, 1_u, 1_u);
        b.Add(ty.f32(), access, access);
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %mat:ptr<__in, mat2x4<f32>, read> = var undef @location(1) @interpolate(flat)
}

%foo = @fragment func():void {
  $B2: {
    %3:mat2x4<f32> = load %mat
    %4:f32 = access %3, 1u, 1u
    %5:f32 = add %4, %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func(%2:vec4<f32> [@location(1), @interpolate(flat)], %3:vec4<f32> [@location(2), @interpolate(flat)]):void {
  $B1: {
    %mat:mat2x4<f32> = construct %2, %3
    %5:f32 = access %mat, 1u, 1u
    %6:f32 = add %5, %5
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, Input_Struct) {
    auto* S = ty.Struct(mod.symbols.New("S"), Vector{
                                                  core::type::Manager::StructMemberDesc{
                                                      mod.symbols.New("a"),
                                                      ty.vec4<f32>(),
                                                  },
                                                  core::type::Manager::StructMemberDesc{
                                                      mod.symbols.New("b"),
                                                      ty.vec4<f32>(),
                                                  },
                                              });
    auto* s = b.Var("s", ty.ptr(core::AddressSpace::kIn, S));
    s->SetLocation(1);
    s->SetInterpolation(core::Interpolation{
        .type = core::InterpolationType::kFlat,
    });
    mod.root_block->Append(s);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(ep->Block(), [&] {
        auto* ld = b.Load(s);
        auto* access = b.Access(ty.f32(), ld, 1_u, 1_u);
        b.Add(ty.f32(), access, access);
        b.Return(ep);
    });

    auto* src = R"(
S = struct @align(16) {
  a:vec4<f32> @offset(0)
  b:vec4<f32> @offset(16)
}

$B1: {  # root
  %s:ptr<__in, S, read> = var undef @location(1) @interpolate(flat)
}

%foo = @fragment func():void {
  $B2: {
    %3:S = load %s
    %4:f32 = access %3, 1u, 1u
    %5:f32 = add %4, %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  a:vec4<f32> @offset(0)
  b:vec4<f32> @offset(16)
}

%foo = @fragment func(%2:vec4<f32> [@location(1), @interpolate(flat)], %3:vec4<f32> [@location(2), @interpolate(flat)]):void {
  $B1: {
    %s:S = construct %2, %3
    %5:f32 = access %s, 1u, 1u
    %6:f32 = add %5, %5
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, Input_ArrayOfStruct) {
    auto* S = ty.Struct(mod.symbols.New("S"), Vector{
                                                  core::type::Manager::StructMemberDesc{
                                                      mod.symbols.New("a"),
                                                      ty.vec4<f32>(),
                                                  },
                                                  core::type::Manager::StructMemberDesc{
                                                      mod.symbols.New("b"),
                                                      ty.vec4<f32>(),
                                                  },
                                              });
    auto* s = b.Var("s", ty.ptr(core::AddressSpace::kIn, ty.array(S, 2)));
    s->SetLocation(1);
    s->SetInterpolation(core::Interpolation{
        .type = core::InterpolationType::kFlat,
    });
    mod.root_block->Append(s);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(ep->Block(), [&] {
        auto* ld = b.Load(s);
        auto* access = b.Access(ty.f32(), ld, 1_u, 1_u, 2_u);
        b.Add(ty.f32(), access, access);
        b.Return(ep);
    });

    auto* src = R"(
S = struct @align(16) {
  a:vec4<f32> @offset(0)
  b:vec4<f32> @offset(16)
}

$B1: {  # root
  %s:ptr<__in, array<S, 2>, read> = var undef @location(1) @interpolate(flat)
}

%foo = @fragment func():void {
  $B2: {
    %3:array<S, 2> = load %s
    %4:f32 = access %3, 1u, 1u, 2u
    %5:f32 = add %4, %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  a:vec4<f32> @offset(0)
  b:vec4<f32> @offset(16)
}

%foo = @fragment func(%2:vec4<f32> [@location(1), @interpolate(flat)], %3:vec4<f32> [@location(2), @interpolate(flat)], %4:vec4<f32> [@location(3), @interpolate(flat)], %5:vec4<f32> [@location(4), @interpolate(flat)]):void {
  $B1: {
    %6:S = construct %2, %3
    %7:S = construct %4, %5
    %s:array<S, 2> = construct %6, %7
    %9:f32 = access %s, 1u, 1u, 2u
    %10:f32 = add %9, %9
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, Output_StructArrayMatrix) {
    auto* S = ty.Struct(mod.symbols.New("S"), Vector{
                                                  core::type::Manager::StructMemberDesc{
                                                      mod.symbols.New("a"),
                                                      ty.array(ty.mat2x4<f32>(), 2),
                                                  },
                                                  core::type::Manager::StructMemberDesc{
                                                      mod.symbols.New("b"),
                                                      ty.vec4<f32>(),
                                                  },
                                                  core::type::Manager::StructMemberDesc{
                                                      mod.symbols.New("c"),
                                                      ty.f32(),
                                                  },
                                              });
    auto* s = b.Var("s", ty.ptr(core::AddressSpace::kOut, ty.array(S, 2)));
    s->SetLocation(1);
    mod.root_block->Append(s);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(ep->Block(), [&] {
        auto* a = b.Access(ty.ptr(core::AddressSpace::kOut, ty.f32(), core::Access::kReadWrite), s,
                           1_u, 2_u);
        b.Store(a, 1_f);
        b.Return(ep);
    });

    auto* src = R"(
S = struct @align(16) {
  a:array<mat2x4<f32>, 2> @offset(0)
  b:vec4<f32> @offset(64)
  c:f32 @offset(80)
}

$B1: {  # root
  %s:ptr<__out, array<S, 2>, read_write> = var undef @location(1)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<__out, f32, read_write> = access %s, 1u, 2u
    store %3, 1.0f
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  a:array<mat2x4<f32>, 2> @offset(0)
  b:vec4<f32> @offset(64)
  c:f32 @offset(80)
}

tint_symbol_20 = struct @align(16) {
  tint_symbol_3:vec4<f32> @offset(0), @location(1)
  tint_symbol_4:vec4<f32> @offset(16), @location(2)
  tint_symbol_6:vec4<f32> @offset(32), @location(3)
  tint_symbol_7:vec4<f32> @offset(48), @location(4)
  tint_symbol_8:vec4<f32> @offset(64), @location(5)
  tint_symbol_9:f32 @offset(80), @location(6)
  tint_symbol_13:vec4<f32> @offset(96), @location(7)
  tint_symbol_14:vec4<f32> @offset(112), @location(8)
  tint_symbol_16:vec4<f32> @offset(128), @location(9)
  tint_symbol_17:vec4<f32> @offset(144), @location(10)
  tint_symbol_18:vec4<f32> @offset(160), @location(11)
  tint_symbol_19:f32 @offset(176), @location(12)
}

$B1: {  # root
  %s:ptr<private, array<S, 2>, read_write> = var undef
}

%foo_inner = func():void {
  $B2: {
    %3:ptr<private, f32, read_write> = access %s, 1u, 2u
    store %3, 1.0f
    ret
  }
}
%foo = @fragment func():tint_symbol_20 {
  $B3: {
    %5:void = call %foo_inner
    %6:array<S, 2> = load %s
    %7:S = access %6, 0u
    %8:array<mat2x4<f32>, 2> = access %7, 0u
    %9:mat2x4<f32> = access %8, 0u
    %10:vec4<f32> = access %9, 0u
    %11:vec4<f32> = access %9, 1u
    %12:mat2x4<f32> = access %8, 1u
    %13:vec4<f32> = access %12, 0u
    %14:vec4<f32> = access %12, 1u
    %15:vec4<f32> = access %7, 1u
    %16:f32 = access %7, 2u
    %17:S = access %6, 1u
    %18:array<mat2x4<f32>, 2> = access %17, 0u
    %19:mat2x4<f32> = access %18, 0u
    %20:vec4<f32> = access %19, 0u
    %21:vec4<f32> = access %19, 1u
    %22:mat2x4<f32> = access %18, 1u
    %23:vec4<f32> = access %22, 0u
    %24:vec4<f32> = access %22, 1u
    %25:vec4<f32> = access %17, 1u
    %26:f32 = access %17, 2u
    %27:tint_symbol_20 = construct %10, %11, %13, %14, %15, %16, %20, %21, %23, %24, %25, %26
    ret %27
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::spirv::reader::lower

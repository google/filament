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
    {
        core::IOAttributes attributes;
        attributes.builtin = core::BuiltinValue::kFrontFacing;
        front_facing->SetAttributes(std::move(attributes));
    }
    auto* position = b.Var("position", ty.ptr(core::AddressSpace::kIn, ty.vec4<f32>()));
    {
        core::IOAttributes attributes;
        attributes.builtin = core::BuiltinValue::kPosition;
        attributes.invariant = true;
        position->SetAttributes(std::move(attributes));
    }
    auto* color1 = b.Var("color1", ty.ptr(core::AddressSpace::kIn, ty.f32()));
    {
        core::IOAttributes attributes;
        attributes.location = 0;
        color1->SetAttributes(std::move(attributes));
    }
    auto* color2 = b.Var("color2", ty.ptr(core::AddressSpace::kIn, ty.f32()));
    {
        core::IOAttributes attributes;
        attributes.location = 1;
        attributes.interpolation = core::Interpolation{core::InterpolationType::kLinear,
                                                       core::InterpolationSampling::kSample};
        color2->SetAttributes(std::move(attributes));
    }
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
  %front_facing:ptr<__in, bool, read> = var @builtin(front_facing)
  %position:ptr<__in, vec4<f32>, read> = var @invariant @builtin(position)
  %color1:ptr<__in, f32, read> = var @location(0)
  %color2:ptr<__in, f32, read> = var @location(1) @interpolate(linear, sample)
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
    {
        core::IOAttributes attributes;
        attributes.builtin = core::BuiltinValue::kFrontFacing;
        front_facing->SetAttributes(std::move(attributes));
    }
    auto* position = b.Var("position", ty.ptr(core::AddressSpace::kIn, ty.vec4<f32>()));
    {
        core::IOAttributes attributes;
        attributes.builtin = core::BuiltinValue::kPosition;
        attributes.invariant = true;
        position->SetAttributes(std::move(attributes));
    }
    auto* color1 = b.Var("color1", ty.ptr(core::AddressSpace::kIn, ty.f32()));
    {
        core::IOAttributes attributes;
        attributes.location = 0;
        color1->SetAttributes(std::move(attributes));
    }
    auto* color2 = b.Var("color2", ty.ptr(core::AddressSpace::kIn, ty.f32()));
    {
        core::IOAttributes attributes;
        attributes.location = 1;
        attributes.interpolation = core::Interpolation{core::InterpolationType::kLinear,
                                                       core::InterpolationSampling::kSample};
        color2->SetAttributes(std::move(attributes));
    }
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
  %front_facing:ptr<__in, bool, read> = var @builtin(front_facing)
  %position:ptr<__in, vec4<f32>, read> = var @invariant @builtin(position)
  %color1:ptr<__in, f32, read> = var @location(0)
  %color2:ptr<__in, f32, read> = var @location(1) @interpolate(linear, sample)
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

TEST_F(SpirvReader_ShaderIOTest, Inputs_UsedEntryPointAndHelper) {
    auto* gid = b.Var("gid", ty.ptr(core::AddressSpace::kIn, ty.vec3<u32>()));
    {
        core::IOAttributes attributes;
        attributes.builtin = core::BuiltinValue::kGlobalInvocationId;
        gid->SetAttributes(std::move(attributes));
    }
    auto* lid = b.Var("lid", ty.ptr(core::AddressSpace::kIn, ty.vec3<u32>()));
    {
        core::IOAttributes attributes;
        attributes.builtin = core::BuiltinValue::kLocalInvocationId;
        lid->SetAttributes(std::move(attributes));
    }
    auto* group_id = b.Var("group_id", ty.ptr(core::AddressSpace::kIn, ty.vec3<u32>()));
    {
        core::IOAttributes attributes;
        attributes.builtin = core::BuiltinValue::kWorkgroupId;
        group_id->SetAttributes(std::move(attributes));
    }
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
  %gid:ptr<__in, vec3<u32>, read> = var @builtin(global_invocation_id)
  %lid:ptr<__in, vec3<u32>, read> = var @builtin(local_invocation_id)
  %group_id:ptr<__in, vec3<u32>, read> = var @builtin(workgroup_id)
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
    {
        core::IOAttributes attributes;
        attributes.builtin = core::BuiltinValue::kGlobalInvocationId;
        gid->SetAttributes(std::move(attributes));
    }
    auto* lid = b.Var("lid", ty.ptr(core::AddressSpace::kIn, ty.vec3<u32>()));
    {
        core::IOAttributes attributes;
        attributes.builtin = core::BuiltinValue::kLocalInvocationId;
        lid->SetAttributes(std::move(attributes));
    }
    auto* group_id = b.Var("group_id", ty.ptr(core::AddressSpace::kIn, ty.vec3<u32>()));
    {
        core::IOAttributes attributes;
        attributes.builtin = core::BuiltinValue::kWorkgroupId;
        group_id->SetAttributes(std::move(attributes));
    }
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
  %gid:ptr<__in, vec3<u32>, read> = var @builtin(global_invocation_id)
  %lid:ptr<__in, vec3<u32>, read> = var @builtin(local_invocation_id)
  %group_id:ptr<__in, vec3<u32>, read> = var @builtin(workgroup_id)
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
    {
        core::IOAttributes attributes;
        attributes.builtin = core::BuiltinValue::kGlobalInvocationId;
        gid->SetAttributes(std::move(attributes));
    }
    auto* lid = b.Var("lid", ty.ptr(core::AddressSpace::kIn, ty.vec3<u32>()));
    {
        core::IOAttributes attributes;
        attributes.builtin = core::BuiltinValue::kLocalInvocationId;
        lid->SetAttributes(std::move(attributes));
    }
    auto* group_id = b.Var("group_id", ty.ptr(core::AddressSpace::kIn, ty.vec3<u32>()));
    {
        core::IOAttributes attributes;
        attributes.builtin = core::BuiltinValue::kWorkgroupId;
        group_id->SetAttributes(std::move(attributes));
    }
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
  %gid:ptr<__in, vec3<u32>, read> = var @builtin(global_invocation_id)
  %lid:ptr<__in, vec3<u32>, read> = var @builtin(local_invocation_id)
  %group_id:ptr<__in, vec3<u32>, read> = var @builtin(workgroup_id)
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
    {
        core::IOAttributes attributes;
        attributes.builtin = core::BuiltinValue::kLocalInvocationId;
        lid->SetAttributes(std::move(attributes));
    }
    mod.root_block->Append(lid);

    auto* ep = b.ComputeFunction("foo");
    b.Append(ep->Block(), [&] {
        b.LoadVectorElement(lid, 2_u);
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %lid:ptr<__in, vec3<u32>, read> = var @builtin(local_invocation_id)
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
  %colors:ptr<__in, Colors, read> = var
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
  color1:vec4<f32> @offset(0), @location(1)
  color2:vec4<f32> @offset(16), @location(2), @interpolate(linear, centroid)
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
%main = @fragment func(%colors_1:Colors):void {  # %colors_1: 'colors'
  $B2: {
    %9:void = call %foo, %colors_1
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, Inputs_Struct_LocationOnVariable) {
    auto* colors_str =
        ty.Struct(mod.symbols.New("Colors"),
                  Vector{
                      core::type::Manager::StructMemberDesc{
                          mod.symbols.New("color1"),
                          ty.vec4<f32>(),
                      },
                      core::type::Manager::StructMemberDesc{
                          mod.symbols.New("color2"),
                          ty.vec4<f32>(),
                          core::IOAttributes{
                              /* location */ std::nullopt,
                              /* index */ std::nullopt,
                              /* color */ std::nullopt,
                              /* builtin */ std::nullopt,
                              /* interpolation */
                              core::Interpolation{core::InterpolationType::kPerspective,
                                                  core::InterpolationSampling::kCentroid},
                              /* invariant */ false,
                          },
                      },
                  });
    auto* colors = b.Var("colors", ty.ptr(core::AddressSpace::kIn, colors_str));
    {
        core::IOAttributes attributes;
        attributes.location = 1u;
        colors->SetAttributes(attributes);
    }
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
  %colors:ptr<__in, Colors, read> = var @location(1)
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
  color1:vec4<f32> @offset(0), @location(1)
  color2:vec4<f32> @offset(16), @location(2), @interpolate(perspective, centroid)
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
%main = @fragment func(%colors_1:Colors):void {  # %colors_1: 'colors'
  $B2: {
    %9:void = call %foo, %colors_1
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
    {
        core::IOAttributes attributes;
        attributes.interpolation = core::Interpolation{core::InterpolationType::kPerspective,
                                                       core::InterpolationSampling::kCentroid};
        colors->SetAttributes(attributes);
    }
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
  %colors:ptr<__in, Colors, read> = var @interpolate(perspective, centroid)
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
  color1:vec4<f32> @offset(0), @location(1), @interpolate(perspective, centroid)
  color2:vec4<f32> @offset(16), @location(2), @interpolate(linear, sample)
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
%main = @fragment func(%colors_1:Colors):void {  # %colors_1: 'colors'
  $B2: {
    %9:void = call %foo, %colors_1
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
  %colors:ptr<__in, Colors, read> = var
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
  color1:vec4<f32> @offset(0), @location(1)
  color2:vec4<f32> @offset(16), @location(2), @interpolate(linear, centroid)
}

%foo = func(%colors:Colors):void {
  $B1: {
    %3:vec4<f32> = access %colors, 0u
    %4:f32 = access %colors, 1u, 2u
    %5:vec4<f32> = mul %3, %4
    ret
  }
}
%main = @fragment func(%colors_1:Colors):void {  # %colors_1: 'colors'
  $B2: {
    %8:void = call %foo, %colors_1
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_ShaderIOTest, SingleOutput_Builtin) {
    auto* position = b.Var("position", ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>()));
    {
        core::IOAttributes attributes;
        attributes.builtin = core::BuiltinValue::kPosition;
        position->SetAttributes(std::move(attributes));
    }
    mod.root_block->Append(position);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {  //
        b.Store(position, b.Splat<vec4<f32>>(1_f));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %position:ptr<__out, vec4<f32>, read_write> = var @builtin(position)
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
  %position:ptr<private, vec4<f32>, read_write> = var
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
    {
        core::IOAttributes attributes;
        attributes.builtin = core::BuiltinValue::kPosition;
        attributes.invariant = true;
        position->SetAttributes(std::move(attributes));
    }
    mod.root_block->Append(position);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {  //
        b.Store(position, b.Splat<vec4<f32>>(1_f));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %position:ptr<__out, vec4<f32>, read_write> = var @invariant @builtin(position)
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
  %position:ptr<private, vec4<f32>, read_write> = var
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
    {
        core::IOAttributes attributes;
        attributes.location = 1u;
        color->SetAttributes(std::move(attributes));
    }
    mod.root_block->Append(color);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(ep->Block(), [&] {  //
        b.Store(color, b.Splat<vec4<f32>>(1_f));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %color:ptr<__out, vec4<f32>, read_write> = var @location(1)
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
  %color:ptr<private, vec4<f32>, read_write> = var
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
    {
        core::IOAttributes attributes;
        attributes.location = 1u;
        attributes.interpolation = core::Interpolation{core::InterpolationType::kPerspective,
                                                       core::InterpolationSampling::kCentroid};
        color->SetAttributes(std::move(attributes));
    }
    mod.root_block->Append(color);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(ep->Block(), [&] {  //
        b.Store(color, b.Splat<vec4<f32>>(1_f));
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %color:ptr<__out, vec4<f32>, read_write> = var @location(1) @interpolate(perspective, centroid)
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
  %color:ptr<private, vec4<f32>, read_write> = var
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
    {
        core::IOAttributes attributes;
        attributes.builtin = core::BuiltinValue::kPosition;
        attributes.invariant = true;
        position->SetAttributes(std::move(attributes));
    }
    auto* color1 = b.Var("color1", ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>()));
    {
        core::IOAttributes attributes;
        attributes.location = 1u;
        color1->SetAttributes(std::move(attributes));
    }
    auto* color2 = b.Var("color2", ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>()));
    {
        core::IOAttributes attributes;
        attributes.location = 1u;
        attributes.interpolation = core::Interpolation{core::InterpolationType::kPerspective,
                                                       core::InterpolationSampling::kCentroid};
        color2->SetAttributes(std::move(attributes));
    }
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
  %position:ptr<__out, vec4<f32>, read_write> = var @invariant @builtin(position)
  %color1:ptr<__out, vec4<f32>, read_write> = var @location(1)
  %color2:ptr<__out, vec4<f32>, read_write> = var @location(1) @interpolate(perspective, centroid)
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
  %position:ptr<private, vec4<f32>, read_write> = var
  %color1:ptr<private, vec4<f32>, read_write> = var
  %color2:ptr<private, vec4<f32>, read_write> = var
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
  %builtins:ptr<__out, Builtins, read_write> = var
  %colors:ptr<__out, Colors, read_write> = var
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
  %builtins:ptr<private, Builtins, read_write> = var
  %colors:ptr<private, Colors, read_write> = var
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
    auto* colors_str =
        ty.Struct(mod.symbols.New("Colors"),
                  Vector{
                      core::type::Manager::StructMemberDesc{
                          mod.symbols.New("color1"),
                          ty.vec4<f32>(),
                      },
                      core::type::Manager::StructMemberDesc{
                          mod.symbols.New("color2"),
                          ty.vec4<f32>(),
                          core::IOAttributes{
                              /* location */ std::nullopt,
                              /* index */ std::nullopt,
                              /* color */ std::nullopt,
                              /* builtin */ std::nullopt,
                              /* interpolation */
                              core::Interpolation{core::InterpolationType::kPerspective,
                                                  core::InterpolationSampling::kCentroid},
                              /* invariant */ false,
                          },
                      },
                  });

    auto* builtins = b.Var("builtins", ty.ptr(core::AddressSpace::kOut, builtin_str));
    auto* colors = b.Var("colors", ty.ptr(core::AddressSpace::kOut, colors_str));
    {
        core::IOAttributes attributes;
        attributes.location = 1u;
        colors->SetAttributes(attributes);
    }
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
  %builtins:ptr<__out, Builtins, read_write> = var
  %colors:ptr<__out, Colors, read_write> = var @location(1)
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
  %builtins:ptr<private, Builtins, read_write> = var
  %colors:ptr<private, Colors, read_write> = var
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
    {
        core::IOAttributes attributes;
        attributes.interpolation = core::Interpolation{core::InterpolationType::kPerspective,
                                                       core::InterpolationSampling::kCentroid};
        colors->SetAttributes(attributes);
    }
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
  %builtins:ptr<__out, Builtins, read_write> = var
  %colors:ptr<__out, Colors, read_write> = var @interpolate(perspective, centroid)
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
  %builtins:ptr<private, Builtins, read_write> = var
  %colors:ptr<private, Colors, read_write> = var
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
    {
        core::IOAttributes attributes;
        attributes.builtin = core::BuiltinValue::kPosition;
        attributes.invariant = true;
        position->SetAttributes(std::move(attributes));
    }
    auto* color1 = b.Var("color1", ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>()));
    {
        core::IOAttributes attributes;
        attributes.location = 1u;
        color1->SetAttributes(std::move(attributes));
    }
    auto* color2 = b.Var("color2", ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>()));
    {
        core::IOAttributes attributes;
        attributes.location = 1u;
        attributes.interpolation = core::Interpolation{core::InterpolationType::kPerspective,
                                                       core::InterpolationSampling::kCentroid};
        color2->SetAttributes(std::move(attributes));
    }
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
  %position:ptr<__out, vec4<f32>, read_write> = var @invariant @builtin(position)
  %color1:ptr<__out, vec4<f32>, read_write> = var @location(1)
  %color2:ptr<__out, vec4<f32>, read_write> = var @location(1) @interpolate(perspective, centroid)
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
  %position:ptr<private, vec4<f32>, read_write> = var
  %color1:ptr<private, vec4<f32>, read_write> = var
  %color2:ptr<private, vec4<f32>, read_write> = var
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
    {
        core::IOAttributes attributes;
        attributes.location = 1u;
        color->SetAttributes(std::move(attributes));
    }
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
  %color:ptr<__out, vec4<f32>, read_write> = var @location(1)
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
  %color:ptr<private, vec4<f32>, read_write> = var
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
    {
        core::IOAttributes attributes;
        attributes.location = 1u;
        color->SetAttributes(std::move(attributes));
    }
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
  %color:ptr<__out, vec4<f32>, read_write> = var @location(1)
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
  %color:ptr<private, vec4<f32>, read_write> = var
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
    {
        core::IOAttributes attributes;
        attributes.builtin = core::BuiltinValue::kPosition;
        attributes.invariant = true;
        position->SetAttributes(std::move(attributes));
    }
    auto* color_in = b.Var("color_in", ty.ptr(core::AddressSpace::kIn, ty.vec4<f32>()));
    {
        core::IOAttributes attributes;
        attributes.location = 0;
        color_in->SetAttributes(std::move(attributes));
    }
    auto* color_out_1 = b.Var("color_out_1", ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>()));
    {
        core::IOAttributes attributes;
        attributes.location = 1;
        color_out_1->SetAttributes(std::move(attributes));
    }
    auto* color_out_2 = b.Var("color_out_2", ty.ptr(core::AddressSpace::kOut, ty.vec4<f32>()));
    {
        core::IOAttributes attributes;
        attributes.location = 2;
        color_out_2->SetAttributes(std::move(attributes));
    }
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
  %position:ptr<__in, vec4<f32>, read> = var @invariant @builtin(position)
  %color_in:ptr<__in, vec4<f32>, read> = var @location(0)
  %color_out_1:ptr<__out, vec4<f32>, read_write> = var @location(1)
  %color_out_2:ptr<__out, vec4<f32>, read_write> = var @location(2)
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
  %color_out_1:ptr<private, vec4<f32>, read_write> = var
  %color_out_2:ptr<private, vec4<f32>, read_write> = var
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

// Test that a sample mask array is converted to a scalar u32 for the entry point.
TEST_F(SpirvReader_ShaderIOTest, SampleMask) {
    auto* arr = ty.array<u32, 1>();
    auto* mask_in = b.Var("mask_in", ty.ptr(core::AddressSpace::kIn, arr));
    {
        core::IOAttributes attributes;
        attributes.builtin = core::BuiltinValue::kSampleMask;
        mask_in->SetAttributes(std::move(attributes));
    }
    auto* mask_out = b.Var("mask_out", ty.ptr(core::AddressSpace::kOut, arr));
    {
        core::IOAttributes attributes;
        attributes.builtin = core::BuiltinValue::kSampleMask;
        mask_out->SetAttributes(std::move(attributes));
    }
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
  %mask_in:ptr<__in, array<u32, 1>, read> = var @builtin(sample_mask)
  %mask_out:ptr<__out, array<u32, 1>, read_write> = var @builtin(sample_mask)
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
  %mask_out:ptr<private, array<u32, 1>, read_write> = var
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

TEST_F(SpirvReader_ShaderIOTest, PointSize) {
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
  %builtins:ptr<__out, Builtins, read_write> = var
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
  %builtins:ptr<private, Builtins, read_write> = var
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
  %builtins:ptr<__out, Builtins, read_write> = var
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
  %builtins:ptr<private, Builtins, read_write> = var
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
  %builtins:ptr<__out, Builtins, read_write> = var
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
  %builtins:ptr<private, Builtins, read_write> = var
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
}  // namespace
}  // namespace tint::spirv::reader::lower

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

#include "src/tint/lang/msl/writer/raise/convert_print_to_log.h"

#include "gtest/gtest.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::msl::writer::raise {
namespace {

class MslWriter_ConvertPrintToLogTest : public core::ir::transform::TransformTest {
    void SetUp() override {
        capabilities.Add(core::ir::Capability::kAllow8BitIntegers);
        capabilities.Add(core::ir::Capability::kAllowNonCoreTypes);
    }
};

TEST_F(MslWriter_ConvertPrintToLogTest, NoPrint) {
    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] { b.Return(f); });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ConvertPrintToLog);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ConvertPrintToLogTest, AllFormats) {
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Call<void>(core::BuiltinFn::kPrint, true);
        b.Call<void>(core::BuiltinFn::kPrint, 42_i);
        b.Call<void>(core::BuiltinFn::kPrint, 42_u);
        b.Call<void>(core::BuiltinFn::kPrint, 42_f);
        b.Call<void>(core::BuiltinFn::kPrint, 42_h);

        b.Call<void>(core::BuiltinFn::kPrint, b.Composite<vec2<bool>>(true, false));
        b.Call<void>(core::BuiltinFn::kPrint, b.Composite<vec3<i32>>(1_i, 2_i, 3_i));
        b.Call<void>(core::BuiltinFn::kPrint, b.Composite<vec4<u32>>(3_u, 4_u, 5_u, 6_u));
        b.Call<void>(core::BuiltinFn::kPrint, b.Composite<vec4<f32>>(1_f, 2_f, 3_f, 4_f));
        b.Call<void>(core::BuiltinFn::kPrint, b.Composite<vec4<f16>>(1_h, 2_h, 3_h, 4_h));
        b.Return(func);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:void = print true
    %3:void = print 42i
    %4:void = print 42u
    %5:void = print 42.0f
    %6:void = print 42.0h
    %7:void = print vec2<bool>(true, false)
    %8:void = print vec3<i32>(1i, 2i, 3i)
    %9:void = print vec4<u32>(3u, 4u, 5u, 6u)
    %10:void = print vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f)
    %11:void = print vec4<f16>(1.0h, 2.0h, 3.0h, 4.0h)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tint_print_invocation_id:ptr<private, vec3<u32>, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func(%tint_symbol:vec3<u32> [@global_invocation_id]):void {
  $B2: {
    store %tint_print_invocation_id, %tint_symbol
    %4:vec3<u32> = load %tint_print_invocation_id
    %5:i32 = convert true
    %6:u32 = swizzle %4, x
    %7:u32 = swizzle %4, y
    %8:u32 = swizzle %4, z
    %9:void = msl.os_log "[ comp foo:L0 global_invocation_id(%u, %u, %u) ] %i", %6, %7, %8, %5
    %10:vec3<u32> = load %tint_print_invocation_id
    %11:u32 = swizzle %10, x
    %12:u32 = swizzle %10, y
    %13:u32 = swizzle %10, z
    %14:void = msl.os_log "[ comp foo:L0 global_invocation_id(%u, %u, %u) ] %i", %11, %12, %13, 42i
    %15:vec3<u32> = load %tint_print_invocation_id
    %16:u32 = swizzle %15, x
    %17:u32 = swizzle %15, y
    %18:u32 = swizzle %15, z
    %19:void = msl.os_log "[ comp foo:L0 global_invocation_id(%u, %u, %u) ] %u", %16, %17, %18, 42u
    %20:vec3<u32> = load %tint_print_invocation_id
    %21:u32 = swizzle %20, x
    %22:u32 = swizzle %20, y
    %23:u32 = swizzle %20, z
    %24:void = msl.os_log "[ comp foo:L0 global_invocation_id(%u, %u, %u) ] %f", %21, %22, %23, 42.0f
    %25:vec3<u32> = load %tint_print_invocation_id
    %26:u32 = swizzle %25, x
    %27:u32 = swizzle %25, y
    %28:u32 = swizzle %25, z
    %29:void = msl.os_log "[ comp foo:L0 global_invocation_id(%u, %u, %u) ] %f", %26, %27, %28, 42.0h
    %30:vec3<u32> = load %tint_print_invocation_id
    %31:vec2<i32> = convert vec2<bool>(true, false)
    %32:u32 = swizzle %30, x
    %33:u32 = swizzle %30, y
    %34:u32 = swizzle %30, z
    %35:void = msl.os_log "[ comp foo:L0 global_invocation_id(%u, %u, %u) ] %v2hli", %32, %33, %34, %31
    %36:vec3<u32> = load %tint_print_invocation_id
    %37:u32 = swizzle %36, x
    %38:u32 = swizzle %36, y
    %39:u32 = swizzle %36, z
    %40:void = msl.os_log "[ comp foo:L0 global_invocation_id(%u, %u, %u) ] %v3hli", %37, %38, %39, vec3<i32>(1i, 2i, 3i)
    %41:vec3<u32> = load %tint_print_invocation_id
    %42:u32 = swizzle %41, x
    %43:u32 = swizzle %41, y
    %44:u32 = swizzle %41, z
    %45:void = msl.os_log "[ comp foo:L0 global_invocation_id(%u, %u, %u) ] %v4hlu", %42, %43, %44, vec4<u32>(3u, 4u, 5u, 6u)
    %46:vec3<u32> = load %tint_print_invocation_id
    %47:u32 = swizzle %46, x
    %48:u32 = swizzle %46, y
    %49:u32 = swizzle %46, z
    %50:void = msl.os_log "[ comp foo:L0 global_invocation_id(%u, %u, %u) ] %v4hlf", %47, %48, %49, vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f)
    %51:vec3<u32> = load %tint_print_invocation_id
    %52:u32 = swizzle %51, x
    %53:u32 = swizzle %51, y
    %54:u32 = swizzle %51, z
    %55:void = msl.os_log "[ comp foo:L0 global_invocation_id(%u, %u, %u) ] %v4hf", %52, %53, %54, vec4<f16>(1.0h, 2.0h, 3.0h, 4.0h)
    ret
  }
}
)";

    Run(ConvertPrintToLog);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ConvertPrintToLogTest, Compute) {
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Call<void>(core::BuiltinFn::kPrint, 42_u);
        b.Return(func);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:void = print 42u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tint_print_invocation_id:ptr<private, vec3<u32>, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func(%tint_symbol:vec3<u32> [@global_invocation_id]):void {
  $B2: {
    store %tint_print_invocation_id, %tint_symbol
    %4:vec3<u32> = load %tint_print_invocation_id
    %5:u32 = swizzle %4, x
    %6:u32 = swizzle %4, y
    %7:u32 = swizzle %4, z
    %8:void = msl.os_log "[ comp foo:L0 global_invocation_id(%u, %u, %u) ] %u", %5, %6, %7, 42u
    ret
  }
}
)";

    Run(ConvertPrintToLog);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ConvertPrintToLogTest, Compute_ExistingBuiltin) {
    auto* func = b.ComputeFunction("foo");
    auto* id = b.FunctionParam("id", ty.vec3<u32>());
    id->SetBuiltin(core::BuiltinValue::kGlobalInvocationId);
    func->AppendParam(id);
    b.Append(func->Block(), [&] {
        b.Call<void>(core::BuiltinFn::kPrint, 42_u);
        b.Return(func);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%id:vec3<u32> [@global_invocation_id]):void {
  $B1: {
    %3:void = print 42u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tint_print_invocation_id:ptr<private, vec3<u32>, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func(%id:vec3<u32> [@global_invocation_id]):void {
  $B2: {
    store %tint_print_invocation_id, %id
    %4:vec3<u32> = load %tint_print_invocation_id
    %5:u32 = swizzle %4, x
    %6:u32 = swizzle %4, y
    %7:u32 = swizzle %4, z
    %8:void = msl.os_log "[ comp foo:L0 global_invocation_id(%u, %u, %u) ] %u", %5, %6, %7, 42u
    ret
  }
}
)";

    Run(ConvertPrintToLog);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ConvertPrintToLogTest, Fragment) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Call<void>(core::BuiltinFn::kPrint, 42_u);
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:void = print 42u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tint_print_invocation_id:ptr<private, vec3<f32>, read_write> = var undef
}

%foo = @fragment func(%tint_symbol:vec4<f32> [@position]):void {
  $B2: {
    %4:vec3<f32> = swizzle %tint_symbol, xyz
    store %tint_print_invocation_id, %4
    %5:vec3<f32> = load %tint_print_invocation_id
    %6:f32 = swizzle %5, x
    %7:f32 = swizzle %5, y
    %8:f32 = swizzle %5, z
    %9:void = msl.os_log "[ frag foo:L0 position(%f, %f, %f) ] %u", %6, %7, %8, 42u
    ret
  }
}
)";

    Run(ConvertPrintToLog);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ConvertPrintToLogTest, Fragment_ExistingBuiltin) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    auto* position = b.FunctionParam("position", ty.vec4<f32>());
    position->SetBuiltin(core::BuiltinValue::kPosition);
    func->AppendParam(position);
    b.Append(func->Block(), [&] {
        b.Call<void>(core::BuiltinFn::kPrint, 42_u);
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func(%position:vec4<f32> [@position]):void {
  $B1: {
    %3:void = print 42u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tint_print_invocation_id:ptr<private, vec3<f32>, read_write> = var undef
}

%foo = @fragment func(%position:vec4<f32> [@position]):void {
  $B2: {
    %4:vec3<f32> = swizzle %position, xyz
    store %tint_print_invocation_id, %4
    %5:vec3<f32> = load %tint_print_invocation_id
    %6:f32 = swizzle %5, x
    %7:f32 = swizzle %5, y
    %8:f32 = swizzle %5, z
    %9:void = msl.os_log "[ frag foo:L0 position(%f, %f, %f) ] %u", %6, %7, %8, 42u
    ret
  }
}
)";

    Run(ConvertPrintToLog);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ConvertPrintToLogTest, Vertex) {
    auto* func = b.Function("foo", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    func->SetReturnBuiltin(core::BuiltinValue::kPosition);
    b.Append(func->Block(), [&] {
        b.Call<void>(core::BuiltinFn::kPrint, 42_u);
        b.Return(func, b.Zero<vec4<f32>>());
    });

    auto* src = R"(
%foo = @vertex func():vec4<f32> [@position] {
  $B1: {
    %2:void = print 42u
    ret vec4<f32>(0.0f)
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tint_print_invocation_id:ptr<private, vec2<u32>, read_write> = var undef
}

%foo = @vertex func(%tint_symbol:u32 [@instance_index], %tint_symbol_1:u32 [@vertex_index]):vec4<f32> [@position] {
  $B2: {
    %5:vec2<u32> = construct %tint_symbol, %tint_symbol_1
    store %tint_print_invocation_id, %5
    %6:vec2<u32> = load %tint_print_invocation_id
    %7:u32 = swizzle %6, x
    %8:u32 = swizzle %6, y
    %9:void = msl.os_log "[ vert foo:L0 instance=%u, vertex=%u ] %u", %7, %8, 42u
    ret vec4<f32>(0.0f)
  }
}
)";

    Run(ConvertPrintToLog);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ConvertPrintToLogTest, Vertex_ExistingBuiltin) {
    auto* func = b.Function("foo", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    func->SetReturnBuiltin(core::BuiltinValue::kPosition);

    auto* instance = b.FunctionParam("instance", ty.u32());
    instance->SetBuiltin(core::BuiltinValue::kInstanceIndex);
    func->AppendParam(instance);

    auto* vertex = b.FunctionParam("vertex", ty.u32());
    vertex->SetBuiltin(core::BuiltinValue::kVertexIndex);
    func->AppendParam(vertex);

    b.Append(func->Block(), [&] {
        b.Call<void>(core::BuiltinFn::kPrint, 42_u);
        b.Return(func, b.Zero<vec4<f32>>());
    });

    auto* src = R"(
%foo = @vertex func(%instance:u32 [@instance_index], %vertex:u32 [@vertex_index]):vec4<f32> [@position] {
  $B1: {
    %4:void = print 42u
    ret vec4<f32>(0.0f)
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tint_print_invocation_id:ptr<private, vec2<u32>, read_write> = var undef
}

%foo = @vertex func(%instance:u32 [@instance_index], %vertex:u32 [@vertex_index]):vec4<f32> [@position] {
  $B2: {
    %5:vec2<u32> = construct %instance, %vertex
    store %tint_print_invocation_id, %5
    %6:vec2<u32> = load %tint_print_invocation_id
    %7:u32 = swizzle %6, x
    %8:u32 = swizzle %6, y
    %9:void = msl.os_log "[ vert foo:L0 instance=%u, vertex=%u ] %u", %7, %8, 42u
    ret vec4<f32>(0.0f)
  }
}
)";

    Run(ConvertPrintToLog);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ConvertPrintToLogTest, HelperFunction) {
    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {
        b.Call<void>(core::BuiltinFn::kPrint, 42_u);
        b.Return(foo);
    });

    auto* ep = b.ComputeFunction("ep");
    b.Append(ep->Block(), [&] {
        b.Call<void>(foo);
        b.Return(ep);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    %2:void = print 42u
    ret
  }
}
%ep = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:void = call %foo
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tint_print_invocation_id:ptr<private, vec3<u32>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:vec3<u32> = load %tint_print_invocation_id
    %4:u32 = swizzle %3, x
    %5:u32 = swizzle %3, y
    %6:u32 = swizzle %3, z
    %7:void = msl.os_log "[ comp ep:L0 global_invocation_id(%u, %u, %u) ] %u", %4, %5, %6, 42u
    ret
  }
}
%ep = @compute @workgroup_size(1u, 1u, 1u) func(%tint_symbol:vec3<u32> [@global_invocation_id]):void {
  $B3: {
    store %tint_print_invocation_id, %tint_symbol
    %10:void = call %foo
    ret
  }
}
)";

    Run(ConvertPrintToLog);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::msl::writer::raise

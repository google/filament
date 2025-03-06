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

#include "src/tint/lang/hlsl/writer/raise/pixel_local.h"

#include <gtest/gtest.h>
#include <tuple>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/clone_context.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer::raise {
namespace {

constexpr auto pixel_local = core::AddressSpace::kPixelLocal;

struct HlslWriterPixelLocalTest : core::ir::transform::TransformTest {
    struct Result {
        core::ir::Function* func;
        core::ir::Var* pl;
    };
    Result OneArgFunc(bool multiple_builtins = false) {
        auto* pixel_local_struct_ty =
            ty.Struct(mod.symbols.New("PixelLocal"), {
                                                         {mod.symbols.New("a"), ty.u32()},
                                                     });
        auto* pl = b.Var("pl", ty.ptr<pixel_local>(pixel_local_struct_ty));
        mod.root_block->Append(pl);

        Vector<core::type::Manager::StructMemberDesc, 3> members;
        core::IOAttributes attrs;
        attrs.builtin = core::BuiltinValue::kPosition;
        members.Emplace(mod.symbols.New("pos"), ty.vec4<f32>(), attrs);
        if (multiple_builtins) {
            attrs.builtin = core::BuiltinValue::kFrontFacing;
            members.Emplace(mod.symbols.New("front_facing"), ty.bool_(), attrs);
            attrs.builtin = core::BuiltinValue::kSampleIndex;
            members.Emplace(mod.symbols.New("sample_index"), ty.u32(), attrs);
        }
        auto* param_struct_ty = ty.Struct(mod.symbols.New("params"), members);

        auto* func =
            b.Function("main", ty.vec4<f32>(), core::ir::Function::PipelineStage::kFragment);
        func->SetReturnLocation(0_u);
        func->SetParams({b.FunctionParam(param_struct_ty)});
        return {func, pl};
    }
    PixelLocalConfig OneArgConfig() {
        PixelLocalConfig config;
        config.options.attachments[0] = {10, PixelLocalAttachment::TexelFormat::kR32Uint};
        config.options.group_index = 7;
        return config;
    }

    Result ThreeArgFunc() {
        auto* pixel_local_struct_ty =
            ty.Struct(mod.symbols.New("PixelLocal"), {{mod.symbols.New("a"), ty.u32()},
                                                      {mod.symbols.New("b"), ty.i32()},
                                                      {mod.symbols.New("c"), ty.f32()}});
        auto* pl = b.Var("pl", ty.ptr<pixel_local>(pixel_local_struct_ty));
        mod.root_block->Append(pl);

        core::IOAttributes attrs;
        attrs.builtin = core::BuiltinValue::kPosition;
        auto* param_struct_ty =
            ty.Struct(mod.symbols.New("params"), {{mod.symbols.New("pos"), ty.vec4<f32>(), attrs}});

        auto* func =
            b.Function("main", ty.vec4<f32>(), core::ir::Function::PipelineStage::kFragment);
        func->SetReturnLocation(0_u);
        func->SetParams({b.FunctionParam(param_struct_ty)});
        return {func, pl};
    }
    PixelLocalConfig ThreeArgConfig() {
        PixelLocalConfig config;
        config.options.attachments[0] = {10, PixelLocalAttachment::TexelFormat::kR32Uint};
        config.options.attachments[1] = {12, PixelLocalAttachment::TexelFormat::kR32Sint};
        config.options.attachments[2] = {14, PixelLocalAttachment::TexelFormat::kR32Float};
        config.options.group_index = 7;
        return config;
    }
};

TEST_F(HlslWriterPixelLocalTest, Unused) {
    auto r = OneArgFunc();
    b.Append(r.func->Block(), [&] {  //
        b.Return(r.func, b.Construct<vec4<f32>>(1_f, 0_f, 0_f, 1_f));
    });

    auto* src = R"(
PixelLocal = struct @align(4) {
  a:u32 @offset(0)
}

params = struct @align(16) {
  pos:vec4<f32> @offset(0), @builtin(position)
}

$B1: {  # root
  %pl:ptr<pixel_local, PixelLocal, read_write> = var
}

%main = @fragment func(%3:params):vec4<f32> [@location(0)] {
  $B2: {
    %4:vec4<f32> = construct 1.0f, 0.0f, 0.0f, 1.0f
    ret %4
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    auto config = OneArgConfig();
    Run(PixelLocal, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterPixelLocalTest, UsedInEntry) {
    auto r = OneArgFunc();
    b.Append(r.func->Block(), [&] {
        auto* access = b.Access(ty.ptr<pixel_local>(ty.u32()), r.pl, 0_u);
        auto* add = b.Add<u32>(b.Load(access), 42_u);
        b.Store(access, add);
        b.Return(r.func, b.Construct<vec4<f32>>(1_f, 0_f, 0_f, 1_f));
    });

    auto* src = R"(
PixelLocal = struct @align(4) {
  a:u32 @offset(0)
}

params = struct @align(16) {
  pos:vec4<f32> @offset(0), @builtin(position)
}

$B1: {  # root
  %pl:ptr<pixel_local, PixelLocal, read_write> = var
}

%main = @fragment func(%3:params):vec4<f32> [@location(0)] {
  $B2: {
    %4:ptr<pixel_local, u32, read_write> = access %pl, 0u
    %5:u32 = load %4
    %6:u32 = add %5, 42u
    store %4, %6
    %7:vec4<f32> = construct 1.0f, 0.0f, 0.0f, 1.0f
    ret %7
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
PixelLocal = struct @align(4) {
  a:u32 @offset(0)
}

params = struct @align(16) {
  pos:vec4<f32> @offset(0), @builtin(position)
}

$B1: {  # root
  %pl:ptr<private, PixelLocal, read_write> = var
  %pixel_local_a:ptr<handle, hlsl.rasterizer_ordered_texture_2d<r32uint>, read> = var @binding_point(7, 10)
}

%main = @fragment func(%4:params):vec4<f32> [@location(0)] {
  $B2: {
    %5:vec4<f32> = access %4, 0u
    %6:vec2<f32> = swizzle %5, xy
    %7:vec2<u32> = convert %6
    %8:hlsl.rasterizer_ordered_texture_2d<r32uint> = load %pixel_local_a
    %9:vec4<u32> = %8.Load %7
    %10:u32 = swizzle %9, x
    %11:ptr<private, u32, read_write> = access %pl, 0u
    store %11, %10
    %12:ptr<private, u32, read_write> = access %pl, 0u
    %13:u32 = load %12
    %14:u32 = add %13, 42u
    store %12, %14
    %15:vec4<f32> = construct 1.0f, 0.0f, 0.0f, 1.0f
    %16:ptr<private, u32, read_write> = access %pl, 0u
    %17:u32 = load %16
    %18:vec4<u32> = construct %17
    %19:hlsl.rasterizer_ordered_texture_2d<r32uint> = load %pixel_local_a
    %20:void = hlsl.textureStore %19, %7, %18
    ret %15
  }
}
)";

    auto config = OneArgConfig();
    Run(PixelLocal, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterPixelLocalTest, UsedInNonEntry) {
    auto r = OneArgFunc();
    auto* func2 = b.Function("foo", ty.void_());
    b.Append(func2->Block(), [&] {
        auto* access = b.Access(ty.ptr<pixel_local>(ty.u32()), r.pl, 0_u);
        auto* add = b.Add<u32>(b.Load(access), 42_u);
        b.Store(access, add);
        b.Return(func2);
    });
    b.Append(r.func->Block(), [&] {
        b.Call(func2);
        b.Return(r.func, b.Construct<vec4<f32>>(1_f, 0_f, 0_f, 1_f));
    });

    auto* src = R"(
PixelLocal = struct @align(4) {
  a:u32 @offset(0)
}

params = struct @align(16) {
  pos:vec4<f32> @offset(0), @builtin(position)
}

$B1: {  # root
  %pl:ptr<pixel_local, PixelLocal, read_write> = var
}

%main = @fragment func(%3:params):vec4<f32> [@location(0)] {
  $B2: {
    %4:void = call %foo
    %6:vec4<f32> = construct 1.0f, 0.0f, 0.0f, 1.0f
    ret %6
  }
}
%foo = func():void {
  $B3: {
    %7:ptr<pixel_local, u32, read_write> = access %pl, 0u
    %8:u32 = load %7
    %9:u32 = add %8, 42u
    store %7, %9
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
PixelLocal = struct @align(4) {
  a:u32 @offset(0)
}

params = struct @align(16) {
  pos:vec4<f32> @offset(0), @builtin(position)
}

$B1: {  # root
  %pl:ptr<private, PixelLocal, read_write> = var
  %pixel_local_a:ptr<handle, hlsl.rasterizer_ordered_texture_2d<r32uint>, read> = var @binding_point(7, 10)
}

%main = @fragment func(%4:params):vec4<f32> [@location(0)] {
  $B2: {
    %5:vec4<f32> = access %4, 0u
    %6:vec2<f32> = swizzle %5, xy
    %7:vec2<u32> = convert %6
    %8:hlsl.rasterizer_ordered_texture_2d<r32uint> = load %pixel_local_a
    %9:vec4<u32> = %8.Load %7
    %10:u32 = swizzle %9, x
    %11:ptr<private, u32, read_write> = access %pl, 0u
    store %11, %10
    %12:void = call %foo
    %14:vec4<f32> = construct 1.0f, 0.0f, 0.0f, 1.0f
    %15:ptr<private, u32, read_write> = access %pl, 0u
    %16:u32 = load %15
    %17:vec4<u32> = construct %16
    %18:hlsl.rasterizer_ordered_texture_2d<r32uint> = load %pixel_local_a
    %19:void = hlsl.textureStore %18, %7, %17
    ret %14
  }
}
%foo = func():void {
  $B3: {
    %20:ptr<private, u32, read_write> = access %pl, 0u
    %21:u32 = load %20
    %22:u32 = add %21, 42u
    store %20, %22
    ret
  }
}
)";

    auto config = OneArgConfig();
    Run(PixelLocal, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterPixelLocalTest, UsedInNonEntryViaPointer) {
    auto r = OneArgFunc();
    auto* func2 = b.Function("foo", ty.void_());
    b.Append(func2->Block(), [&] {
        auto* access = b.Access(ty.ptr<pixel_local>(ty.u32()), r.pl, 0_u);
        auto* p = b.Let("p", access);
        auto* add = b.Add<u32>(b.Load(p), 42_u);
        b.Store(access, add);
        b.Return(func2);
    });
    b.Append(r.func->Block(), [&] {
        b.Call(func2);
        b.Return(r.func, b.Construct<vec4<f32>>(1_f, 0_f, 0_f, 1_f));
    });

    auto* src = R"(
PixelLocal = struct @align(4) {
  a:u32 @offset(0)
}

params = struct @align(16) {
  pos:vec4<f32> @offset(0), @builtin(position)
}

$B1: {  # root
  %pl:ptr<pixel_local, PixelLocal, read_write> = var
}

%main = @fragment func(%3:params):vec4<f32> [@location(0)] {
  $B2: {
    %4:void = call %foo
    %6:vec4<f32> = construct 1.0f, 0.0f, 0.0f, 1.0f
    ret %6
  }
}
%foo = func():void {
  $B3: {
    %7:ptr<pixel_local, u32, read_write> = access %pl, 0u
    %p:ptr<pixel_local, u32, read_write> = let %7
    %9:u32 = load %p
    %10:u32 = add %9, 42u
    store %7, %10
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
PixelLocal = struct @align(4) {
  a:u32 @offset(0)
}

params = struct @align(16) {
  pos:vec4<f32> @offset(0), @builtin(position)
}

$B1: {  # root
  %pl:ptr<private, PixelLocal, read_write> = var
  %pixel_local_a:ptr<handle, hlsl.rasterizer_ordered_texture_2d<r32uint>, read> = var @binding_point(7, 10)
}

%main = @fragment func(%4:params):vec4<f32> [@location(0)] {
  $B2: {
    %5:vec4<f32> = access %4, 0u
    %6:vec2<f32> = swizzle %5, xy
    %7:vec2<u32> = convert %6
    %8:hlsl.rasterizer_ordered_texture_2d<r32uint> = load %pixel_local_a
    %9:vec4<u32> = %8.Load %7
    %10:u32 = swizzle %9, x
    %11:ptr<private, u32, read_write> = access %pl, 0u
    store %11, %10
    %12:void = call %foo
    %14:vec4<f32> = construct 1.0f, 0.0f, 0.0f, 1.0f
    %15:ptr<private, u32, read_write> = access %pl, 0u
    %16:u32 = load %15
    %17:vec4<u32> = construct %16
    %18:hlsl.rasterizer_ordered_texture_2d<r32uint> = load %pixel_local_a
    %19:void = hlsl.textureStore %18, %7, %17
    ret %14
  }
}
%foo = func():void {
  $B3: {
    %20:ptr<private, u32, read_write> = access %pl, 0u
    %21:u32 = load %20
    %22:u32 = add %21, 42u
    store %20, %22
    ret
  }
}
)";

    auto config = OneArgConfig();
    Run(PixelLocal, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterPixelLocalTest, MultipleInputBuiltins) {
    auto r = OneArgFunc(/*multiple_builtins*/ true);
    b.Append(r.func->Block(), [&] {
        auto* access = b.Access(ty.ptr<pixel_local>(ty.u32()), r.pl, 0_u);
        auto* add = b.Add<u32>(b.Load(access), 42_u);
        b.Store(access, add);
        b.Return(r.func, b.Construct<vec4<f32>>(1_f, 0_f, 0_f, 1_f));
    });

    auto* src = R"(
PixelLocal = struct @align(4) {
  a:u32 @offset(0)
}

params = struct @align(16) {
  pos:vec4<f32> @offset(0), @builtin(position)
  front_facing:bool @offset(16), @builtin(front_facing)
  sample_index:u32 @offset(20), @builtin(sample_index)
}

$B1: {  # root
  %pl:ptr<pixel_local, PixelLocal, read_write> = var
}

%main = @fragment func(%3:params):vec4<f32> [@location(0)] {
  $B2: {
    %4:ptr<pixel_local, u32, read_write> = access %pl, 0u
    %5:u32 = load %4
    %6:u32 = add %5, 42u
    store %4, %6
    %7:vec4<f32> = construct 1.0f, 0.0f, 0.0f, 1.0f
    ret %7
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
PixelLocal = struct @align(4) {
  a:u32 @offset(0)
}

params = struct @align(16) {
  pos:vec4<f32> @offset(0), @builtin(position)
  front_facing:bool @offset(16), @builtin(front_facing)
  sample_index:u32 @offset(20), @builtin(sample_index)
}

$B1: {  # root
  %pl:ptr<private, PixelLocal, read_write> = var
  %pixel_local_a:ptr<handle, hlsl.rasterizer_ordered_texture_2d<r32uint>, read> = var @binding_point(7, 10)
}

%main = @fragment func(%4:params):vec4<f32> [@location(0)] {
  $B2: {
    %5:vec4<f32> = access %4, 0u
    %6:vec2<f32> = swizzle %5, xy
    %7:vec2<u32> = convert %6
    %8:hlsl.rasterizer_ordered_texture_2d<r32uint> = load %pixel_local_a
    %9:vec4<u32> = %8.Load %7
    %10:u32 = swizzle %9, x
    %11:ptr<private, u32, read_write> = access %pl, 0u
    store %11, %10
    %12:ptr<private, u32, read_write> = access %pl, 0u
    %13:u32 = load %12
    %14:u32 = add %13, 42u
    store %12, %14
    %15:vec4<f32> = construct 1.0f, 0.0f, 0.0f, 1.0f
    %16:ptr<private, u32, read_write> = access %pl, 0u
    %17:u32 = load %16
    %18:vec4<u32> = construct %17
    %19:hlsl.rasterizer_ordered_texture_2d<r32uint> = load %pixel_local_a
    %20:void = hlsl.textureStore %19, %7, %18
    ret %15
  }
}
)";

    auto config = OneArgConfig();
    Run(PixelLocal, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterPixelLocalTest, MultipleEntryPoints) {
    auto* pixel_local_struct_ty =
        ty.Struct(mod.symbols.New("PixelLocal"), {
                                                     {mod.symbols.New("a"), ty.u32()},
                                                 });
    auto* pl = b.Var("pl", ty.ptr<pixel_local>(pixel_local_struct_ty));
    mod.root_block->Append(pl);

    Vector<core::type::Manager::StructMemberDesc, 3> members;
    core::IOAttributes attrs;
    attrs.builtin = core::BuiltinValue::kPosition;
    members.Emplace(mod.symbols.New("pos"), ty.vec4<f32>(), attrs);
    auto* param_struct_ty = ty.Struct(mod.symbols.New("params"), members);

    for (size_t i = 0; i < 3; ++i) {
        auto* func = b.Function("main" + std::to_string(i), ty.vec4<f32>(),
                                core::ir::Function::PipelineStage::kFragment);
        func->SetReturnLocation(0_u);
        func->SetParams({b.FunctionParam(param_struct_ty)});

        b.Append(func->Block(), [&] {
            auto* access = b.Access(ty.ptr<pixel_local>(ty.u32()), pl, 0_u);
            auto* add = b.Add<u32>(b.Load(access), 42_u);
            b.Store(access, add);
            b.Return(func, b.Construct<vec4<f32>>(1_f, 0_f, 0_f, 1_f));
        });
    }

    auto* src = R"(
PixelLocal = struct @align(4) {
  a:u32 @offset(0)
}

params = struct @align(16) {
  pos:vec4<f32> @offset(0), @builtin(position)
}

$B1: {  # root
  %pl:ptr<pixel_local, PixelLocal, read_write> = var
}

%main0 = @fragment func(%3:params):vec4<f32> [@location(0)] {
  $B2: {
    %4:ptr<pixel_local, u32, read_write> = access %pl, 0u
    %5:u32 = load %4
    %6:u32 = add %5, 42u
    store %4, %6
    %7:vec4<f32> = construct 1.0f, 0.0f, 0.0f, 1.0f
    ret %7
  }
}
%main1 = @fragment func(%9:params):vec4<f32> [@location(0)] {
  $B3: {
    %10:ptr<pixel_local, u32, read_write> = access %pl, 0u
    %11:u32 = load %10
    %12:u32 = add %11, 42u
    store %10, %12
    %13:vec4<f32> = construct 1.0f, 0.0f, 0.0f, 1.0f
    ret %13
  }
}
%main2 = @fragment func(%15:params):vec4<f32> [@location(0)] {
  $B4: {
    %16:ptr<pixel_local, u32, read_write> = access %pl, 0u
    %17:u32 = load %16
    %18:u32 = add %17, 42u
    store %16, %18
    %19:vec4<f32> = construct 1.0f, 0.0f, 0.0f, 1.0f
    ret %19
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
PixelLocal = struct @align(4) {
  a:u32 @offset(0)
}

params = struct @align(16) {
  pos:vec4<f32> @offset(0), @builtin(position)
}

$B1: {  # root
  %pl:ptr<private, PixelLocal, read_write> = var
  %pixel_local_a:ptr<handle, hlsl.rasterizer_ordered_texture_2d<r32uint>, read> = var @binding_point(7, 10)
}

%main0 = @fragment func(%4:params):vec4<f32> [@location(0)] {
  $B2: {
    %5:vec4<f32> = access %4, 0u
    %6:vec2<f32> = swizzle %5, xy
    %7:vec2<u32> = convert %6
    %8:hlsl.rasterizer_ordered_texture_2d<r32uint> = load %pixel_local_a
    %9:vec4<u32> = %8.Load %7
    %10:u32 = swizzle %9, x
    %11:ptr<private, u32, read_write> = access %pl, 0u
    store %11, %10
    %12:ptr<private, u32, read_write> = access %pl, 0u
    %13:u32 = load %12
    %14:u32 = add %13, 42u
    store %12, %14
    %15:vec4<f32> = construct 1.0f, 0.0f, 0.0f, 1.0f
    %16:ptr<private, u32, read_write> = access %pl, 0u
    %17:u32 = load %16
    %18:vec4<u32> = construct %17
    %19:hlsl.rasterizer_ordered_texture_2d<r32uint> = load %pixel_local_a
    %20:void = hlsl.textureStore %19, %7, %18
    ret %15
  }
}
%main1 = @fragment func(%22:params):vec4<f32> [@location(0)] {
  $B3: {
    %23:vec4<f32> = access %22, 0u
    %24:vec2<f32> = swizzle %23, xy
    %25:vec2<u32> = convert %24
    %26:hlsl.rasterizer_ordered_texture_2d<r32uint> = load %pixel_local_a
    %27:vec4<u32> = %26.Load %25
    %28:u32 = swizzle %27, x
    %29:ptr<private, u32, read_write> = access %pl, 0u
    store %29, %28
    %30:ptr<private, u32, read_write> = access %pl, 0u
    %31:u32 = load %30
    %32:u32 = add %31, 42u
    store %30, %32
    %33:vec4<f32> = construct 1.0f, 0.0f, 0.0f, 1.0f
    %34:ptr<private, u32, read_write> = access %pl, 0u
    %35:u32 = load %34
    %36:vec4<u32> = construct %35
    %37:hlsl.rasterizer_ordered_texture_2d<r32uint> = load %pixel_local_a
    %38:void = hlsl.textureStore %37, %25, %36
    ret %33
  }
}
%main2 = @fragment func(%40:params):vec4<f32> [@location(0)] {
  $B4: {
    %41:vec4<f32> = access %40, 0u
    %42:vec2<f32> = swizzle %41, xy
    %43:vec2<u32> = convert %42
    %44:hlsl.rasterizer_ordered_texture_2d<r32uint> = load %pixel_local_a
    %45:vec4<u32> = %44.Load %43
    %46:u32 = swizzle %45, x
    %47:ptr<private, u32, read_write> = access %pl, 0u
    store %47, %46
    %48:ptr<private, u32, read_write> = access %pl, 0u
    %49:u32 = load %48
    %50:u32 = add %49, 42u
    store %48, %50
    %51:vec4<f32> = construct 1.0f, 0.0f, 0.0f, 1.0f
    %52:ptr<private, u32, read_write> = access %pl, 0u
    %53:u32 = load %52
    %54:vec4<u32> = construct %53
    %55:hlsl.rasterizer_ordered_texture_2d<r32uint> = load %pixel_local_a
    %56:void = hlsl.textureStore %55, %43, %54
    ret %51
  }
}
)";

    auto config = OneArgConfig();
    Run(PixelLocal, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterPixelLocalTest, MultipleMembers) {
    auto r = ThreeArgFunc();
    b.Append(r.func->Block(), [&] {
        auto* access = b.Access(ty.ptr<pixel_local>(ty.u32()), r.pl, 0_u);
        auto* add = b.Add<u32>(b.Load(access), 42_u);
        b.Store(access, add);
        b.Return(r.func, b.Construct<vec4<f32>>(1_f, 0_f, 0_f, 1_f));
    });

    auto* src = R"(
PixelLocal = struct @align(4) {
  a:u32 @offset(0)
  b:i32 @offset(4)
  c:f32 @offset(8)
}

params = struct @align(16) {
  pos:vec4<f32> @offset(0), @builtin(position)
}

$B1: {  # root
  %pl:ptr<pixel_local, PixelLocal, read_write> = var
}

%main = @fragment func(%3:params):vec4<f32> [@location(0)] {
  $B2: {
    %4:ptr<pixel_local, u32, read_write> = access %pl, 0u
    %5:u32 = load %4
    %6:u32 = add %5, 42u
    store %4, %6
    %7:vec4<f32> = construct 1.0f, 0.0f, 0.0f, 1.0f
    ret %7
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
PixelLocal = struct @align(4) {
  a:u32 @offset(0)
  b:i32 @offset(4)
  c:f32 @offset(8)
}

params = struct @align(16) {
  pos:vec4<f32> @offset(0), @builtin(position)
}

$B1: {  # root
  %pl:ptr<private, PixelLocal, read_write> = var
  %pixel_local_a:ptr<handle, hlsl.rasterizer_ordered_texture_2d<r32uint>, read> = var @binding_point(7, 10)
  %pixel_local_b:ptr<handle, hlsl.rasterizer_ordered_texture_2d<r32sint>, read> = var @binding_point(7, 12)
  %pixel_local_c:ptr<handle, hlsl.rasterizer_ordered_texture_2d<r32float>, read> = var @binding_point(7, 14)
}

%main = @fragment func(%6:params):vec4<f32> [@location(0)] {
  $B2: {
    %7:vec4<f32> = access %6, 0u
    %8:vec2<f32> = swizzle %7, xy
    %9:vec2<u32> = convert %8
    %10:hlsl.rasterizer_ordered_texture_2d<r32uint> = load %pixel_local_a
    %11:vec4<u32> = %10.Load %9
    %12:u32 = swizzle %11, x
    %13:ptr<private, u32, read_write> = access %pl, 0u
    store %13, %12
    %14:hlsl.rasterizer_ordered_texture_2d<r32sint> = load %pixel_local_b
    %15:vec4<i32> = %14.Load %9
    %16:i32 = swizzle %15, x
    %17:ptr<private, i32, read_write> = access %pl, 1u
    store %17, %16
    %18:hlsl.rasterizer_ordered_texture_2d<r32float> = load %pixel_local_c
    %19:vec4<f32> = %18.Load %9
    %20:f32 = swizzle %19, x
    %21:ptr<private, f32, read_write> = access %pl, 2u
    store %21, %20
    %22:ptr<private, u32, read_write> = access %pl, 0u
    %23:u32 = load %22
    %24:u32 = add %23, 42u
    store %22, %24
    %25:vec4<f32> = construct 1.0f, 0.0f, 0.0f, 1.0f
    %26:ptr<private, u32, read_write> = access %pl, 0u
    %27:u32 = load %26
    %28:vec4<u32> = construct %27
    %29:hlsl.rasterizer_ordered_texture_2d<r32uint> = load %pixel_local_a
    %30:void = hlsl.textureStore %29, %9, %28
    %31:ptr<private, i32, read_write> = access %pl, 1u
    %32:i32 = load %31
    %33:vec4<i32> = construct %32
    %34:hlsl.rasterizer_ordered_texture_2d<r32sint> = load %pixel_local_b
    %35:void = hlsl.textureStore %34, %9, %33
    %36:ptr<private, f32, read_write> = access %pl, 2u
    %37:f32 = load %36
    %38:vec4<f32> = construct %37
    %39:hlsl.rasterizer_ordered_texture_2d<r32float> = load %pixel_local_c
    %40:void = hlsl.textureStore %39, %9, %38
    ret %25
  }
}
)";

    auto config = ThreeArgConfig();
    Run(PixelLocal, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterPixelLocalTest, MultipleMembers_MismatchedTypes) {
    auto r = ThreeArgFunc();
    b.Append(r.func->Block(), [&] {
        auto* access = b.Access(ty.ptr<pixel_local>(ty.u32()), r.pl, 0_u);
        auto* add = b.Add<u32>(b.Load(access), 42_u);
        b.Store(access, add);
        b.Return(r.func, b.Construct<vec4<f32>>(1_f, 0_f, 0_f, 1_f));
    });

    auto* src = R"(
PixelLocal = struct @align(4) {
  a:u32 @offset(0)
  b:i32 @offset(4)
  c:f32 @offset(8)
}

params = struct @align(16) {
  pos:vec4<f32> @offset(0), @builtin(position)
}

$B1: {  # root
  %pl:ptr<pixel_local, PixelLocal, read_write> = var
}

%main = @fragment func(%3:params):vec4<f32> [@location(0)] {
  $B2: {
    %4:ptr<pixel_local, u32, read_write> = access %pl, 0u
    %5:u32 = load %4
    %6:u32 = add %5, 42u
    store %4, %6
    %7:vec4<f32> = construct 1.0f, 0.0f, 0.0f, 1.0f
    ret %7
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
PixelLocal = struct @align(4) {
  a:u32 @offset(0)
  b:i32 @offset(4)
  c:f32 @offset(8)
}

params = struct @align(16) {
  pos:vec4<f32> @offset(0), @builtin(position)
}

$B1: {  # root
  %pl:ptr<private, PixelLocal, read_write> = var
  %pixel_local_a:ptr<handle, hlsl.rasterizer_ordered_texture_2d<r32float>, read> = var @binding_point(7, 10)
  %pixel_local_b:ptr<handle, hlsl.rasterizer_ordered_texture_2d<r32uint>, read> = var @binding_point(7, 12)
  %pixel_local_c:ptr<handle, hlsl.rasterizer_ordered_texture_2d<r32sint>, read> = var @binding_point(7, 14)
}

%main = @fragment func(%6:params):vec4<f32> [@location(0)] {
  $B2: {
    %7:vec4<f32> = access %6, 0u
    %8:vec2<f32> = swizzle %7, xy
    %9:vec2<u32> = convert %8
    %10:hlsl.rasterizer_ordered_texture_2d<r32float> = load %pixel_local_a
    %11:vec4<f32> = %10.Load %9
    %12:f32 = swizzle %11, x
    %13:u32 = convert %12
    %14:ptr<private, u32, read_write> = access %pl, 0u
    store %14, %13
    %15:hlsl.rasterizer_ordered_texture_2d<r32uint> = load %pixel_local_b
    %16:vec4<u32> = %15.Load %9
    %17:u32 = swizzle %16, x
    %18:i32 = convert %17
    %19:ptr<private, i32, read_write> = access %pl, 1u
    store %19, %18
    %20:hlsl.rasterizer_ordered_texture_2d<r32sint> = load %pixel_local_c
    %21:vec4<i32> = %20.Load %9
    %22:i32 = swizzle %21, x
    %23:f32 = convert %22
    %24:ptr<private, f32, read_write> = access %pl, 2u
    store %24, %23
    %25:ptr<private, u32, read_write> = access %pl, 0u
    %26:u32 = load %25
    %27:u32 = add %26, 42u
    store %25, %27
    %28:vec4<f32> = construct 1.0f, 0.0f, 0.0f, 1.0f
    %29:ptr<private, u32, read_write> = access %pl, 0u
    %30:u32 = load %29
    %31:f32 = convert %30
    %32:vec4<f32> = construct %31
    %33:hlsl.rasterizer_ordered_texture_2d<r32float> = load %pixel_local_a
    %34:void = hlsl.textureStore %33, %9, %32
    %35:ptr<private, i32, read_write> = access %pl, 1u
    %36:i32 = load %35
    %37:u32 = convert %36
    %38:vec4<u32> = construct %37
    %39:hlsl.rasterizer_ordered_texture_2d<r32uint> = load %pixel_local_b
    %40:void = hlsl.textureStore %39, %9, %38
    %41:ptr<private, f32, read_write> = access %pl, 2u
    %42:f32 = load %41
    %43:i32 = convert %42
    %44:vec4<i32> = construct %43
    %45:hlsl.rasterizer_ordered_texture_2d<r32sint> = load %pixel_local_c
    %46:void = hlsl.textureStore %45, %9, %44
    ret %28
  }
}
)";

    auto config = ThreeArgConfig();
    // Overwrite the three format types to mismatch the ones in the IR
    config.options.attachments[0].format = PixelLocalAttachment::TexelFormat::kR32Float;
    config.options.attachments[1].format = PixelLocalAttachment::TexelFormat::kR32Uint;
    config.options.attachments[2].format = PixelLocalAttachment::TexelFormat::kR32Sint;
    Run(PixelLocal, config);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::hlsl::writer::raise

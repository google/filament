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

#include "src/tint/lang/core/ir/transform/lower_swizzle_view.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/swizzle_view.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class IR_LowerSwizzleViewTest : public TransformTest {
  public:
    IR_LowerSwizzleViewTest() { mod.properties.Add(core::ir::Property::kAllowSwizzleView); }
};

TEST_F(IR_LowerSwizzleViewTest, Accessor_Load_Constant) {
    auto* func = b.Function("foo", ty.void_());

    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", ty.ptr<function, vec4<f32>, read_write>());
        auto* v_mv = v->Result(0)->Type()->As<core::type::MemoryView>();
        auto* sw_ty = ty.Get<core::type::SwizzleView>(v_mv->AddressSpace(), ty.vec3<f32>(),
                                                      v_mv->Access(), 4u, 3u);
        auto* sw = b.Swizzle(sw_ty, v, {2u, 1u, 0u});  // zyx
        auto* access_ty =
            ty.Get<core::type::SwizzleView>(v_mv->AddressSpace(), ty.f32(), v_mv->Access(), 3u, 1u);
        auto* access = b.Access(access_ty, sw, 0_u);
        auto* load = b.Load(access);
        b.Let("x", load);
        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    %v:ptr<function, vec4<f32>, read_write> = var undef
    %3:swizzle<function, vec3<f32>, read_write, 4, 3> = swizzle %v, zyx
    %4:swizzle<function, f32, read_write, 3, 1> = access %3, 0u
    %5:f32 = load %4
    %x:f32 = let %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():void {
  $B1: {
    %v:ptr<function, vec4<f32>, read_write> = var undef
    %3:f32 = load_vector_element %v, 2u
    %x:f32 = let %3
    ret
  }
}
)";

    Run(LowerSwizzleView);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_LowerSwizzleViewTest, Accessor_Store_Dynamic) {
    auto* func = b.Function("foo", ty.void_());
    auto* dynamic_idx = b.FunctionParam("idx", ty.u32());
    func->SetParams({dynamic_idx});

    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", ty.ptr<function, vec4<f32>, read_write>());
        auto* v_mv = v->Result(0)->Type()->As<core::type::MemoryView>();
        auto* sw_ty = ty.Get<core::type::SwizzleView>(v_mv->AddressSpace(), ty.vec3<f32>(),
                                                      v_mv->Access(), 4u, 3u);
        auto* sw = b.Swizzle(sw_ty, v, {2u, 1u, 0u});  // zyx
        auto* access_ty =
            ty.Get<core::type::SwizzleView>(v_mv->AddressSpace(), ty.f32(), v_mv->Access(), 3u, 1u);
        auto* access = b.Access(access_ty, sw, dynamic_idx);
        b.Store(access, 1.0_f);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%idx:u32):void {
  $B1: {
    %v:ptr<function, vec4<f32>, read_write> = var undef
    %4:swizzle<function, vec3<f32>, read_write, 4, 3> = swizzle %v, zyx
    %5:swizzle<function, f32, read_write, 3, 1> = access %4, %idx
    store %5, 1.0f
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%idx:u32):void {
  $B1: {
    %v:ptr<function, vec4<f32>, read_write> = var undef
    %4:u32 = access array<u32, 3>(2u, 1u, 0u), %idx
    store_vector_element %v, %4, 1.0f
    ret
  }
}
)";

    Run(LowerSwizzleView);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_LowerSwizzleViewTest, Load_MultiElement) {
    auto* func = b.Function("foo", ty.void_());

    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", ty.ptr<function, vec4<f32>, read_write>());
        auto* v_mv = v->Result(0)->Type()->As<core::type::MemoryView>();
        auto* sw_ty = ty.Get<core::type::SwizzleView>(v_mv->AddressSpace(), ty.vec3<f32>(),
                                                      v_mv->Access(), 4u, 3u);
        auto* sw = b.Swizzle(sw_ty, v, {1u, 3u, 0u});
        auto* load = b.Load(sw);
        b.Let("x", load);
        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    %v:ptr<function, vec4<f32>, read_write> = var undef
    %3:swizzle<function, vec3<f32>, read_write, 4, 3> = swizzle %v, ywx
    %4:vec3<f32> = load %3
    %x:vec3<f32> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():void {
  $B1: {
    %v:ptr<function, vec4<f32>, read_write> = var undef
    %3:vec4<f32> = load %v
    %4:vec3<f32> = swizzle %3, ywx
    %x:vec3<f32> = let %4
    ret
  }
}
)";

    Run(LowerSwizzleView);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_LowerSwizzleViewTest, Store_MultiElement) {
    auto* func = b.Function("foo", ty.void_());

    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", ty.ptr<function, vec4<f32>, read_write>());
        auto* v_mv = v->Result(0)->Type()->As<core::type::MemoryView>();
        auto* sw_ty = ty.Get<core::type::SwizzleView>(v_mv->AddressSpace(), ty.vec3<f32>(),
                                                      v_mv->Access(), 4u, 3u);
        auto* sw = b.Swizzle(sw_ty, v, {1u, 3u, 0u});
        b.Store(sw, b.Construct(ty.vec3<f32>(), 1.0_f, 2.0_f, 3.0_f));
        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    %v:ptr<function, vec4<f32>, read_write> = var undef
    %3:swizzle<function, vec3<f32>, read_write, 4, 3> = swizzle %v, ywx
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    store %3, %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():void {
  $B1: {
    %v:ptr<function, vec4<f32>, read_write> = var undef
    %3:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %4:vec4<f32> = load %v
    %5:f32 = access %3, 0u
    %6:f32 = access %3, 1u
    %7:f32 = access %3, 2u
    %8:f32 = access %4, 2u
    %9:vec4<f32> = construct %7, %5, %8, %6
    store %v, %9
    ret
  }
}
)";

    Run(LowerSwizzleView);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_LowerSwizzleViewTest, ChainedSwizzle_Store) {
    auto* func = b.Function("foo", ty.void_());

    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", ty.ptr<function, vec4<f32>, read_write>());
        auto* v_mv = v->Result(0)->Type()->As<core::type::MemoryView>();
        auto* sw_ty1 = ty.Get<core::type::SwizzleView>(v_mv->AddressSpace(), ty.vec3<f32>(),
                                                       v_mv->Access(), 4u, 3u);  // zyx
        auto* sw_ty2 = ty.Get<core::type::SwizzleView>(v_mv->AddressSpace(), ty.vec2<f32>(),
                                                       v_mv->Access(), 3u, 2u);  // zx
        auto* sw1 = b.Swizzle(sw_ty1, v, {2u, 1u, 0u});                          // zyx
        auto* sw2 = b.Swizzle(sw_ty2, sw1, {0u, 2u});  // xz on zyx -> indices [2, 0] on v (zx)
        b.Store(sw2, b.Construct(ty.vec2<f32>(), 1.0_f, 2.0_f));
        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    %v:ptr<function, vec4<f32>, read_write> = var undef
    %3:swizzle<function, vec3<f32>, read_write, 4, 3> = swizzle %v, zyx
    %4:swizzle<function, vec2<f32>, read_write, 3, 2> = swizzle %3, xz
    %5:vec2<f32> = construct 1.0f, 2.0f
    store %4, %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():void {
  $B1: {
    %v:ptr<function, vec4<f32>, read_write> = var undef
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:vec4<f32> = load %v
    %5:f32 = access %3, 0u
    %6:f32 = access %3, 1u
    %7:f32 = access %4, 1u
    %8:f32 = access %4, 3u
    %9:vec4<f32> = construct %6, %7, %5, %8
    store %v, %9
    ret
  }
}
)";

    Run(LowerSwizzleView);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_LowerSwizzleViewTest, ChainedSwizzle_Load) {
    auto* func = b.Function("foo", ty.void_());

    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", ty.ptr<function, vec4<f32>, read_write>());
        auto* v_mv = v->Result(0)->Type()->As<core::type::MemoryView>();
        auto* sw_ty1 = ty.Get<core::type::SwizzleView>(v_mv->AddressSpace(), ty.vec3<f32>(),
                                                       v_mv->Access(), 4u, 3u);
        auto* sw_ty2 = ty.Get<core::type::SwizzleView>(v_mv->AddressSpace(), ty.vec2<f32>(),
                                                       v_mv->Access(), 3u, 2u);
        auto* sw1 = b.Swizzle(sw_ty1, v, {2u, 1u, 0u});  // zyx
        auto* sw2 = b.Swizzle(sw_ty2, sw1, {0u, 2u});    // xz
        auto* load = b.Load(sw2);
        b.Let("l", load);
        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    %v:ptr<function, vec4<f32>, read_write> = var undef
    %3:swizzle<function, vec3<f32>, read_write, 4, 3> = swizzle %v, zyx
    %4:swizzle<function, vec2<f32>, read_write, 3, 2> = swizzle %3, xz
    %5:vec2<f32> = load %4
    %l:vec2<f32> = let %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():void {
  $B1: {
    %v:ptr<function, vec4<f32>, read_write> = var undef
    %3:vec4<f32> = load %v
    %4:vec2<f32> = swizzle %3, zx
    %l:vec2<f32> = let %4
    ret
  }
}
)";

    Run(LowerSwizzleView);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform

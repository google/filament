// Copyright 2023 The Dawn & Tint Authors
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

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/wgsl/writer/raise/raise.h"

namespace tint::wgsl::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class WgslWriter_RaiseTest : public core::ir::transform::TransformTest {
  public:
    WgslWriter_RaiseTest() {
        capabilities.Add(core::ir::Capability::kAllowRefTypes);
        capabilities.Add(core::ir::Capability::kAllowPhonyInstructions);
    }
};

TEST_F(WgslWriter_RaiseTest, BuiltinConversion) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {  //
        b.Call(ty.i32(), core::BuiltinFn::kMax, i32(1), i32(2));
        b.Return(f);
    });

    auto* src = R"(
%f = func():void {
  $B1: {
    %2:i32 = max 1i, 2i
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%f = func():void {
  $B1: {
    %2:i32 = wgsl.max 1i, 2i
    undef = phony %2
    ret
  }
}
)";

    Run(Raise);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_RaiseTest, WorkgroupBarrier) {
    auto* W = b.Var<workgroup, i32, read_write>("W");
    b.ir.root_block->Append(W);
    auto* f = b.Function("f", ty.i32());
    b.Append(f->Block(), [&] {  //
        b.Call(ty.void_(), core::BuiltinFn::kWorkgroupBarrier);
        auto* load = b.Load(W);
        b.Call(ty.void_(), core::BuiltinFn::kWorkgroupBarrier);
        b.Return(f, load);
    });

    auto* src = R"(
$B1: {  # root
  %W:ptr<workgroup, i32, read_write> = var undef
}

%f = func():i32 {
  $B2: {
    %3:void = workgroupBarrier
    %4:i32 = load %W
    %5:void = workgroupBarrier
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %W:ref<workgroup, i32, read_write> = var undef
}

%f = func():i32 {
  $B2: {
    %3:ptr<workgroup, i32, read_write> = ref-to-ptr %W
    %4:i32 = wgsl.workgroupUniformLoad %3
    ret %4
  }
}
)";

    Run(Raise);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_RaiseTest, WorkgroupBarrier_NoMatch) {
    auto* W = b.Var<workgroup, i32, read_write>("W");
    b.ir.root_block->Append(W);
    auto* f = b.Function("f", ty.i32());
    b.Append(f->Block(), [&] {  //
        b.Call(ty.void_(), core::BuiltinFn::kWorkgroupBarrier);
        b.Store(W, 42_i);  // Prevents pattern match
        auto* load = b.Load(W);
        b.Call(ty.void_(), core::BuiltinFn::kWorkgroupBarrier);
        b.Return(f, load);
    });

    auto* src = R"(
$B1: {  # root
  %W:ptr<workgroup, i32, read_write> = var undef
}

%f = func():i32 {
  $B2: {
    %3:void = workgroupBarrier
    store %W, 42i
    %4:i32 = load %W
    %5:void = workgroupBarrier
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %W:ref<workgroup, i32, read_write> = var undef
}

%f = func():i32 {
  $B2: {
    %3:void = wgsl.workgroupBarrier
    store %W, 42i
    %4:i32 = load %W
    %5:i32 = let %4
    %6:void = wgsl.workgroupBarrier
    ret %5
  }
}
)";

    Run(Raise);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_RaiseTest, BuiltinShadowedByUserFunction) {
    auto* user_min = b.Function("min", ty.u32());
    auto* x = b.FunctionParam<u32>("x");
    auto* y = b.FunctionParam<u32>("y");
    user_min->SetParams({x, y});
    b.Append(user_min->Block(), [&] {  //
        b.Return(user_min, b.Add<u32>(x, y));
    });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {  //
        b.Call<u32>(core::BuiltinFn::kMin, 1_u, 2_u);
        b.Return(f);
    });

    auto* src = R"(
%min = func(%x:u32, %y:u32):u32 {
  $B1: {
    %4:u32 = add %x, %y
    ret %4
  }
}
%f = func():void {
  $B2: {
    %6:u32 = min 1u, 2u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%min_1 = func(%x:u32, %y:u32):u32 {
  $B1: {
    %4:u32 = add %x, %y
    ret %4
  }
}
%f = func():void {
  $B2: {
    %6:u32 = wgsl.min 1u, 2u
    undef = phony %6
    ret
  }
}
)";

    Run(Raise);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::wgsl::writer::raise

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

#include "src/tint/lang/wgsl/writer/raise/ptr_to_ref.h"

#include <string>
#include <utility>

#include "gtest/gtest.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::wgsl::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class WgslWriter_PtrToRefTest : public core::ir::transform::TransformTest {
  public:
    void SetUp() override {
        capabilities = core::ir::Capabilities{core::ir::Capability::kAllowRefTypes};
    }
};

TEST_F(WgslWriter_PtrToRefTest, PtrParam_NoChange) {
    auto fn = b.Function(ty.void_());
    fn->SetParams({b.FunctionParam(ty.ptr<function, i32, read_write>())});
    b.Append(fn->Block(), [&] { b.Return(fn); });

    auto* src = R"(
%1 = func(%2:ptr<function, i32, read_write>):void {
  $B1: {
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(PtrToRef);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_PtrToRefTest, Var) {
    b.Append(mod.root_block, [&] { b.Var(ty.ptr<private_, i32>()); });

    auto* src = R"(
$B1: {  # root
  %1:ptr<private, i32, read_write> = var undef
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ref<private, i32, read_write> = var undef
}

)";

    Run(PtrToRef);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_PtrToRefTest, LoadVar) {
    auto fn = b.Function(ty.i32());
    b.Append(fn->Block(), [&] {
        auto* v = b.Var<function, i32>();
        b.Return(fn, b.Load(v));
    });

    auto* src = R"(
%1 = func():i32 {
  $B1: {
    %2:ptr<function, i32, read_write> = var undef
    %3:i32 = load %2
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%1 = func():i32 {
  $B1: {
    %2:ref<function, i32, read_write> = var undef
    %3:i32 = load %2
    ret %3
  }
}
)";

    Run(PtrToRef);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_PtrToRefTest, StoreVar) {
    auto fn = b.Function(ty.void_());
    b.Append(fn->Block(), [&] {
        auto* v = b.Var<function, i32>();
        b.Store(v, 42_i);
        b.Return(fn);
    });

    auto* src = R"(
%1 = func():void {
  $B1: {
    %2:ptr<function, i32, read_write> = var undef
    store %2, 42i
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%1 = func():void {
  $B1: {
    %2:ref<function, i32, read_write> = var undef
    store %2, 42i
    ret
  }
}
)";

    Run(PtrToRef);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_PtrToRefTest, LoadPtrParam) {
    auto fn = b.Function(ty.i32());
    auto* ptr = b.FunctionParam(ty.ptr<function, i32, read_write>());
    fn->SetParams({ptr});
    b.Append(fn->Block(), [&] { b.Return(fn, b.Load(ptr)); });

    auto* src = R"(
%1 = func(%2:ptr<function, i32, read_write>):i32 {
  $B1: {
    %3:i32 = load %2
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%1 = func(%2:ptr<function, i32, read_write>):i32 {
  $B1: {
    %3:ref<function, i32, read_write> = ptr-to-ref %2
    %4:i32 = load %3
    ret %4
  }
}
)";

    Run(PtrToRef);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_PtrToRefTest, StorePtrParam) {
    auto fn = b.Function(ty.void_());
    auto* ptr = b.FunctionParam(ty.ptr<function, i32, read_write>());
    fn->SetParams({ptr});
    b.Append(fn->Block(), [&] {
        b.Store(ptr, 42_i);
        b.Return(fn);
    });

    auto* src = R"(
%1 = func(%2:ptr<function, i32, read_write>):void {
  $B1: {
    store %2, 42i
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%1 = func(%2:ptr<function, i32, read_write>):void {
  $B1: {
    %3:ref<function, i32, read_write> = ptr-to-ref %2
    store %3, 42i
    ret
  }
}
)";

    Run(PtrToRef);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_PtrToRefTest, VarUsedAsPtrArg) {
    auto fn_a = b.Function(ty.void_());
    fn_a->SetParams({b.FunctionParam<ptr<function, i32, read_write>>("p")});
    b.Append(fn_a->Block(), [&] { b.Return(fn_a); });
    auto fn_b = b.Function(ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* v = b.Var<function, i32>();
        b.Call(fn_a, v);
        b.Return(fn_b);
    });

    auto* src = R"(
%1 = func(%p:ptr<function, i32, read_write>):void {
  $B1: {
    ret
  }
}
%3 = func():void {
  $B2: {
    %4:ptr<function, i32, read_write> = var undef
    %5:void = call %1, %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%1 = func(%p:ptr<function, i32, read_write>):void {
  $B1: {
    ret
  }
}
%3 = func():void {
  $B2: {
    %4:ref<function, i32, read_write> = var undef
    %5:ptr<function, i32, read_write> = ref-to-ptr %4
    %6:void = call %1, %5
    ret
  }
}
)";

    Run(PtrToRef);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_PtrToRefTest, LoadPtrParamViaLet) {
    auto fn = b.Function(ty.i32());
    auto* ptr = b.FunctionParam(ty.ptr<function, i32, read_write>());
    fn->SetParams({ptr});
    b.Append(fn->Block(), [&] {
        auto let = b.Let("l", ptr);
        b.Return(fn, b.Load(let));
    });

    auto* src = R"(
%1 = func(%2:ptr<function, i32, read_write>):i32 {
  $B1: {
    %l:ptr<function, i32, read_write> = let %2
    %4:i32 = load %l
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%1 = func(%2:ptr<function, i32, read_write>):i32 {
  $B1: {
    %l:ptr<function, i32, read_write> = let %2
    %4:ref<function, i32, read_write> = ptr-to-ref %l
    %5:i32 = load %4
    ret %5
  }
}
)";

    Run(PtrToRef);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_PtrToRefTest, StorePtrParamViaLet) {
    auto fn = b.Function(ty.void_());
    auto* ptr = b.FunctionParam(ty.ptr<function, i32, read_write>());
    fn->SetParams({ptr});
    b.Append(fn->Block(), [&] {
        auto let = b.Let("l", ptr);
        b.Store(let, 42_i);
        b.Return(fn);
    });

    auto* src = R"(
%1 = func(%2:ptr<function, i32, read_write>):void {
  $B1: {
    %l:ptr<function, i32, read_write> = let %2
    store %l, 42i
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%1 = func(%2:ptr<function, i32, read_write>):void {
  $B1: {
    %l:ptr<function, i32, read_write> = let %2
    %4:ref<function, i32, read_write> = ptr-to-ref %l
    store %4, 42i
    ret
  }
}
)";

    Run(PtrToRef);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_PtrToRefTest, LoadAccessFromPtrArrayParam) {
    auto fn = b.Function(ty.i32());
    auto* param = b.FunctionParam(ty.ptr<function, array<i32, 4>, read_write>());
    fn->SetParams({param});
    b.Append(fn->Block(), [&] {
        auto access = b.Access<ptr<function, i32, read_write>>(param, 2_i);
        b.Return(fn, b.Load(access));
    });

    auto* src = R"(
%1 = func(%2:ptr<function, array<i32, 4>, read_write>):i32 {
  $B1: {
    %3:ptr<function, i32, read_write> = access %2, 2i
    %4:i32 = load %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%1 = func(%2:ptr<function, array<i32, 4>, read_write>):i32 {
  $B1: {
    %3:ref<function, array<i32, 4>, read_write> = ptr-to-ref %2
    %4:ref<function, i32, read_write> = access %3, 2i
    %5:i32 = load %4
    ret %5
  }
}
)";

    Run(PtrToRef);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_PtrToRefTest, StoreAccessFromPtrArrayParam) {
    auto fn = b.Function(ty.void_());
    auto* param = b.FunctionParam(ty.ptr<function, array<i32, 4>, read_write>());
    fn->SetParams({param});
    b.Append(fn->Block(), [&] {
        auto access = b.Access<ptr<function, i32, read_write>>(param, 2_i);
        b.Store(access, 42_i);
        b.Return(fn);
    });

    auto* src = R"(
%1 = func(%2:ptr<function, array<i32, 4>, read_write>):void {
  $B1: {
    %3:ptr<function, i32, read_write> = access %2, 2i
    store %3, 42i
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%1 = func(%2:ptr<function, array<i32, 4>, read_write>):void {
  $B1: {
    %3:ref<function, array<i32, 4>, read_write> = ptr-to-ref %2
    %4:ref<function, i32, read_write> = access %3, 2i
    store %4, 42i
    ret
  }
}
)";

    Run(PtrToRef);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_PtrToRefTest, LoadAccessFromPtrArrayParamViaLet) {
    auto fn = b.Function(ty.i32());
    auto* param = b.FunctionParam(ty.ptr<function, array<i32, 4>, read_write>());
    fn->SetParams({param});
    b.Append(fn->Block(), [&] {
        auto access = b.Access<ptr<function, i32, read_write>>(param, 2_i);
        auto let = b.Let("l", access);
        b.Return(fn, b.Load(let));
    });

    auto* src = R"(
%1 = func(%2:ptr<function, array<i32, 4>, read_write>):i32 {
  $B1: {
    %3:ptr<function, i32, read_write> = access %2, 2i
    %l:ptr<function, i32, read_write> = let %3
    %5:i32 = load %l
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%1 = func(%2:ptr<function, array<i32, 4>, read_write>):i32 {
  $B1: {
    %3:ref<function, array<i32, 4>, read_write> = ptr-to-ref %2
    %4:ref<function, i32, read_write> = access %3, 2i
    %5:ptr<function, i32, read_write> = ref-to-ptr %4
    %l:ptr<function, i32, read_write> = let %5
    %7:ref<function, i32, read_write> = ptr-to-ref %l
    %8:i32 = load %7
    ret %8
  }
}
)";

    Run(PtrToRef);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_PtrToRefTest, StoreAccessFromPtrArrayParamViaLet) {
    auto fn = b.Function(ty.void_());
    auto* param = b.FunctionParam(ty.ptr<function, array<i32, 4>, read_write>());
    fn->SetParams({param});
    b.Append(fn->Block(), [&] {
        auto access = b.Access<ptr<function, i32, read_write>>(param, 2_i);
        auto let = b.Let("l", access);
        b.Store(let, 42_i);
        b.Return(fn);
    });

    auto* src = R"(
%1 = func(%2:ptr<function, array<i32, 4>, read_write>):void {
  $B1: {
    %3:ptr<function, i32, read_write> = access %2, 2i
    %l:ptr<function, i32, read_write> = let %3
    store %l, 42i
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%1 = func(%2:ptr<function, array<i32, 4>, read_write>):void {
  $B1: {
    %3:ref<function, array<i32, 4>, read_write> = ptr-to-ref %2
    %4:ref<function, i32, read_write> = access %3, 2i
    %5:ptr<function, i32, read_write> = ref-to-ptr %4
    %l:ptr<function, i32, read_write> = let %5
    %7:ref<function, i32, read_write> = ptr-to-ref %l
    store %7, 42i
    ret
  }
}
)";

    Run(PtrToRef);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_PtrToRefTest, LoadVectorElementFromPtrParam) {
    auto fn = b.Function(ty.i32());
    auto* param = b.FunctionParam(ty.ptr<function, vec3<i32>, read_write>());
    fn->SetParams({param});
    b.Append(fn->Block(), [&] { b.Return(fn, b.LoadVectorElement(param, 2_i)); });

    auto* src = R"(
%1 = func(%2:ptr<function, vec3<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = load_vector_element %2, 2i
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%1 = func(%2:ptr<function, vec3<i32>, read_write>):i32 {
  $B1: {
    %3:ref<function, vec3<i32>, read_write> = ptr-to-ref %2
    %4:i32 = load_vector_element %3, 2i
    ret %4
  }
}
)";

    Run(PtrToRef);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_PtrToRefTest, StoreVectorElementFromPtrParam) {
    auto fn = b.Function(ty.void_());
    auto* param = b.FunctionParam(ty.ptr<function, vec3<i32>, read_write>());
    fn->SetParams({param});
    b.Append(fn->Block(), [&] {
        b.StoreVectorElement(param, 2_i, 42_i);
        b.Return(fn);
    });

    auto* src = R"(
%1 = func(%2:ptr<function, vec3<i32>, read_write>):void {
  $B1: {
    store_vector_element %2, 2i, 42i
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%1 = func(%2:ptr<function, vec3<i32>, read_write>):void {
  $B1: {
    %3:ref<function, vec3<i32>, read_write> = ptr-to-ref %2
    store_vector_element %3, 2i, 42i
    ret
  }
}
)";

    Run(PtrToRef);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::wgsl::writer::raise

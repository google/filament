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

#include "src/tint/lang/msl/writer/raise/polyfill_bool_vector_dynamic_stores.h"

#include <utility>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::msl::writer::raise {
namespace {

using MslWriter_PolyfillBoolVectorDynamicStoresTest = core::ir::transform::TransformTest;

TEST_F(MslWriter_PolyfillBoolVectorDynamicStoresTest, DynamicStore_Vec3Bool) {
    auto* func = b.Function("foo", ty.void_());
    auto* idx_param = b.FunctionParam("idx", ty.i32());
    auto* val_param = b.FunctionParam("val", ty.bool_());
    func->SetParams({idx_param, val_param});

    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", b.Zero<vec3<bool>>());
        b.StoreVectorElement(v, idx_param, val_param);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%idx:i32, %val:bool):void {
  $B1: {
    %v:ptr<function, vec3<bool>, read_write> = var vec3<bool>(false)
    store_vector_element %v, %idx, %val
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%idx:i32, %val:bool):void {
  $B1: {
    %v:ptr<function, vec3<bool>, read_write> = var vec3<bool>(false)
    %5:vec3<bool> = load %v
    %6:vec3<bool> = construct %val
    %7:u32 = convert %idx
    %8:vec3<u32> = construct %7
    %9:vec3<u32> = construct 0u, 1u, 2u
    %10:vec3<bool> = eq %8, %9
    %11:vec3<bool> = select %5, %6, %10
    store %v, %11
    ret
  }
}
)";

    Run(PolyfillBoolVectorDynamicStores);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PolyfillBoolVectorDynamicStoresTest, ConstantStore_Vec3Bool_NoChange) {
    auto* func = b.Function("foo", ty.void_());
    auto* val_param = b.FunctionParam("val", ty.bool_());
    func->SetParams({val_param});

    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", b.Zero<vec3<bool>>());
        b.StoreVectorElement(v, 1_u, val_param);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%val:bool):void {
  $B1: {
    %v:ptr<function, vec3<bool>, read_write> = var vec3<bool>(false)
    store_vector_element %v, 1u, %val
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(PolyfillBoolVectorDynamicStores);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::msl::writer::raise

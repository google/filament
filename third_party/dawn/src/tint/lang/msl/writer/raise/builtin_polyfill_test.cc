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

#include "src/tint/lang/msl/writer/raise/builtin_polyfill.h"

#include <utility>

#include "gtest/gtest.h"
#include "src/tint/lang/core/access.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/texel_format.h"
#include "src/tint/lang/core/type/atomic.h"
#include "src/tint/lang/core/type/builtin_structs.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::msl::writer::raise {
namespace {

using MslWriter_BuiltinPolyfillTest = core::ir::transform::TransformTest;

TEST_F(MslWriter_BuiltinPolyfillTest, AtomicAdd_Workgroup_I32) {
    auto* a = b.FunctionParam<ptr<workgroup, atomic<i32>>>("a");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({a});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<i32>(core::BuiltinFn::kAtomicAdd, a, 1_i);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = atomicAdd %a, 1i
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = msl.atomic_fetch_add_explicit %a, 1i, 0u
    ret %3
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, AtomicAdd_Storage_U32) {
    auto* a = b.FunctionParam<ptr<storage, atomic<u32>>>("a");
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({a});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<u32>(core::BuiltinFn::kAtomicAdd, a, 1_u);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%a:ptr<storage, atomic<u32>, read_write>):u32 {
  $B1: {
    %3:u32 = atomicAdd %a, 1u
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:ptr<storage, atomic<u32>, read_write>):u32 {
  $B1: {
    %3:u32 = msl.atomic_fetch_add_explicit %a, 1u, 0u
    ret %3
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, AtomicAnd) {
    auto* a = b.FunctionParam<ptr<workgroup, atomic<i32>>>("a");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({a});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<i32>(core::BuiltinFn::kAtomicAnd, a, 1_i);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = atomicAnd %a, 1i
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = msl.atomic_fetch_and_explicit %a, 1i, 0u
    ret %3
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, AtomicExchange) {
    auto* a = b.FunctionParam<ptr<workgroup, atomic<i32>>>("a");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({a});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<i32>(core::BuiltinFn::kAtomicExchange, a, 1_i);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = atomicExchange %a, 1i
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = msl.atomic_exchange_explicit %a, 1i, 0u
    ret %3
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, AtomicLoad) {
    auto* a = b.FunctionParam<ptr<workgroup, atomic<i32>>>("a");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({a});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<i32>(core::BuiltinFn::kAtomicLoad, a);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = atomicLoad %a
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = msl.atomic_load_explicit %a, 0u
    ret %3
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, AtomicMax) {
    auto* a = b.FunctionParam<ptr<workgroup, atomic<i32>>>("a");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({a});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<i32>(core::BuiltinFn::kAtomicMax, a, 1_i);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = atomicMax %a, 1i
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = msl.atomic_fetch_max_explicit %a, 1i, 0u
    ret %3
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, AtomicMin) {
    auto* a = b.FunctionParam<ptr<workgroup, atomic<i32>>>("a");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({a});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<i32>(core::BuiltinFn::kAtomicMin, a, 1_i);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = atomicMin %a, 1i
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = msl.atomic_fetch_min_explicit %a, 1i, 0u
    ret %3
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, AtomicOr) {
    auto* a = b.FunctionParam<ptr<workgroup, atomic<i32>>>("a");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({a});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<i32>(core::BuiltinFn::kAtomicOr, a, 1_i);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = atomicOr %a, 1i
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = msl.atomic_fetch_or_explicit %a, 1i, 0u
    ret %3
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, AtomicStore) {
    auto* a = b.FunctionParam<ptr<workgroup, atomic<i32>>>("a");
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({a});
    b.Append(func->Block(), [&] {
        b.Call<void>(core::BuiltinFn::kAtomicStore, a, 1_i);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):void {
  $B1: {
    %3:void = atomicStore %a, 1i
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):void {
  $B1: {
    %3:void = msl.atomic_store_explicit %a, 1i, 0u
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, AtomicSub) {
    auto* a = b.FunctionParam<ptr<workgroup, atomic<i32>>>("a");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({a});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<i32>(core::BuiltinFn::kAtomicSub, a, 1_i);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = atomicSub %a, 1i
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = msl.atomic_fetch_sub_explicit %a, 1i, 0u
    ret %3
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, AtomicXor) {
    auto* a = b.FunctionParam<ptr<workgroup, atomic<i32>>>("a");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({a});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<i32>(core::BuiltinFn::kAtomicXor, a, 1_i);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = atomicXor %a, 1i
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = msl.atomic_fetch_xor_explicit %a, 1i, 0u
    ret %3
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, AtomicCompareExchangeWeak) {
    auto* a = b.FunctionParam<ptr<workgroup, atomic<i32>>>("a");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({a});
    b.Append(func->Block(), [&] {
        auto* result =
            b.Call(core::type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.i32()),
                   core::BuiltinFn::kAtomicCompareExchangeWeak, a, 1_i, 2_i);
        auto* if_ = b.If(b.Access<bool>(result, 1_u));
        b.Append(if_->True(), [&] {  //
            b.Return(func, b.Access<i32>(result, 0_u));
        });
        b.Return(func, 42_i);
    });

    auto* src = R"(
__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:__atomic_compare_exchange_result_i32 = atomicCompareExchangeWeak %a, 1i, 2i
    %4:bool = access %3, 1u
    if %4 [t: $B2] {  # if_1
      $B2: {  # true
        %5:i32 = access %3, 0u
        ret %5
      }
    }
    ret 42i
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:__atomic_compare_exchange_result_i32 = call %4, %a, 1i, 2i
    %5:bool = access %3, 1u
    if %5 [t: $B2] {  # if_1
      $B2: {  # true
        %6:i32 = access %3, 0u
        ret %6
      }
    }
    ret 42i
  }
}
%4 = func(%atomic_ptr:ptr<workgroup, atomic<i32>, read_write>, %cmp:i32, %val:i32):__atomic_compare_exchange_result_i32 {
  $B3: {
    %old_value:ptr<function, i32, read_write> = var %cmp
    %11:bool = msl.atomic_compare_exchange_weak_explicit %atomic_ptr, %old_value, %val, 0u, 0u
    %12:i32 = load %old_value
    %13:__atomic_compare_exchange_result_i32 = construct %12, %11
    ret %13
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest,
       AtomicCompareExchangeWeak_Multiple_SameAddressSpace_SameType) {
    auto* a = b.FunctionParam<ptr<workgroup, atomic<i32>>>("a");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({a});
    b.Append(func->Block(), [&] {
        auto* result_a =
            b.Call(core::type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.i32()),
                   core::BuiltinFn::kAtomicCompareExchangeWeak, a, 1_i, 2_i);
        auto* result_b =
            b.Call(core::type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.i32()),
                   core::BuiltinFn::kAtomicCompareExchangeWeak, a, 3_i, 4_i);
        auto* if_ = b.If(b.Access<bool>(result_a, 1_u));
        b.Append(if_->True(), [&] {  //
            b.Return(func, b.Access<i32>(result_a, 0_u));
        });
        b.Return(func, b.Access<i32>(result_b, 0_u));
    });

    auto* src = R"(
__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:__atomic_compare_exchange_result_i32 = atomicCompareExchangeWeak %a, 1i, 2i
    %4:__atomic_compare_exchange_result_i32 = atomicCompareExchangeWeak %a, 3i, 4i
    %5:bool = access %3, 1u
    if %5 [t: $B2] {  # if_1
      $B2: {  # true
        %6:i32 = access %3, 0u
        ret %6
      }
    }
    %7:i32 = access %4, 0u
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:__atomic_compare_exchange_result_i32 = call %4, %a, 1i, 2i
    %5:__atomic_compare_exchange_result_i32 = call %4, %a, 3i, 4i
    %6:bool = access %3, 1u
    if %6 [t: $B2] {  # if_1
      $B2: {  # true
        %7:i32 = access %3, 0u
        ret %7
      }
    }
    %8:i32 = access %5, 0u
    ret %8
  }
}
%4 = func(%atomic_ptr:ptr<workgroup, atomic<i32>, read_write>, %cmp:i32, %val:i32):__atomic_compare_exchange_result_i32 {
  $B3: {
    %old_value:ptr<function, i32, read_write> = var %cmp
    %13:bool = msl.atomic_compare_exchange_weak_explicit %atomic_ptr, %old_value, %val, 0u, 0u
    %14:i32 = load %old_value
    %15:__atomic_compare_exchange_result_i32 = construct %14, %13
    ret %15
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest,
       AtomicCompareExchangeWeak_Multiple_SameAddressSpace_DifferentType) {
    auto* ai = b.FunctionParam<ptr<workgroup, atomic<i32>>>("ai");
    auto* au = b.FunctionParam<ptr<workgroup, atomic<u32>>>("au");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({ai, au});
    b.Append(func->Block(), [&] {
        auto* result_a =
            b.Call(core::type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.i32()),
                   core::BuiltinFn::kAtomicCompareExchangeWeak, ai, 1_i, 2_i);
        auto* result_b =
            b.Call(core::type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.u32()),
                   core::BuiltinFn::kAtomicCompareExchangeWeak, au, 3_u, 4_u);
        auto* if_ = b.If(b.Access<bool>(result_a, 1_u));
        b.Append(if_->True(), [&] {  //
            b.Return(func, b.Access<i32>(result_a, 0_u));
        });
        b.Return(func, b.Convert<i32>(b.Access<u32>(result_b, 0_u)));
    });

    auto* src = R"(
__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

__atomic_compare_exchange_result_u32 = struct @align(4) {
  old_value:u32 @offset(0)
  exchanged:bool @offset(4)
}

%foo = func(%ai:ptr<workgroup, atomic<i32>, read_write>, %au:ptr<workgroup, atomic<u32>, read_write>):i32 {
  $B1: {
    %4:__atomic_compare_exchange_result_i32 = atomicCompareExchangeWeak %ai, 1i, 2i
    %5:__atomic_compare_exchange_result_u32 = atomicCompareExchangeWeak %au, 3u, 4u
    %6:bool = access %4, 1u
    if %6 [t: $B2] {  # if_1
      $B2: {  # true
        %7:i32 = access %4, 0u
        ret %7
      }
    }
    %8:u32 = access %5, 0u
    %9:i32 = convert %8
    ret %9
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

__atomic_compare_exchange_result_u32 = struct @align(4) {
  old_value:u32 @offset(0)
  exchanged:bool @offset(4)
}

%foo = func(%ai:ptr<workgroup, atomic<i32>, read_write>, %au:ptr<workgroup, atomic<u32>, read_write>):i32 {
  $B1: {
    %4:__atomic_compare_exchange_result_i32 = call %5, %ai, 1i, 2i
    %6:__atomic_compare_exchange_result_u32 = call %7, %au, 3u, 4u
    %8:bool = access %4, 1u
    if %8 [t: $B2] {  # if_1
      $B2: {  # true
        %9:i32 = access %4, 0u
        ret %9
      }
    }
    %10:u32 = access %6, 0u
    %11:i32 = convert %10
    ret %11
  }
}
%5 = func(%atomic_ptr:ptr<workgroup, atomic<i32>, read_write>, %cmp:i32, %val:i32):__atomic_compare_exchange_result_i32 {
  $B3: {
    %old_value:ptr<function, i32, read_write> = var %cmp
    %16:bool = msl.atomic_compare_exchange_weak_explicit %atomic_ptr, %old_value, %val, 0u, 0u
    %17:i32 = load %old_value
    %18:__atomic_compare_exchange_result_i32 = construct %17, %16
    ret %18
  }
}
%7 = func(%atomic_ptr_1:ptr<workgroup, atomic<u32>, read_write>, %cmp_1:u32, %val_1:u32):__atomic_compare_exchange_result_u32 {  # %atomic_ptr_1: 'atomic_ptr', %cmp_1: 'cmp', %val_1: 'val'
  $B4: {
    %old_value_1:ptr<function, u32, read_write> = var %cmp_1  # %old_value_1: 'old_value'
    %23:bool = msl.atomic_compare_exchange_weak_explicit %atomic_ptr_1, %old_value_1, %val_1, 0u, 0u
    %24:u32 = load %old_value_1
    %25:__atomic_compare_exchange_result_u32 = construct %24, %23
    ret %25
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest,
       AtomicCompareExchangeWeak_Multiple_DifferentAddressSpace_SameType) {
    auto* aw = b.FunctionParam<ptr<workgroup, atomic<i32>>>("aw");
    auto* as = b.FunctionParam<ptr<storage, atomic<i32>>>("as");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({aw, as});
    b.Append(func->Block(), [&] {
        auto* result_a =
            b.Call(core::type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.i32()),
                   core::BuiltinFn::kAtomicCompareExchangeWeak, aw, 1_i, 2_i);
        auto* result_b =
            b.Call(core::type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.i32()),
                   core::BuiltinFn::kAtomicCompareExchangeWeak, as, 3_i, 4_i);
        auto* if_ = b.If(b.Access<bool>(result_a, 1_u));
        b.Append(if_->True(), [&] {  //
            b.Return(func, b.Access<i32>(result_a, 0_u));
        });
        b.Return(func, b.Access<i32>(result_b, 0_u));
    });

    auto* src = R"(
__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

%foo = func(%aw:ptr<workgroup, atomic<i32>, read_write>, %as:ptr<storage, atomic<i32>, read_write>):i32 {
  $B1: {
    %4:__atomic_compare_exchange_result_i32 = atomicCompareExchangeWeak %aw, 1i, 2i
    %5:__atomic_compare_exchange_result_i32 = atomicCompareExchangeWeak %as, 3i, 4i
    %6:bool = access %4, 1u
    if %6 [t: $B2] {  # if_1
      $B2: {  # true
        %7:i32 = access %4, 0u
        ret %7
      }
    }
    %8:i32 = access %5, 0u
    ret %8
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

%foo = func(%aw:ptr<workgroup, atomic<i32>, read_write>, %as:ptr<storage, atomic<i32>, read_write>):i32 {
  $B1: {
    %4:__atomic_compare_exchange_result_i32 = call %5, %aw, 1i, 2i
    %6:__atomic_compare_exchange_result_i32 = call %7, %as, 3i, 4i
    %8:bool = access %4, 1u
    if %8 [t: $B2] {  # if_1
      $B2: {  # true
        %9:i32 = access %4, 0u
        ret %9
      }
    }
    %10:i32 = access %6, 0u
    ret %10
  }
}
%5 = func(%atomic_ptr:ptr<workgroup, atomic<i32>, read_write>, %cmp:i32, %val:i32):__atomic_compare_exchange_result_i32 {
  $B3: {
    %old_value:ptr<function, i32, read_write> = var %cmp
    %15:bool = msl.atomic_compare_exchange_weak_explicit %atomic_ptr, %old_value, %val, 0u, 0u
    %16:i32 = load %old_value
    %17:__atomic_compare_exchange_result_i32 = construct %16, %15
    ret %17
  }
}
%7 = func(%atomic_ptr_1:ptr<storage, atomic<i32>, read_write>, %cmp_1:i32, %val_1:i32):__atomic_compare_exchange_result_i32 {  # %atomic_ptr_1: 'atomic_ptr', %cmp_1: 'cmp', %val_1: 'val'
  $B4: {
    %old_value_1:ptr<function, i32, read_write> = var %cmp_1  # %old_value_1: 'old_value'
    %22:bool = msl.atomic_compare_exchange_weak_explicit %atomic_ptr_1, %old_value_1, %val_1, 0u, 0u
    %23:i32 = load %old_value_1
    %24:__atomic_compare_exchange_result_i32 = construct %23, %22
    ret %24
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, Distance_Scalar) {
    auto* value0 = b.FunctionParam<f32>("value0");
    auto* value1 = b.FunctionParam<f32>("value1");
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({value0, value1});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<f32>(core::BuiltinFn::kDistance, value0, value1);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%value0:f32, %value1:f32):f32 {
  $B1: {
    %4:f32 = distance %value0, %value1
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%value0:f32, %value1:f32):f32 {
  $B1: {
    %4:f32 = sub %value0, %value1
    %5:f32 = abs %4
    ret %5
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, Distance_Vector) {
    auto* value0 = b.FunctionParam<vec4<f32>>("value0");
    auto* value1 = b.FunctionParam<vec4<f32>>("value1");
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({value0, value1});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<f32>(core::BuiltinFn::kDistance, value0, value1);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%value0:vec4<f32>, %value1:vec4<f32>):f32 {
  $B1: {
    %4:f32 = distance %value0, %value1
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%value0:vec4<f32>, %value1:vec4<f32>):f32 {
  $B1: {
    %4:f32 = msl.distance %value0, %value1
    ret %4
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, Dot_I32) {
    auto* value0 = b.FunctionParam<vec4<i32>>("value0");
    auto* value1 = b.FunctionParam<vec4<i32>>("value1");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({value0, value1});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<i32>(core::BuiltinFn::kDot, value0, value1);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%value0:vec4<i32>, %value1:vec4<i32>):i32 {
  $B1: {
    %4:i32 = dot %value0, %value1
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%value0:vec4<i32>, %value1:vec4<i32>):i32 {
  $B1: {
    %4:i32 = call %tint_dot, %value0, %value1
    ret %4
  }
}
%tint_dot = func(%lhs:vec4<i32>, %rhs:vec4<i32>):i32 {
  $B2: {
    %8:vec4<i32> = mul %lhs, %rhs
    %9:i32 = access %8, 0u
    %10:i32 = access %8, 1u
    %11:i32 = add %9, %10
    %12:i32 = access %8, 2u
    %13:i32 = add %11, %12
    %14:i32 = access %8, 3u
    %15:i32 = add %13, %14
    ret %15
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, Dot_U32) {
    auto* value0 = b.FunctionParam<vec3<u32>>("value0");
    auto* value1 = b.FunctionParam<vec3<u32>>("value1");
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({value0, value1});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<u32>(core::BuiltinFn::kDot, value0, value1);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%value0:vec3<u32>, %value1:vec3<u32>):u32 {
  $B1: {
    %4:u32 = dot %value0, %value1
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%value0:vec3<u32>, %value1:vec3<u32>):u32 {
  $B1: {
    %4:u32 = call %tint_dot, %value0, %value1
    ret %4
  }
}
%tint_dot = func(%lhs:vec3<u32>, %rhs:vec3<u32>):u32 {
  $B2: {
    %8:vec3<u32> = mul %lhs, %rhs
    %9:u32 = access %8, 0u
    %10:u32 = access %8, 1u
    %11:u32 = add %9, %10
    %12:u32 = access %8, 2u
    %13:u32 = add %11, %12
    ret %13
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, Dot_F32) {
    auto* value0 = b.FunctionParam<vec2<f32>>("value0");
    auto* value1 = b.FunctionParam<vec2<f32>>("value1");
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({value0, value1});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<f32>(core::BuiltinFn::kDot, value0, value1);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%value0:vec2<f32>, %value1:vec2<f32>):f32 {
  $B1: {
    %4:f32 = dot %value0, %value1
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%value0:vec2<f32>, %value1:vec2<f32>):f32 {
  $B1: {
    %4:f32 = msl.dot %value0, %value1
    ret %4
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, Dot_MultipleCalls) {
    auto* v4i = b.FunctionParam<vec4<i32>>("v4i");
    auto* v4u = b.FunctionParam<vec4<u32>>("v4u");
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({v4i, v4u});
    b.Append(func->Block(), [&] {
        b.Call<i32>(core::BuiltinFn::kDot, v4i, v4i);
        b.Call<i32>(core::BuiltinFn::kDot, v4i, v4i);
        b.Call<u32>(core::BuiltinFn::kDot, v4u, v4u);
        b.Call<u32>(core::BuiltinFn::kDot, v4u, v4u);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%v4i:vec4<i32>, %v4u:vec4<u32>):void {
  $B1: {
    %4:i32 = dot %v4i, %v4i
    %5:i32 = dot %v4i, %v4i
    %6:u32 = dot %v4u, %v4u
    %7:u32 = dot %v4u, %v4u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%v4i:vec4<i32>, %v4u:vec4<u32>):void {
  $B1: {
    %4:i32 = call %tint_dot, %v4i, %v4i
    %6:i32 = call %tint_dot, %v4i, %v4i
    %7:u32 = call %tint_dot_1, %v4u, %v4u
    %9:u32 = call %tint_dot_1, %v4u, %v4u
    ret
  }
}
%tint_dot = func(%lhs:vec4<i32>, %rhs:vec4<i32>):i32 {
  $B2: {
    %12:vec4<i32> = mul %lhs, %rhs
    %13:i32 = access %12, 0u
    %14:i32 = access %12, 1u
    %15:i32 = add %13, %14
    %16:i32 = access %12, 2u
    %17:i32 = add %15, %16
    %18:i32 = access %12, 3u
    %19:i32 = add %17, %18
    ret %19
  }
}
%tint_dot_1 = func(%lhs_1:vec4<u32>, %rhs_1:vec4<u32>):u32 {  # %tint_dot_1: 'tint_dot', %lhs_1: 'lhs', %rhs_1: 'rhs'
  $B3: {
    %22:vec4<u32> = mul %lhs_1, %rhs_1
    %23:u32 = access %22, 0u
    %24:u32 = access %22, 1u
    %25:u32 = add %23, %24
    %26:u32 = access %22, 2u
    %27:u32 = add %25, %26
    %28:u32 = access %22, 3u
    %29:u32 = add %27, %28
    ret %29
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, Frexp_Scalar) {
    auto* value = b.FunctionParam<f32>("value");
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(core::type::CreateFrexpResult(ty, mod.symbols, ty.f32()),
                              core::BuiltinFn::kFrexp, value);
        auto* fract = b.Access<f32>(result, 0_u);
        auto* exp = b.Access<i32>(result, 1_u);
        b.Return(func, b.Add<f32>(fract, b.Convert<f32>(exp)));
    });

    auto* src = R"(
__frexp_result_f32 = struct @align(4) {
  fract:f32 @offset(0)
  exp:i32 @offset(4)
}

%foo = func(%value:f32):f32 {
  $B1: {
    %3:__frexp_result_f32 = frexp %value
    %4:f32 = access %3, 0u
    %5:i32 = access %3, 1u
    %6:f32 = convert %5
    %7:f32 = add %4, %6
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
__frexp_result_f32 = struct @align(4) {
  fract:f32 @offset(0)
  exp:i32 @offset(4)
}

%foo = func(%value:f32):f32 {
  $B1: {
    %3:ptr<function, __frexp_result_f32, read_write> = var undef
    %4:ptr<function, i32, read_write> = access %3, 1u
    %5:i32 = load %4
    %6:f32 = msl.frexp %value, %5
    %7:ptr<function, f32, read_write> = access %3, 0u
    store %7, %6
    %8:__frexp_result_f32 = load %3
    %9:f32 = access %8, 0u
    %10:i32 = access %8, 1u
    %11:f32 = convert %10
    %12:f32 = add %9, %11
    ret %12
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, Frexp_Vector) {
    auto* value = b.FunctionParam<vec4<f32>>("value");
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(core::type::CreateFrexpResult(ty, mod.symbols, ty.vec4<f32>()),
                              core::BuiltinFn::kFrexp, value);
        auto* fract = b.Access<vec4<f32>>(result, 0_u);
        auto* whole = b.Access<vec4<i32>>(result, 1_u);
        b.Return(func, b.Add<vec4<f32>>(fract, b.Convert<vec4<f32>>(whole)));
    });

    auto* src = R"(
__frexp_result_vec4_f32 = struct @align(16) {
  fract:vec4<f32> @offset(0)
  exp:vec4<i32> @offset(16)
}

%foo = func(%value:vec4<f32>):vec4<f32> {
  $B1: {
    %3:__frexp_result_vec4_f32 = frexp %value
    %4:vec4<f32> = access %3, 0u
    %5:vec4<i32> = access %3, 1u
    %6:vec4<f32> = convert %5
    %7:vec4<f32> = add %4, %6
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
__frexp_result_vec4_f32 = struct @align(16) {
  fract:vec4<f32> @offset(0)
  exp:vec4<i32> @offset(16)
}

%foo = func(%value:vec4<f32>):vec4<f32> {
  $B1: {
    %3:ptr<function, __frexp_result_vec4_f32, read_write> = var undef
    %4:ptr<function, vec4<i32>, read_write> = access %3, 1u
    %5:vec4<i32> = load %4
    %6:vec4<f32> = msl.frexp %value, %5
    %7:ptr<function, vec4<f32>, read_write> = access %3, 0u
    store %7, %6
    %8:__frexp_result_vec4_f32 = load %3
    %9:vec4<f32> = access %8, 0u
    %10:vec4<i32> = access %8, 1u
    %11:vec4<f32> = convert %10
    %12:vec4<f32> = add %9, %11
    ret %12
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, Length_Scalar) {
    auto* value = b.FunctionParam<f32>("value");
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<f32>(core::BuiltinFn::kLength, value);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%value:f32):f32 {
  $B1: {
    %3:f32 = length %value
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%value:f32):f32 {
  $B1: {
    %3:f32 = abs %value
    ret %3
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, Length_Vector) {
    auto* value = b.FunctionParam<vec4<f32>>("value");
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<f32>(core::BuiltinFn::kLength, value);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%value:vec4<f32>):f32 {
  $B1: {
    %3:f32 = length %value
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%value:vec4<f32>):f32 {
  $B1: {
    %3:f32 = msl.length %value
    ret %3
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, Modf_Scalar) {
    auto* value = b.FunctionParam<f32>("value");
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(core::type::CreateModfResult(ty, mod.symbols, ty.f32()),
                              core::BuiltinFn::kModf, value);
        auto* fract = b.Access<f32>(result, 0_u);
        auto* whole = b.Access<f32>(result, 1_u);
        b.Return(func, b.Add<f32>(fract, whole));
    });

    auto* src = R"(
__modf_result_f32 = struct @align(4) {
  fract:f32 @offset(0)
  whole:f32 @offset(4)
}

%foo = func(%value:f32):f32 {
  $B1: {
    %3:__modf_result_f32 = modf %value
    %4:f32 = access %3, 0u
    %5:f32 = access %3, 1u
    %6:f32 = add %4, %5
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
__modf_result_f32 = struct @align(4) {
  fract:f32 @offset(0)
  whole:f32 @offset(4)
}

%foo = func(%value:f32):f32 {
  $B1: {
    %3:ptr<function, __modf_result_f32, read_write> = var undef
    %4:ptr<function, f32, read_write> = access %3, 1u
    %5:f32 = load %4
    %6:f32 = msl.modf %value, %5
    %7:ptr<function, f32, read_write> = access %3, 0u
    store %7, %6
    %8:__modf_result_f32 = load %3
    %9:f32 = access %8, 0u
    %10:f32 = access %8, 1u
    %11:f32 = add %9, %10
    ret %11
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, Modf_Vector) {
    auto* value = b.FunctionParam<vec4<f32>>("value");
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(core::type::CreateModfResult(ty, mod.symbols, ty.vec4<f32>()),
                              core::BuiltinFn::kModf, value);
        auto* fract = b.Access<vec4<f32>>(result, 0_u);
        auto* whole = b.Access<vec4<f32>>(result, 1_u);
        b.Return(func, b.Add<vec4<f32>>(fract, whole));
    });

    auto* src = R"(
__modf_result_vec4_f32 = struct @align(16) {
  fract:vec4<f32> @offset(0)
  whole:vec4<f32> @offset(16)
}

%foo = func(%value:vec4<f32>):vec4<f32> {
  $B1: {
    %3:__modf_result_vec4_f32 = modf %value
    %4:vec4<f32> = access %3, 0u
    %5:vec4<f32> = access %3, 1u
    %6:vec4<f32> = add %4, %5
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
__modf_result_vec4_f32 = struct @align(16) {
  fract:vec4<f32> @offset(0)
  whole:vec4<f32> @offset(16)
}

%foo = func(%value:vec4<f32>):vec4<f32> {
  $B1: {
    %3:ptr<function, __modf_result_vec4_f32, read_write> = var undef
    %4:ptr<function, vec4<f32>, read_write> = access %3, 1u
    %5:vec4<f32> = load %4
    %6:vec4<f32> = msl.modf %value, %5
    %7:ptr<function, vec4<f32>, read_write> = access %3, 0u
    store %7, %6
    %8:__modf_result_vec4_f32 = load %3
    %9:vec4<f32> = access %8, 0u
    %10:vec4<f32> = access %8, 1u
    %11:vec4<f32> = add %9, %10
    ret %11
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, QuantizeToF16_Scalar) {
    auto* value = b.FunctionParam<f32>("value");
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<f32>(core::BuiltinFn::kQuantizeToF16, value);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%value:f32):f32 {
  $B1: {
    %3:f32 = quantizeToF16 %value
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%value:f32):f32 {
  $B1: {
    %3:f16 = convert %value
    %4:f32 = convert %3
    ret %4
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, QuantizeToF16_Vector) {
    auto* value = b.FunctionParam<vec4<f32>>("value");
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec4<f32>>(core::BuiltinFn::kQuantizeToF16, value);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%value:vec4<f32>):vec4<f32> {
  $B1: {
    %3:vec4<f32> = quantizeToF16 %value
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%value:vec4<f32>):vec4<f32> {
  $B1: {
    %3:vec4<f16> = convert %value
    %4:vec4<f32> = convert %3
    ret %4
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, Sign_F32) {
    auto* value = b.FunctionParam<f32>("value");
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<f32>(core::BuiltinFn::kSign, value);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%value:f32):f32 {
  $B1: {
    %3:f32 = sign %value
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%value:f32):f32 {
  $B1: {
    %3:f32 = msl.sign %value
    ret %3
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, Sign_Scalar) {
    auto* value = b.FunctionParam<i32>("value");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<i32>(core::BuiltinFn::kSign, value);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%value:i32):i32 {
  $B1: {
    %3:i32 = sign %value
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%value:i32):i32 {
  $B1: {
    %3:bool = gt %value, 0i
    %4:i32 = select -1i, 1i, %3
    %5:bool = eq %value, 0i
    %6:i32 = select %4, 0i, %5
    ret %6
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, Sign_Vector) {
    auto* value = b.FunctionParam<vec4<i32>>("value");
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec4<i32>>(core::BuiltinFn::kSign, value);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%value:vec4<i32>):vec4<i32> {
  $B1: {
    %3:vec4<i32> = sign %value
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%value:vec4<i32>):vec4<i32> {
  $B1: {
    %3:vec4<bool> = gt %value, vec4<i32>(0i)
    %4:vec4<i32> = select vec4<i32>(-1i), vec4<i32>(1i), %3
    %5:vec4<bool> = eq %value, vec4<i32>(0i)
    %6:vec4<i32> = select %4, vec4<i32>(0i), %5
    ret %6
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureDimensions_1d) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k1d, ty.f32()));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<u32>(core::BuiltinFn::kTextureDimensions, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_1d<f32>):u32 {
  $B1: {
    %3:u32 = textureDimensions %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_1d<f32>):u32 {
  $B1: {
    %3:u32 = %t.get_width
    %4:u32 = construct %3
    ret %4
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureDimensions_2d_WithoutLod) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* func = b.Function("foo", ty.vec2<u32>());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec2<u32>>(core::BuiltinFn::kTextureDimensions, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>):vec2<u32> {
  $B1: {
    %3:vec2<u32> = textureDimensions %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>):vec2<u32> {
  $B1: {
    %3:u32 = %t.get_width 0u
    %4:u32 = %t.get_height 0u
    %5:vec2<u32> = construct %3, %4
    ret %5
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureDimensions_2d_WithI32Lod) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* func = b.Function("foo", ty.vec2<u32>());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec2<u32>>(core::BuiltinFn::kTextureDimensions, t, 3_i);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>):vec2<u32> {
  $B1: {
    %3:vec2<u32> = textureDimensions %t, 3i
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>):vec2<u32> {
  $B1: {
    %3:u32 = convert 3i
    %4:u32 = %t.get_width %3
    %5:u32 = %t.get_height %3
    %6:vec2<u32> = construct %4, %5
    ret %6
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureDimensions_3d) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k3d, ty.f32()));
    auto* func = b.Function("foo", ty.vec3<u32>());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec3<u32>>(core::BuiltinFn::kTextureDimensions, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_3d<f32>):vec3<u32> {
  $B1: {
    %3:vec3<u32> = textureDimensions %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_3d<f32>):vec3<u32> {
  $B1: {
    %3:u32 = %t.get_width 0u
    %4:u32 = %t.get_height 0u
    %5:u32 = %t.get_depth 0u
    %6:vec3<u32> = construct %3, %4, %5
    ret %6
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureGather_2d_UnsignedComponent) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec4<f32>>(core::BuiltinFn::kTextureGather, 1_u, t, s, coords);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>):vec4<f32> {
  $B1: {
    %5:vec4<f32> = textureGather 1u, %t, %s, %coords
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>):vec4<f32> {
  $B1: {
    %5:vec4<f32> = %t.gather %s, %coords, vec2<i32>(0i), 1u
    ret %5
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureGather_2d_SignedComponent) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec4<f32>>(core::BuiltinFn::kTextureGather, 2_i, t, s, coords);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>):vec4<f32> {
  $B1: {
    %5:vec4<f32> = textureGather 2i, %t, %s, %coords
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>):vec4<f32> {
  $B1: {
    %5:vec4<f32> = %t.gather %s, %coords, vec2<i32>(0i), 2u
    ret %5
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureGather_2d_WithOffset) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* offset = b.FunctionParam("offset", ty.vec2<i32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, offset});
    b.Append(func->Block(), [&] {
        auto* result =
            b.Call<vec4<f32>>(core::BuiltinFn::kTextureGather, 1_u, t, s, coords, offset);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %offset:vec2<i32>):vec4<f32> {
  $B1: {
    %6:vec4<f32> = textureGather 1u, %t, %s, %coords, %offset
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %offset:vec2<i32>):vec4<f32> {
  $B1: {
    %6:vec4<f32> = %t.gather %s, %coords, %offset, 1u
    ret %6
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureGather_Depth2d) {
    auto* t =
        b.FunctionParam("t", ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2d));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec4<f32>>(core::BuiltinFn::kTextureGather, t, s, coords);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d, %s:sampler, %coords:vec2<f32>):vec4<f32> {
  $B1: {
    %5:vec4<f32> = textureGather %t, %s, %coords
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_depth_2d, %s:sampler, %coords:vec2<f32>):vec4<f32> {
  $B1: {
    %5:vec4<f32> = %t.gather %s, %coords, vec2<i32>(0i)
    ret %5
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureGatherCompare) {
    auto* t =
        b.FunctionParam("t", ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2d));
    auto* s = b.FunctionParam("s", ty.comparison_sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* depth = b.FunctionParam("depth", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, depth});
    b.Append(func->Block(), [&] {
        auto* result =
            b.Call<vec4<f32>>(core::BuiltinFn::kTextureGatherCompare, t, s, coords, depth);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d, %s:sampler_comparison, %coords:vec2<f32>, %depth:f32):vec4<f32> {
  $B1: {
    %6:vec4<f32> = textureGatherCompare %t, %s, %coords, %depth
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_depth_2d, %s:sampler_comparison, %coords:vec2<f32>, %depth:f32):vec4<f32> {
  $B1: {
    %6:vec4<f32> = %t.gather_compare %s, %coords, %depth
    ret %6
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureLoad_1d_U32Coord) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* texture_ty =
        ty.storage_texture(core::type::TextureDimension::k1d, format, core::Access::kRead);
    auto* t = b.FunctionParam("t", texture_ty);
    auto* coords = b.FunctionParam("coords", ty.u32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, coords});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec4<f32>>(core::BuiltinFn::kTextureLoad, t, coords);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_storage_1d<rgba8unorm, read>, %coords:u32):vec4<f32> {
  $B1: {
    %4:vec4<f32> = textureLoad %t, %coords
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_storage_1d<rgba8unorm, read>, %coords:u32):vec4<f32> {
  $B1: {
    %4:vec4<f32> = %t.read %coords
    ret %4
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureLoad_1d_I32Coord) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* texture_ty =
        ty.storage_texture(core::type::TextureDimension::k1d, format, core::Access::kRead);
    auto* t = b.FunctionParam("t", texture_ty);
    auto* coords = b.FunctionParam("coords", ty.i32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, coords});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec4<f32>>(core::BuiltinFn::kTextureLoad, t, coords);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_storage_1d<rgba8unorm, read>, %coords:i32):vec4<f32> {
  $B1: {
    %4:vec4<f32> = textureLoad %t, %coords
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_storage_1d<rgba8unorm, read>, %coords:i32):vec4<f32> {
  $B1: {
    %4:u32 = convert %coords
    %5:vec4<f32> = %t.read %4
    ret %5
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureLoad_2d_U32Coords) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* texture_ty =
        ty.storage_texture(core::type::TextureDimension::k2d, format, core::Access::kRead);
    auto* t = b.FunctionParam("t", texture_ty);
    auto* coords = b.FunctionParam("coords", ty.vec2<u32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, coords});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec4<f32>>(core::BuiltinFn::kTextureLoad, t, coords);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_storage_2d<rgba8unorm, read>, %coords:vec2<u32>):vec4<f32> {
  $B1: {
    %4:vec4<f32> = textureLoad %t, %coords
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_storage_2d<rgba8unorm, read>, %coords:vec2<u32>):vec4<f32> {
  $B1: {
    %4:vec4<f32> = %t.read %coords
    ret %4
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureLoad_2d_I32Coords) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* texture_ty =
        ty.storage_texture(core::type::TextureDimension::k2d, format, core::Access::kRead);
    auto* t = b.FunctionParam("t", texture_ty);
    auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, coords});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec4<f32>>(core::BuiltinFn::kTextureLoad, t, coords);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_storage_2d<rgba8unorm, read>, %coords:vec2<i32>):vec4<f32> {
  $B1: {
    %4:vec4<f32> = textureLoad %t, %coords
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_storage_2d<rgba8unorm, read>, %coords:vec2<i32>):vec4<f32> {
  $B1: {
    %4:vec2<u32> = convert %coords
    %5:vec4<f32> = %t.read %4
    ret %5
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureLoad_2d_WithLevel) {
    auto* texture_ty = ty.sampled_texture(core::type::TextureDimension::k2d, ty.i32());
    auto* t = b.FunctionParam("t", texture_ty);
    auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
    auto* level = b.FunctionParam("level", ty.i32());
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({t, coords, level});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec4<i32>>(core::BuiltinFn::kTextureLoad, t, coords, level);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<i32>, %coords:vec2<i32>, %level:i32):vec4<i32> {
  $B1: {
    %5:vec4<i32> = textureLoad %t, %coords, %level
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<i32>, %coords:vec2<i32>, %level:i32):vec4<i32> {
  $B1: {
    %5:vec2<u32> = convert %coords
    %6:vec4<i32> = %t.read %5, %level
    ret %6
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureLoad_2darray_U32Index) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* texture_ty =
        ty.storage_texture(core::type::TextureDimension::k2dArray, format, core::Access::kRead);
    auto* t = b.FunctionParam("t", texture_ty);
    auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
    auto* index = b.FunctionParam("index", ty.u32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, coords, index});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec4<f32>>(core::BuiltinFn::kTextureLoad, t, coords, index);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_storage_2d_array<rgba8unorm, read>, %coords:vec2<i32>, %index:u32):vec4<f32> {
  $B1: {
    %5:vec4<f32> = textureLoad %t, %coords, %index
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_storage_2d_array<rgba8unorm, read>, %coords:vec2<i32>, %index:u32):vec4<f32> {
  $B1: {
    %5:vec2<u32> = convert %coords
    %6:vec4<f32> = %t.read %5, %index
    ret %6
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureLoad_2darray_I32Index) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* texture_ty =
        ty.storage_texture(core::type::TextureDimension::k2dArray, format, core::Access::kRead);
    auto* t = b.FunctionParam("t", texture_ty);
    auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
    auto* index = b.FunctionParam("index", ty.i32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, coords, index});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec4<f32>>(core::BuiltinFn::kTextureLoad, t, coords, index);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_storage_2d_array<rgba8unorm, read>, %coords:vec2<i32>, %index:i32):vec4<f32> {
  $B1: {
    %5:vec4<f32> = textureLoad %t, %coords, %index
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_storage_2d_array<rgba8unorm, read>, %coords:vec2<i32>, %index:i32):vec4<f32> {
  $B1: {
    %5:vec2<u32> = convert %coords
    %6:vec4<f32> = %t.read %5, %index
    ret %6
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureLoad_2darray_WithLevel) {
    auto* texture_ty = ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32());
    auto* t = b.FunctionParam("t", texture_ty);
    auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
    auto* index = b.FunctionParam("index", ty.i32());
    auto* level = b.FunctionParam("level", ty.i32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, coords, index, level});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec4<f32>>(core::BuiltinFn::kTextureLoad, t, coords, index, level);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d_array<f32>, %coords:vec2<i32>, %index:i32, %level:i32):vec4<f32> {
  $B1: {
    %6:vec4<f32> = textureLoad %t, %coords, %index, %level
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d_array<f32>, %coords:vec2<i32>, %index:i32, %level:i32):vec4<f32> {
  $B1: {
    %6:vec2<u32> = convert %coords
    %7:vec4<f32> = %t.read %6, %index, %level
    ret %7
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureLoad_3d) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* texture_ty =
        ty.storage_texture(core::type::TextureDimension::k3d, format, core::Access::kRead);
    auto* t = b.FunctionParam("t", texture_ty);
    auto* coords = b.FunctionParam("coords", ty.vec3<i32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, coords});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec4<f32>>(core::BuiltinFn::kTextureLoad, t, coords);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_storage_3d<rgba8unorm, read>, %coords:vec3<i32>):vec4<f32> {
  $B1: {
    %4:vec4<f32> = textureLoad %t, %coords
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_storage_3d<rgba8unorm, read>, %coords:vec3<i32>):vec4<f32> {
  $B1: {
    %4:vec3<u32> = convert %coords
    %5:vec4<f32> = %t.read %4
    ret %5
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureNumLayers) {
    auto* t =
        b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32()));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<u32>(core::BuiltinFn::kTextureNumLayers, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d_array<f32>):u32 {
  $B1: {
    %3:u32 = textureNumLayers %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d_array<f32>):u32 {
  $B1: {
    %3:u32 = %t.get_array_size
    ret %3
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureNumLevels) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<u32>(core::BuiltinFn::kTextureNumLevels, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>):u32 {
  $B1: {
    %3:u32 = textureNumLevels %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>):u32 {
  $B1: {
    %3:u32 = %t.get_num_mip_levels
    ret %3
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureNumSamples) {
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::MultisampledTexture>(core::type::TextureDimension::k2d, ty.f32()));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<u32>(core::BuiltinFn::kTextureNumSamples, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_multisampled_2d<f32>):u32 {
  $B1: {
    %3:u32 = textureNumSamples %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_multisampled_2d<f32>):u32 {
  $B1: {
    %3:u32 = %t.get_num_samples
    ret %3
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureSample) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec4<f32>>(core::BuiltinFn::kTextureSample, t, s, coords);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>):vec4<f32> {
  $B1: {
    %5:vec4<f32> = textureSample %t, %s, %coords
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>):vec4<f32> {
  $B1: {
    %5:vec4<f32> = %t.sample %s, %coords
    ret %5
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureSampleBias) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* bias = b.FunctionParam("bias", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, bias});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleBias, t, s, coords, bias);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %bias:f32):vec4<f32> {
  $B1: {
    %6:vec4<f32> = textureSampleBias %t, %s, %coords, %bias
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %bias:f32):vec4<f32> {
  $B1: {
    %6:msl.bias = construct %bias
    %7:vec4<f32> = %t.sample %s, %coords, %6
    ret %7
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureSampleBias_Array) {
    auto* t =
        b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* index = b.FunctionParam("index", ty.u32());
    auto* bias = b.FunctionParam("bias", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, index, bias});
    b.Append(func->Block(), [&] {
        auto* result =
            b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleBias, t, s, coords, index, bias);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d_array<f32>, %s:sampler, %coords:vec2<f32>, %index:u32, %bias:f32):vec4<f32> {
  $B1: {
    %7:vec4<f32> = textureSampleBias %t, %s, %coords, %index, %bias
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d_array<f32>, %s:sampler, %coords:vec2<f32>, %index:u32, %bias:f32):vec4<f32> {
  $B1: {
    %7:msl.bias = construct %bias
    %8:vec4<f32> = %t.sample %s, %coords, %index, %7
    ret %8
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureSampleCompare) {
    auto* t =
        b.FunctionParam("t", ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2d));
    auto* s = b.FunctionParam("s", ty.comparison_sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* depth = b.FunctionParam("depth", ty.f32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({t, s, coords, depth});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<f32>(core::BuiltinFn::kTextureSampleCompare, t, s, coords, depth);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d, %s:sampler_comparison, %coords:vec2<f32>, %depth:f32):f32 {
  $B1: {
    %6:f32 = textureSampleCompare %t, %s, %coords, %depth
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_depth_2d, %s:sampler_comparison, %coords:vec2<f32>, %depth:f32):f32 {
  $B1: {
    %6:f32 = %t.sample_compare %s, %coords, %depth
    ret %6
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureSampleCompareLevel) {
    auto* t =
        b.FunctionParam("t", ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2d));
    auto* s = b.FunctionParam("s", ty.comparison_sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* depth = b.FunctionParam("depth", ty.f32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({t, s, coords, depth});
    b.Append(func->Block(), [&] {
        auto* result =
            b.Call<f32>(core::BuiltinFn::kTextureSampleCompareLevel, t, s, coords, depth);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d, %s:sampler_comparison, %coords:vec2<f32>, %depth:f32):f32 {
  $B1: {
    %6:f32 = textureSampleCompareLevel %t, %s, %coords, %depth
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_depth_2d, %s:sampler_comparison, %coords:vec2<f32>, %depth:f32):f32 {
  $B1: {
    %6:msl.level = construct 0u
    %7:f32 = %t.sample_compare %s, %coords, %depth, %6
    ret %7
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureSampleCompareLevel_WithOffset) {
    auto* t =
        b.FunctionParam("t", ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2d));
    auto* s = b.FunctionParam("s", ty.comparison_sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* depth = b.FunctionParam("depth", ty.f32());
    auto* offset = b.FunctionParam("offset", ty.vec2<i32>());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({t, s, coords, depth, offset});
    b.Append(func->Block(), [&] {
        auto* result =
            b.Call<f32>(core::BuiltinFn::kTextureSampleCompareLevel, t, s, coords, depth, offset);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d, %s:sampler_comparison, %coords:vec2<f32>, %depth:f32, %offset:vec2<i32>):f32 {
  $B1: {
    %7:f32 = textureSampleCompareLevel %t, %s, %coords, %depth, %offset
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_depth_2d, %s:sampler_comparison, %coords:vec2<f32>, %depth:f32, %offset:vec2<i32>):f32 {
  $B1: {
    %7:msl.level = construct 0u
    %8:f32 = %t.sample_compare %s, %coords, %depth, %7, %offset
    ret %8
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureSampleGrad_2d) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* ddx = b.FunctionParam("ddx", ty.vec2<f32>());
    auto* ddy = b.FunctionParam("ddy", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, ddx, ddy});
    b.Append(func->Block(), [&] {
        auto* result =
            b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleGrad, t, s, coords, ddx, ddy);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %ddx:vec2<f32>, %ddy:vec2<f32>):vec4<f32> {
  $B1: {
    %7:vec4<f32> = textureSampleGrad %t, %s, %coords, %ddx, %ddy
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %ddx:vec2<f32>, %ddy:vec2<f32>):vec4<f32> {
  $B1: {
    %7:msl.gradient2d = construct %ddx, %ddy
    %8:vec4<f32> = %t.sample %s, %coords, %7
    ret %8
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureGather_2dArray) {
    auto* t =
        b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* index = b.FunctionParam("index", ty.i32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, index});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec4<f32>>(core::BuiltinFn::kTextureGather, 0_u, t, s, coords, index);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d_array<f32>, %s:sampler, %coords:vec2<f32>, %index:i32):vec4<f32> {
  $B1: {
    %6:vec4<f32> = textureGather 0u, %t, %s, %coords, %index
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d_array<f32>, %s:sampler, %coords:vec2<f32>, %index:i32):vec4<f32> {
  $B1: {
    %6:i32 = max %index, 0i
    %7:vec4<f32> = %t.gather %s, %coords, %6, vec2<i32>(0i), 0u
    ret %7
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureSampleGrad_2dArray) {
    auto* t =
        b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* index = b.FunctionParam("index", ty.i32());
    auto* ddx = b.FunctionParam("ddx", ty.vec2<f32>());
    auto* ddy = b.FunctionParam("ddy", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, index, ddx, ddy});
    b.Append(func->Block(), [&] {
        auto* result =
            b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleGrad, t, s, coords, index, ddx, ddy);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d_array<f32>, %s:sampler, %coords:vec2<f32>, %index:i32, %ddx:vec2<f32>, %ddy:vec2<f32>):vec4<f32> {
  $B1: {
    %8:vec4<f32> = textureSampleGrad %t, %s, %coords, %index, %ddx, %ddy
    ret %8
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d_array<f32>, %s:sampler, %coords:vec2<f32>, %index:i32, %ddx:vec2<f32>, %ddy:vec2<f32>):vec4<f32> {
  $B1: {
    %8:msl.gradient2d = construct %ddx, %ddy
    %9:i32 = max %index, 0i
    %10:vec4<f32> = %t.sample %s, %coords, %9, %8
    ret %10
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureSampleGrad_3d) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k3d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec3<f32>());
    auto* ddx = b.FunctionParam("ddx", ty.vec3<f32>());
    auto* ddy = b.FunctionParam("ddy", ty.vec3<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, ddx, ddy});
    b.Append(func->Block(), [&] {
        auto* result =
            b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleGrad, t, s, coords, ddx, ddy);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_3d<f32>, %s:sampler, %coords:vec3<f32>, %ddx:vec3<f32>, %ddy:vec3<f32>):vec4<f32> {
  $B1: {
    %7:vec4<f32> = textureSampleGrad %t, %s, %coords, %ddx, %ddy
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_3d<f32>, %s:sampler, %coords:vec3<f32>, %ddx:vec3<f32>, %ddy:vec3<f32>):vec4<f32> {
  $B1: {
    %7:msl.gradient3d = construct %ddx, %ddy
    %8:vec4<f32> = %t.sample %s, %coords, %7
    ret %8
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureSampleGrad_Cube) {
    auto* t =
        b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::kCube, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec3<f32>());
    auto* ddx = b.FunctionParam("ddx", ty.vec3<f32>());
    auto* ddy = b.FunctionParam("ddy", ty.vec3<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, ddx, ddy});
    b.Append(func->Block(), [&] {
        auto* result =
            b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleGrad, t, s, coords, ddx, ddy);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_cube<f32>, %s:sampler, %coords:vec3<f32>, %ddx:vec3<f32>, %ddy:vec3<f32>):vec4<f32> {
  $B1: {
    %7:vec4<f32> = textureSampleGrad %t, %s, %coords, %ddx, %ddy
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_cube<f32>, %s:sampler, %coords:vec3<f32>, %ddx:vec3<f32>, %ddy:vec3<f32>):vec4<f32> {
  $B1: {
    %7:msl.gradientcube = construct %ddx, %ddy
    %8:vec4<f32> = %t.sample %s, %coords, %7
    ret %8
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureSampleGrad_WithOffset) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* ddx = b.FunctionParam("ddx", ty.vec2<f32>());
    auto* ddy = b.FunctionParam("ddy", ty.vec2<f32>());
    auto* offset = b.FunctionParam("offset", ty.vec2<i32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, ddx, ddy, offset});
    b.Append(func->Block(), [&] {
        auto* result =
            b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleGrad, t, s, coords, ddx, ddy, offset);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %ddx:vec2<f32>, %ddy:vec2<f32>, %offset:vec2<i32>):vec4<f32> {
  $B1: {
    %8:vec4<f32> = textureSampleGrad %t, %s, %coords, %ddx, %ddy, %offset
    ret %8
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %ddx:vec2<f32>, %ddy:vec2<f32>, %offset:vec2<i32>):vec4<f32> {
  $B1: {
    %8:msl.gradient2d = construct %ddx, %ddy
    %9:vec4<f32> = %t.sample %s, %coords, %8, %offset
    ret %9
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureSampleLevel) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* level = b.FunctionParam("level", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, level});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleLevel, t, s, coords, level);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %level:f32):vec4<f32> {
  $B1: {
    %6:vec4<f32> = textureSampleLevel %t, %s, %coords, %level
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %level:f32):vec4<f32> {
  $B1: {
    %6:msl.level = construct %level
    %7:vec4<f32> = %t.sample %s, %coords, %6
    ret %7
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureSampleLevel_Array) {
    auto* t =
        b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* index = b.FunctionParam("index", ty.u32());
    auto* level = b.FunctionParam("level", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, index, level});
    b.Append(func->Block(), [&] {
        auto* result =
            b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleLevel, t, s, coords, index, level);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d_array<f32>, %s:sampler, %coords:vec2<f32>, %index:u32, %level:f32):vec4<f32> {
  $B1: {
    %7:vec4<f32> = textureSampleLevel %t, %s, %coords, %index, %level
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d_array<f32>, %s:sampler, %coords:vec2<f32>, %index:u32, %level:f32):vec4<f32> {
  $B1: {
    %7:msl.level = construct %level
    %8:vec4<f32> = %t.sample %s, %coords, %index, %7
    ret %8
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureStore_1d_U32Coord) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* texture_ty =
        ty.storage_texture(core::type::TextureDimension::k1d, format, core::Access::kWrite);
    auto* t = b.FunctionParam("t", texture_ty);
    auto* coords = b.FunctionParam("coords", ty.u32());
    auto* value = b.FunctionParam("value", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t, coords, value});
    b.Append(func->Block(), [&] {
        b.Call<void>(core::BuiltinFn::kTextureStore, t, coords, value);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%t:texture_storage_1d<rgba8unorm, write>, %coords:u32, %value:vec4<f32>):void {
  $B1: {
    %5:void = textureStore %t, %coords, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_storage_1d<rgba8unorm, write>, %coords:u32, %value:vec4<f32>):void {
  $B1: {
    %5:void = %t.write %value, %coords
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureStore_1d_I32Coord) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* texture_ty =
        ty.storage_texture(core::type::TextureDimension::k1d, format, core::Access::kWrite);
    auto* t = b.FunctionParam("t", texture_ty);
    auto* coords = b.FunctionParam("coords", ty.i32());
    auto* value = b.FunctionParam("value", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t, coords, value});
    b.Append(func->Block(), [&] {
        b.Call<void>(core::BuiltinFn::kTextureStore, t, coords, value);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%t:texture_storage_1d<rgba8unorm, write>, %coords:i32, %value:vec4<f32>):void {
  $B1: {
    %5:void = textureStore %t, %coords, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_storage_1d<rgba8unorm, write>, %coords:i32, %value:vec4<f32>):void {
  $B1: {
    %5:u32 = convert %coords
    %6:void = %t.write %value, %5
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureStore_2d_U32Coords) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* texture_ty =
        ty.storage_texture(core::type::TextureDimension::k2d, format, core::Access::kWrite);
    auto* t = b.FunctionParam("t", texture_ty);
    auto* coords = b.FunctionParam("coords", ty.vec2<u32>());
    auto* value = b.FunctionParam("value", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t, coords, value});
    b.Append(func->Block(), [&] {
        b.Call<void>(core::BuiltinFn::kTextureStore, t, coords, value);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%t:texture_storage_2d<rgba8unorm, write>, %coords:vec2<u32>, %value:vec4<f32>):void {
  $B1: {
    %5:void = textureStore %t, %coords, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_storage_2d<rgba8unorm, write>, %coords:vec2<u32>, %value:vec4<f32>):void {
  $B1: {
    %5:void = %t.write %value, %coords
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureStore_2d_I32Coords) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* texture_ty =
        ty.storage_texture(core::type::TextureDimension::k2d, format, core::Access::kWrite);
    auto* t = b.FunctionParam("t", texture_ty);
    auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
    auto* value = b.FunctionParam("value", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t, coords, value});
    b.Append(func->Block(), [&] {
        b.Call<void>(core::BuiltinFn::kTextureStore, t, coords, value);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%t:texture_storage_2d<rgba8unorm, write>, %coords:vec2<i32>, %value:vec4<f32>):void {
  $B1: {
    %5:void = textureStore %t, %coords, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_storage_2d<rgba8unorm, write>, %coords:vec2<i32>, %value:vec4<f32>):void {
  $B1: {
    %5:vec2<u32> = convert %coords
    %6:void = %t.write %value, %5
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureStore_2darray_U32Index) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* texture_ty =
        ty.storage_texture(core::type::TextureDimension::k2dArray, format, core::Access::kWrite);
    auto* t = b.FunctionParam("t", texture_ty);
    auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
    auto* index = b.FunctionParam("index", ty.u32());
    auto* value = b.FunctionParam("value", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t, coords, index, value});
    b.Append(func->Block(), [&] {
        b.Call<void>(core::BuiltinFn::kTextureStore, t, coords, index, value);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%t:texture_storage_2d_array<rgba8unorm, write>, %coords:vec2<i32>, %index:u32, %value:vec4<f32>):void {
  $B1: {
    %6:void = textureStore %t, %coords, %index, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_storage_2d_array<rgba8unorm, write>, %coords:vec2<i32>, %index:u32, %value:vec4<f32>):void {
  $B1: {
    %6:vec2<u32> = convert %coords
    %7:void = %t.write %value, %6, %index
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureStore_2darray_I32Index) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* texture_ty =
        ty.storage_texture(core::type::TextureDimension::k2dArray, format, core::Access::kWrite);
    auto* t = b.FunctionParam("t", texture_ty);
    auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
    auto* index = b.FunctionParam("index", ty.i32());
    auto* value = b.FunctionParam("value", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t, coords, index, value});
    b.Append(func->Block(), [&] {
        b.Call<void>(core::BuiltinFn::kTextureStore, t, coords, index, value);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%t:texture_storage_2d_array<rgba8unorm, write>, %coords:vec2<i32>, %index:i32, %value:vec4<f32>):void {
  $B1: {
    %6:void = textureStore %t, %coords, %index, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_storage_2d_array<rgba8unorm, write>, %coords:vec2<i32>, %index:i32, %value:vec4<f32>):void {
  $B1: {
    %6:vec2<u32> = convert %coords
    %7:void = %t.write %value, %6, %index
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureStore_3d) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* texture_ty =
        ty.storage_texture(core::type::TextureDimension::k3d, format, core::Access::kWrite);
    auto* t = b.FunctionParam("t", texture_ty);
    auto* coords = b.FunctionParam("coords", ty.vec3<i32>());
    auto* value = b.FunctionParam("value", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t, coords, value});
    b.Append(func->Block(), [&] {
        b.Call<void>(core::BuiltinFn::kTextureStore, t, coords, value);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%t:texture_storage_3d<rgba8unorm, write>, %coords:vec3<i32>, %value:vec4<f32>):void {
  $B1: {
    %5:void = textureStore %t, %coords, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_storage_3d<rgba8unorm, write>, %coords:vec3<i32>, %value:vec4<f32>):void {
  $B1: {
    %5:vec3<u32> = convert %coords
    %6:void = %t.write %value, %5
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

// Test that we insert a fence after the store to ensure that it is ordered before the load.
TEST_F(MslWriter_BuiltinPolyfillTest, TextureStoreToReadWriteBeforeLoad) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* texture_ty =
        ty.storage_texture(core::type::TextureDimension::k2d, format, core::Access::kReadWrite);
    auto* t = b.FunctionParam("t", texture_ty);
    auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
    auto* value = b.FunctionParam("value", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, coords, value});
    b.Append(func->Block(), [&] {
        b.Call<void>(core::BuiltinFn::kTextureStore, t, coords, value);
        auto* result = b.Call<vec4<f32>>(core::BuiltinFn::kTextureLoad, t, coords);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_storage_2d<rgba8unorm, read_write>, %coords:vec2<i32>, %value:vec4<f32>):vec4<f32> {
  $B1: {
    %5:void = textureStore %t, %coords, %value
    %6:vec4<f32> = textureLoad %t, %coords
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_storage_2d<rgba8unorm, read_write>, %coords:vec2<i32>, %value:vec4<f32>):vec4<f32> {
  $B1: {
    %5:vec2<u32> = convert %coords
    %6:void = %t.write %value, %5
    %7:void = %t.fence
    %8:vec2<u32> = convert %coords
    %9:vec4<f32> = %t.read %8
    ret %9
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, WorkgroupBarrier) {
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kWorkgroupBarrier);
        b.Return(func);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:void = workgroupBarrier
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:void = msl.threadgroup_barrier 4u
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, StorageBarrier) {
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kStorageBarrier);
        b.Return(func);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:void = storageBarrier
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:void = msl.threadgroup_barrier 1u
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureBarrier) {
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kTextureBarrier);
        b.Return(func);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:void = textureBarrier
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:void = msl.threadgroup_barrier 2u
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, Pack2x16Float) {
    auto* func = b.Function("foo", ty.u32());
    auto* input = b.FunctionParam("input", ty.vec2<f32>());
    func->SetParams(Vector{input});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<u32>(core::BuiltinFn::kPack2X16Float, input);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%input:vec2<f32>):u32 {
  $B1: {
    %3:u32 = pack2x16float %input
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%input:vec2<f32>):u32 {
  $B1: {
    %3:vec2<f16> = convert %input
    %4:u32 = bitcast %3
    ret %4
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, Unpack2x16Float) {
    auto* func = b.Function("foo", ty.vec2<f32>());
    auto* input = b.FunctionParam("input", ty.u32());
    func->SetParams(Vector{input});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec2<f32>>(core::BuiltinFn::kUnpack2X16Float, input);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%input:u32):vec2<f32> {
  $B1: {
    %3:vec2<f32> = unpack2x16float %input
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%input:u32):vec2<f32> {
  $B1: {
    %3:vec2<f16> = bitcast %input
    %4:vec2<f32> = convert %3
    ret %4
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, SubgroupMatrixLoad_Storage_F32) {
    auto* mat = ty.subgroup_matrix_result(ty.f32(), 8, 8);
    auto* p = b.FunctionParam<ptr<storage, array<f32, 256>>>("p");
    auto* func = b.Function("foo", mat);
    func->SetParams({p});
    b.Append(func->Block(), [&] {
        auto* call = b.CallExplicit(mat, core::BuiltinFn::kSubgroupMatrixLoad, Vector{mat}, p, 64_u,
                                    false, 32_u);
        b.Return(func, call);
    });

    auto* src = R"(
%foo = func(%p:ptr<storage, array<f32, 256>, read_write>):subgroup_matrix_result<f32, 8, 8> {
  $B1: {
    %3:subgroup_matrix_result<f32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_result<f32, 8, 8>> %p, 64u, false, 32u
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%p:ptr<storage, array<f32, 256>, read_write>):subgroup_matrix_result<f32, 8, 8> {
  $B1: {
    %3:ptr<storage, f32, read_write> = access %p, 64u
    %4:u64 = msl.convert 32u
    %5:ptr<function, subgroup_matrix_result<f32, 8, 8>, read_write> = var undef
    %6:subgroup_matrix_result<f32, 8, 8> = load %5
    %7:void = msl.simdgroup_load %6, %3, %4, vec2<u64>(0u64), false
    %8:subgroup_matrix_result<f32, 8, 8> = load %5
    ret %8
  }
}
)";

    capabilities.Add(core::ir::Capability::kAllow64BitIntegers);
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, SubgroupMatrixLoad_Workgroup_F16) {
    auto* mat = ty.subgroup_matrix_result(ty.f16(), 8, 8);
    auto* p = b.FunctionParam<ptr<workgroup, array<f16, 256>>>("p");
    auto* func = b.Function("foo", mat);
    func->SetParams({p});
    b.Append(func->Block(), [&] {
        auto* call = b.CallExplicit(mat, core::BuiltinFn::kSubgroupMatrixLoad, Vector{mat}, p, 64_u,
                                    false, 32_u);
        b.Return(func, call);
    });

    auto* src = R"(
%foo = func(%p:ptr<workgroup, array<f16, 256>, read_write>):subgroup_matrix_result<f16, 8, 8> {
  $B1: {
    %3:subgroup_matrix_result<f16, 8, 8> = subgroupMatrixLoad<subgroup_matrix_result<f16, 8, 8>> %p, 64u, false, 32u
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%p:ptr<workgroup, array<f16, 256>, read_write>):subgroup_matrix_result<f16, 8, 8> {
  $B1: {
    %3:ptr<workgroup, f16, read_write> = access %p, 64u
    %4:u64 = msl.convert 32u
    %5:ptr<function, subgroup_matrix_result<f16, 8, 8>, read_write> = var undef
    %6:subgroup_matrix_result<f16, 8, 8> = load %5
    %7:void = msl.simdgroup_load %6, %3, %4, vec2<u64>(0u64), false
    %8:subgroup_matrix_result<f16, 8, 8> = load %5
    ret %8
  }
}
)";

    capabilities.Add(core::ir::Capability::kAllow64BitIntegers);
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, SubgroupMatrixStore_Storage_F32) {
    auto* p = b.FunctionParam<ptr<storage, array<f32, 256>>>("p");
    auto* m = b.FunctionParam("m", ty.subgroup_matrix_result(ty.f32(), 8, 8));
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({p, m});
    b.Append(func->Block(), [&] {
        b.Call<void>(core::BuiltinFn::kSubgroupMatrixStore, p, 64_u, m, false, 32_u);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%p:ptr<storage, array<f32, 256>, read_write>, %m:subgroup_matrix_result<f32, 8, 8>):void {
  $B1: {
    %4:void = subgroupMatrixStore %p, 64u, %m, false, 32u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%p:ptr<storage, array<f32, 256>, read_write>, %m:subgroup_matrix_result<f32, 8, 8>):void {
  $B1: {
    %4:ptr<storage, f32, read_write> = access %p, 64u
    %5:u64 = msl.convert 32u
    %6:void = msl.simdgroup_store %m, %4, %5, vec2<u64>(0u64), false
    ret
  }
}
)";

    capabilities.Add(core::ir::Capability::kAllow64BitIntegers);
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, SubgroupMatrixStore_Workgroup_F16) {
    auto* p = b.FunctionParam<ptr<workgroup, array<f16, 256>>>("p");
    auto* m = b.FunctionParam("m", ty.subgroup_matrix_result(ty.f16(), 8, 8));
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({p, m});
    b.Append(func->Block(), [&] {
        b.Call<void>(core::BuiltinFn::kSubgroupMatrixStore, p, 64_u, m, false, 32_u);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%p:ptr<workgroup, array<f16, 256>, read_write>, %m:subgroup_matrix_result<f16, 8, 8>):void {
  $B1: {
    %4:void = subgroupMatrixStore %p, 64u, %m, false, 32u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%p:ptr<workgroup, array<f16, 256>, read_write>, %m:subgroup_matrix_result<f16, 8, 8>):void {
  $B1: {
    %4:ptr<workgroup, f16, read_write> = access %p, 64u
    %5:u64 = msl.convert 32u
    %6:void = msl.simdgroup_store %m, %4, %5, vec2<u64>(0u64), false
    ret
  }
}
)";

    capabilities.Add(core::ir::Capability::kAllow64BitIntegers);
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, SubgroupMatrixMultiply_F32) {
    auto* left = b.FunctionParam("left", ty.subgroup_matrix_left(ty.f32(), 4, 8));
    auto* right = b.FunctionParam("right", ty.subgroup_matrix_right(ty.f32(), 8, 4));
    auto* result = ty.subgroup_matrix_result(ty.f32(), 8, 8);
    auto* func = b.Function("foo", result);
    func->SetParams({left, right});
    b.Append(func->Block(), [&] {
        auto* call = b.CallExplicit(result, core::BuiltinFn::kSubgroupMatrixMultiply,
                                    Vector{ty.f32()}, left, right);
        b.Return(func, call);
    });

    auto* src = R"(
%foo = func(%left:subgroup_matrix_left<f32, 4, 8>, %right:subgroup_matrix_right<f32, 8, 4>):subgroup_matrix_result<f32, 8, 8> {
  $B1: {
    %4:subgroup_matrix_result<f32, 8, 8> = subgroupMatrixMultiply<f32> %left, %right
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%left:subgroup_matrix_left<f32, 4, 8>, %right:subgroup_matrix_right<f32, 8, 4>):subgroup_matrix_result<f32, 8, 8> {
  $B1: {
    %4:ptr<function, subgroup_matrix_result<f32, 8, 8>, read_write> = var undef
    %5:subgroup_matrix_result<f32, 8, 8> = load %4
    %6:void = msl.simdgroup_multiply %5, %left, %right
    %7:subgroup_matrix_result<f32, 8, 8> = load %4
    ret %7
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, SubgroupMatrixMultiply_F16) {
    auto* left = b.FunctionParam("left", ty.subgroup_matrix_left(ty.f16(), 8, 4));
    auto* right = b.FunctionParam("right", ty.subgroup_matrix_right(ty.f16(), 2, 8));
    auto* result = ty.subgroup_matrix_result(ty.f16(), 2, 4);
    auto* func = b.Function("foo", result);
    func->SetParams({left, right});
    b.Append(func->Block(), [&] {
        auto* call = b.CallExplicit(result, core::BuiltinFn::kSubgroupMatrixMultiply,
                                    Vector{ty.f16()}, left, right);
        b.Return(func, call);
    });

    auto* src = R"(
%foo = func(%left:subgroup_matrix_left<f16, 8, 4>, %right:subgroup_matrix_right<f16, 2, 8>):subgroup_matrix_result<f16, 2, 4> {
  $B1: {
    %4:subgroup_matrix_result<f16, 2, 4> = subgroupMatrixMultiply<f16> %left, %right
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%left:subgroup_matrix_left<f16, 8, 4>, %right:subgroup_matrix_right<f16, 2, 8>):subgroup_matrix_result<f16, 2, 4> {
  $B1: {
    %4:ptr<function, subgroup_matrix_result<f16, 2, 4>, read_write> = var undef
    %5:subgroup_matrix_result<f16, 2, 4> = load %4
    %6:void = msl.simdgroup_multiply %5, %left, %right
    %7:subgroup_matrix_result<f16, 2, 4> = load %4
    ret %7
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, SubgroupMatrixMultiplyAccumulate_F32) {
    auto* left = b.FunctionParam("left", ty.subgroup_matrix_left(ty.f32(), 4, 8));
    auto* right = b.FunctionParam("right", ty.subgroup_matrix_right(ty.f32(), 8, 4));
    auto* acc = b.FunctionParam("acc", ty.subgroup_matrix_result(ty.f32(), 8, 8));
    auto* result = ty.subgroup_matrix_result(ty.f32(), 8, 8);
    auto* func = b.Function("foo", result);
    func->SetParams({left, right, acc});
    b.Append(func->Block(), [&] {
        auto* call =
            b.Call(result, core::BuiltinFn::kSubgroupMatrixMultiplyAccumulate, left, right, acc);
        b.Return(func, call);
    });

    auto* src = R"(
%foo = func(%left:subgroup_matrix_left<f32, 4, 8>, %right:subgroup_matrix_right<f32, 8, 4>, %acc:subgroup_matrix_result<f32, 8, 8>):subgroup_matrix_result<f32, 8, 8> {
  $B1: {
    %5:subgroup_matrix_result<f32, 8, 8> = subgroupMatrixMultiplyAccumulate %left, %right, %acc
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%left:subgroup_matrix_left<f32, 4, 8>, %right:subgroup_matrix_right<f32, 8, 4>, %acc:subgroup_matrix_result<f32, 8, 8>):subgroup_matrix_result<f32, 8, 8> {
  $B1: {
    %5:ptr<function, subgroup_matrix_result<f32, 8, 8>, read_write> = var undef
    %6:subgroup_matrix_result<f32, 8, 8> = load %5
    %7:void = msl.simdgroup_multiply_accumulate %6, %left, %right, %acc
    %8:subgroup_matrix_result<f32, 8, 8> = load %5
    ret %8
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, SubgroupMatrixMultiplyAccumulate_F16) {
    auto* left = b.FunctionParam("left", ty.subgroup_matrix_left(ty.f16(), 8, 4));
    auto* right = b.FunctionParam("right", ty.subgroup_matrix_right(ty.f16(), 2, 8));
    auto* acc = b.FunctionParam("acc", ty.subgroup_matrix_result(ty.f16(), 2, 4));
    auto* result = ty.subgroup_matrix_result(ty.f16(), 2, 4);
    auto* func = b.Function("foo", result);
    func->SetParams({left, right, acc});
    b.Append(func->Block(), [&] {
        auto* call =
            b.Call(result, core::BuiltinFn::kSubgroupMatrixMultiplyAccumulate, left, right, acc);
        b.Return(func, call);
    });

    auto* src = R"(
%foo = func(%left:subgroup_matrix_left<f16, 8, 4>, %right:subgroup_matrix_right<f16, 2, 8>, %acc:subgroup_matrix_result<f16, 2, 4>):subgroup_matrix_result<f16, 2, 4> {
  $B1: {
    %5:subgroup_matrix_result<f16, 2, 4> = subgroupMatrixMultiplyAccumulate %left, %right, %acc
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%left:subgroup_matrix_left<f16, 8, 4>, %right:subgroup_matrix_right<f16, 2, 8>, %acc:subgroup_matrix_result<f16, 2, 4>):subgroup_matrix_result<f16, 2, 4> {
  $B1: {
    %5:ptr<function, subgroup_matrix_result<f16, 2, 4>, read_write> = var undef
    %6:subgroup_matrix_result<f16, 2, 4> = load %5
    %7:void = msl.simdgroup_multiply_accumulate %6, %left, %right, %acc
    %8:subgroup_matrix_result<f16, 2, 4> = load %5
    ret %8
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::msl::writer::raise

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

#include "src/tint/lang/core/ir/transform/zero_init_workgroup_memory.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class IR_ZeroInitWorkgroupMemoryTest : public TransformTest {
  protected:
    Function* MakeEntryPoint(const char* name,
                             uint32_t wgsize_x,
                             uint32_t wgsize_y,
                             uint32_t wgsize_z) {
        auto* func = b.ComputeFunction(name, u32(wgsize_x), u32(wgsize_y), u32(wgsize_z));
        return func;
    }

    Var* MakeVar(const char* name, const type::Type* store_type) {
        auto* var = b.Var(name, ty.ptr(workgroup, store_type));
        mod.root_block->Append(var);
        return var;
    }
};

TEST_F(IR_ZeroInitWorkgroupMemoryTest, NoRootBlock) {
    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    auto* expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, WorkgroupVarUnused) {
    MakeVar("wgvar", ty.i32());

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, i32, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, NonWorkgroupVar) {
    auto* var = b.Var("pvar", ty.ptr(private_, ty.bool_()));
    mod.root_block->Append(var);

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %pvar:ptr<private, bool, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:bool = load %pvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %pvar:ptr<private, bool, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:bool = load %pvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, ScalarBool) {
    auto* var = MakeVar("wgvar", ty.bool_());

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, bool, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:bool = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, bool, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func(%tint_local_index:u32 [@local_invocation_index]):void {
  $B2: {
    %4:bool = lt %tint_local_index, 1u
    if %4 [t: $B3] {  # if_1
      $B3: {  # true
        store %wgvar, false
        exit_if  # if_1
      }
    }
    %5:void = workgroupBarrier
    %6:bool = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, ScalarI32) {
    auto* var = MakeVar("wgvar", ty.i32());

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, i32, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:i32 = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, i32, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func(%tint_local_index:u32 [@local_invocation_index]):void {
  $B2: {
    %4:bool = lt %tint_local_index, 1u
    if %4 [t: $B3] {  # if_1
      $B3: {  # true
        store %wgvar, 0i
        exit_if  # if_1
      }
    }
    %5:void = workgroupBarrier
    %6:i32 = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, ScalarU32) {
    auto* var = MakeVar("wgvar", ty.u32());

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, u32, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, u32, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func(%tint_local_index:u32 [@local_invocation_index]):void {
  $B2: {
    %4:bool = lt %tint_local_index, 1u
    if %4 [t: $B3] {  # if_1
      $B3: {  # true
        store %wgvar, 0u
        exit_if  # if_1
      }
    }
    %5:void = workgroupBarrier
    %6:u32 = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, ScalarF32) {
    auto* var = MakeVar("wgvar", ty.f32());

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, f32, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:f32 = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, f32, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func(%tint_local_index:u32 [@local_invocation_index]):void {
  $B2: {
    %4:bool = lt %tint_local_index, 1u
    if %4 [t: $B3] {  # if_1
      $B3: {  # true
        store %wgvar, 0.0f
        exit_if  # if_1
      }
    }
    %5:void = workgroupBarrier
    %6:f32 = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, ScalarF16) {
    auto* var = MakeVar("wgvar", ty.f16());

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, f16, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:f16 = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, f16, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func(%tint_local_index:u32 [@local_invocation_index]):void {
  $B2: {
    %4:bool = lt %tint_local_index, 1u
    if %4 [t: $B3] {  # if_1
      $B3: {  # true
        store %wgvar, 0.0h
        exit_if  # if_1
      }
    }
    %5:void = workgroupBarrier
    %6:f16 = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, AtomicI32) {
    auto* var = MakeVar("wgvar", ty.atomic<i32>());

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Call(ty.i32(), core::BuiltinFn::kAtomicLoad, var);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, atomic<i32>, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:i32 = atomicLoad %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, atomic<i32>, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func(%tint_local_index:u32 [@local_invocation_index]):void {
  $B2: {
    %4:bool = lt %tint_local_index, 1u
    if %4 [t: $B3] {  # if_1
      $B3: {  # true
        %5:void = atomicStore %wgvar, 0i
        exit_if  # if_1
      }
    }
    %6:void = workgroupBarrier
    %7:i32 = atomicLoad %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, AtomicU32) {
    auto* var = MakeVar("wgvar", ty.atomic<u32>());

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Call(ty.u32(), core::BuiltinFn::kAtomicLoad, var);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, atomic<u32>, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = atomicLoad %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, atomic<u32>, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func(%tint_local_index:u32 [@local_invocation_index]):void {
  $B2: {
    %4:bool = lt %tint_local_index, 1u
    if %4 [t: $B3] {  # if_1
      $B3: {  # true
        %5:void = atomicStore %wgvar, 0u
        exit_if  # if_1
      }
    }
    %6:void = workgroupBarrier
    %7:u32 = atomicLoad %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, ArrayOfI32) {
    auto* var = MakeVar("wgvar", ty.array<i32, 4>());

    auto* func = MakeEntryPoint("main", 11, 2, 3);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, array<i32, 4>, read_write> = var
}

%main = @compute @workgroup_size(11u, 2u, 3u) func():void {
  $B2: {
    %3:array<i32, 4> = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, array<i32, 4>, read_write> = var
}

%main = @compute @workgroup_size(11u, 2u, 3u) func(%tint_local_index:u32 [@local_invocation_index]):void {
  $B2: {
    %4:bool = lt %tint_local_index, 4u
    if %4 [t: $B3] {  # if_1
      $B3: {  # true
        %5:ptr<workgroup, i32, read_write> = access %wgvar, %tint_local_index
        store %5, 0i
        exit_if  # if_1
      }
    }
    %6:void = workgroupBarrier
    %7:array<i32, 4> = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, ArrayOfArrayOfU32) {
    auto* var = MakeVar("wgvar", ty.array(ty.array<u32, 5>(), 7));

    auto* func = MakeEntryPoint("main", 11, 2, 3);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, array<array<u32, 5>, 7>, read_write> = var
}

%main = @compute @workgroup_size(11u, 2u, 3u) func():void {
  $B2: {
    %3:array<array<u32, 5>, 7> = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, array<array<u32, 5>, 7>, read_write> = var
}

%main = @compute @workgroup_size(11u, 2u, 3u) func(%tint_local_index:u32 [@local_invocation_index]):void {
  $B2: {
    %4:bool = lt %tint_local_index, 35u
    if %4 [t: $B3] {  # if_1
      $B3: {  # true
        %5:u32 = mod %tint_local_index, 5u
        %6:u32 = div %tint_local_index, 5u
        %7:ptr<workgroup, u32, read_write> = access %wgvar, %6, %5
        store %7, 0u
        exit_if  # if_1
      }
    }
    %8:void = workgroupBarrier
    %9:array<array<u32, 5>, 7> = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, ArrayOfArrayOfArray) {
    auto* var = MakeVar("wgvar", ty.array(ty.array(ty.array<i32, 7>(), 5), 3));

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, array<array<array<i32, 7>, 5>, 3>, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:array<array<array<i32, 7>, 5>, 3> = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, array<array<array<i32, 7>, 5>, 3>, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func(%tint_local_index:u32 [@local_invocation_index]):void {
  $B2: {
    loop [i: $B3, b: $B4, c: $B5] {  # loop_1
      $B3: {  # initializer
        next_iteration %tint_local_index  # -> $B4
      }
      $B4 (%idx:u32): {  # body
        %5:bool = gte %idx, 105u
        if %5 [t: $B6] {  # if_1
          $B6: {  # true
            exit_loop  # loop_1
          }
        }
        %6:u32 = mod %idx, 7u
        %7:u32 = div %idx, 7u
        %8:u32 = mod %7, 5u
        %9:u32 = div %idx, 35u
        %10:ptr<workgroup, i32, read_write> = access %wgvar, %9, %8, %6
        store %10, 0i
        continue  # -> $B5
      }
      $B5: {  # continuing
        %11:u32 = add %idx, 1u
        next_iteration %11  # -> $B4
      }
    }
    %12:void = workgroupBarrier
    %13:array<array<array<i32, 7>, 5>, 3> = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, NestedArrayInnerSizeOne) {
    auto* var = MakeVar("wgvar", ty.array(ty.array(ty.array<i32, 1>(), 5), 3));

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, array<array<array<i32, 1>, 5>, 3>, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:array<array<array<i32, 1>, 5>, 3> = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, array<array<array<i32, 1>, 5>, 3>, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func(%tint_local_index:u32 [@local_invocation_index]):void {
  $B2: {
    loop [i: $B3, b: $B4, c: $B5] {  # loop_1
      $B3: {  # initializer
        next_iteration %tint_local_index  # -> $B4
      }
      $B4 (%idx:u32): {  # body
        %5:bool = gte %idx, 15u
        if %5 [t: $B6] {  # if_1
          $B6: {  # true
            exit_loop  # loop_1
          }
        }
        %6:u32 = mod %idx, 5u
        %7:u32 = div %idx, 5u
        %8:ptr<workgroup, i32, read_write> = access %wgvar, %7, %6, 0u
        store %8, 0i
        continue  # -> $B5
      }
      $B5: {  # continuing
        %9:u32 = add %idx, 1u
        next_iteration %9  # -> $B4
      }
    }
    %10:void = workgroupBarrier
    %11:array<array<array<i32, 1>, 5>, 3> = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, NestedArrayMiddleSizeOne) {
    auto* var = MakeVar("wgvar", ty.array(ty.array(ty.array<i32, 3>(), 1), 5));

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, array<array<array<i32, 3>, 1>, 5>, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:array<array<array<i32, 3>, 1>, 5> = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, array<array<array<i32, 3>, 1>, 5>, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func(%tint_local_index:u32 [@local_invocation_index]):void {
  $B2: {
    loop [i: $B3, b: $B4, c: $B5] {  # loop_1
      $B3: {  # initializer
        next_iteration %tint_local_index  # -> $B4
      }
      $B4 (%idx:u32): {  # body
        %5:bool = gte %idx, 15u
        if %5 [t: $B6] {  # if_1
          $B6: {  # true
            exit_loop  # loop_1
          }
        }
        %6:u32 = mod %idx, 3u
        %7:u32 = div %idx, 3u
        %8:ptr<workgroup, i32, read_write> = access %wgvar, %7, 0u, %6
        store %8, 0i
        continue  # -> $B5
      }
      $B5: {  # continuing
        %9:u32 = add %idx, 1u
        next_iteration %9  # -> $B4
      }
    }
    %10:void = workgroupBarrier
    %11:array<array<array<i32, 3>, 1>, 5> = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, NestedArrayOuterSizeOne) {
    auto* var = MakeVar("wgvar", ty.array(ty.array(ty.array<i32, 3>(), 5), 1));

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, array<array<array<i32, 3>, 5>, 1>, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:array<array<array<i32, 3>, 5>, 1> = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, array<array<array<i32, 3>, 5>, 1>, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func(%tint_local_index:u32 [@local_invocation_index]):void {
  $B2: {
    loop [i: $B3, b: $B4, c: $B5] {  # loop_1
      $B3: {  # initializer
        next_iteration %tint_local_index  # -> $B4
      }
      $B4 (%idx:u32): {  # body
        %5:bool = gte %idx, 15u
        if %5 [t: $B6] {  # if_1
          $B6: {  # true
            exit_loop  # loop_1
          }
        }
        %6:u32 = mod %idx, 3u
        %7:u32 = div %idx, 3u
        %8:ptr<workgroup, i32, read_write> = access %wgvar, 0u, %7, %6
        store %8, 0i
        continue  # -> $B5
      }
      $B5: {  # continuing
        %9:u32 = add %idx, 1u
        next_iteration %9  # -> $B4
      }
    }
    %10:void = workgroupBarrier
    %11:array<array<array<i32, 3>, 5>, 1> = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, NestedArrayTotalSizeOne) {
    auto* var = MakeVar("wgvar", ty.array(ty.array<i32, 1>(), 1));

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, array<array<i32, 1>, 1>, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:array<array<i32, 1>, 1> = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, array<array<i32, 1>, 1>, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func(%tint_local_index:u32 [@local_invocation_index]):void {
  $B2: {
    %4:bool = lt %tint_local_index, 1u
    if %4 [t: $B3] {  # if_1
      $B3: {  # true
        %5:ptr<workgroup, i32, read_write> = access %wgvar, 0u, 0u
        store %5, 0i
        exit_if  # if_1
      }
    }
    %6:void = workgroupBarrier
    %7:array<array<i32, 1>, 1> = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, StructOfScalars) {
    auto* s = ty.Struct(mod.symbols.New("MyStruct"), {
                                                         {mod.symbols.New("a"), ty.i32()},
                                                         {mod.symbols.New("b"), ty.u32()},
                                                         {mod.symbols.New("c"), ty.f32()},
                                                     });
    auto* var = MakeVar("wgvar", s);

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
  c:f32 @offset(8)
}

$B1: {  # root
  %wgvar:ptr<workgroup, MyStruct, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:MyStruct = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
  c:f32 @offset(8)
}

$B1: {  # root
  %wgvar:ptr<workgroup, MyStruct, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func(%tint_local_index:u32 [@local_invocation_index]):void {
  $B2: {
    %4:bool = lt %tint_local_index, 1u
    if %4 [t: $B3] {  # if_1
      $B3: {  # true
        store %wgvar, MyStruct(0i, 0u, 0.0f)
        exit_if  # if_1
      }
    }
    %5:void = workgroupBarrier
    %6:MyStruct = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, NestedStructOfScalars) {
    auto* inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("a"), ty.i32()},
                                                          {mod.symbols.New("b"), ty.u32()},
                                                      });
    auto* outer = ty.Struct(mod.symbols.New("Outer"), {
                                                          {mod.symbols.New("c"), ty.f32()},
                                                          {mod.symbols.New("inner"), inner},
                                                          {mod.symbols.New("d"), ty.bool_()},
                                                      });
    auto* var = MakeVar("wgvar", outer);

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

Outer = struct @align(4) {
  c:f32 @offset(0)
  inner:Inner @offset(4)
  d:bool @offset(12)
}

$B1: {  # root
  %wgvar:ptr<workgroup, Outer, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:Outer = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

Outer = struct @align(4) {
  c:f32 @offset(0)
  inner:Inner @offset(4)
  d:bool @offset(12)
}

$B1: {  # root
  %wgvar:ptr<workgroup, Outer, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func(%tint_local_index:u32 [@local_invocation_index]):void {
  $B2: {
    %4:bool = lt %tint_local_index, 1u
    if %4 [t: $B3] {  # if_1
      $B3: {  # true
        store %wgvar, Outer(0.0f, Inner(0i, 0u), false)
        exit_if  # if_1
      }
    }
    %5:void = workgroupBarrier
    %6:Outer = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, NestedStructOfScalarsWithAtomic) {
    auto* inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("a"), ty.i32()},
                                                          {mod.symbols.New("b"), ty.atomic<u32>()},
                                                      });
    auto* outer = ty.Struct(mod.symbols.New("Outer"), {
                                                          {mod.symbols.New("c"), ty.f32()},
                                                          {mod.symbols.New("inner"), inner},
                                                          {mod.symbols.New("d"), ty.bool_()},
                                                      });
    auto* var = MakeVar("wgvar", outer);

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(4) {
  a:i32 @offset(0)
  b:atomic<u32> @offset(4)
}

Outer = struct @align(4) {
  c:f32 @offset(0)
  inner:Inner @offset(4)
  d:bool @offset(12)
}

$B1: {  # root
  %wgvar:ptr<workgroup, Outer, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:Outer = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(4) {
  a:i32 @offset(0)
  b:atomic<u32> @offset(4)
}

Outer = struct @align(4) {
  c:f32 @offset(0)
  inner:Inner @offset(4)
  d:bool @offset(12)
}

$B1: {  # root
  %wgvar:ptr<workgroup, Outer, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func(%tint_local_index:u32 [@local_invocation_index]):void {
  $B2: {
    %4:bool = lt %tint_local_index, 1u
    if %4 [t: $B3] {  # if_1
      $B3: {  # true
        %5:ptr<workgroup, f32, read_write> = access %wgvar, 0u
        store %5, 0.0f
        %6:ptr<workgroup, i32, read_write> = access %wgvar, 1u, 0u
        store %6, 0i
        %7:ptr<workgroup, atomic<u32>, read_write> = access %wgvar, 1u, 1u
        %8:void = atomicStore %7, 0u
        %9:ptr<workgroup, bool, read_write> = access %wgvar, 2u
        store %9, false
        exit_if  # if_1
      }
    }
    %10:void = workgroupBarrier
    %11:Outer = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, ArrayOfStructOfArrayOfStructWithAtomic) {
    auto* inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("a"), ty.i32()},
                                                          {mod.symbols.New("b"), ty.atomic<u32>()},
                                                      });
    auto* outer =
        ty.Struct(mod.symbols.New("Outer"), {
                                                {mod.symbols.New("c"), ty.f32()},
                                                {mod.symbols.New("inner"), ty.array(inner, 13)},
                                                {mod.symbols.New("d"), ty.bool_()},
                                            });
    auto* var = MakeVar("wgvar", ty.array(outer, 7));

    auto* func = MakeEntryPoint("main", 7, 3, 2);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(4) {
  a:i32 @offset(0)
  b:atomic<u32> @offset(4)
}

Outer = struct @align(4) {
  c:f32 @offset(0)
  inner:array<Inner, 13> @offset(4)
  d:bool @offset(108)
}

$B1: {  # root
  %wgvar:ptr<workgroup, array<Outer, 7>, read_write> = var
}

%main = @compute @workgroup_size(7u, 3u, 2u) func():void {
  $B2: {
    %3:array<Outer, 7> = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(4) {
  a:i32 @offset(0)
  b:atomic<u32> @offset(4)
}

Outer = struct @align(4) {
  c:f32 @offset(0)
  inner:array<Inner, 13> @offset(4)
  d:bool @offset(108)
}

$B1: {  # root
  %wgvar:ptr<workgroup, array<Outer, 7>, read_write> = var
}

%main = @compute @workgroup_size(7u, 3u, 2u) func(%tint_local_index:u32 [@local_invocation_index]):void {
  $B2: {
    %4:bool = lt %tint_local_index, 7u
    if %4 [t: $B3] {  # if_1
      $B3: {  # true
        %5:ptr<workgroup, f32, read_write> = access %wgvar, %tint_local_index, 0u
        store %5, 0.0f
        %6:ptr<workgroup, bool, read_write> = access %wgvar, %tint_local_index, 2u
        store %6, false
        exit_if  # if_1
      }
    }
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration %tint_local_index  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %8:bool = gte %idx, 91u
        if %8 [t: $B7] {  # if_2
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %9:u32 = mod %idx, 13u
        %10:u32 = div %idx, 13u
        %11:ptr<workgroup, i32, read_write> = access %wgvar, %10, 1u, %9, 0u
        store %11, 0i
        %12:u32 = mod %idx, 13u
        %13:u32 = div %idx, 13u
        %14:ptr<workgroup, atomic<u32>, read_write> = access %wgvar, %13, 1u, %12, 1u
        %15:void = atomicStore %14, 0u
        continue  # -> $B6
      }
      $B6: {  # continuing
        %16:u32 = add %idx, 42u
        next_iteration %16  # -> $B5
      }
    }
    %17:void = workgroupBarrier
    %18:array<Outer, 7> = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, MultipleVariables_DifferentIterationCounts) {
    auto* var_a = MakeVar("var_a", ty.bool_());
    auto* var_b = MakeVar("var_b", ty.array<i32, 4>());
    auto* var_c = MakeVar("var_c", ty.array(ty.array<u32, 5>(), 7));

    auto* func = MakeEntryPoint("main", 11, 2, 3);
    b.Append(func->Block(), [&] {  //
        b.Load(var_a);
        b.Load(var_b);
        b.Load(var_c);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %var_a:ptr<workgroup, bool, read_write> = var
  %var_b:ptr<workgroup, array<i32, 4>, read_write> = var
  %var_c:ptr<workgroup, array<array<u32, 5>, 7>, read_write> = var
}

%main = @compute @workgroup_size(11u, 2u, 3u) func():void {
  $B2: {
    %5:bool = load %var_a
    %6:array<i32, 4> = load %var_b
    %7:array<array<u32, 5>, 7> = load %var_c
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %var_a:ptr<workgroup, bool, read_write> = var
  %var_b:ptr<workgroup, array<i32, 4>, read_write> = var
  %var_c:ptr<workgroup, array<array<u32, 5>, 7>, read_write> = var
}

%main = @compute @workgroup_size(11u, 2u, 3u) func(%tint_local_index:u32 [@local_invocation_index]):void {
  $B2: {
    %6:bool = lt %tint_local_index, 1u
    if %6 [t: $B3] {  # if_1
      $B3: {  # true
        store %var_a, false
        exit_if  # if_1
      }
    }
    %7:bool = lt %tint_local_index, 4u
    if %7 [t: $B4] {  # if_2
      $B4: {  # true
        %8:ptr<workgroup, i32, read_write> = access %var_b, %tint_local_index
        store %8, 0i
        exit_if  # if_2
      }
    }
    %9:bool = lt %tint_local_index, 35u
    if %9 [t: $B5] {  # if_3
      $B5: {  # true
        %10:u32 = mod %tint_local_index, 5u
        %11:u32 = div %tint_local_index, 5u
        %12:ptr<workgroup, u32, read_write> = access %var_c, %11, %10
        store %12, 0u
        exit_if  # if_3
      }
    }
    %13:void = workgroupBarrier
    %14:bool = load %var_a
    %15:array<i32, 4> = load %var_b
    %16:array<array<u32, 5>, 7> = load %var_c
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, MultipleVariables_SharedIterationCounts) {
    auto* var_a = MakeVar("var_a", ty.bool_());
    auto* var_b = MakeVar("var_b", ty.i32());
    auto* var_c = MakeVar("var_c", ty.array<i32, 42>());
    auto* var_d = MakeVar("var_d", ty.array(ty.array<u32, 6>(), 7));

    auto* func = MakeEntryPoint("main", 11, 2, 3);
    b.Append(func->Block(), [&] {  //
        b.Load(var_a);
        b.Load(var_b);
        b.Load(var_c);
        b.Load(var_d);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %var_a:ptr<workgroup, bool, read_write> = var
  %var_b:ptr<workgroup, i32, read_write> = var
  %var_c:ptr<workgroup, array<i32, 42>, read_write> = var
  %var_d:ptr<workgroup, array<array<u32, 6>, 7>, read_write> = var
}

%main = @compute @workgroup_size(11u, 2u, 3u) func():void {
  $B2: {
    %6:bool = load %var_a
    %7:i32 = load %var_b
    %8:array<i32, 42> = load %var_c
    %9:array<array<u32, 6>, 7> = load %var_d
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %var_a:ptr<workgroup, bool, read_write> = var
  %var_b:ptr<workgroup, i32, read_write> = var
  %var_c:ptr<workgroup, array<i32, 42>, read_write> = var
  %var_d:ptr<workgroup, array<array<u32, 6>, 7>, read_write> = var
}

%main = @compute @workgroup_size(11u, 2u, 3u) func(%tint_local_index:u32 [@local_invocation_index]):void {
  $B2: {
    %7:bool = lt %tint_local_index, 1u
    if %7 [t: $B3] {  # if_1
      $B3: {  # true
        store %var_a, false
        store %var_b, 0i
        exit_if  # if_1
      }
    }
    %8:bool = lt %tint_local_index, 42u
    if %8 [t: $B4] {  # if_2
      $B4: {  # true
        %9:ptr<workgroup, i32, read_write> = access %var_c, %tint_local_index
        store %9, 0i
        %10:u32 = mod %tint_local_index, 6u
        %11:u32 = div %tint_local_index, 6u
        %12:ptr<workgroup, u32, read_write> = access %var_d, %11, %10
        store %12, 0u
        exit_if  # if_2
      }
    }
    %13:void = workgroupBarrier
    %14:bool = load %var_a
    %15:i32 = load %var_b
    %16:array<i32, 42> = load %var_c
    %17:array<array<u32, 6>, 7> = load %var_d
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, ExistingLocalInvocationIndex) {
    auto* var = MakeVar("wgvar", ty.bool_());

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    auto* global_id = b.FunctionParam("global_id", ty.vec3<u32>());
    global_id->SetBuiltin(BuiltinValue::kGlobalInvocationId);
    auto* index = b.FunctionParam("index", ty.u32());
    index->SetBuiltin(BuiltinValue::kLocalInvocationIndex);
    func->SetParams({global_id, index});
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, bool, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func(%global_id:vec3<u32> [@global_invocation_id], %index:u32 [@local_invocation_index]):void {
  $B2: {
    %5:bool = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, bool, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func(%global_id:vec3<u32> [@global_invocation_id], %index:u32 [@local_invocation_index]):void {
  $B2: {
    %5:bool = lt %index, 1u
    if %5 [t: $B3] {  # if_1
      $B3: {  # true
        store %wgvar, false
        exit_if  # if_1
      }
    }
    %6:void = workgroupBarrier
    %7:bool = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, ExistingLocalInvocationIndexInStruct) {
    auto* var = MakeVar("wgvar", ty.bool_());

    auto* structure = ty.Struct(mod.symbols.New("MyStruct"),
                                {
                                    {
                                        mod.symbols.New("global_id"),
                                        ty.vec3<u32>(),
                                        core::IOAttributes{
                                            /* location */ std::nullopt,
                                            /* index */ std::nullopt,
                                            /* color */ std::nullopt,
                                            /* builtin */ core::BuiltinValue::kGlobalInvocationId,
                                            /* interpolation */ std::nullopt,
                                            /* invariant */ false,
                                        },
                                    },
                                    {
                                        mod.symbols.New("index"),
                                        ty.u32(),
                                        core::IOAttributes{
                                            /* location */ std::nullopt,
                                            /* index */ std::nullopt,
                                            /* color */ std::nullopt,
                                            /* builtin */ core::BuiltinValue::kLocalInvocationIndex,
                                            /* interpolation */ std::nullopt,
                                            /* invariant */ false,
                                        },
                                    },
                                });
    auto* func = MakeEntryPoint("main", 1, 1, 1);
    func->SetParams({b.FunctionParam("params", structure)});
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(16) {
  global_id:vec3<u32> @offset(0), @builtin(global_invocation_id)
  index:u32 @offset(12), @builtin(local_invocation_index)
}

$B1: {  # root
  %wgvar:ptr<workgroup, bool, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func(%params:MyStruct):void {
  $B2: {
    %4:bool = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(16) {
  global_id:vec3<u32> @offset(0), @builtin(global_invocation_id)
  index:u32 @offset(12), @builtin(local_invocation_index)
}

$B1: {  # root
  %wgvar:ptr<workgroup, bool, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func(%params:MyStruct):void {
  $B2: {
    %4:u32 = access %params, 1u
    %5:bool = lt %4, 1u
    if %5 [t: $B3] {  # if_1
      $B3: {  # true
        store %wgvar, false
        exit_if  # if_1
      }
    }
    %6:void = workgroupBarrier
    %7:bool = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, UseInsideNestedBlock) {
    auto* var = MakeVar("wgvar", ty.bool_());

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        auto* ifelse = b.If(true);
        b.Append(ifelse->True(), [&] {  //
            auto* sw = b.Switch(42_i);
            auto* def_case = b.DefaultCase(sw);
            b.Append(def_case, [&] {  //
                auto* loop = b.Loop();
                b.Append(loop->Body(), [&] {  //
                    b.Continue(loop);
                    b.Append(loop->Continuing(), [&] {  //
                        auto* load = b.Load(var);
                        b.BreakIf(loop, load);
                    });
                });
                b.ExitSwitch(sw);
            });
            b.ExitIf(ifelse);
        });
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, bool, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    if true [t: $B3] {  # if_1
      $B3: {  # true
        switch 42i [c: (default, $B4)] {  # switch_1
          $B4: {  # case
            loop [b: $B5, c: $B6] {  # loop_1
              $B5: {  # body
                continue  # -> $B6
              }
              $B6: {  # continuing
                %3:bool = load %wgvar
                break_if %3  # -> [t: exit_loop loop_1, f: $B5]
              }
            }
            exit_switch  # switch_1
          }
        }
        exit_if  # if_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, bool, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func(%tint_local_index:u32 [@local_invocation_index]):void {
  $B2: {
    %4:bool = lt %tint_local_index, 1u
    if %4 [t: $B3] {  # if_1
      $B3: {  # true
        store %wgvar, false
        exit_if  # if_1
      }
    }
    %5:void = workgroupBarrier
    if true [t: $B4] {  # if_2
      $B4: {  # true
        switch 42i [c: (default, $B5)] {  # switch_1
          $B5: {  # case
            loop [b: $B6, c: $B7] {  # loop_1
              $B6: {  # body
                continue  # -> $B7
              }
              $B7: {  # continuing
                %6:bool = load %wgvar
                break_if %6  # -> [t: exit_loop loop_1, f: $B6]
              }
            }
            exit_switch  # switch_1
          }
        }
        exit_if  # if_2
      }
    }
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, UseInsideIndirectFunctionCall) {
    auto* var = MakeVar("wgvar", ty.bool_());

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {  //
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {  //
            b.Continue(loop);
            b.Append(loop->Continuing(), [&] {  //
                auto* load = b.Load(var);
                b.BreakIf(loop, load);
            });
        });
        b.Return(foo);
    });

    auto* bar = b.Function("foo", ty.void_());
    b.Append(bar->Block(), [&] {  //
        auto* ifelse = b.If(true);
        b.Append(ifelse->True(), [&] {  //
            b.Call(ty.void_(), foo);
            b.ExitIf(ifelse);
        });
        b.Return(bar);
    });

    auto* func = MakeEntryPoint("func", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        auto* ifelse = b.If(true);
        b.Append(ifelse->True(), [&] {  //
            auto* sw = b.Switch(42_i);
            auto* def_case = b.DefaultCase(sw);
            b.Append(def_case, [&] {  //
                auto* loop = b.Loop();
                b.Append(loop->Body(), [&] {  //
                    b.Continue(loop);
                    b.Append(loop->Continuing(), [&] {  //
                        b.Call(ty.void_(), bar);
                        b.BreakIf(loop, true);
                    });
                });
                b.ExitSwitch(sw);
            });
            b.ExitIf(ifelse);
        });
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, bool, read_write> = var
}

%foo = func():void {
  $B2: {
    loop [b: $B3, c: $B4] {  # loop_1
      $B3: {  # body
        continue  # -> $B4
      }
      $B4: {  # continuing
        %3:bool = load %wgvar
        break_if %3  # -> [t: exit_loop loop_1, f: $B3]
      }
    }
    ret
  }
}
%foo_1 = func():void {  # %foo_1: 'foo'
  $B5: {
    if true [t: $B6] {  # if_1
      $B6: {  # true
        %5:void = call %foo
        exit_if  # if_1
      }
    }
    ret
  }
}
%func = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B7: {
    if true [t: $B8] {  # if_2
      $B8: {  # true
        switch 42i [c: (default, $B9)] {  # switch_1
          $B9: {  # case
            loop [b: $B10, c: $B11] {  # loop_2
              $B10: {  # body
                continue  # -> $B11
              }
              $B11: {  # continuing
                %7:void = call %foo_1
                break_if true  # -> [t: exit_loop loop_2, f: $B10]
              }
            }
            exit_switch  # switch_1
          }
        }
        exit_if  # if_2
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, bool, read_write> = var
}

%foo = func():void {
  $B2: {
    loop [b: $B3, c: $B4] {  # loop_1
      $B3: {  # body
        continue  # -> $B4
      }
      $B4: {  # continuing
        %3:bool = load %wgvar
        break_if %3  # -> [t: exit_loop loop_1, f: $B3]
      }
    }
    ret
  }
}
%foo_1 = func():void {  # %foo_1: 'foo'
  $B5: {
    if true [t: $B6] {  # if_1
      $B6: {  # true
        %5:void = call %foo
        exit_if  # if_1
      }
    }
    ret
  }
}
%func = @compute @workgroup_size(1u, 1u, 1u) func(%tint_local_index:u32 [@local_invocation_index]):void {
  $B7: {
    %8:bool = lt %tint_local_index, 1u
    if %8 [t: $B8] {  # if_2
      $B8: {  # true
        store %wgvar, false
        exit_if  # if_2
      }
    }
    %9:void = workgroupBarrier
    if true [t: $B9] {  # if_3
      $B9: {  # true
        switch 42i [c: (default, $B10)] {  # switch_1
          $B10: {  # case
            loop [b: $B11, c: $B12] {  # loop_2
              $B11: {  # body
                continue  # -> $B12
              }
              $B12: {  # continuing
                %10:void = call %foo_1
                break_if true  # -> [t: exit_loop loop_2, f: $B11]
              }
            }
            exit_switch  # switch_1
          }
        }
        exit_if  # if_3
      }
    }
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, MultipleEntryPoints_SameVarViaHelper) {
    auto* var = MakeVar("wgvar", ty.bool_());

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {  //
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {  //
            b.Continue(loop);
            b.Append(loop->Continuing(), [&] {  //
                auto* load = b.Load(var);
                b.BreakIf(loop, load);
            });
        });
        b.Return(foo);
    });

    auto* ep1 = MakeEntryPoint("ep1", 1, 1, 1);
    b.Append(ep1->Block(), [&] {  //
        b.Call(ty.void_(), foo);
        b.Return(ep1);
    });

    auto* ep2 = MakeEntryPoint("ep2", 1, 1, 1);
    b.Append(ep2->Block(), [&] {  //
        b.Call(ty.void_(), foo);
        b.Return(ep2);
    });

    auto* src = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, bool, read_write> = var
}

%foo = func():void {
  $B2: {
    loop [b: $B3, c: $B4] {  # loop_1
      $B3: {  # body
        continue  # -> $B4
      }
      $B4: {  # continuing
        %3:bool = load %wgvar
        break_if %3  # -> [t: exit_loop loop_1, f: $B3]
      }
    }
    ret
  }
}
%ep1 = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B5: {
    %5:void = call %foo
    ret
  }
}
%ep2 = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B6: {
    %7:void = call %foo
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, bool, read_write> = var
}

%foo = func():void {
  $B2: {
    loop [b: $B3, c: $B4] {  # loop_1
      $B3: {  # body
        continue  # -> $B4
      }
      $B4: {  # continuing
        %3:bool = load %wgvar
        break_if %3  # -> [t: exit_loop loop_1, f: $B3]
      }
    }
    ret
  }
}
%ep1 = @compute @workgroup_size(1u, 1u, 1u) func(%tint_local_index:u32 [@local_invocation_index]):void {
  $B5: {
    %6:bool = lt %tint_local_index, 1u
    if %6 [t: $B6] {  # if_1
      $B6: {  # true
        store %wgvar, false
        exit_if  # if_1
      }
    }
    %7:void = workgroupBarrier
    %8:void = call %foo
    ret
  }
}
%ep2 = @compute @workgroup_size(1u, 1u, 1u) func(%tint_local_index_1:u32 [@local_invocation_index]):void {  # %tint_local_index_1: 'tint_local_index'
  $B7: {
    %11:bool = lt %tint_local_index_1, 1u
    if %11 [t: $B8] {  # if_2
      $B8: {  # true
        store %wgvar, false
        exit_if  # if_2
      }
    }
    %12:void = workgroupBarrier
    %13:void = call %foo
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform

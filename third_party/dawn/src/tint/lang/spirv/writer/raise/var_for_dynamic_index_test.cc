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

#include "src/tint/lang/spirv/writer/raise/var_for_dynamic_index.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/struct.h"

namespace tint::spirv::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using SpirvWriter_VarForDynamicIndexTest = core::ir::transform::TransformTest;

TEST_F(SpirvWriter_VarForDynamicIndexTest, NoModify_ConstantIndex_ArrayValue) {
    auto* arr = b.FunctionParam(ty.array<i32, 4u>());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arr});

    auto* block = func->Block();
    auto* access = block->Append(b.Access(ty.i32(), arr, 1_i));
    block->Append(b.Return(func, access));

    auto* expect = R"(
%foo = func(%2:array<i32, 4>):i32 {
  $B1: {
    %3:i32 = access %2, 1i
    ret %3
  }
}
)";

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_VarForDynamicIndexTest, NoModify_ConstantIndex_MatrixValue) {
    auto* mat = b.FunctionParam(ty.mat2x2<f32>());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({mat});

    auto* block = func->Block();
    auto* access = block->Append(b.Access(ty.f32(), mat, 1_i, 0_i));
    block->Append(b.Return(func, access));

    auto* expect = R"(
%foo = func(%2:mat2x2<f32>):f32 {
  $B1: {
    %3:f32 = access %2, 1i, 0i
    ret %3
  }
}
)";

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_VarForDynamicIndexTest, NoModify_DynamicIndex_ArrayPointer) {
    auto* arr = b.FunctionParam(ty.ptr<function, array<i32, 4u>>());
    auto* idx = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arr, idx});

    auto* block = func->Block();
    auto* access = block->Append(b.Access(ty.ptr<function, i32>(), arr, idx));
    auto* load = block->Append(b.Load(access));
    block->Append(b.Return(func, load));

    auto* expect = R"(
%foo = func(%2:ptr<function, array<i32, 4>, read_write>, %3:i32):i32 {
  $B1: {
    %4:ptr<function, i32, read_write> = access %2, %3
    %5:i32 = load %4
    ret %5
  }
}
)";

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_VarForDynamicIndexTest, NoModify_DynamicIndex_MatrixPointer) {
    auto* mat = b.FunctionParam(ty.ptr<function, mat2x2<f32>>());
    auto* idx = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({mat, idx});

    auto* block = func->Block();
    auto* access = block->Append(b.Access(ty.ptr<function, vec2<f32>>(), mat, idx));
    auto* load = block->Append(b.LoadVectorElement(access, idx));
    block->Append(b.Return(func, load));

    auto* expect = R"(
%foo = func(%2:ptr<function, mat2x2<f32>, read_write>, %3:i32):f32 {
  $B1: {
    %4:ptr<function, vec2<f32>, read_write> = access %2, %3
    %5:f32 = load_vector_element %4, %3
    ret %5
  }
}
)";

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_VarForDynamicIndexTest, NoModify_DynamicIndex_VectorValue) {
    auto* vec = b.FunctionParam(ty.vec4<f32>());
    auto* idx = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({vec, idx});

    auto* block = func->Block();
    auto* access = block->Append(b.Access(ty.f32(), vec, idx));
    block->Append(b.Return(func, access));

    auto* expect = R"(
%foo = func(%2:vec4<f32>, %3:i32):f32 {
  $B1: {
    %4:f32 = access %2, %3
    ret %4
  }
}
)";

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_VarForDynamicIndexTest, DynamicIndex_ArrayValue) {
    auto* arr = b.FunctionParam(ty.array<i32, 4u>());
    auto* idx = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arr, idx});

    auto* block = func->Block();
    auto* access = block->Append(b.Access(ty.i32(), arr, idx));
    block->Append(b.Return(func, access));

    auto* expect = R"(
%foo = func(%2:array<i32, 4>, %3:i32):i32 {
  $B1: {
    %4:ptr<function, array<i32, 4>, read_write> = var %2
    %5:ptr<function, i32, read_write> = access %4, %3
    %6:i32 = load %5
    ret %6
  }
}
)";

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_VarForDynamicIndexTest, DynamicIndex_MatrixValue) {
    auto* mat = b.FunctionParam(ty.mat2x2<f32>());
    auto* idx = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.vec2<f32>());
    func->SetParams({mat, idx});

    auto* block = func->Block();
    auto* access = block->Append(b.Access(ty.vec2<f32>(), mat, idx));
    block->Append(b.Return(func, access));

    auto* expect = R"(
%foo = func(%2:mat2x2<f32>, %3:i32):vec2<f32> {
  $B1: {
    %4:ptr<function, mat2x2<f32>, read_write> = var %2
    %5:ptr<function, vec2<f32>, read_write> = access %4, %3
    %6:vec2<f32> = load %5
    ret %6
  }
}
)";

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_VarForDynamicIndexTest, DynamicIndex_VectorValue) {
    auto* mat = b.FunctionParam(ty.mat2x2<f32>());
    auto* idx = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({mat, idx});

    auto* block = func->Block();
    auto* access = block->Append(b.Access(ty.f32(), mat, idx, idx));
    block->Append(b.Return(func, access));

    auto* expect = R"(
%foo = func(%2:mat2x2<f32>, %3:i32):f32 {
  $B1: {
    %4:ptr<function, mat2x2<f32>, read_write> = var %2
    %5:ptr<function, vec2<f32>, read_write> = access %4, %3
    %6:f32 = load_vector_element %5, %3
    ret %6
  }
}
)";

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_VarForDynamicIndexTest, AccessChain) {
    auto* arr = b.FunctionParam(ty.array(ty.array(ty.array<i32, 4u>(), 4u), 4u));
    auto* idx = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arr, idx});

    auto* block = func->Block();
    auto* access = block->Append(b.Access(ty.i32(), arr, idx, 1_u, idx));
    block->Append(b.Return(func, access));

    auto* expect = R"(
%foo = func(%2:array<array<array<i32, 4>, 4>, 4>, %3:i32):i32 {
  $B1: {
    %4:ptr<function, array<array<array<i32, 4>, 4>, 4>, read_write> = var %2
    %5:ptr<function, i32, read_write> = access %4, %3, 1u, %3
    %6:i32 = load %5
    ret %6
  }
}
)";

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_VarForDynamicIndexTest, AccessChain_SkipConstantIndices) {
    auto* arr = b.FunctionParam(ty.array(ty.array(ty.array<i32, 4u>(), 4u), 4u));
    auto* idx = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arr, idx});

    auto* block = func->Block();
    auto* access = block->Append(b.Access(ty.i32(), arr, 1_u, 2_u, idx));
    block->Append(b.Return(func, access));

    auto* expect = R"(
%foo = func(%2:array<array<array<i32, 4>, 4>, 4>, %3:i32):i32 {
  $B1: {
    %4:array<i32, 4> = access %2, 1u, 2u
    %5:ptr<function, array<i32, 4>, read_write> = var %4
    %6:ptr<function, i32, read_write> = access %5, %3
    %7:i32 = load %6
    ret %7
  }
}
)";

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_VarForDynamicIndexTest, AccessChain_SkipConstantIndices_Interleaved) {
    auto* arr = b.FunctionParam(ty.array(ty.array(ty.array(ty.array<i32, 4u>(), 4u), 4u), 4u));
    auto* idx = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arr, idx});

    auto* block = func->Block();
    auto* access = block->Append(b.Access(ty.i32(), arr, 1_u, idx, 2_u, idx));
    block->Append(b.Return(func, access));

    auto* expect = R"(
%foo = func(%2:array<array<array<array<i32, 4>, 4>, 4>, 4>, %3:i32):i32 {
  $B1: {
    %4:array<array<array<i32, 4>, 4>, 4> = access %2, 1u
    %5:ptr<function, array<array<array<i32, 4>, 4>, 4>, read_write> = var %4
    %6:ptr<function, i32, read_write> = access %5, %3, 2u, %3
    %7:i32 = load %6
    ret %7
  }
}
)";

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_VarForDynamicIndexTest, AccessChain_SkipConstantIndices_Struct) {
    auto* str_ty = ty.Struct(mod.symbols.Register("MyStruct"),
                             {
                                 {mod.symbols.Register("arr1"), ty.array<f32, 1024>()},
                                 {mod.symbols.Register("mat"), ty.mat4x4<f32>()},
                                 {mod.symbols.Register("arr2"), ty.array<f32, 1024>()},
                             });
    auto* str_val = b.FunctionParam(str_ty);
    auto* idx = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({str_val, idx});

    auto* block = func->Block();
    auto* access = block->Append(b.Access(ty.f32(), str_val, 1_u, idx, 0_u));
    block->Append(b.Return(func, access));

    auto* expect = R"(
MyStruct = struct @align(16) {
  arr1:array<f32, 1024> @offset(0)
  mat:mat4x4<f32> @offset(4096)
  arr2:array<f32, 1024> @offset(4160)
}

%foo = func(%2:MyStruct, %3:i32):f32 {
  $B1: {
    %4:mat4x4<f32> = access %2, 1u
    %5:ptr<function, mat4x4<f32>, read_write> = var %4
    %6:ptr<function, vec4<f32>, read_write> = access %5, %3
    %7:f32 = load_vector_element %6, 0u
    ret %7
  }
}
)";

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_VarForDynamicIndexTest, MultipleAccessesFromSameSource) {
    auto* arr = b.FunctionParam(ty.array<i32, 4u>());
    auto* idx_a = b.FunctionParam(ty.i32());
    auto* idx_b = b.FunctionParam(ty.i32());
    auto* idx_c = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arr, idx_a, idx_b, idx_c});

    auto* block = func->Block();
    block->Append(b.Access(ty.i32(), arr, idx_a));
    block->Append(b.Access(ty.i32(), arr, idx_b));
    auto* access_c = block->Append(b.Access(ty.i32(), arr, idx_c));
    block->Append(b.Return(func, access_c));

    auto* expect = R"(
%foo = func(%2:array<i32, 4>, %3:i32, %4:i32, %5:i32):i32 {
  $B1: {
    %6:ptr<function, array<i32, 4>, read_write> = var %2
    %7:ptr<function, i32, read_write> = access %6, %3
    %8:i32 = load %7
    %9:ptr<function, i32, read_write> = access %6, %4
    %10:i32 = load %9
    %11:ptr<function, i32, read_write> = access %6, %5
    %12:i32 = load %11
    ret %12
  }
}
)";

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_VarForDynamicIndexTest, MultipleAccessesFromSameSource_SkipConstantIndices) {
    auto* arr = b.FunctionParam(ty.array(ty.array(ty.array<i32, 4u>(), 4u), 4u));
    auto* idx_a = b.FunctionParam(ty.i32());
    auto* idx_b = b.FunctionParam(ty.i32());
    auto* idx_c = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arr, idx_a, idx_b, idx_c});

    auto* block = func->Block();
    block->Append(b.Access(ty.i32(), arr, 1_u, 2_u, idx_a));
    block->Append(b.Access(ty.i32(), arr, 1_u, 2_u, idx_b));
    auto* access_c = block->Append(b.Access(ty.i32(), arr, 1_u, 2_u, idx_c));
    block->Append(b.Return(func, access_c));

    auto* expect = R"(
%foo = func(%2:array<array<array<i32, 4>, 4>, 4>, %3:i32, %4:i32, %5:i32):i32 {
  $B1: {
    %6:array<i32, 4> = access %2, 1u, 2u
    %7:ptr<function, array<i32, 4>, read_write> = var %6
    %8:ptr<function, i32, read_write> = access %7, %3
    %9:i32 = load %8
    %10:ptr<function, i32, read_write> = access %7, %4
    %11:i32 = load %10
    %12:ptr<function, i32, read_write> = access %7, %5
    %13:i32 = load %12
    ret %13
  }
}
)";

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_VarForDynamicIndexTest, MultipleAccessesToFuncParam_FromDifferentBlocks) {
    auto* arr = b.FunctionParam(ty.array<i32, 4>());
    auto* cond = b.FunctionParam(ty.bool_());
    auto* idx_a = b.FunctionParam(ty.i32());
    auto* idx_b = b.FunctionParam(ty.i32());
    auto* func = b.Function("func", ty.i32());
    func->SetParams({arr, cond, idx_a, idx_b});
    b.Append(func->Block(), [&] {  //
        auto* if_ = b.If(cond);
        b.Append(if_->True(), [&] {  //
            b.Return(func, b.Access(ty.i32(), arr, idx_a));
        });
        b.Append(if_->False(), [&] {  //
            b.Return(func, b.Access(ty.i32(), arr, idx_b));
        });
        b.Unreachable();
    });

    auto* src = R"(
%func = func(%2:array<i32, 4>, %3:bool, %4:i32, %5:i32):i32 {
  $B1: {
    if %3 [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        %6:i32 = access %2, %4
        ret %6
      }
      $B3: {  # false
        %7:i32 = access %2, %5
        ret %7
      }
    }
    unreachable
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%func = func(%2:array<i32, 4>, %3:bool, %4:i32, %5:i32):i32 {
  $B1: {
    %6:ptr<function, array<i32, 4>, read_write> = var %2
    if %3 [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        %7:ptr<function, i32, read_write> = access %6, %4
        %8:i32 = load %7
        ret %8
      }
      $B3: {  # false
        %9:ptr<function, i32, read_write> = access %6, %5
        %10:i32 = load %9
        ret %10
      }
    }
    unreachable
  }
}
)";

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_VarForDynamicIndexTest,
       MultipleAccessesToFuncParam_FromDifferentBlocks_WithLeadingConstantIndex) {
    auto* arr = b.FunctionParam(ty.array(ty.array<i32, 4>(), 4));
    auto* cond = b.FunctionParam(ty.bool_());
    auto* idx_a = b.FunctionParam(ty.i32());
    auto* idx_b = b.FunctionParam(ty.i32());
    auto* func = b.Function("func", ty.i32());
    func->SetParams({arr, cond, idx_a, idx_b});
    b.Append(func->Block(), [&] {  //
        auto* if_ = b.If(cond);
        b.Append(if_->True(), [&] {  //
            b.Return(func, b.Access(ty.i32(), arr, 0_u, idx_a));
        });
        b.Append(if_->False(), [&] {  //
            b.Return(func, b.Access(ty.i32(), arr, 0_u, idx_b));
        });
        b.Unreachable();
    });

    auto* src = R"(
%func = func(%2:array<array<i32, 4>, 4>, %3:bool, %4:i32, %5:i32):i32 {
  $B1: {
    if %3 [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        %6:i32 = access %2, 0u, %4
        ret %6
      }
      $B3: {  # false
        %7:i32 = access %2, 0u, %5
        ret %7
      }
    }
    unreachable
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%func = func(%2:array<array<i32, 4>, 4>, %3:bool, %4:i32, %5:i32):i32 {
  $B1: {
    %6:array<i32, 4> = access %2, 0u
    %7:ptr<function, array<i32, 4>, read_write> = var %6
    if %3 [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        %8:ptr<function, i32, read_write> = access %7, %4
        %9:i32 = load %8
        ret %9
      }
      $B3: {  # false
        %10:ptr<function, i32, read_write> = access %7, %5
        %11:i32 = load %10
        ret %11
      }
    }
    unreachable
  }
}
)";

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}
TEST_F(SpirvWriter_VarForDynamicIndexTest, MultipleAccessesToBlockParam_FromDifferentBlocks) {
    auto* arr = b.BlockParam(ty.array<i32, 4>());
    auto* cond = b.FunctionParam(ty.bool_());
    auto* idx_a = b.FunctionParam(ty.i32());
    auto* idx_b = b.FunctionParam(ty.i32());
    auto* func = b.Function("func", ty.i32());
    func->SetParams({cond, idx_a, idx_b});
    b.Append(func->Block(), [&] {  //
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            b.NextIteration(loop, b.Splat(arr->Type(), 0_i));
        });
        loop->Body()->SetParams({arr});
        b.Append(loop->Body(), [&] {
            auto* if_ = b.If(cond);
            b.Append(if_->True(), [&] {  //
                b.Return(func, b.Access(ty.i32(), arr, idx_a));
            });
            b.Append(if_->False(), [&] {  //
                b.Return(func, b.Access(ty.i32(), arr, idx_b));
            });
            b.Unreachable();
        });
        b.Unreachable();
    });

    auto* src = R"(
%func = func(%2:bool, %3:i32, %4:i32):i32 {
  $B1: {
    loop [i: $B2, b: $B3] {  # loop_1
      $B2: {  # initializer
        next_iteration array<i32, 4>(0i)  # -> $B3
      }
      $B3 (%5:array<i32, 4>): {  # body
        if %2 [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            %6:i32 = access %5, %3
            ret %6
          }
          $B5: {  # false
            %7:i32 = access %5, %4
            ret %7
          }
        }
        unreachable
      }
    }
    unreachable
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%func = func(%2:bool, %3:i32, %4:i32):i32 {
  $B1: {
    loop [i: $B2, b: $B3] {  # loop_1
      $B2: {  # initializer
        next_iteration array<i32, 4>(0i)  # -> $B3
      }
      $B3 (%5:array<i32, 4>): {  # body
        %6:ptr<function, array<i32, 4>, read_write> = var %5
        if %2 [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            %7:ptr<function, i32, read_write> = access %6, %3
            %8:i32 = load %7
            ret %8
          }
          $B5: {  # false
            %9:ptr<function, i32, read_write> = access %6, %4
            %10:i32 = load %9
            ret %10
          }
        }
        unreachable
      }
    }
    unreachable
  }
}
)";

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_VarForDynamicIndexTest,
       MultipleAccessesToBlockParam_FromDifferentBlocks_WithLeadingConstantIndex) {
    auto* inner_ty = ty.array<i32, 4>();
    auto* arr = b.BlockParam(ty.array(inner_ty, 4));
    auto* cond = b.FunctionParam(ty.bool_());
    auto* idx_a = b.FunctionParam(ty.i32());
    auto* idx_b = b.FunctionParam(ty.i32());
    auto* func = b.Function("func", ty.i32());
    func->SetParams({cond, idx_a, idx_b});
    b.Append(func->Block(), [&] {  //
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            b.NextIteration(loop, b.Splat(arr->Type(), b.Splat(inner_ty, 0_i)));
        });
        loop->Body()->SetParams({arr});
        b.Append(loop->Body(), [&] {
            auto* if_ = b.If(cond);
            b.Append(if_->True(), [&] {  //
                b.Return(func, b.Access(ty.i32(), arr, 0_u, idx_a));
            });
            b.Append(if_->False(), [&] {  //
                b.Return(func, b.Access(ty.i32(), arr, 0_u, idx_b));
            });
            b.Unreachable();
        });
        b.Unreachable();
    });

    auto* src = R"(
%func = func(%2:bool, %3:i32, %4:i32):i32 {
  $B1: {
    loop [i: $B2, b: $B3] {  # loop_1
      $B2: {  # initializer
        next_iteration array<array<i32, 4>, 4>(array<i32, 4>(0i))  # -> $B3
      }
      $B3 (%5:array<array<i32, 4>, 4>): {  # body
        if %2 [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            %6:i32 = access %5, 0u, %3
            ret %6
          }
          $B5: {  # false
            %7:i32 = access %5, 0u, %4
            ret %7
          }
        }
        unreachable
      }
    }
    unreachable
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%func = func(%2:bool, %3:i32, %4:i32):i32 {
  $B1: {
    loop [i: $B2, b: $B3] {  # loop_1
      $B2: {  # initializer
        next_iteration array<array<i32, 4>, 4>(array<i32, 4>(0i))  # -> $B3
      }
      $B3 (%5:array<array<i32, 4>, 4>): {  # body
        %6:array<i32, 4> = access %5, 0u
        %7:ptr<function, array<i32, 4>, read_write> = var %6
        if %2 [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            %8:ptr<function, i32, read_write> = access %7, %3
            %9:i32 = load %8
            ret %9
          }
          $B5: {  # false
            %10:ptr<function, i32, read_write> = access %7, %4
            %11:i32 = load %10
            ret %11
          }
        }
        unreachable
      }
    }
    unreachable
  }
}
)";

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_VarForDynamicIndexTest, MultipleAccessesToConstant_FromDifferentFunctions) {
    auto* arr = b.Constant(mod.constant_values.Zero(ty.array<i32, 4>()));

    auto* idx_a = b.FunctionParam(ty.i32());
    auto* func_a = b.Function("func_a", ty.i32());
    func_a->SetParams({idx_a});
    b.Append(func_a->Block(), [&] {  //
        b.Return(func_a, b.Access(ty.i32(), arr, idx_a));
    });

    auto* idx_b = b.FunctionParam(ty.i32());
    auto* func_b = b.Function("func_b", ty.i32());
    func_b->SetParams({idx_b});
    b.Append(func_b->Block(), [&] {  //
        b.Return(func_b, b.Access(ty.i32(), arr, idx_b));
    });

    auto* idx_c = b.FunctionParam(ty.i32());
    auto* func_c = b.Function("func_c", ty.i32());
    func_c->SetParams({idx_c});
    b.Append(func_c->Block(), [&] {  //
        b.Return(func_c, b.Access(ty.i32(), arr, idx_c));
    });

    auto* src = R"(
%func_a = func(%2:i32):i32 {
  $B1: {
    %3:i32 = access array<i32, 4>(0i), %2
    ret %3
  }
}
%func_b = func(%5:i32):i32 {
  $B2: {
    %6:i32 = access array<i32, 4>(0i), %5
    ret %6
  }
}
%func_c = func(%8:i32):i32 {
  $B3: {
    %9:i32 = access array<i32, 4>(0i), %8
    ret %9
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<private, array<i32, 4>, read_write> = var array<i32, 4>(0i)
}

%func_a = func(%3:i32):i32 {
  $B2: {
    %4:ptr<private, i32, read_write> = access %1, %3
    %5:i32 = load %4
    ret %5
  }
}
%func_b = func(%7:i32):i32 {
  $B3: {
    %8:ptr<private, i32, read_write> = access %1, %7
    %9:i32 = load %8
    ret %9
  }
}
%func_c = func(%11:i32):i32 {
  $B4: {
    %12:ptr<private, i32, read_write> = access %1, %11
    %13:i32 = load %12
    ret %13
  }
}
)";

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_VarForDynamicIndexTest,
       MultipleAccessesToConstant_FromDifferentFunctions_WithLeadingConstantIndex) {
    auto* arr = b.Constant(mod.constant_values.Zero(ty.array(ty.array<i32, 4>(), 4)));

    auto* idx_a = b.FunctionParam(ty.i32());
    auto* func_a = b.Function("func_a", ty.i32());
    func_a->SetParams({idx_a});
    b.Append(func_a->Block(), [&] {  //
        b.Return(func_a, b.Access(ty.i32(), arr, 0_u, idx_a));
    });

    auto* idx_b = b.FunctionParam(ty.i32());
    auto* func_b = b.Function("func_b", ty.i32());
    func_b->SetParams({idx_b});
    b.Append(func_b->Block(), [&] {  //
        b.Return(func_b, b.Access(ty.i32(), arr, 0_u, idx_b));
    });

    auto* idx_c = b.FunctionParam(ty.i32());
    auto* func_c = b.Function("func_c", ty.i32());
    func_c->SetParams({idx_c});
    b.Append(func_c->Block(), [&] {  //
        b.Return(func_c, b.Access(ty.i32(), arr, 0_u, idx_c));
    });

    auto* src = R"(
%func_a = func(%2:i32):i32 {
  $B1: {
    %3:i32 = access array<array<i32, 4>, 4>(array<i32, 4>(0i)), 0u, %2
    ret %3
  }
}
%func_b = func(%5:i32):i32 {
  $B2: {
    %6:i32 = access array<array<i32, 4>, 4>(array<i32, 4>(0i)), 0u, %5
    ret %6
  }
}
%func_c = func(%8:i32):i32 {
  $B3: {
    %9:i32 = access array<array<i32, 4>, 4>(array<i32, 4>(0i)), 0u, %8
    ret %9
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<private, array<i32, 4>, read_write> = var array<i32, 4>(0i)
}

%func_a = func(%3:i32):i32 {
  $B2: {
    %4:ptr<private, i32, read_write> = access %1, %3
    %5:i32 = load %4
    ret %5
  }
}
%func_b = func(%7:i32):i32 {
  $B3: {
    %8:ptr<private, i32, read_write> = access %1, %7
    %9:i32 = load %8
    ret %9
  }
}
%func_c = func(%11:i32):i32 {
  $B4: {
    %12:ptr<private, i32, read_write> = access %1, %11
    %13:i32 = load %12
    ret %13
  }
}
)";

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::spirv::writer::raise

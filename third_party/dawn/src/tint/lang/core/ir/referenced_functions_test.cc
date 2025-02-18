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

#include "src/tint/lang/core/ir/referenced_functions.h"

#include <string>

#include "gmock/gmock.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using ::testing::UnorderedElementsAre;

using namespace tint::core::fluent_types;  // NOLINT

class IR_ReferencedFunctionsTest : public IRTestHelper {
  protected:
    /// @returns the module as a disassembled string
    std::string Disassemble() const { return "\n" + ir::Disassembler(mod).Plain(); }

    /// @returns a new function called @p name
    Function* Func(const char* name) { return b.Function(name, ty.void_()); }
};

TEST_F(IR_ReferencedFunctionsTest, NoReferences) {
    auto* foo = Func("foo");

    auto* src = R"(
%foo = func():void {
  $B1: {
  }
}
)";
    EXPECT_EQ(src, Disassemble());

    ReferencedFunctions<Module> functions(mod);
    EXPECT_TRUE(functions.TransitiveReferences(foo).IsEmpty());
}

TEST_F(IR_ReferencedFunctionsTest, DirectUse) {
    // Referenced.
    auto* func_a = Func("a");
    auto* func_b = Func("b");
    // Not referenced.
    Func("c");

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {  //
        b.Call(func_a);
        b.Call(func_b);
        b.Return(foo);
    });

    auto* src = R"(
%a = func():void {
  $B1: {
  }
}
%b = func():void {
  $B2: {
  }
}
%c = func():void {
  $B3: {
  }
}
%foo = func():void {
  $B4: {
    %5:void = call %a
    %6:void = call %b
    ret
  }
}
)";
    EXPECT_EQ(src, Disassemble());

    ReferencedFunctions<Module> functions(mod);
    EXPECT_THAT(functions.TransitiveReferences(foo), UnorderedElementsAre(func_a, func_b));
}

TEST_F(IR_ReferencedFunctionsTest, DirectUse_MultipleFunctions) {
    auto* func_a = Func("a");
    auto* func_b = Func("b");
    auto* func_c = Func("c");

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {  //
        b.Call(func_a);
        b.Call(func_b);
        b.Return(foo);
    });

    auto* bar = b.Function("bar", ty.void_());
    b.Append(bar->Block(), [&] {  //
        b.Call(func_a);
        b.Call(func_c);
        b.Return(bar);
    });

    auto* zoo = b.Function("zoo", ty.void_());
    b.Append(zoo->Block(), [&] {  //
        b.Return(zoo);
    });

    auto* src = R"(
%a = func():void {
  $B1: {
  }
}
%b = func():void {
  $B2: {
  }
}
%c = func():void {
  $B3: {
  }
}
%foo = func():void {
  $B4: {
    %5:void = call %a
    %6:void = call %b
    ret
  }
}
%bar = func():void {
  $B5: {
    %8:void = call %a
    %9:void = call %c
    ret
  }
}
%zoo = func():void {
  $B6: {
    ret
  }
}
)";
    EXPECT_EQ(src, Disassemble());

    ReferencedFunctions<Module> functions(mod);
    EXPECT_THAT(functions.TransitiveReferences(foo), UnorderedElementsAre(func_a, func_b));
    EXPECT_THAT(functions.TransitiveReferences(bar), UnorderedElementsAre(func_a, func_c));
    EXPECT_TRUE(functions.TransitiveReferences(zoo).IsEmpty());
}

TEST_F(IR_ReferencedFunctionsTest, DirectUse_NestedInControlFlow) {
    auto* func_a = Func("a");
    auto* func_b = Func("b");
    auto* func_c = Func("c");
    auto* func_d = Func("d");

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {  //
        auto* ifelse = b.If(true);
        b.Append(ifelse->True(), [&] {
            b.Call(func_a);
            b.ExitIf(ifelse);
        });
        b.Append(ifelse->False(), [&] {
            auto* loop = b.Loop();
            b.Append(loop->Initializer(), [&] {
                b.Call(func_b);
                b.NextIteration(loop);
            });
            b.Append(loop->Body(), [&] {
                b.Call(func_c);
                b.Continue(loop);
            });
            b.Append(loop->Continuing(), [&] {
                b.Call(func_d);
                b.NextIteration(loop);
            });
            b.ExitIf(ifelse);
        });
        b.Return(foo);
    });

    auto* src = R"(
%a = func():void {
  $B1: {
  }
}
%b = func():void {
  $B2: {
  }
}
%c = func():void {
  $B3: {
  }
}
%d = func():void {
  $B4: {
  }
}
%foo = func():void {
  $B5: {
    if true [t: $B6, f: $B7] {  # if_1
      $B6: {  # true
        %6:void = call %a
        exit_if  # if_1
      }
      $B7: {  # false
        loop [i: $B8, b: $B9, c: $B10] {  # loop_1
          $B8: {  # initializer
            %7:void = call %b
            next_iteration  # -> $B9
          }
          $B9: {  # body
            %8:void = call %c
            continue  # -> $B10
          }
          $B10: {  # continuing
            %9:void = call %d
            next_iteration  # -> $B9
          }
        }
        exit_if  # if_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, Disassemble());

    ReferencedFunctions<Module> functions(mod);
    EXPECT_THAT(functions.TransitiveReferences(foo),
                UnorderedElementsAre(func_a, func_b, func_c, func_d));
}

TEST_F(IR_ReferencedFunctionsTest, IndirectUse) {
    // Directly used by foo.
    auto* func_a = Func("a");
    // Directly used by bar, called by zoo and foo.
    auto* func_b = Func("b");
    // Not used.
    Func("c");

    auto* bar = b.Function("bar", ty.void_());
    b.Append(bar->Block(), [&] {  //
        b.Call(func_b);
        b.Return(bar);
    });

    auto* zoo = b.Function("zoo", ty.void_());
    b.Append(zoo->Block(), [&] {  //
        b.Call(bar);
        b.Return(zoo);
    });

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {  //
        b.Call(func_a);
        b.Call(zoo);
        b.Return(foo);
    });

    auto* src = R"(
%a = func():void {
  $B1: {
  }
}
%b = func():void {
  $B2: {
  }
}
%c = func():void {
  $B3: {
  }
}
%bar = func():void {
  $B4: {
    %5:void = call %b
    ret
  }
}
%zoo = func():void {
  $B5: {
    %7:void = call %bar
    ret
  }
}
%foo = func():void {
  $B6: {
    %9:void = call %a
    %10:void = call %zoo
    ret
  }
}
)";
    EXPECT_EQ(src, Disassemble());

    ReferencedFunctions<Module> functions(mod);
    EXPECT_THAT(functions.TransitiveReferences(bar), UnorderedElementsAre(func_b));
    EXPECT_THAT(functions.TransitiveReferences(zoo), UnorderedElementsAre(bar, func_b));
    EXPECT_THAT(functions.TransitiveReferences(foo),
                UnorderedElementsAre(zoo, bar, func_a, func_b));
}

TEST_F(IR_ReferencedFunctionsTest, ReferencesForNullFunction) {
    ReferencedFunctions<Module> functions(mod);
    EXPECT_TRUE(functions.TransitiveReferences(nullptr).IsEmpty());
}

}  // namespace
}  // namespace tint::core::ir

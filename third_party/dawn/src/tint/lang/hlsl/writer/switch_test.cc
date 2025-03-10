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

#include "src/tint/lang/hlsl/writer/helper_test.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer {
namespace {

TEST_F(HlslWriterTest, Switch) {
    auto* f = b.ComputeFunction("foo");

    b.Append(f->Block(), [&] {
        auto* a = b.Var("a", b.Zero<i32>());
        auto* s = b.Switch(b.Load(a));
        b.Append(b.Case(s, {b.Constant(5_i)}), [&] { b.ExitSwitch(s); });
        b.Append(b.DefaultCase(s), [&] { b.ExitSwitch(s); });
        b.Return(f);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void foo() {
  int a = int(0);
  switch(a) {
    case int(5):
    {
      break;
    }
    default:
    {
      break;
    }
  }
}

)");
}

TEST_F(HlslWriterTest, SwitchMixedDefault) {
    auto* f = b.ComputeFunction("foo");

    b.Append(f->Block(), [&] {
        auto* a = b.Var("a", b.Zero<i32>());
        auto* s = b.Switch(b.Load(a));
        auto* c = b.Case(s, {b.Constant(5_i), nullptr});
        b.Append(c, [&] { b.ExitSwitch(s); });
        b.Return(f);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void foo() {
  int a = int(0);
  switch(a) {
    case int(5):
    default:
    {
      break;
    }
  }
}

)");
}

TEST_F(HlslWriterTest, SwitchOnlyDefaultCaseNoSideEffectsConditionDXC) {
    // var<private> cond : i32;
    // var<private> a : i32;
    // fn test() {
    //   switch(cond) {
    //     default: {
    //       a = 42;
    //     }
    //   }
    // }

    auto* f = b.ComputeFunction("foo");

    b.Append(f->Block(), [&] {
        auto* cond = b.Var("cond", b.Zero<i32>());
        auto* a = b.Var("a", b.Zero<i32>());
        auto* s = b.Switch(b.Load(cond));
        b.Append(b.DefaultCase(s), [&] {
            b.Store(a, 42_i);
            b.ExitSwitch(s);
        });
        b.Return(f);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void foo() {
  int cond = int(0);
  int a = int(0);
  switch(cond) {
    default:
    {
      a = int(42);
      break;
    }
  }
}

)");
}

TEST_F(HlslWriterTest, SwitchOnlyDefaultCaseSideEffectsConditionDXC) {
    // var<private> global : i32;
    // fn bar() -> i32 {
    //   global = 84;
    //   return global;
    // }
    //
    // var<private> a : i32;
    // fn test() {
    //   switch(bar()) {
    //     default: {
    //       a = 42;
    //     }
    //   }
    // }

    auto* global = b.Var<private_>("global", b.Zero<i32>());
    auto* a = b.Var<private_>("a", b.Zero<i32>());
    b.ir.root_block->Append(global);
    b.ir.root_block->Append(a);

    auto* bar = b.Function("bar", ty.i32());
    b.Append(bar->Block(), [&] {
        b.Store(global, 84_i);
        b.Return(bar, b.Load(global));
    });

    auto* f = b.ComputeFunction("foo");

    b.Append(f->Block(), [&] {
        auto* cond = b.Call(bar);
        auto* s = b.Switch(cond);
        b.Append(b.DefaultCase(s), [&] {
            b.Store(a, 42_i);
            b.ExitSwitch(s);
        });
        b.Return(f);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
static int global = int(0);
static int a = int(0);
int bar() {
  global = int(84);
  return global;
}

[numthreads(1, 1, 1)]
void foo() {
  switch(bar()) {
    default:
    {
      a = int(42);
      break;
    }
  }
}

)");
}

TEST_F(HlslWriterTest, SwitchOnlyDefaultCaseNoSideEffectsConditionFXC) {
    // var<private> cond : i32;
    // var<private> a : i32;
    // fn test() {
    //   switch(cond) {
    //     default: {
    //       a = 42;
    //     }
    //   }
    // }

    auto* f = b.ComputeFunction("foo");

    b.Append(f->Block(), [&] {
        auto* cond = b.Var("cond", b.Zero<i32>());
        auto* a = b.Var("a", b.Zero<i32>());
        auto* s = b.Switch(b.Load(cond));
        b.Append(b.DefaultCase(s), [&] {
            b.Store(a, 42_i);
            b.ExitSwitch(s);
        });
        b.Return(f);
    });

    Options options;
    options.compiler = Options::Compiler::kFXC;

    ASSERT_TRUE(Generate(options)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void foo() {
  int cond = int(0);
  int a = int(0);
  {
    while(true) {
      a = int(42);
      break;
    }
  }
}

)");
}

TEST_F(HlslWriterTest, SwitchOnlyDefaultCaseSideEffectsConditionFXC) {
    // var<private> global : i32;
    // fn bar() -> i32 {
    //   global = 84;
    //   return global;
    // }
    //
    // var<private> a : i32;
    // fn test() {
    //   switch(bar()) {
    //     default: {
    //       a = 42;
    //     }
    //   }
    // }

    auto* global = b.Var<private_>("global", b.Zero<i32>());
    auto* a = b.Var<private_>("a", b.Zero<i32>());
    b.ir.root_block->Append(global);
    b.ir.root_block->Append(a);

    auto* bar = b.Function("bar", ty.i32());
    b.Append(bar->Block(), [&] {
        b.Store(global, 84_i);
        b.Return(bar, b.Load(global));
    });

    auto* f = b.ComputeFunction("foo");

    b.Append(f->Block(), [&] {
        auto* cond = b.Call(bar);
        auto* s = b.Switch(cond);
        b.Append(b.DefaultCase(s), [&] {
            b.Store(a, 42_i);
            b.ExitSwitch(s);
        });
        b.Return(f);
    });

    Options options;
    options.compiler = Options::Compiler::kFXC;

    ASSERT_TRUE(Generate(options)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
static int global = int(0);
static int a = int(0);
int bar() {
  global = int(84);
  return global;
}

[numthreads(1, 1, 1)]
void foo() {
  bar();
  {
    while(true) {
      a = int(42);
      break;
    }
  }
}

)");
}

}  // namespace
}  // namespace tint::hlsl::writer

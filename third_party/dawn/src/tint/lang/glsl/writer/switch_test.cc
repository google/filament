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

#include "src/tint/lang/glsl/writer/helper_test.h"

#include "gmock/gmock.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::glsl::writer {
namespace {

TEST_F(GlslWriterTest, Switch) {
    auto* f = b.ComputeFunction("foo");

    b.Append(f->Block(), [&] {
        auto* a = b.Var("a", b.Zero<i32>());
        auto* s = b.Switch(b.Load(a));
        b.Append(b.Case(s, {b.Constant(5_i)}), [&] { b.ExitSwitch(s); });
        b.Append(b.DefaultCase(s), [&] { b.ExitSwitch(s); });
        b.Return(f);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int a = 0;
  switch(a) {
    case 5:
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

TEST_F(GlslWriterTest, Switch_MixedDefault) {
    auto* f = b.ComputeFunction("foo");

    b.Append(f->Block(), [&] {
        auto* a = b.Var("a", b.Zero<i32>());
        auto* s = b.Switch(b.Load(a));
        auto* c = b.Case(s, {b.Constant(5_i), nullptr});
        b.Append(c, [&] { b.ExitSwitch(s); });
        b.Return(f);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int a = 0;
  switch(a) {
    case 5:
    default:
    {
      break;
    }
  }
}
)");
}

}  // namespace
}  // namespace tint::glsl::writer

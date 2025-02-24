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

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::glsl::writer {
namespace {

TEST_F(GlslWriterTest, CallWithoutParams) {
    auto* f = b.Function("a", ty.bool_());
    f->Block()->Append(b.Return(f, false));

    auto* ep = b.ComputeFunction("main");
    b.Append(ep->Block(), [&] {
        b.Let("x", b.Call(f));
        b.Return(ep);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
bool a() {
  return false;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  bool x = a();
}
)");
}

TEST_F(GlslWriterTest, CallWithParams) {
    auto* p1 = b.FunctionParam("p1", ty.f32());
    auto* p2 = b.FunctionParam("p2", ty.bool_());

    auto* f = b.Function("a", ty.bool_());
    f->SetParams({p1, p2});
    f->Block()->Append(b.Return(f, p2));

    auto* ep = b.ComputeFunction("main");
    b.Append(ep->Block(), [&] {
        b.Let("x", b.Call(f, 1.5_f, false));
        b.Return(ep);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
bool a(float p1, bool p2) {
  return p2;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  bool x = a(1.5f, false);
}
)");
}

TEST_F(GlslWriterTest, CallWithPointerParams) {
    auto* p1 = b.FunctionParam("p1", ty.f32());
    auto* p2 = b.FunctionParam("p2", ty.bool_());
    auto* p3 = b.FunctionParam("p3", ty.ptr<function, i32>());

    auto* f = b.Function("a", ty.bool_());
    f->SetParams({p1, p2, p3});
    f->Block()->Append(b.Return(f, p2));

    auto* ep = b.ComputeFunction("main");
    b.Append(ep->Block(), [&] {
        auto* y = b.Var("y", 1_i);
        b.Let("x", b.Call(f, 1.5_f, false, y));
        b.Return(ep);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
bool a(float p1, bool p2, inout int p3) {
  return p2;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int y = 1;
  bool x = a(1.5f, false, y);
}
)");
}

TEST_F(GlslWriterTest, CallStatement) {
    auto* f = b.Function("a", ty.bool_());
    f->Block()->Append(b.Return(f, false));

    auto* ep = b.ComputeFunction("main");
    b.Append(ep->Block(), [&] {
        b.Call(f);
        b.Return(ep);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
bool a() {
  return false;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  a();
}
)");
}

}  // namespace
}  // namespace tint::glsl::writer

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

#include "src/tint/lang/glsl/writer/helper_test.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::glsl::writer {
namespace {

TEST_F(GlslWriterTest, ConstantBoolFalse) {
    auto* f = b.Function("a", ty.bool_());
    f->Block()->Append(b.Return(f, false));

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
bool a() {
  return false;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, ConstantBoolTrue) {
    auto* f = b.Function("a", ty.bool_());
    f->Block()->Append(b.Return(f, true));

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
bool a() {
  return true;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, ConstantInt) {
    auto* f = b.Function("a", ty.i32());
    f->Block()->Append(b.Return(f, -12345_i));

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
int a() {
  return -12345;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, ConstantIntMin) {
    auto* f = b.Function("a", ty.i32());
    b.Append(f->Block(), [&] { b.Return(f, i32(std::numeric_limits<int32_t>::min())); });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
int a() {
  return (-2147483647 - 1);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, ConstantUInt) {
    auto* f = b.Function("a", ty.u32());
    f->Block()->Append(b.Return(f, 56779_u));

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
uint a() {
  return 56779u;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, ConstantFloat) {
    auto* f = b.Function("a", ty.f32());
    // Use a number close to 1<<30 but whose decimal representation ends in 0.
    f->Block()->Append(b.Return(f, f32((1 << 30) - 4)));

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
float a() {
  return 1073741824.0f;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, ConstantF16) {
    auto* f = b.Function("a", ty.f16());
    // Use a number close to 1<<16 but whose decimal representation ends in 0.
    f->Block()->Append(b.Return(f, f16((1 << 15) - 8)));

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require

float16_t a() {
  return 32752.0hf;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

}  // namespace
}  // namespace tint::glsl::writer

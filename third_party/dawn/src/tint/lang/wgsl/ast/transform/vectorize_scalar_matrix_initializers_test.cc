// Copyright 2021 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/ast/transform/vectorize_scalar_matrix_initializers.h"

#include <string>
#include <utility>

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"
#include "src/tint/utils/text/string.h"

namespace tint::ast::transform {
namespace {

using VectorizeScalarMatrixInitializersTest = TransformTestWithParam<std::pair<uint32_t, uint32_t>>;

TEST_F(VectorizeScalarMatrixInitializersTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    EXPECT_FALSE(ShouldRun<VectorizeScalarMatrixInitializers>(src));
}

TEST_P(VectorizeScalarMatrixInitializersTest, MultipleScalars) {
    uint32_t cols = GetParam().first;
    uint32_t rows = GetParam().second;
    std::string mat_type = "mat" + std::to_string(cols) + "x" + std::to_string(rows) + "<f32>";
    std::string vec_type = "vec" + std::to_string(rows) + "<f32>";
    std::string scalar_values;
    std::string vector_values;
    for (uint32_t c = 0; c < cols; c++) {
        if (c > 0) {
            vector_values += ", ";
            scalar_values += ", ";
        }
        vector_values += vec_type + "(";
        for (uint32_t r = 0; r < rows; r++) {
            if (r > 0) {
                scalar_values += ", ";
                vector_values += ", ";
            }
            auto value = std::to_string(c * rows + r) + ".0";
            scalar_values += value;
            vector_values += value;
        }
        vector_values += ")";
    }

    std::string tmpl = R"(
@fragment
fn main() {
  let m = ${matrix}(${values});
}
)";
    tmpl = tint::ReplaceAll(tmpl, "${matrix}", mat_type);
    auto src = tint::ReplaceAll(tmpl, "${values}", scalar_values);
    auto expect = tint::ReplaceAll(tmpl, "${values}", vector_values);

    EXPECT_TRUE(ShouldRun<VectorizeScalarMatrixInitializers>(src));

    auto got = Run<VectorizeScalarMatrixInitializers>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(VectorizeScalarMatrixInitializersTest, MultipleScalarsViaReference) {
    uint32_t cols = GetParam().first;
    uint32_t rows = GetParam().second;
    std::string mat_type = "mat" + std::to_string(cols) + "x" + std::to_string(rows) + "<f32>";
    std::string vec_type = "vec" + std::to_string(rows) + "<f32>";
    std::string scalar_values;
    std::string vector_values;
    for (uint32_t c = 0; c < cols; c++) {
        if (c > 0) {
            vector_values += ", ";
            scalar_values += ", ";
        }
        vector_values += vec_type + "(";
        for (uint32_t r = 0; r < rows; r++) {
            if (r > 0) {
                scalar_values += ", ";
                vector_values += ", ";
            }
            auto value = "v[" + std::to_string((c * rows + r) % 4) + "]";
            scalar_values += value;
            vector_values += value;
        }
        vector_values += ")";
    }

    std::string tmpl = R"(
@fragment
fn main() {
  let v = vec4<f32>(1.0, 2.0, 3.0, 8.0);
  let m = ${matrix}(${values});
}
)";
    tmpl = tint::ReplaceAll(tmpl, "${matrix}", mat_type);
    auto src = tint::ReplaceAll(tmpl, "${values}", scalar_values);
    auto expect = tint::ReplaceAll(tmpl, "${values}", vector_values);

    EXPECT_TRUE(ShouldRun<VectorizeScalarMatrixInitializers>(src));

    auto got = Run<VectorizeScalarMatrixInitializers>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(VectorizeScalarMatrixInitializersTest, MultipleScalarsViaPointerIndex) {
    uint32_t cols = GetParam().first;
    uint32_t rows = GetParam().second;
    std::string mat_type = "mat" + std::to_string(cols) + "x" + std::to_string(rows) + "<f32>";
    std::string vec_type = "vec" + std::to_string(rows) + "<f32>";
    std::string scalar_values;
    std::string vector_values;
    for (uint32_t c = 0; c < cols; c++) {
        if (c > 0) {
            vector_values += ", ";
            scalar_values += ", ";
        }
        vector_values += vec_type + "(";
        for (uint32_t r = 0; r < rows; r++) {
            if (r > 0) {
                scalar_values += ", ";
                vector_values += ", ";
            }
            auto value = "p[" + std::to_string((c * rows + r) % 4) + "]";
            scalar_values += value;
            vector_values += value;
        }
        vector_values += ")";
    }

    std::string tmpl = R"(
@fragment
fn main() {
  var v = vec4<f32>(1.0, 2.0, 3.0, 8.0);
  let p = &(v);
  let m = ${matrix}(${values});
}
)";
    tmpl = tint::ReplaceAll(tmpl, "${matrix}", mat_type);
    auto src = tint::ReplaceAll(tmpl, "${values}", scalar_values);
    auto expect = tint::ReplaceAll(tmpl, "${values}", vector_values);

    EXPECT_TRUE(ShouldRun<VectorizeScalarMatrixInitializers>(src));

    auto got = Run<VectorizeScalarMatrixInitializers>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(VectorizeScalarMatrixInitializersTest, NonScalarInitializers) {
    uint32_t cols = GetParam().first;
    uint32_t rows = GetParam().second;
    std::string mat_type = "mat" + std::to_string(cols) + "x" + std::to_string(rows) + "<f32>";
    std::string vec_type = "vec" + std::to_string(rows) + "<f32>";
    std::string columns;
    for (uint32_t c = 0; c < cols; c++) {
        if (c > 0) {
            columns += ", ";
        }
        columns += vec_type + "()";
    }

    std::string tmpl = R"(
@fragment
fn main() {
  let m = ${matrix}(${columns});
}
)";
    tmpl = tint::ReplaceAll(tmpl, "${matrix}", mat_type);
    auto src = tint::ReplaceAll(tmpl, "${columns}", columns);
    auto expect = src;

    EXPECT_FALSE(ShouldRun<VectorizeScalarMatrixInitializers>(src));

    auto got = Run<VectorizeScalarMatrixInitializers>(src);

    EXPECT_EQ(expect, str(got));
}

INSTANTIATE_TEST_SUITE_P(VectorizeScalarMatrixInitializersTest,
                         VectorizeScalarMatrixInitializersTest,
                         testing::Values(std::make_pair(2, 2),
                                         std::make_pair(2, 3),
                                         std::make_pair(2, 4),
                                         std::make_pair(3, 2),
                                         std::make_pair(3, 3),
                                         std::make_pair(3, 4),
                                         std::make_pair(4, 2),
                                         std::make_pair(4, 3),
                                         std::make_pair(4, 4)));

}  // namespace
}  // namespace tint::ast::transform

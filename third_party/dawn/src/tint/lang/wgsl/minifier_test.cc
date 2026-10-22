// Copyright 2025 The Dawn & Tint Authors
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

// GEN_BUILD:CONDITION(tint_build_wgsl_reader && tint_build_wgsl_writer)

#include "gtest/gtest.h"
#include "src/tint/lang/wgsl/reader/reader.h"
#include "src/tint/lang/wgsl/writer/writer.h"
#include "src/tint/utils/diagnostic/source.h"

namespace tint::wgsl {
namespace {

class WgslMinifierTest : public testing::Test {
  protected:
    Program Build(std::string_view source) {
        auto file = std::make_unique<Source::File>("test.wgsl", source);
        reader::Options options{
            .allowed_features = AllowedFeatures::Everything(),
        };
        auto program = wgsl::reader::Parse(file.get(), options);
        return program;
    }

    /// Minify @p input and compare it to @p expected (if non-empty).
    void Run(std::string_view input, std::string_view expected = "") {
        // Parse the input.
        auto program = Build(input);
        ASSERT_TRUE(program.IsValid()) << program.Diagnostics().Str();

        // Generate minified output.
        writer::Options options{
            .allowed_features = AllowedFeatures::Everything(),
            .minify = true,
        };
        auto result = writer::Generate(program, options);
        ASSERT_EQ(result, Success) << result.Failure().reason;

        if (!expected.empty()) {
            EXPECT_EQ(result->wgsl, expected);
        }

        // Make sure that the minified output parses successfully.
        auto minified_program = Build(result->wgsl);
        EXPECT_TRUE(minified_program.IsValid()) << minified_program.Diagnostics().Str();
    }
};

TEST_F(WgslMinifierTest, AllIdentifiers) {
    auto input = R"(
alias MyAlias = i32;

struct MyStruct {
  struct_member : MyAlias,
};

var<private> global_var : MyStruct;

fn my_func(param : i32) -> MyAlias {
  let local_var = global_var.struct_member + (&global_var).struct_member + param;
  return local_var;
}

fn another_func(param : MyAlias) -> i32 {
  return my_func(param);
}
)";
    auto minified =
        R"(alias
a=i32;struct
b{c:a,}var<private>d:b;fn
e(f:i32)->a{let
g=((d.c+(&(d)).c)+f);return
g;}fn
h(f:a)->i32{return
e(f);})";

    Run(input, minified);
}

TEST_F(WgslMinifierTest, EntryPoint) {
    auto input = R"(@vertex
fn main(@location(0) in : vec4<f32>) -> @builtin(position) vec4<f32> {
  return in;
})";
    auto minified =
        R"(@vertex
fn
main(@location(0)a:vec4<f32>)->@builtin(position)vec4<f32>{return
a;})";

    Run(input, minified);
}

TEST_F(WgslMinifierTest, Override) {
    auto input = R"(override my_override : i32;

fn f() -> i32 {
  return my_override;
}
)";
    auto minified = R"(override
my_override:i32;fn
a()->i32{return
my_override;})";

    Run(input, minified);
}

TEST_F(WgslMinifierTest, BuiltinStructure) {
    auto input = R"(var<workgroup> a: atomic<u32>;

fn f() -> u32 {
  let result = atomicCompareExchangeWeak(&a, 0, 1);
  if (result.exchanged) {
    return result.old_value;
  }
  return 0;
}
)";
    auto minified =
        R"(var<workgroup>a:atomic<u32>;fn
b()->u32{let
c=atomicCompareExchangeWeak(&(a),0,1);if(c.exchanged){return
c.old_value;}return
0;})";

    Run(input, minified);
}

TEST_F(WgslMinifierTest, ReservedWordCollision) {
    // Create a program with many identifiers, to force the minifier to generate names that could
    // clash with reserved words or builtin identifiers.
    std::string source = "fn f() {";
    for (int i = 0; i < 6000; ++i) {
        source += "let var_" + std::to_string(i) + " = 42f;\n";
    }
    source += R"(
      // Use a builtin function in the same scope as the minified identifiers.
      // 'fma' would be hit first by the minified name generation scheme.
      let foo = fma(var_0, var_1, var_2);
    }
    )";

    Run(source);
}

TEST_F(WgslMinifierTest, Unicode) {
    auto input = R"(override 𝖗𝖊𝖘𝖚𝖑𝖙 : f32;

@fragment fn 𝕖𝕟𝕥𝕣𝕪𝕡𝕠𝕚𝕟𝕥() -> @location(0) f32 {
  return 𝖗𝖊𝖘𝖚𝖑𝖙;
}
)";
    auto minified =
        R"(override
𝖗𝖊𝖘𝖚𝖑𝖙:f32;@fragment
fn
𝕖𝕟𝕥𝕣𝕪𝕡𝕠𝕚𝕟𝕥()->@location(0)f32{return
𝖗𝖊𝖘𝖚𝖑𝖙;})";

    Run(input, minified);
}

}  // namespace
}  // namespace tint::wgsl

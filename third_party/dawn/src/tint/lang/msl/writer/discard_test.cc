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

#include "src/tint/lang/msl/writer/helper_test.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::msl::writer {
namespace {

TEST_F(MslWriterTest, DiscardWithDemoteToHelper) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] {
            b.Discard();
            b.ExitIf(if_);
        });
        b.Return(func);
    });

    auto* ep = b.Function("frag_main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(ep->Block(), [&] {
        b.Call(func);
        b.Return(ep);
    });

    Options options;
    options.disable_demote_to_helper = false;

    ASSERT_TRUE(Generate(options)) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
struct tint_module_vars_struct {
  thread bool* continue_execution;
};

void foo(tint_module_vars_struct tint_module_vars) {
  if (true) {
    (*tint_module_vars.continue_execution) = false;
  }
}

fragment void frag_main() {
  thread bool continue_execution = true;
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.continue_execution=(&continue_execution)};
  foo(tint_module_vars);
  if (!((*tint_module_vars.continue_execution))) {
    discard_fragment();
  }
}
)");
}

TEST_F(MslWriterTest, DiscardWithoutDemoteToHelper) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] {
            b.Discard();
            b.ExitIf(if_);
        });
        b.Return(func);
    });

    auto* ep = b.Function("frag_main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(ep->Block(), [&] {
        b.Call(func);
        b.Return(ep);
    });

    Options options;
    options.disable_demote_to_helper = true;

    ASSERT_TRUE(Generate(options)) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  if (true) {
    discard_fragment();
  }
}

fragment void frag_main() {
  foo();
}
)");
}

}  // namespace
}  // namespace tint::msl::writer

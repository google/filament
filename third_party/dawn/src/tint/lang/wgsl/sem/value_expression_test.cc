// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/sem/value_expression.h"

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/wgsl/sem/helper_test.h"
#include "src/tint/lang/wgsl/sem/materialize.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::sem {
namespace {

class MockConstant : public core::constant::Value {
  public:
    explicit MockConstant(const core::type::Type* ty) : type(ty) {}
    ~MockConstant() override {}
    const core::type::Type* Type() const override { return type; }
    const core::constant::Value* Index(size_t) const override { return {}; }
    size_t NumElements() const override { return 0; }
    bool AllZero() const override { return {}; }
    bool AnyZero() const override { return {}; }
    HashCode Hash() const override { return 0; }
    MockConstant* Clone(core::constant::CloneContext&) const override { return nullptr; }

  protected:
    std::variant<std::monostate, AInt, AFloat> InternalValue() const override { return {}; }

  private:
    const core::type::Type* type;
};

using ValueExpressionTest = TestHelper;

TEST_F(ValueExpressionTest, UnwrapMaterialize) {
    MockConstant c(create<core::type::I32>());
    auto* a = create<ValueExpression>(/* declaration */ nullptr, create<core::type::I32>(),
                                      core::EvaluationStage::kRuntime, /* statement */ nullptr,
                                      /* constant_value */ nullptr,
                                      /* has_side_effects */ false, /* root_ident */ nullptr);
    auto* b = create<Materialize>(a, /* statement */ nullptr, c.Type(), &c);

    EXPECT_EQ(a, a->UnwrapMaterialize());
    EXPECT_EQ(a, b->UnwrapMaterialize());
}

}  // namespace
}  // namespace tint::sem

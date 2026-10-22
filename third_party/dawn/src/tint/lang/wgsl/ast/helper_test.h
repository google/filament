// Copyright 2020 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_WGSL_AST_HELPER_TEST_H_
#define SRC_TINT_LANG_WGSL_AST_HELPER_TEST_H_

#include <tuple>
#include <utility>

#include "gtest/gtest.h"
#include "src/tint/lang/wgsl/program/program_builder.h"

namespace tint::ast {

/// Helper base class for testing
template <typename BASE>
class TestHelperBase : public BASE, public ProgramBuilder {};

/// Helper class for testing that derives from testing::Test.
using TestHelper = TestHelperBase<testing::Test>;

/// Helper class for testing that derives from `T`.
template <typename T>
using TestParamHelper = TestHelperBase<testing::TestWithParam<T>>;

/// A structure to hold a TemplatedIdentifier matcher, used by the CheckIdentifier() test helper
template <typename... ARGS>
struct TemplatedIdentifierMatcher {
    /// The expected name of the TemplatedIdentifier
    std::string_view name;
    /// The expected arguments of the TemplatedIdentifier
    std::tuple<ARGS...> args;
};

/// Deduction guide for TemplatedIdentifierMatcher
template <typename... ARGS>
TemplatedIdentifierMatcher(std::string_view, std::tuple<ARGS...>&&)
    -> TemplatedIdentifierMatcher<ARGS...>;

/// A helper function for building a TemplatedIdentifierMatcher
/// @param name the name of the TemplatedIdentifier
/// @param args the template arguments
/// @return a TemplatedIdentifierMatcher
template <typename... ARGS>
auto Template(std::string_view name, ARGS&&... args) {
    return TemplatedIdentifierMatcher{name, std::make_tuple(std::forward<ARGS>(args)...)};
}

/// A traits helper for determining whether the type T is a TemplatedIdentifierMatcher.
template <typename T>
struct IsTemplatedIdentifierMatcher {
    /// True iff T is a TemplatedIdentifierMatcher
    static constexpr bool value = false;
};

/// IsTemplatedIdentifierMatcher specialization for TemplatedIdentifierMatcher.
template <typename... ARGS>
struct IsTemplatedIdentifierMatcher<TemplatedIdentifierMatcher<ARGS...>> {
    /// True iff T is a TemplatedIdentifierMatcher
    static constexpr bool value = true;
};

/// A testing utility for checking that an Identifier matches the expected values.
/// @param got the identifier
/// @param expected the expected identifier name
template <typename... ARGS>
void CheckIdentifier(const Identifier* got, std::string_view expected) {
    EXPECT_FALSE(got->Is<TemplatedIdentifier>());
    EXPECT_EQ(got->symbol.Name(), expected);
}

/// A testing utility for checking that an Identifier matches the expected name and template
/// arguments.
/// @param ident the identifier
/// @param expected the expected identifier name and arguments
template <typename... ARGS>
void CheckIdentifier(const Identifier* ident, const TemplatedIdentifierMatcher<ARGS...>& expected) {
    EXPECT_EQ(ident->symbol.Name(), expected.name);
    ASSERT_TRUE(ident->Is<TemplatedIdentifier>());
    auto* got = ident->As<TemplatedIdentifier>();
    ASSERT_EQ(got->arguments.Length(), std::tuple_size_v<decltype(expected.args)>);

    size_t arg_idx = 0;
    auto check_arg = [&](auto&& expected_arg) {
        const auto* got_arg = got->arguments[arg_idx++];

        using T = std::decay_t<decltype(expected_arg)>;
        if constexpr (tint::traits::IsStringLike<T>) {
            ASSERT_TRUE(got_arg->Is<IdentifierExpression>());
            CheckIdentifier(got_arg->As<IdentifierExpression>()->identifier, expected_arg);
        } else if constexpr (IsTemplatedIdentifierMatcher<T>::value) {
            ASSERT_TRUE(got_arg->Is<IdentifierExpression>());
            auto* got_ident = got_arg->As<IdentifierExpression>()->identifier;
            ASSERT_TRUE(got_ident->Is<TemplatedIdentifier>());
            CheckIdentifier(got_ident->As<TemplatedIdentifier>(), expected_arg);
        } else if constexpr (std::is_same_v<T, bool>) {
            ASSERT_TRUE(got_arg->Is<BoolLiteralExpression>());
            EXPECT_EQ(got_arg->As<BoolLiteralExpression>()->value, expected_arg);
        } else if constexpr (std::is_same_v<T, core::AInt>) {
            ASSERT_TRUE(got_arg->Is<IntLiteralExpression>());
            EXPECT_EQ(got_arg->As<IntLiteralExpression>()->suffix,
                      IntLiteralExpression::Suffix::kNone);
            EXPECT_EQ(core::AInt(got_arg->As<IntLiteralExpression>()->value), expected_arg);
        } else if constexpr (std::is_same_v<T, core::i32>) {
            ASSERT_TRUE(got_arg->Is<IntLiteralExpression>());
            EXPECT_EQ(got_arg->As<IntLiteralExpression>()->suffix,
                      IntLiteralExpression::Suffix::kI);
            EXPECT_EQ(core::i32(got_arg->As<IntLiteralExpression>()->value), expected_arg);
        } else if constexpr (std::is_same_v<T, core::u32>) {
            ASSERT_TRUE(got_arg->Is<IntLiteralExpression>());
            EXPECT_EQ(got_arg->As<IntLiteralExpression>()->suffix,
                      IntLiteralExpression::Suffix::kU);
            EXPECT_EQ(core::u32(got_arg->As<IntLiteralExpression>()->value), expected_arg);
        } else if constexpr (std::is_same_v<T, core::AFloat>) {
            ASSERT_TRUE(got_arg->Is<FloatLiteralExpression>());
            EXPECT_EQ(got_arg->As<FloatLiteralExpression>()->suffix,
                      FloatLiteralExpression::Suffix::kNone);
            EXPECT_EQ(core::AFloat(got_arg->As<FloatLiteralExpression>()->value), expected_arg);
        } else if constexpr (std::is_same_v<T, core::f32>) {
            ASSERT_TRUE(got_arg->Is<FloatLiteralExpression>());
            EXPECT_EQ(got_arg->As<FloatLiteralExpression>()->suffix,
                      FloatLiteralExpression::Suffix::kF);
            EXPECT_EQ(core::f32(got_arg->As<FloatLiteralExpression>()->value), expected_arg);
        } else if constexpr (std::is_same_v<T, core::f16>) {
            ASSERT_TRUE(got_arg->Is<FloatLiteralExpression>());
            EXPECT_EQ(got_arg->As<FloatLiteralExpression>()->suffix,
                      FloatLiteralExpression::Suffix::kH);
            EXPECT_EQ(core::f16(got_arg->As<FloatLiteralExpression>()->value), expected_arg);
        } else {
            FAIL() << "unhandled expected_args type";
        }
    };
    std::apply([&](auto&&... args) { ((check_arg(args)), ...); }, expected.args);
}

/// A testing utility for checking that an IdentifierExpression matches the expected values.
/// @param expr the IdentifierExpression
/// @param expected the expected identifier name
template <typename... ARGS>
void CheckIdentifier(const Expression* expr, std::string_view expected) {
    auto* expr_ident = expr->As<IdentifierExpression>();
    ASSERT_NE(expr_ident, nullptr) << "expression is not a IdentifierExpression";
    CheckIdentifier(expr_ident->identifier, expected);
}

/// A testing utility for checking that an IdentifierExpression matches the expected name and
/// template arguments.
/// @param expr the IdentifierExpression
/// @param expected the expected identifier name and arguments
template <typename... ARGS>
void CheckIdentifier(const Expression* expr, const TemplatedIdentifierMatcher<ARGS...>& expected) {
    auto* expr_ident = expr->As<IdentifierExpression>();
    ASSERT_NE(expr_ident, nullptr) << "expression is not a IdentifierExpression";
    CheckIdentifier(expr_ident->identifier, expected);
}

}  // namespace tint::ast

#endif  // SRC_TINT_LANG_WGSL_AST_HELPER_TEST_H_

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

#include "src/tint/lang/core/constant/eval_test.h"

#include "src/tint/lang/wgsl/builtin_fn.h"
#include "src/tint/utils/result.h"

#if TINT_BUILD_WGSL_READER
#include "src/tint/lang/wgsl/reader/reader.h"
#endif

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT
using ::testing::HasSubstr;

namespace tint::core::constant::test {
namespace {

struct Case {
    struct Success {
        Value value;
    };
    struct Failure {
        std::string error;
    };

    Value lhs;
    Value rhs;
    tint::Result<Success, Failure> expected;
};

struct ErrorCase {
    Value lhs;
    Value rhs;
};

/// Creates a Case with Values of any type
Case C(Value lhs, Value rhs, Value expected) {
    return Case{std::move(lhs), std::move(rhs), Case::Success{std::move(expected)}};
}

/// Convenience overload that creates a Case with just scalars
template <typename T, typename U, typename V, typename = std::enable_if_t<!IsValue<T>>>
Case C(T lhs, U rhs, V expected) {
    return Case{Val(lhs), Val(rhs), Case::Success{Val(expected)}};
}

/// Creates an failure Case with Values of any type
Case E(Value lhs, Value rhs, std::string error) {
    return Case{std::move(lhs), std::move(rhs), Case::Failure{std::move(error)}};
}

/// Convenience overload that creates an error Case with just scalars
template <typename T, typename U, typename = std::enable_if_t<!IsValue<T>>>
Case E(T lhs, U rhs, std::string error) {
    return Case{Val(lhs), Val(rhs), Case::Failure{std::move(error)}};
}

/// Prints Case to ostream
static std::ostream& operator<<(std::ostream& o, const Case& c) {
    o << "lhs: " << c.lhs << ", rhs: " << c.rhs << ", expected: ";
    if (c.expected == Success) {
        auto& s = c.expected.Get();
        o << s.value;
    } else {
        o << "[ERROR: " << c.expected.Failure().error << "]";
    }
    return o;
}

/// Prints ErrorCase to ostream
std::ostream& operator<<(std::ostream& o, const ErrorCase& c) {
    o << c.lhs << ", " << c.rhs;
    return o;
}

using ConstEvalBinaryOpTest = ConstEvalTestWithParam<std::tuple<core::BinaryOp, Case>>;
TEST_P(ConstEvalBinaryOpTest, Test) {
    Enable(wgsl::Extension::kF16);
    auto op = std::get<0>(GetParam());
    auto& c = std::get<1>(GetParam());

    auto* lhs_expr = c.lhs.Expr(*this);
    auto* rhs_expr = c.rhs.Expr(*this);

    auto* expr = create<ast::BinaryExpression>(Source{{12, 34}}, op, lhs_expr, rhs_expr);
    GlobalConst("C", expr);

    if (c.expected == Success) {
        ASSERT_TRUE(r()->Resolve()) << r()->error();
        auto expected_case = c.expected.Get();
        auto& expected = expected_case.value;

        auto* sem = Sem().Get(expr);
        const constant::Value* value = sem->ConstantValue();
        ASSERT_NE(value, nullptr);
        EXPECT_TYPE(value->Type(), sem->Type());

        CheckConstant(value, expected);
    } else {
        ASSERT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(), c.expected.Failure().error);
    }
}

INSTANTIATE_TEST_SUITE_P(MixedAbstractArgs,
                         ConstEvalBinaryOpTest,
                         testing::Combine(testing::Values(core::BinaryOp::kAdd),
                                          testing::ValuesIn(std::vector{
                                              // Mixed abstract type args
                                              C(1_a, 2.3_a, 3.3_a),
                                              C(2.3_a, 1_a, 3.3_a),
                                          })));

template <typename T>
std::vector<Case> OpAddIntCases() {
    static_assert(IsIntegral<T>);
    auto r = std::vector<Case>{
        C(T{0}, T{0}, T{0}),
        C(T{1}, T{2}, T{3}),
        C(T::Lowest(), T{1}, T{T::Lowest() + 1}),
        C(T::Highest(), Negate(T{1}), T{T::Highest() - 1}),
        C(T::Lowest(), T::Highest(), Negate(T{1})),
    };
    if constexpr (IsAbstract<T>) {
        auto error_msg = [](auto a, auto b) {
            return "12:34 error: " + OverflowErrorMessage(a, "+", b);
        };
        ConcatInto(  //
            r, std::vector<Case>{
                   E(T::Highest(), T{1}, error_msg(T::Highest(), T{1})),
                   E(T::Lowest(), Negate(T{1}), error_msg(T::Lowest(), Negate(T{1}))),
               });
    } else {
        ConcatInto(  //
            r, std::vector<Case>{
                   C(T::Highest(), T{1}, T::Lowest()),
                   C(T::Lowest(), Negate(T{1}), T::Highest()),
               });
    }

    return r;
}
template <typename T>
std::vector<Case> OpAddFloatCases() {
    static_assert(IsFloatingPoint<T>);
    auto error_msg = [](auto a, auto b) {
        return "12:34 error: " + OverflowErrorMessage(a, "+", b);
    };
    return std::vector<Case>{
        C(T{0}, T{0}, T{0}),
        C(T{1}, T{2}, T{3}),
        C(T::Lowest(), T{1}, T{T::Lowest() + 1}),
        C(T::Highest(), Negate(T{1}), T{T::Highest() - 1}),
        C(T::Lowest(), T::Highest(), T{0}),

        E(T::Highest(), T::Highest(), error_msg(T::Highest(), T::Highest())),
        E(T::Lowest(), Negate(T::Highest()), error_msg(T::Lowest(), Negate(T::Highest()))),
    };
}
INSTANTIATE_TEST_SUITE_P(Add,
                         ConstEvalBinaryOpTest,
                         testing::Combine(testing::Values(core::BinaryOp::kAdd),
                                          testing::ValuesIn(Concat(  //
                                              OpAddIntCases<AInt>(),
                                              OpAddIntCases<i32>(),
                                              OpAddIntCases<u32>(),
                                              OpAddFloatCases<AFloat>(),
                                              OpAddFloatCases<f32>(),
                                              OpAddFloatCases<f16>()))));

template <typename T>
std::vector<Case> OpSubIntCases() {
    static_assert(IsIntegral<T>);
    auto r = std::vector<Case>{
        C(T{0}, T{0}, T{0}),
        C(T{3}, T{2}, T{1}),
        C(T{T::Lowest() + 1}, T{1}, T::Lowest()),
        C(T{T::Highest() - 1}, Negate(T{1}), T::Highest()),
        C(Negate(T{1}), T::Highest(), T::Lowest()),
    };
    if constexpr (IsAbstract<T>) {
        auto error_msg = [](auto a, auto b) {
            return "12:34 error: " + OverflowErrorMessage(a, "-", b);
        };
        ConcatInto(  //
            r, std::vector<Case>{
                   E(T::Lowest(), T{1}, error_msg(T::Lowest(), T{1})),
                   E(T::Highest(), Negate(T{1}), error_msg(T::Highest(), Negate(T{1}))),
               });
    } else {
        ConcatInto(  //
            r, std::vector<Case>{
                   C(T::Lowest(), T{1}, T::Highest()),
                   C(T::Highest(), Negate(T{1}), T::Lowest()),
               });
    }
    return r;
}
template <typename T>
std::vector<Case> OpSubFloatCases() {
    static_assert(IsFloatingPoint<T>);
    auto error_msg = [](auto a, auto b) {
        return "12:34 error: " + OverflowErrorMessage(a, "-", b);
    };
    return std::vector<Case>{
        C(T{0}, T{0}, T{0}),
        C(T{3}, T{2}, T{1}),
        C(T::Highest(), T{1}, T{T::Highest() - 1}),
        C(T::Lowest(), Negate(T{1}), T{T::Lowest() + 1}),
        C(T{0}, T::Highest(), T::Lowest()),

        E(T::Highest(), Negate(T::Highest()), error_msg(T::Highest(), Negate(T::Highest()))),
        E(T::Lowest(), T::Highest(), error_msg(T::Lowest(), T::Highest())),
    };
}
INSTANTIATE_TEST_SUITE_P(Sub,
                         ConstEvalBinaryOpTest,
                         testing::Combine(testing::Values(core::BinaryOp::kSubtract),
                                          testing::ValuesIn(Concat(  //
                                              OpSubIntCases<AInt>(),
                                              OpSubIntCases<i32>(),
                                              OpSubIntCases<u32>(),
                                              OpSubFloatCases<AFloat>(),
                                              OpSubFloatCases<f32>(),
                                              OpSubFloatCases<f16>()))));

template <typename T>
std::vector<Case> OpMulScalarCases() {
    auto r = std::vector<Case>{
        C(T{0}, T{0}, T{0}),
        C(T{1}, T{2}, T{2}),
        C(T{2}, T{3}, T{6}),
        C(Negate(T{2}), T{3}, Negate(T{6})),
        C(T::Highest(), T{1}, T::Highest()),
        C(T::Lowest(), T{1}, T::Lowest()),
    };
    if constexpr (IsAbstract<T> || IsFloatingPoint<T>) {
        auto error_msg = [](auto a, auto b) {
            return "12:34 error: " + OverflowErrorMessage(a, "*", b);
        };
        ConcatInto(  //
            r, std::vector<Case>{
                   // Fail if result is +/-inf
                   E(T::Highest(), T::Highest(), error_msg(T::Highest(), T::Highest())),
                   E(T::Lowest(), T::Lowest(), error_msg(T::Lowest(), T::Lowest())),
                   E(T::Highest(), T{2}, error_msg(T::Highest(), T{2})),
                   E(T::Lowest(), Negate(T{2}), error_msg(T::Lowest(), Negate(T{2}))),
               });
    } else {
        ConcatInto(  //
            r, std::vector<Case>{
                   C(T::Highest(), T::Highest(), Mul(T::Highest(), T::Highest())),
                   C(T::Lowest(), T::Lowest(), Mul(T::Lowest(), T::Lowest())),
               });
    }
    return r;
}

template <typename T>
std::vector<Case> OpMulVecCases() {
    auto r = std::vector<Case>{
        // s * vec3 = vec3
        C(Val(T{2.0}), Vec(T{1.25}, T{2.25}, T{3.25}), Vec(T{2.5}, T{4.5}, T{6.5})),
        // vec3 * s = vec3
        C(Vec(T{1.25}, T{2.25}, T{3.25}), Val(T{2.0}), Vec(T{2.5}, T{4.5}, T{6.5})),
        // vec3 * vec3 = vec3
        C(Vec(T{1.25}, T{2.25}, T{3.25}), Vec(T{2.0}, T{2.0}, T{2.0}), Vec(T{2.5}, T{4.5}, T{6.5})),
    };
    if constexpr (IsAbstract<T> || IsFloatingPoint<T>) {
        auto error_msg = [](auto a, auto b) {
            return "12:34 error: " + OverflowErrorMessage(a, "*", b);
        };
        ConcatInto(  //
            r,
            std::vector<Case>{
                // Fail if result is +/-inf
                E(Val(T::Highest()), Vec(T{2}, T{1}), error_msg(T::Highest(), T{2})),
                E(Val(T::Lowest()), Vec(Negate(T{2}), T{1}), error_msg(T::Lowest(), Negate(T{2}))),
            });
    } else {
        ConcatInto(  //
            r, std::vector<Case>{
                   C(Val(T::Highest()), Vec(T{2}, T{1}), Vec(T{-2}, T::Highest())),
                   C(Val(T::Lowest()), Vec(Negate(T{2}), T{1}), Vec(T{0}, T{T::Lowest()})),
               });
    }
    return r;
}

template <typename T>
std::vector<Case> OpMulMatCases() {
    auto r = std::vector<Case>{
        // s * mat3x2 = mat3x2
        C(Val(T{2.25}),
          Mat({T{1.0}, T{4.0}},  //
              {T{2.0}, T{5.0}},  //
              {T{3.0}, T{6.0}}),
          Mat({T{2.25}, T{9.0}},   //
              {T{4.5}, T{11.25}},  //
              {T{6.75}, T{13.5}})),
        // mat3x2 * s = mat3x2
        C(Mat({T{1.0}, T{4.0}},  //
              {T{2.0}, T{5.0}},  //
              {T{3.0}, T{6.0}}),
          Val(T{2.25}),
          Mat({T{2.25}, T{9.0}},   //
              {T{4.5}, T{11.25}},  //
              {T{6.75}, T{13.5}})),
        // vec3 * mat2x3 = vec2
        C(Vec(T{1.25}, T{2.25}, T{3.25}),  //
          Mat({T{1.0}, T{2.0}, T{3.0}},    //
              {T{4.0}, T{5.0}, T{6.0}}),   //
          Vec(T{15.5}, T{35.75})),
        // mat2x3 * vec2 = vec3
        C(Mat({T{1.0}, T{2.0}, T{3.0}},   //
              {T{4.0}, T{5.0}, T{6.0}}),  //
          Vec(T{1.25}, T{2.25}),          //
          Vec(T{10.25}, T{13.75}, T{17.25})),
        // mat3x2 * mat2x3 = mat2x2
        C(Mat({T{1.0}, T{2.0}},              //
              {T{3.0}, T{4.0}},              //
              {T{5.0}, T{6.0}}),             //
          Mat({T{1.25}, T{2.25}, T{3.25}},   //
              {T{4.25}, T{5.25}, T{6.25}}),  //
          Mat({T{24.25}, T{31.0}},           //
              {T{51.25}, T{67.0}})),         //
    };
    auto error_msg = [](auto a, const char* op, auto b) {
        return "12:34 error: " + OverflowErrorMessage(a, op, b);
    };
    ConcatIntoIf<IsAbstract<T> || IsFloatingPoint<T>>(  //
        r, std::vector<Case>{
               // vector-matrix multiply

               // Overflow from first multiplication of dot product of vector and matrix column 0
               // i.e. (v[0] * m[0][0] + v[1] * m[0][1])
               //            ^
               E(Vec(T::Highest(), T{1.0}),  //
                 Mat({T{2.0}, T{1.0}},       //
                     {T{1.0}, T{1.0}}),      //
                 error_msg(T{2}, "*", T::Highest())),

               // Overflow from second multiplication of dot product of vector and matrix column 0
               // i.e. (v[0] * m[0][0] + v[1] * m[0][1])
               //                             ^
               E(Vec(T{1.0}, T::Highest()),  //
                 Mat({T{1.0}, T{2.0}},       //
                     {T{1.0}, T{1.0}}),      //
                 error_msg(T{2}, "*", T::Highest())),

               // Overflow from addition of dot product of vector and matrix column 0
               // i.e. (v[0] * m[0][0] + v[1] * m[0][1])
               //                      ^
               E(Vec(T::Highest(), T::Highest()),  //
                 Mat({T{1.0}, T{1.0}},             //
                     {T{1.0}, T{1.0}}),            //
                 error_msg(T::Highest(), "+", T::Highest())),

               // matrix-matrix multiply

               // Overflow from first multiplication of dot product of lhs row 0 and rhs column 0
               // i.e. m1[0][0] * m2[0][0] + m1[0][1] * m[1][0]
               //               ^
               E(Mat({T::Highest(), T{1.0}},  //
                     {T{1.0}, T{1.0}}),       //
                 Mat({T{2.0}, T{1.0}},        //
                     {T{1.0}, T{1.0}}),       //
                 error_msg(T::Highest(), "*", T{2.0})),

               // Overflow from second multiplication of dot product of lhs row 0 and rhs column 0
               // i.e. m1[0][0] * m2[0][0] + m1[0][1] * m[1][0]
               //                                     ^
               E(Mat({T{1.0}, T{1.0}},         //
                     {T::Highest(), T{1.0}}),  //
                 Mat({T{1.0}, T{2.0}},         //
                     {T{1.0}, T{1.0}}),        //
                 error_msg(T::Highest(), "*", T{2.0})),

               // Overflow from addition of dot product of lhs row 0 and rhs column 0
               // i.e. m1[0][0] * m2[0][0] + m1[0][1] * m[1][0]
               //                          ^
               E(Mat({T::Highest(), T{1.0}},   //
                     {T::Highest(), T{1.0}}),  //
                 Mat({T{1.0}, T{1.0}},         //
                     {T{1.0}, T{1.0}}),        //
                 error_msg(T::Highest(), "+", T::Highest())),
           });

    return r;
}

INSTANTIATE_TEST_SUITE_P(Mul,
                         ConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(core::BinaryOp::kMultiply),
                             testing::ValuesIn(Concat(  //
                                 OpMulScalarCases<AInt>(),
                                 OpMulScalarCases<i32>(),
                                 OpMulScalarCases<u32>(),
                                 OpMulScalarCases<AFloat>(),
                                 OpMulScalarCases<f32>(),
                                 OpMulScalarCases<f16>(),
                                 OpMulVecCases<AInt>(),
                                 OpMulVecCases<i32>(),
                                 OpMulVecCases<u32>(),
                                 OpMulVecCases<AFloat>(),
                                 OpMulVecCases<f32>(),
                                 OpMulVecCases<f16>(),
                                 OpMulMatCases<AFloat>(),
                                 OpMulMatCases<f32>(),
                                 OpMulMatCases<f16>()))));

template <typename T>
std::vector<Case> OpDivIntCases() {
    auto error_msg = [](auto a, auto b) {
        return "12:34 error: " + OverflowErrorMessage(a, "/", b);
    };

    std::vector<Case> r = {
        C(T{0}, T{1}, T{0}),
        C(T{1}, T{1}, T{1}),
        C(T{1}, T{1}, T{1}),
        C(T{2}, T{1}, T{2}),
        C(T{4}, T{2}, T{2}),
        C(T::Highest(), T{1}, T::Highest()),
        C(T::Lowest(), T{1}, T::Lowest()),
        C(T::Highest(), T::Highest(), T{1}),
        C(T{0}, T::Highest(), T{0}),

        // Divide by zero
        E(T{123}, T{0}, "12:34 error: integer division by zero is invalid"),
        E(T::Highest(), T{0}, "12:34 error: integer division by zero is invalid"),
        E(T::Lowest(), T{0}, "12:34 error: integer division by zero is invalid"),
    };

    // Error on most negative divided by -1
    ConcatIntoIf<IsSignedIntegral<T>>(  //
        r, std::vector<Case>{
               E(T::Lowest(), T{-1}, error_msg(T::Lowest(), T{-1})),
           });
    return r;
}

template <typename T>
std::vector<Case> OpDivFloatCases() {
    auto error_msg = [](auto a, auto b) {
        return "12:34 error: " + OverflowErrorMessage(a, "/", b);
    };
    std::vector<Case> r = {
        C(T{0}, T{1}, T{0}),
        C(T{1}, T{1}, T{1}),
        C(T{1}, T{1}, T{1}),
        C(T{2}, T{1}, T{2}),
        C(T{4}, T{2}, T{2}),
        C(T::Highest(), T{1}, T::Highest()),
        C(T::Lowest(), T{1}, T::Lowest()),
        C(T::Highest(), T::Highest(), T{1}),
        C(T{0}, T::Highest(), T{0}),
        C(T{0}, T::Lowest(), -T{0}),

        // Divide by zero
        E(T{123}, T{0}, error_msg(T{123}, T{0})),
        E(Negate(T{123}), Negate(T{0}), error_msg(Negate(T{123}), Negate(T{0}))),
        E(Negate(T{123}), T{0}, error_msg(Negate(T{123}), T{0})),
        E(T{123}, Negate(T{0}), error_msg(T{123}, Negate(T{0}))),
    };
    return r;
}
INSTANTIATE_TEST_SUITE_P(Div,
                         ConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(core::BinaryOp::kDivide),
                             testing::ValuesIn(Concat(  //
                                 OpDivIntCases<AInt>(),
                                 OpDivIntCases<i32>(),
                                 OpDivIntCases<u32>(),
                                 OpDivFloatCases<AFloat>(),
                                 OpDivFloatCases<f32>(),
                                 OpDivFloatCases<f16>()))));

template <typename T>
std::vector<Case> OpModCases() {
    auto error_msg = [](auto a, auto b) {
        return "12:34 error: " + OverflowErrorMessage(a, "%", b);
    };

    // Common cases for all types
    std::vector<Case> r = {
        C(T{0}, T{1}, T{0}),    //
        C(T{1}, T{1}, T{0}),    //
        C(T{10}, T{1}, T{0}),   //
        C(T{10}, T{2}, T{0}),   //
        C(T{10}, T{3}, T{1}),   //
        C(T{10}, T{4}, T{2}),   //
        C(T{10}, T{5}, T{0}),   //
        C(T{10}, T{6}, T{4}),   //
        C(T{10}, T{5}, T{0}),   //
        C(T{10}, T{8}, T{2}),   //
        C(T{10}, T{9}, T{1}),   //
        C(T{10}, T{10}, T{0}),  //

        // Error on divide by zero
        E(T{123}, T{0},
          IsIntegral<T> ? "12:34 error: integer division by zero is invalid"
                        : error_msg(T{123}, T{0})),
        E(T::Highest(), T{0},
          IsIntegral<T> ? "12:34 error: integer division by zero is invalid"
                        : error_msg(T::Highest(), T{0})),
        E(T::Lowest(), T{0},
          IsIntegral<T> ? "12:34 error: integer division by zero is invalid"
                        : error_msg(T::Lowest(), T{0})),
    };

    if constexpr (IsIntegral<T>) {
        ConcatInto(  //
            r, std::vector<Case>{
                   C(T::Highest(), T{T::Highest() - T{1}}, T{1}),
               });
    }

    if constexpr (IsSignedIntegral<T>) {
        ConcatInto(  //
            r, std::vector<Case>{
                   C(T::Lowest(), T{T::Lowest() + T{1}}, -T(1)),

                   // Error on most negative integer divided by -1
                   E(T::Lowest(), T{-1}, error_msg(T::Lowest(), T{-1})),
               });
    }

    // Negative values (both signed integrals and floating point)
    if constexpr (IsSignedIntegral<T> || IsFloatingPoint<T>) {
        ConcatInto(  //
            r, std::vector<Case>{
                   C(-T{1}, T{1}, T{0}),  //

                   // lhs negative, rhs positive
                   C(-T{10}, T{1}, T{0}),   //
                   C(-T{10}, T{2}, T{0}),   //
                   C(-T{10}, T{3}, -T{1}),  //
                   C(-T{10}, T{4}, -T{2}),  //
                   C(-T{10}, T{5}, T{0}),   //
                   C(-T{10}, T{6}, -T{4}),  //
                   C(-T{10}, T{5}, T{0}),   //
                   C(-T{10}, T{8}, -T{2}),  //
                   C(-T{10}, T{9}, -T{1}),  //
                   C(-T{10}, T{10}, T{0}),  //

                   // lhs positive, rhs negative
                   C(T{10}, -T{1}, T{0}),   //
                   C(T{10}, -T{2}, T{0}),   //
                   C(T{10}, -T{3}, T{1}),   //
                   C(T{10}, -T{4}, T{2}),   //
                   C(T{10}, -T{5}, T{0}),   //
                   C(T{10}, -T{6}, T{4}),   //
                   C(T{10}, -T{5}, T{0}),   //
                   C(T{10}, -T{8}, T{2}),   //
                   C(T{10}, -T{9}, T{1}),   //
                   C(T{10}, -T{10}, T{0}),  //

                   // lhs negative, rhs negative
                   C(-T{10}, -T{1}, T{0}),   //
                   C(-T{10}, -T{2}, T{0}),   //
                   C(-T{10}, -T{3}, -T{1}),  //
                   C(-T{10}, -T{4}, -T{2}),  //
                   C(-T{10}, -T{5}, T{0}),   //
                   C(-T{10}, -T{6}, -T{4}),  //
                   C(-T{10}, -T{5}, T{0}),   //
                   C(-T{10}, -T{8}, -T{2}),  //
                   C(-T{10}, -T{9}, -T{1}),  //
                   C(-T{10}, -T{10}, T{0}),  //
               });
    }

    // Float values
    if constexpr (IsFloatingPoint<T>) {
        ConcatInto(  //
            r, std::vector<Case>{
                   C(T{10.5}, T{1}, T{0.5}),   //
                   C(T{10.5}, T{2}, T{0.5}),   //
                   C(T{10.5}, T{3}, T{1.5}),   //
                   C(T{10.5}, T{4}, T{2.5}),   //
                   C(T{10.5}, T{5}, T{0.5}),   //
                   C(T{10.5}, T{6}, T{4.5}),   //
                   C(T{10.5}, T{5}, T{0.5}),   //
                   C(T{10.5}, T{8}, T{2.5}),   //
                   C(T{10.5}, T{9}, T{1.5}),   //
                   C(T{10.5}, T{10}, T{0.5}),  //

                   // lhs negative, rhs positive
                   C(-T{10.5}, T{1}, -T{0.5}),   //
                   C(-T{10.5}, T{2}, -T{0.5}),   //
                   C(-T{10.5}, T{3}, -T{1.5}),   //
                   C(-T{10.5}, T{4}, -T{2.5}),   //
                   C(-T{10.5}, T{5}, -T{0.5}),   //
                   C(-T{10.5}, T{6}, -T{4.5}),   //
                   C(-T{10.5}, T{5}, -T{0.5}),   //
                   C(-T{10.5}, T{8}, -T{2.5}),   //
                   C(-T{10.5}, T{9}, -T{1.5}),   //
                   C(-T{10.5}, T{10}, -T{0.5}),  //

                   // lhs positive, rhs negative
                   C(T{10.5}, -T{1}, T{0.5}),   //
                   C(T{10.5}, -T{2}, T{0.5}),   //
                   C(T{10.5}, -T{3}, T{1.5}),   //
                   C(T{10.5}, -T{4}, T{2.5}),   //
                   C(T{10.5}, -T{5}, T{0.5}),   //
                   C(T{10.5}, -T{6}, T{4.5}),   //
                   C(T{10.5}, -T{5}, T{0.5}),   //
                   C(T{10.5}, -T{8}, T{2.5}),   //
                   C(T{10.5}, -T{9}, T{1.5}),   //
                   C(T{10.5}, -T{10}, T{0.5}),  //

                   // lhs negative, rhs negative
                   C(-T{10.5}, -T{1}, -T{0.5}),   //
                   C(-T{10.5}, -T{2}, -T{0.5}),   //
                   C(-T{10.5}, -T{3}, -T{1.5}),   //
                   C(-T{10.5}, -T{4}, -T{2.5}),   //
                   C(-T{10.5}, -T{5}, -T{0.5}),   //
                   C(-T{10.5}, -T{6}, -T{4.5}),   //
                   C(-T{10.5}, -T{5}, -T{0.5}),   //
                   C(-T{10.5}, -T{8}, -T{2.5}),   //
                   C(-T{10.5}, -T{9}, -T{1.5}),   //
                   C(-T{10.5}, -T{10}, -T{0.5}),  //
               });
    }

    return r;
}
INSTANTIATE_TEST_SUITE_P(Mod,
                         ConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(core::BinaryOp::kModulo),
                             testing::ValuesIn(Concat(  //
                                 OpModCases<AInt>(),
                                 OpModCases<i32>(),
                                 OpModCases<u32>(),
                                 OpModCases<AFloat>(),
                                 OpModCases<f32>(),
                                 OpModCases<f16>()))));

template <typename T, bool equals>
std::vector<Case> OpEqualCases() {
    return {
        C(T{0}, T{0}, true == equals),
        C(T{0}, T{1}, false == equals),
        C(T{1}, T{0}, false == equals),
        C(T{1}, T{1}, true == equals),
        C(Vec(T{0}, T{0}), Vec(T{0}, T{0}), Vec(true == equals, true == equals)),
        C(Vec(T{1}, T{0}), Vec(T{0}, T{1}), Vec(false == equals, false == equals)),
        C(Vec(T{1}, T{1}), Vec(T{0}, T{1}), Vec(false == equals, true == equals)),
    };
}
INSTANTIATE_TEST_SUITE_P(Equal,
                         ConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(core::BinaryOp::kEqual),
                             testing::ValuesIn(Concat(  //
                                 OpEqualCases<AInt, true>(),
                                 OpEqualCases<i32, true>(),
                                 OpEqualCases<u32, true>(),
                                 OpEqualCases<AFloat, true>(),
                                 OpEqualCases<f32, true>(),
                                 OpEqualCases<f16, true>(),
                                 OpEqualCases<bool, true>()))));
INSTANTIATE_TEST_SUITE_P(NotEqual,
                         ConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(core::BinaryOp::kNotEqual),
                             testing::ValuesIn(Concat(  //
                                 OpEqualCases<AInt, false>(),
                                 OpEqualCases<i32, false>(),
                                 OpEqualCases<u32, false>(),
                                 OpEqualCases<AFloat, false>(),
                                 OpEqualCases<f32, false>(),
                                 OpEqualCases<f16, false>(),
                                 OpEqualCases<bool, false>()))));

template <typename T, bool less_than>
std::vector<Case> OpLessThanCases() {
    return {
        C(T{0}, T{0}, false == less_than),
        C(T{0}, T{1}, true == less_than),
        C(T{1}, T{0}, false == less_than),
        C(T{1}, T{1}, false == less_than),
        C(Vec(T{0}, T{0}), Vec(T{0}, T{0}), Vec(false == less_than, false == less_than)),
        C(Vec(T{0}, T{0}), Vec(T{1}, T{1}), Vec(true == less_than, true == less_than)),
        C(Vec(T{1}, T{1}), Vec(T{0}, T{0}), Vec(false == less_than, false == less_than)),
        C(Vec(T{1}, T{0}), Vec(T{0}, T{1}), Vec(false == less_than, true == less_than)),
    };
}
INSTANTIATE_TEST_SUITE_P(LessThan,
                         ConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(core::BinaryOp::kLessThan),
                             testing::ValuesIn(Concat(  //
                                 OpLessThanCases<AInt, true>(),
                                 OpLessThanCases<i32, true>(),
                                 OpLessThanCases<u32, true>(),
                                 OpLessThanCases<AFloat, true>(),
                                 OpLessThanCases<f32, true>(),
                                 OpLessThanCases<f16, true>()))));
INSTANTIATE_TEST_SUITE_P(GreaterThanEqual,
                         ConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(core::BinaryOp::kGreaterThanEqual),
                             testing::ValuesIn(Concat(  //
                                 OpLessThanCases<AInt, false>(),
                                 OpLessThanCases<i32, false>(),
                                 OpLessThanCases<u32, false>(),
                                 OpLessThanCases<AFloat, false>(),
                                 OpLessThanCases<f32, false>(),
                                 OpLessThanCases<f16, false>()))));

template <typename T, bool greater_than>
std::vector<Case> OpGreaterThanCases() {
    return {
        C(T{0}, T{0}, false == greater_than),
        C(T{0}, T{1}, false == greater_than),
        C(T{1}, T{0}, true == greater_than),
        C(T{1}, T{1}, false == greater_than),
        C(Vec(T{0}, T{0}), Vec(T{0}, T{0}), Vec(false == greater_than, false == greater_than)),
        C(Vec(T{1}, T{1}), Vec(T{0}, T{0}), Vec(true == greater_than, true == greater_than)),
        C(Vec(T{0}, T{0}), Vec(T{1}, T{1}), Vec(false == greater_than, false == greater_than)),
        C(Vec(T{1}, T{0}), Vec(T{0}, T{1}), Vec(true == greater_than, false == greater_than)),
    };
}
INSTANTIATE_TEST_SUITE_P(GreaterThan,
                         ConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(core::BinaryOp::kGreaterThan),
                             testing::ValuesIn(Concat(  //
                                 OpGreaterThanCases<AInt, true>(),
                                 OpGreaterThanCases<i32, true>(),
                                 OpGreaterThanCases<u32, true>(),
                                 OpGreaterThanCases<AFloat, true>(),
                                 OpGreaterThanCases<f32, true>(),
                                 OpGreaterThanCases<f16, true>()))));
INSTANTIATE_TEST_SUITE_P(LessThanEqual,
                         ConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(core::BinaryOp::kLessThanEqual),
                             testing::ValuesIn(Concat(  //
                                 OpGreaterThanCases<AInt, false>(),
                                 OpGreaterThanCases<i32, false>(),
                                 OpGreaterThanCases<u32, false>(),
                                 OpGreaterThanCases<AFloat, false>(),
                                 OpGreaterThanCases<f32, false>(),
                                 OpGreaterThanCases<f16, false>()))));

// Test that we can compare the maximum and minimum AFloat values in vectors (crbug.com/tint/1999).
struct AbstractFloatVectorCompareCase {
    core::BinaryOp op;
    bool expected_0;
    bool expected_1;
};
using ConstEvalBinaryOpAbstractFloatVectorCompareTest =
    ConstEvalTestWithParam<AbstractFloatVectorCompareCase>;
TEST_P(ConstEvalBinaryOpAbstractFloatVectorCompareTest, Test) {
    auto params = GetParam();

    auto* lhs_expr = Call(ty.vec2<AFloat>(), AFloat::Highest(), AFloat::Lowest());
    auto* rhs_expr = Call(ty.vec2<AFloat>(), AFloat::Lowest(), AFloat::Highest());
    auto* expr = create<ast::BinaryExpression>(Source{{12, 34}}, params.op, lhs_expr, rhs_expr);
    GlobalConst("C", expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* value = Sem().Get(expr)->ConstantValue();
    ASSERT_NE(value, nullptr);
    CheckConstant(value, Value::Create<bool>(Vector{builder::Scalar(params.expected_0),
                                                    builder::Scalar(params.expected_1)}));
}
INSTANTIATE_TEST_SUITE_P(
    HighestLowest,
    ConstEvalBinaryOpAbstractFloatVectorCompareTest,
    testing::Values(AbstractFloatVectorCompareCase{core::BinaryOp::kEqual, false, false},
                    AbstractFloatVectorCompareCase{core::BinaryOp::kNotEqual, true, true},
                    AbstractFloatVectorCompareCase{core::BinaryOp::kLessThan, false, true},
                    AbstractFloatVectorCompareCase{core::BinaryOp::kLessThanEqual, false, true},
                    AbstractFloatVectorCompareCase{core::BinaryOp::kGreaterThan, true, false},
                    AbstractFloatVectorCompareCase{core::BinaryOp::kGreaterThanEqual, true,
                                                   false}));

static std::vector<Case> OpLogicalAndCases() {
    return {
        C(true, true, true),
        C(true, false, false),
        C(false, true, false),
        C(false, false, false),
    };
}
INSTANTIATE_TEST_SUITE_P(LogicalAnd,
                         ConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(core::BinaryOp::kLogicalAnd),
                             testing::ValuesIn(OpLogicalAndCases())));

static std::vector<Case> OpLogicalOrCases() {
    return {
        C(true, true, true),
        C(true, false, true),
        C(false, true, true),
        C(false, false, false),
    };
}
INSTANTIATE_TEST_SUITE_P(LogicalOr,
                         ConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(core::BinaryOp::kLogicalOr),
                             testing::ValuesIn(OpLogicalOrCases())));

static std::vector<Case> OpAndBoolCases() {
    return {
        C(true, true, true),
        C(true, false, false),
        C(false, true, false),
        C(false, false, false),
        C(Vec(true, true), Vec(true, false), Vec(true, false)),
        C(Vec(true, true), Vec(false, true), Vec(false, true)),
        C(Vec(true, false), Vec(true, false), Vec(true, false)),
        C(Vec(false, true), Vec(true, false), Vec(false, false)),
        C(Vec(false, false), Vec(true, false), Vec(false, false)),
    };
}
template <typename T>
std::vector<Case> OpAndIntCases() {
    using B = BitValues<T>;
    return {
        C(T{0b1010}, T{0b1111}, T{0b1010}),
        C(T{0b1010}, T{0b0000}, T{0b0000}),
        C(T{0b1010}, T{0b0011}, T{0b0010}),
        C(T{0b1010}, T{0b1100}, T{0b1000}),
        C(T{0b1010}, T{0b0101}, T{0b0000}),
        C(B::All, B::All, B::All),
        C(B::LeftMost, B::LeftMost, B::LeftMost),
        C(B::RightMost, B::RightMost, B::RightMost),
        C(B::All, T{0}, T{0}),
        C(T{0}, B::All, T{0}),
        C(B::LeftMost, B::AllButLeftMost, T{0}),
        C(B::AllButLeftMost, B::LeftMost, T{0}),
        C(B::RightMost, B::AllButRightMost, T{0}),
        C(B::AllButRightMost, B::RightMost, T{0}),
        C(Vec(B::All, B::LeftMost, B::RightMost),      //
          Vec(B::All, B::All, B::All),                 //
          Vec(B::All, B::LeftMost, B::RightMost)),     //
        C(Vec(B::All, B::LeftMost, B::RightMost),      //
          Vec(T{0}, T{0}, T{0}),                       //
          Vec(T{0}, T{0}, T{0})),                      //
        C(Vec(B::LeftMost, B::RightMost),              //
          Vec(B::AllButLeftMost, B::AllButRightMost),  //
          Vec(T{0}, T{0})),
    };
}
INSTANTIATE_TEST_SUITE_P(And,
                         ConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(core::BinaryOp::kAnd),
                             testing::ValuesIn(            //
                                 Concat(OpAndBoolCases(),  //
                                        OpAndIntCases<AInt>(),
                                        OpAndIntCases<i32>(),
                                        OpAndIntCases<u32>()))));

static std::vector<Case> OpOrBoolCases() {
    return {
        C(true, true, true),
        C(true, false, true),
        C(false, true, true),
        C(false, false, false),
        C(Vec(true, true), Vec(true, false), Vec(true, true)),
        C(Vec(true, true), Vec(false, true), Vec(true, true)),
        C(Vec(true, false), Vec(true, false), Vec(true, false)),
        C(Vec(false, true), Vec(true, false), Vec(true, true)),
        C(Vec(false, false), Vec(true, false), Vec(true, false)),
    };
}
template <typename T>
std::vector<Case> OpOrIntCases() {
    using B = BitValues<T>;
    return {
        C(T{0b1010}, T{0b1111}, T{0b1111}),
        C(T{0b1010}, T{0b0000}, T{0b1010}),
        C(T{0b1010}, T{0b0011}, T{0b1011}),
        C(T{0b1010}, T{0b1100}, T{0b1110}),
        C(T{0b1010}, T{0b0101}, T{0b1111}),
        C(B::All, B::All, B::All),
        C(B::LeftMost, B::LeftMost, B::LeftMost),
        C(B::RightMost, B::RightMost, B::RightMost),
        C(B::All, T{0}, B::All),
        C(T{0}, B::All, B::All),
        C(B::LeftMost, B::AllButLeftMost, B::All),
        C(B::AllButLeftMost, B::LeftMost, B::All),
        C(B::RightMost, B::AllButRightMost, B::All),
        C(B::AllButRightMost, B::RightMost, B::All),
        C(Vec(B::All, B::LeftMost, B::RightMost),      //
          Vec(B::All, B::All, B::All),                 //
          Vec(B::All, B::All, B::All)),                //
        C(Vec(B::All, B::LeftMost, B::RightMost),      //
          Vec(T{0}, T{0}, T{0}),                       //
          Vec(B::All, B::LeftMost, B::RightMost)),     //
        C(Vec(B::LeftMost, B::RightMost),              //
          Vec(B::AllButLeftMost, B::AllButRightMost),  //
          Vec(B::All, B::All)),
    };
}
INSTANTIATE_TEST_SUITE_P(Or,
                         ConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(core::BinaryOp::kOr),
                             testing::ValuesIn(Concat(OpOrBoolCases(),
                                                      OpOrIntCases<AInt>(),
                                                      OpOrIntCases<i32>(),
                                                      OpOrIntCases<u32>()))));

TEST_F(ConstEvalTest, NotAndOrOfVecs) {
    auto v1 = Vec(true, true).Expr(*this);
    auto v2 = Vec(true, false).Expr(*this);
    auto v3 = Vec(false, true).Expr(*this);
    auto expr = Not(Or(And(v1, v2), v3));
    GlobalConst("C", expr);
    auto expected_expr = Vec(false, false).Expr(*this);
    GlobalConst("E", expected_expr);
    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    const constant::Value* value = sem->ConstantValue();
    ASSERT_NE(value, nullptr);
    EXPECT_TYPE(value->Type(), sem->Type());

    auto* expected_sem = Sem().GetVal(expected_expr);
    const constant::Value* expected_value = expected_sem->ConstantValue();
    ASSERT_NE(expected_value, nullptr);
    EXPECT_TYPE(expected_value->Type(), expected_sem->Type());

    ForEachElemPair(value, expected_value, [&](const constant::Value* a, const constant::Value* b) {
        EXPECT_EQ(a->ValueAs<bool>(), b->ValueAs<bool>());
        return HasFailure() ? Action::kStop : Action::kContinue;
    });
}

template <typename T>
std::vector<Case> XorCases() {
    using B = BitValues<T>;
    return {
        C(T{0b1010}, T{0b1111}, T{0b0101}),
        C(T{0b1010}, T{0b0000}, T{0b1010}),
        C(T{0b1010}, T{0b0011}, T{0b1001}),
        C(T{0b1010}, T{0b1100}, T{0b0110}),
        C(T{0b1010}, T{0b0101}, T{0b1111}),
        C(B::All, B::All, T{0}),
        C(B::LeftMost, B::LeftMost, T{0}),
        C(B::RightMost, B::RightMost, T{0}),
        C(B::All, T{0}, B::All),
        C(T{0}, B::All, B::All),
        C(B::LeftMost, B::AllButLeftMost, B::All),
        C(B::AllButLeftMost, B::LeftMost, B::All),
        C(B::RightMost, B::AllButRightMost, B::All),
        C(B::AllButRightMost, B::RightMost, B::All),
        C(Vec(B::All, B::LeftMost, B::RightMost),             //
          Vec(B::All, B::All, B::All),                        //
          Vec(T{0}, B::AllButLeftMost, B::AllButRightMost)),  //
        C(Vec(B::All, B::LeftMost, B::RightMost),             //
          Vec(T{0}, T{0}, T{0}),                              //
          Vec(B::All, B::LeftMost, B::RightMost)),            //
        C(Vec(B::LeftMost, B::RightMost),                     //
          Vec(B::AllButLeftMost, B::AllButRightMost),         //
          Vec(B::All, B::All)),
    };
}
INSTANTIATE_TEST_SUITE_P(Xor,
                         ConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(core::BinaryOp::kXor),
                             testing::ValuesIn(Concat(XorCases<AInt>(),  //
                                                      XorCases<i32>(),   //
                                                      XorCases<u32>()))));

template <typename T>
std::vector<Case> ShiftLeftCases() {
    using ST = u32;  // Shift type is u32
    using B = BitValues<T>;
    auto r = std::vector<Case>{
        C(T{0b1010}, ST{0}, T{0b0000'0000'1010}),  //
        C(T{0b1010}, ST{1}, T{0b0000'0001'0100}),  //
        C(T{0b1010}, ST{2}, T{0b0000'0010'1000}),  //
        C(T{0b1010}, ST{3}, T{0b0000'0101'0000}),  //
        C(T{0b1010}, ST{4}, T{0b0000'1010'0000}),  //
        C(T{0b1010}, ST{5}, T{0b0001'0100'0000}),  //
        C(T{0b1010}, ST{6}, T{0b0010'1000'0000}),  //
        C(T{0b1010}, ST{7}, T{0b0101'0000'0000}),  //
        C(T{0b1010}, ST{8}, T{0b1010'0000'0000}),  //
        C(B::LeftMost, ST{0}, B::LeftMost),        //

        C(Vec(T{0b1010}, T{0b1010}),                                            //
          Vec(ST{0}, ST{1}),                                                    //
          Vec(T{0b0000'0000'1010}, T{0b0000'0001'0100})),                       //
        C(Vec(T{0b1010}, T{0b1010}),                                            //
          Vec(ST{2}, ST{3}),                                                    //
          Vec(T{0b0000'0010'1000}, T{0b0000'0101'0000})),                       //
        C(Vec(T{0b1010}, T{0b1010}),                                            //
          Vec(ST{4}, ST{5}),                                                    //
          Vec(T{0b0000'1010'0000}, T{0b0001'0100'0000})),                       //
        C(Vec(T{0b1010}, T{0b1010}, T{0b1010}),                                 //
          Vec(ST{6}, ST{7}, ST{8}),                                             //
          Vec(T{0b0010'1000'0000}, T{0b0101'0000'0000}, T{0b1010'0000'0000})),  //
    };

    // Abstract 0 can be shifted by any u32 value (0 to 2^32), whereas concrete 0 (or any number)
    // can only by shifted by a value less than the number of bits of the lhs.
    // (see ConstEvalShiftLeftConcreteGeqBitWidthError for negative tests)
    ConcatIntoIf<IsAbstract<T>>(  //
        r, std::vector<Case>{
               C(T{0}, ST{64}, T{0}),                              //
               C(T{0}, ST{65}, T{0}),                              //
               C(T{0}, ST{65}, T{0}),                              //
               C(T{0}, ST{10000}, T{0}),                           //
               C(T{0}, ST{u32::Highest()}, T{0}),                  //
               C(Negate(T{0}), ST{64}, Negate(T{0})),              //
               C(Negate(T{0}), ST{65}, Negate(T{0})),              //
               C(Negate(T{0}), ST{65}, Negate(T{0})),              //
               C(Negate(T{0}), ST{10000}, Negate(T{0})),           //
               C(Negate(T{0}), ST{u32::Highest()}, Negate(T{0})),  //
           });

    // Cases that are fine for signed values (no sign change), but would overflow
    // unsigned values. See below for negative tests.
    ConcatIntoIf<IsSignedIntegral<T>>(  //
        r, std::vector<Case>{
               C(B::TwoLeftMost, ST{1}, B::LeftMost),      //
               C(B::All, ST{1}, B::AllButRightMost),       //
               C(B::All, ST{B::NumBits - 1}, B::LeftMost)  //
           });

    // Cases that are fine for unsigned values, but would overflow (sign change) signed
    // values. See ShiftLeftSignChangeErrorCases() for negative tests.
    ConcatIntoIf<IsUnsignedIntegral<T>>(  //
        r, std::vector<Case>{
               C(T{0b0001}, ST{B::NumBits - 1}, B::Lsh(0b0001, B::NumBits - 1)),
               C(T{0b0010}, ST{B::NumBits - 2}, B::Lsh(0b0010, B::NumBits - 2)),
               C(T{0b0100}, ST{B::NumBits - 3}, B::Lsh(0b0100, B::NumBits - 3)),
               C(T{0b1000}, ST{B::NumBits - 4}, B::Lsh(0b1000, B::NumBits - 4)),

               C(T{0b0011}, ST{B::NumBits - 2}, B::Lsh(0b0011, B::NumBits - 2)),
               C(T{0b0110}, ST{B::NumBits - 3}, B::Lsh(0b0110, B::NumBits - 3)),
               C(T{0b1100}, ST{B::NumBits - 4}, B::Lsh(0b1100, B::NumBits - 4)),

               C(B::AllButLeftMost, ST{1}, B::AllButRightMost),
           });

    auto error_msg = [](auto a, auto b) {
        return "12:34 error: " + OverflowErrorMessage(a, "<<", b);
    };
    ConcatIntoIf<IsAbstract<T>>(  //
        r, std::vector<Case>{
               // ShiftLeft of AInts that result in values not representable as AInts.
               // Note that for i32/u32, these would error because shift value is larger than 32.
               E(B::All, T{B::NumBits}, error_msg(B::All, T{B::NumBits})),
               E(B::RightMost, T{B::NumBits}, error_msg(B::RightMost, T{B::NumBits})),
               E(B::AllButLeftMost, T{B::NumBits}, error_msg(B::AllButLeftMost, T{B::NumBits})),
               E(B::AllButLeftMost, T{B::NumBits + 1},
                 error_msg(B::AllButLeftMost, T{B::NumBits + 1})),
               E(B::AllButLeftMost, T{B::NumBits + 1000},
                 error_msg(B::AllButLeftMost, T{B::NumBits + 1000})),
           });
    ConcatIntoIf<IsUnsignedIntegral<T>>(  //
        r, std::vector<Case>{
               // ShiftLeft of u32s that overflow (non-zero bits get shifted out)
               E(T{0b00010}, T{31}, error_msg(T{0b00010}, T{31})),
               E(T{0b00100}, T{30}, error_msg(T{0b00100}, T{30})),
               E(T{0b01000}, T{29}, error_msg(T{0b01000}, T{29})),
               E(T{0b10000}, T{28}, error_msg(T{0b10000}, T{28})),
               //...
               E(T{1 << 28}, T{4}, error_msg(T{1 << 28}, T{4})),
               E(T{1 << 29}, T{3}, error_msg(T{1 << 29}, T{3})),
               E(T{1 << 30}, T{2}, error_msg(T{1 << 30}, T{2})),
               E(T{1u << 31}, T{1}, error_msg(T{1u << 31}, T{1})),

               // And some more
               E(B::All, T{1}, error_msg(B::All, T{1})),
               E(B::AllButLeftMost, T{2}, error_msg(B::AllButLeftMost, T{2})),
           });

    return r;
}
INSTANTIATE_TEST_SUITE_P(ShiftLeft,
                         ConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(core::BinaryOp::kShiftLeft),
                             testing::ValuesIn(Concat(ShiftLeftCases<AInt>(),  //
                                                      ShiftLeftCases<i32>(),   //
                                                      ShiftLeftCases<u32>()))));

TEST_F(ConstEvalTest, BinaryAbstractAddOverflow_AInt) {
    GlobalConst("c", Add(Source{{1, 1}}, Expr(AInt::Highest()), 1_a));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "1:1 error: '9223372036854775807 + 1' cannot be represented as 'abstract-int'");
}

TEST_F(ConstEvalTest, BinaryAbstractAddUnderflow_AInt) {
    GlobalConst("c", Add(Source{{1, 1}}, Expr(AInt::Lowest()), -1_a));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "1:1 error: '-9223372036854775808 + -1' cannot be represented as 'abstract-int'");
}

TEST_F(ConstEvalTest, BinaryAbstractAddOverflow_AFloat) {
    GlobalConst("c", Add(Source{{1, 1}}, Expr(AFloat::Highest()), AFloat::Highest()));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "1:1 error: "
              "'17976931348623157081452742373170435679807056752584499659891747680315726078002853876"
              "058955863276687817154045895351438246423432132688946418276846754670353751698604991057"
              "655128207624549009038932894407586850845513394230458323690322294816580855933212334827"
              "4797826204144723168738177180919299881250404026184124858368.0 + "
              "179769313486231570814527423731704356798070567525844996598917476803157260780028538760"
              "589558632766878171540458953514382464234321326889464182768467546703537516986049910576"
              "551282076245490090389328944075868508455133942304583236903222948165808559332123348274"
              "797826204144723168738177180919299881250404026184124858368.0' cannot be "
              "represented as 'abstract-float'");
}

TEST_F(ConstEvalTest, BinaryAbstractAddUnderflow_AFloat) {
    GlobalConst("c", Add(Source{{1, 1}}, Expr(AFloat::Lowest()), AFloat::Lowest()));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "1:1 error: "
              "'-"
              "179769313486231570814527423731704356798070567525844996598917476803157260780028538760"
              "589558632766878171540458953514382464234321326889464182768467546703537516986049910576"
              "551282076245490090389328944075868508455133942304583236903222948165808559332123348274"
              "797826204144723168738177180919299881250404026184124858368.0 + "
              "-17976931348623157081452742373170435679807056752584499659891747680315726078002853876"
              "058955863276687817154045895351438246423432132688946418276846754670353751698604991057"
              "655128207624549009038932894407586850845513394230458323690322294816580855933212334827"
              "4797826204144723168738177180919299881250404026184124858368.0' cannot be "
              "represented as 'abstract-float'");
}

// Mixed AInt and AFloat args to test implicit conversion to AFloat
INSTANTIATE_TEST_SUITE_P(
    AbstractMixed,
    ConstEvalBinaryOpTest,
    testing::Combine(
        testing::Values(core::BinaryOp::kAdd),
        testing::Values(C(Val(1_a), Val(2.3_a), Val(3.3_a)),
                        C(Val(2.3_a), Val(1_a), Val(3.3_a)),
                        C(Val(1_a), Vec(2.3_a, 2.3_a, 2.3_a), Vec(3.3_a, 3.3_a, 3.3_a)),
                        C(Vec(2.3_a, 2.3_a, 2.3_a), Val(1_a), Vec(3.3_a, 3.3_a, 3.3_a)),
                        C(Vec(2.3_a, 2.3_a, 2.3_a), Val(1_a), Vec(3.3_a, 3.3_a, 3.3_a)),
                        C(Val(1_a), Vec(2.3_a, 2.3_a, 2.3_a), Vec(3.3_a, 3.3_a, 3.3_a)),
                        C(Mat({1_a, 2_a},        //
                              {1_a, 2_a},        //
                              {1_a, 2_a}),       //
                          Mat({1.2_a, 2.3_a},    //
                              {1.2_a, 2.3_a},    //
                              {1.2_a, 2.3_a}),   //
                          Mat({2.2_a, 4.3_a},    //
                              {2.2_a, 4.3_a},    //
                              {2.2_a, 4.3_a})),  //
                        C(Mat({1.2_a, 2.3_a},    //
                              {1.2_a, 2.3_a},    //
                              {1.2_a, 2.3_a}),   //
                          Mat({1_a, 2_a},        //
                              {1_a, 2_a},        //
                              {1_a, 2_a}),       //
                          Mat({2.2_a, 4.3_a},    //
                              {2.2_a, 4.3_a},    //
                              {2.2_a, 4.3_a}))   //
                        )));

// AInt left shift negative value -> error
TEST_F(ConstEvalTest, BinaryAbstractShiftLeftByNegativeValue_Error) {
    GlobalConst("c", Shl(Expr(1_a), Expr(Source{{1, 1}}, -1_a)));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "1:1 error: value -1 cannot be represented as 'u32'");
}

// AInt left shift by AInt or u32 always results in an AInt
TEST_F(ConstEvalTest, BinaryAbstractShiftLeftRemainsAbstract) {
    auto* expr1 = Shl(Expr(1_a), Expr(1_u));
    GlobalConst("c1", expr1);

    auto* expr2 = Shl(Expr(1_a), Expr(1_a));
    GlobalConst("c2", expr2);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem1 = Sem().Get(expr1);
    ASSERT_NE(sem1, nullptr);
    auto* sem2 = Sem().Get(expr2);
    ASSERT_NE(sem2, nullptr);

    auto aint_ty = create<core::type::AbstractInt>();
    EXPECT_EQ(sem1->Type(), aint_ty);
    EXPECT_EQ(sem2->Type(), aint_ty);
}

// i32/u32 left shift by >= 32 -> error
using ConstEvalShiftLeftConcreteGeqBitWidthError = ConstEvalTestWithParam<ErrorCase>;
TEST_P(ConstEvalShiftLeftConcreteGeqBitWidthError, Test) {
    auto* lhs_expr = GetParam().lhs.Expr(*this);
    auto* rhs_expr = GetParam().rhs.Expr(*this);
    GlobalConst("c", Shl(Source{{1, 1}}, lhs_expr, rhs_expr));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        "1:1 error: shift left value must be less than the bit width of the lhs, which is 32");
}
INSTANTIATE_TEST_SUITE_P(Test,
                         ConstEvalShiftLeftConcreteGeqBitWidthError,
                         testing::Values(                                       //
                             ErrorCase{Val(0_u), Val(32_u)},                    //
                             ErrorCase{Val(0_u), Val(33_u)},                    //
                             ErrorCase{Val(0_u), Val(34_u)},                    //
                             ErrorCase{Val(0_u), Val(10000_u)},                 //
                             ErrorCase{Val(0_u), Val(u32::Highest())},          //
                             ErrorCase{Val(0_i), Val(32_u)},                    //
                             ErrorCase{Val(0_i), Val(33_u)},                    //
                             ErrorCase{Val(0_i), Val(34_u)},                    //
                             ErrorCase{Val(0_i), Val(10000_u)},                 //
                             ErrorCase{Val(0_i), Val(u32::Highest())},          //
                             ErrorCase{Val(Negate(0_u)), Val(32_u)},            //
                             ErrorCase{Val(Negate(0_u)), Val(33_u)},            //
                             ErrorCase{Val(Negate(0_u)), Val(34_u)},            //
                             ErrorCase{Val(Negate(0_u)), Val(10000_u)},         //
                             ErrorCase{Val(Negate(0_u)), Val(u32::Highest())},  //
                             ErrorCase{Val(Negate(0_i)), Val(32_u)},            //
                             ErrorCase{Val(Negate(0_i)), Val(33_u)},            //
                             ErrorCase{Val(Negate(0_i)), Val(34_u)},            //
                             ErrorCase{Val(Negate(0_i)), Val(10000_u)},         //
                             ErrorCase{Val(Negate(0_i)), Val(u32::Highest())},  //
                             ErrorCase{Val(1_i), Val(32_u)},                    //
                             ErrorCase{Val(1_i), Val(33_u)},                    //
                             ErrorCase{Val(1_i), Val(34_u)},                    //
                             ErrorCase{Val(1_i), Val(10000_u)},                 //
                             ErrorCase{Val(1_i), Val(u32::Highest())},          //
                             ErrorCase{Val(1_u), Val(32_u)},                    //
                             ErrorCase{Val(1_u), Val(33_u)},                    //
                             ErrorCase{Val(1_u), Val(34_u)},                    //
                             ErrorCase{Val(1_u), Val(10000_u)},                 //
                             ErrorCase{Val(1_u), Val(u32::Highest())}           //
                             ));

// AInt left shift results in sign change error
using ConstEvalShiftLeftSignChangeError = ConstEvalTestWithParam<ErrorCase>;
TEST_P(ConstEvalShiftLeftSignChangeError, Test) {
    auto* lhs_expr = GetParam().lhs.Expr(*this);
    auto* rhs_expr = GetParam().rhs.Expr(*this);
    GlobalConst("c", Shl(Source{{1, 1}}, lhs_expr, rhs_expr));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "1:1 error: shift left operation results in sign change");
}
template <typename T>
std::vector<ErrorCase> ShiftLeftSignChangeErrorCases() {
    // Shift type is u32 for non-abstract
    using ST = std::conditional_t<IsAbstract<T>, T, u32>;
    using B = BitValues<T>;
    return {
        {Val(T{0b0001}), Val(ST{B::NumBits - 1})},
        {Val(T{0b0010}), Val(ST{B::NumBits - 2})},
        {Val(T{0b0100}), Val(ST{B::NumBits - 3})},
        {Val(T{0b1000}), Val(ST{B::NumBits - 4})},
        {Val(T{0b0011}), Val(ST{B::NumBits - 2})},
        {Val(T{0b0110}), Val(ST{B::NumBits - 3})},
        {Val(T{0b1100}), Val(ST{B::NumBits - 4})},
        {Val(B::AllButLeftMost), Val(ST{1})},
        {Val(B::AllButLeftMost), Val(ST{B::NumBits - 1})},
        {Val(B::LeftMost), Val(ST{1})},
        {Val(B::LeftMost), Val(ST{B::NumBits - 1})},
    };
}
INSTANTIATE_TEST_SUITE_P(Test,
                         ConstEvalShiftLeftSignChangeError,
                         testing::ValuesIn(Concat(  //
                             ShiftLeftSignChangeErrorCases<AInt>(),
                             ShiftLeftSignChangeErrorCases<i32>())));

template <typename T>
std::vector<Case> ShiftRightCases() {
    using B = BitValues<T>;
    auto r = std::vector<Case>{
        C(T{0b10101100}, u32{0}, T{0b10101100}),  //
        C(T{0b10101100}, u32{1}, T{0b01010110}),  //
        C(T{0b10101100}, u32{2}, T{0b00101011}),  //
        C(T{0b10101100}, u32{3}, T{0b00010101}),  //
        C(T{0b10101100}, u32{4}, T{0b00001010}),  //
        C(T{0b10101100}, u32{5}, T{0b00000101}),  //
        C(T{0b10101100}, u32{6}, T{0b00000010}),  //
        C(T{0b10101100}, u32{7}, T{0b00000001}),  //
        C(T{0b10101100}, u32{8}, T{0b00000000}),  //
        C(T{0b10101100}, u32{9}, T{0b00000000}),  //
        C(B::LeftMost, u32{0}, B::LeftMost),      //
    };

    // msb not set, same for all types: inserted bit is 0
    ConcatInto(  //
        r, std::vector<Case>{
               C(T{0b01000000000000000000000010101100}, u32{0},  //
                 T{0b01000000000000000000000010101100}),
               C(T{0b01000000000000000000000010101100}, u32{1},  //
                 T{0b00100000000000000000000001010110}),
               C(T{0b01000000000000000000000010101100}, u32{2},  //
                 T{0b00010000000000000000000000101011}),
               C(T{0b01000000000000000000000010101100}, u32{3},  //
                 T{0b00001000000000000000000000010101}),
               C(T{0b01000000000000000000000010101100}, u32{4},  //
                 T{0b00000100000000000000000000001010}),
               C(T{0b01000000000000000000000010101100}, u32{5},  //
                 T{0b00000010000000000000000000000101}),
               C(T{0b01000000000000000000000010101100}, u32{6},  //
                 T{0b00000001000000000000000000000010}),
               C(T{0b01000000000000000000000010101100}, u32{7},  //
                 T{0b00000000100000000000000000000001}),
               C(T{0b01000000000000000000000010101100}, u32{8},  //
                 T{0b00000000010000000000000000000000}),
               C(T{0b01000000000000000000000010101100}, u32{9},  //
                 T{0b00000000001000000000000000000000}),
           });

    // msb set, result differs for i32 and u32
    if constexpr (std::is_same_v<T, u32>) {
        // If unsigned, insert zero bits at the most significant positions.
        ConcatInto(  //
            r, std::vector<Case>{
                   C(T{0b10000000000000000000000010101100}, u32{0},
                     T{0b10000000000000000000000010101100}),
                   C(T{0b10000000000000000000000010101100}, u32{1},
                     T{0b01000000000000000000000001010110}),
                   C(T{0b10000000000000000000000010101100}, u32{2},
                     T{0b00100000000000000000000000101011}),
                   C(T{0b10000000000000000000000010101100}, u32{3},
                     T{0b00010000000000000000000000010101}),
                   C(T{0b10000000000000000000000010101100}, u32{4},
                     T{0b00001000000000000000000000001010}),
                   C(T{0b10000000000000000000000010101100}, u32{5},
                     T{0b00000100000000000000000000000101}),
                   C(T{0b10000000000000000000000010101100}, u32{6},
                     T{0b00000010000000000000000000000010}),
                   C(T{0b10000000000000000000000010101100}, u32{7},
                     T{0b00000001000000000000000000000001}),
                   C(T{0b10000000000000000000000010101100}, u32{8},
                     T{0b00000000100000000000000000000000}),
                   C(T{0b10000000000000000000000010101100}, u32{9},
                     T{0b00000000010000000000000000000000}),
                   // msb shifted by bit width - 1
                   C(T{0b10000000000000000000000000000000}, u32{31},
                     T{0b00000000000000000000000000000001}),
               });
    } else if constexpr (std::is_same_v<T, i32>) {
        // If signed, each inserted bit is 1, so the result is negative.
        ConcatInto(  //
            r, std::vector<Case>{
                   C(T{0b10000000000000000000000010101100}, u32{0},
                     T{0b10000000000000000000000010101100}),  //
                   C(T{0b10000000000000000000000010101100}, u32{1},
                     T{0b11000000000000000000000001010110}),  //
                   C(T{0b10000000000000000000000010101100}, u32{2},
                     T{0b11100000000000000000000000101011}),  //
                   C(T{0b10000000000000000000000010101100}, u32{3},
                     T{0b11110000000000000000000000010101}),  //
                   C(T{0b10000000000000000000000010101100}, u32{4},
                     T{0b11111000000000000000000000001010}),  //
                   C(T{0b10000000000000000000000010101100}, u32{5},
                     T{0b11111100000000000000000000000101}),  //
                   C(T{0b10000000000000000000000010101100}, u32{6},
                     T{0b11111110000000000000000000000010}),  //
                   C(T{0b10000000000000000000000010101100}, u32{7},
                     T{0b11111111000000000000000000000001}),  //
                   C(T{0b10000000000000000000000010101100}, u32{8},
                     T{0b11111111100000000000000000000000}),  //
                   C(T{0b10000000000000000000000010101100}, u32{9},
                     T{0b11111111110000000000000000000000}),  //
                   // msb shifted by bit width - 1
                   C(T{0b10000000000000000000000000000000}, u32{31},
                     T{0b11111111111111111111111111111111}),
               });
    }

    // Test shift right by bit width or more
    if constexpr (IsAbstract<T>) {
        // For abstract int, no error, result is replaced with all msb (-1 for negative, 0 for
        // non-negative)
        ConcatInto(  //
            r, std::vector<Case>{
                   C(T{0}, u32{B::NumBits}, T{0}),
                   C(T{0}, u32{B::NumBits + 1}, T{0}),
                   C(T{0}, u32{B::NumBits + 1000}, T{0}),
                   C(T{42}, u32{B::NumBits}, T{0}),
                   C(T{42}, u32{B::NumBits + 1}, T{0}),
                   C(T{42}, u32{B::NumBits + 1000}, T{0}),
                   C(T{-42}, u32{B::NumBits + 1}, T{-1}),
                   C(T{-42}, u32{B::NumBits + 1000}, T{-1}),
               });
    } else {
        // For concretes, error
        const char* error_msg =
            "12:34 error: shift right value must be less than the bit width of the lhs, which is "
            "32";
        ConcatInto(  //
            r, std::vector<Case>{
                   E(T{0}, u32{B::NumBits}, error_msg),
                   E(T{0}, u32{B::NumBits + 1}, error_msg),
                   E(T{0}, u32{B::NumBits + 1000}, error_msg),
                   E(T{42}, u32{B::NumBits}, error_msg),
                   E(T{42}, u32{B::NumBits + 1}, error_msg),
                   E(T{42}, u32{B::NumBits + 1000}, error_msg),
               });
    }

    return r;
}
INSTANTIATE_TEST_SUITE_P(ShiftRight,
                         ConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(core::BinaryOp::kShiftRight),
                             testing::ValuesIn(Concat(ShiftRightCases<AInt>(),  //
                                                      ShiftRightCases<i32>(),   //
                                                      ShiftRightCases<u32>()))));

namespace LogicalShortCircuit {

/// Validates that `binary` is a short-circuiting logical and expression
static void ValidateAnd(const sem::Info& sem, const ast::BinaryExpression* binary) {
    auto* lhs = binary->lhs;
    auto* rhs = binary->rhs;

    auto* lhs_sem = sem.GetVal(lhs);
    ASSERT_TRUE(lhs_sem->ConstantValue());
    EXPECT_EQ(lhs_sem->ConstantValue()->ValueAs<bool>(), false);
    EXPECT_EQ(lhs_sem->Stage(), core::EvaluationStage::kConstant);

    auto* rhs_sem = sem.GetVal(rhs);
    EXPECT_EQ(rhs_sem->ConstantValue(), nullptr);
    EXPECT_EQ(rhs_sem->Stage(), core::EvaluationStage::kNotEvaluated);

    auto* binary_sem = sem.Get(binary);
    ASSERT_TRUE(binary_sem->ConstantValue());
    EXPECT_EQ(binary_sem->ConstantValue()->ValueAs<bool>(), false);
    EXPECT_EQ(binary_sem->Stage(), core::EvaluationStage::kConstant);
}

/// Validates that `binary` is a short-circuiting logical or expression
static void ValidateOr(const sem::Info& sem, const ast::BinaryExpression* binary) {
    auto* lhs = binary->lhs;
    auto* rhs = binary->rhs;

    auto* lhs_sem = sem.GetVal(lhs);
    ASSERT_TRUE(lhs_sem->ConstantValue());
    EXPECT_EQ(lhs_sem->ConstantValue()->ValueAs<bool>(), true);
    EXPECT_EQ(lhs_sem->Stage(), core::EvaluationStage::kConstant);

    auto* rhs_sem = sem.GetVal(rhs);
    EXPECT_EQ(rhs_sem->ConstantValue(), nullptr);
    EXPECT_EQ(rhs_sem->Stage(), core::EvaluationStage::kNotEvaluated);

    auto* binary_sem = sem.Get(binary);
    ASSERT_TRUE(binary_sem->ConstantValue());
    EXPECT_EQ(binary_sem->ConstantValue()->ValueAs<bool>(), true);
    EXPECT_EQ(binary_sem->Stage(), core::EvaluationStage::kConstant);
}

// Naming convention for tests below:
//
// [Non]ShortCircuit_[And|Or]_[Error|Invalid]_<Op>
//
// Where:
//  ShortCircuit: the rhs will not be const-evaluated
//  NonShortCircuitL the rhs will be const-evaluated
//
//  And/Or: type of binary expression
//
//  Error: a non-const evaluation error (e.g. parser or validation error)
//  Invalid: a const-evaluation error
//
// <Op> the type of operation on the rhs that may or may not be short-circuited.

////////////////////////////////////////////////
// Short-Circuit Unary
////////////////////////////////////////////////

// NOTE: Cannot demonstrate short-circuiting an invalid unary op as const eval of unary does not
// fail.

TEST_F(ConstEvalTest, ShortCircuit_And_Error_Unary) {
    // const one = 1;
    // const result = (one == 0) && (!0);
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 0_a);
    auto* rhs = Not(Source{{12, 34}}, 0_a);
    GlobalConst("result", LogicalAnd(lhs, rhs));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: no matching overload for 'operator ! (abstract-int)'

2 candidate operators:
 • 'operator ! (bool  ✗ ) -> bool'
 • 'operator ! (vecN<bool>  ✗ ) -> vecN<bool>'
)");
}

TEST_F(ConstEvalTest, ShortCircuit_Or_Error_Unary) {
    // const one = 1;
    // const result = (one == 1) || (!0);
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 1_a);
    auto* rhs = Not(Source{{12, 34}}, 0_a);
    GlobalConst("result", LogicalOr(lhs, rhs));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: no matching overload for 'operator ! (abstract-int)'

2 candidate operators:
 • 'operator ! (bool  ✗ ) -> bool'
 • 'operator ! (vecN<bool>  ✗ ) -> vecN<bool>'
)");
}

////////////////////////////////////////////////
// Short-Circuit Binary
////////////////////////////////////////////////

TEST_F(ConstEvalTest, ShortCircuit_And_Invalid_Binary) {
    // const one = 1;
    // const result = (one == 0) && ((2 / 0) == 0);
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 0_a);
    auto* rhs = Equal(Div(2_a, 0_a), 0_a);
    auto* binary = LogicalAnd(lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    ValidateAnd(Sem(), binary);
}

TEST_F(ConstEvalTest, NonShortCircuit_And_Invalid_Binary) {
    // const one = 1;
    // const result = (one == 1) && ((2 / 0) == 0);
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 1_a);
    auto* rhs = Equal(Div(Source{{12, 34}}, 2_a, 0_a), 0_a);
    auto* binary = LogicalAnd(lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: integer division by zero is invalid");
}

TEST_F(ConstEvalTest, ShortCircuit_And_Error_Binary) {
    // const one = 1;
    // const result = (one == 0) && (2 / 0);
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 0_a);
    auto* rhs = Div(2_a, 0_a);
    auto* binary = LogicalAnd(Source{{12, 34}}, lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: no matching overload for 'operator && (bool, abstract-int)'

1 candidate operator:
 • 'operator && (bool  ✓ , bool  ✗ ) -> bool'
)");
}

TEST_F(ConstEvalTest, ShortCircuit_Or_Invalid_Binary) {
    // const one = 1;
    // const result = (one == 1) || ((2 / 0) == 0);
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 1_a);
    auto* rhs = Equal(Div(2_a, 0_a), 0_a);
    auto* binary = LogicalOr(lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    ValidateOr(Sem(), binary);
}

TEST_F(ConstEvalTest, NonShortCircuit_Or_Invalid_Binary) {
    // const one = 1;
    // const result = (one == 0) || ((2 / 0) == 0);
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 0_a);
    auto* rhs = Equal(Div(Source{{12, 34}}, 2_a, 0_a), 0_a);
    auto* binary = LogicalOr(lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: integer division by zero is invalid");
}

TEST_F(ConstEvalTest, ShortCircuit_Or_Error_Binary) {
    // const one = 1;
    // const result = (one == 1) || (2 / 0);
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 1_a);
    auto* rhs = Div(2_a, 0_a);
    auto* binary = LogicalOr(Source{{12, 34}}, lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: no matching overload for 'operator || (bool, abstract-int)'

1 candidate operator:
 • 'operator || (bool  ✓ , bool  ✗ ) -> bool'
)");
}

////////////////////////////////////////////////
// Short-Circuit Materialize
////////////////////////////////////////////////

TEST_F(ConstEvalTest, ShortCircuit_And_Invalid_Materialize) {
    // const one = 1;
    // const result = (one == 0) && (1.7976931348623157e+308 == 0.0f);
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 0_a);
    auto* rhs = Equal(Expr(1.7976931348623157e+308_a), 0_f);
    auto* binary = LogicalAnd(lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    ValidateAnd(Sem(), binary);
}

TEST_F(ConstEvalTest, NonShortCircuit_And_Invalid_Materialize) {
    // const one = 1;
    // const result = (one == 1) && (1.7976931348623157e+308 == 0.0f);
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 1_a);
    auto* rhs = Equal(Expr(Source{{12, 34}}, 1.7976931348623157e+308_a), 0_f);
    auto* binary = LogicalAnd(lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        "12:34 error: value "
        "179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558"
        "632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245"
        "490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168"
        "738177180919299881250404026184124858368.0 cannot be represented as 'f32'");
}

TEST_F(ConstEvalTest, ShortCircuit_And_Error_Materialize) {
    // const one = 1;
    // const result = (one == 0) && (1.7976931348623157e+308 == 0i);
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 0_a);
    auto* rhs = Equal(Source{{12, 34}}, 1.7976931348623157e+308_a, 0_i);
    auto* binary = LogicalAnd(lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: no matching overload for 'operator == (abstract-float, i32)'

2 candidate operators:
 • 'operator == (T  ✓ , T  ✗ ) -> bool' where:
      ✓  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'operator == (vecN<T>  ✗ , vecN<T>  ✗ ) -> vecN<bool>' where:
      ✗  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
)");
}

TEST_F(ConstEvalTest, ShortCircuit_Or_Invalid_Materialize) {
    // const one = 1;
    // const result = (one == 1) || (1.7976931348623157e+308 == 0.0f);
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 1_a);
    auto* rhs = Equal(1.7976931348623157e+308_a, 0_f);
    auto* binary = LogicalOr(lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    ValidateOr(Sem(), binary);
}

TEST_F(ConstEvalTest, NonShortCircuit_Or_Invalid_Materialize) {
    // const one = 1;
    // const result = (one == 0) || (1.7976931348623157e+308 == 0.0f);
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 0_a);
    auto* rhs = Equal(Expr(Source{{12, 34}}, 1.7976931348623157e+308_a), 0_f);
    auto* binary = LogicalOr(lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        "12:34 error: value "
        "179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558"
        "632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245"
        "490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168"
        "738177180919299881250404026184124858368.0 cannot be represented as 'f32'");
}

TEST_F(ConstEvalTest, ShortCircuit_Or_Error_Materialize) {
    // const one = 1;
    // const result = (one == 1) || (1.7976931348623157e+308 == 0i);
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 1_a);
    auto* rhs = Equal(Source{{12, 34}}, Expr(1.7976931348623157e+308_a), 0_i);
    auto* binary = LogicalOr(lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: no matching overload for 'operator == (abstract-float, i32)'

2 candidate operators:
 • 'operator == (T  ✓ , T  ✗ ) -> bool' where:
      ✓  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'operator == (vecN<T>  ✗ , vecN<T>  ✗ ) -> vecN<bool>' where:
      ✗  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
)");
}

////////////////////////////////////////////////
// Short-Circuit Index
////////////////////////////////////////////////

TEST_F(ConstEvalTest, ShortCircuit_And_Invalid_Index) {
    // const one = 1;
    // const a = array(1i, 2i, 3i);
    // const i = 4;
    // const result = (one == 0) && (a[i] == 0);
    GlobalConst("one", Expr(1_a));
    GlobalConst("a", Call<array<i32, 3>>(1_i, 2_i, 3_i));
    GlobalConst("i", Expr(4_a));
    auto* lhs = Equal("one", 0_a);
    auto* rhs = Equal(IndexAccessor("a", "i"), 0_a);
    auto* binary = LogicalAnd(lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    ValidateAnd(Sem(), binary);
}

TEST_F(ConstEvalTest, NonShortCircuit_And_Invalid_Index) {
    // const one = 1;
    // const a = array(1i, 2i, 3i);
    // const i = 3;
    // const result = (one == 1) && (a[i] == 0);
    GlobalConst("one", Expr(1_a));
    GlobalConst("a", Call<array<i32, 3>>(1_i, 2_i, 3_i));
    GlobalConst("i", Expr(3_a));
    auto* lhs = Equal("one", 1_a);
    auto* rhs = Equal(IndexAccessor("a", Expr(Source{{12, 34}}, "i")), 0_a);
    auto* binary = LogicalAnd(lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: index 3 out of bounds [0..2]");
}

TEST_F(ConstEvalTest, ShortCircuit_And_Error_Index) {
    // const one = 1;
    // const a = array(1i, 2i, 3i);
    // const i = 3;
    // const result = (one == 0) && (a[i] == 0.0f);
    GlobalConst("one", Expr(1_a));
    GlobalConst("a", Call<array<i32, 3>>(1_i, 2_i, 3_i));
    GlobalConst("i", Expr(3_a));
    auto* lhs = Equal("one", 0_a);
    auto* rhs = Equal(Source{{12, 34}}, IndexAccessor("a", "i"), 0.0_f);
    auto* binary = LogicalAnd(lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: no matching overload for 'operator == (i32, f32)'

2 candidate operators:
 • 'operator == (T  ✓ , T  ✗ ) -> bool' where:
      ✓  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'operator == (vecN<T>  ✗ , vecN<T>  ✗ ) -> vecN<bool>' where:
      ✗  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
)");
}

TEST_F(ConstEvalTest, ShortCircuit_Or_Invalid_Index) {
    // const one = 1;
    // const a = array(1i, 2i, 3i);
    // const i = 4;
    // const result = (one == 1) || (a[i] == 0);
    GlobalConst("one", Expr(1_a));
    GlobalConst("a", Call<array<i32, 3>>(1_i, 2_i, 3_i));
    GlobalConst("i", Expr(4_a));
    auto* lhs = Equal("one", 1_a);
    auto* rhs = Equal(IndexAccessor("a", "i"), 0_a);
    auto* binary = LogicalOr(lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    ValidateOr(Sem(), binary);
}

TEST_F(ConstEvalTest, NonShortCircuit_Or_Invalid_Index) {
    // const one = 1;
    // const a = array(1i, 2i, 3i);
    // const i = 3;
    // const result = (one == 0) || (a[i] == 0);
    GlobalConst("one", Expr(1_a));
    GlobalConst("a", Call<array<i32, 3>>(1_i, 2_i, 3_i));
    GlobalConst("i", Expr(3_a));
    auto* lhs = Equal("one", 0_a);
    auto* rhs = Equal(IndexAccessor("a", Expr(Source{{12, 34}}, "i")), 0_a);
    auto* binary = LogicalOr(lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: index 3 out of bounds [0..2]");
}

TEST_F(ConstEvalTest, ShortCircuit_Or_Error_Index) {
    // const one = 1;
    // const a = array(1i, 2i, 3i);
    // const i = 3;
    // const result = (one == 1) || (a[i] == 0.0f);
    GlobalConst("one", Expr(1_a));
    GlobalConst("a", Call<array<i32, 3>>(1_i, 2_i, 3_i));
    GlobalConst("i", Expr(3_a));
    auto* lhs = Equal("one", 1_a);
    auto* rhs = Equal(Source{{12, 34}}, IndexAccessor("a", "i"), 0.0_f);
    auto* binary = LogicalOr(lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: no matching overload for 'operator == (i32, f32)'

2 candidate operators:
 • 'operator == (T  ✓ , T  ✗ ) -> bool' where:
      ✓  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'operator == (vecN<T>  ✗ , vecN<T>  ✗ ) -> vecN<bool>' where:
      ✗  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
)");
}

////////////////////////////////////////////////
// Short-Circuit Bitcast
////////////////////////////////////////////////

TEST_F(ConstEvalTest, ShortCircuit_And_Invalid_Bitcast) {
    // const one = 1;
    // const a = 0x7F800000;
    // const result = (one == 0) && (bitcast<f32>(a) == 0.0);
    GlobalConst("one", Expr(1_a));
    GlobalConst("a", Expr(0x7F800000_a));
    auto* lhs = Equal("one", 0_a);
    auto* rhs = Equal(Bitcast<f32>("a"), 0.0_a);
    auto* binary = LogicalAnd(lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    ValidateAnd(Sem(), binary);
}

TEST_F(ConstEvalTest, NonShortCircuit_And_Invalid_Bitcast) {
    // const one = 1;
    // const a = 0x7F800000;
    // const result = (one == 1) && (bitcast<f32>(a) == 0.0);
    GlobalConst("one", Expr(1_a));
    GlobalConst("a", Expr(0x7F800000_a));
    auto* lhs = Equal("one", 1_a);
    auto* rhs = Equal(Bitcast(Source{{12, 34}}, ty.f32(), "a"), 0.0_a);
    auto* binary = LogicalAnd(lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: value inf cannot be represented as 'f32'");
}

TEST_F(ConstEvalTest, ShortCircuit_And_Error_Bitcast) {
    // const one = 1;
    // const a = 0x7F800000;
    // const result = (one == 0) && (bitcast<f32>(a) == 0i);
    GlobalConst("one", Expr(1_a));
    GlobalConst("a", Expr(0x7F800000_a));
    auto* lhs = Equal("one", 0_a);
    auto* rhs = Equal(Source{{12, 34}}, Bitcast<f32>("a"), 0_i);
    auto* binary = LogicalAnd(lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: no matching overload for 'operator == (f32, i32)'

2 candidate operators:
 • 'operator == (T  ✓ , T  ✗ ) -> bool' where:
      ✓  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'operator == (vecN<T>  ✗ , vecN<T>  ✗ ) -> vecN<bool>' where:
      ✗  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
)");
}

TEST_F(ConstEvalTest, ShortCircuit_Or_Invalid_Bitcast) {
    // const one = 1;
    // const a = 0x7F800000;
    // const result = (one == 1) || (bitcast<f32>(a) == 0.0);
    GlobalConst("one", Expr(1_a));
    GlobalConst("a", Expr(0x7F800000_a));
    auto* lhs = Equal("one", 1_a);
    auto* rhs = Equal(Bitcast<f32>("a"), 0.0_a);
    auto* binary = LogicalOr(lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    ValidateOr(Sem(), binary);
}

TEST_F(ConstEvalTest, NonShortCircuit_Or_Invalid_Bitcast) {
    // const one = 1;
    // const a = 0x7F800000;
    // const result = (one == 0) || (bitcast<f32>(a) == 0.0);
    GlobalConst("one", Expr(1_a));
    GlobalConst("a", Expr(0x7F800000_a));
    auto* lhs = Equal("one", 0_a);
    auto* rhs = Equal(Bitcast(Source{{12, 34}}, ty.f32(), "a"), 0.0_a);
    auto* binary = LogicalOr(lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: value inf cannot be represented as 'f32'");
}

TEST_F(ConstEvalTest, ShortCircuit_Or_Error_Bitcast) {
    // const one = 1;
    // const a = 0x7F800000;
    // const result = (one == 1) || (bitcast<f32>(a) == 0i);
    GlobalConst("one", Expr(1_a));
    GlobalConst("a", Expr(0x7F800000_a));
    auto* lhs = Equal("one", 1_a);
    auto* rhs = Equal(Source{{12, 34}}, Bitcast<f32>("a"), 0_i);
    auto* binary = LogicalOr(lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: no matching overload for 'operator == (f32, i32)'

2 candidate operators:
 • 'operator == (T  ✓ , T  ✗ ) -> bool' where:
      ✓  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'operator == (vecN<T>  ✗ , vecN<T>  ✗ ) -> vecN<bool>' where:
      ✗  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
)");
}

////////////////////////////////////////////////
// Short-Circuit value construction / conversion
////////////////////////////////////////////////

// NOTE: Cannot demonstrate short-circuiting an invalid init/convert as const eval of init/convert
// always succeeds.

TEST_F(ConstEvalTest, ShortCircuit_And_Error_Init) {
    // const one = 1;
    // const result = (one == 0) && (vec2<f32>(1.0, true).x == 0.0);
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 0_a);
    auto* rhs =
        Equal(MemberAccessor(Call<vec2<f32>>(Source{{12, 34}}, 1.0_a, Expr(true)), "x"), 0.0_a);
    GlobalConst("result", LogicalAnd(lhs, rhs));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: no matching constructor for 'vec2<f32>(abstract-float, bool)'

8 candidate constructors:
 • 'vec2<T  ✓ >(x: T  ✓ , y: T  ✗ ) -> vec2<T>' where:
      ✓  'T' is 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec2<T  ✓ >(T  ✓ ) -> vec2<T>' where:
      ✗  overload expects 1 argument, call passed 2 arguments
      ✓  'T' is 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec2<T  ✓ >(vec2<T>  ✗ ) -> vec2<T>' where:
      ✓  'T' is 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec2<T  ✓ >() -> vec2<T>' where:
      ✗  overload expects 0 arguments, call passed 2 arguments
      ✓  'T' is 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec2(x: T  ✓ , y: T  ✗ ) -> vec2<T>' where:
      ✓  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec2(T  ✓ ) -> vec2<T>' where:
      ✗  overload expects 1 argument, call passed 2 arguments
      ✗  overload expects 0 template arguments, call passed 1 argument
      ✓  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec2() -> vec2<abstract-int>' where:
      ✗  overload expects 0 arguments, call passed 2 arguments
      ✗  overload expects 0 template arguments, call passed 1 argument
 • 'vec2(vec2<T>  ✗ ) -> vec2<T>' where:
      ✗  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'

5 candidate conversions:
 • 'vec2<T  ✓ >(vec2<U>  ✗ ) -> vec2<T>' where:
      ✓  'T' is 'f32'
      ✗  'U' is 'abstract-int', 'abstract-float', 'i32', 'f16', 'u32' or 'bool'
 • 'vec2<T  ✗ >(vec2<U>  ✗ ) -> vec2<T>' where:
      ✗  'T' is 'f16'
      ✗  'U' is 'abstract-int', 'abstract-float', 'f32', 'i32', 'u32' or 'bool'
 • 'vec2<T  ✗ >(vec2<U>  ✗ ) -> vec2<T>' where:
      ✗  'T' is 'i32'
      ✗  'U' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'u32' or 'bool'
 • 'vec2<T  ✗ >(vec2<U>  ✗ ) -> vec2<T>' where:
      ✗  'T' is 'u32'
      ✗  'U' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32' or 'bool'
 • 'vec2<T  ✗ >(vec2<U>  ✗ ) -> vec2<T>' where:
      ✗  'T' is 'bool'
      ✗  'U' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32' or 'u32'
)");
}

TEST_F(ConstEvalTest, ShortCircuit_Or_Error_Init) {
    // const one = 1;
    // const result = (one == 1) || (vec2<f32>(1.0, true).x == 0.0);
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 0_a);
    auto* rhs =
        Equal(MemberAccessor(Call<vec2<f32>>(Source{{12, 34}}, 1.0_a, Expr(true)), "x"), 0.0_a);
    GlobalConst("result", LogicalOr(lhs, rhs));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: no matching constructor for 'vec2<f32>(abstract-float, bool)'

8 candidate constructors:
 • 'vec2<T  ✓ >(x: T  ✓ , y: T  ✗ ) -> vec2<T>' where:
      ✓  'T' is 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec2<T  ✓ >(T  ✓ ) -> vec2<T>' where:
      ✗  overload expects 1 argument, call passed 2 arguments
      ✓  'T' is 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec2<T  ✓ >(vec2<T>  ✗ ) -> vec2<T>' where:
      ✓  'T' is 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec2<T  ✓ >() -> vec2<T>' where:
      ✗  overload expects 0 arguments, call passed 2 arguments
      ✓  'T' is 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec2(x: T  ✓ , y: T  ✗ ) -> vec2<T>' where:
      ✓  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec2(T  ✓ ) -> vec2<T>' where:
      ✗  overload expects 1 argument, call passed 2 arguments
      ✗  overload expects 0 template arguments, call passed 1 argument
      ✓  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'vec2() -> vec2<abstract-int>' where:
      ✗  overload expects 0 arguments, call passed 2 arguments
      ✗  overload expects 0 template arguments, call passed 1 argument
 • 'vec2(vec2<T>  ✗ ) -> vec2<T>' where:
      ✗  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'

5 candidate conversions:
 • 'vec2<T  ✓ >(vec2<U>  ✗ ) -> vec2<T>' where:
      ✓  'T' is 'f32'
      ✗  'U' is 'abstract-int', 'abstract-float', 'i32', 'f16', 'u32' or 'bool'
 • 'vec2<T  ✗ >(vec2<U>  ✗ ) -> vec2<T>' where:
      ✗  'T' is 'f16'
      ✗  'U' is 'abstract-int', 'abstract-float', 'f32', 'i32', 'u32' or 'bool'
 • 'vec2<T  ✗ >(vec2<U>  ✗ ) -> vec2<T>' where:
      ✗  'T' is 'i32'
      ✗  'U' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'u32' or 'bool'
 • 'vec2<T  ✗ >(vec2<U>  ✗ ) -> vec2<T>' where:
      ✗  'T' is 'u32'
      ✗  'U' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32' or 'bool'
 • 'vec2<T  ✗ >(vec2<U>  ✗ ) -> vec2<T>' where:
      ✗  'T' is 'bool'
      ✗  'U' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32' or 'u32'
)");
}

////////////////////////////////////////////////
// Short-Circuit Array/Struct Init
////////////////////////////////////////////////

// NOTE: Cannot demonstrate short-circuiting an invalid array/struct init as const eval of
// array/struct init always succeeds.

TEST_F(ConstEvalTest, ShortCircuit_And_Error_StructInit) {
    // struct S {
    //     a : i32,
    //     b : f32,
    // }
    // const one = 1;
    // const result = (one == 0) && Foo(1, true).a == 0;
    Structure("S", Vector{Member("a", ty.i32()), Member("b", ty.f32())});
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 0_a);
    auto* rhs = Equal(MemberAccessor(Call("S", Expr(1_a), Expr(Source{{12, 34}}, true)), "a"), 0_a);
    GlobalConst("result", LogicalAnd(lhs, rhs));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: type in structure constructor does not match struct member type: "
              "expected 'f32', found 'bool'");
}

TEST_F(ConstEvalTest, ShortCircuit_Or_Error_StructInit) {
    // struct S {
    //     a : i32,
    //     b : f32,
    // }
    // const one = 1;
    // const result = (one == 1) || Foo(1, true).a == 0;
    Structure("S", Vector{Member("a", ty.i32()), Member("b", ty.f32())});
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 1_a);
    auto* rhs = Equal(MemberAccessor(Call("S", Expr(1_a), Expr(Source{{12, 34}}, true)), "a"), 0_a);
    GlobalConst("result", LogicalOr(lhs, rhs));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: type in structure constructor does not match struct member type: "
              "expected 'f32', found 'bool'");
}

TEST_F(ConstEvalTest, ShortCircuit_And_Error_ArrayInit) {
    // const one = 1;
    // const result = (one == 0) && array(4) == 0;
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 0_a);
    auto* rhs = Equal(Call("array", Expr(4_a)), 0_a);
    GlobalConst("result", LogicalAnd(lhs, rhs));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: no matching overload for 'operator == (array<abstract-int, 1>, abstract-int)'

2 candidate operators:
 • 'operator == (T  ✗ , T  ✗ ) -> bool' where:
      ✗  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'operator == (vecN<T>  ✗ , vecN<T>  ✗ ) -> vecN<bool>' where:
      ✗  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
)");
}

TEST_F(ConstEvalTest, ShortCircuit_Or_Error_ArrayInit) {
    // const one = 1;
    // const result = (one == 1) || array(4) == 0;
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 1_a);
    auto* rhs = Equal(Call("array", Expr(4_a)), 0_a);
    GlobalConst("result", LogicalOr(lhs, rhs));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: no matching overload for 'operator == (array<abstract-int, 1>, abstract-int)'

2 candidate operators:
 • 'operator == (T  ✗ , T  ✗ ) -> bool' where:
      ✗  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'operator == (vecN<T>  ✗ , vecN<T>  ✗ ) -> vecN<bool>' where:
      ✗  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
)");
}

////////////////////////////////////////////////
// Short-Circuit Builtin Call
////////////////////////////////////////////////

TEST_F(ConstEvalTest, ShortCircuit_And_Invalid_BuiltinCall) {
    // const one = 1;
    // return (one == 0) && (extractBits(1, 0, 99) == 0);
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 0_a);
    auto* rhs = Equal(Call("extractBits", 1_a, 0_a, 99_a), 0_a);
    auto* binary = LogicalAnd(lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    ValidateAnd(Sem(), binary);
}

TEST_F(ConstEvalTest, NonShortCircuit_And_Invalid_BuiltinCall) {
    // const one = 1;
    // return (one == 1) && (extractBits(1, 0, 99) == 0);
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 1_a);
    auto* rhs = Equal(Call(Source{{12, 34}}, "extractBits", 1_a, 0_a, 99_a), 0_a);
    auto* binary = LogicalAnd(lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: 'offset' + 'count' must be less than or equal to the bit width of 'e'");
}

TEST_F(ConstEvalTest, ShortCircuit_And_Error_BuiltinCall) {
    // const one = 1;
    // return (one == 0) && (extractBits(1, 0, 99) == 0.0);
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 0_a);
    auto* rhs = Equal(Source{{12, 34}}, Call("extractBits", 1_a, 0_a, 99_a), 0.0_a);
    auto* binary = LogicalAnd(lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: no matching overload for 'operator == (i32, abstract-float)'

2 candidate operators:
 • 'operator == (T  ✓ , T  ✗ ) -> bool' where:
      ✓  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'operator == (vecN<T>  ✗ , vecN<T>  ✗ ) -> vecN<bool>' where:
      ✗  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
)");
}

TEST_F(ConstEvalTest, ShortCircuit_Or_Invalid_BuiltinCall) {
    // const one = 1;
    // return (one == 1) || (extractBits(1, 0, 99) == 0);
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 1_a);
    auto* rhs = Equal(Call("extractBits", 1_a, 0_a, 99_a), 0_a);
    auto* binary = LogicalOr(lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    ValidateOr(Sem(), binary);
}

TEST_F(ConstEvalTest, NonShortCircuit_Or_Invalid_BuiltinCall) {
    // const one = 1;
    // return (one == 0) || (extractBits(1, 0, 99) == 0);
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 0_a);
    auto* rhs = Equal(Call(Source{{12, 34}}, "extractBits", 1_a, 0_a, 99_a), 0_a);
    auto* binary = LogicalOr(lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: 'offset' + 'count' must be less than or equal to the bit width of 'e'");
}

TEST_F(ConstEvalTest, ShortCircuit_Or_Error_BuiltinCall) {
    // const one = 1;
    // return (one == 1) || (extractBits(1, 0, 99) == 0.0);
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 1_a);
    auto* rhs = Equal(Source{{12, 34}}, Call("extractBits", 1_a, 0_a, 99_a), 0.0_a);
    auto* binary = LogicalOr(lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: no matching overload for 'operator == (i32, abstract-float)'

2 candidate operators:
 • 'operator == (T  ✓ , T  ✗ ) -> bool' where:
      ✓  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
 • 'operator == (vecN<T>  ✗ , vecN<T>  ✗ ) -> vecN<bool>' where:
      ✗  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'i32', 'u32' or 'bool'
)");
}

////////////////////////////////////////////////
// Short-Circuit Literal
////////////////////////////////////////////////

// NOTE: Cannot demonstrate short-circuiting an invalid literal as const eval of a literal does not
// fail.

#if TINT_BUILD_WGSL_READER
TEST_F(ConstEvalTest, ShortCircuit_And_Error_Literal) {
    // NOTE: This fails parsing rather than resolving, which is why we can't use the ProgramBuilder
    // for this test.
    auto src = R"(
const one = 1;
const result = (one == 0) && (1111111111111111111111111111111i == 0);
)";

    auto file = std::make_unique<Source::File>("test", src);
    auto program = wgsl::reader::Parse(file.get());
    EXPECT_FALSE(program.IsValid());

    auto error = program.Diagnostics().Str();
    EXPECT_EQ(error, R"(test:3:31 error: value cannot be represented as 'i32'
const result = (one == 0) && (1111111111111111111111111111111i == 0);
                              ^
)");
}

TEST_F(ConstEvalTest, ShortCircuit_Or_Error_Literal) {
    // NOTE: This fails parsing rather than resolving, which is why we can't use the ProgramBuilder
    // for this test.
    auto src = R"(
const one = 1;
const result = (one == 1) || (1111111111111111111111111111111i == 0);
)";

    auto file = std::make_unique<Source::File>("test", src);
    auto program = wgsl::reader::Parse(file.get());
    EXPECT_FALSE(program.IsValid());

    auto error = program.Diagnostics().Str();
    EXPECT_EQ(error, R"(test:3:31 error: value cannot be represented as 'i32'
const result = (one == 1) || (1111111111111111111111111111111i == 0);
                              ^
)");
}
#endif  // TINT_BUILD_WGSL_READER

////////////////////////////////////////////////
// Short-Circuit Member Access
////////////////////////////////////////////////

// NOTE: Cannot demonstrate short-circuiting an invalid member access as const eval of member access
// always succeeds.

TEST_F(ConstEvalTest, ShortCircuit_And_Error_MemberAccess) {
    // struct S {
    //     a : i32,
    //     b : f32,
    // }
    // const s = S(1, 2.0);
    // const one = 1;
    // const result = (one == 0) && (s.c == 0);
    Structure("S", Vector{Member("a", ty.i32()), Member("b", ty.f32())});
    GlobalConst("s", Call("S", Expr(1_a), Expr(2.0_a)));
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 0_a);
    auto* rhs = Equal(MemberAccessor(Source{{12, 34}}, "s", "c"), 0_a);
    GlobalConst("result", LogicalAnd(lhs, rhs));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: struct member c not found");
}

TEST_F(ConstEvalTest, ShortCircuit_Or_Error_MemberAccess) {
    // struct S {
    //     a : i32,
    //     b : f32,
    // }
    // const s = S(1, 2.0);
    // const one = 1;
    // const result = (one == 1) || (s.c == 0);
    Structure("S", Vector{Member("a", ty.i32()), Member("b", ty.f32())});
    GlobalConst("s", Call("S", Expr(1_a), Expr(2.0_a)));
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 1_a);
    auto* rhs = Equal(MemberAccessor(Source{{12, 34}}, "s", "c"), 0_a);
    GlobalConst("result", LogicalOr(lhs, rhs));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: struct member c not found");
}

////////////////////////////////////////////////
// Short-Circuit with RHS Variable Access
////////////////////////////////////////////////

TEST_F(ConstEvalTest, ShortCircuit_And_RHSConstDecl) {
    // const FALSE = false;
    // const result = FALSE && FALSE;
    GlobalConst("FALSE", Expr(false));
    auto* binary = LogicalAnd("FALSE", "FALSE");
    GlobalConst("result", binary);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    ValidateAnd(Sem(), binary);
}

TEST_F(ConstEvalTest, ShortCircuit_Or_RHSConstDecl) {
    // const TRUE = true;
    // const result = TRUE || TRUE;
    GlobalConst("TRUE", Expr(true));
    auto* binary = LogicalOr("TRUE", "TRUE");
    GlobalConst("result", binary);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    ValidateOr(Sem(), binary);
}

TEST_F(ConstEvalTest, ShortCircuit_And_RHSLetDecl) {
    // fn f() {
    //   let b = false;
    //   let result = false && b;
    // }
    auto* binary = LogicalAnd(false, "b");
    WrapInFunction(Decl(Let("b", Expr(false))), binary);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    ValidateAnd(Sem(), binary);
}

TEST_F(ConstEvalTest, ShortCircuit_Or_RHSLetDecl) {
    // fn f() {
    //   let b = false;
    //   let result = true || b;
    // }
    auto* binary = LogicalOr(true, "b");
    WrapInFunction(Decl(Let("b", Expr(false))), binary);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    ValidateOr(Sem(), binary);
}

TEST_F(ConstEvalTest, ShortCircuit_And_RHSVarDecl) {
    // fn f() {
    //   var b = false;
    //   let result = false && b;
    // }
    auto* binary = LogicalAnd(false, "b");
    WrapInFunction(Decl(Var("b", Expr(false))), binary);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    ASSERT_EQ(Sem().Get(binary)->Stage(), core::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(binary)->ConstantValue()->ValueAs<bool>(), false);
    EXPECT_EQ(Sem().GetVal(binary->lhs)->Stage(), core::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().GetVal(binary->rhs)->Stage(), core::EvaluationStage::kNotEvaluated);
}

TEST_F(ConstEvalTest, ShortCircuit_Or_RHSVarDecl) {
    // fn f() {
    //   var b = false;
    //   let result = true || b;
    // }
    auto* binary = LogicalOr(true, "b");
    WrapInFunction(Decl(Var("b", Expr(false))), binary);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    ASSERT_EQ(Sem().Get(binary)->Stage(), core::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(binary)->ConstantValue()->ValueAs<bool>(), true);
    EXPECT_EQ(Sem().GetVal(binary->lhs)->Stage(), core::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().GetVal(binary->rhs)->Stage(), core::EvaluationStage::kNotEvaluated);
}

TEST_F(ConstEvalTest, ShortCircuit_And_RHSRuntimeBuiltin) {
    // fn f() {
    //   var b = false;
    //   let result = false && any(b);
    // }
    auto* binary = LogicalAnd(false, Call(wgsl::BuiltinFn::kAny, "b"));
    WrapInFunction(Decl(Var("b", Expr(false))), binary);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    ASSERT_EQ(Sem().Get(binary)->Stage(), core::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(binary)->ConstantValue()->ValueAs<bool>(), false);
    EXPECT_EQ(Sem().GetVal(binary->lhs)->Stage(), core::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().GetVal(binary->rhs)->Stage(), core::EvaluationStage::kNotEvaluated);
}

TEST_F(ConstEvalTest, ShortCircuit_Or_RHSRuntimeBuiltin) {
    // fn f() {
    //   var b = false;
    //   let result = true || any(b);
    // }
    auto* binary = LogicalOr(true, Call(wgsl::BuiltinFn::kAny, "b"));
    WrapInFunction(Decl(Var("b", Expr(false))), binary);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    ASSERT_EQ(Sem().Get(binary)->Stage(), core::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(binary)->ConstantValue()->ValueAs<bool>(), true);
    EXPECT_EQ(Sem().GetVal(binary->lhs)->Stage(), core::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().GetVal(binary->rhs)->Stage(), core::EvaluationStage::kNotEvaluated);
}

////////////////////////////////////////////////
// Short-Circuit Swizzle
////////////////////////////////////////////////

// NOTE: Cannot demonstrate short-circuiting an invalid swizzle as const eval of swizzle always
// succeeds.

TEST_F(ConstEvalTest, ShortCircuit_And_Error_Swizzle) {
    // const one = 1;
    // const result = (one == 0) && (vec2(1, 2).z == 0);
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 0_a);
    auto* rhs =
        Equal(MemberAccessor(Call<vec2<Infer>>(1_a, 2_a), Ident(Source{{12, 34}}, "z")), 0_a);
    GlobalConst("result", LogicalAnd(lhs, rhs));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: invalid vector swizzle member");
}

TEST_F(ConstEvalTest, ShortCircuit_Or_Error_Swizzle) {
    // const one = 1;
    // const result = (one == 1) || (vec2(1, 2).z == 0);
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 1_a);
    auto* rhs =
        Equal(MemberAccessor(Call<vec2<Infer>>(1_a, 2_a), Ident(Source{{12, 34}}, "z")), 0_a);
    GlobalConst("result", LogicalOr(lhs, rhs));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: invalid vector swizzle member");
}

////////////////////////////////////////////////
// Short-Circuit Mixed Constant and Runtime
////////////////////////////////////////////////

TEST_F(ConstEvalTest, ShortCircuit_And_MixedConstantAndRuntime) {
    // var j : i32;
    // let result = false && j < (0 - 8);
    auto* j = Decl(Var("j", ty.i32()));
    auto* binary = LogicalAnd(Expr(false), LessThan("j", Sub(0_a, 8_a)));
    auto* result = Let("result", binary);
    WrapInFunction(j, result);
    EXPECT_TRUE(r()->Resolve()) << r()->error();
    ValidateAnd(Sem(), binary);
}

TEST_F(ConstEvalTest, ShortCircuit_Or_MixedConstantAndRuntime) {
    // var j : i32;
    // let result = true || j < (0 - 8);
    auto* j = Decl(Var("j", ty.i32()));
    auto* binary = LogicalOr(Expr(true), LessThan("j", Sub(0_a, 8_a)));
    auto* result = Let("result", binary);
    WrapInFunction(j, result);
    EXPECT_TRUE(r()->Resolve()) << r()->error();
    ValidateOr(Sem(), binary);
}

////////////////////////////////////////////////
// Short-Circuit templated identifier arguments
////////////////////////////////////////////////

TEST_F(ConstEvalTest, ShortCircuit_And_ArrayElementCountTooSmall) {
    // const one = 1;
    // const result = (one == 0) && array<bool, 3-4>()[0];
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 0_a);
    auto* count = Sub(3_a, 4_a);
    auto* rhs = IndexAccessor(Call(ty.array(ty.bool_(), count)), 0_a);
    auto* binary = LogicalAnd(lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "error: array count (-1) must be greater than 0");
}

TEST_F(ConstEvalTest, ShortCircuit_Or_ArrayElementCountTooSmall) {
    // const one = 1;
    // const result = (one == 1) || array<bool, 3-4>()[0];
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 1_a);
    auto* count = Sub(3_a, 4_a);
    auto* rhs = IndexAccessor(Call(ty.array(ty.bool_(), count)), 0_a);
    auto* binary = LogicalOr(lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "error: array count (-1) must be greater than 0");
}

TEST_F(ConstEvalTest, ShortCircuit_And_InvalidArrayElementCount) {
    // const one = 1;
    // const result = (one == 0) && array<bool, u32(sqrt(-1))>()[0];
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 0_a);
    auto* count = Call("u32", Call("sqrt", -1_a));
    auto* rhs = IndexAccessor(Call(ty.array(ty.bool_(), count)), 0_a);
    auto* binary = LogicalAnd(lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "error: sqrt must be called with a value >= 0");
}

TEST_F(ConstEvalTest, ShortCircuit_Or_InvalidArrayElementCount) {
    // const one = 1;
    // const result = (one == 1) || array<bool, u32(sqrt(-1))>()[0];
    GlobalConst("one", Expr(1_a));
    auto* lhs = Equal("one", 1_a);
    auto* count = Call("u32", Call("sqrt", -1_a));
    auto* rhs = IndexAccessor(Call(ty.array(ty.bool_(), count)), 0_a);
    auto* binary = LogicalOr(lhs, rhs);
    GlobalConst("result", binary);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "error: sqrt must be called with a value >= 0");
}

////////////////////////////////////////////////
// Short-Circuit Nested
////////////////////////////////////////////////

#if TINT_BUILD_WGSL_READER
using ConstEvalTestShortCircuit = ConstEvalTestWithParam<std::tuple<const char*, bool>>;
TEST_P(ConstEvalTestShortCircuit, Test) {
    const char* expr = std::get<0>(GetParam());
    bool should_pass = std::get<1>(GetParam());

    auto src = std::string(R"(
const one = 1;
const result = )");
    src = src + expr + ";";
    auto file = std::make_unique<Source::File>("test", src);
    auto program = wgsl::reader::Parse(file.get());

    if (should_pass) {
        auto error = program.Diagnostics().Str();

        EXPECT_TRUE(program.IsValid()) << error;
    } else {
        EXPECT_FALSE(program.IsValid());
    }
}
INSTANTIATE_TEST_SUITE_P(Nested,
                         ConstEvalTestShortCircuit,
                         testing::ValuesIn(std::vector<std::tuple<const char*, bool>>{
                             // AND nested rhs
                             {"(one == 0) && ((one == 0) && ((2/0)==0))", true},
                             {"(one == 1) && ((one == 0) && ((2/0)==0))", true},
                             {"(one == 0) && ((one == 1) && ((2/0)==0))", true},
                             {"(one == 1) && ((one == 1) && ((2/0)==0))", false},
                             // AND nested lhs
                             {"((one == 0) && ((2/0)==0)) && (one == 0)", true},
                             {"((one == 0) && ((2/0)==0)) && (one == 1)", true},
                             {"((one == 1) && ((2/0)==0)) && (one == 0)", false},
                             {"((one == 1) && ((2/0)==0)) && (one == 1)", false},
                             // OR nested rhs
                             {"(one == 1) || ((one == 1) || ((2/0)==0))", true},
                             {"(one == 0) || ((one == 1) || ((2/0)==0))", true},
                             {"(one == 1) || ((one == 0) || ((2/0)==0))", true},
                             {"(one == 0) || ((one == 0) || ((2/0)==0))", false},
                             // OR nested lhs
                             {"((one == 1) || ((2/0)==0)) || (one == 1)", true},
                             {"((one == 1) || ((2/0)==0)) || (one == 0)", true},
                             {"((one == 0) || ((2/0)==0)) || (one == 1)", false},
                             {"((one == 0) || ((2/0)==0)) || (one == 0)", false},
                             // AND nested both sides
                             {"((one == 0) && ((2/0)==0)) && ((one == 0) && ((2/0)==0))", true},
                             {"((one == 0) && ((2/0)==0)) && ((one == 1) && ((2/0)==0))", true},
                             {"((one == 1) && ((2/0)==0)) && ((one == 0) && ((2/0)==0))", false},
                             {"((one == 1) && ((2/0)==0)) && ((one == 1) && ((2/0)==0))", false},
                             // OR nested both sides
                             {"((one == 1) || ((2/0)==0)) && ((one == 1) || ((2/0)==0))", true},
                             {"((one == 1) || ((2/0)==0)) && ((one == 0) || ((2/0)==0))", false},
                             {"((one == 0) || ((2/0)==0)) && ((one == 1) || ((2/0)==0))", false},
                             {"((one == 0) || ((2/0)==0)) && ((one == 0) || ((2/0)==0))", false},
                             // AND chained
                             {"(one == 0) && (one == 0) && ((2 / 0) == 0)", true},
                             {"(one == 1) && (one == 0) && ((2 / 0) == 0)", true},
                             {"(one == 0) && (one == 1) && ((2 / 0) == 0)", true},
                             {"(one == 1) && (one == 1) && ((2 / 0) == 0)", false},
                             // OR chained
                             {"(one == 1) || (one == 1) || ((2 / 0) == 0)", true},
                             {"(one == 0) || (one == 1) || ((2 / 0) == 0)", true},
                             {"(one == 1) || (one == 0) || ((2 / 0) == 0)", true},
                             {"(one == 0) || (one == 0) || ((2 / 0) == 0)", false},
                         }));
#endif  // TINT_BUILD_WGSL_READER

}  // namespace LogicalShortCircuit

}  // namespace
}  // namespace tint::core::constant::test

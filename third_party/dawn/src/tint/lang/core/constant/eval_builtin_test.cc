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

#include "src/tint/lang/core/constant/eval_test.h"

#include "src/tint/utils/result/result.h"

using namespace tint::core::number_suffixes;  // NOLINT
using ::testing::HasSubstr;

namespace tint::core::constant::test {
namespace {

struct Case {
    Case(VectorRef<Value> in_args, VectorRef<Value> expected_values)
        : args(std::move(in_args)),
          expected(Success{std::move(expected_values), CheckConstantFlags{}}) {}

    Case(VectorRef<Value> in_args, std::string expected_err)
        : args(std::move(in_args)), expected(Failure{std::move(expected_err)}) {}

    /// Expected value may be positive or negative
    Case& PosOrNeg() {
        Success s = expected.Get();
        s.flags.pos_or_neg = true;
        expected = s;
        return *this;
    }

    /// Expected value should be compared using EXPECT_FLOAT_EQ instead of EXPECT_EQ.
    /// If optional epsilon is passed in, will be compared using EXPECT_NEAR with that epsilon.
    Case& FloatComp(std::optional<double> epsilon = {}) {
        Success s = expected.Get();
        s.flags.float_compare = true;
        s.flags.float_compare_epsilon = epsilon;
        expected = s;
        return *this;
    }

    struct Success {
        Vector<Value, 2> values;
        CheckConstantFlags flags;
    };
    struct Failure {
        std::string error;
    };

    Vector<Value, 8> args;
    tint::Result<Success, Failure> expected;
};

static std::ostream& operator<<(std::ostream& o, const Case& c) {
    o << "args: ";
    for (auto& a : c.args) {
        o << a << ", ";
    }
    o << "expected: ";
    if (c.expected == Success) {
        auto s = c.expected.Get();
        if (s.values.Length() == 1) {
            o << s.values[0];
        } else {
            o << "[";
            for (auto& v : s.values) {
                if (&v != &s.values[0]) {
                    o << ", ";
                }
                o << v;
            }
            o << "]";
        }
        o << ", pos_or_neg: " << s.flags.pos_or_neg;
        o << ", float_compare: " << s.flags.float_compare;
    } else {
        o << "[ERROR: " << c.expected.Failure().error << "]";
    }
    return o;
}

using ScalarTypes = std::variant<AInt, AFloat, u32, i32, f32, f16>;

/// Creates a Case with Values for args and result
static Case C(std::initializer_list<Value> args, Value result) {
    return Case{Vector<Value, 8>{args}, Vector<Value, 2>{std::move(result)}};
}

/// Creates a Case with Values for args and result
static Case C(std::initializer_list<Value> args, std::initializer_list<Value> results) {
    return Case{Vector<Value, 8>{args}, Vector<Value, 2>{results}};
}

/// Convenience overload that creates a Case with just scalars
static Case C(std::initializer_list<ScalarTypes> sargs, ScalarTypes sresult) {
    Vector<Value, 8> args;
    for (auto& sa : sargs) {
        std::visit([&](auto&& v) { return args.Push(Val(v)); }, sa);
    }
    Value result = Val(0_a);
    std::visit([&](auto&& v) { result = Val(v); }, sresult);
    return Case{std::move(args), Vector<Value, 2>{std::move(result)}};
}

/// Creates a Case with Values for args and result
static Case C(std::initializer_list<ScalarTypes> sargs,
              std::initializer_list<ScalarTypes> sresults) {
    Vector<Value, 8> args;
    for (auto& sa : sargs) {
        std::visit([&](auto&& v) { return args.Push(Val(v)); }, sa);
    }
    Vector<Value, 2> results;
    for (auto& sa : sresults) {
        std::visit([&](auto&& v) { return results.Push(Val(v)); }, sa);
    }
    return Case{std::move(args), std::move(results)};
}

/// Creates a Case with Values for args and expected error
static Case E(std::initializer_list<Value> args, std::string err) {
    return Case{Vector<Value, 8>{args}, std::move(err)};
}

/// Convenience overload that creates an expected-error Case with just scalars
static Case E(std::initializer_list<ScalarTypes> sargs, std::string err) {
    Vector<Value, 8> args;
    for (auto& sa : sargs) {
        std::visit([&](auto&& v) { return args.Push(Val(v)); }, sa);
    }
    return Case{std::move(args), std::move(err)};
}

using ConstEvalBuiltinTest = ConstEvalTestWithParam<std::tuple<core::BuiltinFn, Case>>;

TEST_P(ConstEvalBuiltinTest, Test) {
    Enable(wgsl::Extension::kF16);

    auto builtin = std::get<0>(GetParam());
    auto& c = std::get<1>(GetParam());

    Vector<const ast::Expression*, 8> args;
    for (auto& a : c.args) {
        args.Push(a.Expr(*this));
    }

    auto* expr = Call(Source{{12, 34}}, core::str(builtin), std::move(args));
    GlobalConst("C", expr);

    if (c.expected == Success) {
        auto expected_case = c.expected.Get();

        ASSERT_TRUE(r()->Resolve()) << r()->error();

        auto* sem = Sem().Get(expr);
        ASSERT_NE(sem, nullptr);
        const constant::Value* value = sem->ConstantValue();
        ASSERT_NE(value, nullptr);
        EXPECT_TYPE(value->Type(), sem->Type());

        if (value->Type()->Is<core::type::Struct>()) {
            // The result type of the constant-evaluated expression is a structure.
            // Compare each of the fields individually.
            for (size_t i = 0; i < expected_case.values.Length(); i++) {
                CheckConstant(value->Index(i), expected_case.values[i], expected_case.flags);
            }
        } else {
            // Return type is not a structure. Just compare the single value
            ASSERT_EQ(expected_case.values.Length(), 1u)
                << "const-eval returned non-struct, but Case expected multiple values";
            CheckConstant(value, expected_case.values[0], expected_case.flags);
        }
    } else {
        ASSERT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(), c.expected.Failure().error);
    }
}

INSTANTIATE_TEST_SUITE_P(MixedAbstractArgs_Atan2,
                         ConstEvalBuiltinTest,
                         testing::Combine(testing::Values(core::BuiltinFn::kAtan2),
                                          testing::ValuesIn(std::vector{
                                              C({0.0_a, 1_a}, AFloat{0}),
                                              C({0_a, 1.0_a}, AFloat{0}),
                                              C({1_a, 0.0_a}, kPiOver2<AFloat>),
                                              C({1.0_a, 0_a}, kPiOver2<AFloat>),
                                          })));

INSTANTIATE_TEST_SUITE_P(MixedAbstractArgs_Max,
                         ConstEvalBuiltinTest,
                         testing::Combine(testing::Values(core::BuiltinFn::kMax),
                                          testing::ValuesIn(std::vector{
                                              // AbstractInt first, AbstractFloat second
                                              C({1_a, 2.0_a}, AFloat{2}),
                                              C({-1_a, -2.0_a}, AFloat{-1}),
                                              C({2_a, 0.0_a}, AFloat{2}),
                                              C({-2_a, 0.0_a}, AFloat{0}),
                                              C({0_a, 0.0_a}, AFloat{0}),
                                              C({0_a, -0.0_a}, AFloat{0}),
                                              C({-0_a, 0.0_a}, AFloat{0}),
                                              C({-0_a, -0.0_a}, AFloat{0}),
                                              // AbstractFloat first, AbstractInt second
                                              C({1.0_a, 2_a}, AFloat{2}),
                                              C({-1.0_a, -2_a}, AFloat{-1}),
                                              C({2.0_a, 0_a}, AFloat{2}),
                                              C({-2.0_a, 0_a}, AFloat{0}),
                                              C({0.0_a, 0_a}, AFloat{0}),
                                              C({0.0_a, -0_a}, AFloat{0}),
                                              C({-0.0_a, 0_a}, AFloat{0}),
                                              C({-0.0_a, -0_a}, AFloat{0}),
                                          })));

template <typename T>
std::vector<Case> AbsCases() {
    std::vector<Case> cases = {
        C({T(0)}, T(0)),
        C({T(2.0)}, T(2.0)),
        C({T::Highest()}, T::Highest()),

        // Vector tests
        C({Vec(T(2.0), T::Highest())}, Vec(T(2.0), T::Highest())),
    };
    ConcatIntoIf<IsSignedIntegral<T>>(
        cases,
        std::vector<Case>{
            C({Negate(T(0))}, T(0)),
            C({Negate(T(2.0))}, T(2.0)),
            // If e is signed and is the largest negative, the result is e
            C({T::Lowest()}, T::Lowest()),

            // 1 more then min i32
            C({Negate(T(2147483647))}, T(2147483647)),

            C({Vec(T(0), Negate(T(0)))}, Vec(T(0), T(0))),
            C({Vec(Negate(T(2.0)), T(2.0), T::Highest())}, Vec(T(2.0), T(2.0), T::Highest())),
        });
    return cases;
}
INSTANTIATE_TEST_SUITE_P(  //
    Abs,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kAbs),
                     testing::ValuesIn(Concat(AbsCases<AInt>(),  //
                                              AbsCases<i32>(),
                                              AbsCases<u32>(),
                                              AbsCases<AFloat>(),
                                              AbsCases<f32>(),
                                              AbsCases<f16>()))));

static std::vector<Case> AllCases() {
    return {
        C({Val(true)}, Val(true)),
        C({Val(false)}, Val(false)),

        C({Vec(true, true)}, Val(true)),
        C({Vec(true, false)}, Val(false)),
        C({Vec(false, true)}, Val(false)),
        C({Vec(false, false)}, Val(false)),

        C({Vec(true, true, true)}, Val(true)),
        C({Vec(false, true, true)}, Val(false)),
        C({Vec(true, false, true)}, Val(false)),
        C({Vec(true, true, false)}, Val(false)),
        C({Vec(false, false, false)}, Val(false)),

        C({Vec(true, true, true, true)}, Val(true)),
        C({Vec(false, true, true, true)}, Val(false)),
        C({Vec(true, false, true, true)}, Val(false)),
        C({Vec(true, true, false, true)}, Val(false)),
        C({Vec(true, true, true, false)}, Val(false)),
        C({Vec(false, false, false, false)}, Val(false)),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    All,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kAll), testing::ValuesIn(AllCases())));

static std::vector<Case> AnyCases() {
    return {
        C({Val(true)}, Val(true)),
        C({Val(false)}, Val(false)),

        C({Vec(true, true)}, Val(true)),
        C({Vec(true, false)}, Val(true)),
        C({Vec(false, true)}, Val(true)),
        C({Vec(false, false)}, Val(false)),

        C({Vec(true, true, true)}, Val(true)),
        C({Vec(false, true, true)}, Val(true)),
        C({Vec(true, false, true)}, Val(true)),
        C({Vec(true, true, false)}, Val(true)),
        C({Vec(false, false, false)}, Val(false)),

        C({Vec(true, true, true, true)}, Val(true)),
        C({Vec(false, true, true, true)}, Val(true)),
        C({Vec(true, false, true, true)}, Val(true)),
        C({Vec(true, true, false, true)}, Val(true)),
        C({Vec(true, true, true, false)}, Val(true)),
        C({Vec(false, false, false, false)}, Val(false)),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Any,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kAny), testing::ValuesIn(AnyCases())));

template <typename T>
std::vector<Case> Atan2Cases() {
    return {
        // Beware that y is first:  atan2(y,x)
        //
        // Beware of regions where the error is unbounded:
        //  At the orgin (0,0)
        //  When x has very small magnitude, since atan(y/x) is a
        //    valid formulation, and floating point division can have
        //    large error when the divisor is subnormal.
        //  When y is subnormal or infinite.

        // If x is +/-0 and y is negative, -PI/2 is returned
        C({-T(1.0), T(0.0)}, -kPiOver2<T>).FloatComp(),  //
        C({-T(1.0), -T(0.0)}, -kPiOver2<T>).FloatComp(),

        // If x is +/-0 and y is positive, +PI/2 is returned
        C({T(1.0), T(0.0)}, kPiOver2<T>).FloatComp(),  //
        C({T(1.0), -T(0.0)}, kPiOver2<T>).FloatComp(),

        // If x is positive and y is 0, 0 is returned
        C({T(0.0), T(1.0)}, T(0.0)).FloatComp(),  //
        C({-T(0.0), T(1.0)}, T(0.0)).FloatComp(),

        // Vector tests
        C({Vec(-T(1.0), T(1.0)), Vec(T(0.0), -T(0.0))}, Vec(-kPiOver2<T>, kPiOver2<T>)).FloatComp(),
        C({Vec(T(1.0), -T(1.0)), Vec(-T(0.0), T(0.0))}, Vec(kPiOver2<T>, -kPiOver2<T>)).FloatComp(),
        C({Vec(T(0.0), -T(0.0)), Vec(T(1.0), T(1.0))}, Vec(T(0.0), T(0.0))).FloatComp(),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Atan2,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kAtan2),
                     testing::ValuesIn(Concat(Atan2Cases<AFloat>(),  //
                                              Atan2Cases<f32>(),
                                              Atan2Cases<f16>()))));

template <typename T>
std::vector<Case> AtanCases() {
    return {
        C({T(1.0)}, kPiOver4<T>).FloatComp(),
        C({-T(1.0)}, -kPiOver4<T>).FloatComp(),

        // If i is +/-0, +/-0 is returned
        C({T(0.0)}, T(0.0)).PosOrNeg(),

        // Vector tests
        C({Vec(T(0.0), T(1.0), -T(1.0))}, Vec(T(0.0), kPiOver4<T>, -kPiOver4<T>)).FloatComp(),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Atan,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kAtan),
                     testing::ValuesIn(Concat(AtanCases<AFloat>(),  //
                                              AtanCases<f32>(),
                                              AtanCases<f16>()))));

template <typename T>
std::vector<Case> AtanhCases() {
    return {
        // If i is +/-0, +/-0 is returned
        C({T(0.0)}, T(0.0)).PosOrNeg(),

        C({T(0.9)}, T(1.4722193)).FloatComp(),

        // Vector tests
        C({Vec(T(0.0), T(0.9), -T(0.9))}, Vec(T(0.0), T(1.4722193), -T(1.4722193))).FloatComp(),

        E({T(1.1)},
          "12:34 error: atanh must be called with a value in the range (-1 .. 1) (exclusive)"),
        E({-T(1.1)},
          "12:34 error: atanh must be called with a value in the range (-1 .. 1) (exclusive)"),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Atanh,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kAtanh),
                     testing::ValuesIn(Concat(AtanhCases<AFloat>(),  //
                                              AtanhCases<f32>(),
                                              AtanhCases<f16>()))));

template <typename T>
std::vector<Case> AcosCases() {
    return {
        // If i is +/-0, +/-0 is returned
        C({T(0.87758256189)}, T(0.5)).FloatComp(),

        C({T(1.0)}, T(0.0)),
        C({-T(1.0)}, kPi<T>).FloatComp(),

        // Vector tests
        C({Vec(T(1.0), -T(1.0))}, Vec(T(0), kPi<T>)).FloatComp(),

        E({T(1.1)},
          "12:34 error: acos must be called with a value in the range [-1 .. 1] (inclusive)"),
        E({-T(1.1)},
          "12:34 error: acos must be called with a value in the range [-1 .. 1] (inclusive)"),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Acos,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kAcos),
                     testing::ValuesIn(Concat(AcosCases<AFloat>(),  //
                                              AcosCases<f32>(),
                                              AcosCases<f16>()))));

template <typename T>
std::vector<Case> AcoshCases() {
    return {
        C({T(1.0)}, T(0.0)),
        C({T(11.5919532755)}, kPi<T>).FloatComp(),

        // Vector tests
        C({Vec(T(1.0), T(11.5919532755))}, Vec(T(0), kPi<T>)).FloatComp(),

        E({T::Smallest()}, "12:34 error: acosh must be called with a value >= 1.0"),
        E({-T(1.1)}, "12:34 error: acosh must be called with a value >= 1.0"),
        E({T(0)}, "12:34 error: acosh must be called with a value >= 1.0"),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Acosh,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kAcosh),
                     testing::ValuesIn(Concat(AcoshCases<AFloat>(),  //
                                              AcoshCases<f32>(),
                                              AcoshCases<f16>()))));

template <typename T>
std::vector<Case> AsinCases() {
    return {
        // If i is +/-0, +/-0 is returned
        C({T(0.0)}, T(0.0)),
        C({-T(0.0)}, -T(0.0)),

        C({T(1.0)}, kPiOver2<T>).FloatComp(),
        C({-T(1.0)}, -kPiOver2<T>).FloatComp(),

        // Vector tests
        C({Vec(T(0.0), T(1.0), -T(1.0))}, Vec(T(0.0), kPiOver2<T>, -kPiOver2<T>)).FloatComp(),

        E({T(1.1)},
          "12:34 error: asin must be called with a value in the range [-1 .. 1] (inclusive)"),
        E({-T(1.1)},
          "12:34 error: asin must be called with a value in the range [-1 .. 1] (inclusive)"),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Asin,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kAsin),
                     testing::ValuesIn(Concat(AsinCases<AFloat>(),  //
                                              AsinCases<f32>(),
                                              AsinCases<f16>()))));

template <typename T>
std::vector<Case> AsinhCases() {
    return {
        // If i is +/-0, +/-0 is returned
        C({T(0.0)}, T(0.0)),
        C({-T(0.0)}, -T(0.0)),

        C({T(0.9)}, T(0.80886693565278)).FloatComp(),
        C({-T(2.0)}, -T(1.4436354751788)).FloatComp(),

        // Vector tests
        C({Vec(T(0.0), T(0.9), -T(2.0))},  //
          Vec(T(0.0), T(0.8088669356278), -T(1.4436354751788)))
            .FloatComp(),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Asinh,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kAsinh),
                     testing::ValuesIn(Concat(AsinhCases<AFloat>(),  //
                                              AsinhCases<f32>(),
                                              AsinhCases<f16>()))));

template <typename T>
std::vector<Case> CeilCases() {
    return {
        C({T(0)}, T(0)),
        C({-T(0)}, -T(0)),
        C({-T(1.5)}, -T(1.0)),
        C({T(1.5)}, T(2.0)),
        C({T::Lowest()}, T::Lowest()),
        C({T::Highest()}, T::Highest()),

        C({Vec(T(0), T(1.5), -T(1.5))}, Vec(T(0), T(2.0), -T(1.0))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Ceil,
    ConstEvalBuiltinTest,
    testing::Combine(
        testing::Values(core::BuiltinFn::kCeil),
        testing::ValuesIn(Concat(CeilCases<AFloat>(), CeilCases<f32>(), CeilCases<f16>()))));

template <typename T>
std::vector<Case> ClampCases() {
    auto error_msg = [&](T low, T high) {
        StringStream ss;
        ss << "12:34 error: clamp called with 'low' (" << low << ") greater than 'high' (" << high
           << ")";
        return ss.str();
    };

    return {
        C({T(0), T(0), T(0)}, T(0)), C({T(0), T(42), T::Highest()}, T(42)),
        C({T::Lowest(), T(0), T(42)}, T(0)), C({T(0), T::Lowest(), T::Highest()}, T(0)),
        C({T(0), T::Lowest(), T::Highest()}, T(0)),
        C({T::Highest(), T::Highest(), T::Highest()}, T::Highest()),
        C({T::Lowest(), T::Lowest(), T::Lowest()}, T::Lowest()),
        C({T::Highest(), T::Lowest(), T::Highest()}, T::Highest()),
        C({T::Lowest(), T::Lowest(), T::Highest()}, T::Lowest()),

        // Vector tests
        C({Vec(T(0), T(0)),                       //
           Vec(T(0), T(42)),                      //
           Vec(T(0), T::Highest())},              //
          Vec(T(0), T(42))),                      //
        C({Vec(T::Lowest(), T::Highest(), T(0)),  //
           Vec(T(0), T::Lowest(), T(5)),          //
           Vec(T(42), T::Highest(), T(6))},       //
          Vec(T(0), T::Highest(), T(5))),

        E({T(1), T(2), T(1)}, error_msg(T(2), T(1))),  //
        E({T(1), T(1), T(0)}, error_msg(T(1), T(0))),
        E({Vec(T(0), T(0)),         //
           Vec(T(1), T(2)),         //
           Vec(T(2), T(1))},        //
          error_msg(T(2), T(1))),   //
        E({Vec(T(0), T(1), T(2)),   //
           Vec(T(5), T(6), T(7)),   //
           Vec(T(5), T(7), T(6))},  //
          error_msg(T(7), T(6))),   //
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Clamp,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kClamp),
                     testing::ValuesIn(Concat(ClampCases<AInt>(),  //
                                              ClampCases<i32>(),
                                              ClampCases<u32>(),
                                              ClampCases<AFloat>(),
                                              ClampCases<f32>(),
                                              ClampCases<f16>()))));

template <typename T>
std::vector<Case> CosCases() {
    return {
        C({-T(0)}, T(1)),
        C({T(0)}, T(1)),

        C({T(0.75)}, T(0.7316888689)).FloatComp(),

        // Vector test
        C({Vec(T(0), -T(0), T(0.75))}, Vec(T(1), T(1), T(0.7316888689))).FloatComp(),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Cos,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kCos),
                     testing::ValuesIn(Concat(CosCases<AFloat>(),  //
                                              CosCases<f32>(),
                                              CosCases<f16>()))));

template <typename T>
std::vector<Case> CoshCases() {
    auto error_msg = [](auto a) {
        return "12:34 error: " + OverflowErrorMessage(a, FriendlyName<decltype(a)>());
    };
    return {
        C({T(0)}, T(1)),
        C({-T(0)}, T(1)),
        C({T(1)}, T(1.5430806348)).FloatComp(),

        C({T(.75)}, T(1.2946832847)).FloatComp(),

        // Vector tests
        C({Vec(T(0), -T(0), T(1))}, Vec(T(1), T(1), T(1.5430806348))).FloatComp(),

        E({T(10000)}, error_msg(T::Inf())),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Cosh,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kCosh),
                     testing::ValuesIn(Concat(CoshCases<AFloat>(),  //
                                              CoshCases<f32>(),
                                              CoshCases<f16>()))));

template <typename T>
std::vector<Case> CountLeadingZerosCases() {
    using B = BitValues<T>;
    return {
        C({B::Lsh(1, 31)}, T(0)),  //
        C({B::Lsh(1, 30)}, T(1)),  //
        C({B::Lsh(1, 29)}, T(2)),  //
        C({B::Lsh(1, 28)}, T(3)),
        //...
        C({B::Lsh(1, 3)}, T(28)),  //
        C({B::Lsh(1, 2)}, T(29)),  //
        C({B::Lsh(1, 1)}, T(30)),  //
        C({B::Lsh(1, 0)}, T(31)),

        C({T(0b1111'0000'1111'0000'1111'0000'1111'0000)}, T(0)),
        C({T(0b0111'1000'0111'1000'0111'1000'0111'1000)}, T(1)),
        C({T(0b0011'1100'0011'1100'0011'1100'0011'1100)}, T(2)),
        C({T(0b0001'1110'0001'1110'0001'1110'0001'1110)}, T(3)),
        //...
        C({T(0b0000'0000'0000'0000'0000'0000'0000'0111)}, T(29)),
        C({T(0b0000'0000'0000'0000'0000'0000'0000'0011)}, T(30)),
        C({T(0b0000'0000'0000'0000'0000'0000'0000'0001)}, T(31)),
        C({T(0b0000'0000'0000'0000'0000'0000'0000'0000)}, T(32)),

        // Same as above, but remove leading 0
        C({T(0b1111'1000'0111'1000'0111'1000'0111'1000)}, T(0)),
        C({T(0b1011'1100'0011'1100'0011'1100'0011'1100)}, T(0)),
        C({T(0b1001'1110'0001'1110'0001'1110'0001'1110)}, T(0)),
        //...
        C({T(0b1000'0000'0000'0000'0000'0000'0000'0111)}, T(0)),
        C({T(0b1000'0000'0000'0000'0000'0000'0000'0011)}, T(0)),
        C({T(0b1000'0000'0000'0000'0000'0000'0000'0001)}, T(0)),
        C({T(0b1000'0000'0000'0000'0000'0000'0000'0000)}, T(0)),

        // Vector tests
        C({Vec(B::Lsh(1, 31), B::Lsh(1, 30), B::Lsh(1, 29))}, Vec(T(0), T(1), T(2))),
        C({Vec(B::Lsh(1, 2), B::Lsh(1, 1), B::Lsh(1, 0))}, Vec(T(29), T(30), T(31))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    CountLeadingZeros,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kCountLeadingZeros),
                     testing::ValuesIn(Concat(CountLeadingZerosCases<i32>(),  //
                                              CountLeadingZerosCases<u32>()))));

template <typename T>
std::vector<Case> CountTrailingZerosCases() {
    using B = BitValues<T>;
    return {
        C({B::Lsh(1, 31)}, T(31)),  //
        C({B::Lsh(1, 30)}, T(30)),  //
        C({B::Lsh(1, 29)}, T(29)),  //
        C({B::Lsh(1, 28)}, T(28)),
        //...
        C({B::Lsh(1, 3)}, T(3)),  //
        C({B::Lsh(1, 2)}, T(2)),  //
        C({B::Lsh(1, 1)}, T(1)),  //
        C({B::Lsh(1, 0)}, T(0)),

        C({T(0b0000'1111'0000'1111'0000'1111'0000'1111)}, T(0)),
        C({T(0b0001'1110'0001'1110'0001'1110'0001'1110)}, T(1)),
        C({T(0b0011'1100'0011'1100'0011'1100'0011'1100)}, T(2)),
        C({T(0b0111'1000'0111'1000'0111'1000'0111'1000)}, T(3)),
        //...
        C({T(0b1110'0000'0000'0000'0000'0000'0000'0000)}, T(29)),
        C({T(0b1100'0000'0000'0000'0000'0000'0000'0000)}, T(30)),
        C({T(0b1000'0000'0000'0000'0000'0000'0000'0000)}, T(31)),
        C({T(0b0000'0000'0000'0000'0000'0000'0000'0000)}, T(32)),

        //// Same as above, but remove trailing 0
        C({T(0b0001'1110'0001'1110'0001'1110'0001'1111)}, T(0)),
        C({T(0b0011'1100'0011'1100'0011'1100'0011'1101)}, T(0)),
        C({T(0b0111'1000'0111'1000'0111'1000'0111'1001)}, T(0)),
        //...
        C({T(0b1110'0000'0000'0000'0000'0000'0000'0001)}, T(0)),
        C({T(0b1100'0000'0000'0000'0000'0000'0000'0001)}, T(0)),
        C({T(0b1000'0000'0000'0000'0000'0000'0000'0001)}, T(0)),
        C({T(0b0000'0000'0000'0000'0000'0000'0000'0001)}, T(0)),

        // Vector tests
        C({Vec(B::Lsh(1, 31), B::Lsh(1, 30), B::Lsh(1, 29))}, Vec(T(31), T(30), T(29))),
        C({Vec(B::Lsh(1, 2), B::Lsh(1, 1), B::Lsh(1, 0))}, Vec(T(2), T(1), T(0))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    CountTrailingZeros,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kCountTrailingZeros),
                     testing::ValuesIn(Concat(CountTrailingZerosCases<i32>(),  //
                                              CountTrailingZerosCases<u32>()))));

template <typename T>
std::vector<Case> CountOneBitsCases() {
    using B = BitValues<T>;
    return {
        C({T(0)}, T(0)),  //

        C({B::Lsh(1, 31)}, T(1)),  //
        C({B::Lsh(1, 30)}, T(1)),  //
        C({B::Lsh(1, 29)}, T(1)),  //
        C({B::Lsh(1, 28)}, T(1)),
        //...
        C({B::Lsh(1, 3)}, T(1)),  //
        C({B::Lsh(1, 2)}, T(1)),  //
        C({B::Lsh(1, 1)}, T(1)),  //
        C({B::Lsh(1, 0)}, T(1)),

        C({T(0b1010'1010'1010'1010'1010'1010'1010'1010)}, T(16)),
        C({T(0b0000'1111'0000'1111'0000'1111'0000'1111)}, T(16)),
        C({T(0b0101'0000'0000'0000'0000'0000'0000'0101)}, T(4)),

        // Vector tests
        C({Vec(B::Lsh(1, 31), B::Lsh(1, 30), B::Lsh(1, 29))}, Vec(T(1), T(1), T(1))),
        C({Vec(B::Lsh(1, 2), B::Lsh(1, 1), B::Lsh(1, 0))}, Vec(T(1), T(1), T(1))),

        C({Vec(T(0b1010'1010'1010'1010'1010'1010'1010'1010),
               T(0b0000'1111'0000'1111'0000'1111'0000'1111),
               T(0b0101'0000'0000'0000'0000'0000'0000'0101))},
          Vec(T(16), T(16), T(4))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    CountOneBits,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kCountOneBits),
                     testing::ValuesIn(Concat(CountOneBitsCases<i32>(),  //
                                              CountOneBitsCases<u32>()))));

template <typename T>
std::vector<Case> CrossCases() {
    constexpr auto vec_x = [](T v) { return Vec(T(v), T(0), T(0)); };
    constexpr auto vec_y = [](T v) { return Vec(T(0), T(v), T(0)); };
    constexpr auto vec_z = [](T v) { return Vec(T(0), T(0), T(v)); };

    const auto zero = Vec(T(0), T(0), T(0));
    const auto unit_x = vec_x(T(1));
    const auto unit_y = vec_y(T(1));
    const auto unit_z = vec_z(T(1));
    const auto neg_unit_x = vec_x(-T(1));
    const auto neg_unit_y = vec_y(-T(1));
    const auto neg_unit_z = vec_z(-T(1));
    const auto highest_x = vec_x(T::Highest());
    const auto highest_y = vec_y(T::Highest());
    const auto highest_z = vec_z(T::Highest());
    const auto smallest_x = vec_x(T::Smallest());
    const auto smallest_y = vec_y(T::Smallest());
    const auto smallest_z = vec_z(T::Smallest());
    const auto lowest_x = vec_x(T::Lowest());
    const auto lowest_y = vec_y(T::Lowest());
    const auto lowest_z = vec_z(T::Lowest());

    std::vector<Case> r = {
        C({zero, zero}, zero),

        C({unit_x, unit_x}, zero),
        C({unit_y, unit_y}, zero),
        C({unit_z, unit_z}, zero),

        C({smallest_x, smallest_x}, zero),
        C({smallest_y, smallest_y}, zero),
        C({smallest_z, smallest_z}, zero),

        C({lowest_x, lowest_x}, zero),
        C({lowest_y, lowest_y}, zero),
        C({lowest_z, lowest_z}, zero),

        C({highest_x, highest_x}, zero),
        C({highest_y, highest_y}, zero),
        C({highest_z, highest_z}, zero),

        C({smallest_x, highest_x}, zero),
        C({smallest_y, highest_y}, zero),
        C({smallest_z, highest_z}, zero),

        C({unit_x, neg_unit_x}, zero).PosOrNeg(),
        C({unit_y, neg_unit_y}, zero).PosOrNeg(),
        C({unit_z, neg_unit_z}, zero).PosOrNeg(),

        C({unit_x, unit_y}, unit_z),
        C({unit_y, unit_x}, neg_unit_z),

        C({unit_z, unit_x}, unit_y),
        C({unit_x, unit_z}, neg_unit_y),

        C({unit_y, unit_z}, unit_x),
        C({unit_z, unit_y}, neg_unit_x),

        C({vec_x(T(1)), vec_y(T(2))}, vec_z(T(2))),
        C({vec_y(T(1)), vec_x(T(2))}, vec_z(-T(2))),
        C({vec_x(T(2)), vec_y(T(3))}, vec_z(T(6))),
        C({vec_y(T(2)), vec_x(T(3))}, vec_z(-T(6))),

        C({Vec(T(1), T(2), T(3)), Vec(T(1), T(5), T(7))}, Vec(T(-1), T(-4), T(3))),
        C({Vec(T(33), T(44), T(55)), Vec(T(13), T(42), T(39))}, Vec(T(-594), T(-572), T(814))),
        C({Vec(T(3.5), T(4), T(5.5)), Vec(T(1), T(4.5), T(3.5))},
          Vec(T(-10.75), T(-6.75), T(11.75))),
    };

    std::string pos_error_msg =
        "12:34 error: " + OverflowErrorMessage(T::Highest(), "*", T::Highest());
    std::string neg_error_msg =
        "12:34 error: " + OverflowErrorMessage(T::Lowest(), "*", T::Lowest());
    ConcatInto(  //
        r, std::vector<Case>{
               E({highest_x, highest_y}, pos_error_msg),
               E({highest_y, highest_x}, pos_error_msg),
               E({highest_z, highest_x}, pos_error_msg),
               E({highest_x, highest_z}, pos_error_msg),
               E({highest_y, highest_z}, pos_error_msg),
               E({highest_z, highest_y}, pos_error_msg),
               E({lowest_x, lowest_y}, neg_error_msg),
               E({lowest_y, lowest_x}, neg_error_msg),
               E({lowest_z, lowest_x}, neg_error_msg),
               E({lowest_x, lowest_z}, neg_error_msg),
               E({lowest_y, lowest_z}, neg_error_msg),
               E({lowest_z, lowest_y}, neg_error_msg),
           });

    return r;
}
INSTANTIATE_TEST_SUITE_P(  //
    Cross,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kCross),
                     testing::ValuesIn(Concat(CrossCases<AFloat>(),  //
                                              CrossCases<f32>(),     //
                                              CrossCases<f16>()))));

template <typename T>
std::vector<Case> DistanceCases() {
    auto error_msg = [](auto a, const char* op, auto b) {
        return "12:34 error: " + OverflowErrorMessage(a, op, b) + R"(
12:34 note: when calculating distance)";
    };

    return {
        C({T(0), T(0)}, T(0)),
        // length(-5) -> 5
        C({T(30), T(35)}, T(5)),

        C({Vec(T(30), T(20)), Vec(T(25), T(15))}, Val(T(7.0710678119))).FloatComp(),

        E({T::Lowest(), T::Highest()}, error_msg(T::Lowest(), "-", T::Highest())),
        E({Vec(T::Highest(), T::Highest()), Vec(T(1), T(1))},
          error_msg(T(T::Highest() - T(1)), "*", T(T::Highest() - T(1)))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Distance,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kDistance),
                     testing::ValuesIn(Concat(DistanceCases<AFloat>(),  //
                                              DistanceCases<f32>(),     //
                                              DistanceCases<f16>()))));

template <typename T>
std::vector<Case> DotCases() {
    auto r = std::vector<Case>{
        C({Vec(T(0), T(0)), Vec(T(0), T(0))}, Val(T(0))),
        C({Vec(T(0), T(0), T(0)), Vec(T(0), T(0), T(0))}, Val(T(0))),
        C({Vec(T(0), T(0), T(0), T(0)), Vec(T(0), T(0), T(0), T(0))}, Val(T(0))),
        C({Vec(T(1), T(2), T(3), T(4)), Vec(T(5), T(6), T(7), T(8))}, Val(T(70))),

        C({Vec(T(1), T(1)), Vec(T(1), T(1))}, Val(T(2))),
        C({Vec(T(1), T(2)), Vec(T(2), T(1))}, Val(T(4))),
        C({Vec(T(2), T(2)), Vec(T(2), T(2))}, Val(T(8))),

        C({Vec(T::Highest(), T::Highest()), Vec(T(1), T(0))}, Val(T::Highest())),
        C({Vec(T::Lowest(), T::Lowest()), Vec(T(1), T(0))}, Val(T::Lowest())),
    };

    if constexpr (IsAbstract<T> || IsFloatingPoint<T>) {
        auto error_msg = [](auto a, const char* op, auto b) {
            return "12:34 error: " + OverflowErrorMessage(a, op, b) + R"(
12:34 note: when calculating dot)";
        };
        ConcatInto(  //
            r, std::vector<Case>{
                   E({Vec(T::Highest(), T::Highest()), Vec(T(1), T(1))},
                     error_msg(T::Highest(), "+", T::Highest())),
                   E({Vec(T::Lowest(), T::Lowest()), Vec(T(1), T(1))},
                     error_msg(T::Lowest(), "+", T::Lowest())),
               });
    } else {
        // Overflow is not an error for concrete integrals
        ConcatInto(  //
            r, std::vector<Case>{
                   C({Vec(T::Highest(), T::Highest()), Vec(T(1), T(1))},
                     Val(Add(T::Highest(), T::Highest()))),
                   C({Vec(T::Lowest(), T::Lowest()), Vec(T(1), T(1))},
                     Val(Add(T::Lowest(), T::Lowest()))),
               });
    }
    return r;
}
INSTANTIATE_TEST_SUITE_P(  //
    Dot,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kDot),
                     testing::ValuesIn(Concat(DotCases<AInt>(),    //
                                              DotCases<i32>(),     //
                                              DotCases<u32>(),     //
                                              DotCases<AFloat>(),  //
                                              DotCases<f32>(),     //
                                              DotCases<f16>()))));

template <typename T>
std::vector<Case> DeterminantCases() {
    auto error_msg = [](auto a, const char* op, auto b) {
        return "12:34 error: " + OverflowErrorMessage(a, op, b) + R"(
12:34 note: when calculating determinant)";
    };

    auto r = std::vector<Case>{
        // All zero == 0
        C({Mat({T(0), T(0)},    //
               {T(0), T(0)})},  //
          Val(T(0))),

        C({Mat({T(0), T(0), T(0)},    //
               {T(0), T(0), T(0)},    //
               {T(0), T(0), T(0)})},  //
          Val(T(0))),

        C({Mat({T(0), T(0), T(0), T(0)},    //
               {T(0), T(0), T(0), T(0)},    //
               {T(0), T(0), T(0), T(0)},    //
               {T(0), T(0), T(0), T(0)})},  //
          Val(T(0))),

        // All same == 0
        C({Mat({T(42), T(42)},    //
               {T(42), T(42)})},  //
          Val(T(0))),

        C({Mat({T(42), T(42), T(42)},    //
               {T(42), T(42), T(42)},    //
               {T(42), T(42), T(42)})},  //
          Val(T(0))),

        C({Mat({T(42), T(42), T(42), T(42)},    //
               {T(42), T(42), T(42), T(42)},    //
               {T(42), T(42), T(42), T(42)},    //
               {T(42), T(42), T(42), T(42)})},  //
          Val(T(0))),

        // Various values
        C({Mat({-T(2), T(17)},   //
               {T(5), T(45)})},  //
          Val(-T(175))),

        C({Mat({T(4), T(6), -T(13)},    //
               {T(12), T(5), T(8)},     //
               {T(9), T(17), T(16)})},  //
          Val(-T(3011))),

        C({Mat({T(2), T(9), T(8), T(1)},       //
               {-T(4), T(11), -T(3), T(7)},    //
               {T(6), T(5), T(12), -T(6)},     //
               {T(3), -T(10), T(4), -T(7)})},  //
          Val(T(469))),

        // Overflow during multiply
        E({Mat({T::Highest(), T(0)},  //
               {T(0), T(2)})},        //
          error_msg(T::Highest(), "*", T(2))),

        // Overflow during subtract
        E({Mat({T::Highest(), T::Lowest()},  //
               {T(1), T(1)})},               //
          error_msg(T::Highest(), "-", T::Lowest())),
    };

    return r;
}
INSTANTIATE_TEST_SUITE_P(  //
    Determinant,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kDeterminant),
                     testing::ValuesIn(Concat(DeterminantCases<AFloat>(),  //
                                              DeterminantCases<f32>(),     //
                                              DeterminantCases<f16>()))));

template <typename T>
std::vector<Case> FaceForwardCases() {
    // Rotate v by degs around Z axis
    auto rotate = [&](const Value& v, float degs) {
        auto x = builder::As<T>(v.args[0]);
        auto y = builder::As<T>(v.args[1]);
        auto z = builder::As<T>(v.args[2]);
        auto rads = T(degs) * kPi<T> / T(180);
        auto x2 = T(x * std::cos(rads) - y * std::sin(rads));
        auto y2 = T(x * std::sin(rads) + y * std::cos(rads));
        return Vec(x2, y2, z);
    };

    // An arbitrary input vector and its negation, used for e1 args to FaceForward
    auto pos_vec = Vec(T(1), T(2), T(3));
    auto neg_vec = Vec(-T(1), -T(2), -T(3));

    // An arbitrary vector in the xy plane, used for e2 and e3 args to FaceForward.
    auto fwd_xy = Vec(T(1.23), T(4.56), T(0));

    std::vector<Case> r = {
        C({pos_vec, fwd_xy, rotate(fwd_xy, 85)}, neg_vec),
        C({pos_vec, fwd_xy, rotate(fwd_xy, 85)}, neg_vec),
        C({pos_vec, fwd_xy, rotate(fwd_xy, 95)}, pos_vec),
        C({pos_vec, fwd_xy, rotate(fwd_xy, -95)}, pos_vec),
        C({pos_vec, fwd_xy, rotate(fwd_xy, 180)}, pos_vec),

        C({pos_vec, rotate(fwd_xy, 33), rotate(fwd_xy, 33 + 85)}, neg_vec),
        C({pos_vec, rotate(fwd_xy, 33), rotate(fwd_xy, 33 - 85)}, neg_vec),
        C({pos_vec, rotate(fwd_xy, 33), rotate(fwd_xy, 33 + 95)}, pos_vec),
        C({pos_vec, rotate(fwd_xy, 33), rotate(fwd_xy, 33 - 95)}, pos_vec),
        C({pos_vec, rotate(fwd_xy, 33), rotate(fwd_xy, 33 + 180)}, pos_vec),

        C({pos_vec, rotate(fwd_xy, 234), rotate(fwd_xy, 234 + 85)}, neg_vec),
        C({pos_vec, rotate(fwd_xy, 234), rotate(fwd_xy, 234 - 85)}, neg_vec),
        C({pos_vec, rotate(fwd_xy, 234), rotate(fwd_xy, 234 + 95)}, pos_vec),
        C({pos_vec, rotate(fwd_xy, 234), rotate(fwd_xy, 234 - 95)}, pos_vec),
        C({pos_vec, rotate(fwd_xy, 234), rotate(fwd_xy, 234 + 180)}, pos_vec),

        // Same, but swap input and result vectors
        C({neg_vec, fwd_xy, rotate(fwd_xy, 85)}, pos_vec),
        C({neg_vec, fwd_xy, rotate(fwd_xy, 85)}, pos_vec),
        C({neg_vec, fwd_xy, rotate(fwd_xy, 95)}, neg_vec),
        C({neg_vec, fwd_xy, rotate(fwd_xy, -95)}, neg_vec),
        C({neg_vec, fwd_xy, rotate(fwd_xy, 180)}, neg_vec),

        C({neg_vec, rotate(fwd_xy, 33), rotate(fwd_xy, 33 + 85)}, pos_vec),
        C({neg_vec, rotate(fwd_xy, 33), rotate(fwd_xy, 33 - 85)}, pos_vec),
        C({neg_vec, rotate(fwd_xy, 33), rotate(fwd_xy, 33 + 95)}, neg_vec),
        C({neg_vec, rotate(fwd_xy, 33), rotate(fwd_xy, 33 - 95)}, neg_vec),
        C({neg_vec, rotate(fwd_xy, 33), rotate(fwd_xy, 33 + 180)}, neg_vec),

        C({neg_vec, rotate(fwd_xy, 234), rotate(fwd_xy, 234 + 85)}, pos_vec),
        C({neg_vec, rotate(fwd_xy, 234), rotate(fwd_xy, 234 - 85)}, pos_vec),
        C({neg_vec, rotate(fwd_xy, 234), rotate(fwd_xy, 234 + 95)}, neg_vec),
        C({neg_vec, rotate(fwd_xy, 234), rotate(fwd_xy, 234 - 95)}, neg_vec),
        C({neg_vec, rotate(fwd_xy, 234), rotate(fwd_xy, 234 + 180)}, neg_vec),
    };

    auto error_msg = [](auto a, const char* op, auto b) {
        return "12:34 error: " + OverflowErrorMessage(a, op, b) + R"(
12:34 note: when calculating faceForward)";
    };
    ConcatInto(  //
        r, std::vector<Case>{
               // Overflow the dot product operation
               E({pos_vec, Vec(T::Highest(), T::Highest(), T(0)), Vec(T(1), T(1), T(0))},
                 error_msg(T::Highest(), "+", T::Highest())),
               E({pos_vec, Vec(T::Lowest(), T::Lowest(), T(0)), Vec(T(1), T(1), T(0))},
                 error_msg(T::Lowest(), "+", T::Lowest())),
           });

    return r;
}
INSTANTIATE_TEST_SUITE_P(  //
    FaceForward,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kFaceForward),
                     testing::ValuesIn(Concat(FaceForwardCases<AFloat>(),  //
                                              FaceForwardCases<f32>(),     //
                                              FaceForwardCases<f16>()))));

template <typename T>
std::vector<Case> FirstLeadingBitCases() {
    using B = BitValues<T>;
    auto r = std::vector<Case>{
        // Both signed and unsigned return T(-1) for input 0
        C({T(0)}, T(-1)),

        C({B::Lsh(1, 30)}, T(30)),  //
        C({B::Lsh(1, 29)}, T(29)),  //
        C({B::Lsh(1, 28)}, T(28)),
        //...
        C({B::Lsh(1, 3)}, T(3)),  //
        C({B::Lsh(1, 2)}, T(2)),  //
        C({B::Lsh(1, 1)}, T(1)),  //
        C({B::Lsh(1, 0)}, T(0)),

        C({T(0b0000'0000'0100'1000'1000'1000'0000'0000)}, T(22)),
        C({T(0b0000'0000'0000'0100'1000'1000'0000'0000)}, T(18)),

        // Vector tests
        C({Vec(B::Lsh(1, 30), B::Lsh(1, 29), B::Lsh(1, 28))}, Vec(T(30), T(29), T(28))),
        C({Vec(B::Lsh(1, 2), B::Lsh(1, 1), B::Lsh(1, 0))}, Vec(T(2), T(1), T(0))),
    };

    ConcatIntoIf<IsUnsignedIntegral<T>>(  //
        r, std::vector<Case>{
               C({B::Lsh(1, 31)}, T(31)),

               C({T(0b1111'1111'1111'1111'1111'1111'1111'1110)}, T(31)),
               C({T(0b1111'1111'1111'1111'1111'1111'1111'1100)}, T(31)),
               C({T(0b1111'1111'1111'1111'1111'1111'1111'1000)}, T(31)),
               //...
               C({T(0b1110'0000'0000'0000'0000'0000'0000'0000)}, T(31)),
               C({T(0b1100'0000'0000'0000'0000'0000'0000'0000)}, T(31)),
               C({T(0b1000'0000'0000'0000'0000'0000'0000'0000)}, T(31)),
           });

    ConcatIntoIf<IsSignedIntegral<T>>(  //
        r, std::vector<Case>{
               // Signed returns -1 for input -1
               C({T(-1)}, T(-1)),

               C({B::Lsh(1, 31)}, T(30)),

               C({T(0b1111'1111'1111'1111'1111'1111'1111'1110)}, T(0)),
               C({T(0b1111'1111'1111'1111'1111'1111'1111'1100)}, T(1)),
               C({T(0b1111'1111'1111'1111'1111'1111'1111'1000)}, T(2)),
               //...
               C({T(0b1110'0000'0000'0000'0000'0000'0000'0000)}, T(28)),
               C({T(0b1100'0000'0000'0000'0000'0000'0000'0000)}, T(29)),
               C({T(0b1000'0000'0000'0000'0000'0000'0000'0000)}, T(30)),
           });

    return r;
}
INSTANTIATE_TEST_SUITE_P(  //
    FirstLeadingBit,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kFirstLeadingBit),
                     testing::ValuesIn(Concat(FirstLeadingBitCases<i32>(),  //
                                              FirstLeadingBitCases<u32>()))));

template <typename T>
std::vector<Case> FirstTrailingBitCases() {
    using B = BitValues<T>;
    auto r = std::vector<Case>{
        C({T(0)}, T(-1)),

        C({B::Lsh(1, 31)}, T(31)),  //
        C({B::Lsh(1, 30)}, T(30)),  //
        C({B::Lsh(1, 29)}, T(29)),  //
        C({B::Lsh(1, 28)}, T(28)),
        //...
        C({B::Lsh(1, 3)}, T(3)),  //
        C({B::Lsh(1, 2)}, T(2)),  //
        C({B::Lsh(1, 1)}, T(1)),  //
        C({B::Lsh(1, 0)}, T(0)),

        C({T(0b0000'0000'0100'1000'1000'1000'0000'0000)}, T(11)),
        C({T(0b0000'0100'1000'1000'1000'0000'0000'0000)}, T(15)),

        // Vector tests
        C({Vec(B::Lsh(1, 31), B::Lsh(1, 30), B::Lsh(1, 29))}, Vec(T(31), T(30), T(29))),
        C({Vec(B::Lsh(1, 2), B::Lsh(1, 1), B::Lsh(1, 0))}, Vec(T(2), T(1), T(0))),
    };

    return r;
}
INSTANTIATE_TEST_SUITE_P(  //
    FirstTrailingBit,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kFirstTrailingBit),
                     testing::ValuesIn(Concat(FirstTrailingBitCases<i32>(),  //
                                              FirstTrailingBitCases<u32>()))));

template <typename T>
std::vector<Case> FloorCases() {
    return {
        C({T(0)}, T(0)),
        C({-T(0)}, -T(0)),
        C({-T(1.5)}, -T(2.0)),
        C({T(1.5)}, T(1.0)),
        C({T::Lowest()}, T::Lowest()),
        C({T::Highest()}, T::Highest()),

        C({Vec(T(0), T(1.5), -T(1.5))}, Vec(T(0), T(1.0), -T(2.0))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Floor,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kFloor),
                     testing::ValuesIn(Concat(FloorCases<AFloat>(),  //
                                              FloorCases<f32>(),
                                              FloorCases<f16>()))));

template <typename T>
std::vector<Case> FmaCases() {
    auto error_msg = [](auto a, const char* op, auto b) {
        return "12:34 error: " + OverflowErrorMessage(a, op, b) + R"(
12:34 note: when calculating fma)";
    };
    return {
        C({T(0), T(0), T(0)}, T(0)),
        C({T(1), T(2), T(3)}, T(5)),
        C({Vec(T(1), T(2.5), -T(1)), Vec(T(2), T(2.5), T(1)), Vec(T(4), T(3.75), -T(2))},
          Vec(T(6), T(10), -T(3))),

        E({T::Highest(), T::Highest(), T(0)}, error_msg(T::Highest(), "*", T::Highest())),
        E({T::Highest(), T(1), T::Highest()}, error_msg(T::Highest(), "+", T::Highest())),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Fma,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kFma),
                     testing::ValuesIn(Concat(FmaCases<AFloat>(),  //
                                              FmaCases<f32>(),
                                              FmaCases<f16>()))));

template <typename T>
std::vector<Case> FractCases() {
    auto r = std::vector<Case>{
        C({T(0)}, T(0)),
        C({T(0.1)}, T(0.1)),
        C({T(-0.1)}, T(0.9)),
        C({T(0.0000001)}, T(0.0000001)),
        C({T(-0.0000001)}, T(0.9999999)),
        C({T(12.34567)}, T(0.34567)).FloatComp(0.002),
        C({T(-12.34567)}, T(0.65433)).FloatComp(0.002),
        C({T::Lowest()}, T(0)),
        C({T::Highest()}, T(0)),
        // Vector tests
        C({Vec(T(0.1), T(-0.1), T(-0.0000001))}, Vec(T(0.1), T(0.9), T(0.9999999))),
    };
    // Note: Valid results are in the closed interval [0, 1.0]. For example, if e is a very small
    // negative number, then fract(e) may be 1.0.
    ConcatIntoIf<!std::is_same_v<T, f16>>(  //
        r, std::vector<Case>{
               C({T(-0.000000000000000001)}, T(1)),
           });

    return r;
}
INSTANTIATE_TEST_SUITE_P(  //
    Fract,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kFract),
                     testing::ValuesIn(Concat(FractCases<AFloat>(),  //
                                              FractCases<f32>(),
                                              FractCases<f16>()))));

template <typename T>
std::vector<Case> FrexpCases() {
    using F = T;                                                         // fract type
    using E = std::conditional_t<std::is_same_v<T, AFloat>, AInt, i32>;  // exp type

    std::vector<Case> cases = {
        // Scalar tests
        //  in         fract     exp
        C({T(-3.5)}, {F(-0.875), E(2)}),  //
        C({T(-3.0)}, {F(-0.750), E(2)}),  //
        C({T(-2.5)}, {F(-0.625), E(2)}),  //
        C({T(-2.0)}, {F(-0.500), E(2)}),  //
        C({T(-1.5)}, {F(-0.750), E(1)}),  //
        C({T(-1.0)}, {F(-0.500), E(1)}),  //
        C({T(+0.0)}, {F(+0.000), E(0)}),  //
        C({T(+1.0)}, {F(+0.500), E(1)}),  //
        C({T(+1.5)}, {F(+0.750), E(1)}),  //
        C({T(+2.0)}, {F(+0.500), E(2)}),  //
        C({T(+2.5)}, {F(+0.625), E(2)}),  //
        C({T(+3.0)}, {F(+0.750), E(2)}),  //
        C({T(+3.5)}, {F(+0.875), E(2)}),  //

        // Vector tests
        //         in                 fract                    exp
        C({Vec(T(-2.5), T(+1.0))}, {Vec(F(-0.625), F(+0.500)), Vec(E(2), E(1))}),
        C({Vec(T(+3.5), T(-2.5))}, {Vec(F(+0.875), F(-0.625)), Vec(E(2), E(2))}),
    };

    ConcatIntoIf<std::is_same_v<T, f16>>(cases, std::vector<Case>{
                                                    C({T::Highest()}, {F(0x0.ffep0), E(16)}),  //
                                                    C({T::Lowest()}, {F(-0x0.ffep0), E(16)}),  //
                                                    C({T::Smallest()}, {F(0.5), E(-13)}),      //
                                                });

    ConcatIntoIf<std::is_same_v<T, f32>>(cases,
                                         std::vector<Case>{
                                             C({T::Highest()}, {F(0x0.ffffffp0), E(128)}),  //
                                             C({T::Lowest()}, {F(-0x0.ffffffp0), E(128)}),  //
                                             C({T::Smallest()}, {F(0.5), E(-125)}),         //
                                         });

    ConcatIntoIf<std::is_same_v<T, AFloat>>(
        cases, std::vector<Case>{
                   C({T::Highest()}, {F(0x0.fffffffffffff8p0), E(1024)}),  //
                   C({T::Lowest()}, {F(-0x0.fffffffffffff8p0), E(1024)}),  //
                   C({T::Smallest()}, {F(0.5), E(-1021)}),                 //
               });
    return cases;
}
INSTANTIATE_TEST_SUITE_P(  //
    Frexp,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kFrexp),
                     testing::ValuesIn(Concat(FrexpCases<AFloat>(),  //
                                              FrexpCases<f32>(),     //
                                              FrexpCases<f16>()))));

template <typename T>
std::vector<Case> InsertBitsCases() {
    using UT = Number<std::make_unsigned_t<UnwrapNumber<T>>>;

    auto e = /* */ T(0b0101'1100'0011'1010'0101'1100'0011'1010);
    auto newbits = T{0b1010'0011'1100'0101'1010'0011'1100'0101};

    auto r = std::vector<Case>{
        // args: e, newbits, offset, count

        // If count is 0, result is e
        C({e, newbits, UT(0), UT(0)}, e),  //
        C({e, newbits, UT(1), UT(0)}, e),  //
        C({e, newbits, UT(2), UT(0)}, e),  //
        C({e, newbits, UT(3), UT(0)}, e),  //
        // ...
        C({e, newbits, UT(29), UT(0)}, e),  //
        C({e, newbits, UT(30), UT(0)}, e),  //
        C({e, newbits, UT(31), UT(0)}, e),

        // Copy 1 to 32 bits of newbits to e at offset 0
        C({e, newbits, UT(0), UT(1)}, T(0b0101'1100'0011'1010'0101'1100'0011'1011)),
        C({e, newbits, UT(0), UT(2)}, T(0b0101'1100'0011'1010'0101'1100'0011'1001)),
        C({e, newbits, UT(0), UT(3)}, T(0b0101'1100'0011'1010'0101'1100'0011'1101)),
        C({e, newbits, UT(0), UT(4)}, T(0b0101'1100'0011'1010'0101'1100'0011'0101)),
        C({e, newbits, UT(0), UT(5)}, T(0b0101'1100'0011'1010'0101'1100'0010'0101)),
        C({e, newbits, UT(0), UT(6)}, T(0b0101'1100'0011'1010'0101'1100'0000'0101)),
        // ...
        C({e, newbits, UT(0), UT(29)}, T(0b0100'0011'1100'0101'1010'0011'1100'0101)),
        C({e, newbits, UT(0), UT(30)}, T(0b0110'0011'1100'0101'1010'0011'1100'0101)),
        C({e, newbits, UT(0), UT(31)}, T(0b0010'0011'1100'0101'1010'0011'1100'0101)),
        C({e, newbits, UT(0), UT(32)}, T(0b1010'0011'1100'0101'1010'0011'1100'0101)),

        // Copy at varying offsets and counts
        C({e, newbits, UT(3), UT(8)}, T(0b0101'1100'0011'1010'0101'1110'0010'1010)),
        C({e, newbits, UT(8), UT(8)}, T(0b0101'1100'0011'1010'1100'0101'0011'1010)),
        C({e, newbits, UT(15), UT(1)}, T(0b0101'1100'0011'1010'1101'1100'0011'1010)),
        C({e, newbits, UT(16), UT(16)}, T(0b1010'0011'1100'0101'0101'1100'0011'1010)),

        // Vector tests
        C({Vec(T(0b1111'0000'1111'0000'1111'0000'1111'0000),  //
               T(0b0000'1111'0000'1111'0000'1111'0000'1111),  //
               T(0b1010'0101'1010'0101'1010'0101'1010'0101)),
           Vec(T(0b1111'1111'1111'1111'1111'1111'1111'1111),  //
               T(0b1111'1111'1111'1111'1111'1111'1111'1111),  //
               T(0b1111'1111'1111'1111'1111'1111'1111'1111)),
           Val(UT(3)), Val(UT(8))},
          Vec(T(0b1111'0000'1111'0000'1111'0111'1111'1000),  //
              T(0b0000'1111'0000'1111'0000'1111'1111'1111),  //
              T(0b1010'0101'1010'0101'1010'0111'1111'1101))),
    };

    const char* error_msg =
        "12:34 error: 'offset' + 'count' must be less than or equal to the bit width of 'e'";
    ConcatInto(  //
        r, std::vector<Case>{
               E({T(1), T(1), UT(33), UT(0)}, error_msg),         //
               E({T(1), T(1), UT(34), UT(0)}, error_msg),         //
               E({T(1), T(1), UT(1000), UT(0)}, error_msg),       //
               E({T(1), T(1), UT::Highest(), UT()}, error_msg),   //
               E({T(1), T(1), UT(0), UT(33)}, error_msg),         //
               E({T(1), T(1), UT(0), UT(34)}, error_msg),         //
               E({T(1), T(1), UT(0), UT(1000)}, error_msg),       //
               E({T(1), T(1), UT(0), UT::Highest()}, error_msg),  //
               E({T(1), T(1), UT(33), UT(33)}, error_msg),        //
               E({T(1), T(1), UT(34), UT(34)}, error_msg),        //
               E({T(1), T(1), UT(1000), UT(1000)}, error_msg),    //
               E({T(1), T(1), UT::Highest(), UT(1)}, error_msg),
               E({T(1), T(1), UT(1), UT::Highest()}, error_msg),
               E({T(1), T(1), UT::Highest(), u32::Highest()}, error_msg),
           });

    return r;
}
INSTANTIATE_TEST_SUITE_P(  //
    InsertBits,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kInsertBits),
                     testing::ValuesIn(Concat(InsertBitsCases<i32>(),  //
                                              InsertBitsCases<u32>()))));

template <typename T>
std::vector<Case> InverseSqrtCases() {
    return {
        C({T(25)}, T(.2)),

        // Vector tests
        C({Vec(T(25), T(100))}, Vec(T(.2), T(.1))),

        E({T(0)}, "12:34 error: inverseSqrt must be called with a value > 0"),
        E({-T(0)}, "12:34 error: inverseSqrt must be called with a value > 0"),
        E({-T(25)}, "12:34 error: inverseSqrt must be called with a value > 0"),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    InverseSqrt,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kInverseSqrt),
                     testing::ValuesIn(Concat(InverseSqrtCases<AFloat>(),  //
                                              InverseSqrtCases<f32>(),
                                              InverseSqrtCases<f16>()))));

template <typename T>
std::vector<Case> DegreesAFloatCases() {
    return {
        C({T(0)}, T(0)),                             //
        C({-T(0)}, -T(0)),                           //
        C({T(0.698132)}, T(40)).FloatComp(),         //
        C({-T(1.5708)}, -T(90.000214)).FloatComp(),  //
        C({T(1.5708)}, T(90.000214)).FloatComp(),    //
        C({T(6.28319)}, T(360.00027)).FloatComp(),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    DegreesAFloat,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kDegrees),
                     testing::ValuesIn(DegreesAFloatCases<AFloat>())));

template <typename T>
std::vector<Case> DegreesF32Cases() {
    return {
        C({T(0)}, T(0)),                             //
        C({-T(0)}, -T(0)),                           //
        C({T(0.698132)}, T(40)).FloatComp(),         //
        C({-T(1.5708)}, -T(90.000206)).FloatComp(),  //
        C({T(1.5708)}, T(90.000206)).FloatComp(),    //
        C({T(6.28319)}, T(360.00024)).FloatComp(),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    DegreesF32,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kDegrees),
                     testing::ValuesIn(DegreesF32Cases<f32>())));

template <typename T>
std::vector<Case> DegreesF16Cases() {
    return {
        C({T(0)}, T(0)),                            //
        C({-T(0)}, -T(0)),                          //
        C({T(0.698132)}, T(39.96875)).FloatComp(),  //
        C({-T(1.5708)}, -T(89.9375)).FloatComp(),   //
        C({T(1.5708)}, T(89.9375)).FloatComp(),     //
        C({T(6.28319)}, T(359.75)).FloatComp(),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    DegreesF16,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kDegrees),
                     testing::ValuesIn(DegreesF16Cases<f16>())));

template <typename T>
std::vector<Case> ExpCases() {
    auto error_msg = [](auto a) { return "12:34 error: " + OverflowExpErrorMessage("e", a); };
    return {C({T(0)}, T(1)),   //
            C({-T(0)}, T(1)),  //
            C({T(2)}, T(7.3890562)).FloatComp(),
            C({-T(2)}, T(0.13533528)).FloatComp(),  //
            C({T::Lowest()}, T(0)),

            E({T::Highest()}, error_msg(T::Highest()))};
}
INSTANTIATE_TEST_SUITE_P(  //
    Exp,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kExp),
                     testing::ValuesIn(Concat(ExpCases<AFloat>(),  //
                                              ExpCases<f32>(),
                                              ExpCases<f16>()))));

template <typename T>
std::vector<Case> Exp2Cases() {
    auto error_msg = [](auto a) { return "12:34 error: " + OverflowExpErrorMessage("2", a); };
    return {
        C({T(0)}, T(1)),   //
        C({-T(0)}, T(1)),  //
        C({T(2)}, T(4.0)),
        C({-T(2)}, T(0.25)),  //
        C({T::Lowest()}, T(0)),

        E({T::Highest()}, error_msg(T::Highest())),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Exp2,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kExp2),
                     testing::ValuesIn(Concat(Exp2Cases<AFloat>(),  //
                                              Exp2Cases<f32>(),
                                              Exp2Cases<f16>()))));

template <typename T>
std::vector<Case> ExtractBitsCases() {
    using UT = Number<std::make_unsigned_t<UnwrapNumber<T>>>;

    // If T is signed, fills most significant bits of `val` with 1s
    auto set_msbs_if_signed = [](T val) {
        if constexpr (IsSignedIntegral<T>) {
            T result = T(~0);
            for (size_t b = 0; val; ++b) {
                if ((val & 1) == 0) {
                    result = result & ~(1 << b);  // Clear bit b
                }
                val = val >> 1;
            }
            return result;
        } else {
            return val;
        }
    };

    auto e = T(0b10100011110001011010001111000101);
    auto f = T(0b01010101010101010101010101010101);
    auto g = T(0b11111010001111000101101000111100);

    auto r = std::vector<Case>{
        // args: e, offset, count

        // If count is 0, result is 0
        C({e, UT(0), UT(0)}, T(0)),  //
        C({e, UT(1), UT(0)}, T(0)),  //
        C({e, UT(2), UT(0)}, T(0)),  //
        C({e, UT(3), UT(0)}, T(0)),
        // ...
        C({e, UT(29), UT(0)}, T(0)),  //
        C({e, UT(30), UT(0)}, T(0)),  //
        C({e, UT(31), UT(0)}, T(0)),

        // Extract at offset 0, varying counts
        C({e, UT(0), UT(1)}, set_msbs_if_signed(T(0b1))),    //
        C({e, UT(0), UT(2)}, T(0b01)),                       //
        C({e, UT(0), UT(3)}, set_msbs_if_signed(T(0b101))),  //
        C({e, UT(0), UT(4)}, T(0b0101)),                     //
        C({e, UT(0), UT(5)}, T(0b00101)),                    //
        C({e, UT(0), UT(6)}, T(0b000101)),                   //
        // ...
        C({e, UT(0), UT(28)}, T(0b0011110001011010001111000101)),                        //
        C({e, UT(0), UT(29)}, T(0b00011110001011010001111000101)),                       //
        C({e, UT(0), UT(30)}, set_msbs_if_signed(T(0b100011110001011010001111000101))),  //
        C({e, UT(0), UT(31)}, T(0b0100011110001011010001111000101)),                     //
        C({e, UT(0), UT(32)}, T(0b10100011110001011010001111000101)),                    //

        // Extract at varying offsets and counts
        C({e, UT(0), UT(1)}, set_msbs_if_signed(T(0b1))),                   //
        C({e, UT(31), UT(1)}, set_msbs_if_signed(T(0b1))),                  //
        C({e, UT(3), UT(5)}, set_msbs_if_signed(T(0b11000))),               //
        C({e, UT(4), UT(7)}, T(0b0111100)),                                 //
        C({e, UT(10), UT(16)}, set_msbs_if_signed(T(0b1111000101101000))),  //
        C({e, UT(10), UT(22)}, set_msbs_if_signed(T(0b1010001111000101101000))),

        // Vector tests
        C({Vec(e, f, g),                          //
           Val(UT(5)), Val(UT(8))},               //
          Vec(T(0b00011110),                      //
              set_msbs_if_signed(T(0b10101010)),  //
              set_msbs_if_signed(T(0b11010001)))),
    };

    const char* error_msg =
        "12:34 error: 'offset' + 'count' must be less than or equal to the bit width of 'e'";
    ConcatInto(  //
        r, std::vector<Case>{
               E({T(1), UT(33), UT(0)}, error_msg),
               E({T(1), UT(34), UT(0)}, error_msg),
               E({T(1), UT(1000), UT(0)}, error_msg),
               E({T(1), UT::Highest(), UT(0)}, error_msg),
               E({T(1), UT(0), UT(33)}, error_msg),
               E({T(1), UT(0), UT(34)}, error_msg),
               E({T(1), UT(0), UT(1000)}, error_msg),
               E({T(1), UT(0), UT::Highest()}, error_msg),
               E({T(1), UT(33), UT(33)}, error_msg),
               E({T(1), UT(34), UT(34)}, error_msg),
               E({T(1), UT(1000), UT(1000)}, error_msg),
               E({T(1), UT::Highest(), UT(1)}, error_msg),
               E({T(1), UT(1), UT::Highest()}, error_msg),
               E({T(1), UT::Highest(), UT::Highest()}, error_msg),
           });

    return r;
}
INSTANTIATE_TEST_SUITE_P(  //
    ExtractBits,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kExtractBits),
                     testing::ValuesIn(Concat(ExtractBitsCases<i32>(),  //
                                              ExtractBitsCases<u32>()))));

template <typename T>
std::vector<Case> LdexpCases() {
    using T2 = std::conditional_t<std::is_same_v<T, AFloat>, AInt, i32>;
    T2 bias;
    if constexpr (std::is_same_v<T, f16>) {
        bias = 15;
    } else if constexpr (std::is_same_v<T, f32>) {
        bias = 127;
    } else {
        bias = 1023;
    }

    auto compute = [](T e1, T2 e2) { return T{std::ldexp(e1.value, static_cast<int>(e2.value))}; };

    auto r = std::vector<Case>{
        C({T(0), T2(0)}, T(0)),          //
        C({T(7), T2(4)}, T(112)),        //
        C({T(7), T2(5)}, T(224)),        //
        C({T(7), T2(6)}, T(448)),        //
        C({T(7), T2(-4)}, T(0.4375)),    //
        C({T(7), T2(-5)}, T(0.21875)),   //
        C({T(7), T2(-6)}, T(0.109375)),  //
        // With bias exponent
        C({T(0), T2(bias)}, T(0)),                           //
        C({T(0), T2(bias + 1)}, T(0)),                       //
        C({T(1), T2(bias)}, compute(T(1), T2(bias))),        //
        C({T(0.5), T2(bias)}, compute(T(0.5), T2(bias))),    //
        C({T(0.25), T2(bias)}, compute(T(0.25), T2(bias))),  //
        // The result may be zero if e2 + bias ≤ 0.
        C({T(0), T2(-bias)}, T(0)),      //
        C({T(0), T2(-bias - 1)}, T(0)),  //
        C({T(0), T2(-bias - 2)}, T(0)),  //

        // Vector tests
        C({Vec(T(0), T(7), T(7)), Vec(T2(0), T2(4), T2(-4))}, Vec(T(0), T(112), T(0.4375))),
    };

    std::string e2_too_large_error_msg =
        "12:34 error: e2 must be less than or equal to " + std::to_string(bias + 1);
    auto val_overflow_error_msg = [](auto val) {
        return "12:34 error: " + OverflowErrorMessage(val, FriendlyName<T>());
    };
    ConcatInto(r, std::vector<Case>{
                      // e2 is > bias + 1
                      E({T(0), T2(bias + 2)}, e2_too_large_error_msg),
                      E({T(0), T2(bias + 1000)}, e2_too_large_error_msg),
                      E({T(0), T2::Highest()}, e2_too_large_error_msg),
                      // Result is inf
                      E({T(1), T2(bias + 1)}, val_overflow_error_msg(T::Inf())),
                      E({T(2), T2(bias + 1)}, val_overflow_error_msg(T::Inf())),
                      E({T::Highest(), T2(bias + 1)}, val_overflow_error_msg(T::Inf())),
                      E({T(-1), T2(bias + 1)}, val_overflow_error_msg(-T::Inf())),
                      E({T(-2), T2(bias + 1)}, val_overflow_error_msg(-T::Inf())),
                      E({T::Lowest(), T2(bias + 1)}, val_overflow_error_msg(-T::Inf())),
                  });
    return r;
}
INSTANTIATE_TEST_SUITE_P(  //
    Ldexp,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kLdexp),
                     testing::ValuesIn(Concat(LdexpCases<AFloat>(),  //
                                              LdexpCases<f32>(),
                                              LdexpCases<f16>()))));

template <typename T>
std::vector<Case> LengthCases() {
    const auto kSqrtOfHighest = T(std::sqrt(T::Highest()));
    const auto kSqrtOfHighestSquared = T(kSqrtOfHighest * kSqrtOfHighest);

    auto error_msg = [](auto a, const char* op, auto b) {
        return "12:34 error: " + OverflowErrorMessage(a, op, b) + R"(
12:34 note: when calculating length)";
    };
    return {
        C({T(0)}, T(0)),
        C({Vec(T(0), T(0))}, Val(T(0))),
        C({Vec(T(0), T(0), T(0))}, Val(T(0))),
        C({Vec(T(0), T(0), T(0), T(0))}, Val(T(0))),

        C({T(1)}, T(1)),
        C({Vec(T(1), T(1))}, Val(T(std::sqrt(2)))),
        C({Vec(T(1), T(1), T(1))}, Val(T(std::sqrt(3)))),
        C({Vec(T(1), T(1), T(1), T(1))}, Val(T(std::sqrt(4)))),

        C({T(2)}, T(2)),
        C({Vec(T(2), T(2))}, Val(T(std::sqrt(8)))),
        C({Vec(T(2), T(2), T(2))}, Val(T(std::sqrt(12)))),
        C({Vec(T(2), T(2), T(2), T(2))}, Val(T(std::sqrt(16)))),

        C({Vec(T(2), T(3))}, Val(T(std::sqrt(13)))),
        C({Vec(T(2), T(3), T(4))}, Val(T(std::sqrt(29)))),
        C({Vec(T(2), T(3), T(4), T(5))}, Val(T(std::sqrt(54)))),

        C({T(-5)}, T(5)),
        C({T::Highest()}, T::Highest()),
        C({T::Lowest()}, T::Highest()),

        C({Vec(T(-2), T(-3), T(-4), T(-5))}, Val(T(std::sqrt(54)))),
        C({Vec(T(2), T(-3), T(4), T(-5))}, Val(T(std::sqrt(54)))),
        C({Vec(T(-2), T(3), T(-4), T(5))}, Val(T(std::sqrt(54)))),

        C({Vec(kSqrtOfHighest, T(0))}, Val(kSqrtOfHighest)).FloatComp(0.2),
        C({Vec(T(0), kSqrtOfHighest)}, Val(kSqrtOfHighest)).FloatComp(0.2),

        C({Vec(-kSqrtOfHighest, T(0))}, Val(kSqrtOfHighest)).FloatComp(0.2),
        C({Vec(T(0), -kSqrtOfHighest)}, Val(kSqrtOfHighest)).FloatComp(0.2),

        // Overflow when squaring a term
        E({Vec(T::Highest(), T(0))}, error_msg(T::Highest(), "*", T::Highest())),
        E({Vec(T(0), T::Highest())}, error_msg(T::Highest(), "*", T::Highest())),
        // Overflow when adding squared terms
        E({Vec(kSqrtOfHighest, kSqrtOfHighest)},
          error_msg(kSqrtOfHighestSquared, "+", kSqrtOfHighestSquared)),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Length,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kLength),
                     testing::ValuesIn(Concat(LengthCases<AFloat>(),  //
                                              LengthCases<f32>(),
                                              LengthCases<f16>()))));

template <typename T>
std::vector<Case> LogCases() {
    auto error_msg = [] { return "12:34 error: log must be called with a value > 0"; };
    return {C({T(1)}, T(0)),                              //
            C({T(54.598150033)}, T(4)).FloatComp(0.002),  //

            E({T::Lowest()}, error_msg()), E({T(0)}, error_msg()), E({-T(0)}, error_msg())};
}
INSTANTIATE_TEST_SUITE_P(  //
    Log,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kLog),
                     testing::ValuesIn(Concat(LogCases<AFloat>(),  //
                                              LogCases<f32>(),
                                              LogCases<f16>()))));
template <typename T>
std::vector<Case> LogF16Cases() {
    return {
        C({T::Highest()}, T(11.085938)).FloatComp(),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    LogF16,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kLog),
                     testing::ValuesIn(LogF16Cases<f16>())));
template <typename T>
std::vector<Case> LogF32Cases() {
    return {
        C({T::Highest()}, T(88.722839)).FloatComp(),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    LogF32,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kLog),
                     testing::ValuesIn(LogF32Cases<f32>())));

template <typename T>
std::vector<Case> LogAbstractCases() {
    return {
        C({T::Highest()}, T(709.78271)).FloatComp(),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    LogAbstract,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kLog),
                     testing::ValuesIn(LogAbstractCases<AFloat>())));

template <typename T>
std::vector<Case> Log2Cases() {
    auto error_msg = [] { return "12:34 error: log2 must be called with a value > 0"; };
    return {
        C({T(1)}, T(0)),  //
        C({T(4)}, T(2)),  //

        E({T::Lowest()}, error_msg()),
        E({T(0)}, error_msg()),
        E({-T(0)}, error_msg()),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Log2,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kLog2),
                     testing::ValuesIn(Concat(Log2Cases<AFloat>(),  //
                                              Log2Cases<f32>(),
                                              Log2Cases<f16>()))));
template <typename T>
std::vector<Case> Log2F16Cases() {
    return {
        C({T::Highest()}, T(15.9922)).FloatComp(),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Log2F16,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kLog2),
                     testing::ValuesIn(Log2F16Cases<f16>())));
template <typename T>
std::vector<Case> Log2F32Cases() {
    return {
        C({T::Highest()}, T(128)).FloatComp(),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Log2F32,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kLog2),
                     testing::ValuesIn(Log2F32Cases<f32>())));
template <typename T>
std::vector<Case> Log2AbstractCases() {
    return {
        C({T::Highest()}, T(1024)).FloatComp(),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Log2Abstract,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kLog2),
                     testing::ValuesIn(Log2AbstractCases<AFloat>())));

template <typename T>
std::vector<Case> MaxCases() {
    return {
        C({T(0), T(0)}, T(0)),
        C({T(0), T::Highest()}, T::Highest()),
        C({T::Lowest(), T(0)}, T(0)),
        C({T::Highest(), T::Lowest()}, T::Highest()),
        C({T::Highest(), T::Highest()}, T::Highest()),
        C({T::Lowest(), T::Lowest()}, T::Lowest()),

        // Vector tests
        C({Vec(T(0), T(0)), Vec(T(0), T(42))}, Vec(T(0), T(42))),
        C({Vec(T::Lowest(), T(0)), Vec(T(0), T::Lowest())}, Vec(T(0), T(0))),
        C({Vec(T::Lowest(), T::Highest()), Vec(T::Highest(), T::Lowest())},
          Vec(T::Highest(), T::Highest())),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Max,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kMax),
                     testing::ValuesIn(Concat(MaxCases<AInt>(),  //
                                              MaxCases<i32>(),
                                              MaxCases<u32>(),
                                              MaxCases<AFloat>(),
                                              MaxCases<f32>(),
                                              MaxCases<f16>()))));

template <typename T>
std::vector<Case> MinCases() {
    return {C({T(0), T(0)}, T(0)),                //
            C({T(0), T(42)}, T(0)),               //
            C({T::Lowest(), T(0)}, T::Lowest()),  //
            C({T(0), T::Highest()}, T(0)),        //
            C({T::Highest(), T::Lowest()}, T::Lowest()),
            C({T::Highest(), T::Highest()}, T::Highest()),
            C({T::Lowest(), T::Lowest()}, T::Lowest()),

            // Vector tests
            C({Vec(T(0), T(0)), Vec(T(0), T(42))}, Vec(T(0), T(0))),
            C({Vec(T::Lowest(), T(0), T(1)), Vec(T(0), T(42), T::Highest())},
              Vec(T::Lowest(), T(0), T(1)))};
}
INSTANTIATE_TEST_SUITE_P(  //
    Min,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kMin),
                     testing::ValuesIn(Concat(MinCases<AInt>(),  //
                                              MinCases<i32>(),
                                              MinCases<u32>(),
                                              MinCases<AFloat>(),
                                              MinCases<f32>(),
                                              MinCases<f16>()))));

template <typename T>
std::vector<Case> MixCases() {
    auto r = std::vector<Case>{
        C({T(0), T(1), T(0)}, T(0)),                         //
        C({T(0), T(1), T(1)}, T(1)),                         //
        C({T(0), T(1), T(2)}, T(2)),                         //
        C({T(0), T(1), T::Highest()}, T::Highest()),         //
        C({T::Lowest(), T::Highest(), T(1)}, T::Highest()),  //
        C({T::Lowest(), T::Highest(), T(0)}, T::Lowest()),   //
        C({T(0), T(1), T(0.25)}, T(0.25)),                   //
        C({T(0), T(1), T(0.5)}, T(0.5)),                     //
        C({T(0), T(1), T(0.75)}, T(0.75)),                   //
        C({T(0), T(1000), T(0.25)}, T(250)),                 //
        C({T(0), T(1000), T(0.5)}, T(500)),                  //
        C({T(0), T(1000), T(0.75)}, T(750)),                 //
        // Swap e1 and e2//
        C({T(1), T(0), T(0)}, T(1)),                         //
        C({T(1), T(0), T(1)}, T(0)),                         //
        C({T(1), T(0), T(2)}, T(-1)),                        //
        C({T::Highest(), T::Lowest(), T(1)}, T::Lowest()),   //
        C({T::Highest(), T::Lowest(), T(0)}, T::Highest()),  //
        C({T(1), T(0), T(0.25)}, T(0.75)),                   //
        C({T(1), T(0), T(0.5)}, T(0.5)),                     //
        C({T(1), T(0), T(0.75)}, T(0.25)),                   //
        C({T(1000), T(0), T(0.25)}, T(750)),                 //
        C({T(1000), T(0), T(0.5)}, T(500)),                  //
        C({T(1000), T(0), T(0.75)}, T(250)),

        // mix(vec, vec, vec) cases
        C({Vec(T(0), T(0), T(0)),  //
           Vec(T(1), T(1), T(1)),  //
           Vec(T(0), T(1), T(2))},
          Vec(T(0), T(1), T(2))),

        // mix(vec, vec, scalar) cases
        C({Vec(T(0), T(1), T(0)),     //
           Vec(T(1), T(0), T(1000)),  //
           Val(T(0.25))},
          Vec(T(0.25), T(0.75), T(250))),
    };
    // Can't interpolate lowest value for f16 because (1 - lowest) is not representable as f16.
    if constexpr (!std::is_same_v<T, f16>) {
        ConcatInto(r, std::vector<Case>{
                          C({T(0), T(1), T::Lowest()}, T::Lowest()),
                          C({T(1), T(0), T::Highest()}, T::Lowest()),
                      });
    }

    auto error_msg = [](auto a, const char* op, auto b) {
        return "12:34 error: " + OverflowErrorMessage(a, op, b) + R"(
12:34 note: when calculating mix)";
    };
    auto kLargeValue = T{T::Highest() / 2};
    // Test f16 separately as it overflows for a different reason at the boundary inputs.
    // Specifically, (1 - lowest) fails for f16 because the result is not representable.
    if constexpr (!std::is_same_v<T, f16>) {
        ConcatInto(  //
            r,
            std::vector<Case>{
                E({T(0), T::Highest(), T::Highest()}, error_msg(T::Highest(), "*", T::Highest())),
                E({T(0), T::Lowest(), T::Lowest()}, error_msg(T::Lowest(), "*", T::Lowest())),
                E({T::Highest(), T(0), T::Lowest()}, error_msg(T::Highest(), "*", T::Highest())),
                E({-kLargeValue, kLargeValue, T(2)},
                  error_msg(T{-kLargeValue * T(1 - 2)}, "+", T{kLargeValue * T(2)})),
            });
    } else {
        ConcatInto(  //
            r,
            std::vector<Case>{
                E({T(0), T::Highest(), T::Highest()}, error_msg(T::Highest(), "*", T::Highest())),
                E({T(0), T::Lowest(), T::Lowest()}, error_msg(T(1), "-", T::Lowest())),
                E({T::Highest(), T(0), T::Lowest()}, error_msg(T(1), "-", T::Lowest())),
                E({-kLargeValue, kLargeValue, T(2)},
                  error_msg(T{-kLargeValue * T(1 - 2)}, "+", T{kLargeValue * T(2)})),
            });
    }

    return r;
}
INSTANTIATE_TEST_SUITE_P(  //
    Mix,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kMix),
                     testing::ValuesIn(Concat(MixCases<AFloat>(),  //
                                              MixCases<f32>(),     //
                                              MixCases<f16>()))));

template <typename T>
std::vector<Case> ModfCases() {
    return {
        // Scalar tests
        //  in     fract    whole
        C({T(0.0)}, {T(0.0), T(0.0)}),              //
        C({T(1.0)}, {T(0.0), T(1.0)}),              //
        C({T(2.0)}, {T(0.0), T(2.0)}),              //
        C({T(1.5)}, {T(0.5), T(1.0)}),              //
        C({T(4.25)}, {T(0.25), T(4.0)}),            //
        C({T(-1.0)}, {T(0.0), T(-1.0)}),            //
        C({T(-2.0)}, {T(0.0), T(-2.0)}),            //
        C({T(-1.5)}, {T(-0.5), T(-1.0)}),           //
        C({T(-4.25)}, {T(-0.25), T(-4.0)}),         //
        C({T::Lowest()}, {T(0.0), T::Lowest()}),    //
        C({T::Highest()}, {T(0.0), T::Highest()}),  //

        // Vector tests
        //         in                 fract                    whole
        C({Vec(T(0.0), T(0.0))}, {Vec(T(0.0), T(0.0)), Vec(T(0.0), T(0.0))}),
        C({Vec(T(1.0), T(2.0))}, {Vec(T(0.0), T(0.0)), Vec(T(1), T(2))}),
        C({Vec(T(-2.0), T(1.0))}, {Vec(T(0.0), T(0.0)), Vec(T(-2), T(1))}),
        C({Vec(T(1.5), T(-2.25))}, {Vec(T(0.5), T(-0.25)), Vec(T(1.0), T(-2.0))}),
        C({Vec(T::Lowest(), T::Highest())}, {Vec(T(0.0), T(0.0)), Vec(T::Lowest(), T::Highest())}),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Modf,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kModf),
                     testing::ValuesIn(Concat(ModfCases<AFloat>(),  //
                                              ModfCases<f32>(),     //
                                              ModfCases<f16>()))));

template <typename T>
std::vector<Case> NormalizeCases() {
    auto error_msg = [&](auto a) {
        return "12:34 error: " + OverflowErrorMessage(a, "*", a) + R"(
12:34 note: when calculating normalize)";
    };

    return {
        C({Vec(T(2), T(4), T(2))}, Vec(T(0.4082482905), T(0.8164965809), T(0.4082482905)))
            .FloatComp(),

        C({Vec(T(2), T(0), T(0))}, Vec(T(1), T(0), T(0))),
        C({Vec(T(0), T(2), T(0))}, Vec(T(0), T(1), T(0))),
        C({Vec(T(0), T(0), T(2))}, Vec(T(0), T(0), T(1))),
        C({Vec(-T(2), T(0), T(0))}, Vec(-T(1), T(0), T(0))),
        C({Vec(T(0), -T(2), T(0))}, Vec(T(0), -T(1), T(0))),
        C({Vec(T(0), T(0), -T(2))}, Vec(T(0), T(0), -T(1))),

        E({Vec(T(0), T(0), T(0))}, "12:34 error: zero length vector can not be normalized"),
        E({Vec(T::Highest(), T::Highest(), T::Highest())}, error_msg(T::Highest())),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Normalize,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kNormalize),
                     testing::ValuesIn(Concat(NormalizeCases<AFloat>(),  //
                                              NormalizeCases<f32>(),     //
                                              NormalizeCases<f16>()))));

std::vector<Case> Pack4x8snormCases() {
    return {
        C({Vec(f32(0), f32(0), f32(0), f32(0))}, Val(u32(0x0000'0000))),
        C({Vec(f32(0), f32(0), f32(0), f32(-1))}, Val(u32(0x8100'0000))),
        C({Vec(f32(0), f32(0), f32(0), f32(1))}, Val(u32(0x7f00'0000))),
        C({Vec(f32(0), f32(0), f32(-1), f32(0))}, Val(u32(0x0081'0000))),
        C({Vec(f32(0), f32(1), f32(0), f32(0))}, Val(u32(0x0000'7f00))),
        C({Vec(f32(-1), f32(0), f32(0), f32(0))}, Val(u32(0x0000'0081))),
        C({Vec(f32(1), f32(-1), f32(1), f32(-1))}, Val(u32(0x817f'817f))),
        C({Vec(f32::Highest(), f32(-0.5), f32(0.5), f32::Lowest())}, Val(u32(0x8140'c17f))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Pack4x8snorm,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kPack4X8Snorm),
                     testing::ValuesIn(Pack4x8snormCases())));

std::vector<Case> Pack4x8unormCases() {
    return {
        C({Vec(f32(0), f32(0), f32(0), f32(0))}, Val(u32(0x0000'0000))),
        C({Vec(f32(0), f32(0), f32(0), f32(1))}, Val(u32(0xff00'0000))),
        C({Vec(f32(0), f32(0), f32(1), f32(0))}, Val(u32(0x00ff'0000))),
        C({Vec(f32(0), f32(1), f32(0), f32(0))}, Val(u32(0x0000'ff00))),
        C({Vec(f32(1), f32(0), f32(0), f32(0))}, Val(u32(0x0000'00ff))),
        C({Vec(f32(1), f32(0), f32(1), f32(0))}, Val(u32(0x00ff'00ff))),
        C({Vec(f32::Highest(), f32(0), f32(0.5), f32::Lowest())}, Val(u32(0x0080'00ff))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Pack4x8unorm,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kPack4X8Unorm),
                     testing::ValuesIn(Pack4x8unormCases())));

std::vector<Case> Pack2x16floatCases() {
    return {
        C({Vec(f32(f16::Lowest()), f32(f16::Highest()))}, Val(u32(0x7bff'fbff))),
        C({Vec(f32(1), f32(-1))}, Val(u32(0xbc00'3c00))),
        C({Vec(f32(0), f32(0))}, Val(u32(0x0000'0000))),
        C({Vec(f32(10), f32(-10.5))}, Val(u32(0xc940'4900))),

        E({Vec(f32(0), f32::Highest())},
          "12:34 error: value 340282346638528859811704183484516925440.0 cannot be "
          "represented as 'f16'"),
        E({Vec(f32::Lowest(), f32(0))},
          "12:34 error: value -340282346638528859811704183484516925440.0 cannot be "
          "represented as 'f16'"),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Pack2x16float,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kPack2X16Float),
                     testing::ValuesIn(Pack2x16floatCases())));

std::vector<Case> Pack2x16snormCases() {
    return {
        C({Vec(f32(0), f32(0))}, Val(u32(0x0000'0000))),
        C({Vec(f32(0), f32(-1))}, Val(u32(0x8001'0000))),
        C({Vec(f32(0), f32(1))}, Val(u32(0x7fff'0000))),
        C({Vec(f32(-1), f32(0))}, Val(u32(0x0000'8001))),
        C({Vec(f32(1), f32(0))}, Val(u32(0x0000'7fff))),
        C({Vec(f32(1), f32(-1))}, Val(u32(0x8001'7fff))),
        C({Vec(f32::Highest(), f32::Lowest())}, Val(u32(0x8001'7fff))),
        C({Vec(f32(-0.5), f32(0.5))}, Val(u32(0x4000'c001))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Pack2x16snorm,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kPack2X16Snorm),
                     testing::ValuesIn(Pack2x16snormCases())));

std::vector<Case> Pack2x16unormCases() {
    return {
        C({Vec(f32(0), f32(1))}, Val(u32(0xffff'0000))),
        C({Vec(f32(1), f32(0))}, Val(u32(0x0000'ffff))),
        C({Vec(f32(0.5), f32(0))}, Val(u32(0x0000'8000))),
        C({Vec(f32::Highest(), f32::Lowest())}, Val(u32(0x0000'ffff))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Pack2x16unorm,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kPack2X16Unorm),
                     testing::ValuesIn(Pack2x16unormCases())));

template <typename T>
std::vector<Case> PowCases() {
    auto error_msg = [](auto base, auto exp) {
        return "12:34 error: " + OverflowErrorMessage(base, "^", exp);
    };
    return {
        C({T(0), T(1)}, T(0)),          //
        C({T(0), T::Highest()}, T(0)),  //
        C({T(1), T(1)}, T(1)),          //
        C({T(1), T::Lowest()}, T(1)),   //
        C({T(2), T(2)}, T(4)),          //
        C({T(2), T(3)}, T(8)),          //
        // Positive base, negative exponent
        C({T(1), T::Highest()}, T(1)),  //
        C({T(1), -T(1)}, T(1)),         //
        C({T(2), -T(2)}, T(0.25)),      //
        C({T(2), -T(3)}, T(0.125)),     //
        // Decimal values
        C({T(2.5), T(3)}, T(15.625)),                      //
        C({T(2), T(3.5)}, T(11.313708498)).FloatComp(),    //
        C({T(2.5), T(3.5)}, T(24.705294220)).FloatComp(),  //
        C({T(2), -T(3.5)}, T(0.0883883476)).FloatComp(),   //

        // Vector tests
        C({Vec(T(0), T(1), T(2)), Vec(T(2), T(2), T(2))}, Vec(T(0), T(1), T(4))),
        C({Vec(T(2), T(2), T(2)), Vec(T(2), T(3), T(4))}, Vec(T(4), T(8), T(16))),

        // Error if base < 0
        E({-T(1), T(1)}, error_msg(-T(1), T(1))),
        E({-T(1), T::Highest()}, error_msg(-T(1), T::Highest())),
        E({T::Lowest(), T(1)}, error_msg(T::Lowest(), T(1))),
        E({T::Lowest(), T::Highest()}, error_msg(T::Lowest(), T::Highest())),
        E({T::Lowest(), T::Lowest()}, error_msg(T::Lowest(), T::Lowest())),

        // Error if base == 0 and exp <= 0
        E({T(0), T(0)}, error_msg(T(0), T(0))),
        E({T(0), -T(1)}, error_msg(T(0), -T(1))),
        E({T(0), T::Lowest()}, error_msg(T(0), T::Lowest())),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Pow,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kPow),
                     testing::ValuesIn(Concat(PowCases<AFloat>(),  //
                                              PowCases<f32>(),     //
                                              PowCases<f16>()))));

template <typename T>
std::vector<Case> ReverseBitsCases() {
    using B = BitValues<T>;
    return {
        C({T(0)}, T(0)),

        C({B::Lsh(1, 0)}, B::Lsh(1, 31)),  //
        C({B::Lsh(1, 1)}, B::Lsh(1, 30)),  //
        C({B::Lsh(1, 2)}, B::Lsh(1, 29)),  //
        C({B::Lsh(1, 3)}, B::Lsh(1, 28)),  //
        C({B::Lsh(1, 4)}, B::Lsh(1, 27)),  //
        //...
        C({B::Lsh(1, 27)}, B::Lsh(1, 4)),  //
        C({B::Lsh(1, 28)}, B::Lsh(1, 3)),  //
        C({B::Lsh(1, 29)}, B::Lsh(1, 2)),  //
        C({B::Lsh(1, 30)}, B::Lsh(1, 1)),  //
        C({B::Lsh(1, 31)}, B::Lsh(1, 0)),  //

        C({/**/ T(0b00010001000100010000000000000000)},
          /* */ T(0b00000000000000001000100010001000)),

        C({/**/ T(0b00011000000110000000000000000000)},
          /* */ T(0b00000000000000000001100000011000)),

        C({/**/ T(0b00000100000000001111111111111111)},
          /* */ T(0b11111111111111110000000000100000)),

        C({/**/ T(0b10010101111000110000011111101010)},
          /* */ T(0b01010111111000001100011110101001)),

        // Vector tests
        C({/**/ Vec(T(0b00010001000100010000000000000000),  //
                    T(0b00011000000110000000000000000000),  //
                    T(0b00000000000000001111111111111111))},
          /* */ Vec(T(0b00000000000000001000100010001000),  //
                    T(0b00000000000000000001100000011000),  //
                    T(0b11111111111111110000000000000000))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    ReverseBits,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kReverseBits),
                     testing::ValuesIn(Concat(ReverseBitsCases<i32>(),  //
                                              ReverseBitsCases<u32>()))));

template <typename T>
std::vector<Case> ReflectCases() {
    auto pos_y = Vec(T(0), T(1), T(0));
    auto neg_y = Vec(T(0), -T(1), T(0));
    auto pos_large_y = Vec(T(0), T(10000), T(0));
    auto neg_large_y = Vec(T(0), -T(10000), T(0));

    auto cos_45 = T(0.70710678118654752440084436210485);
    auto pos_xyz = Vec(cos_45, cos_45, cos_45);

    auto r = std::vector<Case>{
        C({Vec(T(1), -T(1), T(0)), pos_y}, Vec(T(1), T(1), T(0))),
        C({Vec(T(24), -T(42), T(0)), pos_y}, Vec(T(24), T(42), T(0))),
        // Flipping reflection vector doesn't change the result
        C({Vec(T(1), -T(1), T(0)), neg_y}, Vec(T(1), T(1), T(0))),
        C({Vec(T(24), -T(42), T(0)), neg_y}, Vec(T(24), T(42), T(0))),
        // Parallel input and reflection vectors: result is negation of input
        C({pos_y, pos_y}, neg_y),
        C({neg_y, pos_y}, pos_y),
        C({pos_large_y, pos_y}, neg_large_y),
        C({neg_large_y, pos_y}, pos_large_y),
        // Input axis vectors reflected by normalized(vec(1,1,1)) vector.
        C({Vec(T(1), T(0), T(0)), pos_xyz}, Vec(T(0), -T(1), -T(1))).FloatComp(0.02),
        C({Vec(T(0), T(1), T(0)), pos_xyz}, Vec(-T(1), T(0), -T(1))).FloatComp(0.02),
        C({Vec(T(0), T(0), T(1)), pos_xyz}, Vec(-T(1), -T(1), T(0))).FloatComp(0.02),
        C({Vec(-T(1), T(0), T(0)), pos_xyz}, Vec(T(0), T(1), T(1))).FloatComp(0.02),
        C({Vec(T(0), -T(1), T(0)), pos_xyz}, Vec(T(1), T(0), T(1))).FloatComp(0.02),
        C({Vec(T(0), T(0), -T(1)), pos_xyz}, Vec(T(1), T(1), T(0))).FloatComp(0.02),
    };

    auto error_msg = [](auto a, const char* op, auto b) {
        return "12:34 error: " + OverflowErrorMessage(a, op, b) + R"(
12:34 note: when calculating reflect)";
    };
    ConcatInto(  //
        r, std::vector<Case>{
               // Overflow the dot product operation
               E({Vec(T::Highest(), T::Highest(), T(0)), Vec(T(1), T(1), T(0))},
                 error_msg(T::Highest(), "+", T::Highest())),
               E({Vec(T::Lowest(), T::Lowest(), T(0)), Vec(T(1), T(1), T(0))},
                 error_msg(T::Lowest(), "+", T::Lowest())),
           });

    return r;
}
INSTANTIATE_TEST_SUITE_P(  //
    Reflect,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kReflect),
                     testing::ValuesIn(Concat(ReflectCases<AFloat>(),  //
                                              ReflectCases<f32>(),     //
                                              ReflectCases<f16>()))));

template <typename T>
std::vector<Case> RefractCases() {
    // Returns "eta" (Greek letter) that denotes the ratio of indices of refraction for the input
    // and output vector angles from the normal vector.
    auto eta = [](auto angle1, auto angle2) {
        // Snell's law: sin(angle1) / sin(angle2) == n2 / n1
        // We want the ratio of n1 to n2, so sin(angle2) / sin(angle1)
        auto angle1_rads = T(angle1) * kPi<T> / T(180);
        auto angle2_rads = T(angle2) * kPi<T> / T(180);
        return T(std::sin(angle2_rads) / std::sin(angle1_rads));
    };

    auto zero = Vec(T(0), T(0), T(0));
    auto pos_y = Vec(T(0), T(1), T(0));
    auto neg_y = Vec(T(0), -T(1), T(0));
    auto pos_x = Vec(T(1), T(0), T(0));
    auto neg_x = Vec(-T(1), T(0), T(0));
    auto cos_45 = T(0.70710678118654752440084436210485);
    auto cos_30 = T(0.86602540378443864676372317075294);
    auto down_right = Vec(T(cos_45), -T(cos_45), T(0));
    auto up_right = Vec(T(cos_45), T(cos_45), T(0));

    auto eps = 0.001;
    if constexpr (std::is_same_v<T, f16>) {
        eps = 0.1;
    }

    auto r = std::vector<Case>{
        // e3 (eta) == 1, no refraction, so input is same as output
        C({down_right, pos_y, Val(T(1))}, down_right),
        C({neg_y, pos_y, Val(T(1))}, neg_y),
        // Varying etas
        C({down_right, pos_y, Val(eta(45, 45))}, down_right).FloatComp(eps),  // e3 == 1
        C({down_right, pos_y, Val(eta(45, 30))}, Vec(T(0.5), -T(cos_30), T(0))).FloatComp(eps),
        C({down_right, pos_y, Val(eta(45, 60))}, Vec(T(cos_30), -T(0.5), T(0))).FloatComp(eps),
        C({down_right, pos_y, Val(eta(45, 90))}, Vec(T(1), T(0), T(0))).FloatComp(eps),
        // Flip input and normal, same result
        C({up_right, neg_y, Val(eta(45, 45))}, up_right).FloatComp(eps),  // e3 == 1
        C({up_right, neg_y, Val(eta(45, 30))}, Vec(T(0.5), T(cos_30), T(0))).FloatComp(eps),
        C({up_right, neg_y, Val(eta(45, 60))}, Vec(T(cos_30), T(0.5), T(0))).FloatComp(eps),
        C({up_right, neg_y, Val(eta(45, 90))}, Vec(T(1), T(0), T(0))).FloatComp(eps),
        // Flip only normal, result is flipped
        C({down_right, neg_y, Val(eta(45, 45))}, up_right).FloatComp(eps),  // e3 == 1
        C({down_right, neg_y, Val(eta(45, 30))}, Vec(T(0.5), T(cos_30), T(0))).FloatComp(eps),
        C({down_right, neg_y, Val(eta(45, 60))}, Vec(T(cos_30), T(0.5), T(0))).FloatComp(eps),
        C({down_right, neg_y, Val(eta(45, 90))}, Vec(T(1), T(0), T(0))).FloatComp(eps),

        // If k < 0.0, returns the refraction vector 0.0
        C({down_right, pos_y, Val(T(2))}, zero).FloatComp(eps),

        // A few more with a different normal (e2)
        C({down_right, neg_x, Val(eta(45, 45))}, down_right).FloatComp(eps),  // e3 == 1
        C({down_right, neg_x, Val(eta(45, 30))}, Vec(cos_30, -T(0.5), T(0))).FloatComp(eps),
        C({down_right, neg_x, Val(eta(45, 60))}, Vec(T(0.5), -T(cos_30), T(0))).FloatComp(eps),
    };

    auto error_msg = [](auto a, const char* op, auto b) {
        return "12:34 error: " + OverflowErrorMessage(a, op, b) + R"(
12:34 note: when calculating refract)";
    };
    ConcatInto(  //
        r,
        std::vector<Case>{
            // Overflow the dot product operation
            E({Vec(T::Highest(), T::Highest(), T(0)), Vec(T(1), T(1), T(0)), Val(T(1))},
              error_msg(T::Highest(), "+", T::Highest())),
            E({Vec(T::Lowest(), T::Lowest(), T(0)), Vec(T(1), T(1), T(0)), Val(T(1))},
              error_msg(T::Lowest(), "+", T::Lowest())),
            // Overflow the k^2 operation
            E({down_right, pos_y, Val(T::Highest())}, error_msg(T::Highest(), "*", T::Highest())),
        });
    ConcatIntoIf<std::is_same_v<T, f32>>(  //
        r, std::vector<Case>{
               // Overflow the final multiply by e2 operation
               // From https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=58526
               E({Vec(T(-2.22218755e-15), T(0)), Vec(T(-198225753253481323832809619456.0), T(0)),
                  Val(T(40.0313720703125))},
                 error_msg(T(35267222007971840.0), "*", T(-198225753253481323832809619456.0))),
           });

    return r;
}
INSTANTIATE_TEST_SUITE_P(  //
    Refract,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kRefract),
                     testing::ValuesIn(Concat(RefractCases<AFloat>(),  //
                                              RefractCases<f32>(),     //
                                              RefractCases<f16>()))));

template <typename T>
std::vector<Case> RadiansCases() {
    return {
        C({T(0)}, T(0)),                         //
        C({-T(0)}, -T(0)),                       //
        C({T(40)}, T(0.69813168)).FloatComp(),   //
        C({-T(90)}, -T(1.5707964)).FloatComp(),  //
        C({T(90)}, T(1.5707964)).FloatComp(),    //
        C({T(360)}, T(6.2831855)).FloatComp(),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Radians,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kRadians),
                     testing::ValuesIn(Concat(RadiansCases<AFloat>(),  //
                                              RadiansCases<f32>()))));

template <typename T>
std::vector<Case> RadiansF16Cases() {
    return {
        C({T(0)}, T(0)),                         //
        C({-T(0)}, -T(0)),                       //
        C({T(40)}, T(0.69726562)).FloatComp(),   //
        C({-T(90)}, -T(1.5693359)).FloatComp(),  //
        C({T(90)}, T(1.5693359)).FloatComp(),    //
        C({T(360)}, T(6.2773438)).FloatComp(),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    RadiansF16,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kRadians),
                     testing::ValuesIn(RadiansF16Cases<f16>())));

template <typename T>
std::vector<Case> RoundCases() {
    return {
        C({T(0.0)}, T(0.0)),      //
        C({-T(0.0)}, -T(0.0)),    //
        C({T(1.5)}, T(2.0)),      //
        C({T(2.5)}, T(2.0)),      //
        C({T(2.4)}, T(2.0)),      //
        C({T(2.6)}, T(3.0)),      //
        C({T(1.49999)}, T(1.0)),  //
        C({T(1.50001)}, T(2.0)),  //
        C({-T(1.5)}, -T(2.0)),    //
        C({-T(2.5)}, -T(2.0)),    //
        C({-T(2.6)}, -T(3.0)),    //
        C({-T(2.4)}, -T(2.0)),    //

        // Vector tests
        C({Vec(T(0.0), T(1.5), T(2.5))}, Vec(T(0.0), T(2.0), T(2.0))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Round,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kRound),
                     testing::ValuesIn(Concat(RoundCases<AFloat>(),  //
                                              RoundCases<f32>(),
                                              RoundCases<f16>()))));

template <typename T>
std::vector<Case> SaturateCases() {
    return {
        C({T(0)}, T(0)),
        C({T(1)}, T(1)),
        C({T::Lowest()}, T(0)),
        C({T::Highest()}, T(1)),

        // Vector tests
        C({Vec(T(0), T(0))},                       //
          Vec(T(0), T(0))),                        //
        C({Vec(T(1), T(1))},                       //
          Vec(T(1), T(1))),                        //
        C({Vec(T::Lowest(), T(0), T::Highest())},  //
          Vec(T(0), T(0), T(1))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Saturate,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kSaturate),
                     testing::ValuesIn(Concat(SaturateCases<AFloat>(),  //
                                              SaturateCases<f32>(),
                                              SaturateCases<f16>()))));

template <typename T>
std::vector<Case> SelectCases() {
    return {
        C({Val(T{1}), Val(T{2}), Val(false)}, Val(T{1})),
        C({Val(T{1}), Val(T{2}), Val(true)}, Val(T{2})),

        C({Val(T{2}), Val(T{1}), Val(false)}, Val(T{2})),
        C({Val(T{2}), Val(T{1}), Val(true)}, Val(T{1})),

        C({Vec(T{1}, T{2}), Vec(T{3}, T{4}), Vec(false, false)}, Vec(T{1}, T{2})),
        C({Vec(T{1}, T{2}), Vec(T{3}, T{4}), Vec(false, true)}, Vec(T{1}, T{4})),
        C({Vec(T{1}, T{2}), Vec(T{3}, T{4}), Vec(true, false)}, Vec(T{3}, T{2})),
        C({Vec(T{1}, T{2}), Vec(T{3}, T{4}), Vec(true, true)}, Vec(T{3}, T{4})),

        C({Vec(T{1}, T{1}, T{2}, T{2}),     //
           Vec(T{2}, T{2}, T{1}, T{1}),     //
           Vec(false, true, false, true)},  //
          Vec(T{1}, T{2}, T{2}, T{1})),     //
    };
}
static std::vector<Case> SelectBoolCases() {
    return {
        C({Val(true), Val(false), Val(false)}, Val(true)),
        C({Val(true), Val(false), Val(true)}, Val(false)),

        C({Val(false), Val(true), Val(true)}, Val(true)),
        C({Val(false), Val(true), Val(false)}, Val(false)),

        C({Vec(true, true, false, false),   //
           Vec(false, false, true, true),   //
           Vec(false, true, true, false)},  //
          Vec(true, false, true, false)),   //
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Select,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kSelect),
                     testing::ValuesIn(Concat(SelectCases<AInt>(),  //
                                              SelectCases<i32>(),
                                              SelectCases<u32>(),
                                              SelectCases<AFloat>(),
                                              SelectCases<f32>(),
                                              SelectCases<f16>(),
                                              SelectBoolCases()))));

template <typename T>
std::vector<Case> SignCases() {
    std::vector<Case> cases = {
        C({T(0)}, T(0)),
        C({-T(0)}, T(0)),

        C({-T(1)}, -T(1)),
        C({-T(10)}, -T(1)),
        C({-T(100)}, -T(1)),
        C({T(1)}, T(1)),
        C({T(10)}, T(1)),
        C({T(100)}, T(1)),

        C({T::Highest()}, T(1.0)),
        C({T::Lowest()}, -T(1.0)),

        // Vector tests
        C({Vec(T::Highest(), T::Lowest())}, Vec(T(1.0), -T(1.0))),
    };

    ConcatIntoIf<IsFloatingPoint<T>>(
        cases, std::vector<Case>{
                   C({-T(0.5)}, -T(1)),
                   C({T(0.5)}, T(1)),
                   C({Vec(-T(0.5), T(0), T(0.5))}, Vec(-T(1.0), T(0.0), T(1.0))),
               });

    return cases;
}
INSTANTIATE_TEST_SUITE_P(  //
    Sign,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kSign),
                     testing::ValuesIn(Concat(SignCases<AInt>(),  //
                                              SignCases<i32>(),
                                              SignCases<AFloat>(),
                                              SignCases<f32>(),
                                              SignCases<f16>()))));

template <typename T>
std::vector<Case> SinCases() {
    return {
        C({-T(0)}, -T(0)),
        C({T(0)}, T(0)),
        C({T(0.75)}, T(0.68163876)).FloatComp(),
        C({-T(0.75)}, -T(0.68163876)).FloatComp(),

        // Vector test
        C({Vec(T(0), -T(0), T(0.75))}, Vec(T(0), -T(0), T(0.68163876))).FloatComp(),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Sin,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kSin),
                     testing::ValuesIn(Concat(SinCases<AFloat>(),  //
                                              SinCases<f32>(),
                                              SinCases<f16>()))));

template <typename T>
std::vector<Case> SinhCases() {
    auto error_msg = [](auto a) {
        return "12:34 error: " + OverflowErrorMessage(a, FriendlyName<decltype(a)>());
    };
    return {
        C({T(0)}, T(0)),
        C({-T(0)}, -T(0)),
        C({T(1)}, T(1.1752012)).FloatComp(),
        C({T(-1)}, -T(1.1752012)).FloatComp(),

        // Vector tests
        C({Vec(T(0), -T(0), T(1))}, Vec(T(0), -T(0), T(1.1752012))).FloatComp(),

        E({T(10000)}, error_msg(T::Inf())),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Sinh,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kSinh),
                     testing::ValuesIn(Concat(SinhCases<AFloat>(),  //
                                              SinhCases<f32>(),
                                              SinhCases<f16>()))));

template <typename T>
std::vector<Case> SmoothstepCases() {
    auto error_overflow = [](auto a, const char* op, auto b) {
        return "12:34 error: " + OverflowErrorMessage(a, op, b) + R"(
12:34 note: when calculating smoothstep)";
    };
    auto error_low_equal_high = [](auto a, auto b) {
        StringStream ss;
        ss << "12:34 error: smoothstep called with 'low' (" << a << ") equal to 'high' (" << b
           << ")";
        return ss.str();
    };
    return {
        // t == 0
        C({T(4), T(6), T(2)}, T(0)),
        // t == 1
        C({T(4), T(6), T(8)}, T(1)),
        // t == .5
        C({T(4), T(6), T(5)}, T(.5)),

        // Vector tests
        C({Vec(T(4), T(4)), Vec(T(6), T(6)), Vec(T(2), T(8))}, Vec(T(0), T(1))),

        // `x - low` underflows
        E({T(T::Highest() * T(0.5)), T::Highest(), T::Lowest()},
          error_overflow(T::Lowest(), "-", T(T::Highest() * T(0.5)))),
        // `high - low` underflows
        E({T::Lowest(), T::Highest(), T(0)}, error_overflow(T::Highest(), "-", T::Lowest())),
        // low == high
        E({T(0), T(0), T(0)}, error_low_equal_high(T(0), T(0))),
        E({T(1), T(1), T(5)}, error_low_equal_high(T(1), T(1))),
        // low > high
        C({T(1), T(0), T(5)}, T(0)),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Smoothstep,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kSmoothstep),
                     testing::ValuesIn(Concat(SmoothstepCases<AFloat>(),  //
                                              SmoothstepCases<f32>(),
                                              SmoothstepCases<f16>()))));

template <typename T>
std::vector<Case> StepCases() {
    return {
        C({T(0), T(0)}, T(1.0)),
        C({T(0), T(0.5)}, T(1.0)),
        C({T(0.5), T(0)}, T(0.0)),
        C({T(1), T(0.5)}, T(0.0)),
        C({T(0.5), T(1)}, T(1.0)),
        C({T(1.5), T(1)}, T(0.0)),
        C({T(1), T(1.5)}, T(1.0)),
        C({T(-1), T(1)}, T(1.0)),
        C({T(-1), T(1)}, T(1.0)),
        C({T(1), T(-1)}, T(0.0)),
        C({T(-1), T(-1.5)}, T(0.0)),
        C({T(-1.5), T(-1)}, T(1.0)),
        C({T::Highest(), T::Lowest()}, T(0.0)),
        C({T::Lowest(), T::Highest()}, T(1.0)),

        // Vector tests
        C({Vec(T(0), T(0)), Vec(T(0), T(0))}, Vec(T(1.0), T(1.0))),
        C({Vec(T(-1), T(1)), Vec(T(0), T(0))}, Vec(T(1.0), T(0.0))),
        C({Vec(T::Highest(), T::Lowest()), Vec(T::Lowest(), T::Highest())}, Vec(T(0.0), T(1.0))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Step,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kStep),
                     testing::ValuesIn(Concat(StepCases<AFloat>(),  //
                                              StepCases<f32>(),
                                              StepCases<f16>()))));

template <typename T>
std::vector<Case> SqrtCases() {
    return {
        C({-T(0)}, -T(0)),  //
        C({T(0)}, T(0)),    //
        C({T(25)}, T(5)),

        // Vector tests
        C({Vec(T(25), T(100))}, Vec(T(5), T(10))),

        E({-T(25)}, "12:34 error: sqrt must be called with a value >= 0"),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Sqrt,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kSqrt),
                     testing::ValuesIn(Concat(SqrtCases<AFloat>(),  //
                                              SqrtCases<f32>(),
                                              SqrtCases<f16>()))));

template <typename T>
std::vector<Case> TanCases() {
    return {
        C({-T(0)}, -T(0)),
        C({T(0)}, T(0)),
        C({T(.75)}, T(0.9315964599)).FloatComp(),

        // Vector test
        C({Vec(T(0), -T(0), T(.75))}, Vec(T(0), -T(0), T(0.9315964599))).FloatComp(),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Tan,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kTan),
                     testing::ValuesIn(Concat(TanCases<AFloat>(),  //
                                              TanCases<f32>(),
                                              TanCases<f16>()))));

template <typename T>
std::vector<Case> TanhCases() {
    return {
        C({T(0)}, T(0)),
        C({-T(0)}, -T(0)),
        C({T(1)}, T(0.761594156)).FloatComp(),
        C({T(-1)}, -T(0.761594156)).FloatComp(),

        // Vector tests
        C({Vec(T(0), -T(0), T(1))}, Vec(T(0), -T(0), T(0.761594156))).FloatComp(),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Tanh,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kTanh),
                     testing::ValuesIn(Concat(TanhCases<AFloat>(),  //
                                              TanhCases<f32>(),
                                              TanhCases<f16>()))));

template <typename T>
std::vector<Case> TransposeCases() {
    return {
        // 2x2
        C({Mat({T(1), T(2)},    //
               {T(3), T(4)})},  //
          Mat({T(1), T(3)},     //
              {T(2), T(4)})),

        // 3x3
        C({Mat({T(1), T(2), T(3)},    //
               {T(4), T(5), T(6)},    //
               {T(7), T(8), T(9)})},  //
          Mat({T(1), T(4), T(7)},     //
              {T(2), T(5), T(8)},     //
              {T(3), T(6), T(9)})),

        // 4x4
        C({Mat({T(1), T(2), T(3), T(4)},        //
               {T(5), T(6), T(7), T(8)},        //
               {T(9), T(10), T(11), T(12)},     //
               {T(13), T(14), T(15), T(16)})},  //
          Mat({T(1), T(5), T(9), T(13)},        //
              {T(2), T(6), T(10), T(14)},       //
              {T(3), T(7), T(11), T(15)},       //
              {T(4), T(8), T(12), T(16)})),

        // 4x2
        C({Mat({T(1), T(2), T(3), T(4)},    //
               {T(5), T(6), T(7), T(8)})},  //
          Mat({T(1), T(5)},                 //
              {T(2), T(6)},                 //
              {T(3), T(7)},                 //
              {T(4), T(8)})),

        // 2x4
        C({Mat({T(1), T(2)},             //
               {T(3), T(4)},             //
               {T(5), T(6)},             //
               {T(7), T(8)})},           //
          Mat({T(1), T(3), T(5), T(7)},  //
              {T(2), T(4), T(6), T(8)})),

        // 3x2
        C({Mat({T(1), T(2), T(3)},    //
               {T(4), T(5), T(6)})},  //
          Mat({T(1), T(4)},           //
              {T(2), T(5)},           //
              {T(3), T(6)})),

        // 2x3
        C({Mat({T(1), T(2)},       //
               {T(3), T(4)},       //
               {T(5), T(6)})},     //
          Mat({T(1), T(3), T(5)},  //
              {T(2), T(4), T(6)})),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Transpose,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kTranspose),
                     testing::ValuesIn(Concat(TransposeCases<AFloat>(),  //
                                              TransposeCases<f32>(),
                                              TransposeCases<f16>()))));

template <typename T>
std::vector<Case> TruncCases() {
    return {C({T(0)}, T(0)),    //
            C({-T(0)}, -T(0)),  //
            C({T(1.5)}, T(1)),  //
            C({-T(1.5)}, -T(1)),

            // Vector tests
            C({Vec(T(0.0), T(1.5), -T(2.2))}, Vec(T(0), T(1), -T(2)))};
}
INSTANTIATE_TEST_SUITE_P(  //
    Trunc,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kTrunc),
                     testing::ValuesIn(Concat(TruncCases<AFloat>(),  //
                                              TruncCases<f32>(),
                                              TruncCases<f16>()))));

std::vector<Case> Unpack4x8snormCases() {
    return {
        C({Val(u32(0x0000'0000))}, Vec(f32(0), f32(0), f32(0), f32(0))),
        C({Val(u32(0x8100'0000))}, Vec(f32(0), f32(0), f32(0), f32(-1))),
        C({Val(u32(0x7f00'0000))}, Vec(f32(0), f32(0), f32(0), f32(1))),
        C({Val(u32(0x0081'0000))}, Vec(f32(0), f32(0), f32(-1), f32(0))),
        C({Val(u32(0x0000'7f00))}, Vec(f32(0), f32(1), f32(0), f32(0))),
        C({Val(u32(0x0000'0081))}, Vec(f32(-1), f32(0), f32(0), f32(0))),
        C({Val(u32(0x817f'817f))}, Vec(f32(1), f32(-1), f32(1), f32(-1))),
        C({Val(u32(0x816d'937f))},
          Vec(f32(1), f32(-0.8582677165354), f32(0.8582677165354), f32(-1))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Unpack4x8snorm,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kUnpack4X8Snorm),
                     testing::ValuesIn(Unpack4x8snormCases())));

std::vector<Case> Unpack4x8unormCases() {
    return {
        C({Val(u32(0x0000'0000))}, Vec(f32(0), f32(0), f32(0), f32(0))),
        C({Val(u32(0xff00'0000))}, Vec(f32(0), f32(0), f32(0), f32(1))),
        C({Val(u32(0x00ff'0000))}, Vec(f32(0), f32(0), f32(1), f32(0))),
        C({Val(u32(0x0000'ff00))}, Vec(f32(0), f32(1), f32(0), f32(0))),
        C({Val(u32(0x0000'00ff))}, Vec(f32(1), f32(0), f32(0), f32(0))),
        C({Val(u32(0x00ff'00ff))}, Vec(f32(1), f32(0), f32(1), f32(0))),
        C({Val(u32(0x0066'00ff))}, Vec(f32(1), f32(0), f32(0.4), f32(0))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Unpack4x8unorm,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kUnpack4X8Unorm),
                     testing::ValuesIn(Unpack4x8unormCases())));

std::vector<Case> Unpack2x16floatCases() {
    return {
        C({Val(u32(0x7bff'fbff))}, Vec(f32(f16::Lowest()), f32(f16::Highest()))),
        C({Val(u32(0xbc00'3c00))}, Vec(f32(1), f32(-1))),
        C({Val(u32(0x0000'0000))}, Vec(f32(0), f32(0))),
        C({Val(u32(0xc940'4900))}, Vec(f32(10), f32(-10.5))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Unpack2x16float,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kUnpack2X16Float),
                     testing::ValuesIn(Unpack2x16floatCases())));

std::vector<Case> Unpack2x16snormCases() {
    return {
        C({Val(u32(0x0000'0000))}, Vec(f32(0), f32(0))),
        C({Val(u32(0x8001'0000))}, Vec(f32(0), f32(-1))),
        C({Val(u32(0x7fff'0000))}, Vec(f32(0), f32(1))),
        C({Val(u32(0x0000'8001))}, Vec(f32(-1), f32(0))),
        C({Val(u32(0x0000'7fff))}, Vec(f32(1), f32(0))),
        C({Val(u32(0x8001'7fff))}, Vec(f32(1), f32(-1))),
        C({Val(u32(0x8001'7fff))}, Vec(f32(1), f32(-1))),
        C({Val(u32(0x4000'999a))}, Vec(f32(-0.80001220740379), f32(0.500015259254737))).FloatComp(),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Unpack2x16snorm,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kUnpack2X16Snorm),
                     testing::ValuesIn(Unpack2x16snormCases())));

std::vector<Case> Unpack2x16unormCases() {
    return {
        C({Val(u32(0xffff'0000))}, Vec(f32(0), f32(1))),
        C({Val(u32(0x0000'ffff))}, Vec(f32(1), f32(0))),
        C({Val(u32(0x0000'6666))}, Vec(f32(0.4), f32(0))),
        C({Val(u32(0x0000'ffff))}, Vec(f32(1), f32(0))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Unpack2x16unorm,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kUnpack2X16Unorm),
                     testing::ValuesIn(Unpack2x16unormCases())));

std::vector<Case> QuantizeToF16Cases() {
    return {
        C({0_f}, 0_f),    //
        C({-0_f}, -0_f),  //
        C({1_f}, 1_f),    //
        C({-1_f}, -1_f),  //

        //   0.00006106496 quantized to 0.000061035156 = 0x1p-14
        C({0.00006106496_f}, 0.000061035156_f),    //
        C({-0.00006106496_f}, -0.000061035156_f),  //

        //   1.0004883 quantized to 1.0 = 0x1p0
        C({1.0004883_f}, 1.0_f),    //
        C({-1.0004883_f}, -1.0_f),  //

        //   8196.0 quantized to 8192.0 = 0x1p13
        C({8196_f}, 8192_f),    //
        C({-8196_f}, -8192_f),  //

        // Value in subnormal f16 range
        C({0x0.034p-14_f}, 0x0.034p-14_f),    //
        C({-0x0.034p-14_f}, -0x0.034p-14_f),  //
        C({0x0.068p-14_f}, 0x0.068p-14_f),    //
        C({-0x0.068p-14_f}, -0x0.068p-14_f),  //

        //   0x0.06b7p-14 quantized to 0x0.068p-14
        C({0x0.06b7p-14_f}, 0x0.068p-14_f),    //
        C({-0x0.06b7p-14_f}, -0x0.068p-14_f),  //

        // Vector tests
        C({Vec(0_f, -0_f)}, Vec(0_f, -0_f)),  //
        C({Vec(1_f, -1_f)}, Vec(1_f, -1_f)),  //

        C({Vec(0.00006106496_f, -0.00006106496_f, 1.0004883_f, -1.0004883_f)},
          Vec(0.000061035156_f, -0.000061035156_f, 1.0_f, -1.0_f)),

        C({Vec(8196_f, 8192_f, 0x0.034p-14_f)}, Vec(8192_f, 8192_f, 0x0.034p-14_f)),

        C({Vec(0x0.034p-14_f, -0x0.034p-14_f, 0x0.068p-14_f, -0x0.068p-14_f)},
          Vec(0x0.034p-14_f, -0x0.034p-14_f, 0x0.068p-14_f, -0x0.068p-14_f)),

        // Value out of f16 range
        E({65504.003_f}, "12:34 error: value 65504.00390625 cannot be represented as 'f16'"),
        E({-65504.003_f}, "12:34 error: value -65504.00390625 cannot be represented as 'f16'"),
        E({0x1.234p56_f}, "12:34 error: value 81979586966978560.0 cannot be represented as 'f16'"),
        E({0x4.321p65_f},
          "12:34 error: value 154788719192723947520.0 cannot be represented as 'f16'"),
        E({Vec(65504.003_f, 0_f)},
          "12:34 error: value 65504.00390625 cannot be represented as 'f16'"),
        E({Vec(0_f, -0x4.321p65_f)},
          "12:34 error: value -154788719192723947520.0 cannot be represented as 'f16'"),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    QuantizeToF16,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kQuantizeToF16),
                     testing::ValuesIn(QuantizeToF16Cases())));

std::vector<Case> Dot4I8PackedCases() {
    return {
        // {-1, -2, -3, -4} . {-5, -6, -7, -8}
        C({Val(u32(0xFFFEFDFC)), Val(u32(0xFBFAF9F8))}, Val(i32(70))),
        // {1, 2, 3, 4} . {-1, -2, -3, -4}
        C({Val(u32(0x01020304)), Val(u32(0xFFFEFDFC))}, Val(i32(-30))),
        // {-9, -10, -11, -12} . {5, 6, 7, 8}
        C({Val(u32(0xF7F6F5F4)), Val(u32(0x05060708))}, Val(i32(-278))),
        // {0, 0, 0, 0} . {0, 0, 0, 0}
        C({Val(u32(0)), Val(u32(0))}, Val(i32(0))),
        // {127, 127, 127, 127} . {127, 127, 127, 127}
        C({Val(u32(0x7F7F7F7F)), Val(u32(0x7F7F7F7F))}, Val(i32(64516))),
        // {-128, -128, -128, -128} . {-128, -128, -128, -128}
        C({Val(u32(0x80808080)), Val(u32(0x80808080))}, Val(i32(65536))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Dot4I8Packed,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kDot4I8Packed),
                     testing::ValuesIn(Dot4I8PackedCases())));

std::vector<Case> Dot4U8PackedCases() {
    return {
        // {255, 254, 253, 252} . {251, 250, 249, 248}
        C({Val(u32(0xFFFEFDFC)), Val(u32(0xFBFAF9F8))}, Val(u32(252998))),
        // {1, 2, 3, 4} . {255, 254, 253, 252}
        C({Val(u32(0x01020304)), Val(u32(0xFFFEFDFC))}, Val(u32(2530))),
        // {0, 0, 0, 0} . {0, 0, 0, 0}
        C({Val(u32(0)), Val(u32(0))}, Val(u32(0))),
        // {255, 255, 255, 255} . {255, 255, 255, 255}
        C({Val(u32(0xFFFFFFFF)), Val(u32(0xFFFFFFFF))}, Val(u32(260100))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Dot4U8Packed,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kDot4U8Packed),
                     testing::ValuesIn(Dot4U8PackedCases())));

std::vector<Case> Pack4xI8Cases() {
    return {
        C({Vec(i32(0), i32(0), i32(0), i32(0))}, Val(u32(0x0000'0000))),
        C({Vec(i32(1), i32(2), i32(3), i32(4))}, Val(u32(0x0403'0201))),
        C({Vec(i32(-1), i32(-2), i32(-3), i32(-4))}, Val(u32(0xFCFD'FEFF))),
        C({Vec(i32(-1), i32(2), i32(3), i32(4))}, Val(u32(0x0403'02FF))),
        C({Vec(i32(1), i32(-2), i32(3), i32(4))}, Val(u32(0x0403'FE01))),
        C({Vec(i32(1), i32(2), i32(-3), i32(4))}, Val(u32(0x04FD'0201))),
        C({Vec(i32(1), i32(2), i32(3), i32(-4))}, Val(u32(0xFC03'0201))),
        C({Vec(i32(1), i32(-2), i32(-3), i32(-4))}, Val(u32(0xFCFD'FE01))),
        C({Vec(i32(-1), i32(2), i32(-3), i32(-4))}, Val(u32(0xFCFD'02FF))),
        C({Vec(i32(-1), i32(-2), i32(3), i32(-4))}, Val(u32(0xFC03'FEFF))),
        C({Vec(i32(-1), i32(-2), i32(-3), i32(4))}, Val(u32(0x04FD'FEFF))),
        C({Vec(i32(-1), i32(-2), i32(3), i32(4))}, Val(u32(0x0403'FEFF))),
        C({Vec(i32(-1), i32(2), i32(-3), i32(4))}, Val(u32(0x04FD'02FF))),
        C({Vec(i32(-1), i32(2), i32(3), i32(-4))}, Val(u32(0xFC03'02FF))),
        C({Vec(i32(1), i32(-2), i32(-3), i32(4))}, Val(u32(0x04FD'FE01))),
        C({Vec(i32(1), i32(-2), i32(3), i32(-4))}, Val(u32(0xFC03'FE01))),
        C({Vec(i32(1), i32(2), i32(-3), i32(-4))}, Val(u32(0xFCFD'0201))),
        C({Vec(i32(127), i32(128), i32(-128), i32(-129))}, Val(u32(0x7F80'807F))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Pack4xI8,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kPack4XI8),
                     testing::ValuesIn(Pack4xI8Cases())));

std::vector<Case> Pack4xU8Cases() {
    return {
        C({Vec(u32(0), u32(0), u32(0), u32(0))}, Val(u32(0x0000'0000))),
        C({Vec(u32(2), u32(4), u32(6), u32(8))}, Val(u32(0x0806'0402))),
        C({Vec(u32(255), u32(255), u32(255), u32(255))}, Val(u32(0xFFFF'FFFF))),
        C({Vec(u32(254), u32(255), u32(256), u32(257))}, Val(u32(0x0100'FFFE))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Pack4xU8,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kPack4XU8),
                     testing::ValuesIn(Pack4xU8Cases())));

std::vector<Case> Pack4xI8ClampCases() {
    return {
        C({Vec(i32(0), i32(0), i32(0), i32(0))}, Val(u32(0x0000'0000))),
        C({Vec(i32(1), i32(2), i32(3), i32(4))}, Val(u32(0x0403'0201))),
        C({Vec(i32(-1), i32(-2), i32(-3), i32(-4))}, Val(u32(0xFCFD'FEFF))),
        C({Vec(i32(-1), i32(2), i32(3), i32(4))}, Val(u32(0x0403'02FF))),
        C({Vec(i32(1), i32(-2), i32(3), i32(4))}, Val(u32(0x0403'FE01))),
        C({Vec(i32(1), i32(2), i32(-3), i32(4))}, Val(u32(0x04FD'0201))),
        C({Vec(i32(1), i32(2), i32(3), i32(-4))}, Val(u32(0xFC03'0201))),
        C({Vec(i32(1), i32(-2), i32(-3), i32(-4))}, Val(u32(0xFCFD'FE01))),
        C({Vec(i32(-1), i32(2), i32(-3), i32(-4))}, Val(u32(0xFCFD'02FF))),
        C({Vec(i32(-1), i32(-2), i32(3), i32(-4))}, Val(u32(0xFC03'FEFF))),
        C({Vec(i32(-1), i32(-2), i32(-3), i32(4))}, Val(u32(0x04FD'FEFF))),
        C({Vec(i32(-1), i32(-2), i32(3), i32(4))}, Val(u32(0x0403'FEFF))),
        C({Vec(i32(-1), i32(2), i32(-3), i32(4))}, Val(u32(0x04FD'02FF))),
        C({Vec(i32(-1), i32(2), i32(3), i32(-4))}, Val(u32(0xFC03'02FF))),
        C({Vec(i32(1), i32(-2), i32(-3), i32(4))}, Val(u32(0x04FD'FE01))),
        C({Vec(i32(1), i32(-2), i32(3), i32(-4))}, Val(u32(0xFC03'FE01))),
        C({Vec(i32(1), i32(2), i32(-3), i32(-4))}, Val(u32(0xFCFD'0201))),
        C({Vec(i32(127), i32(128), i32(-128), i32(-129))}, Val(u32(0x8080'7F7F))),
        C({Vec(i32(-32768), i32(-65536), i32(32767), i32(65535))}, Val(u32(0x7F7F'8080))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Pack4xI8Clamp,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kPack4XI8Clamp),
                     testing::ValuesIn(Pack4xI8ClampCases())));

std::vector<Case> Pack4xU8ClampCases() {
    return {
        C({Vec(u32(0), u32(0), u32(0), u32(0))}, Val(u32(0x0000'0000))),
        C({Vec(u32(2), u32(4), u32(6), u32(8))}, Val(u32(0x0806'0402))),
        C({Vec(u32(255), u32(255), u32(255), u32(255))}, Val(u32(0xFFFF'FFFF))),
        C({Vec(u32(254), u32(255), u32(256), u32(257))}, Val(u32(0xFFFF'FFFE))),
        C({Vec(u32(65535), u32(65536), u32(255), u32(254))}, Val(u32(0xFEFF'FFFF))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Pack4xU8Clamp,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kPack4XU8Clamp),
                     testing::ValuesIn(Pack4xU8ClampCases())));

std::vector<Case> Unpack4xI8Cases() {
    return {
        C({Val(u32(0x0000'0000))}, Vec(i32(0), i32(0), i32(0), i32(0))),
        C({Val(u32(0x0102'0304))}, Vec(i32(4), i32(3), i32(2), i32(1))),
        C({Val(u32(0xFCFD'FEFF))}, Vec(i32(-1), i32(-2), i32(-3), i32(-4))),
        C({Val(u32(0x0403'02FF))}, Vec(i32(-1), i32(2), i32(3), i32(4))),
        C({Val(u32(0x0403'FE01))}, Vec(i32(1), i32(-2), i32(3), i32(4))),
        C({Val(u32(0x04FD'0201))}, Vec(i32(1), i32(2), i32(-3), i32(4))),
        C({Val(u32(0xFC03'0201))}, Vec(i32(1), i32(2), i32(3), i32(-4))),
        C({Val(u32(0xFCFD'FE01))}, Vec(i32(1), i32(-2), i32(-3), i32(-4))),
        C({Val(u32(0xFCFD'02FF))}, Vec(i32(-1), i32(2), i32(-3), i32(-4))),
        C({Val(u32(0xFC03'FEFF))}, Vec(i32(-1), i32(-2), i32(3), i32(-4))),
        C({Val(u32(0x04FD'FEFF))}, Vec(i32(-1), i32(-2), i32(-3), i32(4))),
        C({Val(u32(0x0403'FEFF))}, Vec(i32(-1), i32(-2), i32(3), i32(4))),
        C({Val(u32(0x04FD'02FF))}, Vec(i32(-1), i32(2), i32(-3), i32(4))),
        C({Val(u32(0xFC03'02FF))}, Vec(i32(-1), i32(2), i32(3), i32(-4))),
        C({Val(u32(0x04FD'FE01))}, Vec(i32(1), i32(-2), i32(-3), i32(4))),
        C({Val(u32(0xFC03'FE01))}, Vec(i32(1), i32(-2), i32(3), i32(-4))),
        C({Val(u32(0xFCFD'0201))}, Vec(i32(1), i32(2), i32(-3), i32(-4))),
        C({Val(u32(0x8081'7F7E))}, Vec(i32(126), i32(127), i32(-127), i32(-128))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Unpack4xI8,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kUnpack4XI8),
                     testing::ValuesIn(Unpack4xI8Cases())));

std::vector<Case> Unpack4xU8Cases() {
    return {
        C({Val(u32(0x0000'0000))}, Vec(u32(0), u32(0), u32(0), u32(0))),
        C({Val(u32(0x0806'0402))}, Vec(u32(2), u32(4), u32(6), u32(8))),
        C({Val(u32(0xFFFF'FFFF))}, Vec(u32(255), u32(255), u32(255), u32(255))),
        C({Val(u32(0xFFFE'FDFC))}, Vec(u32(252), u32(253), u32(254), u32(255))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Unpack4xU8,
    ConstEvalBuiltinTest,
    testing::Combine(testing::Values(core::BuiltinFn::kUnpack4XU8),
                     testing::ValuesIn(Unpack4xU8Cases())));
}  // namespace
}  // namespace tint::core::constant::test

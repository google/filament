// RUN: %clang_cc1 -std=c++1z -verify %s

template<typename ...T> constexpr auto sum(T ...t) { return (... + t); }
template<typename ...T> constexpr auto product(T ...t) { return (t * ...); }
template<typename ...T> constexpr auto all(T ...t) { return (true && ... && t); }
template<typename ...T> constexpr auto all2(T ...t) { return (t && ... && true); }

int k1 = (1 + ... + 2); // expected-error {{does not contain any unexpanded parameter packs}}
int k2 = (1 + ...); // expected-error {{does not contain any unexpanded parameter packs}}
int k3 = (... + 2); // expected-error {{does not contain any unexpanded parameter packs}}

template<int ...N> void bad1() { (N + ... + N); } // expected-error {{unexpanded parameter packs in both operands}}
// FIXME: it would be reasonable to support this as an extension.
template<int ...N> void bad2() { (2 * N + ... + 1); } // expected-error {{expression not permitted as operand}}
template<int ...N> void bad3() { (2 + N * ... * 1); } // expected-error {{expression not permitted as operand}}
template<int ...N, int ...M> void bad4(int (&...x)[N]) { (N + M * ... * 1); } // expected-error {{expression not permitted as operand}}
template<int ...N, int ...M> void fixed4(int (&...x)[N]) { ((N + M) * ... * 1); }

// Parens are mandatory.
template<int ...N> void bad5() { N + ...; } // expected-error {{expected expression}} expected-error +{{}}
template<int ...N> void bad6() { ... + N; } // expected-error {{expected expression}}
template<int ...N> void bad7() { N + ... + N; } // expected-error {{expected expression}} expected-error +{{}}

// Must have a fold-operator in the relevant places.
template<int ...N> int bad8() { return (N + ... * 3); } // expected-error {{operators in fold expression must be the same}}
template<int ...N> int bad9() { return (3 + ... * N); } // expected-error {{operators in fold expression must be the same}}
template<int ...N> int bad10() { return (3 ? ... : N); } // expected-error +{{}} expected-note {{to match}}
template<int ...N> int bad11() { return (N + ... 0); } // expected-error {{expected a foldable binary operator}} expected-error {{expected expression}}
template<int ...N> int bad12() { return (... N); } // expected-error {{expected expression}}

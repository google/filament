// RUN: %dxc -I %hlsl_headers -T cs_6_10 -verify %s

#include <dx/linalg.h>
using namespace dx::linalg;

using MatrixATy = Matrix<ComponentType::F32, 4, 4, MatrixUse::A, MatrixScope::Wave>;

[nodiscard] int fn() { return 42; }
[[nodiscard]] int fn2() { return 42; }

struct [[nodiscard]] S {
  int x;

  [[clang::warn_unused_result]] static S fn4() { return (S)42; }
  [[clang::warn_unused_result]] S fn5() { return (S)42; }

  static S fn6() { return (S)42; }
  S fn7() { return (S)42; }
};

S fn3() { return (S)42; }

[numthreads(4, 4, 4)]
void main(uint ID : SV_GroupID)
{
  MatrixATy MatA1;
  MatA1.Splat(1.0f); // expected-warning {{ignoring return value of function declared with 'nodiscard' attribute}}
  fn(); // expected-warning {{ignoring return value of function declared with 'nodiscard' attribute}}
  fn2(); // expected-warning {{ignoring return value of function declared with 'nodiscard' attribute}}
  fn3(); // expected-warning {{ignoring return value of function declared with 'nodiscard' attribute}}
  S::fn4(); // expected-warning {{ignoring return value of function declared with 'warn_unused_result' attribute}}
  S s;
  s.fn5(); // expected-warning {{ignoring return value of function declared with 'warn_unused_result' attribute}}
  S::fn6(); // expected-warning {{ignoring return value of function declared with 'nodiscard' attribute}}
  s.fn7(); // expected-warning {{ignoring return value of function declared with 'nodiscard' attribute}}
}

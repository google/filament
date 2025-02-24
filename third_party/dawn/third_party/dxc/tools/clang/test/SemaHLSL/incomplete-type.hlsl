// RUN: %dxc -Tlib_6_3 -Wno-unused-value -verify %s

// Tests that the compiler is well-behaved with regard to uses of incomplete types.
// Regression test for GitHub #2058, which crashed in this case.

// expected-note@+4 {{forward declaration of 'S'}}
// expected-note@+3 {{forward declaration of 'S'}}
// expected-note@+2 {{forward declaration of 'S'}}
// expected-note@+1 {{forward declaration of 'S'}}
struct S;
ConstantBuffer<S> CB; // expected-error {{variable has incomplete type 'S'}}
S func( // expected-error {{incomplete result type 'S' in function definition}}
  S param) // expected-error {{variable has incomplete type 'S'}}
{
  S local; // expected-error {{variable has incomplete type 'S'}}
  return (S)0; // expected-error {{'S' is an incomplete type}}
}

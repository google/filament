// RUN: %dxc -T lib_6_3 -verify %s

struct Complete {};

struct Incomplete; // expected-note{{forward declaration of 'Incomplete'}}
template<typename T> struct CompleteTemplate {};

void fn() {
  uint s;
  // Complete types are easy. They are complete before we get to the expression.
  s = sizeof(Complete); // This works!

  // A type may be incomplete for several reasons.

  // It may be incomplete because there is only a forward declaration, which
  // should produce an error since we can't materialize a definition.
  s = sizeof(Incomplete); // expected-error{{invalid application of 'sizeof' to an incomplete type 'Incomplete'}}

  // It may be incomplete because it is an un-instantiated template, which
  // should work because we can just instantiate it.
  s = sizeof(CompleteTemplate<int>); // This works!

  // It may be incomplete because it is a lazy-initialized type from HLSL,
  // which can be completed, and then will report a non-numeric type error.
  // expected-error@+1{{invalid application of 'sizeof' to non-numeric type 'Buffer'}}
  s = sizeof(Buffer);
}

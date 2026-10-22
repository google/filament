// RUN: %dxc -T lib_6_3 -exports main %s -verify

// Regression test for GitHub #1943, where recursive struct member functions
// would crash the compiler.

// The SCCP pass replaces the recursive call with an undef value,
// which is why validation fails with a non-obvious error.

struct S
{
 // expected-error@+2{{recursive functions are not allowed: function 'main' calls recursive function 'S::func'}}
 // expected-note@+1{{recursive function located here:}}
  int func() { return func(); }
};

export int main() : OUT
{
  S s;
  return s.func();
}
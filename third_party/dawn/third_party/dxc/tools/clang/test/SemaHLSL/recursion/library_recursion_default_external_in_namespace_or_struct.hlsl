// RUN: %dxc -T lib_6_3 -default-linkage external %s -verify

// This file tests that default-linkage external works, and applies
// external linkage to decls inside namespaces, structs, or classes
// It will find such decls, and perform recursion validation on them.

struct S
{
// expected-error@+2{{recursive functions are not allowed: function 'main' calls recursive function 'S::func'}}
// expected-note@+1{{recursive function located here:}}
  int func() { return func(); }
};

namespace something {
// expected-error@+2{{recursive functions are not allowed: function 'something::func2' calls recursive function 'something::func2'}}
// expected-note@+1{{recursive function located here:}}
  int func2() { return func2(); }
}

class S2
{
// expected-error@+2{{recursive functions are not allowed: function 'S2::func3' calls recursive function 'S2::func3'}}
// expected-note@+1{{recursive function located here:}}
  int func3() { return func3(); }
};

export int main() : OUT
{
  S s;
  return s.func();
}
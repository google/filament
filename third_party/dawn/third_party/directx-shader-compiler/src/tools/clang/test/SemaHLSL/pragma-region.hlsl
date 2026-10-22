// RUN: %dxc -Tlib_6_3   -verify %s

// expected-no-diagnostics

// Clang doesn't do anything with the pragma region diagnostics, so there's
// nothing to test here except that we ignore them...

#pragma region Doggo
struct Doggo {
  int legs;
  int tailLength;
};
#pragma endregion Doggo

#pragma region functions

#pragma region foo
int foo() { return 0; }
#pragma region functions

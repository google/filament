// RUN: %dxc -Tlib_6_3 -Wno-unused-value -verify %s
// RUN: %dxc -Tcs_6_0 -Wno-unused-value -verify %s

// Verify that pointer and reference types produced by __decltype cannot be
// used as struct instance variable types or local variable types in HLSL.
// Also verifies that template instantiation with pointer/reference type
// arguments is rejected, and that valid __decltype uses are not affected.

// __decltype is the GCC way of saying 'decltype', but doesn't require C++11.

// groupshared variables are mutable (unlike plain global variables in HLSL),
// which allows their lvalue expressions to produce reference types via
// __decltype.
groupshared int g;

// --- Struct field: reference type from __decltype ---
struct RefField {
  __decltype(++g) ref_field; // expected-error {{references are unsupported in HLSL}}
};

// --- Struct field: pointer type via explicit cast in __decltype ---
struct PtrField {
  __decltype((int *)0) ptr_field; // expected-error {{pointers are unsupported in HLSL}}
};

// --- Struct field: plain (non-reference) type from __decltype is valid ---
struct PlainField {
  __decltype(0 + 1) plain_field; // no error: int rvalue, not a reference
};

// --- Local variable: reference type from __decltype ---
void test_local_ref() {
  int x = 0;
  __decltype(++x) local_ref; // expected-error {{references are unsupported in HLSL}}
}

// --- Local variable: pointer type via __decltype ---
void test_local_ptr() {
  __decltype((int *)0) local_ptr; // expected-error {{pointers are unsupported in HLSL}}
}

// --- Template: field with explicit reference type is rejected at definition ---
template <typename T>
struct RefContainer {
  T &ref_field; // expected-error {{references are unsupported in HLSL}}
};

// --- Template instantiation: type argument is an explicit pointer type ---
// This verifies that pointer instance variables cannot be created through
// template instantiation with pointer type arguments.
template <typename T>
struct Container {
  T field;
};

void test_template_ptr_explicit() {
  Container<int *> c; // expected-error {{pointers are unsupported in HLSL}}
}

// --- Regression: __decltype in is_same template argument still works ---
// HLSL has a non-standard is_same extension that treats T and T& as the same
// type. This is used in scalar-operators tests and must not be broken.
void test_is_same_regression() {
  int x = 0;
  _Static_assert(std::is_same<int, __decltype(x += 1)>::value,
                 "regression: is_same with lvalue __decltype must still work");
}

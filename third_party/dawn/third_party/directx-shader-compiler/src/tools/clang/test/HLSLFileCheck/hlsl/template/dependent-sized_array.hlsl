// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s | FileCheck %s
// CHECK: error: 'a3' declared as an array with a negative size

template<int N>
void f() {
  int a[] = { 1, 2, 3, N };
  uint numAs = sizeof(a) / sizeof(int);
}

template void f<17>();

template<int N>
void f1() {
  int a0[] = {}; // expected-warning{{zero}}
  int a1[] = { 1, 2, 3, N };
  int a3[sizeof(a1)/sizeof(int) != 4? 1 : -1]; // expected-error{{negative}}
}

namespace PR13788 {
  template <uint __N>
  struct S {
    int V;
  };
  template <int N>
  void foo() {
    S<0> arr[N] = {{ 4 }};
  }
}

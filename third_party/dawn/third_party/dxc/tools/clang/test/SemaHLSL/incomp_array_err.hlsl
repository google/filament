// RUN: %dxc -Tlib_6_3 -verify %s

// Verify error on on incomplete array in a struct or class

typedef const int inta[];                                   /* fxc-error {{X3072: array dimensions of type must be explicit}} */

static inta s_test1 = {1, 2, 3};                            /* fxc-error {{X3000: unrecognized identifier 'inta'}} */

static int s_test2[] = { 4, 5, 6 };

struct foo1 {
  float4 member;
  inta a;   // expected-error {{array dimensions of struct/class members must be explicit}} fxc-error {{X3000: unrecognized identifier 'inta'}}
};

struct foo2 {
  int a[];  // expected-error {{array dimensions of struct/class members must be explicit}} fxc-error {{X3072: 'foo2::a': array dimensions of struct/class members must be explicit}}
  float4 member;
};

class foo3 {
  float4 member;
  inta a;   // expected-error {{array dimensions of struct/class members must be explicit}} fxc-error {{X3000: unrecognized identifier 'inta'}}
};

class foo4 {
  float4 member;
  int a[];  // expected-error {{array dimensions of struct/class members must be explicit}} fxc-error {{X3072: 'foo4::a': array dimensions of struct/class members must be explicit}}
};

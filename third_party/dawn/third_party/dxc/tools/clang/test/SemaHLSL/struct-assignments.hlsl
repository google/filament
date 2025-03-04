// RUN: %dxc -Tlib_6_3 -verify %s
// RUN: %dxc -Tvs_6_0 -verify %s

// To test with the classic compiler, run
// %sdxroot%\tools\x86\fxc.exe /T vs_5_1 struct-assignments.hlsl

struct s_f {
 float f;
};
struct s_i {
 int i;
};
struct s_i_f2 {
 float2 f2;
 int i;
};
struct s_f2 {
 float2 f2;
};
struct s_ff {
 float f0;
 float f1;
};
struct s_f3_f3 {
 float3 f0;
 float3 f1;
};

float pick_one(s_f2 sf2) {
  return sf2.f2.x;
}
s_f2 pick_two(s_f2 sf2) {
  return sf2;
}

// Classes
class c_f2 {  // uses 'this.' for member access.
  float2 f2;
  void set(float2 v);
  float2 get();
  float2 get_inc();
  float2 get_inc_inline() {
    return this.f2++;
  }
};
void c_f2::set(float2 v) {
  this.f2 = v;
}
float2 c_f2::get() {
  return this.f2;
}
float2 c_f2::get_inc() {
  return this.f2++;
}

class c_f3 {
  float3 f3;
  void set(float3 v);
  float3 get();
  float3 get_inc();
  float3 get_inc_inline() {
    return f3++;
  }
};
void c_f3::set(float3 v) {
  f3 = v;
}
float3 c_f3::get() {
  return f3;
}
float3 c_f3::get_inc() {
  return f3++;
}

[shader("vertex")]
void main() {

  s_f2 zsf2_zero_cast = (s_f2)1;

  // No initialization.
  s_f sf_none;

  // Direct initialization fails.
  // fxc error: error X3000: syntax error: unexpected token '('
  s_f sf_direct(0.1f); /* expected-error {{expected ')'}} expected-error {{expected parameter declarator}} expected-note {{to match this '('}} fxc-error {{X3000: syntax error: unexpected token '('}} */


  // Initialization list with members.
  s_f  sf_all       = { 0.1f };
  s_f2 sf2_all      = { float2(1, 2) };
  s_f2 sf2_all_flat = { 0.1f, 0.2f };
  s_ff sff_all      = { 0.1f, 0.2f };
  s_f3_f3 sf3f3_all = { float3(1,2,3), float3(3,2,1) };
  s_f3_f3 sf3f3_all_flat = { 1,2,3, 3,2,1 };
  s_f3_f3 sf3f3_all_straddle = { float2(1,2), float2(3,4), float2(5,6) };
  s_f3_f3 sf3f3_all_straddle_instances[2] = { float2(1,2), sf3f3_all, float4(1,2,3,4) };
  s_f3_f3 sf3f3_nested = { { 1, 2, 3, 4, 5 }, { 1 } };
  // fxc error X3037 : constructors only defined for numeric base types
  s_f2 sf2_ctor     = s_f2(0.1f, 0.2f); // expected-error {{constructors only defined for numeric base types}} fxc-error {{X3037: constructors only defined for numeric base types}}
  s_f2 sf2_parens   = (s_f2){ float2(1, 2) };   // expected-error {{compound literal is unsupported in HLSL}} fxc-error {{X3000: syntax error: unexpected token '{'}} fxc-error {{X3000: syntax error: unexpected token '}'}}
  s_f2 sf2_parens2  = (s_f2)({ float2(1, 2) }); // expected-error {{expected ';' after expression}} fxc-error {{X3000: syntax error: unexpected token '{'}} fxc-error {{X3000: syntax error: unexpected token '}'}}
  s_f2 sf2_direct   = s_f2{ float2(1, 2) };     // expected-error {{expected '(' for function-style cast or type construction}} fxc-error {{X3000: syntax error: unexpected token '{'}}

  // zero must be cast to struct for assignment to struct
  s_f2 sf2_zero_cast = (s_f2)1;

  // Initialization list with insufficient members fails.
  // fxc error: error X3017: 'f2_missing': initializer does not match type
  s_f2 f2_missing = { 0.1f }; // expected-error {{too few elements in vector initialization (expected 2 elements, have 1)}} fxc-error {{X3017: 'f2_missing': initializer does not match type}}

  // Initialization list with too many members fails.
  // fxc error: error X3017: 'f2_too_many': initializer does not match type
  s_f2 f2_too_many = { 0.1f, 0.2f, 0.3f }; // expected-error {{too many elements in vector initialization (expected 2 elements, have 3)}} fxc-error {{X3017: 'f2_too_many': initializer does not match type}}

  // Initialization list with different element types.
  s_f2 f2_ints = { 1, 2 };

  // Initialization list with different element types.
  double d = 0.123;
  s_f2 f2_int_double = { 1, d };

  // Initialization list with packed element.
  s_f2 f2_f2 = { f2_ints };

  // Initialization list with mixed packed elements.
  s_ff sff = { 1, 0.1f };

  // fxc error X3017: cannot convert from 'int' to 'struct s_f2'
  s_f2 sf2_zero = 0; // expected-error {{cannot initialize a variable of type 's_f2' with an rvalue of type 'literal int'}} fxc-error {{X3017: cannot convert from 'int' to 'struct s_f2'}}

  s_f2 sf2_zero_assign;
  // fxc error X3017: cannot convert from 'int' to 'struct s_f2'
  sf2_zero_assign = 0; // expected-error {{cannot implicitly convert from 'literal int' to 's_f2'}} fxc-error {{X3017: cannot convert from 'int' to 'struct s_f2'}}

  // Initialization list with too few mixed packed elements fails.
  // fxc error: error X3017: 'f3_f2_f_f': initializer does not match type
  s_f3_f3 f3_f2_f_f = { sf2_all, 0.1f, 0.2f }; // expected-error {{too few elements in vector initialization (expected 6 elements, have 4)}} fxc-error {{X3017: 'f3_f2_f_f': initializer does not match type}}

  // Constructor with wrong element count fails.
  // fxc error: error X3014: incorrect number of arguments to numeric-type constructor
  s_f2 f2c = float2(); // expected-error {{'float2' cannot have an explicit empty initializer}} fxc-error {{X3014: incorrect number of arguments to numeric-type constructor}}
  // fxc error: error X3014: incorrect number of arguments to numeric-type constructor
  s_f2 f2c_f = float2(0.1f); // expected-error {{cannot initialize a variable of type 's_f2' with an rvalue of type 'float2'}} expected-error {{too few elements in vector initialization (expected 2 elements, have 1)}} fxc-error {{X3014: incorrect number of arguments to numeric-type constructor}}

  // Simple parameter passing.
  pick_one(sf2_all);
  sf2_all = pick_two(sf2_all);

}

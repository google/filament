// RUN: %clang_cc1 -fsyntax-only -ffreestanding -verify %s

// To test with the classic compiler, run
// %sdxroot%\tools\x86\fxc.exe /T vs_2_0 vector-assignments.hlsl

float pick_one(float2 f2) {
  return f2.x;
}

void main() {

// No initialization.
float2 f2_none;

// Direct initialization fails.
// fxc error: error X3000: syntax error: unexpected token '('
//float2 f2_direct(0.1f, 0.2f); /* expected-error {{expected ')'}} expected-error {{expected parameter declarator}} expected-note {{to match this '('}} fxc-error {{X3000: syntax error: unexpected token '('}} */ /* */ /* */

// Initialization list with members.
float2 f2_all = { 0.1f, 0.2f };

// Initialization list with insufficient members fails.
// fxc error: error X3017: 'f2_missing': initializer does not match type
//float2 f2_missing = { 0.1f }; // expected-error {{too few elements in vector initialization (expected 2 elements, have 1)}} fxc-error {{X3017: 'f2_missing': initializer does not match type}} 

// Initialization list with too many members fails.
// fxc error: error X3017: 'f2_too_many': initializer does not match type
//float2 f2_too_many = { 0.1f, 0.2f, 0.3f }; // expected-error {{too many elements in vector initialization (expected 2 elements, have 3)}} fxc-error {{X3017: 'f2_too_many': initializer does not match type}} 

// Initialization list with different element types.
float2 f2_ints = { 1, 2 };

// Initialization list with different element types.
double d = 0.123;
float2 f2_int_double = { 1, d };

// Initialization list with packed element.
float2 f2_f2 = { f2_all };

// Initialization list with mixed packed elements.
float3 f3_f2_f = { f2_all, 0.1f };

// Initialization list with too many mixed packed elements fails.
// fxc error: error X3017: 'f3_f2_f_f': initializer does not match type
//float3 f3_f2_f_f = { f2_all, 0.1f, 0.2f }; // expected-error {{too many elements in vector initialization (expected 3 elements, have 4)}} fxc-error {{X3017: 'f3_f2_f_f': initializer does not match type}} 

// Constructor with wrong element count fails.
// fxc error: error X3014: incorrect number of arguments to numeric-type constructor
//float2 f2c = float2(); // expected-error {{'float2' cannot have an explicit empty initializer}} fxc-error {{X3014: incorrect number of arguments to numeric-type constructor}} 
// fxc error: error X3014: incorrect number of arguments to numeric-type constructor
//float2 f2c_f = float2(0.1f); // expected-error {{too few elements in vector initialization (expected 2 elements, have 1)}} fxc-error {{X3014: incorrect number of arguments to numeric-type constructor}} 

// Construct with exact number.
float2 f2c_f_f = float2(0.1f, 0.2f);

// Construct with packed value.
float2 f2c_f2 = float2(f2c_f_f);

// Construct with mixed values.
float3 f3c_f2_f = float3(f2c_f_f, 1);

// *assignments* don't mind if they are narrowing, but warn.
// fxc error: warning X3206: implicit truncation of vector type
//float2 f2a_f2_f = f3c_f2_f; // expected-warning {{implicit truncation of vector type}} fxc-warning {{X3206: implicit truncation of vector type}} 
//float2 f2c_f2_f = float3(f2c_f_f, 1); // expected-warning {{implicit truncation of vector type}} fxc-warning {{X3206: implicit truncation of vector type}} 

// *assignments* do mind if they are widening.
// fxc error: error X3017: cannot implicitly convert from 'float3' to 'float4'
//float4 f4c_f2_f = f3c_f2_f; // expected-error {{cannot initialize a variable of type 'float4' with an lvalue of type 'float3'}} fxc-error {{X3017: cannot implicitly convert from 'float3' to 'float4'}} 

// initializers only work on assignment.
// fxc error: // error X3000: syntax error: unexpected token '{'
//pick_one({0.1f, 0.2f}); // expected-error {{expected expression}} fxc-error {{X3000: syntax error: unexpected token '{'}} fxc-error {{X3013: 'pick_one': no matching 0 parameter function}} 

// constructrs work without special cases.
pick_one(float2(0.1f, 0.2f));

}
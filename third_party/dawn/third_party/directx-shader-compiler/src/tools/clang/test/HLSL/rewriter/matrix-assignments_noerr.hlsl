// RUN: %clang_cc1 -fsyntax-only -ffreestanding -verify %s

// To test with the classic compiler, run
// %sdxroot%\tools\x86\fxc.exe /T vs_2_0 matrix-assignments.hlsl

float pick_one(float2x2 f2) {
  // TODO: implement swizzling members return f2._m00;
  return 1;
}

float2x2 ret_f22() {
  float2x2 result = 0;
  return result;
}

//float2x2 ret_f22_list() {
//  // fxc error: error X3000: syntax error: unexpected token '{'
//  return { 1, 2, 3, 4 }; // expected-error {{generalized initializer lists are incompatible with HLSL}} fxc-error {{X3000: syntax error: unexpected token '{'}} 
//  // return 0;
//}

void main() {

float2 f2;

// Default initialization.
// See http://en.cppreference.com/w/cpp/language/default_initialization
float1x1 f11_default;
float2x2 f22_default;
matrix<int, 2, 3> i23_default;

// Direct initialization.
// See http://en.cppreference.com/w/cpp/language/direct_initialization
// fxc error X3000: syntax error: unexpected token '('
//float2x2 f22_direct(0.1f, 0.2f, 0.3f, 0.4f); // expected-error {{expected ')'}} expected-error {{expected parameter declarator}} expected-note {{to match this '('}} fxc-error {{X3000: syntax error: unexpected token '('}} 
// fxc error X3000: syntax error: unexpected float constant
//float2x2 f22_direct_braces { 0.1f, 0.2f, 0.3f, 0.4f }; // expected-error {{expected ';' at end of declaration}} fxc-error {{X3000: syntax error: unexpected float constant}} 
float2x2 f22_target = float2x2(0.1f, 0.2f, 0.3f, 0.4f);
float2x2 f22_target_clone = float2x2(f22_target);
// fxc warning X3081: comma expression used where a vector constructor may have been intended
//float2x2 f22_target_cast = (float2x2)(0.1f, 0.2f, 0.3f, 0.4f); // expected-warning {{comma expression used where a constructor list may have been intended}} fxc-warning {{X3081: comma expression used where a vector constructor may have been intended}} 
float2x2 f22_target_mix = float2x2(0.1f, f11_default, f2);
// fxc error X3014: incorrect number of arguments to numeric-type constructor
//float2x2 f22_target_missing = float2x2(0.1f, f11_default); // expected-error {{too few elements in vector initialization (expected 4 elements, have 2)}} fxc-error {{X3014: incorrect number of arguments to numeric-type constructor}} 
// fxc error X3014: incorrect number of arguments to numeric-type constructor
//float2x2 f22_target_too_many = float2x2(0.1f, f11_default, f2, 1); // expected-error {{too many elements in vector initialization (expected 4 elements, have 5)}} fxc-error {{X3014: incorrect number of arguments to numeric-type constructor}} 

// Copy initialization.
// See http://en.cppreference.com/w/cpp/language/copy_initialization
matrix<float, 2, 2> f22_copy = f22_default;
float f = pick_one(f22_default);
matrix<float, 2, 2> f22_copy_ret = ret_f22();
float1x2 f22_arr[2] = { 1, 2, 10, 20 };

// List initialization.
// See http://en.cppreference.com/w/cpp/language/list_initialization
// fxc error X3000: syntax error: unexpected token '{'
//float2x2 f22_list_braces{ 1, 2, 3, 4 }; // expected-error {{expected ';' at end of declaration}} fxc-error {{X3000: syntax error: unexpected integer constant}} 
// fxc error error X3000: syntax error: unexpected token '{'
//pick_one(float2x2 { 1, 2, 3, 4 }); // expected-error {{expected '(' for function-style cast or type construction}} fxc-error {{X3000: syntax error: unexpected token '{'}} fxc-error {{X3013: 'pick_one': no matching 0 parameter function}} 
//ret_f22_list();
// fxc error: error X3000: syntax error: unexpected token '{'
//pick_one({ 1, 2, 3, 4 }); // expected-error {{expected expression}} fxc-error {{X3000: syntax error: unexpected token '{'}} fxc-error {{X3013: 'pick_one': no matching 0 parameter function}} 
// TODO: test in subscript expression
// fxc error: error X3000: syntax error: unexpected token '{'
//pick_one(float2x2({ 1, 2, 3, 4 })); // expected-error {{expected expression}} fxc-error {{X3000: syntax error: unexpected token '{'}} fxc-error {{X3013: 'pick_one': no matching 0 parameter function}} 
float2x2 f22_list_copy = { 1, 2, 3, 4 };
int2x2 i22_list_narrowing = { 1.5f, 1.5f, 1.5f, 1.5f };

}
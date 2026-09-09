// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main

// To test with the classic compiler, run
// fxc.exe /T vs_2_0 matrix-assignments.hlsl

float pick_one(float2x2 f2) {
  // TODO: implement swizzling members return f2._m00;
  return 1;
}

float2x2 ret_f22() {
  float2x2 result = 0;
  return result;
}

float2x2 ret_f22_list() {
  return 0;
}

void main() {

float2 f2;

// Default initialization.
// See http://en.cppreference.com/w/cpp/language/default_initialization
float1x1 f11_default;
float2x2 f22_default;
matrix<int, 2, 3> i23_default;
/*verify-ast
  DeclStmt <col:1, col:30>
  `-VarDecl <col:1, col:19> i23_default 'matrix<int, 2, 3>':'matrix<int, 2, 3>'
*/

// Direct initialization.
// See http://en.cppreference.com/w/cpp/language/direct_initialization
float2x2 f22_target = float2x2(0.1f, 0.2f, 0.3f, 0.4f);

float2x2 f22_target_clone = float2x2(f22_target);

// fxc warning X3081: comma expression used where a vector constructor may have been intended
float2x2 f22_target_cast = (float2x2)(0.1f, 0.2f, 0.3f, 0.4f); // expected-warning {{comma expression used where a constructor list may have been intended}} fxc-warning {{X3081: comma expression used where a vector constructor may have been intended}}
float2x2 f22_target_mix = float2x2(0.1f, f11_default, f2);

// Copy initialization.
// See http://en.cppreference.com/w/cpp/language/copy_initialization
matrix<float, 2, 2> f22_copy = f22_default;

float f = pick_one(f22_default);
matrix<float, 2, 2> f22_copy_ret = ret_f22();

float1x2 f22_arr[2] = { 1, 2, 10, 20 };

// List initialization.
// See http://en.cppreference.com/w/cpp/language/list_initialization
ret_f22_list();
// TODO: test in subscript expression
float2x2 f22_list_copy = { 1, 2, 3, 4 };

int2x2 i22_list_narrowing = { 1.5f, 1.5f, 1.5f, 1.5f };     /* expected-warning {{implicit conversion from 'float' to 'int' changes value from 1.5 to 1}} expected-warning {{implicit conversion from 'float' to 'int' changes value from 1.5 to 1}} expected-warning {{implicit conversion from 'float' to 'int' changes value from 1.5 to 1}} expected-warning {{implicit conversion from 'float' to 'int' changes value from 1.5 to 1}} fxc-pass {{}} */


}
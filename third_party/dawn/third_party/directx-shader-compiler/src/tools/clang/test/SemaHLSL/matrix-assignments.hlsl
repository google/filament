// RUN: %dxc -Tlib_6_3   -verify %s
// RUN: %dxc -Tps_6_0   -verify %s

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

float2x2 ret_f22_list() {
  // fxc error: error X3000: syntax error: unexpected token '{'
  return { 1, 2, 3, 4 }; // expected-error {{generalized initializer lists are incompatible with HLSL}} fxc-error {{X3000: syntax error: unexpected token '{'}}
  // return 0;
}


void fn1(out float2x3 val[2])
{
  val = (float2x3[2])1;
}

void fn2(inout float2x3 val)
{
  val += 1;
}

void fn3(inout float3 val)
{
  val.y += 2;
}

[shader("pixel")]
float4 main() : SV_Target {

  float2 f2;

  // Default initialization.
  // See http://en.cppreference.com/w/cpp/language/default_initialization
  float1x1 f11_default;
  float2x2 f22_default;
  matrix<int, 2, 3> i23_default;
  /*verify-ast
    DeclStmt <col:3, col:32>
    `-VarDecl <col:3, col:21> col:21 i23_default 'matrix<int, 2, 3>':'matrix<int, 2, 3>'
  */

  // Direct initialization.
  // See http://en.cppreference.com/w/cpp/language/direct_initialization
  // fxc error X3000: syntax error: unexpected token '('
  float2x2 f22_direct(0.1f, 0.2f, 0.3f, 0.4f); // expected-error {{expected ')'}} expected-error {{expected parameter declarator}} expected-note {{to match this '('}} fxc-error {{X3000: syntax error: unexpected token '('}}
  // fxc error X3000: syntax error: unexpected float constant
  float2x2 f22_direct_braces { 0.1f, 0.2f, 0.3f, 0.4f }; // expected-warning {{effect state block ignored - effect syntax is deprecated}} fxc-error {{X3000: syntax error: unexpected float constant}}
  float2x2 f22_target = float2x2(0.1f, 0.2f, 0.3f, 0.4f);
  /*verify-ast
    DeclStmt <col:3, col:57>
    `-VarDecl <col:3, col:56> col:12 used f22_target 'float2x2':'matrix<float, 2, 2>' cinit
      `-CXXFunctionalCastExpr <col:25, col:56> 'float2x2':'matrix<float, 2, 2>' functional cast to float2x2 <NoOp>
        `-InitListExpr <col:33, col:56> 'float2x2':'matrix<float, 2, 2>'
          |-FloatingLiteral <col:34> 'float' 1.000000e-01
          |-FloatingLiteral <col:40> 'float' 2.000000e-01
          |-FloatingLiteral <col:46> 'float' 3.000000e-01
          `-FloatingLiteral <col:52> 'float' 4.000000e-01
  */
  float2x2 f22_target_clone = float2x2(f22_target);
  /*verify-ast
    DeclStmt <col:3, col:51>
    `-VarDecl <col:3, col:50> col:12 f22_target_clone 'float2x2':'matrix<float, 2, 2>' cinit
      `-CXXFunctionalCastExpr <col:31, col:50> 'float2x2':'matrix<float, 2, 2>' functional cast to float2x2 <NoOp>
        `-InitListExpr <col:31, col:40> 'float2x2':'matrix<float, 2, 2>'
          `-ImplicitCastExpr <col:40> 'float2x2':'matrix<float, 2, 2>' <LValueToRValue>
            `-DeclRefExpr <col:40> 'float2x2':'matrix<float, 2, 2>' lvalue Var 'f22_target' 'float2x2':'matrix<float, 2, 2>'
  */
  // fxc warning X3081: comma expression used where a vector constructor may have been intended
  float2x2 f22_target_cast = (float2x2)(0.1f, 0.2f, 0.3f, 0.4f); // expected-warning {{comma expression used where a constructor list may have been intended}} fxc-warning {{X3081: comma expression used where a vector constructor may have been intended}}
  float2x2 f22_target_mix = float2x2(0.1f, f11_default, f2);
  // fxc error X3014: incorrect number of arguments to numeric-type constructor
  float2x2 f22_target_missing = float2x2(0.1f, f11_default); // expected-error {{too few elements in vector initialization (expected 4 elements, have 2)}} fxc-error {{X3014: incorrect number of arguments to numeric-type constructor}}
  // fxc error X3014: incorrect number of arguments to numeric-type constructor
  float2x2 f22_target_too_many = float2x2(0.1f, f11_default, f2, 1); // expected-error {{too many elements in vector initialization (expected 4 elements, have 5)}} fxc-error {{X3014: incorrect number of arguments to numeric-type constructor}}

  // Copy initialization.
  // See http://en.cppreference.com/w/cpp/language/copy_initialization
  matrix<float, 2, 2> f22_copy = f22_default;
  /*verify-ast
    DeclStmt <col:3, col:45>
    `-VarDecl <col:3, col:34> col:23 f22_copy 'matrix<float, 2, 2>':'matrix<float, 2, 2>' cinit
      `-ImplicitCastExpr <col:34> 'float2x2':'matrix<float, 2, 2>' <LValueToRValue>
        `-DeclRefExpr <col:34> 'float2x2':'matrix<float, 2, 2>' lvalue Var 'f22_default' 'float2x2':'matrix<float, 2, 2>'
  */
  float f = pick_one(f22_default);
  matrix<float, 2, 2> f22_copy_ret = ret_f22();
  /*verify-ast
    DeclStmt <col:3, col:47>
    `-VarDecl <col:3, col:46> col:23 f22_copy_ret 'matrix<float, 2, 2>':'matrix<float, 2, 2>' cinit
      `-CallExpr <col:38, col:46> 'float2x2':'matrix<float, 2, 2>'
        `-ImplicitCastExpr <col:38> 'float2x2 (*)()' <FunctionToPointerDecay>
          `-DeclRefExpr <col:38> 'float2x2 ()' lvalue Function 'ret_f22' 'float2x2 ()'
  */
  float1x2 f22_arr[2] = { 1, 2, 10, 20 };

  // List initialization.
  // See http://en.cppreference.com/w/cpp/language/list_initialization
  // fxc error X3000: syntax error: unexpected token '{'
  float2x2 f22_list_braces{ 1, 2, 3, 4 }; // expected-warning {{effect state block ignored - effect syntax is deprecated}} fxc-error {{X3000: syntax error: unexpected integer constant}}
  // fxc error error X3000: syntax error: unexpected token '{'
  pick_one(float2x2 { 1, 2, 3, 4 }); // expected-error {{expected '(' for function-style cast or type construction}} fxc-error {{X3000: syntax error: unexpected token '{'}} fxc-error {{X3013: 'pick_one': no matching 0 parameter function}}
  ret_f22_list();
  // fxc error: error X3000: syntax error: unexpected token '{'
  pick_one({ 1, 2, 3, 4 }); // expected-error {{expected expression}} fxc-error {{X3000: syntax error: unexpected token '{'}} fxc-error {{X3013: 'pick_one': no matching 0 parameter function}}
  // TODO: test in subscript expression
  // fxc error: error X3000: syntax error: unexpected token '{'
  pick_one(float2x2({ 1, 2, 3, 4 })); // expected-error {{expected expression}} fxc-error {{X3000: syntax error: unexpected token '{'}} fxc-error {{X3013: 'pick_one': no matching 0 parameter function}}
  float2x2 f22_list_copy = { 1, 2, 3, 4 };
  /*verify-ast
    DeclStmt <col:3, col:42>
    `-VarDecl <col:3, col:41> col:12 f22_list_copy 'float2x2':'matrix<float, 2, 2>' cinit
      `-InitListExpr <col:28, col:41> 'float2x2':'matrix<float, 2, 2>'
        |-ImplicitCastExpr <col:30> 'float' <IntegralToFloating>
        | `-IntegerLiteral <col:30> 'literal int' 1
        |-ImplicitCastExpr <col:33> 'float' <IntegralToFloating>
        | `-IntegerLiteral <col:33> 'literal int' 2
        |-ImplicitCastExpr <col:36> 'float' <IntegralToFloating>
        | `-IntegerLiteral <col:36> 'literal int' 3
        `-ImplicitCastExpr <col:39> 'float' <IntegralToFloating>
          `-IntegerLiteral <col:39> 'literal int' 4
  */
  int2x2 i22_list_narrowing = { 1.5f, 1.5f, 1.5f, 1.5f };     /* expected-warning {{implicit conversion from 'float' to 'int' changes value from 1.5 to 1}} expected-warning {{implicit conversion from 'float' to 'int' changes value from 1.5 to 1}} expected-warning {{implicit conversion from 'float' to 'int' changes value from 1.5 to 1}} expected-warning {{implicit conversion from 'float' to 'int' changes value from 1.5 to 1}} fxc-pass {{}} */
  /*verify-ast
    DeclStmt <col:3, col:57>
    `-VarDecl <col:3, col:56> col:10 i22_list_narrowing 'int2x2':'matrix<int, 2, 2>' cinit
      `-InitListExpr <col:31, col:56> 'int2x2':'matrix<int, 2, 2>'
        |-ImplicitCastExpr <col:33> 'int' <FloatingToIntegral>
        | `-FloatingLiteral <col:33> 'float' 1.500000e+00
        |-ImplicitCastExpr <col:39> 'int' <FloatingToIntegral>
        | `-FloatingLiteral <col:39> 'float' 1.500000e+00
        |-ImplicitCastExpr <col:45> 'int' <FloatingToIntegral>
        | `-FloatingLiteral <col:45> 'float' 1.500000e+00
        `-ImplicitCastExpr <col:51> 'int' <FloatingToIntegral>
          `-FloatingLiteral <col:51> 'float' 1.500000e+00
  */

// Initialization list with invalid types
  Texture2D t;
  int2x2 i22_list_type_mismatch = { 1, t, 3, 4 };             /* expected-error {{type mismatch}} fxc-pass {{}} */

  float2x3 val[2];

  fn1(val);
  /*verify-ast
    CallExpr <col:3, col:10> 'void'
    |-ImplicitCastExpr <col:3> 'void (*)(float2x3 __restrict[2])' <FunctionToPointerDecay>
    | `-DeclRefExpr <col:3> 'void (float2x3 __restrict[2])' lvalue Function 'fn1' 'void (float2x3 __restrict[2])'
    `-ImplicitCastExpr <col:7> 'float2x3 [2]' <LValueToRValue>
      `-DeclRefExpr <col:7> 'float2x3 [2]' lvalue Var 'val' 'float2x3 [2]'
  */

  fn2(val[1]);
  /*verify-ast
    CallExpr <col:3, col:13> 'void'
    |-ImplicitCastExpr <col:3> 'void (*)(float2x3 &__restrict)' <FunctionToPointerDecay>
    | `-DeclRefExpr <col:3> 'void (float2x3 &__restrict)' lvalue Function 'fn2' 'void (float2x3 &__restrict)'
    `-ArraySubscriptExpr <col:7, col:12> 'float2x3':'matrix<float, 2, 3>' lvalue
      |-ImplicitCastExpr <col:7> 'float2x3 [2]' <LValueToRValue>
      | `-DeclRefExpr <col:7> 'float2x3 [2]' lvalue Var 'val' 'float2x3 [2]'
      `-IntegerLiteral <col:11> 'literal int' 1
  */

  // TODO: Should the above AST contain the LValueToRValue cast for ArraySubscriptExpr?
  //  |-ImplicitCastExpr <col:7> 'float2x3 [2]' <LValueToRValue>
  // Same question applies to the next case as well

  fn3(val[0][0]);
  /*verify-ast
    CallExpr <col:3, col:16> 'void'
    |-ImplicitCastExpr <col:3> 'void (*)(float3 &__restrict)' <FunctionToPointerDecay>
    | `-DeclRefExpr <col:3> 'void (float3 &__restrict)' lvalue Function 'fn3' 'void (float3 &__restrict)'
    `-CXXOperatorCallExpr <col:7, col:15> 'vector<float, 3>':'vector<float, 3>' lvalue
      |-ImplicitCastExpr <col:13, col:15> 'vector<float, 3> &(*)(unsigned int)' <FunctionToPointerDecay>
      | `-DeclRefExpr <col:13, col:15> 'vector<float, 3> &(unsigned int)' lvalue CXXMethod 'operator[]' 'vector<float, 3> &(unsigned int)'
      |-ArraySubscriptExpr <col:7, col:12> 'float2x3':'matrix<float, 2, 3>' lvalue
      | |-ImplicitCastExpr <col:7> 'float2x3 [2]' <LValueToRValue>
      | | `-DeclRefExpr <col:7> 'float2x3 [2]' lvalue Var 'val' 'float2x3 [2]'
      | `-IntegerLiteral <col:11> 'literal int' 0
      `-ImplicitCastExpr <col:14> 'unsigned int' <IntegralCast>
        `-IntegerLiteral <col:14> 'literal int' 0
  */

  return float4(val[0][0] + val[1][0], 1);

}
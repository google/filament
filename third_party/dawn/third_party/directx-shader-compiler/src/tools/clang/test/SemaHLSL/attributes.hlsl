// RUN: %dxc -Tlib_6_3 -Wno-misplaced-attributes  -HV 2018 -verify %s
// RUN: %dxc -Tps_6_0  -HV 2018 -verify %s

// To test with the classic compiler, run
// %sdxroot%\tools\x86\fxc.exe /T ps_5_1 attributes.hlsl

// The following is a directive to override default behavior for "VerifyHelper.py fxc RunAttributes".  When this is specified, main shader must be defined manually.
// :FXC_VERIFY_ARGUMENTS: /T ps_5_1 /E main

int loop_before_assignment() {
  // fxc warning X3554: unknown attribute loop, or attribute invalid for this statement
  [loop] // expected-warning {{attribute 'loop' can only be applied to 'for', 'while' and 'do' loop statements}} fxc-pass {{}}
  int val = 2;
  return val;
}

int loop_before_return() {
  // fxc warning X3554: unknown attribute loop, or attribute invalid for this statement
  [loop] // expected-warning {{attribute 'loop' can only be applied to 'for', 'while' and 'do' loop statements}} fxc-warning {{X3554: unknown attribute loop, or attribute invalid for this statement}}
  return 0;
}

int loop_before_if(int a) {
  [loop] // expected-warning {{attribute 'loop' can only be applied to 'for', 'while' and 'do' loop statements}} fxc-pass {{}}
  if (a > 0) return -a;
  return a;
}

int loop_before_switch(int a) {
  [loop] // expected-warning {{attribute 'loop' can only be applied to 'for', 'while' and 'do' loop statements}} fxc-pass {{}}
  switch (a) {
    case 0:
      return 1;
      break;
  }
  return 0;
}

int fastopt_before_if(int a) {
  [fastopt] // expected-warning {{attribute 'fastopt' can only be applied to 'for', 'while' and 'do' loop statements}} fxc-pass {{}}
  if (a > 0) return -a;
  return a;
}

int fastopt_before_switch(int a) {
  [fastopt] // expected-warning {{attribute 'fastopt' can only be applied to 'for', 'while' and 'do' loop statements}} fxc-pass {{}}
  switch (a) {
    case 0:
      return 1;
      break;
  }
  return 0;
}

int unroll_before_if(int a) {
  [unroll] // expected-warning {{attribute 'unroll' can only be applied to 'for', 'while' and 'do' loop statements}} fxc-pass {{}}
  if (a > 0) return -a;
  return a;
}

int unroll_before_switch(int a) {
  [unroll] // expected-warning {{attribute 'unroll' can only be applied to 'for', 'while' and 'do' loop statements}} fxc-pass {{}}
  switch (a) {
    case 0:
      return 1;
      break;
  }
  return 0;
}

int allow_uav_condition_before_if(int a) {
  [allow_uav_condition] // expected-warning {{attribute 'allow_uav_condition' can only be applied to 'for', 'while' and 'do' loop statements}} fxc-pass {{}}
  if (a > 0) return -a;
  return a;
}

int allow_uav_condition_before_switch(int a) {
  [allow_uav_condition] // expected-warning {{attribute 'allow_uav_condition' can only be applied to 'for', 'while' and 'do' loop statements}} fxc-pass {{}}
  switch (a) {
    case 0:
      return 1;
      break;
  }
  return 0;
}

int branch_before_for() {
  int result = 0;
  [branch] // expected-warning {{attribute 'branch' can only be applied to 'if' and 'switch' statements}} fxc-pass {{}}
  for (int i = 0; i < 10; i++) result++;
  return result;
}

int branch_before_while() {
  int result = 0;
  int i = 0;
  [branch] // expected-warning {{attribute 'branch' can only be applied to 'if' and 'switch' statements}} fxc-pass {{}}
  while(i < 10) {
    result++;
    i++;
  }
  return result;
}

int branch_before_do() {
  int result = 0;
  int i = 0;
  [branch] // expected-warning {{attribute 'branch' can only be applied to 'if' and 'switch' statements}} fxc-pass {{}}
  do {
    result++;
    i++;
  } while(i < 10);
  return result;
}

int flatten_before_for() {
  int result = 0;
  [flatten] // expected-warning {{attribute 'flatten' can only be applied to 'if' and 'switch' statements}} fxc-pass {{}}
  for (int i = 0; i < 10; i++) result++;
  return result;
}

int flatten_before_while() {
  int result = 0;
  int i = 0;
  [flatten] // expected-warning {{attribute 'flatten' can only be applied to 'if' and 'switch' statements}} fxc-pass {{}}
  while(i < 10) {
    result++;
    i++;
  }
  return result;
}

int flatten_before_do() {
  int result = 0;
  int i = 0;
  [flatten] // expected-warning {{attribute 'flatten' can only be applied to 'if' and 'switch' statements}} fxc-pass {{}}
  do {
    result++;
    i++;
  } while(i < 10);
  return result;
}

int forcecase_before_for() {
  int result = 0;
  [forcecase] // expected-warning {{attribute 'forcecase' can only be applied to 'switch' statements}} fxc-pass {{}}
  for (int i = 0; i < 10; i++) result++;
  return result;
}

int forcecase_before_while() {
  int result = 0;
  int i = 0;
  [forcecase] // expected-warning {{attribute 'forcecase' can only be applied to 'switch' statements}} fxc-pass {{}}
  while(i < 10) {
    result++;
    i++;
  }
  return result;
}

int forcecase_before_do() {
  int result = 0;
  int i = 0;
  [forcecase] // expected-warning {{attribute 'forcecase' can only be applied to 'switch' statements}} fxc-pass {{}}
  do {
    result++;
    i++;
  } while(i < 10);
  return result;
}

int forcecase_before_if(int a) {
  [forcecase] // expected-warning {{attribute 'forcecase' can only be applied to 'switch' statements}} fxc-pass {{}}
  if (a > 0) return -a;
  return a;
}

int call_before_for() {
  int result = 0;
  [call] // expected-warning {{attribute 'call' can only be applied to 'switch' statements}} fxc-pass {{}}
  for (int i = 0; i < 10; i++) result++;
  return result;
}

int call_before_while() {
  int result = 0;
  int i = 0;
  [call] // expected-warning {{attribute 'call' can only be applied to 'switch' statements}} fxc-pass {{}}
  while(i < 10) {
    result++;
    i++;
  }
  return result;
}

int call_before_do() {
  int result = 0;
  int i = 0;
  [call] // expected-warning {{attribute 'call' can only be applied to 'switch' statements}} fxc-pass {{}}
  do {
    result++;
    i++;
  } while(i < 10);
  return result;
}

int call_before_if(int a) {
  [call] // expected-warning {{attribute 'call' can only be applied to 'switch' statements}} fxc-pass {{}}
  if (a > 0) return -a;
  return a;
}

int short_unroll() {
  int result = 2;

  [unroll(2)] for (int i = 0; i < 100; i++) result++;
  /*verify-ast
    AttributedStmt <col:3, col:51>
    |-HLSLUnrollAttr <col:4, col:12> 2
    `-ForStmt <col:15, col:51>
      |-DeclStmt <col:20, col:29>
      | `-VarDecl <col:20, col:28> col:24 used i 'int' cinit
      |   `-ImplicitCastExpr <col:28> 'int' <IntegralCast>
      |     `-IntegerLiteral <col:28> 'literal int' 0
      |-<<<NULL>>>
      |-BinaryOperator <col:31, col:35> 'const bool' '<'
      | |-ImplicitCastExpr <col:31> 'int' <LValueToRValue>
      | | `-DeclRefExpr <col:31> 'int' lvalue Var 'i' 'int'
      | `-ImplicitCastExpr <col:35> 'int' <IntegralCast>
      |   `-IntegerLiteral <col:35> 'literal int' 100
      |-UnaryOperator <col:40, col:41> 'int' postfix '++'
      | `-DeclRefExpr <col:40> 'int' lvalue Var 'i' 'int'
      `-UnaryOperator <col:45, col:51> 'int' postfix '++'
        `-DeclRefExpr <col:45> 'int' lvalue Var 'result' 'int'
  */

  [unroll("2")] // expected-error {{'unroll' attribute requires an integer constant}} fxc-warning {{X3554: cannot match attribute unroll, parameter 1 is expected to be of type int}} fxc-warning {{X3554: unknown attribute unroll, or attribute invalid for this statement, valid attributes are: loop, fastopt, unroll, allow_uav_condition}}
  for (int j = 0; j < 100; j++) result++;

  return result;
}

int long_unroll() {
  int result = 2;

  [unroll(200)]
  for (int i = 0; i < 100; i++) result++;

  return result;
}

int neg_unroll() {
  int result = 2;

  // fxc error X3084: cannot match attribute unroll, non-uint parameters found
  [unroll(-1)] // expected-warning {{attribute 'unroll' must have a uint literal argument}} fxc-error {{X3084: cannot match attribute unroll, non-uint parameters found}}
  /*verify-ast
    AttributedStmt <col:3, line:277:39>
    |-HLSLUnrollAttr <col:4, col:13> -1
    `-ForStmt <line:277:3, col:39>
      |-DeclStmt <col:8, col:17>
      | `-VarDecl <col:8, col:16> col:12 used i 'int' cinit
      |   `-ImplicitCastExpr <col:16> 'int' <IntegralCast>
      |     `-IntegerLiteral <col:16> 'literal int' 0
      |-<<<NULL>>>
      |-BinaryOperator <col:19, col:23> 'const bool' '<'
      | |-ImplicitCastExpr <col:19> 'int' <LValueToRValue>
      | | `-DeclRefExpr <col:19> 'int' lvalue Var 'i' 'int'
      | `-ImplicitCastExpr <col:23> 'int' <IntegralCast>
      |   `-IntegerLiteral <col:23> 'literal int' 100
      |-UnaryOperator <col:28, col:29> 'int' postfix '++'
      | `-DeclRefExpr <col:28> 'int' lvalue Var 'i' 'int'
      `-UnaryOperator <col:33, col:39> 'int' postfix '++'
        `-DeclRefExpr <col:33> 'int' lvalue Var 'result' 'int'
  */
  for (int i = 0; i < 100; i++) result++;

  return result;
}

int flt_unroll() {
  int result = 2;

  // fxc warning X3554: cannot match attribute unroll, parameter 1 is expected to be of type int
  // fxc warning X3554: unknown attribute unroll, or attribute invalid for this statement, valid attributes are: loop, fastopt, unroll, allow_uav_condition
  [unroll(1.5)] // expected-warning {{attribute 'unroll' must have a uint literal argument}} fxc-warning {{X3554: cannot match attribute unroll, parameter 1 is expected to be of type int}} fxc-warning {{X3554: unknown attribute unroll, or attribute invalid for this statement, valid attributes are: loop, fastopt, unroll, allow_uav_condition}}
  /*verify-ast
    AttributedStmt <col:3, line:307:39>
    |-HLSLUnrollAttr <col:4, col:14> 0
    `-ForStmt <line:307:3, col:39>
      |-DeclStmt <col:8, col:17>
      | `-VarDecl <col:8, col:16> col:12 used i 'int' cinit
      |   `-ImplicitCastExpr <col:16> 'int' <IntegralCast>
      |     `-IntegerLiteral <col:16> 'literal int' 0
      |-<<<NULL>>>
      |-BinaryOperator <col:19, col:23> 'const bool' '<'
      | |-ImplicitCastExpr <col:19> 'int' <LValueToRValue>
      | | `-DeclRefExpr <col:19> 'int' lvalue Var 'i' 'int'
      | `-ImplicitCastExpr <col:23> 'int' <IntegralCast>
      |   `-IntegerLiteral <col:23> 'literal int' 100
      |-UnaryOperator <col:28, col:29> 'int' postfix '++'
      | `-DeclRefExpr <col:28> 'int' lvalue Var 'i' 'int'
      `-UnaryOperator <col:33, col:39> 'int' postfix '++'
        `-DeclRefExpr <col:33> 'int' lvalue Var 'result' 'int'
  */
  for (int i = 0; i < 100; i++) result++;

  return result;
}

RWByteAddressBuffer bab;
int bab_address;
bool g_bool;
uint g_uint;
uint g_dealiasTableOffset;
uint g_dealiasTableSize;

int uav() {
  // fxc not complaining.
  //int result = bab_address;
  //while (bab.Load(result) != 0) {
  //  bab.Store(bab_address, g_dealiasTableOffset - result);
  //  result++;
  //}
  //return result;
  uint i;
  [allow_uav_condition]
  /*verify-ast
    AttributedStmt <col:3, line:348:3>
    |-HLSLAllowUAVConditionAttr <col:4>
    `-ForStmt <line:347:3, line:348:3>
      |-BinaryOperator <col:8, col:12> 'uint':'unsigned int' '='
      | |-DeclRefExpr <col:8> 'uint':'unsigned int' lvalue Var 'i' 'uint':'unsigned int'
      | `-ImplicitCastExpr <col:12> 'uint':'unsigned int' <LValueToRValue>
      |   `-DeclRefExpr <col:12> 'const uint':'const unsigned int' lvalue Var 'g_dealiasTableOffset' 'const uint':'const unsigned int'
      |-<<<NULL>>>
      |-BinaryOperator <col:34, col:38> 'const bool' '<'
      | |-ImplicitCastExpr <col:34> 'uint':'unsigned int' <LValueToRValue>
      | | `-DeclRefExpr <col:34> 'uint':'unsigned int' lvalue Var 'i' 'uint':'unsigned int'
      | `-ImplicitCastExpr <col:38> 'uint':'unsigned int' <LValueToRValue>
      |   `-DeclRefExpr <col:38> 'const uint':'const unsigned int' lvalue Var 'g_dealiasTableSize' 'const uint':'const unsigned int'
      |-UnaryOperator <col:58, col:60> 'uint':'unsigned int' lvalue prefix '++'
      | `-DeclRefExpr <col:60> 'uint':'unsigned int' lvalue Var 'i' 'uint':'unsigned int'
      `-CompoundStmt <col:63, line:348:3>
  */
  for (i = g_dealiasTableOffset; i < g_dealiasTableSize; ++i) {
  }
  return i;
}

[domain] int domain_fn_missing() { return 1; }          // expected-error {{'domain' attribute takes one argument}} fxc-pass {{}}
[domain()] int domain_fn_empty() { return 1; }          // expected-error {{'domain' attribute takes one argument}} fxc-error {{X3000: syntax error: unexpected token ')'}}
[domain("blerch")]  int domain_fn_bad() { return 1; }    // expected-error {{attribute 'domain' must have one of these values: tri,quad,isoline}} fxc-pass {{}} 
[domain("quad")]  int domain_fn() { return 1; }          /* fxc-warning {{X3554: unknown attribute domain, or attribute invalid for this statement}} */
[domain(1)]  int domain_fn_int() { return 1; }           // expected-error {{attribute 'domain' must have a string literal argument}} fxc-pass {{}}
[domain("quad","quad")] int domain_fn_mul() { return 1; } // expected-error {{'domain' attribute takes one argument}} fxc-pass {{}}
[instance] int instance_fn() { return 1; }             // expected-error {{'instance' attribute takes one argument}} fxc-warning {{X3554: unknown attribute instance, or attribute invalid for this statement}}
[maxtessfactor] int maxtessfactor_fn() { return 1; }   // expected-error {{'maxtessfactor' attribute takes one argument}} fxc-warning {{X3554: unknown attribute maxtessfactor, or attribute invalid for this statement}}
[numthreads] int numthreads_fn() { return 1; }         // expected-error {{'numthreads' attribute requires exactly 3 arguments}} fxc-warning {{X3554: unknown attribute numthreads, or attribute invalid for this statement}}
[outputcontrolpoints] int outputcontrolpoints_fn() { return 1; } // expected-error {{'outputcontrolpoints' attribute takes one argument}} fxc-warning {{X3554: unknown attribute outputcontrolpoints, or attribute invalid for this statement}}
[outputtopology] int outputtopology_fn() { return 1; } // expected-error {{'outputtopology' attribute takes one argument}} fxc-warning {{X3554: unknown attribute outputtopology, or attribute invalid for this statement}}
[partitioning] int partitioning_fn() { return 1; }     // expected-error {{'partitioning' attribute takes one argument}} fxc-warning {{X3554: unknown attribute partitioning, or attribute invalid for this statement}}
[patchconstantfunc] int patchconstantfunc_fn() { return 1; } // expected-error {{'patchconstantfunc' attribute takes one argument}} fxc-warning {{X3554: unknown attribute patchconstantfunc, or attribute invalid for this statement}}

[partitioning("fractional_even")]  int partitioning_fn_ok() { return 1; }

struct HSFoo
{
    float3 pos : POSITION;
};

Texture2D<float4> tex1[10] : register( t20, space10 );
/*verify-ast
  VarDecl <col:1, col:26> col:19 used tex1 'Texture2D<float4> [10]'
  `-RegisterAssignment <col:30> register(t20, space10)
*/

[domain(123)]     // expected-error {{attribute 'domain' must have a string literal argument}} fxc-pass {{}}
[partitioning()]  // expected-error {{'partitioning' attribute takes one argument}} fxc-error {{X3000: syntax error: unexpected token ')'}}
[outputtopology("not_triangle_cw")] // expected-error {{attribute 'outputtopology' must have one of these values: point,line,triangle,triangle_cw,triangle_ccw}} fxc-pass {{}}
[outputcontrolpoints(-1)] // expected-warning {{attribute 'outputcontrolpoints' must have a uint literal argument}} expected-error {{number of control points -1 is outside the valid range of [1..32]}} fxc-pass {{}}
[patchconstantfunc("PatchFoo", "ExtraArgument")] // expected-error {{'patchconstantfunc' attribute takes one argument}} fxc-pass {{}}

void all_wrong() { }

[shader("hull")]
void hull_wrong() { } // expected-error {{hull entry point must have a valid patchconstantfunc attribute}} expected-error {{hull entry point must have a valid outputtopology attribute}} expected-error {{hull entry point must have a valid outputcontrolpoints attribute}}

[domain("quad")]
/*verify-ast
  HLSLDomainAttr <col:2, col:15> "quad"
*/

[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(16)]
[patchconstantfunc("PatchFoo")]

HSFoo HSMain( InputPatch<HSFoo, 16> p,
/*verify-ast
  FunctionDecl <col:1, line:468:1> line:394:7 HSMain 'HSFoo (InputPatch<HSFoo, 16>, uint, uint)'
  |-ParmVarDecl <col:15, col:37> col:37 used p 'InputPatch<HSFoo, 16>':'InputPatch<HSFoo, 16>'
  |-ParmVarDecl <line:460:15, col:20> col:20 used i 'uint':'unsigned int'
  | `-SemanticDecl <col:24> "SV_OutputControlPointID"
  |-ParmVarDecl <line:461:15, col:20> col:20 used PatchID 'uint':'unsigned int'
  | `-SemanticDecl <col:30> "SV_PrimitiveID"
  |-CompoundStmt <line:462:1, line:468:1>
  | |-DeclStmt <line:463:5, col:17>
  | | `-VarDecl <col:5, col:11> col:11 used output 'HSFoo'
  | |-DeclStmt <line:464:5, col:41>
  | | `-VarDecl <col:5, col:40> col:12 used r 'float4':'vector<float, 4>' cinit
  | |   `-CXXFunctionalCastExpr <col:16, col:40> 'float4':'vector<float, 4>' functional cast to float4 <NoOp>
  | |     `-InitListExpr <col:22, col:40> 'float4':'vector<float, 4>'
  | |       |-ImplicitCastExpr <col:23, col:34> 'const float3':'const vector<float, 3>' <LValueToRValue>
  | |       | `-MemberExpr <col:23, col:34> 'const float3':'const vector<float, 3>' lvalue .pos
  | |       |   `-CXXOperatorCallExpr <col:23, col:32> 'const HSFoo' lvalue
  | |       |     |-ImplicitCastExpr <col:24, col:32> 'const HSFoo &(*)(unsigned int) const' <FunctionToPointerDecay>
  | |       |     | `-DeclRefExpr <col:24, col:32> 'const HSFoo &(unsigned int) const' lvalue CXXMethod 'operator[]' 'const HSFoo &(unsigned int) const'
  | |       |     |-ImplicitCastExpr <col:23> 'const InputPatch<HSFoo, 16>' lvalue <NoOp>
  | |       |     | `-DeclRefExpr <col:23> 'InputPatch<HSFoo, 16>':'InputPatch<HSFoo, 16>' lvalue ParmVar 'p' 'InputPatch<HSFoo, 16>':'InputPatch<HSFoo, 16>'
  | |       |     `-ImplicitCastExpr <col:25> 'uint':'unsigned int' <LValueToRValue>
  | |       |       `-DeclRefExpr <col:25> 'uint':'unsigned int' lvalue ParmVar 'PatchID' 'uint':'unsigned int'
  | |       `-ImplicitCastExpr <col:39> 'float' <IntegralToFloating>
  | |         `-IntegerLiteral <col:39> 'literal int' 1
  | |-CompoundAssignOperator <line:465:5, col:30> 'float4':'vector<float, 4>' lvalue '+=' ComputeLHSTy='float4':'vector<float, 4>' ComputeResultTy='float4':'vector<float, 4>'
  | | |-DeclRefExpr <col:5> 'float4':'vector<float, 4>' lvalue Var 'r' 'float4':'vector<float, 4>'
  | | `-CXXMemberCallExpr <col:10, col:30> 'vector<float, 4>'
  | |   |-MemberExpr <col:10, col:20> '<bound member function type>' .Load
  | |   | `-ArraySubscriptExpr <col:10, col:18> 'Texture2D<float4>':'Texture2D<vector<float, 4> >' lvalue
  | |   |   |-ImplicitCastExpr <col:10> 'Texture2D<float4> [10]' <LValueToRValue>
  | |   |   | `-DeclRefExpr <col:10> 'Texture2D<float4> [10]' lvalue Var 'tex1' 'Texture2D<float4> [10]'
  | |   |   `-ImplicitCastExpr <col:15, col:17> 'unsigned int' <FloatingToIntegral>
  | |   |     `-ImplicitCastExpr <col:15, col:17> 'float' <LValueToRValue>
  | |   |       `-HLSLVectorElementExpr <col:15, col:17> 'float' lvalue vectorcomponent x
  | |   |         `-DeclRefExpr <col:15> 'float4':'vector<float, 4>' lvalue Var 'r' 'float4':'vector<float, 4>'
  | |   `-ImplicitCastExpr <col:25, col:27> 'vector<int, 3>' <HLSLCC_FloatingToIntegral>
  | |     `-ImplicitCastExpr <col:25, col:27> 'vector<float, 3>':'vector<float, 3>' <LValueToRValue>
  | |       `-HLSLVectorElementExpr <col:25, col:27> 'vector<float, 3>':'vector<float, 3>' lvalue vectorcomponent xyz
  | |         `-DeclRefExpr <col:25> 'float4':'vector<float, 4>' lvalue Var 'r' 'float4':'vector<float, 4>'
  | |-BinaryOperator <line:466:5, col:31> 'float3':'vector<float, 3>' '='
  | | |-MemberExpr <col:5, col:12> 'float3':'vector<float, 3>' lvalue .pos
  | | | `-DeclRefExpr <col:5> 'HSFoo' lvalue Var 'output' 'HSFoo'
  | | `-BinaryOperator <col:18, col:31> 'float3':'vector<float, 3>' '+'
  | |   |-ImplicitCastExpr <col:18, col:23> 'float3':'vector<float, 3>' <LValueToRValue>
  | |   | `-MemberExpr <col:18, col:23> 'const float3':'const vector<float, 3>' lvalue .pos
  | |   |   `-CXXOperatorCallExpr <col:18, col:21> 'const HSFoo' lvalue
  | |   |     |-ImplicitCastExpr <col:19, col:21> 'const HSFoo &(*)(unsigned int) const' <FunctionToPointerDecay>
  | |   |     | `-DeclRefExpr <col:19, col:21> 'const HSFoo &(unsigned int) const' lvalue CXXMethod 'operator[]' 'const HSFoo &(unsigned int) const'
  | |   |     |-ImplicitCastExpr <col:18> 'const InputPatch<HSFoo, 16>' lvalue <NoOp>
  | |   |     | `-DeclRefExpr <col:18> 'InputPatch<HSFoo, 16>':'InputPatch<HSFoo, 16>' lvalue ParmVar 'p' 'InputPatch<HSFoo, 16>':'InputPatch<HSFoo, 16>'
  | |   |     `-ImplicitCastExpr <col:20> 'uint':'unsigned int' <LValueToRValue>
  | |   |       `-DeclRefExpr <col:20> 'uint':'unsigned int' lvalue ParmVar 'i' 'uint':'unsigned int'
  | |   `-ImplicitCastExpr <col:29, col:31> 'vector<float, 3>':'vector<float, 3>' <LValueToRValue>
  | |     `-HLSLVectorElementExpr <col:29, col:31> 'vector<float, 3>':'vector<float, 3>' lvalue vectorcomponent xyz
  | |       `-DeclRefExpr <col:29> 'float4':'vector<float, 4>' lvalue Var 'r' 'float4':'vector<float, 4>'
  | `-ReturnStmt <line:467:5, col:12>
  |   `-ImplicitCastExpr <col:12> 'HSFoo' <LValueToRValue>
  |     `-DeclRefExpr <col:12> 'HSFoo' lvalue Var 'output' 'HSFoo'
  |-HLSLPatchConstantFuncAttr <line:393:2, col:30> "PatchFoo"
  |-HLSLOutputControlPointsAttr <line:392:2, col:24> 16
  |-HLSLOutputTopologyAttr <line:391:2, col:30> "triangle_cw"
  |-HLSLPartitioningAttr <line:390:2, col:24> "integer"
  `-HLSLDomainAttr <line:386:2, col:15> "quad"
*/
              uint i : SV_OutputControlPointID,
              uint PatchID : SV_PrimitiveID )
{
    HSFoo output;
    float4 r = float4(p[PatchID].pos, 1);
    r += tex1[r.x].Load(r.xyz);
    output.pos = p[i].pos + r.xyz;
    return output;
}

float4 cp4[2];
int4 i4;
float4 cp5;
float4x4 m4;
float f;

struct global_struct { float4 cp5[5]; };

struct main_output
{
  //float4 t0 : SV_Target0;
  float4 p0 : SV_Position0;
};

float4 myexpr() { return cp5; }
static const float4 f4_const = float4(1, 2, 3, 4);
bool b;
int clip_index;
static const bool b_true = true;
global_struct gs;
float4 f4;

//
// Things we can't do in clipplanes:
//
// - use literal in any part of expression
// - use static const in any part of expression
// - use ternary operator
// - use binary operator
// - make function calls
// - use indexed expressions
// - implicit conversions
// - explicit conversions
//

// fxc error X3084: Clip plane attribute parameters must be non-literal constants
[clipplanes(float4(1, 2, f, 4))] // expected-error {{invalid expression for clipplanes argument: must be global reference, member access or array subscript}} fxc-pass {{}}
void clipplanes_literals();

[clipplanes(b)]           // expected-error {{clipplanes argument must be a float4 type but is 'const bool'}} fxc-pass {{}}
void clipplanes_const();

// fxc error X3084: Clip plane attribute parameters must be non-literal constants
[clipplanes(f4_const)]    // expected-error {{invalid expression for clipplanes argument: must be global reference, member access or array subscript}} fxc-pass {{}}
void clipplanes_const();

// fxc error X3084: Clip plane attribute parameters must be non-literal constants
[clipplanes(b ? cp4[clip_index] : cp4[clip_index])] // expected-error {{invalid expression for clipplanes argument: must be global reference, member access or array subscript}} fxc-pass {{}}
void clipplanes_bad_ternary();

// fxc error X3084: Clip plane attribute parameters must be non-literal constants
[clipplanes(cp5 + cp5)]      // expected-error {{invalid expression for clipplanes argument: must be global reference, member access or array subscript}} fxc-pass {{}}
void clipplanes_bad_binop();

// fxc error X3682: expressions with side effects are illegal as attribute parameters
[clipplanes(myexpr())]      // expected-error {{invalid expression for clipplanes argument: must be global reference, member access or array subscript}} fxc-pass {{}}
void clipplanes_bad_call();

// fxc error X3084: Indexed expressions are illegal as attribute parameters
[clipplanes(cp4[clip_index])] // expected-error {{invalid expression for clipplanes argument array subscript: must be numeric literal}} fxc-pass {{}}
void clipplanes_bad_indexed();

//
// Thing we can do in clipplanes:
//
// - index into array
// - index into vector
// - swizzle (even into an rvalue, even from scalar)
// - member access
// - constructors
//

// index into swizzle into r-value - allowed in fxc, disallowed in new compiler
[clipplanes(cp4[0].xyzw)]               // expected-error {{invalid expression for clipplanes argument: must be global reference, member access or array subscript}} fxc-pass {{}}
void clipplanes_bad_swizzle();

// index into vector - allowed in fxc, disallowed in new compiler
[clipplanes(cp4[0][0].xxxx)]            // expected-error {{invalid expression for clipplanes argument: must be global reference, member access or array subscript}} fxc-pass {{}}
void clipplanes_bad_vector_index();

// index into matrix - allowed in fxc, disallowed in new compiler
[clipplanes(m4[0])]                     // expected-error {{invalid expression for clipplanes argument: must be global reference, member access or array subscript}} fxc-pass {{}}
void clipplanes_bad_matrix_index();

// constructor - allowed in fxc, disallowed in new compiler
[clipplanes(float4(f, f, f, f))]        // expected-error {{invalid expression for clipplanes argument: must be global reference, member access or array subscript}} fxc-pass {{}}
void clipplanes_bad_matrix_index();

// swizzle from scalar - allowed in fxc, disallowed in new compiler
[clipplanes(f.xxxx)]                    // expected-error {{invalid expression for clipplanes argument: must be global reference, member access or array subscript}} fxc-pass {{}}
void clipplanes_bad_scalar_swizzle();

[clipplanes(
/*verify-ast
  HLSLClipPlanesAttr <col:2, line:582:3>
  |-DeclRefExpr <line:579:3> 'const float4':'const vector<float, 4>' lvalue Var 'f4' 'const float4':'const vector<float, 4>'
  |-ArraySubscriptExpr <line:580:3, col:8> 'float4':'vector<float, 4>' lvalue
  | |-ImplicitCastExpr <col:3> 'const float4 [2]' <LValueToRValue>
  | | `-DeclRefExpr <col:3> 'const float4 [2]' lvalue Var 'cp4' 'const float4 [2]'
  | `-IntegerLiteral <col:7> 'literal int' 0
  |-ArraySubscriptExpr <line:581:3, col:11> 'const float4':'const vector<float, 4>' lvalue
  | |-ImplicitCastExpr <col:3, col:6> 'float4 const[5]' <LValueToRValue>
  | | `-MemberExpr <col:3, col:6> 'float4 const[5]' lvalue .cp5
  | |   `-DeclRefExpr <col:3> 'const global_struct' lvalue Var 'gs' 'const global_struct'
  | `-IntegerLiteral <col:10> 'literal int' 2
  |-<<<NULL>>>
  |-<<<NULL>>>
  `-<<<NULL>>>
*/
  f4,         // simple global reference
  cp4[0],     // index into array
  gs.cp5[2]   // use '.' operator
  )]
float4 clipplanes_good();


[clipplanes(
/*verify-ast
  HLSLClipPlanesAttr <col:2, line:616:3>
  |-ParenExpr <line:612:3, col:6> 'const float4':'const vector<float, 4>' lvalue
  | `-DeclRefExpr <col:4> 'const float4':'const vector<float, 4>' lvalue Var 'f4' 'const float4':'const vector<float, 4>'
  |-ArraySubscriptExpr <line:613:3, col:10> 'float4':'vector<float, 4>' lvalue
  | |-ImplicitCastExpr <col:3> 'const float4 [2]' <LValueToRValue>
  | | `-DeclRefExpr <col:3> 'const float4 [2]' lvalue Var 'cp4' 'const float4 [2]'
  | `-ParenExpr <col:7, col:9> 'literal int'
  |   `-IntegerLiteral <col:8> 'literal int' 0
  |-ArraySubscriptExpr <line:614:3, col:13> 'const float4':'const vector<float, 4>' lvalue
  | |-ImplicitCastExpr <col:3, col:8> 'float4 const[5]' <LValueToRValue>
  | | `-MemberExpr <col:3, col:8> 'float4 const[5]' lvalue .cp5
  | |   `-ParenExpr <col:3, col:6> 'const global_struct' lvalue
  | |     `-DeclRefExpr <col:4> 'const global_struct' lvalue Var 'gs' 'const global_struct'
  | `-IntegerLiteral <col:12> 'literal int' 2
  |-ParenExpr <line:615:3, col:15> 'const float4':'const vector<float, 4>' lvalue
  | `-ArraySubscriptExpr <col:4, col:14> 'const float4':'const vector<float, 4>' lvalue
  |   |-ImplicitCastExpr <col:4, col:9> 'float4 const[5]' <LValueToRValue>
  |   | `-MemberExpr <col:4, col:9> 'float4 const[5]' lvalue .cp5
  |   |   `-ParenExpr <col:4, col:7> 'const global_struct' lvalue
  |   |     `-DeclRefExpr <col:5> 'const global_struct' lvalue Var 'gs' 'const global_struct'
  |   `-IntegerLiteral <col:13> 'literal int' 2
  |-<<<NULL>>>
  `-<<<NULL>>>
*/
  (f4),         // simple global reference
  cp4[(0)],     // index into array
  (gs).cp5[2],  // use '.' operator
  ((gs).cp5[2]) // use '.' operator
  )]
float4 clipplanes_good_parens();


// GS Attribute maxvertexcount
// Note: fxc only reports errors when function is the entry, therefore, I will
// place the errors in comments before the function, but not with the standard
// fxc error comments on the line.
struct GSVertex { float4 pos : SV_Position; };
[maxvertexcount (12)]

/*verify-ast
  HLSLMaxVertexCountAttr <col:2, col:20> 12
*/
void maxvertexcount_valid1(triangle GSVertex v[3], inout TriangleStream<GSVertex> stream)
{ stream.Append(v[0]); }

static const int sc_count = 12;
[maxvertexcount (sc_count)]

/*verify-ast
  HLSLMaxVertexCountAttr <col:2, col:26> 12
*/
void maxvertexcount_valid2(triangle GSVertex v[3], inout TriangleStream<GSVertex> stream)
{ stream.Append(v[0]); }
[maxvertexcount (sc_count + 3)]

/*verify-ast
  HLSLMaxVertexCountAttr <col:2, col:30> 15
*/
void maxvertexcount_valid3(triangle GSVertex v[3], inout TriangleStream<GSVertex> stream)
{ stream.Append(v[0]); }

static const int4 sc_count4 = int4(3,6,9,12);

// The following passes fxc, but fails clang.
[maxvertexcount (sc_count4.w)]          /* expected-error {{'maxvertexcount' attribute requires an integer constant}} fxc-pass {{}} */

/*verify-ast
  HLSLMaxVertexCountAttr <col:2, col:29> 0
*/
void maxvertexcount_valid4(triangle GSVertex v[3], inout TriangleStream<GSVertex> stream)
{ stream.Append(v[0]); }

// fxc:
// error X3084: cannot match attribute maxvertexcount, non-uint parameters found
[maxvertexcount (-12)]                  /* expected-warning {{attribute 'maxvertexcount' must have a uint literal argument}} fxc-pass {{}} */

void negative_maxvertexcount(triangle GSVertex v[3], inout TriangleStream<GSVertex> stream)
{ stream.Append(v[0]); }

// fxc:
// warning X3554: cannot match attribute maxvertexcount, parameter 1 is expected to be of type int
// warning X3554: unknown attribute maxvertexcount, or attribute invalid for this statement, valid attributes are: maxvertexcount, MaxVertexCount, instance, RootSignature
// error X3514: 'float_maxvertexcount1' must have a max vertex count
[maxvertexcount (1.5)]                  /* expected-warning {{attribute 'maxvertexcount' must have a uint literal argument}} fxc-pass {{}} */

void float_maxvertexcount1(triangle GSVertex v[3], inout TriangleStream<GSVertex> stream)
{ stream.Append(v[0]); }

// fxc:
// warning X3554: cannot match attribute maxvertexcount, parameter 1 is expected to be of type int
// warning X3554: unknown attribute maxvertexcount, or attribute invalid for this statement, valid attributes are: maxvertexcount, MaxVertexCount, instance, RootSignature
// error X3514: 'float_maxvertexcount2' must have a max vertex count
static const float sc_float = 1.5;
[maxvertexcount (sc_float)]             /* expected-error {{'maxvertexcount' attribute requires an integer constant}} fxc-pass {{}} */

void float_maxvertexcount2(triangle GSVertex v[3], inout TriangleStream<GSVertex> stream)
{ stream.Append(v[0]); }

int i_count;
float f_count;

// fxc:
// error X3084: non-literal parameter(s) found for attribute maxvertexcount
// error X3514: 'uniform_maxvertexcount1' must have a max vertex count
[maxvertexcount (i_count)]              /* expected-error {{'maxvertexcount' attribute requires an integer constant}} fxc-pass {{}} */

void uniform_maxvertexcount1(triangle GSVertex v[3], inout TriangleStream<GSVertex> stream)
{ stream.Append(v[0]); }

// fxc:
// warning X3554: cannot match attribute maxvertexcount, parameter 1 is expected to be of type int
// warning X3554: unknown attribute maxvertexcount, or attribute invalid for this statement, valid attributes are: maxvertexcount, MaxVertexCount, instance, RootSignature
// error X3514: 'uniform_maxvertexcount2' must have a max vertex count
[maxvertexcount (f_count)]              /* expected-error {{'maxvertexcount' attribute requires an integer constant}} fxc-pass {{}} */

void uniform_maxvertexcount2(triangle GSVertex v[3], inout TriangleStream<GSVertex> stream)
{ stream.Append(v[0]); }


[shader("pixel")]
[earlydepthstencil]
/*verify-ast
  HLSLEarlyDepthStencilAttr <col:2>
*/
float4 main() : SV_Target0 {
    int val = 2;

    [earlydepthstencil] // expected-error {{attribute is valid only on functions}} fxc-pass {{}}
    float2 f2 = { 1, 2 };

    [loop]
    for (int i = 0; i < 4; i++) { // expected-note {{previous definition is here}} fxc-pass {{}}
        val *= 2;
    }

    [unroll]
    // fxc warning X3078: 'i': loop control variable conflicts with a previous declaration in the outer scope; most recent declaration will be used
    for (int i = 0; i < 4; i++) { // expected-warning {{redefinition of 'i' shadows declaration in the outer scope; most recent declaration will be used}} fxc-warning {{X3078: 'i': loop control variable conflicts with a previous declaration in the outer scope; most recent declaration will be used}}
        val *= 2;
    }

    // fxc error X3524: can't use loop and unroll attributes together
    [loop][unroll] // expected-error {{loop and unroll attributes are not compatible}} fxc-error {{X3524: can't use loop and unroll attributes together}}
    for (int k = 0; k < 4; k++) { val *= 2; } // expected-note {{previous definition is here}} fxc-pass {{}}

    // fxc error X3524: can't use fastopt and unroll attributes together
    [fastopt][unroll] // expected-error {{fastopt and unroll attributes are not compatible}} fxc-error {{X3524: can't use fastopt and unroll attributes together}}
    for (int k = 0; k < 4; k++) { val *= 2; } // expected-warning {{redefinition of 'k' shadows declaration in the outer scope; most recent declaration will be used}} fxc-pass {{}}

    loop_before_assignment();
    loop_before_return();

    val = 2;

    [loop]
    do { val *= 2; } while (val < 10);

    [fastopt]
    while (val > 10) { val--; }

    [branch]
    if (g_bool) {
      val += 4;
    }

    [flatten]
    if (!g_bool) {
      val += 4;
    }

    [flatten]
    switch (g_uint) {
      // fxc error X3533: non-empty case statements must have break or return
      // case 0: val += 100;
      case 1: val += 101; break;
      case 2:
      case 3: val += 102; break;
      break;
    }

    [branch]
    switch (g_uint) {
      case 1: val += 101; break;
      case 2:
      case 3: val += 102; break;
      break;
    }

    [forcecase]
    switch (g_uint) {
      case 1: val += 101; break;
      case 2:
      case 3: val += 102; break;
      break;
    }

    [call]
    switch (g_uint) {
      case 1: val += 101; break;
      case 2:
      case 3: val += 102; break;
      break;
    }

    val += domain_fn();
    val += instance_fn();
    val += maxtessfactor_fn();
    val += numthreads_fn();
    val += outputcontrolpoints_fn();
    val += outputtopology_fn();
    val += partitioning_fn();
    val += patchconstantfunc_fn();

    val += long_unroll();
    val += short_unroll();
    val += neg_unroll();
    val += flt_unroll();
    val += uav();

    return val;
}

// Test support for title case in attributes.
// Test Allow_UAV_Condition
bool Test_Allow_UAV_Condition() {
  [Allow_UAV_Condition]
  while (1) { return true; }
}

// Test Branch
bool Test_Branch() {
  [Branch]
  if (g_bool) return true; else return false;
}

// Test Call
bool Test_Call() {
  [Call]
  switch (g_uint) {
  case 1: return true;
  default: return false;
  }
}

// Test EarlyDepthStencil
[EarlyDepthStencil]

bool Test_EarlyDepthStencil() {
  return true;
}

// Test FastOpt
bool Test_FastOpt() {
  [FastOpt] while (g_bool) return g_bool;
}

// Test Flatten
bool Test_Flatten() {
  [Flatten] if (g_bool) return true; else return false;
}

// Test Forcecase
bool Test_Forcecase() {
  [ForceCase]
  switch (g_uint) {
  case 1: return true;
  default: return false;
  }
}

// Test Loop
bool Test_Loop() {
  [Loop] while (g_bool) return g_bool;
}

// Test ClipPlanes
float4 ClipPlanesVal;
[ClipPlanes(ClipPlanesVal)]

bool Test_ClipPlanes() {
  return true;
}

// Test Domain
[Domain("tri")]

bool Test_Domain() {
  return true;
}

// Test Instance
[Instance(1)]

bool Test_Instance() {
  return true;
}

// Test MaxTessFactor
[MaxTessFactor(1)]

bool Test_MaxTessFactor() {
  return true;
}

// Test MaxVertexCount
[MaxVertexCount(1)]

bool Test_MaxVertexCount() {
  return true;
}

// Test NumThreads
[NumThreads(1,2,3)]

bool Test_NumThreads() {
  return true;
}

// Test OutputControlPoints
[OutputControlPoints(2)]

bool Test_OutputControlPoints() {
  return true;
}

// Test OutputTopology
[OutputTopology("line")]

bool Test_OutputTopology() {
  return true;
}

// Test Partitioning
[Partitioning("integer")]

bool Test_Partitioning() {
  return true;
}

// Test PatchConstantFunc
[PatchConstantFunc("Test_Partitioning")]

bool Test_PatchConstantFunc() {
  return true;
}

// Test RootSignature
// strange how RootSignature is the only attribute that is spelled with capitals.
[RootSignature("")]

bool Test_RootSignature() {
  return true;
}

// Test Unroll
bool Test_Unroll() {
  [Unroll] while (g_bool) return g_bool;
}

// Test NoInline
[noinline] bool Test_noinline() {
  [noinline] bool b = false;                  /* expected-warning {{'noinline' attribute only applies to functions}} fxc-pass {{}} */
  [noinline] while (g_bool) return g_bool;    /* expected-error {{'noinline' attribute cannot be applied to a statement}} fxc-pass {{}} */
  return true;
}

// Test unknown attribute warning
bool Test_Unknown() {
  [unknown] bool b1 = false;                  /* expected-warning {{unknown attribute 'unknown' ignored}} fxc-pass {{}} */
  [unknown(1)] bool b2 = false;               /* expected-warning {{unknown attribute 'unknown' ignored}} fxc-pass {{}} */
  [unknown(1)] while (g_bool) return g_bool;  /* expected-warning {{unknown attribute 'unknown' ignored}} fxc-pass {{}} */
}

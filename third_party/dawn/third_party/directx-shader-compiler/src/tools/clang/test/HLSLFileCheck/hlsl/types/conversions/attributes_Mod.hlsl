// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main

// To test with the classic compiler, run
// fxc.exe /T ps_5_1 attributes.hlsl

// The following is a directive to override default behavior for "VerifyHelper.py fxc RunAttributes".  When this is specified, main shader must be defined manually.
// :FXC_VERIFY_ARGUMENTS: /T ps_5_1 /E main

int loop_before_assignment() {
  // fxc warning X3554: unknown attribute loop, or attribute invalid for this statement
  [loop] // expected-warning {{attribute 'loop' can only be applied to 'for', 'while' and 'do' loop statements}} fxc-pass {{}}
  int val = 2;
   int left12; bool right12; left12 = right12;
  return val;
}

int loop_before_return() {
  // fxc warning X3554: unknown attribute loop, or attribute invalid for this statement
  [loop] // expected-warning {{attribute 'loop' can only be applied to 'for', 'while' and 'do' loop statements}} fxc-warning {{X3554: unknown attribute loop, or attribute invalid for this statement}}
  return 0;
}

int short_unroll() {
  int result = 2;
  
  [unroll(2)] for (int i = 0; i < 100; i++) result++;
  /*verify-ast
    AttributedStmt <col:3, col:51>
    |-HLSLUnrollAttr <col:4, col:12> 2
    `-ForStmt <col:15, col:51>
      |-DeclStmt <col:20, col:29>
      | `-VarDecl <col:20, col:28> i 'int'
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
  
  /*verify-ast
    AttributedStmt <col:3, line:85:39>
    |-HLSLUnrollAttr <col:4, col:13> -1
    `-ForStmt <line:85:3, col:39>
      |-DeclStmt <col:8, col:17>
      | `-VarDecl <col:8, col:16> i 'int'
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
    AttributedStmt <col:3, line:115:39>
    |-HLSLUnrollAttr <col:4, col:14> 0
    `-ForStmt <line:115:3, col:39>
      |-DeclStmt <col:8, col:17>
      | `-VarDecl <col:8, col:16> i 'int'
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
    AttributedStmt <col:3, line:156:3>
    |-HLSLAllowUAVConditionAttr <col:4>
    `-ForStmt <line:155:3, line:156:3>
      |-BinaryOperator <col:8, col:12> 'uint':'unsigned int' '='
      | |-DeclRefExpr <col:8> 'uint':'unsigned int' lvalue Var 'i' 'uint':'unsigned int'
      | `-ImplicitCastExpr <col:12> 'uint':'unsigned int' <LValueToRValue>
      |   `-DeclRefExpr <col:12> 'uint':'unsigned int' lvalue Var 'g_dealiasTableOffset' 'uint':'unsigned int'
      |-<<<NULL>>>
      |-BinaryOperator <col:34, col:38> 'const bool' '<'
      | |-ImplicitCastExpr <col:34> 'uint':'unsigned int' <LValueToRValue>
      | | `-DeclRefExpr <col:34> 'uint':'unsigned int' lvalue Var 'i' 'uint':'unsigned int'
      | `-ImplicitCastExpr <col:38> 'uint':'unsigned int' <LValueToRValue>
      |   `-DeclRefExpr <col:38> 'uint':'unsigned int' lvalue Var 'g_dealiasTableSize' 'uint':'unsigned int'
      |-UnaryOperator <col:58, col:60> 'uint':'unsigned int' lvalue prefix '++'
      | `-DeclRefExpr <col:60> 'uint':'unsigned int' lvalue Var 'i' 'uint':'unsigned int'
      `-CompoundStmt <col:63, line:156:3>
  */
  for (i = g_dealiasTableOffset; i < g_dealiasTableSize; ++i) {
  }
  return i;
}

[domain("quad")] int domain_fn() { return 1; }          /* fxc-warning {{X3554: unknown attribute domain, or attribute invalid for this statement}} */
[maxtessfactor(1)] int maxtessfactor_fn() { return 1; }
[numthreads(8,8,1)] int numthreads_fn() { return 1; } 
[instance(1)] int instance_fn() { return 1; }
[outputcontrolpoints(12)] int outputcontrolpoints_fn() { return 1; }
[outputtopology("point")] int outputtopology_fn() { return 1; } 
[partitioning("integer")] int partitioning_fn() { return 1; }
[patchconstantfunc("uav")] int patchconstantfunc_fn() { return 1; }

struct HSFoo
{
    float3 pos : POSITION;
};

Texture2D<float4> tex1[10] : register( t20, space10 );
/*verify-ast
  VarDecl <col:1, col:26> tex1 'Texture2D<float4> [10]'
  |-RegisterAssignment <col:30> register(t20, space10)
*/

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
  FunctionDecl <col:1, line:228:1> HSMain 'HSFoo (InputPatch<HSFoo, 16>, uint, uint)'
  |-ParmVarDecl <col:15, col:37> p 'InputPatch<HSFoo, 16>':'InputPatch<HSFoo, 16>'
  |-ParmVarDecl <line:219:15, col:20> i 'uint':'unsigned int'
  | |-SemanticDecl <col:24> "SV_OutputControlPointID"
  |-ParmVarDecl <line:220:15, col:20> PatchID 'uint':'unsigned int'
  | |-SemanticDecl <col:30> "SV_PrimitiveID"
  |-CompoundStmt <line:221:1, line:228:1>
  | |-DeclStmt <line:222:5, col:17>
  | | `-VarDecl <col:5, col:11> output 'HSFoo' nrvo
  | `-ReturnStmt <line:227:5, col:12>
  |   `-DeclRefExpr <col:12> 'HSFoo' lvalue Var 'output' 'HSFoo'
  |-HLSLPatchConstantFuncAttr <line:199:2, col:30> "PatchFoo"
  |-HLSLOutputControlPointsAttr <line:198:2, col:24> 16
  |-HLSLOutputTopologyAttr <line:197:2, col:30> "triangle_cw"
  |-HLSLPartitioningAttr <line:196:2, col:24> "integer"
  `-HLSLDomainAttr <line:192:2, col:15> "quad"
*/
              uint i : SV_OutputControlPointID,
              uint PatchID : SV_PrimitiveID )
{
    HSFoo output;
    // TODO: subscript missing from InputPatch object.
    //float4 r = float4(p[PatchID].pos, 1);
    //r += tex1[r.x].Load(r.xyz);
    //output.pos = p[i].pos + r.xyz;
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

void clipplanes_literals();

void clipplanes_const();

void clipplanes_const();  

void clipplanes_bad_ternary();

void clipplanes_bad_binop();

void clipplanes_bad_call();

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
void clipplanes_bad_swizzle();

// index into vector - allowed in fxc, disallowed in new compiler
void clipplanes_bad_vector_index();

// index into matrix - allowed in fxc, disallowed in new compiler
void clipplanes_bad_matrix_index();

// constructor - allowed in fxc, disallowed in new compiler
void clipplanes_bad_matrix_index();

// swizzle from scalar - allowed in fxc, disallowed in new compiler
void clipplanes_bad_scalar_swizzle();

[clipplanes(
/*verify-ast
  HLSLClipPlanesAttr <col:2, line:342:3>
  |-DeclRefExpr <line:339:3> 'float4':'vector<float, 4>' lvalue Var 'f4' 'float4':'vector<float, 4>'
  |-ArraySubscriptExpr <line:340:3, col:8> 'float4':'vector<float, 4>' lvalue
  | |-ImplicitCastExpr <col:3> 'float4 [2]' <LValueToRValue>
  | | `-DeclRefExpr <col:3> 'float4 [2]' lvalue Var 'cp4' 'float4 [2]'
  | `-IntegerLiteral <col:7> 'literal int' 0
  |-ArraySubscriptExpr <line:341:3, col:11> 'float4':'vector<float, 4>' lvalue
  | |-ImplicitCastExpr <col:3, col:6> 'float4 [5]' <LValueToRValue>
  | | `-MemberExpr <col:3, col:6> 'float4 [5]' lvalue .cp5
  | |   `-DeclRefExpr <col:3> 'global_struct' lvalue Var 'gs' 'global_struct'
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
  HLSLClipPlanesAttr <col:2, line:376:3>
  |-ParenExpr <line:372:3, col:6> 'float4':'vector<float, 4>' lvalue
  | `-DeclRefExpr <col:4> 'float4':'vector<float, 4>' lvalue Var 'f4' 'float4':'vector<float, 4>'
  |-ArraySubscriptExpr <line:373:3, col:10> 'float4':'vector<float, 4>' lvalue
  | |-ImplicitCastExpr <col:3> 'float4 [2]' <LValueToRValue>
  | | `-DeclRefExpr <col:3> 'float4 [2]' lvalue Var 'cp4' 'float4 [2]'
  | `-ParenExpr <col:7, col:9> 'literal int'
  |   `-IntegerLiteral <col:8> 'literal int' 0
  |-ArraySubscriptExpr <line:374:3, col:13> 'float4':'vector<float, 4>' lvalue
  | |-ImplicitCastExpr <col:3, col:8> 'float4 [5]' <LValueToRValue>
  | | `-MemberExpr <col:3, col:8> 'float4 [5]' lvalue .cp5
  | |   `-ParenExpr <col:3, col:6> 'global_struct' lvalue
  | |     `-DeclRefExpr <col:4> 'global_struct' lvalue Var 'gs' 'global_struct'
  | `-IntegerLiteral <col:12> 'literal int' 2
  |-ParenExpr <line:375:3, col:15> 'float4':'vector<float, 4>' lvalue
  | `-ArraySubscriptExpr <col:4, col:14> 'float4':'vector<float, 4>' lvalue
  |   |-ImplicitCastExpr <col:4, col:9> 'float4 [5]' <LValueToRValue>
  |   | `-MemberExpr <col:4, col:9> 'float4 [5]' lvalue .cp5
  |   |   `-ParenExpr <col:4, col:7> 'global_struct' lvalue
  |   |     `-DeclRefExpr <col:5> 'global_struct' lvalue Var 'gs' 'global_struct'
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


[earlydepthstencil]
/*verify-ast
  HLSLEarlyDepthStencilAttr <col:2>
*/
float4 main() : SV_Target0 {
    int val = 2;

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
    
    for (int k = 0; k < 4; k++) { val *= 2; } // expected-note {{previous definition is here}} fxc-pass {{}}
    
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
// RUN: %dxc -E cs_main -T cs_6_0 %s | FileCheck %s

// CHECK: @cs_main

float overload1(float f) { return 1; }                        /* expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} fxc-pass {{}} */
double overload1(double f) { return 2; }                       /* expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} fxc-pass {{}} */
int overload1(int i) { return 3; }                             /* expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} fxc-pass {{}} */
uint overload1(uint i) { return 4; }                           /* expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} fxc-pass {{}} */
min12int overload1(min12int i) { return 5; }                   /* expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} fxc-pass {{}} */


static const float2 g_f2_arr[8] =
{
  float2 (-0.1234,  0.4321) / 0.8750,
  float2 ( 0.0000, -0.0012) / 0.8750,
  float2 ( 0.5555,  0.5555) / 0.8750,
  float2 (-0.6666,  0.0000) / 0.8750,
  float2 ( 0.3333, -0.0000) / 0.8750,
  float2 ( 0.0000,  0.3213) / 0.8750,
  float2 (-0.1234, -0.4567) / 0.8750,
  float2 ( 0.1255,  0.0000) / 0.8750,
};

float2 g_f2;
float2 rotation;
float fn_f_f(float r)
{
  // 1-digit integers go through separate code path, so also check multi-digit:

  int tap = 0;
  float2x2 rotationMatrix = { rotation.x, rotation.y, -rotation.y, rotation.x };
  /*verify-ast
    DeclStmt <col:3, col:80>
    `-VarDecl <col:3, col:79> rotationMatrix 'float2x2':'matrix<float, 2, 2>'
      `-InitListExpr <col:29, col:79> 'float2x2':'matrix<float, 2, 2>'
        |-ImplicitCastExpr <col:31, col:40> 'float' <LValueToRValue>
        | `-HLSLVectorElementExpr <col:31, col:40> 'float' lvalue vectorcomponent x
        |   `-DeclRefExpr <col:31> 'float2':'vector<float, 2>' lvalue Var 'rotation' 'float2':'vector<float, 2>'
        |-ImplicitCastExpr <col:43, col:52> 'float' <LValueToRValue>
        | `-HLSLVectorElementExpr <col:43, col:52> 'float' lvalue vectorcomponent y
        |   `-DeclRefExpr <col:43> 'float2':'vector<float, 2>' lvalue Var 'rotation' 'float2':'vector<float, 2>'
        |-UnaryOperator <col:55, col:65> 'float' prefix '-'
        | `-ImplicitCastExpr <col:56, col:65> 'float' <LValueToRValue>
        |   `-HLSLVectorElementExpr <col:56, col:65> 'float' lvalue vectorcomponent y
        |     `-DeclRefExpr <col:56> 'float2':'vector<float, 2>' lvalue Var 'rotation' 'float2':'vector<float, 2>'
        `-ImplicitCastExpr <col:68, col:77> 'float' <LValueToRValue>
          `-HLSLVectorElementExpr <col:68, col:77> 'float' lvalue vectorcomponent x
            `-DeclRefExpr <col:68> 'float2':'vector<float, 2>' lvalue Var 'rotation' 'float2':'vector<float, 2>'
  */
  float2 offs = mul(g_f2_arr[tap], rotationMatrix) * r;
  /*verify-ast
    DeclStmt <col:3, col:55>
    `-VarDecl <col:3, col:54> offs 'float2':'vector<float, 2>'
      `-BinaryOperator <col:17, col:54> 'vector<float, 2>' '*'
        |-CallExpr <col:17, col:50> 'vector<float, 2>':'vector<float, 2>'
        | |-ImplicitCastExpr <col:17> 'vector<float, 2> (*)(const vector<float, 2> &, matrix<float, 2, 2>)' <FunctionToPointerDecay>
        | | `-DeclRefExpr <col:17> 'vector<float, 2> (const vector<float, 2> &, matrix<float, 2, 2>)' lvalue Function 'mul' 'vector<float, 2> (const vector<float, 2> &, matrix<float, 2, 2>)'
        | |-ArraySubscriptExpr <col:21, col:33> 'const float2':'const vector<float, 2>' lvalue
        | | |-ImplicitCastExpr <col:21> 'const float2 [8]' <LValueToRValue>
        | | | `-DeclRefExpr <col:21> 'const float2 [8]' lvalue Var 'g_f2_arr' 'const float2 [8]'
        | | `-ImplicitCastExpr <col:30> 'int' <LValueToRValue>
        | |   `-DeclRefExpr <col:30> 'int' lvalue Var 'tap' 'int'
        | `-DeclRefExpr <col:36> 'float2x2':'matrix<float, 2, 2>' lvalue Var 'rotationMatrix' 'float2x2':'matrix<float, 2, 2>'
        `-ImplicitCastExpr <col:54> 'vector<float, 2>':'vector<float, 2>' <HLSLVectorSplat>
          `-ImplicitCastExpr <col:54> 'float' <LValueToRValue>
            `-DeclRefExpr <col:54> 'float' lvalue ParmVar 'r' 'float'
  */
  return offs.x;
}

uint fn_f3_f3io_u(float3 wn, inout float3 tsn)
{
  uint  e3 = 0;
  float d1 = (wn.x + wn.y + wn.z) * 0.5;
  /*verify-ast
    DeclStmt <col:3, col:40>
    `-VarDecl <col:3, col:37> d1 'float'
      `-BinaryOperator <col:14, col:37> 'float' '*'
        |-ParenExpr <col:14, col:33> 'float'
        | `-BinaryOperator <col:15, col:32> 'float' '+'
        |   |-BinaryOperator <col:15, col:25> 'float' '+'
        |   | |-ImplicitCastExpr <col:15, col:18> 'float' <LValueToRValue>
        |   | | `-HLSLVectorElementExpr <col:15, col:18> 'float' lvalue vectorcomponent x
        |   | |   `-DeclRefExpr <col:15> 'float3':'vector<float, 3>' lvalue ParmVar 'wn' 'float3':'vector<float, 3>'
        |   | `-ImplicitCastExpr <col:22, col:25> 'float' <LValueToRValue>
        |   |   `-HLSLVectorElementExpr <col:22, col:25> 'float' lvalue vectorcomponent y
        |   |     `-DeclRefExpr <col:22> 'float3':'vector<float, 3>' lvalue ParmVar 'wn' 'float3':'vector<float, 3>'
        |   `-ImplicitCastExpr <col:29, col:32> 'float' <LValueToRValue>
        |     `-HLSLVectorElementExpr <col:29, col:32> 'float' lvalue vectorcomponent z
        |       `-DeclRefExpr <col:29> 'float3':'vector<float, 3>' lvalue ParmVar 'wn' 'float3':'vector<float, 3>'
        `-ImplicitCastExpr <col:37> 'float' <FloatingCast>
          `-FloatingLiteral <col:37> 'literal float' 5.000000e-01
  */
  float d2 = wn.x - d1;
  float d3 = wn.y - d1;
  float d4 = wn.z - d1;
  float dm = max(max(d1, d2), max(d3, d4));

  float3 nn = tsn;
  if (d2 == dm) { e3 = 1; nn *= float3 (1, -1, -1); dm += 2; }
  if (d3 == dm) { e3 = 2; nn *= float3 (-1, 1, -1); dm += 2; }
  if (d4 == dm) { e3 = 3; nn *= float3 (-1, -1, 1); }

  tsn.z = nn.x + nn.y + nn.z;
  tsn.y = nn.z - nn.x;
  tsn.x = tsn.z - 3 * nn.y;

  const float sqrt_2 = 1.414213562373f;
  const float sqrt_3 = 1.732050807569f;
  const float sqrt_6 = 2.449489742783f;

  tsn *= float3 (1.0 / sqrt_6, 1.0 / sqrt_2, 1.0 / sqrt_3);
  /*verify-ast
    CompoundAssignOperator <col:3, col:58> 'float3':'vector<float, 3>' lvalue '*=' ComputeLHSTy='vector<float, 3>' ComputeResultTy='vector<float, 3>'
    |-DeclRefExpr <col:3> 'float3':'vector<float, 3>' lvalue ParmVar 'tsn' 'float3 &'
    `-CXXFunctionalCastExpr <col:10, col:58> 'float3':'vector<float, 3>' functional cast to float3 <NoOp>
      `-InitListExpr <col:18, col:52> 'float3':'vector<float, 3>'
        |-BinaryOperator <col:18, col:24> 'float' '/'
        | |-ImplicitCastExpr <col:18> 'float' <FloatingCast>
        | | `-FloatingLiteral <col:18> 'literal float' 1.000000e+00
        | `-ImplicitCastExpr <col:24> 'float' <LValueToRValue>
        |   `-DeclRefExpr <col:24> 'const float' lvalue Var 'sqrt_6' 'const float'
        |-BinaryOperator <col:32, col:38> 'float' '/'
        | |-ImplicitCastExpr <col:32> 'float' <FloatingCast>
        | | `-FloatingLiteral <col:32> 'literal float' 1.000000e+00
        | `-ImplicitCastExpr <col:38> 'float' <LValueToRValue>
        |   `-DeclRefExpr <col:38> 'const float' lvalue Var 'sqrt_2' 'const float'
        `-BinaryOperator <col:46, col:52> 'float' '/'
          |-ImplicitCastExpr <col:46> 'float' <FloatingCast>
          | `-FloatingLiteral <col:46> 'literal float' 1.000000e+00
          `-ImplicitCastExpr <col:52> 'float' <LValueToRValue>
            `-DeclRefExpr <col:52> 'const float' lvalue Var 'sqrt_3' 'const float'
  */

  return e3;
}


[numthreads(8,8,1)]
void cs_main() {
}
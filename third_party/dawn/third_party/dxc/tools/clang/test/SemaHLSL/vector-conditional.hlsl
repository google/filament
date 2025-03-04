// RUN: %dxc -Tlib_6_3 -verify -HV 2018 %s
// RUN: %dxc -Tps_6_0 -verify -HV 2018 %s

// Disable validation on fxc (/Vd) because
// non-literal ternary with objects results in bad code.
// :FXC_VERIFY_ARGUMENTS: /E main /T ps_5_1 /Vd

/****************************************************************************
Notes:
- try various object types:
  Basic Numerics: scalar, vector, matrix
  Arrays
  User Defined Types
  Objects: Textures/Samplers/Streams/Patches?
  Inner Objects: intermediate mips object?
- try various dimensions, testing truncation/replication, etc...
- try various numeric component types, testing promotion, etc...
- try l-value vs. r-value cases: like out params, etc...
****************************************************************************/

bool4 b4;
bool4x4 b4x4;
bool3x3 b3x3;
Texture2D T1;
Texture2D T2;
SamplerState S;

float4x4 m1, m2;

// UDT1, UDT2, and UDT3 have the same number of components:
struct UDT1 {
  float4 a;
  float4 b;
};
struct UDT2 {
  row_major float4x2 m;
};
struct UDT3 {
  float a;
  float b;
  float3 c;
  float3 d;
};
UDT1 u1, u1b;
UDT2 u2;
UDT3 u3;

float4 a1[2];
float4 a2[2];
float4 a3[8];
float a4[8];

void Select(out Texture2D TOut) { /* expected-note {{candidate function}} fxc-pass {{}} */
  TOut = b4.x ? T1 : T2;
}
[shader("pixel")]
float4 main(float4 v0 : TEXCOORD) : SV_Target
{
  float4 acc = 0.0F;

  acc += b4.x ?: v0;                                        /* expected-error {{use of GNU ?: conditional expression extension, omitting middle operand is unsupported in HLSL}} fxc-error {{X3000: syntax error: unexpected token ':'}} */

  acc += b4.x ? v0 : (v0 + 1.0F);
  /*verify-ast
    CompoundAssignOperator <col:3, col:32> 'float4':'vector<float, 4>' lvalue '+=' ComputeLHSTy='float4':'vector<float, 4>' ComputeResultTy='float4':'vector<float, 4>'
    |-DeclRefExpr <col:3> 'float4':'vector<float, 4>' lvalue Var 'acc' 'float4':'vector<float, 4>'
    `-ConditionalOperator <col:10, col:32> 'vector<float, 4>'
      |-ImplicitCastExpr <col:10, col:13> 'vector<bool, 4>':'vector<bool, 4>' <HLSLVectorSplat>
      | `-ImplicitCastExpr <col:10, col:13> 'const bool' <LValueToRValue>
      |   `-HLSLVectorElementExpr <col:10, col:13> 'const bool' lvalue vectorcomponent x
      |     `-DeclRefExpr <col:10> 'const bool4':'const vector<bool, 4>' lvalue Var 'b4' 'const bool4':'const vector<bool, 4>'
      |-ImplicitCastExpr <col:17> 'float4':'vector<float, 4>' <LValueToRValue>
      | `-DeclRefExpr <col:17> 'float4':'vector<float, 4>' lvalue ParmVar 'v0' 'float4':'vector<float, 4>'
      `-ParenExpr <col:22, col:32> 'float4':'vector<float, 4>'
        `-BinaryOperator <col:23, col:28> 'float4':'vector<float, 4>' '+'
          |-ImplicitCastExpr <col:23> 'float4':'vector<float, 4>' <LValueToRValue>
          | `-DeclRefExpr <col:23> 'float4':'vector<float, 4>' lvalue ParmVar 'v0' 'float4':'vector<float, 4>'
          `-ImplicitCastExpr <col:28> 'vector<float, 4>':'vector<float, 4>' <HLSLVectorSplat>
            `-FloatingLiteral <col:28> 'float' 1.000000e+00
  */

  acc += b4.x ? v0 : 1.0F;
  /*verify-ast
    CompoundAssignOperator <col:3, col:22> 'float4':'vector<float, 4>' lvalue '+=' ComputeLHSTy='float4':'vector<float, 4>' ComputeResultTy='float4':'vector<float, 4>'
    |-DeclRefExpr <col:3> 'float4':'vector<float, 4>' lvalue Var 'acc' 'float4':'vector<float, 4>'
    `-ConditionalOperator <col:10, col:22> 'vector<float, 4>'
      |-ImplicitCastExpr <col:10, col:13> 'vector<bool, 4>':'vector<bool, 4>' <HLSLVectorSplat>
      | `-ImplicitCastExpr <col:10, col:13> 'const bool' <LValueToRValue>
      |   `-HLSLVectorElementExpr <col:10, col:13> 'const bool' lvalue vectorcomponent x
      |     `-DeclRefExpr <col:10> 'const bool4':'const vector<bool, 4>' lvalue Var 'b4' 'const bool4':'const vector<bool, 4>'
      |-ImplicitCastExpr <col:17> 'float4':'vector<float, 4>' <LValueToRValue>
      | `-DeclRefExpr <col:17> 'float4':'vector<float, 4>' lvalue ParmVar 'v0' 'float4':'vector<float, 4>'
      `-ImplicitCastExpr <col:22> 'vector<float, 4>':'vector<float, 4>' <HLSLVectorSplat>
        `-FloatingLiteral <col:22> 'float' 1.000000e+00
  */

  acc += b4 ? v0 : (v0 + 1.0F);
  /*verify-ast
    CompoundAssignOperator <col:3, col:30> 'float4':'vector<float, 4>' lvalue '+=' ComputeLHSTy='float4':'vector<float, 4>' ComputeResultTy='float4':'vector<float, 4>'
    |-DeclRefExpr <col:3> 'float4':'vector<float, 4>' lvalue Var 'acc' 'float4':'vector<float, 4>'
    `-ConditionalOperator <col:10, col:30> 'vector<float, 4>'
      |-ImplicitCastExpr <col:10> 'const bool4':'const vector<bool, 4>' <LValueToRValue>
      | `-DeclRefExpr <col:10> 'const bool4':'const vector<bool, 4>' lvalue Var 'b4' 'const bool4':'const vector<bool, 4>'
      |-ImplicitCastExpr <col:15> 'float4':'vector<float, 4>' <LValueToRValue>
      | `-DeclRefExpr <col:15> 'float4':'vector<float, 4>' lvalue ParmVar 'v0' 'float4':'vector<float, 4>'
      `-ParenExpr <col:20, col:30> 'float4':'vector<float, 4>'
        `-BinaryOperator <col:21, col:26> 'float4':'vector<float, 4>' '+'
          |-ImplicitCastExpr <col:21> 'float4':'vector<float, 4>' <LValueToRValue>
          | `-DeclRefExpr <col:21> 'float4':'vector<float, 4>' lvalue ParmVar 'v0' 'float4':'vector<float, 4>'
          `-ImplicitCastExpr <col:26> 'vector<float, 4>':'vector<float, 4>' <HLSLVectorSplat>
            `-FloatingLiteral <col:26> 'float' 1.000000e+00
  */

  acc += b4 ? v0 : 1.0F;
  /*verify-ast
    CompoundAssignOperator <col:3, col:20> 'float4':'vector<float, 4>' lvalue '+=' ComputeLHSTy='float4':'vector<float, 4>' ComputeResultTy='float4':'vector<float, 4>'
    |-DeclRefExpr <col:3> 'float4':'vector<float, 4>' lvalue Var 'acc' 'float4':'vector<float, 4>'
    `-ConditionalOperator <col:10, col:20> 'vector<float, 4>'
      |-ImplicitCastExpr <col:10> 'const bool4':'const vector<bool, 4>' <LValueToRValue>
      | `-DeclRefExpr <col:10> 'const bool4':'const vector<bool, 4>' lvalue Var 'b4' 'const bool4':'const vector<bool, 4>'
      |-ImplicitCastExpr <col:15> 'float4':'vector<float, 4>' <LValueToRValue>
      | `-DeclRefExpr <col:15> 'float4':'vector<float, 4>' lvalue ParmVar 'v0' 'float4':'vector<float, 4>'
      `-ImplicitCastExpr <col:20> 'vector<float, 4>':'vector<float, 4>' <HLSLVectorSplat>
        `-FloatingLiteral <col:20> 'float' 1.000000e+00
  */

  acc.xy += b4.xy ? v0.xy : (v0 + 1.0F);                    /* expected-warning {{implicit truncation of vector type}} fxc-warning {{X3206: implicit truncation of vector type}} */
  /*verify-ast
    CompoundAssignOperator <col:3, col:39> 'vector<float, 2>':'vector<float, 2>' lvalue vectorcomponent '+=' ComputeLHSTy='vector<float, 2>':'vector<float, 2>' ComputeResultTy='vector<float, 2>':'vector<float, 2>'
    |-HLSLVectorElementExpr <col:3, col:7> 'vector<float, 2>':'vector<float, 2>' lvalue vectorcomponent xy
    | `-DeclRefExpr <col:3> 'float4':'vector<float, 4>' lvalue Var 'acc' 'float4':'vector<float, 4>'
    `-ConditionalOperator <col:13, col:39> 'vector<float, 2>'
      |-ImplicitCastExpr <col:13, col:16> 'const vector<bool, 2>':'const vector<bool, 2>' <LValueToRValue>
      | `-HLSLVectorElementExpr <col:13, col:16> 'const vector<bool, 2>':'const vector<bool, 2>' lvalue vectorcomponent xy
      |   `-DeclRefExpr <col:13> 'const bool4':'const vector<bool, 4>' lvalue Var 'b4' 'const bool4':'const vector<bool, 4>'
      |-ImplicitCastExpr <col:21, col:24> 'vector<float, 2>':'vector<float, 2>' <LValueToRValue>
      | `-HLSLVectorElementExpr <col:21, col:24> 'vector<float, 2>':'vector<float, 2>' lvalue vectorcomponent xy
      |   `-DeclRefExpr <col:21> 'float4':'vector<float, 4>' lvalue ParmVar 'v0' 'float4':'vector<float, 4>'
      `-ImplicitCastExpr <col:29, col:39> 'vector<float, 2>':'vector<float, 2>' <HLSLVectorTruncationCast>
        `-ParenExpr <col:29, col:39> 'float4':'vector<float, 4>'
          `-BinaryOperator <col:30, col:35> 'float4':'vector<float, 4>' '+'
            |-ImplicitCastExpr <col:30> 'float4':'vector<float, 4>' <LValueToRValue>
            | `-DeclRefExpr <col:30> 'float4':'vector<float, 4>' lvalue ParmVar 'v0' 'float4':'vector<float, 4>'
            `-ImplicitCastExpr <col:35> 'vector<float, 4>':'vector<float, 4>' <HLSLVectorSplat>
              `-FloatingLiteral <col:35> 'float' 1.000000e+00
  */

  acc += b4 ? v0.xy : 1.0F;                                 /* expected-error {{conditional operator condition and result dimensions mismatch.}} fxc-error {{X3017: cannot implicitly convert from 'float2' to 'float4'}} */
  acc += b4 ? v0.xy : (v0 + 1.0F);                          /* expected-error {{conditional operator condition and result dimensions mismatch.}} fxc-error {{X3017: cannot implicitly convert from 'float2' to 'const float4'}} */
  acc += b4.xy ? v0 : (v0 + 1.0F);                          /* expected-error {{conditional operator condition and result dimensions mismatch.}} fxc-error {{X3020: dimension of conditional does not match value}} */
  acc += b4.xy ? v0 : (v0.xy + 1.0F);                       /* expected-error {{cannot convert from 'vector<float, 2>' to 'float4'}} expected-warning {{implicit truncation of vector type}} fxc-error {{X3017: cannot implicitly convert from 'const float2' to 'float4'}} fxc-warning {{X3206: implicit truncation of vector type}} */

  // lit float/int
  acc += b4 ? v0 : 1.1;
  /*verify-ast
    CompoundAssignOperator <col:3, col:20> 'float4':'vector<float, 4>' lvalue '+=' ComputeLHSTy='float4':'vector<float, 4>' ComputeResultTy='float4':'vector<float, 4>'
    |-DeclRefExpr <col:3> 'float4':'vector<float, 4>' lvalue Var 'acc' 'float4':'vector<float, 4>'
    `-ConditionalOperator <col:10, col:20> 'vector<float, 4>'
      |-ImplicitCastExpr <col:10> 'const bool4':'const vector<bool, 4>' <LValueToRValue>
      | `-DeclRefExpr <col:10> 'const bool4':'const vector<bool, 4>' lvalue Var 'b4' 'const bool4':'const vector<bool, 4>'
      |-ImplicitCastExpr <col:15> 'float4':'vector<float, 4>' <LValueToRValue>
      | `-DeclRefExpr <col:15> 'float4':'vector<float, 4>' lvalue ParmVar 'v0' 'float4':'vector<float, 4>'
      `-ImplicitCastExpr <col:20> 'vector<float, 4>':'vector<float, 4>' <HLSLVectorSplat>
        `-ImplicitCastExpr <col:20> 'float' <FloatingCast>
          `-FloatingLiteral <col:20> 'literal float' 1.100000e+00
  */
  acc += b4 ? v0 : -2;
  /*verify-ast
    CompoundAssignOperator <col:3, col:21> 'float4':'vector<float, 4>' lvalue '+=' ComputeLHSTy='float4':'vector<float, 4>' ComputeResultTy='float4':'vector<float, 4>'
    |-DeclRefExpr <col:3> 'float4':'vector<float, 4>' lvalue Var 'acc' 'float4':'vector<float, 4>'
    `-ConditionalOperator <col:10, col:21> 'vector<float, 4>'
      |-ImplicitCastExpr <col:10> 'const bool4':'const vector<bool, 4>' <LValueToRValue>
      | `-DeclRefExpr <col:10> 'const bool4':'const vector<bool, 4>' lvalue Var 'b4' 'const bool4':'const vector<bool, 4>'
      |-ImplicitCastExpr <col:15> 'float4':'vector<float, 4>' <LValueToRValue>
      | `-DeclRefExpr <col:15> 'float4':'vector<float, 4>' lvalue ParmVar 'v0' 'float4':'vector<float, 4>'
      `-ImplicitCastExpr <col:20, col:21> 'vector<float, 4>':'vector<float, 4>' <HLSLVectorSplat>
        `-ImplicitCastExpr <col:20, col:21> 'float' <IntegralToFloating>
          `-UnaryOperator <col:20, col:21> 'literal int' prefix '-'
            `-IntegerLiteral <col:21> 'literal int' 2
  */

  min16float4 mf4 = (min16float4)acc * (min16float4)v0;
  mf4 += b4 ? v0 : mf4;                                     /* expected-warning {{conversion from larger type 'vector<float, 4>' to smaller type 'min16float4', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
  /*verify-ast
    CompoundAssignOperator <col:3, col:20> 'min16float4':'vector<min16float, 4>' lvalue '+=' ComputeLHSTy='vector<float, 4>':'vector<float, 4>' ComputeResultTy='vector<float, 4>':'vector<float, 4>'
    |-DeclRefExpr <col:3> 'min16float4':'vector<min16float, 4>' lvalue Var 'mf4' 'min16float4':'vector<min16float, 4>'
    `-ConditionalOperator <col:10, col:20> 'vector<float, 4>'
      |-ImplicitCastExpr <col:10> 'const bool4':'const vector<bool, 4>' <LValueToRValue>
      | `-DeclRefExpr <col:10> 'const bool4':'const vector<bool, 4>' lvalue Var 'b4' 'const bool4':'const vector<bool, 4>'
      |-ImplicitCastExpr <col:15> 'float4':'vector<float, 4>' <LValueToRValue>
      | `-DeclRefExpr <col:15> 'float4':'vector<float, 4>' lvalue ParmVar 'v0' 'float4':'vector<float, 4>'
      `-ImplicitCastExpr <col:20> 'vector<float, 4>' <HLSLCC_FloatingCast>
        `-ImplicitCastExpr <col:20> 'min16float4':'vector<min16float, 4>' <LValueToRValue>
          `-DeclRefExpr <col:20> 'min16float4':'vector<min16float, 4>' lvalue Var 'mf4' 'min16float4':'vector<min16float, 4>'
  */
  acc += b4 ? v0 : mf4;
  /*verify-ast
    CompoundAssignOperator <col:3, col:20> 'float4':'vector<float, 4>' lvalue '+=' ComputeLHSTy='float4':'vector<float, 4>' ComputeResultTy='float4':'vector<float, 4>'
    |-DeclRefExpr <col:3> 'float4':'vector<float, 4>' lvalue Var 'acc' 'float4':'vector<float, 4>'
    `-ConditionalOperator <col:10, col:20> 'vector<float, 4>'
      |-ImplicitCastExpr <col:10> 'const bool4':'const vector<bool, 4>' <LValueToRValue>
      | `-DeclRefExpr <col:10> 'const bool4':'const vector<bool, 4>' lvalue Var 'b4' 'const bool4':'const vector<bool, 4>'
      |-ImplicitCastExpr <col:15> 'float4':'vector<float, 4>' <LValueToRValue>
      | `-DeclRefExpr <col:15> 'float4':'vector<float, 4>' lvalue ParmVar 'v0' 'float4':'vector<float, 4>'
      `-ImplicitCastExpr <col:20> 'vector<float, 4>' <HLSLCC_FloatingCast>
        `-ImplicitCastExpr <col:20> 'min16float4':'vector<min16float, 4>' <LValueToRValue>
          `-DeclRefExpr <col:20> 'min16float4':'vector<min16float, 4>' lvalue Var 'mf4' 'min16float4':'vector<min16float, 4>'
  */

  Texture2D T;
  Select(T);
  acc += T.Sample(S, v0.xy);

  // Texture object as condition
  acc += T ? v0 : (v0 + 1.0F);                              /* expected-error {{conditional operator only supports condition with scalar, vector, or matrix types convertable to bool.}} fxc-error {{X3020: conditional must be numeric}} */

  Texture2D TOut1 = T1;
  Texture2D TOut2 = T2;
  // fxc doesn't like this because ?: results in RValue:
  Select(b4.y ? TOut1 : TOut2);                             /* expected-error {{no matching function for call to 'Select'}} fxc-error {{X3025: out parameters require l-value arguments}} */
  acc += TOut1.Sample(S, v0.xy);
  acc += TOut2.Sample(S, v0.xy);

  // Matrix ternary support:
  acc += mul(acc, b4x4 ? m1 : m2);
  /*verify-ast
    CompoundAssignOperator <col:3, col:33> 'float4':'vector<float, 4>' lvalue '+=' ComputeLHSTy='float4':'vector<float, 4>' ComputeResultTy='float4':'vector<float, 4>'
    |-DeclRefExpr <col:3> 'float4':'vector<float, 4>' lvalue Var 'acc' 'float4':'vector<float, 4>'
    `-CallExpr <col:10, col:33> 'vector<float, 4>':'vector<float, 4>'
      |-ImplicitCastExpr <col:10> 'vector<float, 4> (*)(vector<float, 4>, matrix<float, 4, 4>)' <FunctionToPointerDecay>
      | `-DeclRefExpr <col:10> 'vector<float, 4> (vector<float, 4>, matrix<float, 4, 4>)' lvalue Function 'mul' 'vector<float, 4> (vector<float, 4>, matrix<float, 4, 4>)'
      |-ImplicitCastExpr <col:14> 'float4':'vector<float, 4>' <LValueToRValue>
      | `-DeclRefExpr <col:14> 'float4':'vector<float, 4>' lvalue Var 'acc' 'float4':'vector<float, 4>'
      `-ConditionalOperator <col:19, col:31> 'matrix<float, 4, 4>'
        |-ImplicitCastExpr <col:19> 'const bool4x4':'const matrix<bool, 4, 4>' <LValueToRValue>
        | `-DeclRefExpr <col:19> 'const bool4x4':'const matrix<bool, 4, 4>' lvalue Var 'b4x4' 'const bool4x4':'const matrix<bool, 4, 4>'
        |-ImplicitCastExpr <col:26> 'const float4x4':'const matrix<float, 4, 4>' <LValueToRValue>
        | `-DeclRefExpr <col:26> 'const float4x4':'const matrix<float, 4, 4>' lvalue Var 'm1' 'const float4x4':'const matrix<float, 4, 4>'
        `-ImplicitCastExpr <col:31> 'const float4x4':'const matrix<float, 4, 4>' <LValueToRValue>
          `-DeclRefExpr <col:31> 'const float4x4':'const matrix<float, 4, 4>' lvalue Var 'm2' 'const float4x4':'const matrix<float, 4, 4>'
  */
  // float : double and matrix : scalar
  acc += mul(acc, b4x4 ? m1 : (double)1.0);                 /*  expected-warning {{conversion from larger type 'vector<double, 4>' to smaller type 'float4', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
  /*verify-ast
    CompoundAssignOperator <col:3, col:42> 'float4':'vector<float, 4>' lvalue '+=' ComputeLHSTy='vector<double, 4>':'vector<double, 4>' ComputeResultTy='vector<double, 4>':'vector<double, 4>'
    |-DeclRefExpr <col:3> 'float4':'vector<float, 4>' lvalue Var 'acc' 'float4':'vector<float, 4>'
    `-CallExpr <col:10, col:42> 'vector<double, 4>':'vector<double, 4>'
      |-ImplicitCastExpr <col:10> 'vector<double, 4> (*)(vector<double, 4>, matrix<double, 4, 4>)' <FunctionToPointerDecay>
      | `-DeclRefExpr <col:10> 'vector<double, 4> (vector<double, 4>, matrix<double, 4, 4>)' lvalue Function 'mul' 'vector<double, 4> (vector<double, 4>, matrix<double, 4, 4>)'
      |-ImplicitCastExpr <col:14> 'vector<double, 4>' <HLSLCC_FloatingCast>
      | `-ImplicitCastExpr <col:14> 'float4':'vector<float, 4>' <LValueToRValue>
      |   `-DeclRefExpr <col:14> 'float4':'vector<float, 4>' lvalue Var 'acc' 'float4':'vector<float, 4>'
      `-ConditionalOperator <col:19, col:39> 'matrix<double, 4, 4>'
        |-ImplicitCastExpr <col:19> 'const bool4x4':'const matrix<bool, 4, 4>' <LValueToRValue>
        | `-DeclRefExpr <col:19> 'const bool4x4':'const matrix<bool, 4, 4>' lvalue Var 'b4x4' 'const bool4x4':'const matrix<bool, 4, 4>'
        |-ImplicitCastExpr <col:26> 'matrix<double, 4, 4>' <HLSLCC_FloatingCast>
        | `-ImplicitCastExpr <col:26> 'const float4x4':'const matrix<float, 4, 4>' <LValueToRValue>
        |   `-DeclRefExpr <col:26> 'const float4x4':'const matrix<float, 4, 4>' lvalue Var 'm1' 'const float4x4':'const matrix<float, 4, 4>'
        `-ImplicitCastExpr <col:31, col:39> 'matrix<double, 4, 4>':'matrix<double, 4, 4>' <HLSLMatrixSplat>
          `-CStyleCastExpr <col:31, col:39> 'double' <NoOp>
            `-ImplicitCastExpr <col:39> 'double' <FloatingCast>
              `-FloatingLiteral <col:39> 'literal float' 1.000000e+00
  */

  // Truncation to condition size not supported:
  acc.xyz += (b3x3 ? m1 : m2)[1];                           /* expected-error {{conditional operator condition and result dimensions mismatch.}} fxc-error {{X3020: dimension of conditional does not match value}} */

  // fxc doesn't like UDT's in a ternary operator.  It results in:
  // internal error: assertion failed! onecoreuap\windows\directx\dxg\d3d12\compiler\tok\types.cpp(1460): pLeft->CollectInfo(AR_TINFO_SIMPLE_OBJECTS, &LeftInfo) && pRight->CollectInfo(AR_TINFO_SIMPLE_OBJECTS, &RightInfo)
  // error X3020: type mismatch between conditional values
  UDT3 lu3 = (UDT3)0;
  lu3 = b4.x ? u3 : u3;                                     /* expected-error {{conditional operator only supports results with numeric scalar, vector, or matrix types.}} fxc-error {{X3020: type mismatch between conditional values}} */
  lu3 += b4.x ? u1 : u1b;                                   /* expected-error {{conditional operator only supports results with numeric scalar, vector, or matrix types.}} fxc-error {{X3020: type mismatch between conditional values}} */
  lu3 += b4.x ? u1 : u2;                                    /* expected-error {{conditional operator only supports results with numeric scalar, vector, or matrix types.}} fxc-error {{X3020: type mismatch between conditional values}} */
  lu3 += ((bool4x2)b4x4) ? u1 : u2;                         /* expected-error {{conditional operator only supports results with numeric scalar, vector, or matrix types.}} fxc-error {{X3020: type mismatch between conditional values}} */
  acc += float4(lu3.a, lu3.b, lu3.c.xy);
  acc += float4(lu3.c.z, lu3.d.xyz);

  // Same problem with arrays:
  acc += (b4.x ? a1 : a2)[0];                               /* expected-error {{conditional operator only supports results with numeric scalar, vector, or matrix types.}} fxc-error {{X3020: type mismatch between conditional values}} */
  acc += (b4.x ? a1 : a3)[0];                               /* expected-error {{conditional operator only supports results with numeric scalar, vector, or matrix types.}} fxc-error {{X3020: type mismatch between conditional values}} */
  acc += (b4.x ? a1 : a4)[0];                               /* expected-error {{conditional operator only supports results with numeric scalar, vector, or matrix types.}} fxc-error {{X3020: type mismatch between conditional values}} */

  return acc;
}

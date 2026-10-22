// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

StructuredBuffer<float3> buffer_vec;
StructuredBuffer<float3x3> buffer_mat;

/*
According to HLSL reference, mul() has the following versions:

|Name|Purpose|Template|Component Type  |size|
|====|=======|========|================|==========================================================================|
|x   |in     |scalar  |float, int      |1                                                                         |
|y   |in     |scalar  |same as input x |1                                                                         |
|ret |out    |scalar  |same as input x |1                                                                         |
|====|=======|========|================|==========================================================================|
|x   |in     |scalar  |float, int      | 1                                                                        |
|y   |in     |vector  |float, int      |any                                                                       |
|ret |out    |vector  |float, int      |same dimension(s) as input y                                              |
|====|=======|========|================|==========================================================================|
|x   |in     |scalar  |float, int      |1                                                                         |
|y   |in     |matrix  |float, int      |any                                                                       |
|ret |out    |matrix  |same as intput y|same dimension(s) as input y                                              |
|====|=======|========|================|==========================================================================|
|x   |in     |vector  |float, int      |any                                                                       |
|y   |in     |scalar  |float, int      |1                                                                         |
|ret |out    |vector  |float, int      |same dimension(s) as input x                                              |
|====|=======|========|================|==========================================================================|
|x   |in     |vector  |float, int      |any                                                                       |
|y   |in     |vector  |float, int      |same dimension(s) as input x                                              |
|ret |out    |scalar  |float, int      |1                                                                         |
|====|=======|========|================|==========================================================================|
|x   |in     |vector  |float, int      |any                                                                       |
|y   |in     |matrix  |float, int      |rows = same dimension(s) as input x, columns = any                        |
|ret |out    |vector  |float, int      |same dimension(s) as input y columns                                      |
|====|=======|========|================|==========================================================================|
|x   |in     |matrix  |float, int      |any                                                                       |
|y   |in     |scalar  |float, int      |1                                                                         |
|ret |out    |matrix  |float, int      |same dimension(s) as input x                                              |
|====|=======|========|================|==========================================================================|
|x   |in     |matrix  |float, int      |any                                                                       |
|y   |in     |vector  |float, int      |number of columns in input x                                              |
|ret |out    |vector  |float, int      |number of rows in input x                                                 |
|====|=======|========|================|==========================================================================|
|x   |in     |matrix  |float, int      |any                                                                       |
|y   |in     |matrix  |float, int      |rows = number of columns in input x                                       |
|ret |out    |matrix  |float, int      |rows = number of rows in input x, columns = number of columns in input y  |
|====|=======|========|================|==========================================================================|
*/

void main() {

  float a, b;
// CHECK: {{%[0-9]+}} = OpFMul %float {{%[0-9]+}} {{%[0-9]+}}
  float scalarMulscalar = mul(a,b);

  float float_c;
  float4 float4_d;

// CHECK:      [[float4_d:%[0-9]+]] = OpLoad %v4float %float4_d
// CHECK-NEXT: [[float_c:%[0-9]+]] = OpLoad %float %float_c
// CHECK-NEXT: {{%[0-9]+}} = OpVectorTimesScalar %v4float [[float4_d]] [[float_c]]
  float4 float_scalarMulVector = mul(float_c,float4_d);

// CHECK:      [[float4_d1:%[0-9]+]] = OpLoad %v4float %float4_d
// CHECK-NEXT: [[float_c1:%[0-9]+]] = OpLoad %float %float_c
// CHECK-NEXT: {{%[0-9]+}} = OpVectorTimesScalar %v4float [[float4_d1]] [[float_c1]]
  float4 float_vectorMulScalar = mul(float4_d,float_c);

  int int_c;
  int4 int4_d;

// CHECK:      [[int4_d:%[0-9]+]] = OpLoad %v4int %int4_d
// CHECK-NEXT: [[int_c:%[0-9]+]] = OpLoad %int %int_c
// CHECK-NEXT: [[c_splat:%[0-9]+]] = OpCompositeConstruct %v4int [[int_c]] [[int_c]] [[int_c]] [[int_c]]
// CHECK-NEXT: {{%[0-9]+}} = OpIMul %v4int [[c_splat]] [[int4_d]]
  int4 int_scalarMulVector = mul(int_c,int4_d);

// CHECK:      [[int4_d1:%[0-9]+]] = OpLoad %v4int %int4_d
// CHECK-NEXT: [[int_c1:%[0-9]+]] = OpLoad %int %int_c
// CHECK-NEXT: [[c_splat1:%[0-9]+]] = OpCompositeConstruct %v4int [[int_c1]] [[int_c1]] [[int_c1]] [[int_c1]]
// CHECK-NEXT: {{%[0-9]+}} = OpIMul %v4int [[int4_d1]] [[c_splat1]]
  int4 int_vectorMulScalar = mul(int4_d,int_c);

  float e;
  float3x4 f;

// CHECK:      [[e:%[0-9]+]] = OpLoad %float %e
// CHECK-NEXT: [[f:%[0-9]+]] = OpLoad %mat3v4float %f
// CHECK-NEXT: {{%[0-9]+}} = OpMatrixTimesScalar %mat3v4float [[f]] [[e]]
  float3x4 scalarMulMatrix = mul(e,f);

// CHECK:      [[f1:%[0-9]+]] = OpLoad %mat3v4float %f
// CHECK-NEXT: [[e1:%[0-9]+]] = OpLoad %float %e
// CHECK-NEXT: {{%[0-9]+}} = OpMatrixTimesScalar %mat3v4float [[f1]] [[e1]]
  float3x4 matrixMulScalar = mul(f,e);


  int4 g,h;
// CHECK:      [[g:%[0-9]+]] = OpLoad %v4int %g
// CHECK-NEXT: [[h:%[0-9]+]] = OpLoad %v4int %h
// CHECK-NEXT: [[g0:%[0-9]+]] = OpCompositeExtract %int [[g]] 0
// CHECK-NEXT: [[h0:%[0-9]+]] = OpCompositeExtract %int [[h]] 0
// CHECK-NEXT: [[g0h0:%[0-9]+]] = OpIMul %int [[g0]] [[h0]]
// CHECK-NEXT: [[g1:%[0-9]+]] = OpCompositeExtract %int [[g]] 1
// CHECK-NEXT: [[h1:%[0-9]+]] = OpCompositeExtract %int [[h]] 1
// CHECK-NEXT: [[g1h1:%[0-9]+]] = OpIMul %int [[g1]] [[h1]]
// CHECK-NEXT: [[g2:%[0-9]+]] = OpCompositeExtract %int [[g]] 2
// CHECK-NEXT: [[h2:%[0-9]+]] = OpCompositeExtract %int [[h]] 2
// CHECK-NEXT: [[g2h2:%[0-9]+]] = OpIMul %int [[g2]] [[h2]]
// CHECK-NEXT: [[g3:%[0-9]+]] = OpCompositeExtract %int [[g]] 3
// CHECK-NEXT: [[h3:%[0-9]+]] = OpCompositeExtract %int [[h]] 3
// CHECK-NEXT: [[g3h3:%[0-9]+]] = OpIMul %int [[g3]] [[h3]]
// CHECK-NEXT: [[add_1:%[0-9]+]] = OpIAdd %int [[g0h0]] [[g1h1]]
// CHECK-NEXT: [[add_2:%[0-9]+]] = OpIAdd %int [[add_1]] [[g2h2]]
// CHECK-NEXT: [[add_3:%[0-9]+]] = OpIAdd %int [[add_2]] [[g3h3]]
// CHECK-NEXT: OpStore %vectorMulVector [[add_3]]
  int vectorMulVector = mul(g,h);

  float3 float_g, float_h;
// CHECK:      [[float_g:%[0-9]+]] = OpLoad %v3float %float_g
// CHECK-NEXT: [[float_h:%[0-9]+]] = OpLoad %v3float %float_h
// CHECK-NEXT: {{%[0-9]+}} = OpDot %float [[float_g]] [[float_h]]
  float float_vectorMulVector = mul(float_g, float_h);

  float4 i;
  float4x3 j;
// CHECK:      [[i:%[0-9]+]] = OpLoad %v4float %i
// CHECK-NEXT: [[j:%[0-9]+]] = OpLoad %mat4v3float %j
// CHECK-NEXT: {{%[0-9]+}} = OpMatrixTimesVector %v3float [[j]] [[i]]
  float3 vectorMulMatrix = mul(i,j);

  float2x3 k;
  float3 l;
// CHECK:      [[k:%[0-9]+]] = OpLoad %mat2v3float %k
// CHECK-NEXT: [[l:%[0-9]+]] = OpLoad %v3float %l
// CHECK-NEXT: {{%[0-9]+}} = OpVectorTimesMatrix %v2float [[l]] [[k]]
  float2 matrixMulVector = mul(k,l);


  float3x4 m;
  float4x2 n;
// CHECK:      [[m:%[0-9]+]] = OpLoad %mat3v4float %m
// CHECK-NEXT: [[n:%[0-9]+]] = OpLoad %mat4v2float %n
// CHECK-NEXT: {{%[0-9]+}} = OpMatrixTimesMatrix %mat3v2float [[n]] [[m]]
  float3x2 matrixMulMatrix = mul(m,n);

///////////////////////////////////////
/// Non-floating point matrix cases ///
///////////////////////////////////////

  uint  uintScalar;
  int   intScalar;
  float floatScalar;

  // Scalar * Matrix
// CHECK:        [[intScalar:%[0-9]+]] = OpLoad %int %intScalar
// CHECK-NEXT:      [[intMat:%[0-9]+]] = OpLoad %_arr_v3int_uint_2 %intMat2x3
// CHECK-NEXT: [[v3intScalar:%[0-9]+]] = OpCompositeConstruct %v3int [[intScalar]] [[intScalar]] [[intScalar]]
// CHECK-NEXT:     [[intMat0:%[0-9]+]] = OpCompositeExtract %v3int [[intMat]] 0
// CHECK-NEXT:        [[mul0:%[0-9]+]] = OpIMul %v3int [[intMat0]] [[v3intScalar]]
// CHECK-NEXT:     [[intMat1:%[0-9]+]] = OpCompositeExtract %v3int [[intMat]] 1
// CHECK-NEXT:        [[mul1:%[0-9]+]] = OpIMul %v3int [[intMat1]] [[v3intScalar]]
// CHECK-NEXT:             {{%[0-9]+}} = OpCompositeConstruct %_arr_v3int_uint_2 [[mul0]] [[mul1]]
  int2x3   intMat2x3;
  int2x3 o = mul(intScalar, intMat2x3);

  // Matrix * Scalar
// CHECK:           [[uintMat:%[0-9]+]] = OpLoad %_arr_v3uint_uint_2 %uintMat2x3
// CHECK-NEXT:   [[uintScalar:%[0-9]+]] = OpLoad %uint %uintScalar
// CHECK-NEXT: [[v3uintScalar:%[0-9]+]] = OpCompositeConstruct %v3uint [[uintScalar]] [[uintScalar]] [[uintScalar]]
// CHECK-NEXT:     [[uintMat0:%[0-9]+]] = OpCompositeExtract %v3uint [[uintMat]] 0
// CHECK-NEXT:         [[mul0_0:%[0-9]+]] = OpIMul %v3uint [[uintMat0]] [[v3uintScalar]]
// CHECK-NEXT:     [[uintMat1:%[0-9]+]] = OpCompositeExtract %v3uint [[uintMat]] 1
// CHECK-NEXT:         [[mul1_0:%[0-9]+]] = OpIMul %v3uint [[uintMat1]] [[v3uintScalar]]
// CHECK-NEXT:              {{%[0-9]+}} = OpCompositeConstruct %_arr_v3uint_uint_2 [[mul0_0]] [[mul1_0]]
  uint2x3  uintMat2x3;
  uint2x3 p = mul(uintMat2x3, uintScalar);

  // Matrix * Scalar (different types)
  // Casting AST nodes are inserted by the front-end. Mul works same as above.
// CHECK:           [[intMat_0:%[0-9]+]] = OpLoad %_arr_v4int_uint_2 %intMat2x4
// CHECK-NEXT:     [[intMat0_0:%[0-9]+]] = OpCompositeExtract %v4int [[intMat_0]] 0
// CHECK-NEXT:   [[floatMat0:%[0-9]+]] = OpConvertSToF %v4float [[intMat0_0]]
// CHECK-NEXT:     [[intMat1_0:%[0-9]+]] = OpCompositeExtract %v4int [[intMat_0]] 1
// CHECK-NEXT:   [[floatMat1:%[0-9]+]] = OpConvertSToF %v4float [[intMat1_0]]
// CHECK-NEXT:    [[floatMat:%[0-9]+]] = OpCompositeConstruct %mat2v4float [[floatMat0]] [[floatMat1]]
// CHECK-NEXT: [[floatScalar:%[0-9]+]] = OpLoad %float %floatScalar
// CHECK-NEXT:             {{%[0-9]+}} = OpMatrixTimesScalar %mat2v4float [[floatMat]] [[floatScalar]]
  int2x4 intMat2x4;
  float2x4 q = mul(intMat2x4, floatScalar);

  // Vector * Matrix
  // First, we need to get vectors for the columns of the matrix, and then perform
  // dot product of the vector and the matrix columns.
// CHECK:               [[intVec:%[0-9]+]] = OpLoad %v2int %intVec2
// CHECK-NEXT:          [[intMat_1:%[0-9]+]] = OpLoad %_arr_v3int_uint_2 %intMat2x3
// CHECK-NEXT:        [[intMat00:%[0-9]+]] = OpCompositeExtract %int [[intMat_1]] 0 0
// CHECK-NEXT:        [[intMat01:%[0-9]+]] = OpCompositeExtract %int [[intMat_1]] 0 1
// CHECK-NEXT:        [[intMat02:%[0-9]+]] = OpCompositeExtract %int [[intMat_1]] 0 2
// CHECK-NEXT:        [[intMat10:%[0-9]+]] = OpCompositeExtract %int [[intMat_1]] 1 0
// CHECK-NEXT:        [[intMat11:%[0-9]+]] = OpCompositeExtract %int [[intMat_1]] 1 1
// CHECK-NEXT:        [[intMat12:%[0-9]+]] = OpCompositeExtract %int [[intMat_1]] 1 2
// CHECK-NEXT:      [[intMatCol0:%[0-9]+]] = OpCompositeConstruct %v2int [[intMat00]] [[intMat10]]
// CHECK-NEXT:      [[intMatCol1:%[0-9]+]] = OpCompositeConstruct %v2int [[intMat01]] [[intMat11]]
// CHECK-NEXT:      [[intMatCol2:%[0-9]+]] = OpCompositeConstruct %v2int [[intMat02]] [[intMat12]]
// CHECK-NEXT: [[intMatTranspose:%[0-9]+]] = OpCompositeConstruct %_arr_v2int_uint_3 [[intMatCol0]] [[intMatCol1]] [[intMatCol2]]
// CHECK-NEXT:      [[intMatCol0_0:%[0-9]+]] = OpCompositeExtract %v2int [[intMatTranspose]] 0
// CHECK-NEXT:         [[intVec0:%[0-9]+]] = OpCompositeExtract %int [[intVec]] 0
// CHECK-NEXT:     [[intMatCol00:%[0-9]+]] = OpCompositeExtract %int [[intMatCol0_0]] 0
// CHECK-NEXT:            [[mul1_1:%[0-9]+]] = OpIMul %int [[intVec0]] [[intMatCol00]]
// CHECK-NEXT:         [[intVec1:%[0-9]+]] = OpCompositeExtract %int [[intVec]] 1
// CHECK-NEXT:     [[intMatCol01:%[0-9]+]] = OpCompositeExtract %int [[intMatCol0_0]] 1
// CHECK-NEXT:            [[mul2:%[0-9]+]] = OpIMul %int [[intVec1]] [[intMatCol01]]
// CHECK-NEXT:              [[r0:%[0-9]+]] = OpIAdd %int [[mul1_1]] [[mul2]]
// CHECK-NEXT:      [[intMatCol1_0:%[0-9]+]] = OpCompositeExtract %v2int [[intMatTranspose]] 1
// CHECK-NEXT:         [[intVec0_0:%[0-9]+]] = OpCompositeExtract %int [[intVec]] 0
// CHECK-NEXT:     [[intMatCol10:%[0-9]+]] = OpCompositeExtract %int [[intMatCol1_0]] 0
// CHECK-NEXT:            [[mul3:%[0-9]+]] = OpIMul %int [[intVec0_0]] [[intMatCol10]]
// CHECK-NEXT:         [[intVec1_0:%[0-9]+]] = OpCompositeExtract %int [[intVec]] 1
// CHECK-NEXT:     [[intMatCol11:%[0-9]+]] = OpCompositeExtract %int [[intMatCol1_0]] 1
// CHECK-NEXT:            [[mul4:%[0-9]+]] = OpIMul %int [[intVec1_0]] [[intMatCol11]]
// CHECK-NEXT:              [[r1:%[0-9]+]] = OpIAdd %int [[mul3]] [[mul4]]
// CHECK-NEXT:      [[intMatCol2_0:%[0-9]+]] = OpCompositeExtract %v2int [[intMatTranspose]] 2
// CHECK-NEXT:         [[intVec0_1:%[0-9]+]] = OpCompositeExtract %int [[intVec]] 0
// CHECK-NEXT:     [[intMatCol20:%[0-9]+]] = OpCompositeExtract %int [[intMatCol2_0]] 0
// CHECK-NEXT:            [[mul5:%[0-9]+]] = OpIMul %int [[intVec0_1]] [[intMatCol20]]
// CHECK-NEXT:         [[intVec1_1:%[0-9]+]] = OpCompositeExtract %int [[intVec]] 1
// CHECK-NEXT:     [[intMatCol21:%[0-9]+]] = OpCompositeExtract %int [[intMatCol2_0]] 1
// CHECK-NEXT:            [[mul6:%[0-9]+]] = OpIMul %int [[intVec1_1]] [[intMatCol21]]
// CHECK-NEXT:              [[r2:%[0-9]+]] = OpIAdd %int [[mul5]] [[mul6]]
// CHECK-NEXT:                 {{%[0-9]+}} = OpCompositeConstruct %v3int [[r0]] [[r1]] [[r2]]
  int2   intVec2;
  int3 r = mul(intVec2, intMat2x3);

  // Matrix * Vector
// CHECK:        [[uintMat_0:%[0-9]+]] = OpLoad %_arr_v2uint_uint_3 %uintMat3x2
// CHECK-NEXT:   [[uintVec:%[0-9]+]] = OpLoad %v2uint %uintVec2
// CHECK-NEXT:  [[uintMat0_0:%[0-9]+]] = OpCompositeExtract %v2uint [[uintMat_0]] 0
// CHECK-NEXT: [[uintMat00:%[0-9]+]] = OpCompositeExtract %uint [[uintMat0_0]] 0
// CHECK-NEXT:  [[uintVec0:%[0-9]+]] = OpCompositeExtract %uint [[uintVec]] 0
// CHECK-NEXT:      [[mul1_2:%[0-9]+]] = OpIMul %uint [[uintMat00]] [[uintVec0]]
// CHECK-NEXT: [[uintMat01:%[0-9]+]] = OpCompositeExtract %uint [[uintMat0_0]] 1
// CHECK-NEXT:  [[uintVec1:%[0-9]+]] = OpCompositeExtract %uint [[uintVec]] 1
// CHECK-NEXT:      [[mul2_0:%[0-9]+]] = OpIMul %uint [[uintMat01]] [[uintVec1]]
// CHECK-NEXT:        [[s0:%[0-9]+]] = OpIAdd %uint [[mul1_2]] [[mul2_0]]
// CHECK-NEXT:  [[uintMat1_0:%[0-9]+]] = OpCompositeExtract %v2uint [[uintMat_0]] 1
// CHECK-NEXT: [[uintMat10:%[0-9]+]] = OpCompositeExtract %uint [[uintMat1_0]] 0
// CHECK-NEXT:  [[uintVec0_0:%[0-9]+]] = OpCompositeExtract %uint [[uintVec]] 0
// CHECK-NEXT:      [[mul3_0:%[0-9]+]] = OpIMul %uint [[uintMat10]] [[uintVec0_0]]
// CHECK-NEXT: [[uintMat11:%[0-9]+]] = OpCompositeExtract %uint [[uintMat1_0]] 1
// CHECK-NEXT:  [[uintVec1_0:%[0-9]+]] = OpCompositeExtract %uint [[uintVec]] 1
// CHECK-NEXT:      [[mul4_0:%[0-9]+]] = OpIMul %uint [[uintMat11]] [[uintVec1_0]]
// CHECK-NEXT:        [[s1:%[0-9]+]] = OpIAdd %uint [[mul3_0]] [[mul4_0]]
// CHECK-NEXT:  [[uintMat2:%[0-9]+]] = OpCompositeExtract %v2uint [[uintMat_0]] 2
// CHECK-NEXT: [[uintMat20:%[0-9]+]] = OpCompositeExtract %uint [[uintMat2]] 0
// CHECK-NEXT:  [[uintVec0_1:%[0-9]+]] = OpCompositeExtract %uint [[uintVec]] 0
// CHECK-NEXT:      [[mul5_0:%[0-9]+]] = OpIMul %uint [[uintMat20]] [[uintVec0_1]]
// CHECK-NEXT: [[uintMat21:%[0-9]+]] = OpCompositeExtract %uint [[uintMat2]] 1
// CHECK-NEXT:  [[uintVec1_1:%[0-9]+]] = OpCompositeExtract %uint [[uintVec]] 1
// CHECK-NEXT:      [[mul6_0:%[0-9]+]] = OpIMul %uint [[uintMat21]] [[uintVec1_1]]
// CHECK-NEXT:        [[s2:%[0-9]+]] = OpIAdd %uint [[mul5_0]] [[mul6_0]]
// CHECK-NEXT:           {{%[0-9]+}} = OpCompositeConstruct %v3uint [[s0]] [[s1]] [[s2]]
  uint2     uintVec2;
  uint3x2   uintMat3x2;
  uint3 s = mul(uintMat3x2, uintVec2);

  // Matrix * Matrix
// CHECK:           [[lhs:%[0-9]+]] = OpLoad %_arr_v4int_uint_2 %intMat2x4
// CHECK-NEXT:      [[rhs:%[0-9]+]] = OpLoad %_arr_v3int_uint_4 %intMat4x3

  ///////////////////////////////////////////
  /////////// Transpose the rhs /////////////
  ///////////////////////////////////////////
// CHECK-NEXT:        [[rhs00:%[0-9]+]] = OpCompositeExtract %int [[rhs]] 0 0
// CHECK-NEXT:        [[rhs01:%[0-9]+]] = OpCompositeExtract %int [[rhs]] 0 1
// CHECK-NEXT:        [[rhs02:%[0-9]+]] = OpCompositeExtract %int [[rhs]] 0 2
// CHECK-NEXT:        [[rhs10:%[0-9]+]] = OpCompositeExtract %int [[rhs]] 1 0
// CHECK-NEXT:        [[rhs11:%[0-9]+]] = OpCompositeExtract %int [[rhs]] 1 1
// CHECK-NEXT:        [[rhs12:%[0-9]+]] = OpCompositeExtract %int [[rhs]] 1 2
// CHECK-NEXT:        [[rhs20:%[0-9]+]] = OpCompositeExtract %int [[rhs]] 2 0
// CHECK-NEXT:        [[rhs21:%[0-9]+]] = OpCompositeExtract %int [[rhs]] 2 1
// CHECK-NEXT:        [[rhs22:%[0-9]+]] = OpCompositeExtract %int [[rhs]] 2 2
// CHECK-NEXT:        [[rhs30:%[0-9]+]] = OpCompositeExtract %int [[rhs]] 3 0
// CHECK-NEXT:        [[rhs31:%[0-9]+]] = OpCompositeExtract %int [[rhs]] 3 1
// CHECK-NEXT:        [[rhs32:%[0-9]+]] = OpCompositeExtract %int [[rhs]] 3 2
// CHECK-NEXT:      [[rhsCol0:%[0-9]+]] = OpCompositeConstruct %v4int [[rhs00]] [[rhs10]] [[rhs20]] [[rhs30]]
// CHECK-NEXT:      [[rhsCol1:%[0-9]+]] = OpCompositeConstruct %v4int [[rhs01]] [[rhs11]] [[rhs21]] [[rhs31]]
// CHECK-NEXT:      [[rhsCol2:%[0-9]+]] = OpCompositeConstruct %v4int [[rhs02]] [[rhs12]] [[rhs22]] [[rhs32]]
// CHECK-NEXT: [[rhsTranspose:%[0-9]+]] = OpCompositeConstruct %_arr_v4int_uint_3 [[rhsCol0]] [[rhsCol1]] [[rhsCol2]]
  ///////////////////////////////////////////
  /////////// End: Transpose the rhs ////////
  ///////////////////////////////////////////

  ///////////////////////////////////////////
  /////////// LHS Row0 *dot* RHS Col0 ///////
  ///////////////////////////////////////////
// CHECK-NEXT:  [[lhsRow0:%[0-9]+]] = OpCompositeExtract %v4int [[lhs]] 0
// CHECK-NEXT:  [[rhsCol0_0:%[0-9]+]] = OpCompositeExtract %v4int [[rhsTranspose]] 0
// CHECK-NEXT: [[lhsRow00:%[0-9]+]] = OpCompositeExtract %int [[lhsRow0]] 0
// CHECK-NEXT: [[rhsCol00:%[0-9]+]] = OpCompositeExtract %int [[rhsCol0_0]] 0
// CHECK-NEXT:     [[mul1_3:%[0-9]+]] = OpIMul %int [[lhsRow00]] [[rhsCol00]]
// CHECK-NEXT: [[lhsRow01:%[0-9]+]] = OpCompositeExtract %int [[lhsRow0]] 1
// CHECK-NEXT: [[rhsCol01:%[0-9]+]] = OpCompositeExtract %int [[rhsCol0_0]] 1
// CHECK-NEXT:     [[mul2_1:%[0-9]+]] = OpIMul %int [[lhsRow01]] [[rhsCol01]]
// CHECK-NEXT: [[lhsRow02:%[0-9]+]] = OpCompositeExtract %int [[lhsRow0]] 2
// CHECK-NEXT: [[rhsCol02:%[0-9]+]] = OpCompositeExtract %int [[rhsCol0_0]] 2
// CHECK-NEXT:     [[mul3_1:%[0-9]+]] = OpIMul %int [[lhsRow02]] [[rhsCol02]]
// CHECK-NEXT: [[lhsRow03:%[0-9]+]] = OpCompositeExtract %int [[lhsRow0]] 3
// CHECK-NEXT: [[rhsCol03:%[0-9]+]] = OpCompositeExtract %int [[rhsCol0_0]] 3
// CHECK-NEXT:     [[mul4_1:%[0-9]+]] = OpIMul %int [[lhsRow03]] [[rhsCol03]]
// CHECK-NEXT:      [[mul:%[0-9]+]] = OpIAdd %int [[mul1_3]] [[mul2_1]]
// CHECK-NEXT:      [[mul_0:%[0-9]+]] = OpIAdd %int [[mul]] [[mul3_1]]
// CHECK-NEXT:      [[t00:%[0-9]+]] = OpIAdd %int [[mul_0]] [[mul4_1]]
  ///////////////////////////////////////////
  ////// END: LHS Row0 *dot* RHS Col0 ///////
  ///////////////////////////////////////////

  ///////////////////////////////////////////
  /////////// LHS Row0 *dot* RHS Col1 ///////
  ///////////////////////////////////////////
// CHECK-NEXT:  [[rhsCol1_0:%[0-9]+]] = OpCompositeExtract %v4int [[rhsTranspose]] 1
// CHECK-NEXT: [[lhsRow00_0:%[0-9]+]] = OpCompositeExtract %int [[lhsRow0]] 0
// CHECK-NEXT: [[rhsCol10:%[0-9]+]] = OpCompositeExtract %int [[rhsCol1_0]] 0
// CHECK-NEXT:     [[mul5_1:%[0-9]+]] = OpIMul %int [[lhsRow00_0]] [[rhsCol10]]
// CHECK-NEXT: [[lhsRow01_0:%[0-9]+]] = OpCompositeExtract %int [[lhsRow0]] 1
// CHECK-NEXT: [[rhsCol11:%[0-9]+]] = OpCompositeExtract %int [[rhsCol1_0]] 1
// CHECK-NEXT:     [[mul6_1:%[0-9]+]] = OpIMul %int [[lhsRow01_0]] [[rhsCol11]]
// CHECK-NEXT: [[lhsRow02_0:%[0-9]+]] = OpCompositeExtract %int [[lhsRow0]] 2
// CHECK-NEXT: [[rhsCol12:%[0-9]+]] = OpCompositeExtract %int [[rhsCol1_0]] 2
// CHECK-NEXT:     [[mul7:%[0-9]+]] = OpIMul %int [[lhsRow02_0]] [[rhsCol12]]
// CHECK-NEXT: [[lhsRow03_0:%[0-9]+]] = OpCompositeExtract %int [[lhsRow0]] 3
// CHECK-NEXT: [[rhsCol13:%[0-9]+]] = OpCompositeExtract %int [[rhsCol1_0]] 3
// CHECK-NEXT:     [[mul8:%[0-9]+]] = OpIMul %int [[lhsRow03_0]] [[rhsCol13]]
// CHECK-NEXT:      [[mul_1:%[0-9]+]] = OpIAdd %int [[mul5_1]] [[mul6_1]]
// CHECK-NEXT:      [[mul_2:%[0-9]+]] = OpIAdd %int [[mul_1]] [[mul7]]
// CHECK-NEXT:      [[t01:%[0-9]+]] = OpIAdd %int [[mul_2]] [[mul8]]
  ///////////////////////////////////////////
  ////// END: LHS Row0 *dot* RHS Col1 ///////
  ///////////////////////////////////////////

  ///////////////////////////////////////////
  /////////// LHS Row0 *dot* RHS Col2 ///////
  ///////////////////////////////////////////
// CHECK-NEXT:  [[rhsCol2_0:%[0-9]+]] = OpCompositeExtract %v4int [[rhsTranspose]] 2
// CHECK-NEXT: [[lhsRow00_1:%[0-9]+]] = OpCompositeExtract %int [[lhsRow0]] 0
// CHECK-NEXT: [[rhsCol20:%[0-9]+]] = OpCompositeExtract %int [[rhsCol2_0]] 0
// CHECK-NEXT:     [[mul9:%[0-9]+]] = OpIMul %int [[lhsRow00_1]] [[rhsCol20]]
// CHECK-NEXT: [[lhsRow01_1:%[0-9]+]] = OpCompositeExtract %int [[lhsRow0]] 1
// CHECK-NEXT: [[rhsCol21:%[0-9]+]] = OpCompositeExtract %int [[rhsCol2_0]] 1
// CHECK-NEXT:    [[mul10:%[0-9]+]] = OpIMul %int [[lhsRow01_1]] [[rhsCol21]]
// CHECK-NEXT: [[lhsRow02_1:%[0-9]+]] = OpCompositeExtract %int [[lhsRow0]] 2
// CHECK-NEXT: [[rhsCol22:%[0-9]+]] = OpCompositeExtract %int [[rhsCol2_0]] 2
// CHECK-NEXT:    [[mul11:%[0-9]+]] = OpIMul %int [[lhsRow02_1]] [[rhsCol22]]
// CHECK-NEXT: [[lhsRow03_1:%[0-9]+]] = OpCompositeExtract %int [[lhsRow0]] 3
// CHECK-NEXT: [[rhsCol23:%[0-9]+]] = OpCompositeExtract %int [[rhsCol2_0]] 3
// CHECK-NEXT:    [[mul12:%[0-9]+]] = OpIMul %int [[lhsRow03_1]] [[rhsCol23]]
// CHECK-NEXT:      [[mul_3:%[0-9]+]] = OpIAdd %int [[mul9]] [[mul10]]
// CHECK-NEXT:      [[mul_4:%[0-9]+]] = OpIAdd %int [[mul_3]] [[mul11]]
// CHECK-NEXT:      [[t02:%[0-9]+]] = OpIAdd %int [[mul_4]] [[mul12]]
  ///////////////////////////////////////////
  ////// END: LHS Row0 *dot* RHS Col2 ///////
  ///////////////////////////////////////////

// Result row 0:
// CHECK-NEXT: [[t0:%[0-9]+]] = OpCompositeConstruct %v3int [[t00]] [[t01]] [[t02]]

  ///////////////////////////////////////////
  /////////// LHS Row1 *dot* RHS Col0 ///////
  ///////////////////////////////////////////
// CHECK-NEXT:  [[lhsRow1:%[0-9]+]] = OpCompositeExtract %v4int [[lhs]] 1
// CHECK-NEXT:  [[rhsCol0_1:%[0-9]+]] = OpCompositeExtract %v4int [[rhsTranspose]] 0
// CHECK-NEXT: [[lhsRow10:%[0-9]+]] = OpCompositeExtract %int [[lhsRow1]] 0
// CHECK-NEXT: [[rhsCol00_0:%[0-9]+]] = OpCompositeExtract %int [[rhsCol0_1]] 0
// CHECK-NEXT:     [[mul1_4:%[0-9]+]] = OpIMul %int [[lhsRow10]] [[rhsCol00_0]]
// CHECK-NEXT: [[lhsRow11:%[0-9]+]] = OpCompositeExtract %int [[lhsRow1]] 1
// CHECK-NEXT: [[rhsCol01_0:%[0-9]+]] = OpCompositeExtract %int [[rhsCol0_1]] 1
// CHECK-NEXT:     [[mul2_2:%[0-9]+]] = OpIMul %int [[lhsRow11]] [[rhsCol01_0]]
// CHECK-NEXT: [[lhsRow12:%[0-9]+]] = OpCompositeExtract %int [[lhsRow1]] 2
// CHECK-NEXT: [[rhsCol02_0:%[0-9]+]] = OpCompositeExtract %int [[rhsCol0_1]] 2
// CHECK-NEXT:     [[mul3_2:%[0-9]+]] = OpIMul %int [[lhsRow12]] [[rhsCol02_0]]
// CHECK-NEXT: [[lhsRow13:%[0-9]+]] = OpCompositeExtract %int [[lhsRow1]] 3
// CHECK-NEXT: [[rhsCol03_0:%[0-9]+]] = OpCompositeExtract %int [[rhsCol0_1]] 3
// CHECK-NEXT:     [[mul4_2:%[0-9]+]] = OpIMul %int [[lhsRow13]] [[rhsCol03_0]]
// CHECK-NEXT:      [[mul_5:%[0-9]+]] = OpIAdd %int [[mul1_4]] [[mul2_2]]
// CHECK-NEXT:      [[mul_6:%[0-9]+]] = OpIAdd %int [[mul_5]] [[mul3_2]]
// CHECK-NEXT:      [[t10:%[0-9]+]] = OpIAdd %int [[mul_6]] [[mul4_2]]
  ///////////////////////////////////////////
  ////// END: LHS Row1 *dot* RHS Col0 ///////
  ///////////////////////////////////////////

  ///////////////////////////////////////////
  /////////// LHS Row1 *dot* RHS Col1 ///////
  ///////////////////////////////////////////
// CHECK-NEXT:  [[rhsCol1_1:%[0-9]+]] = OpCompositeExtract %v4int [[rhsTranspose]] 1
// CHECK-NEXT: [[lhsRow10_0:%[0-9]+]] = OpCompositeExtract %int [[lhsRow1]] 0
// CHECK-NEXT: [[rhsCol10_0:%[0-9]+]] = OpCompositeExtract %int [[rhsCol1_1]] 0
// CHECK-NEXT:     [[mul5_2:%[0-9]+]] = OpIMul %int [[lhsRow10_0]] [[rhsCol10_0]]
// CHECK-NEXT: [[lhsRow11_0:%[0-9]+]] = OpCompositeExtract %int [[lhsRow1]] 1
// CHECK-NEXT: [[rhsCol11_0:%[0-9]+]] = OpCompositeExtract %int [[rhsCol1_1]] 1
// CHECK-NEXT:     [[mul6_2:%[0-9]+]] = OpIMul %int [[lhsRow11_0]] [[rhsCol11_0]]
// CHECK-NEXT: [[lhsRow12_0:%[0-9]+]] = OpCompositeExtract %int [[lhsRow1]] 2
// CHECK-NEXT: [[rhsCol12_0:%[0-9]+]] = OpCompositeExtract %int [[rhsCol1_1]] 2
// CHECK-NEXT:     [[mul7_0:%[0-9]+]] = OpIMul %int [[lhsRow12_0]] [[rhsCol12_0]]
// CHECK-NEXT: [[lhsRow13_0:%[0-9]+]] = OpCompositeExtract %int [[lhsRow1]] 3
// CHECK-NEXT: [[rhsCol13_0:%[0-9]+]] = OpCompositeExtract %int [[rhsCol1_1]] 3
// CHECK-NEXT:     [[mul8_0:%[0-9]+]] = OpIMul %int [[lhsRow13_0]] [[rhsCol13_0]]
// CHECK-NEXT:      [[mul_7:%[0-9]+]] = OpIAdd %int [[mul5_2]] [[mul6_2]]
// CHECK-NEXT:      [[mul_8:%[0-9]+]] = OpIAdd %int [[mul_7]] [[mul7_0]]
// CHECK-NEXT:      [[t11:%[0-9]+]] = OpIAdd %int [[mul_8]] [[mul8_0]]
  ///////////////////////////////////////////
  ////// END: LHS Row1 *dot* RHS Col1 ///////
  ///////////////////////////////////////////

  ///////////////////////////////////////////
  /////////// LHS Row1 *dot* RHS Col2 ///////
  ///////////////////////////////////////////
// CHECK-NEXT:  [[rhsCol2_1:%[0-9]+]] = OpCompositeExtract %v4int [[rhsTranspose]] 2
// CHECK-NEXT: [[lhsRow10_1:%[0-9]+]] = OpCompositeExtract %int [[lhsRow1]] 0
// CHECK-NEXT: [[rhsCol20_0:%[0-9]+]] = OpCompositeExtract %int [[rhsCol2_1]] 0
// CHECK-NEXT:     [[mul9_0:%[0-9]+]] = OpIMul %int [[lhsRow10_1]] [[rhsCol20_0]]
// CHECK-NEXT: [[lhsRow11_1:%[0-9]+]] = OpCompositeExtract %int [[lhsRow1]] 1
// CHECK-NEXT: [[rhsCol21_0:%[0-9]+]] = OpCompositeExtract %int [[rhsCol2_1]] 1
// CHECK-NEXT:    [[mul10_0:%[0-9]+]] = OpIMul %int [[lhsRow11_1]] [[rhsCol21_0]]
// CHECK-NEXT: [[lhsRow12_1:%[0-9]+]] = OpCompositeExtract %int [[lhsRow1]] 2
// CHECK-NEXT: [[rhsCol22_0:%[0-9]+]] = OpCompositeExtract %int [[rhsCol2_1]] 2
// CHECK-NEXT:    [[mul11_0:%[0-9]+]] = OpIMul %int [[lhsRow12_1]] [[rhsCol22_0]]
// CHECK-NEXT: [[lhsRow13_1:%[0-9]+]] = OpCompositeExtract %int [[lhsRow1]] 3
// CHECK-NEXT: [[rhsCol23_0:%[0-9]+]] = OpCompositeExtract %int [[rhsCol2_1]] 3
// CHECK-NEXT:    [[mul12_0:%[0-9]+]] = OpIMul %int [[lhsRow13_1]] [[rhsCol23_0]]
// CHECK-NEXT:      [[mul_9:%[0-9]+]] = OpIAdd %int [[mul9_0]] [[mul10_0]]
// CHECK-NEXT:      [[mul_10:%[0-9]+]] = OpIAdd %int [[mul_9]] [[mul11_0]]
// CHECK-NEXT:      [[t12:%[0-9]+]] = OpIAdd %int [[mul_10]] [[mul12_0]]
  ///////////////////////////////////////////
  ////// END: LHS Row1 *dot* RHS Col2 ///////
  ///////////////////////////////////////////

// Result row 1:
// CHECK-NEXT: [[t1:%[0-9]+]] = OpCompositeConstruct %v3int [[t10]] [[t11]] [[t12]]

// Final result:
// CHECK-NEXT:    {{%[0-9]+}} = OpCompositeConstruct %_arr_v3int_uint_2 [[t0]] [[t1]]
  int4x3 intMat4x3;
  int2x3 t = mul(intMat2x4, intMat4x3);


//
// 1-D matrices passed to mul
//

// mul( Mat(1xM) * Mat(MxN) ) --> Mat(1xN) vector
// mul( Mat(1xM) * Mat(Mx1) ) --> Scalar
// mul( Mat(Mx1) * Mat(1xN) ) --> Mat(MxN) matrix
  float1x3 mat1x3;
  float3x2 mat3x2;
  float3x3 mat3x3;
  float3x1 mat3x1;
  float1x4 mat1x4;

// CHECK:       [[mat1x3:%[0-9]+]] = OpLoad %v3float %mat1x3
// CHECK-NEXT:  [[mat3x2:%[0-9]+]] = OpLoad %mat3v2float %mat3x2
// CHECK-NEXT: [[result1:%[0-9]+]] = OpMatrixTimesVector %v2float [[mat3x2]] [[mat1x3]]
// CHECK-NEXT:                    OpStore %result1 [[result1]]
  float1x2   result1 = mul( mat1x3, mat3x2 ); // result is float2 vector

// CHECK:       [[mat1x3_0:%[0-9]+]] = OpLoad %v3float %mat1x3
// CHECK-NEXT:  [[mat3x1:%[0-9]+]] = OpLoad %v3float %mat3x1
// CHECK-NEXT: [[result2:%[0-9]+]] = OpDot %float [[mat1x3_0]] [[mat3x1]]
// CHECK-NEXT:                    OpStore %result2 [[result2]]
  float      result2 = mul( mat1x3, mat3x1 ); // result is scalar

// CHECK:       [[mat3x1_0:%[0-9]+]] = OpLoad %v3float %mat3x1
// CHECK-NEXT:  [[mat1x4:%[0-9]+]] = OpLoad %v4float %mat1x4
// CHECK-NEXT:   [[elem0:%[0-9]+]] = OpCompositeExtract %float [[mat3x1_0]] 0
// CHECK-NEXT:    [[row0:%[0-9]+]] = OpVectorTimesScalar %v4float [[mat1x4]] [[elem0]]
// CHECK-NEXT:   [[elem1:%[0-9]+]] = OpCompositeExtract %float [[mat3x1_0]] 1
// CHECK-NEXT:    [[row1:%[0-9]+]] = OpVectorTimesScalar %v4float [[mat1x4]] [[elem1]]
// CHECK-NEXT:   [[elem2:%[0-9]+]] = OpCompositeExtract %float [[mat3x1_0]] 2
// CHECK-NEXT:    [[row2:%[0-9]+]] = OpVectorTimesScalar %v4float [[mat1x4]] [[elem2]]
// CHECK-NEXT: [[result3:%[0-9]+]] = OpCompositeConstruct %mat3v4float [[row0]] [[row1]] [[row2]]
// CHECK-NEXT:                    OpStore %result3 [[result3]]
  float3x4   result3 = mul( mat3x1, mat1x4 ); // result is float3x4 matrix

  float3 v3;

// CHECK: [[matp:%[0-9]+]] = OpAccessChain %_ptr_Uniform_mat3v3float %buffer_mat %int_0 %int_0
// CHECK:  [[mat:%[0-9]+]] = OpLoad %mat3v3float [[matp]]
// CHECK:  [[vec:%[0-9]+]] = OpLoad %v3float %v3
// CHECK:           {{.*}} = OpVectorTimesMatrix %v3float [[vec]] [[mat]]
  float3 result4 = mul(buffer_mat.Load(0), v3);

// CHECK:  [[mat:%[0-9]+]] = OpLoad %mat3v3float %mat3x3
// CHECK: [[vecp:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v3float %buffer_vec %int_0 %int_1
// CHECK:  [[vec:%[0-9]+]] = OpLoad %v3float [[vecp]]
// CHECK:           {{.*}} = OpVectorTimesMatrix %v3float [[vec]] [[mat]]
  float3 result5 = mul(mat3x3, buffer_vec.Load(1));

// CHECK: [[matp:%[0-9]+]] = OpAccessChain %_ptr_Uniform_mat3v3float %buffer_mat %int_0 %int_2
// CHECK:  [[mat:%[0-9]+]] = OpLoad %mat3v3float [[matp]]
// CHECK: [[vecp:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v3float %buffer_vec %int_0 %int_2
// CHECK:  [[vec:%[0-9]+]] = OpLoad %v3float [[vecp]]
// CHECK:           {{.*}} = OpVectorTimesMatrix %v3float [[vec]] [[mat]]
  float3 result6 = mul(buffer_mat.Load(2), buffer_vec.Load(2));
}

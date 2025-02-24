// RUN: %dxc -Tlib_6_3 -enable-16bit-types   -verify -HV 2018 %s
// RUN: %dxc -Tvs_6_2 -enable-16bit-types   -verify -HV 2018 %s

// To test with the classic compiler, run
// %sdxroot%\tools\x86\fxc.exe /T vs_5_1 matrix-syntax.hlsl

matrix m;

void abs_without_using_result() {
    matrix<float, 4, 4> mymatrix;
    abs(mymatrix);            /* expected-warning {{ignoring return value of function declared with const attribute}} fxc-pass {{}} */

    matrix<float, 1, 4> mymatrix2;
    abs(mymatrix2);           /* expected-warning {{ignoring return value of function declared with const attribute}} fxc-pass {{}} */
}

void abs_with_assignment() {
    matrix<float, 4, 4> mymatrix;
    matrix<float, 4, 4> absMatrix;
    absMatrix = abs(mymatrix);
}

matrix<float, 4, 4> abs_for_result(matrix<float, 4, 4> value) {
    return abs(value);
}

void fn_use_matrix(matrix<float, 4, 4> value) { }

void abs_in_argument() {
    matrix<float, 4, 4> mymatrix;
    fn_use_matrix(abs(mymatrix));
    /*verify-ast
      CallExpr <col:5, col:32> 'void'
      |-ImplicitCastExpr <col:5> 'void (*)(matrix<float, 4, 4>)' <FunctionToPointerDecay>
      | `-DeclRefExpr <col:5> 'void (matrix<float, 4, 4>)' lvalue Function 'fn_use_matrix' 'void (matrix<float, 4, 4>)'
      `-CallExpr <col:19, col:31> 'matrix<float, 4, 4>':'matrix<float, 4, 4>'
        |-ImplicitCastExpr <col:19> 'matrix<float, 4, 4> (*)(matrix<float, 4, 4>)' <FunctionToPointerDecay>
        | `-DeclRefExpr <col:19> 'matrix<float, 4, 4> (matrix<float, 4, 4>)' lvalue Function 'abs' 'matrix<float, 4, 4> (matrix<float, 4, 4>)'
        `-ImplicitCastExpr <col:23> 'matrix<float, 4, 4>':'matrix<float, 4, 4>' <LValueToRValue>
          `-DeclRefExpr <col:23> 'matrix<float, 4, 4>':'matrix<float, 4, 4>' lvalue Var 'mymatrix' 'matrix<float, 4, 4>':'matrix<float, 4, 4>'
    */
}

void matrix_on_demand() {
    float4x4 thematrix;
    float4x4 anotherMatrix;
    bool2x1 boolMatrix;
    unsigned int4x2 unsignedMatrix;
}

void abs_on_demand() {
   float1x2 f12;
   float1x2 result = abs(f12);
}

void matrix_out_of_bounds() {
  matrix<float, 1, 8> matrix_oob_0; // expected-error {{invalid value, valid range is between 1 and 4 inclusive}} fxc-error {{X3053: matrix dimensions must be between 1 and 4}}
  matrix<float, 0, 1> matrix_oob_1; // expected-error {{invalid value, valid range is between 1 and 4 inclusive}} fxc-error {{X3053: matrix dimensions must be between 1 and 4}}
  matrix<float, -1, 1> matrix_oob_2; // expected-error {{invalid value, valid range is between 1 and 4 inclusive}} fxc-error {{X3053: matrix dimensions must be between 1 and 4}}
}

void matrix_unsigned() {
   unsigned int4x2 intMatrix;
   unsigned min16int4x3 min16Matrix; /* expected-warning {{'min16uint' is promoted to 'uint16_t'}} fxc-error {{X3085: unsigned can not be used with type}} */
   unsigned int64_t3x3 int64Matrix;  /* fxc-error {{X3000: syntax error: unexpected token 'int64_t3x3'}} */
   unsigned uint3x4 uintMatrix;      /* fxc-error {{X3085: unsigned can not be used with type}} */
   unsigned min16uint4x1 min16uintMatrix;                   /* expected-warning {{'min16uint' is promoted to 'uint16_t'}} fxc-error {{X3085: unsigned can not be used with type}} */
   unsigned uint64_t2x2 int64uintMatrix;                    /* fxc-error {{X3000: syntax error: unexpected token 'uint64_t2x2'}} */
   unsigned dword3x2 dwordmector; /* fxc-error {{X3000: syntax error: unexpected token 'dword3x2'}} */

   unsigned float2x3 floatMatrix; /* expected-error {{'float' cannot be signed or unsigned}} fxc-error {{X3085: unsigned can not be used with type}} */
   unsigned bool3x4 boolMatirx;   /* expected-error {{'bool' cannot be signed or unsigned}} fxc-error {{X3085: unsigned can not be used with type}} */
   unsigned half4x1 halfMatrix;   /* expected-error {{'half' cannot be signed or unsigned}} fxc-error {{X3085: unsigned can not be used with type}} */
   unsigned double1x2 doubleMatrix;                           /* expected-error {{'double' cannot be signed or unsigned}} fxc-error {{X3085: unsigned can not be used with type}} */
   unsigned min12int2x3 min12intMatrix;                       /* expected-error {{'min12int' cannot be signed or unsigned}} expected-warning {{'min12int' is promoted to 'int16_t'}} fxc-error {{X3085: unsigned can not be used with type}} */
   unsigned min16float3x4 min16floatMatrix;                   /* expected-error {{'min16float' cannot be signed or unsigned}} expected-warning {{'min16float' is promoted to 'half'}} fxc-error {{X3085: unsigned can not be used with type}} */

   unsigned int16_t2x3 uint16_tMatrix1;                       /* fxc-error {{X3000: syntax error: unexpected token 'int16_t2x3'}} */
   unsigned int32_t4x2 uint32_tMatrix1;                       /* fxc-error {{X3000: syntax error: unexpected token 'int32_t4x2'}} */
   unsigned int64_t1x4 uint64_tMatrix1;                       /* fxc-error {{X3000: syntax error: unexpected token 'int64_t1x4'}} */
}

[shader("vertex")]
void main() {
    // Multiple assignments in a chain.
    matrix<float, 4, 4> mymatrix;
    matrix<float, 4, 4> absMatrix = abs(mymatrix);
    matrix<float, 4, 4> absMatrix2 = abs(absMatrix);
    const matrix<float, 4, 4> myConstMatrix = mymatrix;                /* expected-note {{variable 'myConstMatrix' declared const here}} expected-note {{variable 'myConstMatrix' declared const here}} fxc-pass {{}} */

    matrix<float, 2, 4> f24;
    float f;
    float2 f2;
    float3 f3;
    float4 f4;
    float farr2[2];

    // zero-based positions.
    f = mymatrix._m00;
    f2 = mymatrix._m00_m11;
    f4 = mymatrix._m00_m11_m00_m11;
    /*verify-ast
      BinaryOperator <col:5, col:19> 'float4':'vector<float, 4>' '='
      |-DeclRefExpr <col:5> 'float4':'vector<float, 4>' lvalue Var 'f4' 'float4':'vector<float, 4>'
      `-ExtMatrixElementExpr <col:10, col:19> 'vector<float, 4>':'vector<float, 4>' _m00_m11_m00_m11
        `-DeclRefExpr <col:10> 'matrix<float, 4, 4>':'matrix<float, 4, 4>' lvalue Var 'mymatrix' 'matrix<float, 4, 4>':'matrix<float, 4, 4>'
    */
    //fxc error X3018 : invalid subscript '_m00_11'
    f2 = mymatrix._m00_11; // expected-error {{matrix subscript '_m00_11' mixes one-based and zero-based references}} fxc-error {{X3018: invalid subscript '_m00_11'}}
    //fxc error X3018: invalid subscript '_m00_m11_m00_m11_m00_m11_m00_m11'
    f24 = mymatrix._m00_m11_m00_m11_m00_m11_m00_m11; // expected-error {{more than four positions are referenced in '_m00_m11_m00_m11_m00_m11_m00_m11'}} fxc-error {{X3018: invalid subscript '_m00_m11_m00_m11_m00_m11_m00_m11'}}
    //fxc error X3017: cannot convert from 'float2' to 'float[2]'
    farr2 = mymatrix._m00_m01; // expected-error {{cannot implicitly convert from 'vector<float, 2>' to 'float [2]'}} fxc-error {{X3017: cannot convert from 'float2' to 'float[2]'}}
    //fxc error X3018: invalid subscript '_m04'
    f = mymatrix._m04; // expected-error {{the digit '4' is used in '_m04', but the syntax is for zero-based rows and columns}} fxc-error {{X3018: invalid subscript '_m04'}}
    f2 = mymatrix._m00_m01;
    //fxc error X3017: cannot implicitly convert from 'float2' to 'float3'
    f3 = mymatrix._m00_m01; // expected-error {{cannot convert from 'vector<float, 2>' to 'float3'}} fxc-error {{X3017: cannot implicitly convert from 'float2' to 'float3'}}
    //fxc warning X3206: implicit truncation of vector type
    f2 = mymatrix._m00_m01_m00; // expected-warning {{implicit truncation of vector type}} fxc-warning {{X3206: implicit truncation of vector type}}
    mymatrix._m00 = mymatrix._m01;
    mymatrix._m00_m11_m02_m13 = mymatrix._m10_m21_m10_m21;
    /*verify-ast
      BinaryOperator <col:5, col:42> 'vector<float, 4>':'vector<float, 4>' '='
      |-ExtMatrixElementExpr <col:5, col:14> 'vector<float, 4>':'vector<float, 4>' lvalue vectorcomponent _m00_m11_m02_m13
      | `-DeclRefExpr <col:5> 'matrix<float, 4, 4>':'matrix<float, 4, 4>' lvalue Var 'mymatrix' 'matrix<float, 4, 4>':'matrix<float, 4, 4>'
      `-ExtMatrixElementExpr <col:33, col:42> 'vector<float, 4>':'vector<float, 4>' _m10_m21_m10_m21
        `-DeclRefExpr <col:33> 'matrix<float, 4, 4>':'matrix<float, 4, 4>' lvalue Var 'mymatrix' 'matrix<float, 4, 4>':'matrix<float, 4, 4>'
    */
    //fxc error X3025: l-value specifies const object
    mymatrix._m00_m11_m00_m11 = mymatrix._m10_m21_m10_m21; // expected-error {{matrix is not assignable (contains duplicate components)}} fxc-error {{X3025: l-value specifies const object}}

    // one-based positions.
    //fxc error X3018: invalid subscript '_00'
    f = mymatrix._00; // expected-error {{the digit '0' is used in '_00', but the syntax is for one-based rows and columns}} fxc-error {{X3018: invalid subscript '_00'}}
    f = mymatrix._11;
    f2 = mymatrix._11_11;
    f4 = mymatrix._11_11_44_44;
    /*verify-ast
      BinaryOperator <col:5, col:19> 'float4':'vector<float, 4>' '='
      |-DeclRefExpr <col:5> 'float4':'vector<float, 4>' lvalue Var 'f4' 'float4':'vector<float, 4>'
      `-ExtMatrixElementExpr <col:10, col:19> 'vector<float, 4>':'vector<float, 4>' _11_11_44_44
        `-DeclRefExpr <col:10> 'matrix<float, 4, 4>':'matrix<float, 4, 4>' lvalue Var 'mymatrix' 'matrix<float, 4, 4>':'matrix<float, 4, 4>'
    */
    // member assignment using subscript syntax
    f = mymatrix[0][0];
    f = mymatrix[1][1];
    f2 = mymatrix[1].xx;
    f4 = mymatrix[2];

    f = mymatrix[0][4];                                     /* expected-error {{vector element index '4' is out of bounds}} fxc-pass {{}} */
    f = mymatrix[-1][3];                                    /* expected-error {{matrix row index '-1' is out of bounds}} fxc-pass {{}} */
    f4 = mymatrix[10];                                      /* expected-error {{matrix row index '10' is out of bounds}} fxc-pass {{}} */

    // accessing const member
    f = myConstMatrix[0][0];
    f = myConstMatrix[1][1];
    f2 = myConstMatrix[1].xx;
    f4 = myConstMatrix[2];

    myConstMatrix[0][0] = 3;                                /* expected-error {{cannot assign to variable 'myConstMatrix' with const-qualified type 'const matrix<float, 4, 4>'}} fxc-error {{X3025: l-value specifies const object}} */
    myConstMatrix[3] = float4(1,2,3,4);                     /* expected-error {{cannot assign to variable 'myConstMatrix' with const-qualified type 'const matrix<float, 4, 4>'}} fxc-error {{X3025: l-value specifies const object}} */

    // vector for fixed width data types
    int16_t1x3 int16_t1x3Matrix1 = 0;                       /* fxc-error {{X3000: unrecognized identifier 'int16_t1x3'}} fxc-error {{X3000: unrecognized identifier 'int16_t1x3Matrix1'}} */
    int16_t2x4 int16_t1x3Matrix2 = 1;                       /* fxc-error {{X3000: unrecognized identifier 'int16_t1x3Matrix2'}} fxc-error {{X3000: unrecognized identifier 'int16_t2x4'}} */
    int16_t3x2 int16_t1x3Matrix3 = 2;                       /* fxc-error {{X3000: unrecognized identifier 'int16_t1x3Matrix3'}} fxc-error {{X3000: unrecognized identifier 'int16_t3x2'}} */
    int16_t4x2 int16_t1x3Matrix4 = 3;                       /* fxc-error {{X3000: unrecognized identifier 'int16_t1x3Matrix4'}} fxc-error {{X3000: unrecognized identifier 'int16_t4x2'}} */
    int32_t1x3 int32_t1Matrix1 = 4;                         /* fxc-error {{X3000: unrecognized identifier 'int32_t1Matrix1'}} fxc-error {{X3000: unrecognized identifier 'int32_t1x3'}} */
    int32_t2x2 int32_t1Matrix2 = 5;                         /* fxc-error {{X3000: unrecognized identifier 'int32_t1Matrix2'}} fxc-error {{X3000: unrecognized identifier 'int32_t2x2'}} */
    int32_t3x4 int32_t1Matrix3 = 6;                         /* fxc-error {{X3000: unrecognized identifier 'int32_t1Matrix3'}} fxc-error {{X3000: unrecognized identifier 'int32_t3x4'}} */
    int32_t4x1 int32_t1Matrix4 = 7;                         /* fxc-error {{X3000: unrecognized identifier 'int32_t1Matrix4'}} fxc-error {{X3000: unrecognized identifier 'int32_t4x1'}} */
    int64_t1x2 int64_t1Matrix1 = 8;                         /* fxc-error {{X3000: unrecognized identifier 'int64_t1Matrix1'}} fxc-error {{X3000: unrecognized identifier 'int64_t1x2'}} */
    int64_t2x3 int64_t1Matrix2 = 9;                         /* fxc-error {{X3000: unrecognized identifier 'int64_t1Matrix2'}} fxc-error {{X3000: unrecognized identifier 'int64_t2x3'}} */
    int64_t3x3 int64_t1Matrix3 = 10;                        /* fxc-error {{X3000: unrecognized identifier 'int64_t1Matrix3'}} fxc-error {{X3000: unrecognized identifier 'int64_t3x3'}} */
    int64_t4x4 int64_t1Matrix4 = 11;                        /* fxc-error {{X3000: unrecognized identifier 'int64_t1Matrix4'}} fxc-error {{X3000: unrecognized identifier 'int64_t4x4'}} */

    uint16_t1x3 uint16_t1x3Matrix1 = 0;                     /* fxc-error {{X3000: unrecognized identifier 'uint16_t1x3'}} fxc-error {{X3000: unrecognized identifier 'uint16_t1x3Matrix1'}} */
    uint16_t2x4 uint16_t1x3Matrix2 = 1;                     /* fxc-error {{X3000: unrecognized identifier 'uint16_t1x3Matrix2'}} fxc-error {{X3000: unrecognized identifier 'uint16_t2x4'}} */
    uint16_t3x2 uint16_t1x3Matrix3 = 2;                     /* fxc-error {{X3000: unrecognized identifier 'uint16_t1x3Matrix3'}} fxc-error {{X3000: unrecognized identifier 'uint16_t3x2'}} */
    uint16_t4x2 uint16_t1x3Matrix4 = 3;                     /* fxc-error {{X3000: unrecognized identifier 'uint16_t1x3Matrix4'}} fxc-error {{X3000: unrecognized identifier 'uint16_t4x2'}} */
    uint32_t1x3 uint32_t1Matrix1 = 4;                       /* fxc-error {{X3000: unrecognized identifier 'uint32_t1Matrix1'}} fxc-error {{X3000: unrecognized identifier 'uint32_t1x3'}} */
    uint32_t2x2 uint32_t1Matrix2 = 5;                       /* fxc-error {{X3000: unrecognized identifier 'uint32_t1Matrix2'}} fxc-error {{X3000: unrecognized identifier 'uint32_t2x2'}} */
    uint32_t3x4 uint32_t1Matrix3 = 6;                       /* fxc-error {{X3000: unrecognized identifier 'uint32_t1Matrix3'}} fxc-error {{X3000: unrecognized identifier 'uint32_t3x4'}} */
    uint32_t4x1 uint32_t1Matrix4 = 7;                       /* fxc-error {{X3000: unrecognized identifier 'uint32_t1Matrix4'}} fxc-error {{X3000: unrecognized identifier 'uint32_t4x1'}} */
    uint64_t1x2 uint64_t1Matrix1 = 8;                       /* fxc-error {{X3000: unrecognized identifier 'uint64_t1Matrix1'}} fxc-error {{X3000: unrecognized identifier 'uint64_t1x2'}} */
    uint64_t2x3 uint64_t1Matrix2 = 9;                       /* fxc-error {{X3000: unrecognized identifier 'uint64_t1Matrix2'}} fxc-error {{X3000: unrecognized identifier 'uint64_t2x3'}} */
    uint64_t3x3 uint64_t1Matrix3 = 10;                      /* fxc-error {{X3000: unrecognized identifier 'uint64_t1Matrix3'}} fxc-error {{X3000: unrecognized identifier 'uint64_t3x3'}} */
    uint64_t4x4 uint64_t1Matrix4 = 11;                      /* fxc-error {{X3000: unrecognized identifier 'uint64_t1Matrix4'}} fxc-error {{X3000: unrecognized identifier 'uint64_t4x4'}} */

    float16_t1x3 float16_t1x3Matrix1 = 0;                   /* fxc-error {{X3000: unrecognized identifier 'float16_t1x3'}} fxc-error {{X3000: unrecognized identifier 'float16_t1x3Matrix1'}} */
    float16_t2x4 float16_t1x3Matrix2 = 1;                   /* fxc-error {{X3000: unrecognized identifier 'float16_t1x3Matrix2'}} fxc-error {{X3000: unrecognized identifier 'float16_t2x4'}} */
    float16_t3x2 float16_t1x3Matrix3 = 2;                   /* fxc-error {{X3000: unrecognized identifier 'float16_t1x3Matrix3'}} fxc-error {{X3000: unrecognized identifier 'float16_t3x2'}} */
    float16_t4x2 float16_t1x3Matrix4 = 3;                   /* fxc-error {{X3000: unrecognized identifier 'float16_t1x3Matrix4'}} fxc-error {{X3000: unrecognized identifier 'float16_t4x2'}} */
    float32_t1x3 float32_t1Matrix1 = 4;                     /* fxc-error {{X3000: unrecognized identifier 'float32_t1Matrix1'}} fxc-error {{X3000: unrecognized identifier 'float32_t1x3'}} */
    float32_t2x2 float32_t1Matrix2 = 5;                     /* fxc-error {{X3000: unrecognized identifier 'float32_t1Matrix2'}} fxc-error {{X3000: unrecognized identifier 'float32_t2x2'}} */
    float32_t3x4 float32_t1Matrix3 = 6;                     /* fxc-error {{X3000: unrecognized identifier 'float32_t1Matrix3'}} fxc-error {{X3000: unrecognized identifier 'float32_t3x4'}} */
    float32_t4x1 float32_t1Matrix4 = 7;                     /* fxc-error {{X3000: unrecognized identifier 'float32_t1Matrix4'}} fxc-error {{X3000: unrecognized identifier 'float32_t4x1'}} */
    float64_t1x2 float64_t1Matrix1 = 8;                     /* fxc-error {{X3000: unrecognized identifier 'float64_t1Matrix1'}} fxc-error {{X3000: unrecognized identifier 'float64_t1x2'}} */
    float64_t2x3 float64_t1Matrix2 = 9;                     /* fxc-error {{X3000: unrecognized identifier 'float64_t1Matrix2'}} fxc-error {{X3000: unrecognized identifier 'float64_t2x3'}} */
    float64_t3x3 float64_t1Matrix3 = 10;                    /* fxc-error {{X3000: unrecognized identifier 'float64_t1Matrix3'}} fxc-error {{X3000: unrecognized identifier 'float64_t3x3'}} */
    float64_t4x4 float64_t1Matrix4 = 11;                    /* fxc-error {{X3000: unrecognized identifier 'float64_t1Matrix4'}} fxc-error {{X3000: unrecognized identifier 'float64_t4x4'}} */
}

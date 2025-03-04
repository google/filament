// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main

// To test with the classic compiler, run
// fxc.exe /T vs_5_1 matrix-syntax.hlsl

matrix m;

void abs_without_using_result() {
    matrix<float, 4, 4> mymatrix;
    abs(mymatrix);

    matrix<float, 1, 4> mymatrix2;
    abs(mymatrix2);
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
        |-ImplicitCastExpr <col:19> 'matrix<float, 4, 4> (*)(const matrix<float, 4, 4> &)' <FunctionToPointerDecay>
        | `-DeclRefExpr <col:19> 'matrix<float, 4, 4> (const matrix<float, 4, 4> &)' lvalue Function 'abs' 'matrix<float, 4, 4> (const matrix<float, 4, 4> &)'
        `-DeclRefExpr <col:23> 'matrix<float, 4, 4>':'matrix<float, 4, 4>' lvalue Var 'mymatrix' 'matrix<float, 4, 4>':'matrix<float, 4, 4>'
    */
}

void matrix_on_demand() {
    float4x4 thematrix;
    float4x4 anotherMatrix;
    bool2x1 boolMatrix;   
}

void abs_on_demand() {
   float1x2 f12;
   float1x2 result = abs(f12);
}

void matrix_out_of_bounds() {
}

void main() {
    // Multiple assignments in a chain.
    matrix<float, 4, 4> mymatrix;
    matrix<float, 4, 4> absMatrix = abs(mymatrix);
    matrix<float, 4, 4> absMatrix2 = abs(absMatrix);

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
    f2 = mymatrix._m00_m01;
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

    // one-based positions.
    f = mymatrix._11;
    f2 = mymatrix._11_11;
    f4 = mymatrix._11_11_44_44;
    /*verify-ast
      BinaryOperator <col:5, col:19> 'float4':'vector<float, 4>' '='
      |-DeclRefExpr <col:5> 'float4':'vector<float, 4>' lvalue Var 'f4' 'float4':'vector<float, 4>'
      `-ExtMatrixElementExpr <col:10, col:19> 'vector<float, 4>':'vector<float, 4>' _11_11_44_44
        `-DeclRefExpr <col:10> 'matrix<float, 4, 4>':'matrix<float, 4, 4>' lvalue Var 'mymatrix' 'matrix<float, 4, 4>':'matrix<float, 4, 4>'
    */
}
// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main

// To test with the classic compiler, run
// fxc.exe /T vs_5_1 vector-syntax.hlsl

vector v;
vector<float, 1+1> v1p1;
/*verify-ast
  VarDecl <col:1, col:20> v1p1 'vector<float, 1 + 1>':'vector<float, 2>'
*/
static const int i = 1;
vector<float, i+i> vfi;
/*verify-ast
  VarDecl <col:1, col:20> vfi 'vector<float, i + i>':'vector<float, 2>'
*/
static const int2 g_i2 = {1,2};
/*verify-ast
  VarDecl <col:1, col:30> g_i2 'const int2':'const vector<int, 2>' static
  `-InitListExpr <col:26, col:30> 'const int2':'const vector<int, 2>'
    |-ImplicitCastExpr <col:27> 'int' <IntegralCast>
    | `-IntegerLiteral <col:27> 'literal int' 1
    `-ImplicitCastExpr <col:29> 'int' <IntegralCast>
      `-IntegerLiteral <col:29> 'literal int' 2
*/

// Clang front-end currently doesn't support non-basic types in constant expressions, such as below:
vector<float, 2> v_const_vec_expr2;  
vector<float, 3> v_const_vec_expr3;

void use_const_vec_expr() {
    float2 f2 = v_const_vec_expr2.xy;
    float3 f3 = v_const_vec_expr3.xyz;

    // These should fail (but don't in clang, since it couldn't construct the original vectors)
}

void abs_without_using_result() {
    vector<float, 4> myvector;
    abs(myvector);

    vector<float, 4> myvector2;
    abs(myvector2);
}

void abs_with_assignment() {
    vector<float, 4> myvector;
    vector<float, 4> absvector;
    absvector = abs(myvector);
}

vector<float, 4> abs_for_result(vector<float, 4> value) {
    return abs(value);
}

void fn_use_vector(vector<float, 4> value) { }

void abs_in_argument() {
    vector<float, 4> myvector;
    fn_use_vector(abs(myvector));
    /*verify-ast
      CallExpr <col:5, col:32> 'void'
      |-ImplicitCastExpr <col:5> 'void (*)(vector<float, 4>)' <FunctionToPointerDecay>
      | `-DeclRefExpr <col:5> 'void (vector<float, 4>)' lvalue Function 'fn_use_vector' 'void (vector<float, 4>)'
      `-CallExpr <col:19, col:31> 'vector<float, 4>':'vector<float, 4>'
        |-ImplicitCastExpr <col:19> 'vector<float, 4> (*)(const vector<float, 4> &)' <FunctionToPointerDecay>
        | `-DeclRefExpr <col:19> 'vector<float, 4> (const vector<float, 4> &)' lvalue Function 'abs' 'vector<float, 4> (const vector<float, 4> &)'
        `-DeclRefExpr <col:23> 'vector<float, 4>':'vector<float, 4>' lvalue Var 'myvector' 'vector<float, 4>':'vector<float, 4>'
    */
}

void vector_on_demand() {
    float4 thevector;
    float4 anothervector;
    bool2 boolvector;
}

void abs_on_demand() {
   float2 f2;
   float2 result = abs(f2);
}

void vector_out_of_bounds() {
}

float fn() {
    float4 myvar = float4(1,2,3,4);
    myvar.x = 1.0f;
    myvar.y = 1.0f;
    myvar.z = 1.0f;
    myvar.w = 1.0f;

    float4 myothervar;
    myothervar.rgba = myvar.xyzw;
    /*verify-ast
      BinaryOperator <col:5, col:29> 'vector<float, 4>':'vector<float, 4>' '='
      |-HLSLVectorElementExpr <col:5, col:16> 'vector<float, 4>':'vector<float, 4>' lvalue vectorcomponent rgba
      | `-DeclRefExpr <col:5> 'float4':'vector<float, 4>' lvalue Var 'myothervar' 'float4':'vector<float, 4>'
      `-ImplicitCastExpr <col:23, col:29> 'vector<float, 4>':'vector<float, 4>' <LValueToRValue>
        `-HLSLVectorElementExpr <col:23, col:29> 'vector<float, 4>':'vector<float, 4>' lvalue vectorcomponent xyzw
          `-DeclRefExpr <col:23> 'float4':'vector<float, 4>' lvalue Var 'myvar' 'float4':'vector<float, 4>'
    */

    float f;
    f.x = 1;

    uint u;
    u = f.x;

    uint3 u3;
    u3.xyz = f.xxx;
    /*verify-ast
      BinaryOperator <col:5, col:16> 'vector<uint, 3>':'vector<uint, 3>' '='
      |-HLSLVectorElementExpr <col:5, col:8> 'vector<uint, 3>':'vector<uint, 3>' lvalue vectorcomponent xyz
      | `-DeclRefExpr <col:5> 'uint3':'vector<uint, 3>' lvalue Var 'u3' 'uint3':'vector<uint, 3>'
      `-ImplicitCastExpr <col:14, col:16> 'vector<uint, 3>' <HLSLCC_FloatingToIntegral>
        `-HLSLVectorElementExpr <col:14, col:16> 'vector<float, 3>':'vector<float, 3>' xxx
          `-ImplicitCastExpr <col:14> 'vector<float, 1>':'vector<float, 1>' lvalue <HLSLVectorSplat>
            `-DeclRefExpr <col:14> 'float' lvalue Var 'f' 'float'
    */
    u3 = (!u3).zyx;
    /*verify-ast
      BinaryOperator <col:5, col:16> 'uint3':'vector<uint, 3>' '='
      |-DeclRefExpr <col:5> 'uint3':'vector<uint, 3>' lvalue Var 'u3' 'uint3':'vector<uint, 3>'
      `-HLSLVectorElementExpr <col:10, col:16> 'vector<bool, 3>':'vector<bool, 3>' zyx
        `-ParenExpr <col:10, col:14> 'vector<bool, 3>':'vector<bool, 3>'
          `-UnaryOperator <col:11, col:12> 'vector<bool, 3>':'vector<bool, 3>' prefix '!'
            `-ImplicitCastExpr <col:12> 'vector<bool, 3>' <HLSLCC_IntegralToBoolean>
              `-ImplicitCastExpr <col:12> 'uint3':'vector<uint, 3>' <LValueToRValue>
                `-DeclRefExpr <col:12> 'uint3':'vector<uint, 3>' lvalue Var 'u3' 'uint3':'vector<uint, 3>'
    */

    return f.x;
}

float4 main(float4 param4 : FOO): SV_Target {
  return fn();
}
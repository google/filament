// RUN: %dxc -Tlib_6_3 -enable-16bit-types -verify -HV 2018 %s

float3 f3_ones = 1.0.xxx;
float3 f3_ones_exp = 2.0e+2.rrr;

vector v;
vector<float, 1+1> v1p1;
/*verify-ast
  VarDecl <col:1, col:20> col:20 v1p1 'vector<float, 1 + 1>':'vector<float, 2>'
*/
static const int i = 1;
vector<float, i+i> vfi;
/*verify-ast
  VarDecl <col:1, col:20> col:20 vfi 'vector<float, i + i>':'vector<float, 2>'
*/
static const int2 g_i2 = {1,2};
/*verify-ast
  VarDecl <col:1, col:30> col:19 used g_i2 'const int2':'const vector<int, 2>' static cinit
  `-InitListExpr <col:26, col:30> 'const int2':'const vector<int, 2>'
    |-ImplicitCastExpr <col:27> 'int' <IntegralCast>
    | `-IntegerLiteral <col:27> 'literal int' 1
    `-ImplicitCastExpr <col:29> 'int' <IntegralCast>
      `-IntegerLiteral <col:29> 'literal int' 2
*/

// Clang front-end currently doesn't support non-basic types in constant expressions, such as below:
vector<float, g_i2.y> v_const_vec_expr2;                    /* expected-error {{non-type template argument of type 'int' is not an integral constant expression}} fxc-pass {{}} */
vector<float, g_i2.x + g_i2.y> v_const_vec_expr3;           /* expected-error {{non-type template argument of type 'int' is not an integral constant expression}} fxc-pass {{}} */

void use_const_vec_expr() {
    float2 f2 = v_const_vec_expr2.xy;
    float3 f3 = v_const_vec_expr3.xyz;

    // These should fail (but don't in clang, since it couldn't construct the original vectors)
    float f_error2 = v_const_vec_expr2.z;                   /* fxc-error {{X3018: invalid subscript 'z'}} */
    float f_error3 = v_const_vec_expr3.w;                   /* fxc-error {{X3018: invalid subscript 'w'}} */
}

void abs_without_using_result() {
    vector<float, 4> myvector;
    abs(myvector);                  /* expected-warning {{ignoring return value of function declared with const attribute}} fxc-pass {{}} */

    vector<float, 4> myvector2;
    abs(myvector2);                 /* expected-warning {{ignoring return value of function declared with const attribute}} fxc-pass {{}} */
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
        |-ImplicitCastExpr <col:19> 'vector<float, 4> (*)(vector<float, 4>)' <FunctionToPointerDecay>
        | `-DeclRefExpr <col:19> 'vector<float, 4> (vector<float, 4>)' lvalue Function 'abs' 'vector<float, 4> (vector<float, 4>)'
        `-ImplicitCastExpr <col:23> 'vector<float, 4>':'vector<float, 4>' <LValueToRValue>
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
  vector<float, 8> vector_oob_0; // expected-error {{invalid value, valid range is between 1 and 4 inclusive}} fxc-error {{X3052: vector dimension must be between 1 and 4}}
  vector<float, 0> vector_oob_1; // expected-error {{invalid value, valid range is between 1 and 4 inclusive}} fxc-error {{X3052: vector dimension must be between 1 and 4}}
  vector<float, -1> vector_oob_2; // expected-error {{invalid value, valid range is between 1 and 4 inclusive}} fxc-error {{X3052: vector dimension must be between 1 and 4}}
}

void vector_unsigned() {
   unsigned int4 intvector;
   unsigned min16int4 min16vector;                          /* expected-warning {{'min16uint' is promoted to 'uint16_t'}} fxc-error {{X3085: unsigned can not be used with type}} */
   unsigned int64_t3 int64vector;                           /* fxc-error {{X3000: syntax error: unexpected token 'int64_t3'}} */
   unsigned uint3 uintvector;                               /* fxc-error {{X3085: unsigned can not be used with type}} */
   unsigned min16uint4 min16uintvector;                     /* expected-warning {{'min16uint' is promoted to 'uint16_t'}} fxc-error {{X3085: unsigned can not be used with type}} */
   unsigned uint64_t2 int64uintvector;                      /* fxc-error {{X3000: syntax error: unexpected token 'uint64_t2'}} */
   unsigned dword3 dwordvector; /* fxc-error {{X3000: syntax error: unexpected token 'dword3'}} */

   unsigned float2 floatvector; /* expected-error {{'float' cannot be signed or unsigned}} fxc-error {{X3085: unsigned can not be used with type}} */
   unsigned bool3 boolvector;   /* expected-error {{'bool' cannot be signed or unsigned}} fxc-error {{X3085: unsigned can not be used with type}} */
   unsigned half4 halfvector;   /* expected-error {{'half' cannot be signed or unsigned}} fxc-error {{X3085: unsigned can not be used with type}} */
   unsigned double1 doublevector;                           /* expected-error {{'double' cannot be signed or unsigned}} fxc-error {{X3085: unsigned can not be used with type}} */
   unsigned min12int2 min12intvector;                       /* expected-error {{'min12int' cannot be signed or unsigned}} expected-warning {{'min12int' is promoted to 'int16_t'}} fxc-error {{X3085: unsigned can not be used with type}} */
   unsigned min16float3 min16floatvector;                   /* expected-error {{'min16float' cannot be signed or unsigned}} expected-warning {{'min16float' is promoted to 'half'}} fxc-error {{X3085: unsigned can not be used with type}} */

   unsigned int16_t1 int16_tvector;                         /* fxc-error {{X3000: syntax error: unexpected token 'int16_t1'}} */
   unsigned int32_t1 int32_tvector;                         /* fxc-error {{X3000: syntax error: unexpected token 'int32_t1'}} */
   unsigned int64_t1 int64_tvector;                         /* fxc-error {{X3000: syntax error: unexpected token 'int64_t1'}} */

}

float fn() {
    float4 myvar = float4(1,2,3,4);
    myvar.x = 1.0f;
    myvar.y = 1.0f;
    myvar.z = 1.0f;
    myvar.w = 1.0f;
    myvar[0] = 1.0f;
    myvar[1]= 1.0f;
    myvar[2] = 1.0f;
    myvar[3] = 1.0f;
    myvar[-10] = 1.0f;            /* expected-error {{vector element index '-10' is out of bounds}} fxc-pass {{}} */
    myvar[4] = 1.0f;              /* expected-error {{vector element index '4' is out of bounds}} fxc-pass {{}} */

    float3 myvar3 = float3(1,2,3);
    myvar3[3] = 1.0f;             /* expected-error {{vector element index '3' is out of bounds}} fxc-pass {{}} */

    float2 myvar2 = float2(1,2);
    myvar2[2] = 1.0f;             /* expected-error {{vector element index '2' is out of bounds}} fxc-pass {{}} */

    float1 myvar1 = float1(1);
    myvar1[1] = 1.0f;             /* expected-error {{vector element index '1' is out of bounds}} fxc-pass {{}} */

    const float4 constMyVar = float4(1,2,3,4);              /* expected-note {{variable 'constMyVar' declared const here}} expected-note {{variable 'constMyVar' declared const here}} fxc-pass {{}} */
    constMyVar = float4(1,1,1,1); /* expected-error {{cannot assign to variable 'constMyVar' with const-qualified type 'const float4'}} fxc-error {{X3025: l-value specifies const object}} */
    constMyVar[0] = 2.0f;         /* expected-error {{cannot assign to variable 'constMyVar' with const-qualified type 'const float4'}} fxc-error {{X3025: l-value specifies const object}} */

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
      BinaryOperator <col:5, col:16> 'vector<uint, 3>':'vector<unsigned int, 3>' '='
      |-HLSLVectorElementExpr <col:5, col:8> 'vector<uint, 3>':'vector<unsigned int, 3>' lvalue vectorcomponent xyz
      | `-DeclRefExpr <col:5> 'uint3':'vector<unsigned int, 3>' lvalue Var 'u3' 'uint3':'vector<unsigned int, 3>'
      `-ImplicitCastExpr <col:14, col:16> 'vector<unsigned int, 3>' <HLSLCC_FloatingToIntegral>
        `-HLSLVectorElementExpr <col:14, col:16> 'vector<float, 3>':'vector<float, 3>' xxx
          `-ImplicitCastExpr <col:14> 'vector<float, 1>':'vector<float, 1>' lvalue <HLSLVectorSplat>
            `-DeclRefExpr <col:14> 'float' lvalue Var 'f' 'float'
    */
    u3 = (!u3).zyx;
    /*verify-ast
      BinaryOperator <col:5, col:16> 'uint3':'vector<unsigned int, 3>' '='
      |-DeclRefExpr <col:5> 'uint3':'vector<unsigned int, 3>' lvalue Var 'u3' 'uint3':'vector<unsigned int, 3>'
      `-ImplicitCastExpr <col:10, col:16> 'vector<unsigned int, 3>' <HLSLCC_IntegralCast>
        `-HLSLVectorElementExpr <col:10, col:16> 'vector<bool, 3>':'vector<bool, 3>' zyx
          `-ParenExpr <col:10, col:14> 'vector<bool, 3>':'vector<bool, 3>'
            `-UnaryOperator <col:11, col:12> 'vector<bool, 3>':'vector<bool, 3>' prefix '!'
              `-ImplicitCastExpr <col:12> 'vector<bool, 3>' <HLSLCC_IntegralToBoolean>
                `-ImplicitCastExpr <col:12> 'uint3':'vector<unsigned int, 3>' <LValueToRValue>
                  `-DeclRefExpr <col:12> 'uint3':'vector<unsigned int, 3>' lvalue Var 'u3' 'uint3':'vector<unsigned int, 3>'
    */
    f.xx = 2; // expected-error {{vector is not assignable (contains duplicate components)}} fxc-error {{X3025: l-value specifies const object}}
    u3.x = u3.w;                                            /* expected-error {{vector swizzle 'w' is out of bounds}} fxc-error {{X3018: invalid subscript 'w'}} */


    // vector for fixed width data types
    int16_t1 int16_t1Vector1 = 0;                           /* fxc-error {{X3000: unrecognized identifier 'int16_t1'}} fxc-error {{X3000: unrecognized identifier 'int16_t1Vector1'}} */
    int16_t2 int16_t1Vector2 = 1;                           /* fxc-error {{X3000: unrecognized identifier 'int16_t1Vector2'}} fxc-error {{X3000: unrecognized identifier 'int16_t2'}} */
    int16_t3 int16_t1Vector3 = 2;                           /* fxc-error {{X3000: unrecognized identifier 'int16_t1Vector3'}} fxc-error {{X3000: unrecognized identifier 'int16_t3'}} */
    int16_t4 int16_t1Vector4 = 3;                           /* fxc-error {{X3000: unrecognized identifier 'int16_t1Vector4'}} fxc-error {{X3000: unrecognized identifier 'int16_t4'}} */
    int32_t1 int32_t1Vector1 = 4;                           /* fxc-error {{X3000: unrecognized identifier 'int32_t1'}} fxc-error {{X3000: unrecognized identifier 'int32_t1Vector1'}} */
    int32_t2 int32_t1Vector2 = 5;                           /* fxc-error {{X3000: unrecognized identifier 'int32_t1Vector2'}} fxc-error {{X3000: unrecognized identifier 'int32_t2'}} */
    int32_t3 int32_t1Vector3 = 6;                           /* fxc-error {{X3000: unrecognized identifier 'int32_t1Vector3'}} fxc-error {{X3000: unrecognized identifier 'int32_t3'}} */
    int32_t4 int32_t1Vector4 = 7;                           /* fxc-error {{X3000: unrecognized identifier 'int32_t1Vector4'}} fxc-error {{X3000: unrecognized identifier 'int32_t4'}} */
    int64_t1 int64_t1Vector1 = 8;                           /* fxc-error {{X3000: unrecognized identifier 'int64_t1'}} fxc-error {{X3000: unrecognized identifier 'int64_t1Vector1'}} */
    int64_t2 int64_t1Vector2 = 9;                           /* fxc-error {{X3000: unrecognized identifier 'int64_t1Vector2'}} fxc-error {{X3000: unrecognized identifier 'int64_t2'}} */
    int64_t3 int64_t1Vector3 = 10;                          /* fxc-error {{X3000: unrecognized identifier 'int64_t1Vector3'}} fxc-error {{X3000: unrecognized identifier 'int64_t3'}} */
    int64_t4 int64_t1Vector4 = 11;                          /* fxc-error {{X3000: unrecognized identifier 'int64_t1Vector4'}} fxc-error {{X3000: unrecognized identifier 'int64_t4'}} */

    uint16_t1 uint16_t1Vector1 = 0;                         /* fxc-error {{X3000: unrecognized identifier 'uint16_t1'}} fxc-error {{X3000: unrecognized identifier 'uint16_t1Vector1'}} */
    uint16_t2 uint16_t1Vector2 = 1;                         /* fxc-error {{X3000: unrecognized identifier 'uint16_t1Vector2'}} fxc-error {{X3000: unrecognized identifier 'uint16_t2'}} */
    uint16_t3 uint16_t1Vector3 = 2;                         /* fxc-error {{X3000: unrecognized identifier 'uint16_t1Vector3'}} fxc-error {{X3000: unrecognized identifier 'uint16_t3'}} */
    uint16_t4 uint16_t1Vector4 = 3;                         /* fxc-error {{X3000: unrecognized identifier 'uint16_t1Vector4'}} fxc-error {{X3000: unrecognized identifier 'uint16_t4'}} */
    uint32_t1 uint32_t1Vector1 = 4;                         /* fxc-error {{X3000: unrecognized identifier 'uint32_t1'}} fxc-error {{X3000: unrecognized identifier 'uint32_t1Vector1'}} */
    uint32_t2 uint32_t1Vector2 = 5;                         /* fxc-error {{X3000: unrecognized identifier 'uint32_t1Vector2'}} fxc-error {{X3000: unrecognized identifier 'uint32_t2'}} */
    uint32_t3 uint32_t1Vector3 = 6;                         /* fxc-error {{X3000: unrecognized identifier 'uint32_t1Vector3'}} fxc-error {{X3000: unrecognized identifier 'uint32_t3'}} */
    uint32_t4 uint32_t1Vector4 = 7;                         /* fxc-error {{X3000: unrecognized identifier 'uint32_t1Vector4'}} fxc-error {{X3000: unrecognized identifier 'uint32_t4'}} */
    uint64_t1 uint64_t1Vector1 = 8;                         /* fxc-error {{X3000: unrecognized identifier 'uint64_t1'}} fxc-error {{X3000: unrecognized identifier 'uint64_t1Vector1'}} */
    uint64_t2 uint64_t1Vector2 = 9;                         /* fxc-error {{X3000: unrecognized identifier 'uint64_t1Vector2'}} fxc-error {{X3000: unrecognized identifier 'uint64_t2'}} */
    uint64_t3 uint64_t1Vector3 = 10;                        /* fxc-error {{X3000: unrecognized identifier 'uint64_t1Vector3'}} fxc-error {{X3000: unrecognized identifier 'uint64_t3'}} */
    uint64_t4 uint64_t1Vector4 = 11;                        /* fxc-error {{X3000: unrecognized identifier 'uint64_t1Vector4'}} fxc-error {{X3000: unrecognized identifier 'uint64_t4'}} */

    float16_t1 float16_t1Vector1 = 1;                       /* fxc-error {{X3000: unrecognized identifier 'float16_t1'}} fxc-error {{X3000: unrecognized identifier 'float16_t1Vector1'}} */
    float16_t2 float16_t1Vector2 = 2;                       /* fxc-error {{X3000: unrecognized identifier 'float16_t1Vector2'}} fxc-error {{X3000: unrecognized identifier 'float16_t2'}} */
    float16_t3 float16_t1Vector3 = 3;                       /* fxc-error {{X3000: unrecognized identifier 'float16_t1Vector3'}} fxc-error {{X3000: unrecognized identifier 'float16_t3'}} */
    float16_t4 float16_t1Vector4 = 4;                       /* fxc-error {{X3000: unrecognized identifier 'float16_t1Vector4'}} fxc-error {{X3000: unrecognized identifier 'float16_t4'}} */
    float32_t1 float32_t1Vector1 = 5;                       /* fxc-error {{X3000: unrecognized identifier 'float32_t1'}} fxc-error {{X3000: unrecognized identifier 'float32_t1Vector1'}} */
    float32_t2 float32_t1Vector2 = 6;                       /* fxc-error {{X3000: unrecognized identifier 'float32_t1Vector2'}} fxc-error {{X3000: unrecognized identifier 'float32_t2'}} */
    float32_t3 float32_t1Vector3 = 7;                       /* fxc-error {{X3000: unrecognized identifier 'float32_t1Vector3'}} fxc-error {{X3000: unrecognized identifier 'float32_t3'}} */
    float32_t4 float32_t1Vector4 = 8;                       /* fxc-error {{X3000: unrecognized identifier 'float32_t1Vector4'}} fxc-error {{X3000: unrecognized identifier 'float32_t4'}} */
    float64_t1 float64_t1Vector1 = 9;                       /* fxc-error {{X3000: unrecognized identifier 'float64_t1'}} fxc-error {{X3000: unrecognized identifier 'float64_t1Vector1'}} */
    float64_t2 float64_t1Vector2 = 10;                      /* fxc-error {{X3000: unrecognized identifier 'float64_t1Vector2'}} fxc-error {{X3000: unrecognized identifier 'float64_t2'}} */
    float64_t3 float64_t1Vector3 = 11;                      /* fxc-error {{X3000: unrecognized identifier 'float64_t1Vector3'}} fxc-error {{X3000: unrecognized identifier 'float64_t3'}} */
    float64_t4 float64_t1Vector4 = 12;                      /* fxc-error {{X3000: unrecognized identifier 'float64_t1Vector4'}} fxc-error {{X3000: unrecognized identifier 'float64_t4'}} */

    return f.x;
}

// float4 main(float4 param4 : FOO) : FOO {
float4 plain(float4 param4 /* : FOO */) /*: FOO */{
  return fn();
}

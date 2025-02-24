// RUN: %dxc -Tlib_6_3  -Wno-unused-value  -verify %s
// RUN: %dxc -Tps_6_0  -Wno-unused-value  -verify %s

// This file includes operator tests that target specific cases that are not
// otherwise covered by the other generated files.

// we use -Wno-unused-value because we generate some no-op expressions to yield errors
// without also putting them in a static assertion

#define OVERLOAD_IMPLEMENTED

// __decltype is the GCC way of saying 'decltype', but doesn't require C++11
// _Static_assert is the C11 way of saying 'static_assert', but doesn't require C++11
#ifdef VERIFY_FXC
#define _Static_assert(a,b,c) ;
#endif

// To test with the classic compiler, run
// %sdxroot%\tools\x86\fxc.exe /T vs_5_1 more-operators.hlsl
// with vs_2_0 (the default) min16float usage produces a complaint that it's not supported

struct f3_s    { float3 f3; };
struct mixed_s { float3 f3; uint s; };
struct many_s  { float4 f4; double4 d4; };
struct u3_s    { uint u3; };

SamplerState  g_SamplerState;
f3_s          g_f3_s;
mixed_s       g_mixed_s;
many_s        g_many_s;
u3_s          g_u3_s;

// Until initialization lists are complete, this is the easiest way to initialize these.
int1x1 g_i11;
int1x2 g_i12;
int2x1 g_i21;
int2x2 g_i22;

float  int_to_float(int i)    { return i; }
int1x1 float_to_i11(float f)  { return f; }
float  i11_to_float(int1x1 v) { return v; }

void into_out_i(out int i) { i = g_i11; }
void into_out_i3(out int3 i3) { i3 = int3(1, 2, 3); } // expected-note {{candidate function}} expected-note {{passing argument to parameter 'i3' here}} fxc-pass {{}}
void into_out_f(out float i) { i = g_i11; }
void into_out_f3_s(out f3_s i) { }
void into_out_ss(out SamplerState ss) { ss = g_SamplerState; }

float4 plain(float4 param4 /* : FOO */) /*: FOO */{
    bool bools = 0;
    int ints = 0;
    const int intc = 1;                               /* expected-note {{variable 'intc' declared const here}} fxc-pass {{}} */
    int1 i1 = 0;
    int2 i2 = { 0, 0 };
    int3 i3 = { 1, 2, 3 };
    int4 i4 = { 1, 2, 3, 4 };
    int1x1 i11 = g_i11;
    int1x2 i12 = g_i12;
    int2x1 i21 = g_i21;
    int2x2 i22 = g_i22;
    int ari1[1] = { 0 };
    int ari2[2] = { 0, 1 };
    float floats = 0;
    SamplerState SamplerStates = g_SamplerState;
    f3_s f3_ss = g_f3_s;
    mixed_s mixed_ss = g_mixed_s;

    _Static_assert(std::is_same<int, int>::value, "");
    _Static_assert(!std::is_same<int, float>::value, "");

    // Tests for complex objects.
    f3_ss = mixed_ss; // expected-error {{cannot implicitly convert from 'mixed_s' to 'f3_s'}} fxc-error {{X3017: cannot convert from 'struct mixed_s' to 'struct f3_s'}}
    _Static_assert(std::is_same<f3_s, __decltype(f3_ss = (f3_s)mixed_ss)>::value, "");
    mixed_ss = f3_ss; // expected-error {{cannot convert from 'f3_s' to 'mixed_s'}} fxc-error {{X3017: cannot implicitly convert from 'struct f3_s' to 'struct mixed_s'}}
    mixed_ss = (mixed_s)f3_ss; // expected-error {{cannot convert from 'f3_s' to 'mixed_s'}} fxc-error {{X3017: cannot convert from 'struct f3_s' to 'struct mixed_s'}}

    // Tests with vectors of one.
    ints = i1; // assign to scalar
    i2 = i1;   // assign to wider type
    i1 = ints; // assign from scalar
    i12 = i1;  // assign to matrix type (one row)
    i21 = i1;  // assign to matrix type (one col)
    i22 = i1;  // assign to matrix type (square)
    floats = i1; // assign to scalar of different but compatible type
    floats = i11; // assign to scalar of different but compatible type
    i11 = floats; // assign from scalar of different but compatible type

    // Tests with 1x1 matrices.
    ints = i11;         // assign to scalar
    i22 = i11;          // assign to wider type
    i22 = i21;          // expected-error {{cannot convert from 'int2x1' to 'int2x2'}} fxc-error {{X3017: cannot implicitly convert from 'int2x1' to 'int2x2'}}
    i22 = (int2x2)i21;  // expected-error {{cannot convert from 'int2x1' to 'int2x2'}} fxc-error {{X3017: cannot convert from 'int2x1' to 'int2x2'}}

    // Tests with arrays of one and one-dimensional arrays.
    ints = ari1; // expected-error {{cannot implicitly convert from 'int [1]' to 'int'}} fxc-error {{X3017: cannot convert from 'int[1]' to 'int'}}
    i2 = ari1;   // expected-error {{cannot convert from 'int [1]' to 'int2'}} fxc-error {{X3017: cannot implicitly convert from 'int[1]' to 'int2'}}
    i2 = (int2)ari1;   // expected-error {{cannot convert from 'int [1]' to 'int2'}} fxc-error {{X3017: cannot convert from 'int[1]' to 'int2'}}
    ari1 = ints; // expected-error {{cannot implicitly convert from 'int' to 'int [1]'}} fxc-error {{X3017: cannot convert from 'int' to 'int[1]'}}
    ari1 = (int[1])ints; // explicit conversion works
    ari1 = ari1; // assign to same-sized array
    ari2 = ari1; // expected-error {{cannot convert from 'int [1]' to 'int [2]'}} fxc-error {{X3017: cannot implicitly convert from 'int[1]' to 'int[2]'}}
    ari2 = (int[2])ari1; // expected-error {{cannot convert from 'int [1]' to 'int [2]'}} fxc-error {{X3017: cannot convert from 'int[1]' to 'int[2]'}}
    ari1 = ari2; // expected-error {{cannot implicitly convert from 'int [2]' to 'int [1]'}} fxc-error {{X3017: cannot convert from 'int[2]' to 'int[1]'}}
    ari1 = (int[1])ari2; // explicit conversion to smaller size
    i12 = ari1;  // expected-error {{cannot convert from 'int [1]' to 'int1x2'}} fxc-error {{X3017: cannot implicitly convert from 'int[1]' to 'int2'}}
    i21 = ari1;  // expected-error {{cannot convert from 'int [1]' to 'int2x1'}} fxc-error {{X3017: cannot implicitly convert from 'int[1]' to 'int2x1'}}
    i22 = ari1;  // expected-error {{cannot convert from 'int [1]' to 'int2x2'}} fxc-error {{X3017: cannot implicitly convert from 'int[1]' to 'int2x2'}}
    floats = ari1; // expected-error {{cannot implicitly convert from 'int [1]' to 'float'}} fxc-error {{X3017: cannot convert from 'int[1]' to 'float'}}
    floats = (float)ari1; // assign to scalar of compatible type
    ari1 = ari1 + ari1; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
    ari1 = ints + ari1; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}

    // Tests that introduce const into expression.
    intc = ints; // expected-error {{cannot assign to variable 'intc' with const-qualified type 'const int'}} fxc-error {{X3025: l-value specifies const object}}
    ints = intc;
    i2 = intc;

    // Tests that perform a conversion by virtue of making a function call.
    // Same rules, mostly ensuring that the same codepath is taken.
    floats = int_to_float(ints);
    ints = int_to_float(floats);

    // Tests that perform a conversion by virtue of returning a type.
    // Same rules, mostly ensuring that the same codepath is taken.
    floats = int_to_float(ints);
    ints = int_to_float(floats);
    i11 = float_to_i11(floats);
    floats = i11_to_float(i11);

    // Tests that perform equality checks on object types. (not supported in prior compiler)
    bools = g_SamplerState == g_SamplerState; // expected-error {{operator cannot be used with built-in type 'SamplerState'}} fxc-pass {{}}
    bools = g_SamplerState != g_SamplerState; // expected-error {{operator cannot be used with built-in type 'SamplerState'}} fxc-pass {{}}
    bools = g_SamplerState < g_SamplerState; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}

    // Tests that perform the conversion into an out parameter.
    into_out_i(ints);
    into_out_f(floats);
    into_out_f3_s(f3_ss);
    into_out_ss(SamplerStates);
    // fxc error X3017: 'into_out_i3': cannot implicitly convert from 'int2' to 'int3'
    into_out_i3(i2); // expected-error {{cannot initialize a parameter of type 'int3 &' with an lvalue of type 'int2'}} fxc-error {{X3017: 'into_out_i3': cannot convert output parameter from 'int3' to 'int2'}}
    // fxc error X3017: cannot convert from 'int2' to 'int3'
    into_out_i3((int3)i2); // expected-error {{cannot convert from 'int2' to 'int3'}} fxc-error {{X3013: 'into_out_i3': no matching 1 parameter function}} fxc-error {{X3017: cannot convert from 'int2' to 'int3'}}
    into_out_i3(i4); // expected-error {{no matching function for call to 'into_out_i3'}} fxc-error {{X3017: 'into_out_i3': cannot implicitly convert output parameter from 'int3' to 'int4'}}
    into_out_i3((int3)i4);

    // Tests that perform casts that yield lvalues.
    ((int3)i4).x = 1;
    // fxc error X4555: cannot use casts on l-values
    ((float4)i4).x = 1; // expected-error {{expression is not assignable}} fxc-pass {{}}

    // Tests that work with unary operators.
    bool bool_l = 1;
    int int_l = 1;
    uint uint_l = 1;
    half half_l = 1;
    float float_l = 1;
    double double_l = 1;
    min16float min16float_l = 1;
    min10float min10float_l = 1;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
    min16int min16int_l = 1;
    min12int min12int_l = 1;        // expected-warning {{'min12int' is promoted to 'min16int'}} fxc-pass {{}}
    min16uint min16uint_l = 1;
    SamplerState SamplerState_l = g_SamplerState;
    bool1 bool1_l = 0;
    bool2 bool2_l = 0;
    float3 float3_l = 0;
    double4 double4_l = 0;
    min10float1x2 min10float1x2_l = 0;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
    uint2x3 uint2x3_l = 0;
    min16uint4x4 min16uint4x4_l = 0;
    int3x2 int3x2_l = 0;
    f3_s f3_s_l = g_f3_s;
    mixed_s mixed_s_l = g_mixed_s;

    // An artifact of the code generation is that some results were classified
    // as int or bool2 when int1 and int2 were more accurate (but the overload was missing
    // in the test).
    //
    // Some 'int or unsigned int type require' messages are 'scalar, vector, or matrix expected'
    // when both might apply.
    //
    // pre/post inc/dec on unsupported types was silently ignored and resulted in
    // 'get_value': no matching 1 parameter function errors, which are now
    // 'scalar, vector, or matrix expected'.

    _Static_assert(std::is_same<int, __decltype(-bool_l)>::value, "");
    _Static_assert(std::is_same<int, __decltype(-int_l)>::value, "");
    _Static_assert(std::is_same<uint, __decltype(-uint_l)>::value, "");
    _Static_assert(std::is_same<half, __decltype(-half_l)>::value, "");
    _Static_assert(std::is_same<float, __decltype(-float_l)>::value, "");
    _Static_assert(std::is_same<double, __decltype(-double_l)>::value, "");
    _Static_assert(std::is_same<min16float, __decltype(-min16float_l)>::value, "");
    _Static_assert(std::is_same<min10float, __decltype(-min10float_l)>::value, "");  // fxc-pass {{}}
    _Static_assert(std::is_same<min16int, __decltype(-min16int_l)>::value, "");
    _Static_assert(std::is_same<min12int, __decltype(-min12int_l)>::value, "");    // fxc-pass {{}}
    _Static_assert(std::is_same<min16uint, __decltype(-min16uint_l)>::value, "");
    (-SamplerState_l); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
    (-f3_s_l); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
    (-mixed_s_l); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
    _Static_assert(std::is_same<int1, __decltype(-bool1_l)>::value, "");
    _Static_assert(std::is_same<int2, __decltype(-bool2_l)>::value, "");
    _Static_assert(std::is_same<float3, __decltype(-float3_l)>::value, "");
    _Static_assert(std::is_same<double4, __decltype(-double4_l)>::value, "");
    _Static_assert(std::is_same<min10float1x2, __decltype(-min10float1x2_l)>::value, "");      /* fxc-pass {{}} */
    _Static_assert(std::is_same<uint2x3, __decltype(-uint2x3_l)>::value, "");
    _Static_assert(std::is_same<min16uint4x4, __decltype(-min16uint4x4_l)>::value, "");
    _Static_assert(std::is_same<int3x2, __decltype(-int3x2_l)>::value, "");
    _Static_assert(std::is_same<int, __decltype(+bool_l)>::value, "");
    _Static_assert(std::is_same<int, __decltype(+int_l)>::value, "");
    _Static_assert(std::is_same<uint, __decltype(+uint_l)>::value, "");
    _Static_assert(std::is_same<half, __decltype(+half_l)>::value, "");
    _Static_assert(std::is_same<float, __decltype(+float_l)>::value, "");
    _Static_assert(std::is_same<double, __decltype(+double_l)>::value, "");
    _Static_assert(std::is_same<min16float, __decltype(+min16float_l)>::value, "");
    _Static_assert(std::is_same<min10float, __decltype(+min10float_l)>::value, "");  // fxc-pass {{}}
    _Static_assert(std::is_same<min16int, __decltype(+min16int_l)>::value, "");
    _Static_assert(std::is_same<min12int, __decltype(+min12int_l)>::value, "");    // fxc-pass {{}}
    _Static_assert(std::is_same<min16uint, __decltype(+min16uint_l)>::value, "");
    (+SamplerState_l); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
    (+f3_s_l); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
    (+mixed_s_l); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
    _Static_assert(std::is_same<int1, __decltype(+bool1_l)>::value, "");
    _Static_assert(std::is_same<int2, __decltype(+bool2_l)>::value, "");
    _Static_assert(std::is_same<float3, __decltype(+float3_l)>::value, "");
    _Static_assert(std::is_same<double4, __decltype(+double4_l)>::value, "");
    _Static_assert(std::is_same<min10float1x2, __decltype(+min10float1x2_l)>::value, "");      /* fxc-pass {{}} */
    _Static_assert(std::is_same<uint2x3, __decltype(+uint2x3_l)>::value, "");
    _Static_assert(std::is_same<min16uint4x4, __decltype(+min16uint4x4_l)>::value, "");
    _Static_assert(std::is_same<int3x2, __decltype(+int3x2_l)>::value, "");
    _Static_assert(std::is_same<int, __decltype(~bool_l)>::value, "");
    _Static_assert(std::is_same<int, __decltype(~int_l)>::value, "");
    _Static_assert(std::is_same<uint, __decltype(~uint_l)>::value, "");
    (~half_l); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
    (~float_l); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
    (~double_l); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
    (~min16float_l); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
    (~min10float_l); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
    _Static_assert(std::is_same<min16int, __decltype(~min16int_l)>::value, "");
    _Static_assert(std::is_same<min12int, __decltype(~min12int_l)>::value, "");    // fxc-pass {{}}
    _Static_assert(std::is_same<min16uint, __decltype(~min16uint_l)>::value, "");
    (~SamplerState_l); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
    (~f3_s_l); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
    (~mixed_s_l); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
    _Static_assert(std::is_same<int1, __decltype(~bool1_l)>::value, "");
    _Static_assert(std::is_same<int2, __decltype(~bool2_l)>::value, "");
    (~float3_l); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
    (~double4_l); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
    (~min10float1x2_l); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
    _Static_assert(std::is_same<uint2x3, __decltype(~uint2x3_l)>::value, "");
    _Static_assert(std::is_same<min16uint4x4, __decltype(~min16uint4x4_l)>::value, "");
    _Static_assert(std::is_same<int3x2, __decltype(~int3x2_l)>::value, "");
#ifdef OVERLOAD_IMPLEMENTED
    (!bool_l); //
    (!int_l); //
    (!uint_l); //
    (!half_l); //
    (!float_l); //
    (!double_l); //
    (!min16float_l); //
    (!min10float_l); //
    (!min16int_l); //
    (!min12int_l); //
    (!min16uint_l); //
    (!SamplerState_l); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
    (!f3_s_l); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
    (!mixed_s_l); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
    (!bool1_l); //
#endif
    _Static_assert(std::is_same<bool2, __decltype(!bool2_l)>::value, "");
    _Static_assert(std::is_same<bool3, __decltype(!float3_l)>::value, "");
    _Static_assert(std::is_same<bool4, __decltype(!double4_l)>::value, "");
    _Static_assert(std::is_same<bool1x2, __decltype(!min10float1x2_l)>::value, "");
    _Static_assert(std::is_same<bool2x3, __decltype(!uint2x3_l)>::value, "");
    _Static_assert(std::is_same<bool4x4, __decltype(!min16uint4x4_l)>::value, "");
    _Static_assert(std::is_same<bool3x2, __decltype(!int3x2_l)>::value, "");
    (--bool_l); // expected-error {{operator cannot be used with a bool lvalue}} fxc-error {{X3020: operator cannot be used with a bool lvalue}}
    (bool_l--); // expected-error {{operator cannot be used with a bool lvalue}} fxc-error {{X3020: operator cannot be used with a bool lvalue}}
    _Static_assert(std::is_same<int&, __decltype(--int_l)>::value, ""); // expected-error {{pointers are unsupported in HLSL}} fxc-pass {{}}
    _Static_assert(std::is_same<int, __decltype(int_l--)>::value, "");
    _Static_assert(std::is_same<uint&, __decltype(--uint_l)>::value, ""); // expected-error {{pointers are unsupported in HLSL}} fxc-pass {{}}
    _Static_assert(std::is_same<uint, __decltype(uint_l--)>::value, "");
    _Static_assert(std::is_same<half&, __decltype(--half_l)>::value, ""); // expected-error {{pointers are unsupported in HLSL}} fxc-pass {{}}
    _Static_assert(std::is_same<half, __decltype(half_l--)>::value, "");
    _Static_assert(std::is_same<float&, __decltype(--float_l)>::value, ""); // expected-error {{pointers are unsupported in HLSL}} fxc-pass {{}}
    _Static_assert(std::is_same<float, __decltype(float_l--)>::value, "");
    _Static_assert(std::is_same<double&, __decltype(--double_l)>::value, ""); // expected-error {{pointers are unsupported in HLSL}} fxc-pass {{}}
    _Static_assert(std::is_same<double, __decltype(double_l--)>::value, "");
    _Static_assert(std::is_same<min16float&, __decltype(--min16float_l)>::value, ""); // expected-error {{pointers are unsupported in HLSL}} fxc-pass {{}}
    _Static_assert(std::is_same<min16float, __decltype(min16float_l--)>::value, "");
    _Static_assert(std::is_same<min10float&, __decltype(--min10float_l)>::value, ""); // expected-error {{pointers are unsupported in HLSL}} fxc-pass {{}}
    _Static_assert(std::is_same<min10float, __decltype(min10float_l--)>::value, "");  // fxc-pass {{}}
    _Static_assert(std::is_same<min16int&, __decltype(--min16int_l)>::value, ""); // expected-error {{pointers are unsupported in HLSL}} fxc-pass {{}}
    _Static_assert(std::is_same<min16int, __decltype(min16int_l--)>::value, "");
    _Static_assert(std::is_same<min12int&, __decltype(--min12int_l)>::value, ""); // expected-error {{pointers are unsupported in HLSL}} fxc-pass {{}}
    _Static_assert(std::is_same<min12int, __decltype(min12int_l--)>::value, "");  // fxc-pass {{}}
    _Static_assert(std::is_same<min16uint&, __decltype(--min16uint_l)>::value, ""); // expected-error {{pointers are unsupported in HLSL}} fxc-pass {{}}
    _Static_assert(std::is_same<min16uint, __decltype(min16uint_l--)>::value, "");
    (--SamplerState_l); // expected-error {{scalar, vector, or matrix expected}} fxc-pass {{}}
    (SamplerState_l--); // expected-error {{scalar, vector, or matrix expected}} fxc-pass {{}}
    (--f3_s_l); // expected-error {{scalar, vector, or matrix expected}} fxc-pass {{}}
    (f3_s_l--); // expected-error {{scalar, vector, or matrix expected}} fxc-pass {{}}
    (--mixed_s_l); // expected-error {{scalar, vector, or matrix expected}} fxc-pass {{}}
    (mixed_s_l--); // expected-error {{scalar, vector, or matrix expected}} fxc-pass {{}}
    (--bool1_l); // expected-error {{operator cannot be used with a bool lvalue}} fxc-error {{X3020: operator cannot be used with a bool lvalue}}
    (bool1_l--); // expected-error {{operator cannot be used with a bool lvalue}} fxc-error {{X3020: operator cannot be used with a bool lvalue}}
    (--bool2_l); // expected-error {{operator cannot be used with a bool lvalue}} fxc-error {{X3020: operator cannot be used with a bool lvalue}}
    (bool2_l--); // expected-error {{operator cannot be used with a bool lvalue}} fxc-error {{X3020: operator cannot be used with a bool lvalue}}
    _Static_assert(std::is_same<float3&, __decltype(--float3_l)>::value, ""); // expected-error {{pointers are unsupported in HLSL}} fxc-pass {{}}
    _Static_assert(std::is_same<float3, __decltype(float3_l--)>::value, "");
    _Static_assert(std::is_same<double4&, __decltype(--double4_l)>::value, ""); // expected-error {{pointers are unsupported in HLSL}} fxc-pass {{}}
    _Static_assert(std::is_same<double4, __decltype(double4_l--)>::value, "");
    _Static_assert(std::is_same<min10float1x2&, __decltype(--min10float1x2_l)>::value, ""); // expected-error {{pointers are unsupported in HLSL}} fxc-pass {{}}
    _Static_assert(std::is_same<min10float1x2, __decltype(min10float1x2_l--)>::value, "");      /* fxc-pass {{}} */
    _Static_assert(std::is_same<uint2x3&, __decltype(--uint2x3_l)>::value, ""); // expected-error {{pointers are unsupported in HLSL}} fxc-pass {{}}
    _Static_assert(std::is_same<uint2x3, __decltype(uint2x3_l--)>::value, "");
    _Static_assert(std::is_same<min16uint4x4&, __decltype(--min16uint4x4_l)>::value, ""); // expected-error {{pointers are unsupported in HLSL}} fxc-pass {{}}
    _Static_assert(std::is_same<min16uint4x4, __decltype(min16uint4x4_l--)>::value, "");
    _Static_assert(std::is_same<int3x2&, __decltype(--int3x2_l)>::value, ""); // expected-error {{pointers are unsupported in HLSL}} fxc-pass {{}}
    _Static_assert(std::is_same<int3x2, __decltype(int3x2_l--)>::value, "");
    (++bool_l); // expected-error {{operator cannot be used with a bool lvalue}} fxc-error {{X3020: operator cannot be used with a bool lvalue}}
    (bool_l++); // expected-error {{operator cannot be used with a bool lvalue}} fxc-error {{X3020: operator cannot be used with a bool lvalue}}
    _Static_assert(std::is_same<int&, __decltype(++int_l)>::value, ""); // expected-error {{pointers are unsupported in HLSL}} fxc-pass {{}}
    _Static_assert(std::is_same<int, __decltype(int_l++)>::value, "");
    _Static_assert(std::is_same<uint&, __decltype(++uint_l)>::value, ""); // expected-error {{pointers are unsupported in HLSL}} fxc-pass {{}}
    _Static_assert(std::is_same<uint, __decltype(uint_l++)>::value, "");
    _Static_assert(std::is_same<half&, __decltype(++half_l)>::value, ""); // expected-error {{pointers are unsupported in HLSL}} fxc-pass {{}}
    _Static_assert(std::is_same<half, __decltype(half_l++)>::value, "");
    _Static_assert(std::is_same<float&, __decltype(++float_l)>::value, ""); // expected-error {{pointers are unsupported in HLSL}} fxc-pass {{}}
    _Static_assert(std::is_same<float, __decltype(float_l++)>::value, "");
    _Static_assert(std::is_same<double&, __decltype(++double_l)>::value, ""); // expected-error {{pointers are unsupported in HLSL}} fxc-pass {{}}
    _Static_assert(std::is_same<double, __decltype(double_l++)>::value, "");
    _Static_assert(std::is_same<min16float&, __decltype(++min16float_l)>::value, ""); // expected-error {{pointers are unsupported in HLSL}} fxc-pass {{}}
    _Static_assert(std::is_same<min16float, __decltype(min16float_l++)>::value, "");
    _Static_assert(std::is_same<min10float&, __decltype(++min10float_l)>::value, ""); // expected-error {{pointers are unsupported in HLSL}} fxc-pass {{}}
    _Static_assert(std::is_same<min10float, __decltype(min10float_l++)>::value, "");  // fxc-pass {{}}
    _Static_assert(std::is_same<min16int&, __decltype(++min16int_l)>::value, ""); // expected-error {{pointers are unsupported in HLSL}} fxc-pass {{}}
    _Static_assert(std::is_same<min16int, __decltype(min16int_l++)>::value, "");
    _Static_assert(std::is_same<min12int&, __decltype(++min12int_l)>::value, ""); // expected-error {{pointers are unsupported in HLSL}} fxc-pass {{}}
    _Static_assert(std::is_same<min12int, __decltype(min12int_l++)>::value, "");  // fxc-pass {{}}
    _Static_assert(std::is_same<min16uint&, __decltype(++min16uint_l)>::value, ""); // expected-error {{pointers are unsupported in HLSL}} fxc-pass {{}}
    _Static_assert(std::is_same<min16uint, __decltype(min16uint_l++)>::value, "");
    (++SamplerState_l); // expected-error {{scalar, vector, or matrix expected}} fxc-pass {{}}
    (SamplerState_l++); // expected-error {{scalar, vector, or matrix expected}} fxc-pass {{}}
    (++f3_s_l); // expected-error {{scalar, vector, or matrix expected}} fxc-pass {{}}
    (f3_s_l++); // expected-error {{scalar, vector, or matrix expected}} fxc-pass {{}}
    (++mixed_s_l); // expected-error {{scalar, vector, or matrix expected}} fxc-pass {{}}
    (mixed_s_l++); // expected-error {{scalar, vector, or matrix expected}} fxc-pass {{}}
    (++bool1_l); // expected-error {{operator cannot be used with a bool lvalue}} fxc-error {{X3020: operator cannot be used with a bool lvalue}}
    (bool1_l++); // expected-error {{operator cannot be used with a bool lvalue}} fxc-error {{X3020: operator cannot be used with a bool lvalue}}
    (++bool2_l); // expected-error {{operator cannot be used with a bool lvalue}} fxc-error {{X3020: operator cannot be used with a bool lvalue}}
    (bool2_l++); // expected-error {{operator cannot be used with a bool lvalue}} fxc-error {{X3020: operator cannot be used with a bool lvalue}}
    _Static_assert(std::is_same<float3&, __decltype(++float3_l)>::value, ""); // expected-error {{pointers are unsupported in HLSL}} fxc-pass {{}}
    _Static_assert(std::is_same<float3, __decltype(float3_l++)>::value, "");
    _Static_assert(std::is_same<double4&, __decltype(++double4_l)>::value, ""); // expected-error {{pointers are unsupported in HLSL}} fxc-pass {{}}
    _Static_assert(std::is_same<double4, __decltype(double4_l++)>::value, "");
    _Static_assert(std::is_same<min10float1x2&, __decltype(++min10float1x2_l)>::value, ""); // expected-error {{pointers are unsupported in HLSL}} fxc-pass {{}}
    _Static_assert(std::is_same<min10float1x2, __decltype(min10float1x2_l++)>::value, "");  /* fxc-pass {{}} */
    _Static_assert(std::is_same<uint2x3&, __decltype(++uint2x3_l)>::value, ""); // expected-error {{pointers are unsupported in HLSL}} fxc-pass {{}}
    _Static_assert(std::is_same<uint2x3, __decltype(uint2x3_l++)>::value, "");
    _Static_assert(std::is_same<min16uint4x4&, __decltype(++min16uint4x4_l)>::value, ""); // expected-error {{pointers are unsupported in HLSL}} fxc-pass {{}}
    _Static_assert(std::is_same<min16uint4x4, __decltype(min16uint4x4_l++)>::value, "");
    _Static_assert(std::is_same<int3x2&, __decltype(++int3x2_l)>::value, ""); // expected-error {{pointers are unsupported in HLSL}} fxc-pass {{}}
    _Static_assert(std::is_same<int3x2, __decltype(int3x2_l++)>::value, "");

    // Tests with multidimensional arrays.
    int arr23[2][3] = { 1, 2, 3, 4, 5, 6 };
    // fxc error X3000: syntax error: unexpected token '{'
    arr23 = { 1, 2, 3, 4, 5, 6 }; // expected-error {{expected expression}} fxc-error {{X3000: syntax error: unexpected token '{'}}
    // fxc error X3000: syntax error: unexpected token '{'
    arr23[0] = { 1, 2, 3 }; // expected-error {{expected expression}} fxc-error {{X3000: syntax error: unexpected token '{'}}
    // fxc error X3000: syntax error: unexpected token '{'
    arr23 = { 1, 2, 3, 4, 5, 6 }; // expected-error {{expected expression}} fxc-error {{X3000: syntax error: unexpected token '{'}}

    float farr23[2][3];
    // fxc error X3017: cannot convert from 'int[2][3]' to 'float[2][3]'
    farr23 = arr23; // expected-error {{cannot implicitly convert from 'int [2][3]' to 'float [2][3]'}} fxc-error {{X3017: cannot convert from 'int[2][3]' to 'float[2][3]'}}
    // arithmetic
    // fxc error X3022: scalar, vector, or matrix expected
    farr23 = farr23 + farr23; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
    // fxc error X3022: scalar, vector, or matrix expected
    farr23 = farr23 * farr23; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
    // fxc error X3022: scalar, vector, or matrix expected
    farr23 = farr23 / farr23; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
    // fxc error X3022: scalar, vector, or matrix expected
    farr23 = farr23 % farr23; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
    // order comparison
    // fxc error X3022: scalar, vector, or matrix expected
    bools = farr23 < farr23; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
    // fxc error X3022: scalar, vector, or matrix expected
    bools = farr23 <= farr23; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
    // fxc error X3022: scalar, vector, or matrix expected
    bools = farr23 > farr23; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
    // fxc error X3022: scalar, vector, or matrix expected
    bools = farr23 >= farr23; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
    // equality comparison
    // fxc error X3020: type mismatch
    bools = farr23 == farr23; // expected-error {{equality operators cannot be used with array types}} fxc-error {{X3020: type mismatch}}
    // fxc error X3020: type mismatch
    bools = farr23 != farr23; // expected-error {{equality operators cannot be used with array types}} fxc-error {{X3020: type mismatch}}
    // bitwise
    // fxc error X3082: int or unsigned int type required
    arr23 = arr23 << 1; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
    // fxc error X3082: int or unsigned int type required
    arr23 = arr23 >> 1; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
    // fxc error X3082: int or unsigned int type required
    arr23 = arr23 & 1; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
    // fxc error X3082: int or unsigned int type required
    arr23 = arr23 & arr23; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
    // fxc error X3082: int or unsigned int type required
    arr23 = arr23 | 1; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
    // fxc error X3082: int or unsigned int type required
    arr23 = arr23 | arr23; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
    // fxc error X3082: int or unsigned int type required
    arr23 = arr23 ^ 1; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
    // fxc error X3082: int or unsigned int type required
    arr23 = arr23 ^ arr23; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
    // logical
    // fxc error X3022: scalar, vector, or matrix expected
    bools = farr23 && farr23; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
    // fxc error X3022 : scalar, vector, or matrix expected
    bools = farr23 || farr23; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
    farr23 = farr23;
    // fxc error X3022 : scalar, vector, or matrix expected
    farr23 += 1; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
    // fxc error X3022 : scalar, vector, or matrix expected
    farr23 = +farr23; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}

    float farr3[3];
    farr3 = farr23[0];
    farr3 = farr23[1];

    // Tests with structures with all primitives.
    many_s many_l = g_many_s;
    u3_s u3_l = g_u3_s;

    // fxc error: error X3022: scalar, vector, or matrix expected
    many_l = many_l + many_l; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
    // fxc error X3022: scalar, vector, or matrix expected
    many_l = many_l + 1; // expected-error {{vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
    // fxc error X3022: scalar, vector, or matrix expected
    bool_l = many_l > many_l; // expected-error {{vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
    // fxc error X3020: type mismatch
    bool_l = many_l == many_l; // expected-error {{operator cannot be used with user-defined type 'many_s'}} fxc-error {{X3020: type mismatch}}
    // fxc error X3022: scalar, vector, or matrix expected
    many_l = !many_l; // expected-error {{vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
    // fxc error X3022: scalar, vector, or matrix expected
    many_l = -many_l; // expected-error {{vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
    // error X3082: int or unsigned int type required
    many_l = many_l << 1; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
    // error X3022: scalar, vector, or matrix expected
    many_l += 1; // expected-error {{, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}

    // fxc error: error X3022: scalar, vector, or matrix expected
    u3_l = u3_l + u3_l; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
    // fxc error X3022: scalar, vector, or matrix expected
    u3_l = u3_l + 1; // expected-error {{vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
    // fxc error X3022: scalar, vector, or matrix expected
    bool_l = u3_l > u3_l; // expected-error {{vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
    // fxc error X3020: type mismatch
    bool_l = u3_l == u3_l; // expected-error {{operator cannot be used with user-defined type 'u3_s'}} fxc-error {{X3020: type mismatch}}
    // fxc error X3022: scalar, vector, or matrix expected
    u3_l = !u3_l; // expected-error {{vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
    // fxc error X3022: scalar, vector, or matrix expected
    u3_l = -u3_l; // expected-error {{vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
    // error X3082: int or unsigned int type required
    u3_l = u3_l << 1; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
    // error X3022: scalar, vector, or matrix expected
    u3_l += 1; // expected-error {{, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}

    // Tests with comma operator.
    _Static_assert(std::is_same<min16uint, __decltype(min16uint_l, min16uint_l)>::value, "");
    _Static_assert(std::is_same<min16uint, __decltype(float_l, min16uint_l)>::value, "");
    _Static_assert(std::is_same<many_s, __decltype(farr3, many_l)>::value, "");
    _Static_assert(std::is_same<SamplerState, __decltype(farr3, SamplerStates)>::value, "");
    _Static_assert(std::is_same<int3, __decltype(farr3, (int3)i4)>::value, "");

    // Note that this would work in C/C++.
    (floats, ints) = 1; // expected-error {{expression is not assignable}} fxc-error {{X3025: l-value specifies const object}} fxc-warning {{X3081: comma expression used where a vector constructor may have been intended}}
};

Texture2D tex12[12];
SamplerState samp;

float4 DoSample(Texture2D tex[12], float2 coord) {
  return tex[3].Sample(samp, -dot(coord, -foo));    // expected-error {{use of undeclared identifier 'foo'}} fxc-error {{X3004: undeclared identifier 'foo'}} fxc-error {{X3013:     dot(floatM|halfM|doubleM|min10floatM|min16floatM|intM|uintM|min12intM|min16intM|min16uintM, floatM|halfM|doubleM|min10floatM|min16floatM|intM|uintM|min12intM|min16intM|min16uintM)}} fxc-error {{X3013: 'dot': no matching 2 parameter intrinsic function}} fxc-error {{X3013: Possible intrinsic functions are:}}
}

float3 fn(float a, float b)
{
    float3 bar = (float3(2,3,4) * foo) ? a : b;     // expected-error {{use of undeclared identifier 'foo'}} fxc-error {{X3004: undeclared identifier 'foo'}}
    return bar < 1 ? float3(foo) : bar;             // expected-error {{use of undeclared identifier 'foo'}} fxc-error {{X3004: undeclared identifier 'bar'}}
}

[shader("pixel")]
float4 main(float2 coord : TEXCOORD) : SV_Target
{
  return DoSample(tex12, fn(coord.x, coord.y).xy);
}

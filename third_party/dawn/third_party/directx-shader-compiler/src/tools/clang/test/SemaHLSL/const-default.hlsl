// RUN: %dxc -Tlib_6_3  -Wno-unused-value   -verify %s
// RUN: %dxc -Tps_6_0  -Wno-unused-value   -verify %s

float g_float1;                                             /* expected-note {{variable 'g_float1' declared const here}} expected-note {{variable 'g_float1' declared const here}} fxc-pass {{}} */
int4 g_vec1;                                                /* expected-note {{variable 'g_vec1' declared const here}} expected-note {{variable 'g_vec1' declared const here}} fxc-pass {{}} */
uint64_t3x4 g_mat1;                                         /* fxc-error {{X3000: unrecognized identifier 'uint64_t3x4'}} */

cbuffer g_cbuffer {
    min12int m_buffer_min12int;                             /* expected-note {{variable 'm_buffer_min12int' declared const here}} expected-warning {{'min12int' is promoted to 'min16int'}} fxc-pass {{}} */
    float4 m_buffer_float4;                                 /* expected-note {{variable 'm_buffer_float4' declared const here}} fxc-pass {{}} */
    int3x4 m_buffer_int3x4;
}

tbuffer g_tbuffer {
    float m_tbuffer_float;                                  /* expected-note {{variable 'm_tbuffer_float' declared const here}} fxc-pass {{}} */
    int3 m_tbuffer_int3;                                    /* expected-note {{variable 'm_tbuffer_int3' declared const here}} fxc-pass {{}} */
    double2x1 m_tbuffer_double2x1;                          /* expected-note {{variable 'm_tbuffer_double2x1' declared const here}} fxc-pass {{}} */
}

struct MyStruct {
    float3 my_float3;
    int3x4 my_int3x4;
};

ConstantBuffer<MyStruct> g_const_buffer;
TextureBuffer<MyStruct> g_texture_buffer;

class MyClass {
    float3 my_float3;
    int3x4 my_int3x4;
};

ConstantBuffer<MyClass> g_const_buffer2;
TextureBuffer<MyClass> g_texture_buffer2;

// expected-note@+2 {{forward declaration of 'FWDDeclStruct'}}
// expected-note@+1 {{forward declaration of 'FWDDeclStruct'}}
struct FWDDeclStruct;
// expected-note@+2 {{forward declaration of 'FWDDeclClass'}}
// expected-note@+1 {{forward declaration of 'FWDDeclClass'}}
class FWDDeclClass;

// Ensure forward declared struct/class fails as expected
ConstantBuffer<FWDDeclStruct> g_const_buffer3;              /* expected-error {{variable has incomplete type 'FWDDeclStruct'}} */
TextureBuffer<FWDDeclStruct> g_texture_buffer3;             /* expected-error {{variable has incomplete type 'FWDDeclStruct'}} */
ConstantBuffer<FWDDeclClass> g_const_buffer4;               /* expected-error {{variable has incomplete type 'FWDDeclClass'}} */
TextureBuffer<FWDDeclClass> g_texture_buffer4;              /* expected-error {{variable has incomplete type 'FWDDeclClass'}} */

[shader("pixel")]
float4 main() : SV_TARGET
{
    g_float1 = g_float1 + 10.0;                             /* expected-error {{cannot assign to variable 'g_float1' with const-qualified type 'const float'}} fxc-error {{X3025: global variables are implicitly constant, enable compatibility mode to allow modification}} */
    g_float1 += 3.5;                                         /* expected-error {{cannot assign to variable 'g_float1' with const-qualified type 'const float'}} fxc-error {{X3025: global variables are implicitly constant, enable compatibility mode to allow modification}} */
    g_vec1 = g_vec1 + 3;                                    /* expected-error {{cannot assign to variable 'g_vec1' with const-qualified type 'const int4'}} fxc-error {{X3025: global variables are implicitly constant, enable compatibility mode to allow modification}} */
    g_vec1 -= 1;                                            /* expected-error {{cannot assign to variable 'g_vec1' with const-qualified type 'const int4'}} fxc-error {{X3025: global variables are implicitly constant, enable compatibility mode to allow modification}} */
    g_mat1._12 = 3;                                         /* expected-error {{read-only variable is not assignable}} fxc-error {{X3004: undeclared identifier 'g_mat1'}} */
    g_mat1._34 *= 4;                                        /* expected-error {{read-only variable is not assignable}} fxc-error {{X3004: undeclared identifier 'g_mat1'}} */
    m_buffer_min12int = 12;                                 /* expected-error {{cannot assign to variable 'm_buffer_min12int' with const-qualified type 'const min12int'}} fxc-error {{X3025: global variables are implicitly constant, enable compatibility mode to allow modification}} */
    m_buffer_float4 += 3.4;                                 /* expected-error {{cannot assign to variable 'm_buffer_float4' with const-qualified type 'const float4'}} fxc-error {{X3025: global variables are implicitly constant, enable compatibility mode to allow modification}} */
    m_buffer_int3x4._m01 -= 10;                             /* expected-error {{read-only variable is not assignable}} fxc-error {{X3025: global variables are implicitly constant, enable compatibility mode to allow modification}} */
    m_tbuffer_float *= 2;                                   /* expected-error {{cannot assign to variable 'm_tbuffer_float' with const-qualified type 'const float'}} fxc-error {{X3025: global variables are implicitly constant, enable compatibility mode to allow modification}} */
    m_tbuffer_int3 = 10;                                    /* expected-error {{cannot assign to variable 'm_tbuffer_int3' with const-qualified type 'const int3'}} fxc-error {{X3025: global variables are implicitly constant, enable compatibility mode to allow modification}} */
    m_tbuffer_double2x1 *= 3;                               /* expected-error {{cannot assign to variable 'm_tbuffer_double2x1' with const-qualified type 'const double2x1'}} fxc-error {{X3025: global variables are implicitly constant, enable compatibility mode to allow modification}} */

    g_const_buffer.my_float3.x = 1.5;                       /* expected-error {{read-only variable is not assignable}} fxc-error {{X3025: global variables are implicitly constant, enable compatibility mode to allow modification}} */
    g_const_buffer.my_int3x4._21 -= 2;                      /* expected-error {{read-only variable is not assignable}} fxc-error {{X3025: global variables are implicitly constant, enable compatibility mode to allow modification}} */
    g_texture_buffer.my_float3.y += 2.0;                      /* expected-error {{read-only variable is not assignable}} fxc-error {{X3025: global variables are implicitly constant, enable compatibility mode to allow modification}} */
    g_texture_buffer.my_int3x4._14 = 3;                     /* expected-error {{read-only variable is not assignable}} fxc-error {{X3025: global variables are implicitly constant, enable compatibility mode to allow modification}} */

    g_const_buffer2.my_float3.x = 1.5;                      /* expected-error {{read-only variable is not assignable}} fxc-error {{X3025: global variables are implicitly constant, enable compatibility mode to allow modification}} */
    g_const_buffer2.my_int3x4._21 -= 2;                     /* expected-error {{read-only variable is not assignable}} fxc-error {{X3025: global variables are implicitly constant, enable compatibility mode to allow modification}} */
    g_texture_buffer2.my_float3.y += 2.0;                     /* expected-error {{read-only variable is not assignable}} fxc-error {{X3025: global variables are implicitly constant, enable compatibility mode to allow modification}} */
    g_texture_buffer2.my_int3x4._14 = 3;                    /* expected-error {{read-only variable is not assignable}} fxc-error {{X3025: global variables are implicitly constant, enable compatibility mode to allow modification}} */

    return (float4)g_float1;
}

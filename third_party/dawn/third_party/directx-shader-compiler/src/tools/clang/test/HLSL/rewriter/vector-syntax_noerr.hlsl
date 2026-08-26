// RUN: %clang_cc1 -fsyntax-only -ffreestanding -verify %s

// To test with the classic compiler, run
// %sdxroot%\tools\x86\fxc.exe /T vs_5_1 vector-syntax.hlsl

float fn() {
    float4 myvar = float4(1,2,3,4);
    myvar.x = 1.0f;
    myvar.y = 1.0f;
    myvar.z = 1.0f;
    myvar.w = 1.0f;

    float4 myothervar;
    myothervar.rgba = myvar.xyzw;

    float f;
    f.x = 1;
    
    uint u;
    u = f.x;
    
    uint3 u3;
    u3.xyz = f.xxx;
    //f.xx = 2; // expected-error {{vector is not assignable (contains duplicate components)}} fxc-error {{X3025: l-value specifies const object}} 

    return f.x;
}

// float4 main(float4 param4 : FOO) : FOO {
float4 plain(float4 param4 /* : FOO */) /*: FOO */{
  return fn();
}
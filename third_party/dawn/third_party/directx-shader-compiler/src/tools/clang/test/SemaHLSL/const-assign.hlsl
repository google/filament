// RUN: %dxc -Tlib_6_3  -Wno-unused-value   -verify %s
// RUN: %dxc -Tps_6_0  -Wno-unused-value   -verify %s

[shader("pixel")]
float4 main() : SV_TARGET
{
    const float c = 2.0; // expected-note {{variable 'c' declared const here}} expected-note {{variable 'c' declared const here}} fxc-pass {{}}
    c = c + 3.0; // expected-error {{cannot assign to variable 'c' with const-qualified type 'const float'}} fxc-error {{X3025: l-value specifies const object}}
    c += 3.0; // expected-error {{cannot assign to variable 'c' with const-qualified type 'const float'}} fxc-pass {{}}
    return (float4)c;
}
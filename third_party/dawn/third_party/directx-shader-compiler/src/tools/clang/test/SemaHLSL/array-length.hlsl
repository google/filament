// RUN: %dxc -Tlib_6_3   -verify -HV 2016  %s
// RUN: %dxc -Tps_6_0   -verify -HV 2016  %s

// :FXC_VERIFY_ARGUMENTS: /T ps_5_0 /E main

float4 planes1[8];
float4 planes2[3+2]; 

struct S {
    float4 planes[2]; 
};

[shader("pixel")]
[RootSignature("CBV(b0, space=0, visibility=SHADER_VISIBILITY_ALL)")]
float main(S s:POSITION) : SV_Target {
    float4 planes3[] = {{ 1.0, 2.0, 3.0, 4.0 }};
    
    int total = planes1.Length;	// expected-warning {{Length is deprecated}} fxc-pass {{}}
    total += planes2.Length;  	// expected-warning {{Length is deprecated}} fxc-pass {{}}
    total += planes3.Length;    // expected-warning {{Length is deprecated}} fxc-pass {{}}
    total += s.planes.Length;   // expected-warning {{Length is deprecated}} fxc-pass {{}}

    return total;
}
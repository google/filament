// RUN: %dxc -Tlib_6_3 -Wno-unused-value -verify %s
// RUN: %dxc -Tps_6_0 -Wno-unused-value -verify %s

// To test with the classic compiler, run
// %sdxroot%\tools\x86\fxc.exe /T ps_5_1 uint4add3.hlsl
// :FXC_VERIFY_ARGUMENTS: /E main /T ps_5_1

// we use -Wno-unused-value because we generate some no-op expressions to yield errors
// without also putting them in a static assertion

[shader("pixel")]
float4 main(float4 a : A, float3 c :C) : SV_TARGET {
  float4 b = a;
  b += a.xyz;         /* expected-error {{cannot convert from 'vector<float, 3>' to 'float4'}} fxc-error {{X3017: cannot implicitly convert from 'const float3' to 'float4'}} fxc-warning {{X3206: implicit truncation of vector type}} */
  float4 d = 0;
  d = b+c;            /* expected-error {{cannot convert from 'float3' to 'float4'}} expected-warning {{implicit truncation of vector type}} fxc-error {{X3017: cannot implicitly convert from 'const float3' to 'float4'}} fxc-warning {{X3206: implicit truncation of vector type}} */
  return b + d;
}

// RUN: %dxc -T ps_6_0 -E main -verify %s

// Quoted #includes should still go through the normal filesystem search
// path; the embedded-headers mechanism only kicks in for angle-bracket
// includes.  Without a -I pointing at the hlsl/ directory, this quoted
// include must fail.

#include "enable_if.h" // expected-error{{'enable_if.h' file not found}}

float4 main() : SV_Target { return 0; }


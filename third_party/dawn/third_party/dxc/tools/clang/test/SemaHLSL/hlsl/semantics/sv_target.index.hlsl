// RUN: %dxc -T ps_6_0 -E zero -verify %s
// RUN: %dxc -T ps_6_0 -E seven -verify %s

float4 zero() : SV_Target0 { /* expected-no-diagnostics */
    return float4(0, 1, 2, 3);
}

float4 seven() : SV_Target7 { /* expected-no-diagnostics */
    return float4(0, 1, 2, 3);
}
